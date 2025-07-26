#include <iostream>
#include <vector>

// 共 n-1 轮，第 i 轮将 0~n-i-1 的最大值放到 n-i 处
std::vector<int> bubbleSort(std::vector<int>& nums) {
    int n = nums.size();
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < n-i; ++j) {
            if (nums[j] > nums[j+1]) std::swap(nums[j], nums[j+1]);
        }
    }
    return nums;
} 

int main() {
    std::vector<int> nums = {1, 3, 5, 2, 4, 6};
    bubbleSort(nums);
    for (int x : nums) std::cout << x << " ";
    std::cout << std::endl;
    return 0;
}
