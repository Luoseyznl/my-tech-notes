### 控制台记事本

- 使用控制台界面，用户通过命令交互

1. 基本功能：
   - 新建文件：清空当前内容，重置文件名和状态。
   - 编辑文件：通过 editFile 添加新行，每次编辑保存状态到撤销栈。
   - 保存文件：将内容写入 currentFile ，若无文件名则提示输入。
   - 另存为：允许用户指定新文件名并保存。

```
notepad/
├── include/
│   ├── Notepad.h         // 核心记事本逻辑
│   ├── ConsoleInterface.h // 命令行界面
│   └── INotepadInterface.h // 抽象界面接口（为Qt准备）
├── src/
│   ├── Notepad.cpp
│   ├── ConsoleInterface.cpp
│   └── main.cpp
├── CMakeLists.txt        // CMake配置文件
└── README.md             // 项目说明
```

