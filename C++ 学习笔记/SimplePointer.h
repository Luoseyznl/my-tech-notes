#include <utility>
#include <cstddef>

/*
 SimpleUniquePtr 独占指针
 目标功能：
    1 类内部有一个指针可以管理动态分配的对象（用 new 分配的内存）
    2 析构时自动释放资源
    3 禁用拷贝构造、拷贝赋值（只有一个指针独占式拥有对象）
    4 支持移动构造、移动赋值
    5 支持指针语义（解引用* 成员访问->）
*/

template<typename T>
class SimpleUniquePtr {
public:
    explicit SimpleUniquePtr(T* ptr = nullptr) : ptr(ptr) {}    // 1 构造函数禁止隐式转换

    // 2 禁用：拷贝构造、拷贝赋值
    SimpleUniquePtr(const SimpleUniquePtr&) = delete;
    SimpleUniquePtr& operator=(const SimpleUniquePtr&) = delete;

    // 3 开启移动构造、移动赋值以转移所有权（noexcept）
    SimpleUniquePtr(SimpleUniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    SimpleUniquePtr& operator=(SimpleUniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    ~SimpleUniquePtr() {
        delete ptr;
    }

    // 4 指针语义
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }

    // 5 获取原始指针、释放指针所有权、重置指针
    T* get() const { return ptr; }
    T* release() noexcept {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    void reset(T* new_ptr = nullptr) noexcept {
        if (ptr != new_ptr) {
            delete ptr;
            ptr = new_ptr;
        }
    }

private:
    T* ptr;
};

/*
  ControlBlock：管理引用计数、动态内存指针
  SimpleSharedPtr 共享指针
  目标功能：
    1 可以管理动态分配的对象（用 new 分配的内存）
    2 引用计数（共享计数、弱计数）追踪资源使用
    3 析构时减少引用计数，最终会释放内存
    4 支持拷贝构造、拷贝赋值、移动构造、移动赋值
    5 支持指针语义（解引用* 成员访问->）
  SimpleWeakPtr 弱指针
  目标功能：
    1 不拥有资源，仅观察对象
*/

template<typename T>
class SimpleSharedPtr;

template<typename T>
class SimpleWeakPtr;

// 控制块，管理引用计数和资源
class ControlBlock {
public:
    ControlBlock(void* ptr) : shared_count(1), weak_count(0), raw_ptr(ptr) {}
    
    void increment_shared() { ++shared_count; }
    void decrement_shared() {
        if (--shared_count == 0) {  // 共享计数为0时释放动态内存
            delete static_cast<void*>(raw_ptr);
            raw_ptr = nullptr;
            if (weak_count == 0) {  // 弱计数为0时释放控制块
                delete this;
            }
        }
    }
    
    void increment_weak() { ++weak_count; }
    void decrement_weak() {
        if (--weak_count == 0 && shared_count == 0) {
            delete this;
        }
    }
    
    size_t get_shared_count() const { return shared_count; }
    void* get_raw_ptr() const { return raw_ptr; }

private:
    size_t shared_count;
    size_t weak_count;
    void* raw_ptr;
};

// 共享指针
template<typename T>
class SimpleSharedPtr {
public:
    // 1 显式构造函数，实例化用于管理动态内存的控制块
    explicit SimpleSharedPtr(T* ptr = nullptr) : ptr(ptr), control(ptr ? new ControlBlock(ptr) : nullptr) {}
    
    // 2 开启拷贝构造、拷贝赋值（增加共享计数）
    SimpleSharedPtr(const SimpleSharedPtr& other) : ptr(other.ptr), control(other.control) {
        if (control) {
            control->increment_shared();
        }
    }
    SimpleSharedPtr& operator=(const SimpleSharedPtr& other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            control = other.control;
            if (control) {
                control->increment_shared();
            }
        }
        return *this;
    }
    
    // 3 开启移动构造、移动赋值
    SimpleSharedPtr(SimpleSharedPtr&& other) noexcept : ptr(other.ptr), control(other.control) {
        other.ptr = nullptr;
        other.control = nullptr;
    }
    SimpleSharedPtr& operator=(SimpleSharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            control = other.control;
            other.ptr = nullptr;
            other.control = nullptr;
        }
        return *this;
    }

    ~SimpleSharedPtr() {
        release();
    }
    
    // 4 指针语义
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    
    // 5 获取原始指针、获取引用计数、转换为弱指针
    T* get() const { return ptr; }

    size_t use_count() const { return control ? control->get_shared_count() : 0; }

    SimpleWeakPtr<T> to_weak() const;

private:
    T* ptr;
    ControlBlock* control;
    
    void release() {
        if (control) {
            control->decrement_shared();
            ptr = nullptr;
            control = nullptr;
        }
    }
    
    friend class SimpleWeakPtr<T>;
};

// 定义 to_weak
template<typename T>
SimpleWeakPtr<T> SimpleSharedPtr<T>::to_weak() const {
    return SimpleWeakPtr<T>(*this);
}

// 弱指针
template<typename T>
class SimpleWeakPtr {
public:
    // 1 构造函数（默认，不可从普通指针转换）
    SimpleWeakPtr() : ptr(nullptr), control(nullptr) {}
    
    // 2 从共享指针构造、拷贝构造、拷贝赋值、移动构造、移动赋值
    SimpleWeakPtr(const SimpleSharedPtr<T>& shared) : ptr(shared.ptr), control(shared.control) {
        if (control) {
            control->increment_weak();
        }
    }
    SimpleWeakPtr(const SimpleWeakPtr& other) : ptr(other.ptr), control(other.control) {
        if (control) {
            control->increment_weak();
        }
    }
    SimpleWeakPtr& operator=(const SimpleWeakPtr& other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            control = other.control;
            if (control) {
                control->increment_weak();
            }
        }
        return *this;
    }
    SimpleWeakPtr(SimpleWeakPtr&& other) noexcept : ptr(other.ptr), control(other.control) {
        other.ptr = nullptr;
        other.control = nullptr;
    }
    SimpleWeakPtr& operator=(SimpleWeakPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            control = other.control;
            other.ptr = nullptr;
            other.control = nullptr;
        }
        return *this;
    }
    
    ~SimpleWeakPtr() {
        release();
    }
    
    // 检查是否过期
    bool expired() const { return control == nullptr || control->get_shared_count() == 0; }
    
    // 转换为共享指针
    SimpleSharedPtr<T> lock() const {
        if (expired()) {
            return SimpleSharedPtr<T>();
        }
        SimpleSharedPtr<T> result;
        result.ptr = ptr;
        result.control = control;
        control->increment_shared();
        return result;
    }

private:
    T* ptr;
    ControlBlock* control;
    
    void release() {
        if (control) {
            control->decrement_weak();
            ptr = nullptr;
            control = nullptr;
        }
    }
    
    friend class SimpleSharedPtr<T>;
};

