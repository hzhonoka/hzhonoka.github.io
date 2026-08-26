# 示例题解：两数之和

> 这是「题目」板块的示例文章。你可以把它删掉，换上自己的题目或题解。

## 题目描述

给定一个整数数组 `nums` 和一个整数目标值 `target`，请你在该数组中找出**和为目标值**的那两个整数，并返回它们的数组下标。

你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。

## 示例

```
输入：nums = [2, 7, 11, 15], target = 9
输出：[0, 1]
解释：因为 nums[0] + nums[1] == 9，返回 [0, 1]。
```

## 思路

暴力枚举是 $O(n^2)$，但用哈希表可以把时间复杂度降到 $O(n)$：遍历数组时，检查 `target - nums[i]` 是否已经出现在哈希表里。

## 参考代码（C++）

```cpp
vector<int> twoSum(vector<int>& nums, int target) {
    unordered_map<int, int> mp;
    for (int i = 0; i < nums.size(); ++i) {
        int need = target - nums[i];
        if (mp.count(need)) return {mp[need], i};
        mp[nums[i]] = i;
    }
    return {};
}
```

## 复杂度

- 时间复杂度：$O(n)$
- 空间复杂度：$O(n)$
