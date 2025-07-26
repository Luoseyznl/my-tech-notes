#ifndef NOTEPAD_H
#define NOTEPAD_H

/*
 ************************************ 记事本核心操作 ************************************
 新建文件、编辑文件、保存文件、另存为、打开文件、获取内容、检查是否修改、获取文件名
 ***************************************************************************************

 异常情况：
 1. 程序无法恢复（错误超出了函数的控制范围）
 2. 跨层错误传递（错误需要跳过多层调用栈，直接传递到高层处理）
 3. 开发错误（错误使用 API）

 返回 false
 1. 常见操作失败（文件不存在、用户输入无效）
 2. 性能敏感场景（避免异常的开销）
*/

#include <string>
#include <vector>
#include <stack>
#include <deque>

// 使用命名空间 my::notepad 组织代码
namespace my::notepad
{

    class Notepad
    {
    private:
        std::vector<std::string> lines; // 内容
        std::string currentFile;        // 当前文件名
        bool isModified;                // 已修改标志
        const size_t MAX_HISTORY = 50;  // 最多保存 50 步历史

        std::deque<std::vector<std::string>> undoHistory;   // 撤销队列（提供容量限制和高效的插入/删除操作）
        std::deque<std::vector<std::string>> redoHistory;   // 重做队列
        bool isSameAsSavedFile() const;                   // 检查当前内容是否与保存的文件相同

    public:
        Notepad();
        void newFile();                                   // 创建文件（抛出异常：未保存更改）
        bool editFile(const std::string &text);           // 编辑文件（返回false：无效输入）
        bool saveFile();                                  // 保存文件（返回false：无文件名或写入失败）
        bool saveAs(const std::string &fileName);         // 另存为  （抛出异常：无效文件名；返回false：写入失败）
        bool openFile(const std::string &filename);       // 打开文件（返回false：文件无法打开）
        const std::vector<std::string> &getLines() const; // 获取内容
        bool isFileModified() const;                      // 检查是否修改
        std::string getCurrentFile() const;               // 获取文件名
        
        bool undo(); // 撤销最近一次 editFile 操作（返回 false：无可撤销内容）
        bool redo(); // 重做最近一次被撤销的 editFile 操作（返回 false：无可重做内容）
    };

} // namespace my::notepad

#endif // NOTEPAD_H