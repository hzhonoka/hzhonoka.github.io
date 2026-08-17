# C++进阶笔记：第14课——模板基础

> 前面学了面向对象的继承多态，今天进入C++的另一大支柱——泛型编程。模板让代码脱离具体类型，只保留算法骨架。

---

## 14.1 为什么需要模板？

假设你要写一个求最大值的函数：

```cpp
int max(int a, int b) { return a > b ? a : b; }

double max(double a, double b) { return a > b ? a : b; }

std::string max(const std::string& a, const std::string& b) {
    return a > b ? a : b;
}
```

逻辑完全一样，只是类型不同。模板让你写一次，编译器帮你生成各种版本：

```cpp
template<typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

max(3, 5);              // 编译器自动生成 int 版本
max(3.14, 2.71);        // 编译器自动生成 double 版本
max(std::string("a"), std::string("b"));  // 编译器自动生成 string 版本
```

模板不是运行时的多态（不像虚函数那样查表），而是**编译期的代码生成**。每种类型都生成一份独立的机器码，零运行时开销。

---

## 14.2 函数模板

语法：`template<typename T>` 或 `template<class T>`（两者等价）

```cpp
template<typename T>
void swap(T& a, T& b) {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}
```

**类型推导**：

```cpp
int x = 1, y = 2;
swap(x, y);  // 编译器看到 x 和 y 是 int，自动推导出 T = int
```

你也可以显式指定：

```cpp
swap<int>(x, y);  // 显式实例化，通常没必要
```

**多个模板参数**：

```cpp
template<typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {  // C++11 尾置返回类型
    return a + b;
}

auto c = add(1, 2.5);  // T=int, U=double, 返回 double
```

C++14 以后可以简写：

```cpp
template<typename T, typename U>
auto add(T a, U b) {   // 编译器自动推导返回类型
    return a + b;
}
```

---

## 14.3 类模板

STL 容器全是类模板。你自己也可以写：

```cpp
template<typename T>
class Box {
    T value;
public:
    Box(T v) : value(std::move(v)) {}

    T get() const { return value; }
    void set(T v) { value = std::move(v); }
};

Box<int> b1(42);           // 显式指定类型
Box<std::string> b2("hi"); // 显式指定类型
```

类模板的成员函数如果在类外定义：

```cpp
template<typename T>
class Container {
    T* data;
    size_t size;
public:
    Container(size_t n);
    ~Container();
};

// 类外定义时，每个函数都要带 template<typename T>
template<typename T>
Container<T>::Container(size_t n) : size(n) {
    data = new T[n];
}

template<typename T>
Container<T>::~Container() {
    delete[] data;
}
```

注意：类名是 `Container<T>`，不是 `Container`。模板参数必须带着。

---

## 14.4 非类型模板参数

模板参数不一定是类型，也可以是编译期常量：

```cpp
template<typename T, size_t N>
class Array {
    T data[N];  // N 是编译期已知的！
public:
    T& operator[](size_t i) { return data[i]; }
    size_t size() const { return N; }
};

Array<int, 100> arr;  // 100 是编译期常量，和 std::array 一样
```

这和 `std::vector<int> v(100)` 的区别：
- `Array<int, 100>`：`data` 是栈数组（或类内数组成员），大小编译期确定，零开销
- `vector<int> v(100)`：`data` 是堆指针，运行时动态分配

非类型模板参数必须是编译期可计算的整数、指针、引用或枚举。

---

## 14.5 特化：给特定类型特殊待遇

**全特化**：给某个具体类型写专用版本。

```cpp
template<typename T>
class Storage {
public:
    void info() { std::cout << "通用版本\n"; }
};

// int 的专用版本
template<>
class Storage<int> {
public:
    void info() { std::cout << "int 专用版本\n"; }
};

Storage<double> s1;  // 通用版本
Storage<int> s2;     // int 专用版本
```

函数模板特化（用得较少，通常用重载代替）：

```cpp
template<typename T>
void print(const T& x) {
    std::cout << x << '\n';
}

template<>
void print<const char*>(const char* const& s) {  // 全特化
    std::cout << "字符串: " << s << '\n';
}
```

---

## 14.6 偏特化：部分参数固定

```cpp
template<typename T, typename U>
class Pair {
    // 通用版本
};

// 偏特化：第二个参数固定为 int
template<typename T>
class Pair<T, int> {
    // 专用版本
};

// 偏特化：两个参数相同
template<typename T>
class Pair<T, T> {
    // 专用版本
};
```

类模板支持偏特化，函数模板不支持（函数用重载代替）。

---

## 14.7 类型推导的陷阱

```cpp
template<typename T>
void foo(T x) {}

template<typename T>
void bar(T& x) {}

int a = 10;
foo(a);   // T = int，x 是值拷贝
bar(a);   // T = int，x 是 int&

const int b = 20;
foo(b);   // T = int，const 被剥掉了！x 是值拷贝，拷贝本身就去掉了 const
bar(b);   // T = const int，x 是 const int&，const 保留！
```

**规则**：
- 值传递 `T`：顶层 const 会被忽略（反正要拷贝一份）
- 引用传递 `T&`：const 会保留，推导出的 `T` 可能带 const

---

## 14.8 结合竞赛：写一个通用线段树

你写过线段树，如果用模板封装：

```cpp
template<typename T, typename Merge>
class SegTree {
    std::vector<T> tree;
    Merge merge;
    int n;

public:
    SegTree(const std::vector<T>& arr, Merge m) : merge(m) {
        n = arr.size();
        tree.resize(4 * n);
        build(arr, 1, 0, n - 1);
    }

    T query(int l, int r) { return query(1, 0, n - 1, l, r); }

private:
    void build(const std::vector<T>& arr, int node, int nl, int nr);
    T query(int node, int nl, int nr, int l, int r);
    // ...
};
```

使用：

```cpp
auto sum = [](int a, int b) { return a + b; };
SegTree<int, decltype(sum)> st(arr, sum);

auto mx = [](int a, int b) { return std::max(a, b); };
SegTree<int, decltype(mx)> st2(arr, mx);
```

模板让数据结构脱离具体业务，只保留算法骨架。

---

## 本课重点

1. **模板是编译期代码生成**：每种类型生成一份独立代码，零运行时开销
2. **函数模板**：编译器自动推导类型，也可显式指定
3. **类模板**：STL 容器的根基，类外定义要带 `template<typename T>`
4. **非类型模板参数**：编译期常量（如数组大小），实现零开销抽象
5. **全特化/偏特化**：给特定类型或特定模式写专用优化版本
6. **类型推导规则**：值传参会去掉顶层 const，引用传递会保留

---

## 课后练习与答案

### 练习题1：模板重载解析

```cpp
template<typename T>
void foo(T x) { std::cout << "1\n"; }

template<typename T>
void foo(T& x) { std::cout << "2\n"; }

int a = 10;
foo(a);        // 调用哪个？
foo<int&>(a);  // 调用哪个？
```

提示：模板重载解析优先级。`T&` 比 `T` 更"特化"吗？

### 答案

**`foo(a)` 调用版本 2（`T&`）。**

分析：两个模板都匹配 `foo(a)`：
- 版本1：`T = int`，`foo(int x)` —— 匹配
- 版本2：`T = int`，`foo(int& x)` —— 匹配

模板重载解析规则：**更特化的模板优先**。`T&` 比 `T` 更特化，因为 `T&` 只能匹配引用类型，而 `T` 可以匹配任何类型。所以版本2胜出。

**`foo<int&>(a)` 调用版本 1。**

分析：显式指定 `T = int&`：
- 版本1：`T = int&`，`foo(int& x)` —— 匹配（`int&` 传值拷贝？不，`T` 已经是 `int&`，所以 `x` 的类型是 `int&`，即引用）
- 版本2：`T = int&`，`foo(int&& x)` —— 这是右值引用！不匹配 `a`（左值）

等等，这里需要更仔细分析。版本2的参数是 `T& x`，当 `T = int&` 时，根据引用折叠规则，`T&` 变成 `int& &` → `int&`。所以版本2的参数实际上是 `int& x`，也能匹配。

但C++模板重载有一个特殊规则：当显式指定模板参数时，如果两个模板都匹配，编译器会报错（歧义）。不过实际测试中，大多数编译器会选择版本1，因为显式实例化时版本1的签名更直接。

**更安全的理解**：显式指定模板参数时，尽量避免这种有歧义的重载。实际比赛中也很少这么写。

### 练习题2：类模板参数推导

```cpp
template<typename T>
class A {
    T data;
public:
    A(T v) : data(v) {}
};

A a(10);  // 这里
```

能编译吗？如果不能，为什么？

### 答案

**C++17 之前：不能编译。**

C++17 之前，类模板必须显式指定模板参数：

```cpp
A<int> a(10);  // ✅ C++17 之前必须这么写
A a(10);       // ❌ C++17 之前编译错误
```

**C++17 及之后：可以编译。**

C++17 引入了**类模板参数推导（CTAD, Class Template Argument Deduction）**。编译器看到 `A a(10)`，根据构造函数的参数 `10`（`int` 类型），自动推导出 `T = int`。

**注意**：CTAD 只适用于有构造函数能从参数推导出类型的情况。如果类没有合适的构造函数，或者推导有歧义，还是需要显式指定。

```cpp
// C++17 起，这些都可以
A a(10);        // T = int
A b(3.14);      // T = double
A c("hello");   // T = const char*
```

**竞赛建议**：如果比赛环境是 C++17 之前（比如某些 OJ），记得显式写模板参数。如果是 C++17/20，可以用 CTAD 让代码更简洁。


