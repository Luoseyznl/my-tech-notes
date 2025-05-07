#include "Notepad.h"
#include <fstream>
#include <stdexcept>

namespace my::notepad
{
    // 构造函数：初始化一个空记事本，无文件名且未修改状态
    Notepad::Notepad() : isModified(false) {}

    // 新建文件：清空当前内容，创建新文件
    void Notepad::newFile()
    {
        if (isModified) {
            throw std::runtime_error("存在未保存的更改，请先保存或丢弃再新建文件。");
        }
        lines.clear();
        currentFile.clear();
        undoHistory.clear();
        redoHistory.clear();
        isModified = false;
    }

    // 编辑文件：向内容追加一行文本
    bool Notepad::editFile(const std::string &text)
    {
        if (text.empty()) return false;

        undoHistory.push_back(lines);     // 保存当前状态
        redoHistory.clear();              // 编辑后清空 redo 历史
        lines.push_back(text);
        isModified = true;
        return true;
    }

    // 保存文件：将内容保存到当前文件名
    bool Notepad::saveFile()
    {
        if (currentFile.empty()) return false;

        std::ofstream outFile(currentFile);
        if (!outFile.is_open()) return false;

        for (const auto& line : lines) {
            outFile << line << '\n';
        }

        outFile.close();
        isModified = false;
        return true;
    }

    // 另存为：将内容保存到指定文件名
    bool Notepad::saveAs(const std::string& fileName)
    {
        if (fileName.empty()) {
            throw std::invalid_argument("文件名不能为空。");
        }
        currentFile = fileName;
        return saveFile();  // 委托给 saveFile();
    }

    // 打开文件：从指定文件读取内容到内存。
    bool Notepad::openFile(const std::string& fileName)
    {
        std::ifstream inFile(fileName);
        if (!inFile.is_open()) return false;  // 文件无法打开

        lines.clear();
        std::string line;
        while (std::getline(inFile, line)) {  // 逐行读取文件内容
            lines.push_back(line);
        }
        inFile.close();

        currentFile = fileName;
        isModified = false;
        undoHistory.clear();
        redoHistory.clear();
        return true;
    }

    bool Notepad::undo() {
        if (undoHistory.empty()) {
            return false;                   // 无可撤销内容
        }
        redoHistory.push_back(lines);       // 将当前内容保存到重做队列
        lines = undoHistory.back();         // 恢复上一个状态
        undoHistory.pop_back();             // 移除已撤销的状态
        isModified = !isSameAsSavedFile();  // 是否与保存的文件相同
        return true;
    }

    bool Notepad::redo() {
        if (redoHistory.empty()) {
            return false;                   // 无可重做内容
        }
        undoHistory.push_back(lines);       // 将当前内容保存到撤销队列
        lines = redoHistory.back();         // 恢复上一个状态
        redoHistory.pop_back();             // 移除已重做的状态
        isModified = !isSameAsSavedFile();  // 是否与保存的文件相同
        return true;
    }

    // 获取当前内容：返回文本行向量。
    const std::vector<std::string>& Notepad::getLines() const {
        return lines;
    }

    // 检查是否修改：判断内容是否有未保存的更改。
    bool Notepad::isFileModified() const {
        return isModified;
    }

    // 获取当前文件名。
    std::string Notepad::getCurrentFile() const {
        return currentFile;
    }

    // 检查当前内容是否与保存的文件相同
    bool Notepad::isSameAsSavedFile() const
    {
        if (currentFile.empty()) return true; // 没有保存过文件视为一致

        std::ifstream inFile(currentFile);
        if (!inFile.is_open()) return false;

        std::string fileLine;
        size_t i = 0;
        while (std::getline(inFile, fileLine)) {
            if (i >= lines.size() || lines[i] != fileLine) {
                return false;
            }
            ++i;
        }
        return i == lines.size(); // 行数也必须一致
    }
} // namespace my::notepad
