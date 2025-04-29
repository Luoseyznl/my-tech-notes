#### Qt 项目
```
.pro 文件：项目配置文件，定义依赖、源文件等。
.cpp 文件：主程序逻辑。
.h 文件：头文件，声明类。
.ui 文件：界面设计文件（使用 Qt Designer 设计 UI）。
```
- 编译链接：
  1. qmake 生成 Makefile（基于 .pro 文件），然后编译
  2. Qt Creator 自动链接 Qt 库（例如 QtCore、QtGui、QtWidgets）

#### Qt 的基本语法
1. QObject 和信号槽机制：信号（事件发生时发出） + 槽（处理信号的函数）
    ```cpp
    #include <QPushButton>
    class MyWindow : public QMainWindow {
        Q_OBJECT
    public:
        MyWindow() {
            QPushButton *button = new QPushButton("Click Me", this);
            connect(button, &QPushButton::clicked, this, &MyWindow::onButtonClicked);
        }
    private slots:
        void onButtonClicked() {
            setWindowTitle("Button Clicked!");
        }
    };
    ```
   - 注意：类中需包含 Q_OBJECT 宏，触发 moc（元对象编译器）处理信号槽。

2. 常用类：

   - QWidget：所有 UI 组件的基类（如按钮、标签）。
   - QMainWindow：提供主窗口，包含菜单栏、工具栏等。
   - QPushButton、QLabel、QLineEdit：常见控件。
   - QLayout：布局管理（如 QVBoxLayout、QHBoxLayout）。

