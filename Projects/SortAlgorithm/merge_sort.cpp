#include <iostream>
#include <vector>

// 归并排序：分治（分为 low~mid 与 mid+1~high 两个数组）

// 合并左右两个有序数组，需要额外的 temp[n]   O(n)
void merge(std::vector<int>& nums, int low, int mid, int high) {
    std::vector<int> temp(nums.size());  // 暂存 nums 方便重新排放nums的左右有序数组
    for (int i = low; i <= high; ++i) temp[i] = nums[i];

    int i = low, j = mid+1, k = low;
    while (i <= mid && j <= high) {
        if (temp[i] < temp[j]) nums[k++] = temp[i++];
        else nums[k++] = temp[j++];
    }
    while(i <= mid) nums[k++] = temp[i++];
    while(j <= high) nums[k++] = temp[j++];
}

// 以二分区间递归，速度稳定 O(log n)
void mergeSort(std::vector<int>& nums, int low, int high) {
    if (low >= high) return;    // 0或1个数字
    int mid = low + (high-low) / 2;
    mergeSort(nums, low, mid);
    mergeSort(nums, mid+1, high);
    merge(nums, low, mid, high);
}

int main() {
    std::vector<int> nums = {1, 3, 5, 9, 6, 2, 4, 6};
    mergeSort(nums, 0, nums.size()-1);
    for (int x : nums) std::cout << x << " ";
    std::cout << std::endl;
    return 0;
}

