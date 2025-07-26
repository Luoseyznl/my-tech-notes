#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>   // Qt 文件对话框，用于选择文件
#include <QMessageBox>   // Qt 消息框，显示提示或错误
#include <QToolBar>      // Qt 工具栏，显示快捷按钮

// 构造函数：初始化主窗口，设置界面和信号槽连接
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) // 基类构造函数，初始化主窗口
    , ui(new Ui::MainWindow) // 分配 UI 对象，由 Qt Designer 生成
    , statusLabel(new QLabel(tr("Ready"))) // 创建状态栏标签，初始文本为“就绪”（支持翻译）
    , isTextModified(false) // 初始化 QTextEdit 修改状态为未修改
{
    ui->setupUi(this); // 加载 mainwindow.ui，初始化界面（菜单栏、工具栏、QTextEdit 等）

    // 启用 QTextEdit 的撤销/重做功能（默认启用，但显式设置以确保）
    ui->textEdit->setUndoRedoEnabled(true);

    // 设置状态栏标签：左对齐，添加到状态栏并拉伸占满空间
    statusLabel->setAlignment(Qt::AlignLeft); // 文本左对齐
    ui->statusBar->addPermanentWidget(statusLabel, 1); // 添加到状态栏，1 表示拉伸因子

    // 连接信号和槽：响应用户操作（如菜单点击、文本变化）
    // 菜单动作的 triggered 信号连接到对应槽函数
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(ui->actionInsertText, &QAction::triggered, this, &MainWindow::onInsertText);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExit);
    // QTextEdit 的 textChanged 信号连接到 onTextChanged，跟踪文本修改
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::onUndo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::onRedo);

    // 连接 QTextEdit 的撤销/重做状态变化信号
    connect(ui->textEdit->document(), &QTextDocument::undoAvailable, this, &MainWindow::updateUndoRedoState);
    connect(ui->textEdit->document(), &QTextDocument::redoAvailable, this, &MainWindow::updateUndoRedoState);

    // 初始化界面：加载 Notepad 内容到 QTextEdit，更新状态栏
    updateTextEdit();
    updateStatusBar();
    updateUndoRedoState(false); // 初始状态：撤销/重做禁用
}

// 析构函数：清理 UI 对象（状态栏标签由对象树自动管理）
MainWindow::~MainWindow() {
    delete ui; // 释放 UI 对象，自动清理 QTextEdit、菜单栏等
}

// 槽函数：处理“新建”动作
void MainWindow::onNewFile() {
    // 检查是否有未保存的更改（QTextEdit 或 Notepad）
    if (isTextModified || notepad.isFileModified()) {
        // 显示提示框，询问是否保存
        auto ret = QMessageBox::question(
            this, tr("New File"), tr("File has unsaved changes. Save before creating new?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            onSaveFile(); // 保存当前内容
        } else if (ret == QMessageBox::Cancel) {
            return; // 取消新建操作
        }
    }
    try {
        notepad.newFile();      // 调用 Notepad 清空内容
        ui->textEdit->clear();  // 清空 QTextEdit
        isTextModified = false; // 重置修改状态
        updateStatusBar();      // 更新状态栏显示
        updateUndoRedoState(false);
    } catch (const std::exception &e) {
        // 显示错误消息（例如未保存更改）
        ui->statusBar->showMessage(tr("Cannot create new file: %1").arg(e.what()), 5000);
    }
}

// 槽函数：处理“打开”动作
void MainWindow::onOpenFile() {
    // 检查未保存更改
    if (isTextModified || notepad.isFileModified()) {
        auto ret = QMessageBox::question(
            this, tr("Open File"), tr("File has unsaved changes. Save before opening?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            onSaveFile();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    // 打开文件对话框，选择 .txt 文件
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        if (notepad.openFile(fileName.toStdString())) {
            updateTextEdit();       // 加载文件内容到 QTextEdit
            isTextModified = false; // 重置修改状态
            updateStatusBar();      // 更新状态栏
            updateUndoRedoState(false); // 重置撤销/重做状态
        } else {
            // 显示错误消息（例如文件无法打开）
            ui->statusBar->showMessage(tr("Cannot open file"), 5000);
        }
    }
}

// 槽函数：处理“保存”动作
void MainWindow::onSaveFile() {
    // 同步 QTextEdit 内容到 Notepad
    notepad.editFile(ui->textEdit->toPlainText().toStdString());
    if (notepad.saveFile()) {
        isTextModified = false; // 重置修改状态
        updateStatusBar();      // 更新状态栏
        ui->statusBar->showMessage(tr("File saved"), 5000); // 显示临时消息
    } else {
        // 如果没有文件名（例如新文件），调用另存为
        onSaveAs();
    }
}

// 槽函数：处理“另存为”动作
void MainWindow::onSaveAs() {
    // 打开保存对话框，选择文件名
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save As"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        // 同步 QTextEdit 内容到 Notepad
        notepad.editFile(ui->textEdit->toPlainText().toStdString());
        try {
            if (notepad.saveAs(fileName.toStdString())) {
                isTextModified = false; // 重置修改状态
                updateStatusBar();      // 更新状态栏
                ui->statusBar->showMessage(tr("File saved"), 5000);
            } else {
                ui->statusBar->showMessage(tr("Cannot save file"), 5000);
            }
        } catch (const std::exception &e) {
            ui->statusBar->showMessage(tr("Cannot save file: %1").arg(e.what()), 5000);
        }
    }
}

// 槽函数：处理“插入文本”动作（可选功能）
void MainWindow::onInsertText() {
    // 示例：插入固定文本到 QTextEdit
    ui->textEdit->insertPlainText("New text\n");
    isTextModified = true; // 标记文本已修改
    updateStatusBar();     // 更新状态栏
    // 不需手动调用 updateUndoRedoState，textChanged 信号会触发
}

// 槽函数：处理“退出”动作
void MainWindow::onExit() {
    // 检查未保存更改
    if (isTextModified || notepad.isFileModified()) {
        auto ret = QMessageBox::question(
            this, tr("Exit"), tr("File has unsaved changes. Save before exit?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            onSaveFile();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    // 退出应用程序
    QApplication::quit();
}

// 槽函数：处理“撤销”动作
void MainWindow::onUndo() {
    ui->textEdit->undo(); // 调用 QTextEdit 的撤销功能
    // updateUndoRedoState 由 redoAvailable 信号自动触发
}

// 槽函数：处理“重做”动作
void MainWindow::onRedo() {
    ui->textEdit->redo(); // 调用 QTextEdit 的重做功能
}

// 槽函数：处理 QTextEdit 的文本变化
void MainWindow::onTextChanged() {
    isTextModified = true; // 标记 QTextEdit 已修改
    updateStatusBar();     // 更新状态栏显示“ (Modified)”
}

// 更新 QTextEdit 内容，从 Notepad 加载文本
void MainWindow::updateTextEdit() {
    QString text;
    // 将 Notepad 的 lines 转换为 QString，逐行添加换行符
    for (const auto &line : notepad.getLines()) {
        text += QString::fromStdString(line) + "\n";
    }
    ui->textEdit->setText(text); // 设置 QTextEdit 内容
    isTextModified = false;      // 重置修改状态（刚加载内容）
    updateUndoRedoState(false); // 重置撤销/重做状态
}

// 更新状态栏，显示文件名和修改状态
void MainWindow::updateStatusBar() {
    // 获取当前文件名，转换为 QString
    QString status = tr("File: %1").arg(QString::fromStdString(notepad.getCurrentFile()));
    // 如果 QTextEdit 或 Notepad 有修改，显示“ (Modified)”
    if (isTextModified || notepad.isFileModified()) {
        status += tr(" (Modified)");
    }
    statusLabel->setText(status); // 更新状态栏标签
}

// 修正：更新撤销/重做动作的启用状态
void MainWindow::updateUndoRedoState(bool /*available*/) {
    // 使用 QTextDocument 的 isUndoAvailable 和 isRedoAvailable
    ui->actionUndo->setEnabled(ui->textEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->textEdit->document()->isRedoAvailable());
}
