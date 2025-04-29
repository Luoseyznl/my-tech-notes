#### 1 模版
- 
- 静态多态：**编译时**通过模板参数化和代码生成，允许同一段代码处理多种类型或行为
  - 函数模板、类模板、CRTP（Curiously Recurring Template Pattern）
    ```cpp
    template<typename Derived>
    class Base {
    public:
        void interface() {
            static_cast<Derived*>(this)->implementation();
        }
    };

    class Derived : public Base<Derived> {
    public:
        void implementation() { /* 具体实现 */ }
    };
    ```
  - 可变参数模版：允许模板接受不定数量的模板参数（类型、值或混合）
    - 参数包（模板参数包、函数参数包）、参数数量 `sizeof...(Args)`
        ```cpp
        template<typename... Args> // Args 是一个模板参数包，可以包含零个或多个类型
        struct MyTemplate {};

        template<typename... Args>
        void func(Args... args) {}  // args 是函数参数包，对应模板参数包 Args 的实例化
        ```
    - 参数包展开 
      - 递归展开：基本情况（终止递归的模板特化）+ 递归情况（处理第一个参数（head）和剩余参数（tail））
        ```cpp
        // 基本情况：终止递归
        void print() {}

        // 递归情况：处理参数包
        template<typename T, typename... Args>
        void print(const T& first, Args... args) {
            std::cout << first << " ";
            print(args...); // 递归调用，处理剩余参数
        }
        ```
      - 折叠表达式：通过一元或二元操作符直接展开参数
        ```cpp
        void print(Args... args) {
            (std::cout << ... << args); // 折叠表达式：依次对每个参数应用 << 运算符
        }
        ```
  - 模板元编程：利用模板在编译时进行计算和代码生成的一种编程范式。

#### 2 STL

- 容器
    | 容器                     | 底层数据结构                     | 说明                                                                 |
    |--------------------------|----------------------------------|----------------------------------------------------------------------|
    | `vector`                 | 动态数组                         | 连续内存，尾部增删快，随机访问高效。                                  |
    | `deque`                  | 双端队列（分块数组）             | 分块存储，支持两端快速增删，随机访问稍慢。                          |
    | `list`                   | 双向链表                         | 非连续内存，支持快速插入/删除，随机访问慢。                          |
    | `forward_list`           | 单向链表                         | 单向遍历，内存效率高，插入/删除快。                                  |
    | `array`                  | 固定大小数组                     | 静态连续内存，固定大小，随机访问高效。                              |
    | `set`/`multiset`         | 红黑树                           | 自动排序，快速查找/插入/删除（O(log n)）。           |
    | `map`/`multimap`         | 红黑树                           | 基于键排序，快速查找/插入/删除（O(log n)）。             |
    | `unordered_set`/`unordered_multiset` | 哈希表                  | 哈希存储，无序，平均 O(1) 查找/插入/删除。                          |
    | `unordered_map`/`unordered_multimap` | 哈希表                  | 键值对存储，无序，平均 O(1) 查找/插入/删除。                        |
    | `stack`                  | 底层容器（`deque`）         | 后进先出（LIFO），限制为顶部操作。                                   |
    | `queue`                  | 底层容器（`deque`）         | 先进先出（FIFO），限制为首尾操作。                                   |
    | `priority_queue`         | 堆（`vector`）          | 最大/最小堆，优先级最高元素优先访问。                                |

- 迭代器

  - 输入迭代器：常用于流（`std::istream_iterator`）	单向只读遍历，读取外部数据
  - 输出迭代器：常用于流（`std::ostream_iterator`） 单向只写遍历，写入外部数据
  - 前向迭代器：`forward_list` `unordered_set` `unordered_multiset` `unordered_map``unordered_multimap`
  - 双向迭代器：`list` `set` `multiset` `map` `multimap`
  - 随机访问迭代器：`vector` `deque` `array`

#### 3 I/O

1. 读取文本文件

    ```cpp
    std::ifstream inFile("example.txt"); // 创建一个 ifstream 对象，并打开文件
    std::string line;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {} // 逐行读取文件
        inFile.close(); // 关闭文件
    } else { std::cerr << "无法打开文件以读取" << std::endl; }
    ```

2. 写入文本文件

    ```cpp
    std::ofstream outFile("example.txt"); // 创建一个 ofstream 对象，并打开文件

    if (outFile.is_open()) {
        outFile << "Hello, world!" << std::endl;
        outFile.close(); // 关闭文件
    } else {}
    ```

3. 读取二进制文件

    ```cpp
    std::ifstream inFile("data.bin", std::ios::binary); // 以二进制模式打开文件

    if (inFile.is_open()) {
        int data[5];
        inFile.read(reinterpret_cast<char*>(data), sizeof(data)); // 以字节流读取（需给出字节数）
        inFile.close();
    } else {}
    ```

4. 写入二进制文件

    ```cpp
    std::ofstream outFile("data.bin", std::ios::binary); // 以二进制模式打开文件

    if (outFile.is_open()) {
        int data[] = {1, 2, 3, 4, 5};
        outFile.write(reinterpret_cast<char*>(data), sizeof(data)); // 以字节流写入
        outFile.close();
    } else {}
    ```

#### 4 异常处理
- `try` 代码块 `catch` 捕获并处理异常 `throw` 抛出异常
- 标准异常类 `<stdexcept>`

    |异常类 | 描述|
    |------|-----|
    |`std::runtime_error`|	运行时错误（通常由逻辑错误引起）|
    |`std::logic_error`|	逻辑错误（如无效参数）|
    |`std::invalid_argument`|	无效参数|
    |`std::out_of_range`|	超出范围（如数组越界）|
    |`std::bad_alloc`|	内存分配失败|

- 自定义异常类

    ```cpp
    class MyException : public std::exception {
    private:
        std::string message_;

    public:
        MyException(const std::string& msg) : message_(msg) {}
        
        // 重写 what() 函数，返回错误信息
        const char* what() const noexcept override {
            return message_.c_str();
        }
    };
    ```

- 异常安全：程序在抛出异常后仍能保持**一致性**（对象状态仍然有效且符合其设计约束）
- 异常安全的实现原则：RAII、Copy-and-Swap、标记 noexcept、避免半完成状态
  - 无异常安全：抛出异常后，程序状态不可预测（可能导致资源泄漏、数据损坏或未定义行为）
  - 基本异常安全（弱异常保证）：抛出异常后程序不会泄漏资源，且对象保持有效状态（内容可能改变）
  - 强异常安全（强异常保证）：操作要么完全成功，要么完全失败（**事务性**）
  - 无异常抛出保证（不抛异常）：函数保证不抛出任何异常 `noexcept`



