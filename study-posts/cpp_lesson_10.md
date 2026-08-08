# C++进阶笔记：第10课——STL容器精讲（下）

> 第9课讲了序列容器vector、list、deque，今天讲关联容器：map、set、unordered_map，以及它们背后的数据结构。

---

## 10.1 map 与 set：有序的世界

`std::map<Key, Value>` 是有序键值对容器，底层是红黑树（一种自平衡二叉搜索树）。

```cpp
std::map<std::string, int> scores;
scores["Alice"] = 90;      // 插入或修改
scores["Bob"] = 85;
scores["Alice"] = 95;      // 修改

std::cout << scores["Alice"];  // 95
```

**特点**：
- 按键的升序自动排序
- 查找、插入、删除都是 O(log n)
- 不允许重复键

**底层结构（红黑树）**：

```
        [Bob:85]
       /        [Alice:95]   [Charlie:70]
```

每个节点保存键、值、颜色（红/黑），通过旋转保持平衡，保证树高是 O(log n)。

`std::set<T>` 就是只有键、没有值的 map：

```cpp
std::set<int> s;
s.insert(3);
s.insert(1);
s.insert(2);

for (int x : s) {
    std::cout << x << " ";  // 输出：1 2 3（自动排序）
}
```

---

## 10.2 自定义比较器

默认按 `<` 升序排。如果想降序，或按自定义规则：

```cpp
// 降序
std::map<int, std::string, std::greater<int>> desc;

// 自定义：按字符串长度排序
struct LengthCompare {
    bool operator()(const std::string& a, const std::string& b) const {
        return a.length() < b.length();
    }
};

std::set<std::string, LengthCompare> words;
words.insert("apple");   // 长度 5
words.insert("pie");     // 长度 3
words.insert("banana");  // 长度 6

// 遍历顺序：pie(3) → apple(5) → banana(6)
```

**注意**：比较器必须是**严格弱序**：
- 如果 `a < b` 为真，则 `b < a` 必须为假
- 如果 `a < b` 且 `b < c`，则 `a < c`
- `a < a` 必须为假

违反这些会导致红黑树结构破坏，程序崩溃或无限循环。

---

## 10.3 unordered_map：哈希表的暴力美学

`std::unordered_map` 底层是哈希表，不是树。

```cpp
std::unordered_map<std::string, int> scores;
scores["Alice"] = 90;
scores["Bob"] = 85;
```

| | map | unordered_map |
|:---|:---|:---|
| 底层 | 红黑树 | 哈希表 |
| 查找/插入/删除 | O(log n) | 平均 O(1)，最坏 O(n) |
| 有序性 | ✅ 按键排序 | ❌ 无序 |
| 内存占用 | 较小（只需节点指针） | 较大（需要维护桶数组） |
| 自定义比较 | 比较器 | 哈希函数 + 相等判断 |

**什么时候用哪个？**
- 需要有序遍历或范围查询（找比 x 大的最小键）→ `map`
- 只关心快速查找，不需要顺序 → `unordered_map`

---

## 10.4 自定义哈希

对于自定义类型，需要告诉 `unordered_map` 怎么算哈希：

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& other) const {  // 还需要相等判断
        return x == other.x && y == other.y;
    }
};

// 哈希函数
struct PointHash {
    size_t operator()(const Point& p) const {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
};

std::unordered_map<Point, std::string, PointHash> grid;
grid[{1, 2}] = "hello";
```

`std::hash<int>()` 是标准库提供的整数哈希函数。上面的写法把 x 和 y 的哈希值异或混合在一起。

**更好的混合方式**（减少冲突）：

```cpp
return std::hash<int>()(p.x) * 73856093 ^ std::hash<int>()(p.y) * 19349663;
```

用大质数乘后再异或，分布更均匀。

---

## 10.5 [] 运算符的陷阱

```cpp
std::map<std::string, int> m;
std::cout << m["nonexistent"];  // 输出 0

// 发生了什么？
// m["nonexistent"] 发现键不存在，**自动插入一个默认值**！
```

`operator[]` 的语义是：
1. 查找键
2. 如果存在，返回引用
3. 如果不存在，**插入一个默认构造的值**，再返回引用

如果你只是想查一下存不存在，用 `find`：

```cpp
auto it = m.find("nonexistent");
if (it != m.end()) {
    std::cout << it->second;  // 找到了
} else {
    std::cout << "不存在";     // 没找到，也不会插入
}
```

**只读查询时，用 `find` 或 `count`，别用 `[]`。**

---

## 10.6 迭代器稳定性

| 容器 | 插入后 | 删除后 |
|:---|:---|:---|
| map / set | 已有迭代器不失效 | 只有指向被删节点的迭代器失效 |
| unordered_map | 可能全部失效（rehash 时） | 同桶及后续可能失效 |

```cpp
std::map<int, std::string> m;
auto it = m.insert({1, "a"}).first;  // 指向新插入的元素

m.insert({2, "b"});  // it 仍然有效！
m.erase(1);           // it 失效了，因为 1 被删了
```

---

## 本课重点

1. **map / set**：红黑树，有序，O(log n)
2. **unordered_map**：哈希表，平均 O(1)，无序，需自定义哈希
3. **自定义比较器**：必须满足严格弱序
4. **`operator[]` 会插入默认值**：只读查询用 `find` 或 `count`
5. **迭代器稳定性**：map 插入稳定，删除局部失效；unordered_map rehash 时全失效

---

## 课后练习与答案

### 练习题1：map的迭代器稳定性

```cpp
std::map<int, std::string> m;
m[5] = "five";

auto it = m.find(5);
m[3] = "three";   // 这行会影响 it 吗？
m.erase(5);       // 这行之后 it 还能用吗？
```

提示：map 的节点是独立分配的，插入不移动已有节点。

### 答案

**`m[3] = "three"` 不影响 `it`。**

原因：map 底层是红黑树，每个节点独立分配在堆上。插入新节点时，只需要调整树中少量指针关系，**不会移动或重新分配已有节点**。所以 `it` 指向的内存位置不变，仍然有效。

**`m.erase(5)` 之后 `it` 失效。**

原因：`erase(5)` 删除了键为 5 的节点，释放了该节点的内存。`it` 原来指向的就是这个节点，现在变成了悬空指针（dangling iterator）。后续使用 `it` 是未定义行为。

**正确做法**：

```cpp
auto it = m.find(5);
if (it != m.end()) {
    // 先用 it，再 erase
    std::cout << it->second << std::endl;
    m.erase(it++);  // 先让 it 指向下一个，再删除当前
    // 或者：it = m.erase(it);  // C++11起，map::erase 返回下一个迭代器
}
```

### 练习题2：unordered_map的erase

```cpp
std::unordered_map<std::string, int> freq;
std::string word;
while (std::cin >> word) {
    freq[word]++;   // 统计词频
}

for (auto it = freq.begin(); it != freq.end(); ) {
    if (it->second < 2) {
        freq.erase(it++);   // 注意这里
    } else {
        ++it;
    }
}
```

`unordered_map` 的 `erase` 返回什么？这种写法安全吗？

### 答案

**C++11 之前**：`unordered_map::erase` 返回 `void`，不返回迭代器。`freq.erase(it++)` 这种写法依赖的是**后置递增的求值顺序保证**：

```cpp
freq.erase(it++);
// 等价于：
// freq.erase(it);   // 先传 it 的当前值给 erase
// ++it;             // 然后 it 自增，指向下一个
```

这里的关键是：`it++` 返回的是**自增前的旧值**（一个副本），`erase` 删除的是这个旧值指向的元素，而 `it` 本身已经指向下一个了。所以是安全的。

**C++11 起**：`unordered_map::erase` 返回**被删除元素之后的迭代器**。所以更安全的写法是：

```cpp
for (auto it = freq.begin(); it != freq.end(); ) {
    if (it->second < 2) {
        it = freq.erase(it);   // C++11 起，直接返回下一个
    } else {
        ++it;
    }
}
```

**注意**：`unordered_map` 的迭代器失效规则比 `map` 更复杂。删除一个元素时，只有指向被删元素的迭代器失效，其他迭代器仍然有效。但如果触发了 rehash（负载因子超过阈值），**所有迭代器全部失效**。


