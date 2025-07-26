/*
 ************* 二叉搜索树（BST） *************
 - 一种基于键值比较的树结构，满足：左子树键 < 根节点键 < 右子树键。
 - 支持高效查找、插入和删除。
 - 复杂度：
   - 理想情况（平衡树）：查找/插入/删除 O(log n)。
   - 最坏情况（退化为链表）：O(n)，如插入有序序列。
 - 注意：当前实现是 BST，未实现红黑树的平衡逻辑，复杂度可能退化。

 ************* 红黑树（Red-Black Tree） *************
 - 一种自平衡二叉搜索树，通过旋转和重新着色保证树高为 O(log n)。
 - 着色：
   1. 节点是红色或黑色。
   2. 根节点、叶子节点（NIL）始终黑色。
   3. 红色节点的子节点必须是黑色（无连续红色节点）。
   4. 从根到每个叶节点（NIL）的路径有相同数量的黑色节点（黑色高度）。
 - 旋转：
   1. 左旋转（右子树升高）
        x             y
       / \           / \
      a   y   --->  x   \
         / \       / \   c
        b   c     a   b
   2. 右旋转（左子树升高） 
        y             x
       / \           / \
      x   c   --->  a   y
     / \               / \
    a   b             b   c
 - 原理：
   1. 通过黑色节点数量确保最大树高：2log(n+1)，保证查找/插入/删除为 O(log n)。
   2. 着色防止树退化为一条单链表
 - 应用：STL 的 std::map 和 std::set 使用红黑树实现有序键存储。
 - 注意：当前代码仅实现 BST 插入和查找，未实现红黑树平衡（旋转/着色）。

*/

#include <memory>
#include <functional>

// 红黑树节点结构
template <typename K, typename V>
struct RBNode {
    K key;                      // 键
    V value;                    // 值
    bool is_red;                // 节点颜色（true=红色，false=黑色）
    std::unique_ptr<RBNode> left, right; // 指向左右子节点的独占指针，RAII 自动管理内存
    RBNode* parent;             // 指向父节点的普通指针，用于向上调整树（无所有权）

    // 构造函数：初始化键值对，节点默认红色（红黑树插入规则）
    RBNode(const K& k, const V& v)
        : key(k), value(v), is_red(true), left(nullptr), right(nullptr), parent(nullptr) {}
};

// 简易红黑树实现的 Map，支持键值对存储
template <typename K, typename V, typename Compare = std::less<K>>
class MyMap {
private:
    std::unique_ptr<RBNode<K, V>> root_; // 树的根节点，独占所有权
    Compare comp_;                       // 二元谓词，定义键比较规则（默认升序）
    size_t size_;                        // 存储的键值对数量

    // 递归插入节点
    // 参数：当前子树根节点、父节点指针、插入的键和值
    // 返回：更新后的子树根节点
    std::unique_ptr<RBNode<K, V>> insert_recursive(std::unique_ptr<RBNode<K, V>> node,
                                                   RBNode<K, V>* parent,
                                                   const K& key, const V& value) {
        // 递归基例：空节点，创建新节点
        if (!node) {
            auto new_node = std::make_unique<RBNode<K, V>>(key, value);
            new_node->parent = parent; // 设置父节点指针
            ++size_;                   // 增加大小计数
            return new_node;           // 返回新节点（未实现红黑树平衡）
        }

        // 按 BST 规则插入
        if (comp_(key, node->key)) {
            // 键小于当前节点，插入左子树
            node->left = insert_recursive(std::move(node->left), node.get(), key, value);
        } else if (comp_(node->key, key)) {
            // 键大于当前节点，插入右子树
            node->right = insert_recursive(std::move(node->right), node.get(), key, value);
        } else {
            // 键相等，更新值
            node->value = value;
        }
        return node; // 返回当前子树根节点
    }

    // 查找节点
    // 参数：目标键
    // 返回：找到的节点指针，或 nullptr（未找到）
    RBNode<K, V>* find_node(const K& key) const {
        RBNode<K, V>* current = root_.get();
        // 按 BST 规则查找：左子树 < 根 < 右子树
        while (current) {
            if (comp_(key, current->key)) {
                current = current->left.get(); // 键小于当前，搜索左子树
            } else if (comp_(current->key, key)) {
                current = current->right.get(); // 键大于当前，搜索右子树
            } else {
                return current; // 找到匹配键
            }
        }
        return nullptr; // 未找到
    }

public:
    // 构造函数：初始化空树
    MyMap() : root_(nullptr), size_(0) {}

    // 插入或更新键值对
    // 复杂度：O(log n)（平衡时），当前未平衡可能为 O(n)
    void insert(const K& key, const V& value) {
        root_ = insert_recursive(std::move(root_), nullptr, key, value);
        if (root_) root_->is_red = false; // 强制根节点黑色（红黑树性质2）
    }

    // 查找键对应的值
    // 返回：值指针（若存在），或 nullptr（未找到）
    // 复杂度：O(log n)（平衡时），当前未平衡可能为 O(n)
    V* find(const K& key) {
        RBNode<K, V>* node = find_node(key);
        return node ? &node->value : nullptr;
    }

    // 返回键值对数量
    size_t size() const { return size_; }
};