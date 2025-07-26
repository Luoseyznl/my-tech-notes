#include <iostream>

template<typename T>
class SimpleVector {
private:
    T* data;
    size_t capacity;
    size_t size;

    // 辅助函数，动态扩容
    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (size_t i = 0; i < size; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    SimpleVector() : data(new T[1]), capacity(1), size(0) {}
    ~SimpleVector() {
        delete[] data;
    }

    void push_back(const T& value) {
        // 动态扩容
        if (size == capacity) {
            resize();
        }
        data[size++] = value;
    }

    // 返回原对象引用，可用于左值赋值
    T& operator[] (size_t index) {
        // 先处理异常
        if (index >= size) 
            throw std::out_of_range("Index out of range");

        return data[index];
    }

    // 将获取 size 的函数设置为 const，确保 const 对象也可以调用，且符合逻辑
    size_t getSize() const {
        return size;
    }
};
