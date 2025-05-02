#include <iostream>
#include <vector>

// 快速排序：选择枢轴，分区后递归排序
// 分区：将 nums[high] 放到正确索引处 O(n)
int partition(std::vector<int>& nums, int low, int high) {
    int pivot = nums[high];

    // 遍历 [low, high) 将小于 pivot 的元素放到 i=low, i++ 索引处，最后将 i 为 pivotIndex
    int i = low;
    for (int j = low; j < high; ++j) {
        if (nums[j] <= pivot) {
            std::swap(nums[i], nums[j]);
            ++i;
        }
    }
    std::swap(nums[i], nums[high]);
    return i;
}

// 以 pivotIndex 区间递归，不稳定 O(log n)~O(n)
void quickSort(std::vector<int>& nums, int low, int high) {
    if (low >= high) return;

    int pivotIndex = partition(nums, low, high);
    quickSort(nums, low, pivotIndex-1);
    quickSort(nums, pivotIndex+1, high);
}

int main() {
    std::vector<int> nums = {1, 3, 5, 9, 6, 2, 4, 6};
    quickSort(nums, 0, nums.size()-1);
    for (int x : nums) std::cout << x << " ";
    std::cout << std::endl;
    return 0;
}

