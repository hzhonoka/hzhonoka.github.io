# C++进阶笔记：第7课——移动语义与右值引用

> 第6课讲了拷贝构造和拷贝赋值，解决了"如何安全复制"的问题。但当对象即将死亡时，复制是浪费的——移动语义让我们可以直接"偷"走它的资源。

---

## 7.1 拷贝有时候太浪费了

假设你有一个管理大内存的类：

```cpp
class BigData {
    int* huge;
    size_t size;
public:
    BigData(size_t n) : size(n) {
        huge = new int[n];
    }

    // 深拷贝：申请新内存，全部复制
    BigData(const BigData& other) : size(other.size) {
        huge = new int[size];
        std::copy(other.huge, other.huge + size, huge);
    }

    ~BigData() { delete[] huge; }
};
```

现在：

```cpp
BigData make_data() {
    BigData tmp(1000000);   // 构造
    return tmp;              // 返回
}

BigData a = make_data();    // 这里发生了什么？
```

按照传统语义，`tmp` 是局部变量，返回时要拷贝一份给 `a`。100万个 int = 4MB 内存，全部复制一遍！

但 `tmp` 马上就要死了（离开作用域销毁）。我们能不能不拷贝，直接把 `tmp` 的资源转移给 `a`？

这就是**移动语义**要解决的问题。

---

## 7.2 右值引用 T&&

C++11 引入了新语法：`T&&`，读作"右值引用"。

它专门用来绑定即将死亡的临时对象（右值 / 将亡值）：

```cpp
void foo(BigData& x);       // 左值引用：只能绑定左值（有名字的对象）
void foo(BigData&& x);      // 右值引用：只能绑定右值（临时对象）

BigData a(100);
foo(a);                     // 调用 foo(BigData&) —— a 是左值

foo(BigData(100));          // 调用 foo(BigData&&) —— BigData(100) 是临时对象
foo(make_data());           // 调用 foo(BigData&&) —— 返回值是临时对象
```

---

## 7.3 移动构造函数

```cpp
class BigData {
    int* huge;
    size_t size;
public:
    BigData(size_t n) : size(n) {
        huge = new int[n];
    }

    // 拷贝构造：深拷贝
    BigData(const BigData& other) : size(other.size) {
        huge = new int[size];
        std::copy(other.huge, other.huge + size, huge);
    }

    // 移动构造：偷资源！
    BigData(BigData&& other) noexcept   // 注意：参数是非const的右值引用
        : huge(other.huge)             // 1. 直接拿走对方的指针
        , size(other.size)             // 2. 拿走对方的大小
    {
        other.huge = nullptr;           // 3. 把对方置空！
        other.size = 0;
    }

    ~BigData() { delete[] huge; }
};
```

**移动构造做了什么？**
1. 直接接管对方的指针和大小（不申请新内存，不复制数据）
2. 把对方置空，让它析构时 `delete[] nullptr`（安全无操作）

就像搬家：你把对方的家具直接搬到自己家，然后给对方留下一间空房子。

---

## 7.4 std::move：把左值"变成"右值

右值引用只能绑定临时对象。但如果我想主动把一个左值移动走呢？

```cpp
BigData a(1000000);
BigData b = a;           // 拷贝构造——a 还要用，不能偷

BigData c = std::move(a); // 移动构造——告诉编译器："a 我不要了，你可以偷"
```

`std::move` 本质上是一个类型转换：

```cpp
// std::move 的简化实现
template<typename T>
typename remove_reference<T>::type&& move(T&& t) {
    return static_cast<typename remove_reference<T>::type&&>(t);
}
```

它把左值强制转换成将亡值（xvalue），让编译器可以匹配移动构造。

**重要警告**：`std::move(a)` 之后，`a` 进入了**有效但未指定状态**。你可以重新赋值给它，但不要再假设它还有原来的值。

```cpp
BigData a(100);
BigData b = std::move(a);

// a.huge 现在是 nullptr！
// a.size 现在是 0！
// 不要读 a 的内容，但可以：a = something_new; 重新赋值
```

---

## 7.5 移动赋值运算符

和拷贝赋值对应，也有移动赋值：

```cpp
BigData& operator=(BigData&& other) noexcept {
    if (this != &other) {       // 自赋值检查
        delete[] huge;          // 释放自己的旧资源

        huge = other.huge;      // 偷对方的资源
        size = other.size;

        other.huge = nullptr;   // 把对方置空
        other.size = 0;
    }
    return *this;
}
```

---

## 7.6 五法则（Rule of Five）

C++11 扩展了三法则：

> 如果一个类管理资源，通常需要定义以下五个特殊成员：
1. 析构函数
2. 拷贝构造函数
3. 拷贝赋值运算符
4. 移动构造函数
5. 移动赋值运算符

但现代 C++ 有一个更简洁的惯用法：把资源管理交给智能指针或容器，让编译器自动生成这五个函数。

```cpp
class Modern {
    std::unique_ptr<int[]> data;  // 让 unique_ptr 管内存
    size_t size;
public:
    Modern(size_t n) : data(new int[n]), size(n) {}
    // 编译器自动生成的拷贝/移动/析构都是正确的！
};
```

---

## 7.7 为什么移动构造要加 noexcept？

```cpp
BigData(BigData&& other) noexcept { ... }
```

`noexcept` 告诉编译器："这个函数不会抛异常。"

标准库容器（如 `vector`）在扩容时，如果元素的移动构造是 `noexcept`，它会用移动来转移旧元素；如果不是，它只能用拷贝（因为拷贝抛异常时还能回滚，移动抛异常就回不来了）。

所以移动构造加 `noexcept` 是性能关键。

---

## 本课重点

1. **移动语义**：不拷贝资源，直接"偷"走，原对象置空
2. **右值引用 `T&&`**：专门绑定临时对象，区分于左值引用 `T&`
3. **`std::move`**：把左值强制转成将亡值，允许被移动
4. **移动后原对象处于"空"状态**：可以重新赋值，但不要再读旧值
5. **五法则**：析构、拷贝构、拷贝赋、移动构、移动赋
6. **`noexcept`**：移动构造标记不抛异常，让标准库敢用移动代替拷贝

---

## 课后练习与答案

### 练习题1：区分拷贝与移动

```cpp
BigData a(100);
BigData b = a;              // ① 拷贝还是移动？
BigData c = std::move(a);   // ② 拷贝还是移动？
BigData d = BigData(50);    // ③ 拷贝还是移动？
```

### 答案

**① `BigData b = a;`**：**拷贝构造**。`a` 是左值（有名字的变量），匹配 `const BigData&` 参数，调用拷贝构造。`a` 还要继续使用，不能偷它的资源。

**② `BigData c = std::move(a);`**：**移动构造**。`std::move(a)` 把 `a` 转换成将亡值（xvalue），类型为 `BigData&&`，匹配移动构造函数 `BigData(BigData&&)`。`a` 的资源被转移到 `c`，`a` 本身被置空。

**③ `BigData d = BigData(50);`**：**移动构造**（理论上），但实际可能是**零拷贝**。`BigData(50)` 是纯右值（prvalue），类型为 `BigData&&`，匹配移动构造。但现代编译器会应用 RVO/NRVO，直接把 `d` 在 `BigData(50)` 的位置构造，完全不调用任何构造/移动函数。

### 练习题2：移动构造的安全性

下面这个移动构造安全吗？如果 `other` 本来就是空的，会出问题吗？

```cpp
BigData(BigData&& other) noexcept
    : huge(other.huge)
    , size(other.size)
{
    other.huge = nullptr;
    other.size = 0;
}
```

### 答案

**是安全的，即使 `other` 是空的也没问题。**

分析各种情况：

**情况A：`other` 是正常对象（有资源）**
- `huge = other.huge` —— 接管指针
- `size = other.size` —— 接管大小
- `other.huge = nullptr` —— 对方置空，析构时 `delete[] nullptr` 安全
- `other.size = 0` —— 对方大小归零
- ✅ 正常

**情况B：`other` 已经被移动过（是空的）**
- `huge = other.huge` —— `huge = nullptr`
- `size = other.size` —— `size = 0`
- `other.huge = nullptr` —— 已经是 `nullptr`，无变化
- `other.size = 0` —— 已经是 `0`，无变化
- ✅ 安全，只是把自己的 `huge` 也设成了 `nullptr`

**情况C：自移动 `a = std::move(a)`**
- `huge = other.huge` —— 但 `other` 就是 `*this`，所以 `huge = huge`（无变化）
- `size = other.size` —— `size = size`（无变化）
- `other.huge = nullptr` —— 把自己的 `huge` 设成 `nullptr`！
- `other.size = 0` —— 把自己的 `size` 设成 `0`！
- ⚠️ **自移动会导致对象变成空！**

虽然自移动在正确代码中很少见，但严谨的实现应该加自赋值检查：

```cpp
BigData(BigData&& other) noexcept
    : huge(other.huge)
    , size(other.size)
{
    if (this != &other) {       // 自移动检查
        other.huge = nullptr;
        other.size = 0;
    }
}
```

不过实际上，标准库的移动构造通常**不检查自移动**，因为 `std::move(a)` 后再用 `a` 是未定义行为，程序员不应该这么做。自移动检查主要是防御性编程。


