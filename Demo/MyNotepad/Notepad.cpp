#include "Notepad.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace my::notepad
{
    // 构造函数：初始化一个空记事本，无文件名且未修改状态
    Notepad::Notepad() : isModified(false) {}

    // 新建文件：清空当前内容，创建新文件
    void Notepad::newFile()
    {
        if (isModified)
        {
            throw std::runtime_error("存在未保存的更改，请先保存或丢弃再新建文件。");
        }
        lines.clear();
        currentFile.clear();
    }

    // 编辑文件：向内容追加一行文本
    bool Notepad::editFile(const std::string &text) {
        lines.clear(); // 清空现有内容
        if (text.empty()) {
            isModified = true;
            return true; // 空文本有效，允许清空文件
        }

        // 按行分割文本
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }
        isModified = true;
        return true;
    }

    // 保存文件：将内容保存到当前文件名
    bool Notepad::saveFile()
    {
        if (currentFile.empty()) {
            return false; // 未指定文件名，用户应使用另存为
        }

        std::ofstream outFile(currentFile);
        if (!outFile.is_open()) {
            return false; // 文件无法打开（例如权限不足、磁盘已满）
        }

        for (const auto& line : lines) {
            outFile << line << '\n'; // 逐行写入，添加换行符
        }

        outFile.close();
        isModified = false;
        return true;
    }

    // 另存为：将内容保存到指定文件名
    bool Notepad::saveAs(const std::string& fileName) {
        if (fileName.empty()) {
            throw std::invalid_argument("文件名不能为空。");
        }
        currentFile = fileName; // 设置新文件名
        return saveFile();      // 委托给 saveFile
    }

    // 打开文件：从指定文件读取内容到内存。
    bool Notepad::openFile(const std::string& fileName) {
        std::ifstream inFile(fileName); // 打开文件进行读取
        if (!inFile.is_open()) {
            return false; // 文件无法打开（例如不存在、无权限）
        }

        lines.clear();
        std::string line;
        while (std::getline(inFile, line)) {
            lines.push_back(line); // 逐行读取到 vector
        }
        inFile.close();
        currentFile = fileName;
        isModified = false;
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
}
