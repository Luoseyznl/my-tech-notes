#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> // Qt 主窗口类，提供菜单栏、工具栏、状态栏等功能
#include <QLabel>      // Qt 标签控件，用于状态栏显示文本
#include "Notepad.h"   // 自定义 Notepad 类，处理文件操作逻辑

namespace Ui {
class MainWindow;   // 前向声明，Qt Designer 生成的 UI 类
}

class MainWindow : public QMainWindow {
    Q_OBJECT    // Qt 宏，启用信号槽机制，必须包含以支持 Qt 的元对象编译器 (moc)

public:
    // 构造：初始化主窗口，parent 是父窗口指针（默认为 nullptr）
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Qt 槽函数，响应信号（如菜单点击、文本变化）
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAs();
    void onInsertText();
    void onExit();
    void onTextChanged();
    void onUndo();
    void onRedo();
    // 更新撤销/重做动作状态
    void updateUndoRedoState(bool available); // 参数接收 undoAvailable/redoAvailable 信号

private:
    void updateTextEdit();  // 更新 QTextEdit 内容，从 Notepad 加载文本
    void updateStatusBar(); // 更新状态栏，显示文件名和修改状态
    Ui::MainWindow *ui;                // Qt Designer 生成的 UI 对象，管理界面元素
    my::notepad::Notepad notepad;      // Notepad 实例，处理文件操作
    QLabel *statusLabel;               // 状态栏的 QLabel，显示文件状态
    bool isTextModified;               // 跟踪 QTextEdit 是否修改，优化保存逻辑
};

#endif // MAINWINDOW_H
