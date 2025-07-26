/*
采用归并排序：
1 使用快慢指针递归拆分位置 next = slow->next; slow->next = nullptr; 
    sortList(ListNode* head);
2 合并两个有序链表 
    mergeList(ListNode* head1, ListNode* head2);
*/

#include <iostream>

struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution {
public:
    ListNode* sortList(ListNode* head) {
        if (!head || !head->next) return head;

        ListNode dummy(0); dummy.next = head;
        ListNode* slow = &dummy, *fast = head;
        while (fast && fast->next) {
            fast = fast->next->next;
            slow = slow->next;
        }
        ListNode* next = slow->next;
        slow->next = nullptr;

        ListNode* head1 = sortList(head);
        ListNode* head2 = sortList(next);
        

        return mergeList(head1, head2);
    }

private:
    ListNode* mergeList(ListNode* head1, ListNode* head2) {
        ListNode dummy(0); ListNode* curr = &dummy;

        while (head1 && head2) {
            if (head1->val < head2->val) {
                curr->next = head1; head1 = head1->next;
            } else {
                curr->next = head2; head2 = head2->next;
            }
            curr = curr->next;
        }
        if (head1) curr->next = head1;
        if (head2) curr->next = head2;

        return dummy.next;
    }
};

ListNode* createList(const int arr[], int n) {
    if (n == 0) return nullptr;

    ListNode dummy(0); ListNode* curr = &dummy;
    for (int i = 0; i < n; ++i) {
        curr->next = new ListNode(arr[i]);
        curr = curr->next;
    }
    return dummy.next;
}

void deleteList(ListNode* head) {
    while (head) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void printList(ListNode* head) {
    while (head) {
        std::cout << head->val;
        if (head->next) std::cout << " -> ";
        head = head->next;
    }
    std::cout << std::endl;
}

