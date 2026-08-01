# C++进阶笔记：第4课——初始化：对象诞生的那一刻

> 前面三课讲了值语义、引用和值类别，今天来聊C++里最容易被忽视、但极其重要的话题：对象是怎么出生的？

---

## 4.1 初始化 vs 赋值：这是两回事

很多初学者分不清，但C++严格区分：

```cpp
std::string s = "hello";   // 初始化：对象诞生时就带有值
s = "world";                // 赋值：对象已经存在，换掉它的值
```

初始化发生在对象生命周期开始的一刹那。C++提供了好几种初始化方式：

```cpp
int a = 10;         // 拷贝初始化
int b(10);          // 直接初始化
int c{10};          // 列表初始化（C++11，推荐）
int d = {10};       // 列表初始化（带等号版本）

std::vector<int> v{1, 2, 3};  // 列表初始化，填充三个元素
std::vector<int> v2(10, 0);   // 直接初始化，10个0
```

推荐用 `{}` 列表初始化，因为它最严格——遇到"窄化转换"会报错：

```cpp
int x = 3.14;       // ✅ 编译通过，x = 3（隐式截断，危险）
// int y{3.14};     // ❌ 编译错误！double 到 int 是窄化转换
```

---

## 4.2 构造函数与初始化列表

当对象有多个成员时，初始化列表是成员诞生的"真正时刻"：

```cpp
class Student {
    std::string name;
    int age;
    const int id;           // const 成员
    std::string& advisor;   // 引用成员

public:
    // ❌ 错误写法：在函数体里赋值
    Student(std::string n, int a, int i, std::string& adv) {
        name = n;      // 这里 name 已经默认构造过了，再赋值 = 浪费
        age = a;
        // id = i;      // 编译错误！const 成员不能赋值
        // advisor = adv; // 编译错误！引用必须初始化，不能赋值
    }

    // ✅ 正确写法：初始化列表
    Student(std::string n, int a, int i, std::string& adv)
        : name(n), age(a), id(i), advisor(adv)  // 成员在这里"诞生"
    {}
};
```

初始化列表的语法：在构造函数参数列表后面，用冒号 `:` 开始，逗号分隔。

**必须使用初始化列表的情况**：
- `const` 成员 —— 只能初始化，不能赋值
- 引用成员 —— 只能初始化，不能改绑
- 没有默认构造的类类型成员 —— 必须在诞生时给参数
- 基类 —— 基类部分也要在初始化列表构造

---

## 4.3 初始化顺序：不是列表里的顺序！

```cpp
class Demo {
    int a;
    int b;
public:
    Demo() : b(1), a(b) {}  // 看起来先初始化 b，再初始化 a？
};
```

**错！** C++规定：成员的初始化顺序严格按它们在类里声明的顺序，和初始化列表里的顺序无关。

上面代码中，`a` 在类里先声明，所以先初始化 `a`。但 `a(b)` 时 `b` 还没初始化！这是未定义行为（UB），可能读到垃圾值。

**规则**：初始化列表的书写顺序，必须和类内声明顺序一致。

---

## 4.4 默认构造、默认成员初始化（C++11）

```cpp
class Point {
    int x = 0;      // C++11：默认成员初始化
    int y = 0;
public:
    Point() = default;           // 显式要求编译器生成默认构造
    Point(int a, int b) : x(a), y(b) {}
};
```

`int x = 0` 是C++11的新特性：如果构造函数没有初始化 `x`，就用 `0` 作为默认值。但如果初始化列表里写了 `x(a)`，就用列表里的值。

`Point() = default;` 是告诉编译器："请帮我生成一个默认构造函数。" 这在类里你写了其他构造函数、但又需要默认构造时很有用。

---

## 4.5 委托构造（C++11）

一个构造函数可以"委托"给另一个构造函数：

```cpp
class Clock {
    int hour, minute;
public:
    Clock(int h, int m) : hour(h), minute(m) {}

    Clock() : Clock(0, 0) {}  // 委托给上面的构造函数
    Clock(int h) : Clock(h, 0) {}  // 委托给上面的构造函数
};
```

避免重复写初始化逻辑。

---

## 4.6 explicit：防止隐式转换

```cpp
class String {
public:
    String(int size);           // 分配 size 字节
    String(const char* str);    // 从 C 字符串构造
};

String s1 = 100;        // ❓ 这合法吗？
// 编译器理解为：String s1 = String(100);
// 但语义上很奇怪：把 100 当作字符串？
```

`String s1 = 100;` 会调用 `String(int)` 构造，但语义上很荒谬。用 `explicit` 阻止这种隐式转换：

```cpp
class String {
public:
    explicit String(int size);           // 禁止隐式转换
    String(const char* str);              // 隐式转换仍允许
};

String s1(100);           // ✅ 显式构造
// String s2 = 100;     // ❌ 编译错误！不能隐式转换
String s3 = "hello";     // ✅ 可以，const char* 版本不是 explicit
```

`explicit` 只加在单参数构造函数上，防止"意外的类型转换"。

---

## 本课重点

1. **初始化 ≠ 赋值**：初始化是诞生时赋值，赋值是诞生后修改
2. **用 `{}` 列表初始化**：安全、严格、推荐
3. **初始化列表是成员真正的"诞生时刻"**：const、引用、无默认构造的成员必须用它
4. **初始化顺序 = 声明顺序**，不是列表顺序
5. **C++11 新特性**：默认成员初始化、`= default`、委托构造
6. **`explicit` 阻止隐式转换**：保护构造函数不被意外调用

---

## 课后练习与答案

### 练习题1：代码找错

下面代码有什么问题？

```cpp
class Person {
    std::string name;
    int age;
    const int id;
    std::string nickname;

public:
    Person(std::string n, int a, int i, std::string nick)
        : age(a), id(i), name(n), nickname(name)   // 注意这一行
    {}
};
```

### 答案

**问题一：初始化顺序错误。**

类内声明顺序是 `name → age → id → nickname`，但初始化列表写的是 `age → id → name → nickname`。C++规定按声明顺序初始化，所以实际执行顺序是：
1. `name(n)` —— 正常
2. `age(a)` —— 正常
3. `id(i)` —— 正常
4. `nickname(name)` —— 这里 `name` 已经初始化好了，所以没问题

等等，这个例子其实碰巧安全？不，仔细看：声明顺序是 `name, age, id, nickname`，列表顺序是 `age, id, name, nickname`。实际执行顺序是声明顺序：`name(n)` → `age(a)` → `id(i)` → `nickname(name)`。`nickname(name)` 时 `name` 已经初始化，所以语义上没问题。

但**问题二**确实存在：`nickname(name)` 的语义是"用 `name` 的值初始化 `nickname`"，这本身没问题。但如果把 `nickname(name)` 改成 `nickname(nick)`，同时列表顺序和声明顺序不一致，就可能在某些编译器上触发警告（因为C++标准建议初始化列表顺序和声明顺序一致，不一致时很多编译器会报 `-Wreorder` 警告）。

更深层的问题：如果类内声明顺序是 `name, age, id, nickname`，而你想在初始化 `name` 时用 `nickname` 的值，那就会因为 `nickname` 还没初始化而读到未定义值。这个例子的核心陷阱就是：**初始化顺序和列表顺序无关**，如果你依赖"列表里的先后顺序"来安排成员间的依赖关系，就会踩坑。

**正确写法**：

```cpp
Person(std::string n, int a, int i, std::string nick)
    : name(n), age(a), id(i), nickname(nick)   // 按声明顺序写
{}
```

### 练习题2：默认成员初始化

```cpp
struct Foo {
    int x = 10;
    Foo() = default;
    Foo(int v) : x(v) {}
};

Foo a;          // a.x = ?
Foo b(20);      // b.x = ?
Foo c{};        // c.x = ?
Foo c{} 会调用默认构造吗？x 的值是多少？
```

### 答案

- `Foo a;`：`a.x = 10`。调用编译器生成的默认构造，`x` 用类内初始化的默认值 `10`。

- `Foo b(20);`：`b.x = 20`。调用 `Foo(int v)`，初始化列表里显式写了 `x(v)`，覆盖类内默认值。

- `Foo c{};`：`c.x = 10`。`c{}` 是值初始化（value-initialization），对于 `Foo` 这种有默认构造的类，它会调用默认构造。由于 `Foo() = default` 存在，调用的是默认构造，`x` 用类内默认值 `10`。

**注意**：如果没有 `Foo() = default;`，且只写了 `Foo(int v)`，那 `Foo c{};` 会编译错误——因为编译器不会自动生成默认构造了。`Foo() = default;` 的作用就是显式保留默认构造的能力。


