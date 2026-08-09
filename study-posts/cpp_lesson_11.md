# C++进阶笔记：第11课——STL算法与函数对象

> 第9、10课讲了STL容器，今天讲STL的灵魂——算法。容器管存储，算法管操作，通过迭代器搭桥，这是STL最核心的设计思想。

---

## 11.1 算法 + 迭代器 = 魔法

STL 的精髓在于分离数据结构和算法。容器管存储，算法管操作，通过迭代器搭桥。

```cpp
std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};

// 排序
std::sort(v.begin(), v.end());  // 原地排序

// 查找
auto it = std::find(v.begin(), v.end(), 5);  // 找值为5的元素

// 二分查找（必须先有序）
bool exists = std::binary_search(v.begin(), v.end(), 5);

// 去重（先排序，再去重）
auto last = std::unique(v.begin(), v.end());  // 返回新的逻辑末尾
v.erase(last, v.end());  // 把尾部重复元素删掉
```

**核心思想**：算法不 care 你是 vector、deque 还是普通数组，只要你能提供随机访问迭代器，`sort` 就能工作。

---

## 11.2 迭代器类别：算法的"能力契约"

C++ 迭代器分五级，算法根据迭代器能力选择实现：

| 类别 | 能力 | 代表容器 |
|:---|:---|:---|
| 输入 | 只读，单次遍历 | `istream_iterator` |
| 输出 | 只写，单次遍历 | `ostream_iterator` |
| 前向 | 读写，可多遍，只能 `++` | `forward_list`、`unordered_*` |
| 双向 | 前向 + 可以 `--` | `list`、`map`、`set` |
| 随机访问 | 双向 + `+n`、`-n`、`[]`、比较大小 | `vector`、`deque`、数组 |

```cpp
std::list<int> lst{3, 1, 4, 1, 5};

// std::sort(lst.begin(), lst.end());  // ❌ 编译错误！
// list 提供的是双向迭代器，sort 需要随机访问迭代器

lst.sort();  // ✅ list 自带成员函数 sort，用归并排序实现
```

**规则**：算法名和成员函数名冲突时，优先用成员函数版本（更了解容器特性）。

---

## 11.3 函数对象（仿函数）

STL 算法经常需要自定义比较逻辑。C++ 用函数对象实现——一个重载了 `operator()` 的类。

```cpp
// 判断是否为偶数
struct IsEven {
    bool operator()(int x) const {
        return x % 2 == 0;
    }
};

std::vector<int> v{1, 2, 3, 4, 5, 6};

// 统计有多少偶数
int count = std::count_if(v.begin(), v.end(), IsEven());
// 输出：3
```

`IsEven()` 创建一个临时对象，算法内部调用 `obj(x)`，编译器翻译成 `obj.operator()(x)`。

**为什么不用普通函数？**

**内联优化**：函数对象通常是 struct，编译器容易内联，零开销

**可以带状态**：

```cpp
struct GreaterThan {
    int threshold;
    GreaterThan(int t) : threshold(t) {}

    bool operator()(int x) const {
        return x > threshold;
    }
};

std::count_if(v.begin(), v.end(), GreaterThan(3));  // 统计 >3 的个数
```

普通函数做不到"带状态"，除非用全局变量（脏）。

---

## 11.4 Lambda：匿名函数对象

C++11 引入 lambda，让写函数对象像写数学公式一样自然：

```cpp
std::count_if(v.begin(), v.end(), [](int x) {
    return x > 3;
});
```

编译器会自动把这个 lambda 翻译成类似这样的函数对象：

```cpp
struct __lambda_1 {
    bool operator()(int x) const {
        return x > 3;
    }
};
```

**捕获列表**：

```cpp
int threshold = 3;

// [threshold] 按值捕获
std::count_if(v.begin(), v.end(), [threshold](int x) {
    return x > threshold;
});

// [&threshold] 按引用捕获
// [=] 按值捕获所有外部变量
// [&] 按引用捕获所有外部变量
// [&, threshold] 默认引用捕获，但 threshold 按值
```

**mutable lambda**：

```cpp
int count = 0;
std::for_each(v.begin(), v.end(), [count](int x) mutable {
    if (x > 3) ++count;  // 不加 mutable 会编译错误！
});
// count 还是 0（按值捕获，改的是 lambda 内部的副本）
```

默认 lambda 的 `operator()` 是 `const` 的，`mutable` 去掉 `const`，允许修改按值捕获的副本。

---

## 11.5 常用算法速查

| 算法 | 作用 | 复杂度 |
|:---|:---|:---|
| `std::sort` | 快速排序（内省排序） | O(n log n) |
| `std::stable_sort` | 稳定排序 | O(n log n) |
| `std::binary_search` | 二分查找 | O(log n) |
| `std::lower_bound` | 第一个 `>= x` 的位置 | O(log n) |
| `std::upper_bound` | 第一个 `> x` 的位置 | O(log n) |
| `std::find` | 线性查找 | O(n) |
| `std::count` | 统计出现次数 | O(n) |
| `std::count_if` | 按条件统计 | O(n) |
| `std::copy` | 拷贝区间 | O(n) |
| `std::transform` | 映射变换 | O(n) |
| `std::accumulate` | 累加（在 `<numeric>`） | O(n) |
| `std::max_element` | 最大元素位置 | O(n) |

```cpp
// lower_bound / upper_bound 示例
std::vector<int> v{1, 3, 3, 3, 5};

auto lo = std::lower_bound(v.begin(), v.end(), 3);  // 指向第一个 3
auto hi = std::upper_bound(v.begin(), v.end(), 3);  // 指向 5（最后一个 3 的后面）

// 等于 3 的元素个数
int cnt = hi - lo;  // 3 个
```

---

## 11.6 算法复杂度保证

C++ 标准对算法有严格的复杂度要求，这是和手写代码的重要区别：
- `std::sort`：最坏 O(n log n)（内省排序，不是普通快排）
- `std::nth_element`：平均 O(n)，找第 k 大元素
- `std::make_heap`：O(n)，建堆

这意味着你可以放心用，不用担心有人实现了个 O(n²) 的快排坑你。

---

## 本课重点

1. **算法通过迭代器操作容器**，不依赖具体容器类型
2. **迭代器分五级**，算法根据能力选择实现；不匹配会编译错误
3. **函数对象是重载 `operator()` 的类**，可内联、可带状态
4. **Lambda 是语法糖**，编译器自动生成函数对象
5. **捕获列表**控制外部变量访问方式；`mutable` 允许修改按值捕获的副本
6. **标准算法有严格复杂度保证**，放心用

---

## 课后练习与答案

### 练习题1：自定义比较器排序

```cpp
std::vector<int> v{5, 3, 1, 4, 2};

auto cmp = [](int a, int b) { return a > b; };
std::sort(v.begin(), v.end(), cmp);

// 排序后 v 是什么顺序？
```

提示：`std::sort` 的第三个参数是"严格弱序比较器"，返回 `true` 表示 `a` 应该在 `b` 前面。

### 答案

排序后 `v` 是 **降序**：`{5, 4, 3, 2, 1}`

分析：`cmp(a, b)` 返回 `a > b`，意思是"如果 `a > b`，则 `a` 应该在 `b` 前面"。这定义了一个降序关系：
- `cmp(5, 3)` → `true`，所以 5 在 3 前面
- `cmp(3, 5)` → `false`，所以 3 不在 5 前面
- `cmp(1, 1)` → `false`，满足严格弱序的自反性

**注意**：`std::sort` 的比较器语义是"a 是否应该在 b 前面"，不是"a 是否小于 b"。虽然默认用 `<` 时两者等价，但自定义比较器时要理解这个语义。

### 练习题2：迭代器类别不匹配

```cpp
std::list<int> lst{3, 1, 4, 1, 5};

auto it = std::find(lst.begin(), lst.end(), 4);
std::sort(lst.begin(), lst.end());
```

能编译吗？如果不能，为什么？

### 答案

**不能编译。`std::sort` 需要随机访问迭代器，但 `list` 只提供双向迭代器。**

具体分析：
- `std::find` 只需要**输入迭代器**（或前向迭代器），`list` 的双向迭代器满足要求，所以第一行没问题
- `std::sort` 内部需要随机访问——比如快速排序要频繁 `+n`、`-n` 跳来跳去，双向迭代器不支持这些操作
- `list` 的迭代器只能 `++` 和 `--`，不能 `it + 5` 或 `it[3]`

**正确做法**：用 `list` 自带的成员函数 `sort()`：

```cpp
lst.sort();  // list::sort 用归并排序实现，专为双向迭代器优化
```

或者先把数据拷贝到 `vector` 里排序，再拷回来（如果需要随机访问的话）。


