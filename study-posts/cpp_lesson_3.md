# C++进阶笔记：第3课——值类别与const正确性

> 前两课讲了值语义和引用，今天解决上节课末尾留下的问题：为什么 `const string&` 能绑定 `"hello"`？这背后涉及C++最核心的概念之一——值类别。

---

## 3.1 为什么 const string& 能绑定 "hello"？

上节课末尾留了个问题：

```cpp
const std::string& ref = "hello";   // ✅ 可以
std::string& ref2 = "hello";        // ❌ 编译错误
```

要理解这个，必须先理解C++的值类别（Value Categories）。

---

## 3.2 左值 vs 右值：不是"在等号左边/右边"那么简单

很多教材说"左值能放等号左边，右值放右边"，这太粗糙了。C++的区分更本质：

| | 左值（lvalue） | 右值（rvalue） |
|:---|:---|:---|
| 核心特征 | 有身份（identity），有地址 | 是临时的，即将被销毁 |
| 能不能取地址 | `&x` 合法 | `&42` 非法 |
| 能不能被赋值 | 通常可以 | 不可以 |
| 例子 | 变量名、函数返回的引用、解引用后的指针 | 字面量 `42`、`a+b` 的结果、函数按值返回的对象 |

```cpp
int a = 10;
int b = 20;

int* p = &a;        // ✅ a 是左值，有地址
// int* q = &(a+b); // ❌ a+b 是右值，没有固定地址

a = 30;             // ✅ 左值可以被赋值
// (a+b) = 40;      // ❌ 右值不能被赋值
```

**关键认知**：左值和右值的区别，在于这个表达式是否代表一个持久的、可寻址的对象。

---

## 3.3 C++11 的扩展：将亡值（xvalue）

C++11之前只有左值和右值。但为了支持移动语义，C++11把右值又拆成了两类：

```
        表达式
       /          glvalue   rvalue
    /    \    /     lvalue   xvalue   prvalue
```

| 类别 | 全称 | 含义 | 例子 |
|:---|:---|:---|:---|
| lvalue | left value | 传统的左值：有身份，持久 | 变量名、`*p`、`a[i]` |
| xvalue | eXpiring value | 将亡值：有身份，但即将被移动走 | `std::move(a)`、函数返回右值引用 |
| prvalue | pure rvalue | 纯右值：临时，无身份 | `42`、`a+b`、`std::string("hi")` |
| glvalue | generalized lvalue | 广义左值 = lvalue + xvalue | 有身份的所有值 |

现在你只需要记住两个新面孔：

**xvalue（将亡值）**：有名字、有地址，但编译器知道"它快死了，可以把它资源偷走"。

```cpp
std::string a = "hello";
std::string b = std::move(a);  
// std::move(a) 的类型是 std::string&&
// 它把 a 变成了"将亡值"：a 还在，但编译器允许 b 把 a 的资源"偷"走
```

`std::move` 本身不移动任何东西，它只是把左值强制转换成将亡值，告诉编译器："这个人可以欺负，它的资源可以抢。"

---

## 3.4 为什么 const T& 能绑定临时对象？

现在回到最初的问题：

```cpp
const std::string& ref = "hello";
```

`"hello"` 是一个字符串字面量，类型是 `const char[6]`。它在表达式里是纯右值（prvalue）——没有固定地址，随时会消失。

普通引用 `std::string&` 只能绑定左值。因为如果你允许它绑定临时对象：

```cpp
std::string& ref = "hello";
ref += " world";  // 你在修改一个即将消失的临时对象！
// 这毫无意义，而且极其危险
```

但 `const std::string&` 不同：

```cpp
const std::string& ref = "hello";
```

这里发生了隐式类型转换：`"hello"` 先被构造出一个临时的 `std::string` 对象，然后 const 引用绑定到它。

因为加了 const，你承诺不修改它。C++编译器就放心地做了一个生命周期延长：这个临时对象不会立刻销毁，而是活到引用 `ref` 的作用域结束。

```cpp
void foo() {
    const std::string& ref = "hello";  
    // 临时 string 对象在这里创建
    std::cout << ref << std::endl;    
    // 出了 foo，ref 销毁，临时 string 才跟着销毁
}
```

**这就是答案**：

> `const T&` 能绑定临时对象，是因为 const 保证了"只读不修改"，编译器觉得安全，就允许延长临时对象的生命周期。

---

## 3.5 const 正确性（const correctness）

C++里 `const` 不是"可有可无的修饰符"，它是一种语义约束，是类型系统的一部分。

### 3.5.1 指针的 const：顶层 vs 底层

这是C++里最容易晕的地方，但理解了就很清晰：

```cpp
const int* p;      // 底层const：p 指向的内容不能改，但 p 可以指向别处
int* const p;      // 顶层const：p 本身不能改指向，但内容可以改
const int* const p; // 双const：内容和指向都不能改
```

**读法技巧**：从右往左读，遇到 const 就翻译"常量"
- `const int* p` → p 是指针，指向 const int → 指向的内容是常量
- `int* const p` → p 是const 指针，指向 int → 指针本身是常量

### 3.5.2 const 成员函数

```cpp
class Date {
    int year, month, day;
public:
    int getYear() const {   // 承诺：这个函数不会修改对象的状态
        return year;
    }

    void setYear(int y) {   // 没有 const，可以修改
        year = y;
    }
};

const Date d{2026, 7, 8};
int y = d.getYear();    // ✅ const 对象只能调用 const 成员函数
// d.setYear(2027);     // ❌ 编译错误！
```

const 成员函数是C++接口设计的重要部分：它让调用者一眼就知道——这个函数是只读查询，还是状态修改。

---

## 3.6 值类别 + const = 函数重载的利器

理解了值类别，你就能看懂为什么标准库里有这样的重载：

```cpp
std::vector<int> v;
v.push_back(42);           // 传右值，可以移动
v.push_back(some_vector);   // 传左值，只能拷贝
```

`push_back` 有两个版本：

```cpp
void push_back(const T& value);  // 绑定左值，拷贝
void push_back(T&& value);       // 绑定右值，移动
```

`T&&` 叫右值引用，专门用来"捕获"将亡值和纯右值，从而偷走资源而不是拷贝。

这就是第8、9课要讲的移动语义的核心机制。今天先埋个种子。

---

## 本课重点

1. **左值有身份、可寻址；右值是临时、将亡**
2. **C++11 引入 xvalue（将亡值）**：`std::move` 把左值变成将亡值，允许资源被"偷"
3. **`const T&` 能绑定临时对象**，因为 const 承诺不修改，编译器会延长临时对象生命周期
4. **const 是类型系统的一部分**：顶层const（自身不可变）vs 底层const（指向内容不可变）
5. **const 成员函数是接口契约**：调用方一眼分辨"查询"和"修改"

---

## 课后练习与答案

### 练习题

下面代码中，`a`、`b`、`c` 分别是什么值类别？（左值 / 纯右值 / 将亡值）

```cpp
int x = 10;
int y = 20;

auto a = x;           // a 是？
auto b = x + y;       // b 是？  x+y 是？
auto c = std::move(x); // c 是？  std::move(x) 是？
```

以及一个小陷阱：

```cpp
std::string s = "hello";
std::string&& r = std::move(s);   // r 是右值引用，但 r 本身是什么值类别？
// 是左值、纯右值、还是将亡值？
```

### 答案

**第一题：**

- `auto a = x;`：`a` 是**左值**。`x` 是左值，赋值给 `a` 后 `a` 成为一个新的命名变量，有身份有地址，是左值。

- `auto b = x + y;`：`b` 是**左值**（命名变量），但 `x + y` 是**纯右值（prvalue）**。`x + y` 的计算结果是一个临时整数，没有固定地址，不能被取地址。`b` 通过拷贝构造从这个纯右值创建。

- `auto c = std::move(x);`：`c` 是**左值**（命名变量），但 `std::move(x)` 是**将亡值（xvalue）**。`std::move` 把左值 `x` 强制转换为将亡值，告诉编译器可以移动它的资源。`c` 通过移动构造（如果类型支持）从这个将亡值创建。

**第二题（陷阱）：**

```cpp
std::string&& r = std::move(s);
```

`r` 的类型是 `std::string&&`（右值引用），但 **`r` 本身是左值**。

**原因**：值类别和类型是两个不同的维度。`r` 是一个有名字的变量，有固定的存储位置，可以被取地址（`&r` 合法），所以它是左值。C++规定：**有名字的东西，基本都是左值**——哪怕它的类型是"右值引用"。

这个陷阱在写移动语义时非常重要：如果你有一个 `T&&` 类型的参数，它在函数体内部是左值，如果你想把它继续作为右值传出去，必须再次用 `std::move` 转换。

```cpp
void foo(std::string&& s) {
    // s 的类型是 string&&，但 s 本身是左值！
    std::string t = s;        // 这是拷贝，不是移动！
    std::string u = std::move(s);  // 这才是移动
}
```

---

**下一课预告**：第4课讲 **函数重载与运算符重载**——理解C++如何通过类型系统实现多态接口。
