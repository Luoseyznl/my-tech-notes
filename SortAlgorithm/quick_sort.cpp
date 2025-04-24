#include <iostream>
#include <vector>

// 快速排序：选择枢轴，分区后递归排序

int partition(std::vector<int>& nums, int low, int high) {
    int pivot = nums[high];

    // 将小于 pivot 的元素放到 low~high-1 合适的位置并返回索引
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

