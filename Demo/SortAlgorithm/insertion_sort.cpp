#include <iostream>
#include <vector>

// 在升序数组索引 [low, high] 中查找插入位置
int binarySearch(const std::vector<int>& nums, int target, int low, int high) {
    // low > high 时，low 指向的数字恰好大于 target，high 指向的数字恰好小于 target
    while (low <= high) {
        int mid = low + (high-low) / 2;
        if (nums[mid] < target) low = mid + 1;
        else if (nums[mid] > target) high = mid - 1;
        else return mid;
    }
    return low; // target 应该插入 low 指向的位置，low 以后的数字依次后移
}

// 将未排序的元素插入已排序的数组
std::vector<int> insertionSort(std::vector<int>& nums) {
    int n = nums.size();

    for (int i = 1; i < n; ++i) {
        int key = nums[i];
        int sortedTail = i - 1; 
        int insertPos = binarySearch(nums, key, 0, sortedTail);
        while (sortedTail >= insertPos) {
            std::swap(nums[sortedTail], nums[sortedTail+1]); // 抱薪救火式移位
            --sortedTail;
        }
    }

    return nums;
}

int main() {
    std::vector<int> nums = {1, 3, 5, 9, 6, 2, 4, 6};
    insertionSort(nums);
    for (int x : nums) std::cout << x << " ";
    std::cout << std::endl;
    return 0;
}

