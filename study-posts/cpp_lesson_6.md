# C++进阶笔记：第6课——拷贝构造与拷贝赋值

> 前面五课讲了对象怎么诞生、怎么死亡、以及RAII的哲学。今天进入资源管理的核心：当一个对象需要"复制"另一个对象时，C++默认做了什么？为什么这往往是错的？

---

## 6.1 默认的拷贝够不够用？

假设你写了一个管理动态数组的类：

```cpp
class Buffer {
    int* data;
    size_t len;
public:
    Buffer(size_t n) : len(n) {
        data = new int[n]();  // 申请堆内存
    }

    ~Buffer() {
        delete[] data;  // 释放堆内存
    }
};
```

现在你想复制它：

```cpp
Buffer a(100);      // a 拥有 100 个 int 的内存
Buffer b = a;       // 想复制一份给 b
```

如果你没写任何拷贝相关的代码，C++ 编译器会帮你生成一个默认的拷贝构造函数。它做的事情很简单：

**把每个成员逐字节拷贝过去。**

也就是：

```cpp
// 编译器默认生成的拷贝构造，逻辑上等价于：
b.data = a.data;   // b.data 和 a.data 指向同一块内存！
b.len = a.len;
```

这看起来复制了，但 `a` 和 `b` 的 `data` 指针指向同一块堆内存。

然后程序结束时：
1. `b` 先析构 → `delete[] data` → 这块内存被释放了
2. `a` 再析构 → `delete[] data` → 同一块内存又被释放一次！

这就是 **double free（重复释放）**，程序直接崩溃。

---

## 6.2 浅拷贝 vs 深拷贝

| | 浅拷贝（shallow copy） | 深拷贝（deep copy） |
|:---|:---|:---|
| 行为 | 只拷贝指针值，共享同一块内存 | 申请新内存，拷贝内容 |
| 后果 | 多个对象指向同一块资源，析构时重复释放 | 每个对象拥有自己的独立资源 |
| 适用场景 | 对象不管理资源（比如纯数据类） | 对象管理动态内存、文件句柄等资源 |

C++ 默认生成的拷贝是浅拷贝。**只要你的类里有指针指向堆内存，就必须自己写深拷贝。**

---

## 6.3 拷贝构造函数

语法：`类名(const 类名& other)`

```cpp
class Buffer {
    int* data;
    size_t len;
public:
    Buffer(size_t n) : len(n) {
        data = new int[n]();
    }

    // 拷贝构造函数：深拷贝
    Buffer(const Buffer& other) : len(other.len) {
        data = new int[len];           // 1. 给自己申请新内存
        std::copy(other.data, other.data + len, data);  // 2. 把内容复制过来
    }

    ~Buffer() {
        delete[] data;
    }
};
```

**关键点**：
- 参数是 `const Buffer&`：引用传参，避免无限递归拷贝
- `const` 修饰：承诺不修改原对象
- 先申请新内存，再复制内容

现在：

```cpp
Buffer a(100);
Buffer b = a;      // 调用拷贝构造，b 拥有独立的 100 个 int
Buffer c(a);       // 也是拷贝构造，c 也独立
// a、b、c 各管各的内存，析构时互不干扰
```

---

## 6.4 拷贝赋值运算符

拷贝构造用于**创建新对象时**的复制。但如果对象已经存在，你想重新赋值呢？

```cpp
Buffer a(100);
Buffer b(50);
b = a;   // b 已经存在了，现在想让 b 变成 a 的副本
```

这需要拷贝赋值运算符：

```cpp
class Buffer {
    // ... 前面的代码不变 ...

    // 拷贝赋值运算符
    Buffer& operator=(const Buffer& other) {
        // 1. 自赋值检查：a = a 时不要瞎操作
        if (this != &other) {
            // 2. 释放自己原来的资源
            delete[] data;

            // 3. 重新申请，复制新内容
            len = other.len;
            data = new int[len];
            std::copy(other.data, other.data + len, data);
        }

        // 4. 返回自身引用，支持链式赋值：a = b = c
        return *this;
    }
};
```

**为什么返回 `Buffer&`？**

为了支持：

```cpp
a = b = c;  // 等价于 a = (b = c);
```

`b = c` 返回 `b` 的引用，然后 `a = b` 再执行。

**为什么参数是 `const Buffer&`？**

和拷贝构造一样：避免拷贝原对象，且承诺不修改它。

---

## 6.5 自赋值检查：为什么 `if (this != &other)` 很重要？

```cpp
Buffer a(100);
a = a;  // 自赋值
```

如果没有自赋值检查：
1. `delete[] data;` —— 把自己的内存释放了！
2. `len = other.len;` —— 没问题
3. `data = new int[len];` —— 申请新内存
4. `std::copy(other.data, ...)` —— `other.data` 就是刚才被 `delete` 的内存！

读已经释放的内存 = 未定义行为（UB），程序可能崩溃，可能读到垃圾值。

所以自赋值检查是拷贝赋值的必备安全措施。

---

## 6.6 三法则（Rule of Three）

C++ 社区总结出一个经验规律：

> 如果一个类需要自定义析构函数，那么它几乎肯定也需要自定义拷贝构造函数和拷贝赋值运算符。

这就是**三法则**：
1. 析构函数（释放资源）
2. 拷贝构造函数（复制资源）
3. 拷贝赋值运算符（复制资源并释放旧资源）

三者要么都自定义，要么都不需要自定义（编译器生成的默认版本就够）。

后来 C++11 加入了移动语义，扩展为**五法则**（加上移动构造和移动赋值），那个我们第8、9课再讲。

---

## 6.7 拷贝省略（RVO）：编译器帮你偷懒

有时候编译器比你聪明：

```cpp
Buffer make_buffer() {
    Buffer tmp(100);
    return tmp;  // 返回 tmp
}

Buffer a = make_buffer();  // 这里发生了几次拷贝？
```

理论上：
1. `make_buffer` 里构造 `tmp`
2. `return tmp` 拷贝构造一个临时对象
3. `Buffer a = ...` 再用临时对象拷贝构造 `a`

两次拷贝。

但现代编译器会优化：

```cpp
Buffer a(100);  // 编译器直接把 a 当作 tmp 来构造，零拷贝！
```

这叫 **RVO（Return Value Optimization，返回值优化）**。C++17 起，这种优化是强制保证的，即使你的拷贝构造有副作用（比如打印），也不会执行。

所以：**不要让你的拷贝构造函数有副作用**（比如计数、打印），因为编译器可能完全跳过它。

---

## 本课重点

1. **默认拷贝是浅拷贝**：逐成员复制，指针指向同一块内存
2. **深拷贝**：申请新内存，复制内容，各自独立
3. **拷贝构造函数**：`类名(const 类名&)`，用于创建新对象时的复制
4. **拷贝赋值运算符**：`类名& operator=(const 类名&)`，用于已存在对象的重新赋值
5. **自赋值检查**：`if (this != &other)` 是拷贝赋值的安全底线
6. **三法则**：需要自定义析构的类，通常也需要自定义拷贝构造和拷贝赋值
7. **RVO**：编译器可能省略拷贝，不要依赖拷贝构造的副作用

---

## 课后练习与答案

### 练习题1：代码找错

```cpp
class String {
    char* str;
public:
    String(const char* s) {
        str = new char[strlen(s) + 1];
        strcpy(str, s);
    }

    ~String() {
        delete[] str;
    }

    // 下面这个拷贝赋值有什么问题？
    String& operator=(const String& other) {
        delete[] str;
        str = new char[strlen(other.str) + 1];
        strcpy(str, other.str);
        return *this;
    }
};
```

### 答案

**缺少自赋值检查。**

如果执行 `s = s`：
1. `delete[] str;` —— 把自己的内存释放了
2. `str = new char[strlen(other.str) + 1];` —— `other.str` 就是刚才被释放的内存
3. `strcpy(str, other.str);` —— 读取已释放内存 = 未定义行为

**正确写法**：

```cpp
String& operator=(const String& other) {
    if (this != &other) {           // 自赋值检查
        delete[] str;
        str = new char[strlen(other.str) + 1];
        strcpy(str, other.str);
    }
    return *this;
}
```

更健壮的写法（异常安全）：先申请新内存、复制成功后再释放旧内存，避免 `new` 失败时把原数据丢了：

```cpp
String& operator=(const String& other) {
    if (this != &other) {
        char* tmp = new char[strlen(other.str) + 1];
        strcpy(tmp, other.str);
        delete[] str;   // 新内存申请成功后再释放旧的
        str = tmp;
    }
    return *this;
}
```

### 练习题2：区分拷贝构造和拷贝赋值

```cpp
Buffer a(10);
Buffer b = a;   // 调用的是拷贝构造还是拷贝赋值？
Buffer c(5);
c = a;          // 调用的是拷贝构造还是拷贝赋值？
```

### 答案

- `Buffer b = a;`：**拷贝构造函数**。`b` 在这里被创建，`= a` 是拷贝初始化的语法，不是赋值运算符。等价于 `Buffer b(a);`。

- `c = a;`：**拷贝赋值运算符**。`c` 已经存在（前面 `Buffer c(5)` 已经构造了），`=` 是赋值操作，调用 `operator=`。

**区分口诀**：
- 有 `=` 但左边是**新对象在诞生** → 拷贝构造
- 有 `=` 但左边对象**已经存在** → 拷贝赋值

```cpp
Buffer b = a;     // 拷贝构造（b 刚诞生）
Buffer c(a);      // 拷贝构造（显式写法）
c = a;            // 拷贝赋值（c 已存在）
```

