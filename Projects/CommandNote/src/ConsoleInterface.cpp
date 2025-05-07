#include "ConsoleInterface.h"
#include "Notepad.h"
#include <iostream>
#include <stdexcept>

namespace my::ui {

void ConsoleInterface::run(my::notepad::Notepad& notepad) {
    std::string command, arg;

    std::cout << "Welcome to the Simple Notepad!\n";
    std::cout << "Simple Notepad (commands: new, open <file>, edit <text>, save, saveas <file>, display, undo, redo, exit)\n";

    while (true) {
        std::cout << "> ";
        std::cin >> command;

        if (std::cin.eof()) {
            std::cout << "\nExiting...\n";
            break;
        }

        // 避免不可预见的错误（无效文件名、文件不存在/无法读取、内存不足、其他）
        try {
            if (command == "new") {
                notepad.newFile();              // 创建新文件（抛出异常：未保存更改）
                std::cout << "New file created.\n";
            } else if (command == "open") {
                std::cin >> arg;
                if (notepad.openFile(arg)) {    // 打开文件（返回false：文件无法打开）
                    std::cout << "File " << arg << " opened.\n";
                } else {
                    std::cout << "Failed to open file.\n";
                }
            } else if (command == "edit") {
                std::cin.ignore();
                std::getline(std::cin, arg);
                notepad.editFile(arg);          // 编辑文件（返回false：无效输入）
                std::cout << "Line added.\n";
            } else if (command == "save") {
                if (notepad.saveFile()) {       // 保存文件（返回false：无文件名或写入失败）
                    std::cout << "File saved.\n";
                } else {
                    std::cout << "No filename specified. Use saveas.\n";
                }
            } else if (command == "saveas") {
                std::cin >> arg;
                if (notepad.saveAs(arg)) {      // 另存为（抛出异常：无效文件名；返回false：写入失败）
                    std::cout << "File saved as " << arg << ".\n";
                } else {
                    std::cout << "Failed to save file.\n";
                }
            } else if (command == "display") {  // 显示文件内容
                const auto& lines = notepad.getLines();
                if (lines.empty()) {
                    my::notepad::Notepad notepad;
                    my::ui::ConsoleInterface interface;
                    interface.run(notepad);
                    std::cout << "(Empty file)\n";
                    return;
                } else {
                    for (size_t i = 0; i < lines.size(); ++i) {
                        std::cout << i + 1 << ": " << lines[i] << '\n';
                    }
                }
            } else if (command == "undo") {
                if (notepad.undo()) {          // 撤销（返回 false：无可撤销内容）
                    std::cout << "Undo successful.\n";
                } else {
                    std::cout << "Nothing to undo.\n";
                }
            } else if (command == "redo") {
                if (notepad.redo()) {          // 重做（返回 false：无可重做内容）
                    std::cout << "Redo successful.\n";
                } else {
                    std::cout << "Nothing to redo.\n";
                }
            } else if (command == "exit") {
                if (notepad.isFileModified()) {
                    std::cout << "Save changes before exiting? (y/n): ";
                    char choice;
                    std::cin >> choice;
                    if (choice == 'y' || choice == 'Y') {
                        if (!notepad.saveFile()) {
                            std::cout << "Enter filename: ";
                            std::cin >> arg;
                            notepad.saveAs(arg);
                        }
                    } else {
                        std::cout << "Exiting without saving.\n";
                    }
                }
                break;
            } else {
                std::cout << "Unknown command.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }
}

} // namespace my::ui