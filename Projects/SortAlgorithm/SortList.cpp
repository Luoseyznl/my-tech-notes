#include "SortList.h"


int main() {
    int arr1[] = {4, 2, 1, 3};
    ListNode* list1 = createList(arr1, 4);
    std::cout << "Original list: ";
    printList(list1);

    Solution solution;
    list1 = solution.sortList(list1);
    std::cout << "Sorted list: ";
    printList(list1);
    deleteList(list1);

    // 测试用例 2: 已排序链表
    int arr2[] = {1, 2, 3, 4};
    ListNode* list2 = createList(arr2, 4);
    std::cout << "\nOriginal list: ";
    printList(list2);

    list2 = solution.sortList(list2);
    std::cout << "Sorted list: ";
    printList(list2);
    deleteList(list2);

    // 测试用例 3: 空链表
    ListNode* list3 = nullptr;
    std::cout << "\nOriginal list: (empty)\n";
    list3 = solution.sortList(list3);
    std::cout << "Sorted list: ";
    printList(list3);

    // 测试用例 4: 单节点链表
    int arr4[] = {1};
    ListNode* list4 = createList(arr4, 1);
    std::cout << "\nOriginal list: ";
    printList(list4);

    list4 = solution.sortList(list4);
    std::cout << "Sorted list: ";
    printList(list4);
    deleteList(list4);

    // 测试用例 5: 包含重复元素的链表
    int arr5[] = {3, 1, 4, 1, 2};
    ListNode* list5 = createList(arr5, 5);
    std::cout << "\nOriginal list: ";
    printList(list5);

    list5 = solution.sortList(list5);
    std::cout << "Sorted list: ";
    printList(list5);
    deleteList(list5);

    return 0;
}
