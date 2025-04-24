#include <iostream>
#include <vector>

// 在升序索引中查找插入位置
int binarySearch(const std::vector<int>& nums, int target, int low, int high) {
    while (low <= high) {
        int mid = low + (high-low) / 2;
        if (nums[mid] < target) low = mid + 1;
        else if (nums[mid] > target) high = mid - 1;
        else return mid;
    }
    return low;
}

// 将未排序的元素插入已排序的数组
std::vector<int> insertionSort(std::vector<int>& nums) {
    int n = nums.size();

    for (int i = 1; i < n; ++i) {
        int key = nums[i];
        int sortedTail = i - 1; 
        int insertPos = binarySearch(nums, key, 0, sortedTail);
        while (sortedTail >= insertPos) {
            std::swap(nums[sortedTail], nums[sortedTail+1]);
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

