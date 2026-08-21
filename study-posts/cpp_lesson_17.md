# C++进阶笔记：第17课——现代C++特性速览

> 前面系统学了C++的核心机制，今天快速过一遍C++11/14/17带来的语法糖和新工具，让代码更简洁、更安全。

---

## 17.1 auto：让编译器帮你写类型

```cpp
std::map<std::string, std::vector<int>> m;
// 以前：std::map<std::string, std::vector<int>>::iterator it = m.begin();
auto it = m.begin();   // 编译器自动推导类型

auto x = 42;           // int
auto y = 3.14;         // double
auto z = "hello";      // const char*
```

`auto` 不是动态类型，类型在编译期确定，只是省掉你手写冗长类型。

**陷阱**：`auto` 会去掉引用和顶层 const：

```cpp
int a = 10;
int& ref = a;
const int c = 20;

auto x = ref;   // x 是 int，不是 int&！
auto y = c;     // y 是 int，不是 const int！

auto& xr = ref;      // xr 是 int&
const auto yc = c;   // yc 是 const int
```

`decltype`：获取表达式的精确类型：

```cpp
decltype(ref) x2 = a;   // x2 是 int&，精确保留
decltype(c) y2 = 0;      // y2 是 const int
```

---

## 17.2 范围 for：像 Python 一样遍历

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

// 以前
for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
    std::cout << *it;
}

// 现在
for (int x : v) {
    std::cout << x;
}

// 修改元素（引用）
for (int& x : v) {
    x *= 2;
}

// 只读 + 避免拷贝（const 引用）
for (const auto& x : v) {
    std::cout << x;
}
```

底层：编译器翻译成迭代器版本，零开销。

**注意**：遍历过程中不要修改容器大小（比如 push_back），否则迭代器失效。

---

## 17.3 nullptr：类型安全的空指针

```cpp
void foo(int x) { std::cout << "int
"; }
void foo(int* p) { std::cout << "pointer
"; }

foo(NULL);      // ❌ 歧义！NULL 是宏，通常定义为 0
foo(nullptr);   // ✅ 明确调用 foo(int*)
```

`nullptr` 的类型是 `std::nullptr_t`，只能隐式转换成指针类型，不会变成整数 0。

---

## 17.4 constexpr：编译期计算

```cpp
constexpr int square(int x) {   // C++11/14
    return x * x;
}

constexpr int a = square(5);    // a = 25，编译期计算
int b = 10;
// int c = square(b);           // ❌ b 不是编译期常量

// C++14 后 constexpr 函数可以写循环
constexpr int factorial(int n) {
    int result = 1;             // C++11 不允许变量，C++14 允许
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

constexpr int f5 = factorial(5);  // 120，编译期算出
```

编译期计算的好处：
- 运行时零开销
- 可用于模板参数、数组大小等需要编译期常量的地方

```cpp
std::array<int, factorial(5)> arr;   // 大小是 120，编译期确定
```

---

## 17.5 enum class：类型安全的枚举

```cpp
// 旧枚举：会隐式转成 int，容易命名冲突
enum Color { Red, Green, Blue };
enum Status { OK, Error };
// OK 和 Red 都暴露到全局命名空间

// 新枚举
enum class Color { Red, Green, Blue };
enum class Status { OK, Error };

Color c = Color::Red;      // 必须加作用域
// if (c == 0) {}         // ❌ 编译错误！不会隐式转 int
if (c == Color::Red) {}    // ✅ 必须显式比较
```

---

## 17.6 using：类型别名（替代 typedef）

```cpp
// 以前
typedef std::vector<std::pair<int, std::string>> VecPair;

// 现在（更直观，像赋值语句）
using VecPair = std::vector<std::pair<int, std::string>>;

// 函数指针别名：using 清晰得多
typedef void (*FuncPtr)(int, int);
using FuncPtr = void(*)(int, int);   // 更易读

// 模板别名（typedef 做不到！）
template<typename T>
using Vec = std::vector<T>;

Vec<int> v;      // 等价于 std::vector<int>
Vec<std::string> vs;
```

---

## 17.7 结构化绑定（C++17）：一键拆包

```cpp
std::pair<int, std::string> p{1, "hello"};

// 以前
int id = p.first;
std::string name = p.second;

// C++17
auto [id, name] = p;   // id = 1, name = "hello"

// 用于 map 遍历
std::map<int, std::string> m;
for (const auto& [key, val] : m) {   // 直接拿到 key 和 val
    std::cout << key << ": " << val << "
";
}

// 用于返回多个值
std::tuple<int, double, char> foo() {
    return {1, 3.14, 'a'};
}

auto [x, y, z] = foo();   // x=1, y=3.14, z='a'
```

---

## 17.8 std::optional（C++17）：可能不存在的值

```cpp
#include <optional>

std::optional<int> maybe;       // 目前没有值
maybe = 42;                     // 现在有值了

if (maybe) {                    // 检查是否有值
    std::cout << *maybe;        // 解引用获取值
}

maybe.reset();                  // 清空值

// 函数返回"可能失败"的结果
std::optional<std::string> readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return std::nullopt;   // 返回"无值"

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    return content;
}

auto result = readFile("data.txt");
if (!result) {
    std::cout << "读取失败
";
}
```

替代方案：以前用指针（nullptr 表示失败）或特殊值（-1 表示失败），optional 让语义更清晰。

---

## 本课重点

1. **auto**：省掉冗长类型，但注意会去掉引用和顶层 const
2. **decltype**：精确获取表达式类型
3. **范围 for**：遍历容器最简洁的写法，底层是迭代器
4. **nullptr**：类型安全的空指针，不会变成整数 0
5. **constexpr**：编译期计算，零运行时开销
6. **enum class**：类型安全枚举，不隐式转 int
7. **using**：类型别名，支持模板别名（typedef 做不到）
8. **结构化绑定**：C++17 一键拆包 pair、tuple、数组
9. **std::optional**：明确表示"值可能存在也可能不存在"

---

## 课后练习与答案

### 练习题1：结构化绑定与auto

```cpp
std::map<std::string, int> scores = {
    {"Alice", 90},
    {"Bob", 85}
};

for (auto [name, score] : scores) {
    std::cout << name << ": " << score << "
";
}
```

这段代码能编译吗？`name` 和 `score` 的类型分别是什么？如果 `scores` 很大，这里有没有性能问题？

### 答案

**能编译（C++17 起）。**

**类型分析**：
- `scores` 的元素类型是 `std::pair<const std::string, int>`
- `auto [name, score]` 中，编译器会推导 `name` 和 `score` 的类型
- `name` 的类型是 `std::string`（`const` 被 auto 剥掉了）
- `score` 的类型是 `int`

**性能问题：有拷贝开销。**

因为 `auto [name, score]` 是按值绑定（copy），每次循环都会：
1. 拷贝 `name`（深拷贝字符串）
2. 拷贝 `score`（int 的拷贝很轻）

如果 `scores` 很大，字符串拷贝会很慢。

**优化写法**：

```cpp
// 按引用绑定，避免拷贝
for (const auto& [name, score] : scores) {
    std::cout << name << ": " << score << "
";
}
```

`const auto& [name, score]` 中，`name` 是 `const std::string&`，`score` 是 `const int&`。没有拷贝，只有引用绑定。

**注意**：`auto&` 和 `auto` 在结构化绑定中的区别和普通变量一样——`auto` 会去掉引用和顶层 const，所以 `auto [name, score]` 是值拷贝。

### 练习题2：constexpr 的运行时调用

```cpp
constexpr int foo(int x) {
    int result = 0;
    for (int i = 0; i < x; ++i) {
        result += i;
    }
    return result;
}

int main() {
    int a = 5;
    std::cout << foo(a) << std::endl;
}
```

这段代码能编译吗？`foo(a)` 是编译期计算还是运行时计算？

### 答案

**能编译。**

`constexpr` 函数**可以**用运行时参数调用。此时它退化为普通函数，在**运行时**计算。

分析：
- `a` 是运行时变量（不是编译期常量）
- `foo(a)` 用运行时参数调用 `constexpr` 函数
- 编译器无法在编译期算出 `foo(a)` 的值（因为 `a` 的值运行时才确定）
- 所以 `foo(a)` 在运行时执行，和普通的 `int foo(int x)` 没有性能区别

**关键区分**：

```cpp
constexpr int foo(int x) { return x * x; }

int a = 5;

int b = foo(a);        // 运行时计算，a 不是编译期常量
constexpr int c = foo(5);  // 编译期计算，5 是编译期常量
// constexpr int d = foo(a); // ❌ 编译错误！a 不是编译期常量
```

`constexpr` 函数的语义是："这个函数**可以**在编译期执行，但也可以在运行时执行。"编译器会根据调用上下文决定。

**什么时候编译器会强制编译期计算？**

- 用于 `constexpr` 变量的初始化：`constexpr int c = foo(5);`
- 用于模板参数：`std::array<int, foo(5)> arr;`
- 用于数组大小：`int arr[foo(5)];`

这些场景要求编译期常量，所以编译器必须在编译期算出 `foo(5)`。

---

**下一课预告**：第18课讲 **编译与链接**——理解预处理、编译、汇编、链接四个阶段，以及头文件、内联、ODR等核心概念。
