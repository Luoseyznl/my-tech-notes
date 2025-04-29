#include <iostream>
#include <string>
#include "SimpleMap.h"

// 测试 MyMap 的功能
int main() {
    // 创建 MyMap 实例，使用默认 std::less<int>（升序）
    MyMap<int, std::string> map;

    // 测试插入
    map.insert(3, "three");
    map.insert(1, "one");
    map.insert(4, "four");
    map.insert(1, "one_updated"); // 更新键 1 的值
    map.insert(2, "two");

    // 测试大小
    std::cout << "Map size: " << map.size() << std::endl; // 应输出: 4

    // 测试查找
    std::cout << "Find results:\n";
    for (int key : {1, 2, 3, 4, 5}) {
        if (auto* value = map.find(key)) {
            std::cout << "Key " << key << ": " << *value << std::endl;
        } else {
            std::cout << "Key " << key << ": Not found" << std::endl;
        }
    }

    // 测试降序排序（使用 std::greater<int>）
    MyMap<int, std::string, std::greater<int>> descending_map;
    descending_map.insert(3, "three");
    descending_map.insert(1, "one");
    descending_map.insert(4, "four");
    std::cout << "\nDescending map size: " << descending_map.size() << std::endl;
    for (int key : {1, 3, 4}) {
        if (auto* value = descending_map.find(key)) {
            std::cout << "Key " << key << ": " << *value << std::endl;
        }
    }

    return 0;
}