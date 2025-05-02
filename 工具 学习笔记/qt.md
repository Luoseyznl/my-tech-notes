#### Qt 项目结构

```
MyQtProject/
├── MyQtProject.pro       # 项目配置文件，定义依赖、源文件、资源
├── MyQtProject.pro.user  # Qt Creator用户配置文件（不纳入版本控制）
├── main.cpp              # 程序入口，初始化QApplication和主窗口
├── mainwindow.h          # 主窗口头文件，声明类和信号槽
├── mainwindow.cpp        # 主窗口实现文件，定义逻辑
├── mainwindow.ui         # 主窗口界面文件（Qt Designer生成）
├── resources/            # 资源文件夹
│   ├── images/           # 图片资源
│   │   └── icon.png
│   └── resources.qrc     # 资源配置文件，管理嵌入资源
├── forms/                # 界面文件文件夹（可选，存放多个.ui文件）
│   └── mainwindow.ui
├── translations/         # 翻译文件文件夹（可选，支持国际化）
│   ├── app_en.ts         # 英语翻译源文件
│   └── app_zh.qm         # 中文翻译编译文件
└── build/                # 构建生成目录（自动生成）
├── debug/            # 调试版本，带调试信息的可执行文件
└── release/          # 发布版本，优化后的可执行文件
```


- 编译链接：
1. `qmake` 解析 `.pro` 文件，生成 `Makefile`（跨平台支持，如Linux用 `gcc`，Windows用 `msvc`）。
2. `moc` 处理包含 `Q_OBJECT` 宏的头文件，生成 `moc_*.cpp`，支持信号槽。
3. `uic` 将 `.ui` 文件转换为C++头文件（如 `ui_mainwindow.h`），描述界面布局。
4. `rcc` 将 `.qrc` 文件编译为 `qrc_*.cpp`，嵌入资源（如图片）。
5. `make` 调用编译器（如 `g++`），编译 `.cpp` 文件为目标文件（`.o` 或 `.obj`）。
6. `ld`（链接器）将目标文件和Qt库链接，生成可执行文件。
7. （可选）`make clean` 清理中间文件，`make distclean` 清理所有生成文件。

#### 基类
```
QObject
   ├── QCoreApplication
   │   └── QGuiApplication
   │       └── QApplication
   ├── QWidget
   │   ├── QFrame
   │   │   └── QMainWindow
   │   ├── QAbstractButton
   │   │   └── QPushButton
   │   ├── QLabel
   │   ├── QLineEdit
   │   ├── QDialog
   │   └── QAbstractScrollArea
   │       ├── QTextEdit
   │       └── QTableView
   ├── QTimer
   ├── QThread
   ├── QAbstractItemModel
   └── QLayout
       ├── QVBoxLayout
       └── QHBoxLayout
```
1. 核心基类 QObject
   - 核心机制：信号槽、事件循环、父子对象管理（内存）、动态属性系统
2. 控件基类 QWidget
    `QPushButton`、`QLabel`、`QLineEdit`、`QMainWindow`
   - 提供功能：可视化、交互（事件处理）、坐标系统
3. 主窗口基类 QMainWindow
   - 提供标准窗口布局（菜单栏、工具栏、状态栏、可停靠窗口）、支持 MDI
4. 应用程序 QApplication
   - 管理主事件循环、全局设置、协调控件

#### 对象树模型
1. 对象层次管理：通过父子关系组织对象，便于管理控件、管理资源、传递事件
2. 自动内存管理：当父对象销毁时，其所有子对象会自动销毁

#### 信号槽机制
1. 信号：signal 成员函数（由 Qt 元对象编译器 moc 生成）
2. 槽：`访问控制符 slots` 成员函数（可以被信号触发，也可以直接调用）
3. 连接：使用 `QObject::connect` 建立信号和槽的连接
    ```cpp
    connect(sender, &Sender::signalName, receiver, &Receiver::slotName);
    ```
   - 连接类型：
    Qt::AutoConnection（默认）：根据线程自动选择直接调用或排队
    Qt::DirectConnection：立即调用槽函数（同一线程）
    Qt::QueuedConnection：信号排队，稍后在接收者线程中调用（跨线程）
4. 元对象编译器（moc）
   - moc 工具解析包含 `Q_OBJECT` 宏的头文件，生成信号槽的元代码
        ```cpp
        class MyClass : public QObject {
            Q_OBJECT
        public:
            MyClass(QObject *parent = nullptr) : QObject(parent) {}
        signals:
            void mySignal();
        public slots:
            void mySlot();
        };
        ```
