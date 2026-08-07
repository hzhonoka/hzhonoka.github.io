# C++进阶笔记：第9课——STL容器精讲（上）

> 前面八课讲了C++的对象模型、生命周期、资源管理和运算符重载。从今天开始进入STL系列，先讲最常用的三种序列容器：vector、list、deque。

---

## 9.1 vector：会自己长大的数组

`std::vector` 是 C++ 最常用的容器。你可以把它理解为一个会自动扩容的数组。

```cpp
std::vector<int> v;
v.push_back(1);
v.push_back(2);
v.push_back(3);

std::cout << v[0];      // O(1) 随机访问，和数组一样快
std::cout << v.size();  // 3，当前有多少个元素
std::cout << v.capacity(); // 可能 >=3，当前能容纳多少元素而不扩容
```

**底层结构**：一块连续的堆内存，三个指针（或指针+大小）管理：

```
[v.begin()] [1] [2] [3] [ ] [ ] [ ] [ ]
 ↑          ↑           ↑
begin()   元素区域    end()      capacity() 的边界
```

**扩容机制**：

当 `size == capacity` 时再 `push_back`，vector 会：
1. 申请一块更大的新内存（通常是2倍）
2. 把旧元素搬过去（拷贝或移动）
3. 释放旧内存
4. 插入新元素

```cpp
std::vector<int> v;
v.reserve(100);     // 预先申请100个位置，避免多次扩容
v.push_back(1);     // 不触发扩容
```

**迭代器失效（重点！）**：

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
auto it = v.begin() + 2;   // 指向 3

v.push_back(6);            // 可能触发扩容！

// 此时 it 已经失效！指向被释放的旧内存！
std::cout << *it;          // 未定义行为！可能崩溃，可能读到垃圾值
```

**vector 扩容后，所有迭代器、指针、引用全部失效。**

安全写法：

```cpp
auto it = v.begin();
v.push_back(6);
it = v.begin() + 2;   // 重新获取迭代器
```

---

## 9.2 list：松散的双向链表

`std::list` 的每个元素存在独立的节点里，节点之间用指针串起来。

```
[head] ↔ [1] ↔ [2] ↔ [3] ↔ [tail]
          ↑     ↑
       迭代器指向节点本身
```

**特点**：
- 不连续存储，不支持 `v[i]` 这种随机访问（要访问第 i 个，只能从头走 i 步）
- 插入删除是 O(1)，且不移动其他元素
- **迭代器稳定**：插入新节点不会影响已有迭代器；删除节点时，只有指向被删节点的迭代器失效

```cpp
std::list<int> lst{1, 2, 3};
auto it = ++lst.begin();   // 指向 2

lst.push_back(4);          // it 仍然有效！
lst.push_front(0);         // it 仍然有效！

lst.erase(it);             // 删除 2，此时 it 失效
// 但指向 1 和 3 的迭代器仍然有效
```

**什么时候用 list？**

当你需要频繁在中间插入/删除，且不在乎随机访问速度时。比如实现一个 LRU 缓存、任务队列。

---

## 9.3 deque：双端队列

`std::deque`（发音 deck）是"double-ended queue"的缩写。

它像是 vector 和 list 的折中：

| | vector | deque | list |
|:---|:---|:---|:---|
| 随机访问 | O(1) ✅ | O(1) ✅ | O(n) ❌ |
| 尾部插入 | 均摊 O(1)，但扩容时全搬 | O(1) ✅ | O(1) ✅ |
| 头部插入 | O(n) ❌ | O(1) ✅ | O(1) ✅ |
| 内存连续性 | 完全连续 | 分段连续 | 不连续 |
| 迭代器稳定性 | 扩容全失效 | 中间操作可能失效 | 插入稳定，删除局部失效 |

**底层结构**：

deque 不是一块连续内存，而是多块小数组（buffer），由一个中央映射表（map）管理：

```
中央映射表：[ptr1] [ptr2] [ptr3] [ptr4]
              ↓      ↓      ↓      ↓
           [数组1][数组2][数组3][数组4]
```

当头部需要空间时，在前面加一块新数组；尾部需要空间时，在后面加一块。不需要像 vector 那样把全部元素搬一次。

```cpp
std::deque<int> d;
d.push_back(1);     // 尾部插入，快
d.push_front(0);    // 头部插入，也快！vector 做不到
```

**迭代器失效**：

deque 的迭代器失效规则比 vector 复杂：
- 在中间插入/删除：所有迭代器可能失效
- 在头尾插入：通常不会使已有迭代器失效（但 C++ 标准不严格保证，具体看实现）

**保守原则**：对 deque 做任何修改后，都重新获取迭代器。

---

## 9.4 emplace：原地构造，省一次搬运

这是 C++11 引入的优化。先看传统写法的问题：

```cpp
struct Point {
    int x, y;
    Point(int a, int b) : x(a), y(b) {
        std::cout << "构造\n";
    }
    Point(const Point&) {
        std::cout << "拷贝\n";
    }
    Point(Point&&) {
        std::cout << "移动\n";
    }
};

std::vector<Point> v;
v.push_back(Point(1, 2));   // 先构造临时对象，再移动进 vector
// 输出：构造 → 移动
```

`push_back` 接收一个已经构造好的对象，然后把它拷贝或移动进容器。

`emplace_back` 则不同：

```cpp
v.emplace_back(1, 2);   // 直接在 vector 的内存里构造 Point
// 输出：构造
```

**区别**：

| | push_back | emplace_back |
|:---|:---|:---|
| 参数 | 对象本身 | 构造函数的参数 |
| 过程 | 先构造临时对象 → 再拷贝/移动进容器 | 直接在容器内存位置构造 |
| 开销 | 一次构造 + 一次拷贝/移动 | 只一次构造 |

```cpp
std::vector<std::pair<int, std::string>> v;

// push_back：要先构造 pair
v.push_back(std::make_pair(1, "hello"));

// emplace_back：直接传构造参数
v.emplace_back(1, "hello");   // 更简洁，更高效
```

**现代 C++ 推荐**：能用 `emplace_back` 就别用 `push_back`。

**注意**：`emplace` 不是万能的。如果对象已经构造好了（比如一个局部变量），那 `push_back` 和 `emplace_back` 没区别，甚至 `push_back` 更直观。

---

## 本课重点

1. **vector**：连续内存，随机访问 O(1)，尾部插入均摊 O(1)，扩容时所有迭代器失效
2. **list**：双向链表，插入删除 O(1) 且稳定，不支持随机访问
3. **deque**：分段连续，头尾插入都 O(1)，中间操作迭代器易失效
4. **emplace_back**：原地构造，省一次拷贝/移动，参数是构造函数的参数而非对象

---

## 课后练习与答案

### 练习题1：迭代器失效

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

for (auto it = v.begin(); it != v.end(); ) {
    if (*it % 2 == 0) {
        it = v.erase(it);   // 注意这里
    } else {
        ++it;
    }
}
```

vector 的 `erase` 返回什么？为什么必须写 `it = v.erase(it)` 而不是直接 `v.erase(it)`？

### 答案

`vector::erase(it)` 返回**被删除元素之后的迭代器**（即下一个有效元素的位置）。

**为什么不能直接 `v.erase(it)` 然后 `++it`？**

因为 `erase` 后，被删除位置及之后的迭代器全部失效。如果写：

```cpp
v.erase(it);   // it 已经失效！
++it;          // 未定义行为！
```

`it` 指向的内存可能已经被移动或释放，`++it` 是在操作一个失效迭代器。

**正确写法** `it = v.erase(it)` 的含义：
- `erase(it)` 删除当前元素，返回下一个有效元素的迭代器
- 把这个新迭代器重新赋给 `it`
- 循环继续时，`it` 指向的是下一个待检查的元素，不需要再 `++it`

**最终效果**：遍历 vector，删除所有偶数。

### 练习题2：容器选择

下面三个场景，分别最适合用 vector、list 还是 deque？

1. 需要存储 100 万个坐标点，频繁按索引访问，偶尔在末尾追加新点。
2. 实现一个文本编辑器的行列表，需要在中间频繁插入和删除行。
3. 实现一个任务队列，任务不断从头部取出，新任务不断从尾部加入。

### 答案

**场景1：vector**

- 频繁按索引访问 → 需要 O(1) 随机访问，vector 最优
- 偶尔在末尾追加 → `push_back` 均摊 O(1)，vector 完全胜任
- 100万个点连续存储，缓存友好，性能最好

**场景2：list**

- 频繁在中间插入删除 → list 的 `insert`/`erase` 是 O(1)，且不影响其他元素
- 文本编辑器不需要按行号随机访问（用户通常逐行浏览）
- 如果用 vector，中间插入会导致大量元素后移，O(n) 且迭代器失效

**场景3：deque**

- 头部取出 + 尾部加入 → 双端操作
- vector 头部删除是 O(n)（所有元素前移）
- list 虽然也可以，但 deque 的内存局部性更好，缓存命中率更高
- deque 的 `push_back`/`pop_front` 都是 O(1)


