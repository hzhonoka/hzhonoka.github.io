# C++进阶笔记：第5课——析构函数与RAII

> 第4课讲了对象怎么诞生（构造、初始化），今天讲对象怎么死亡——以及为什么C++把"死亡"设计得和"诞生"一样重要。

---

## 5.1 对象的生命周期：有诞生，就有死亡

第4课讲了对象怎么诞生（构造、初始化）。C++ 要求：每一个对象诞生时，必须有且仅有一个构造函数被调用；每一个对象死亡时，必须有且仅有一个析构函数被调用。

这是 C++ 和许多其他语言最本质的区别之一。

```cpp
class File {
    std::FILE* handle;
public:
    File(const char* path) {
        handle = std::fopen(path, "r");  // 诞生：打开文件
        std::cout << "文件打开\n";
    }

    ~File() {  // 析构函数：名字是 ~类名，无参数，无返回值
        std::fclose(handle);  // 死亡：关闭文件
        std::cout << "文件关闭\n";
    }
};

void read() {
    File f("data.txt");  // 构造：打开文件
    // ... 读取操作 ...
}  // f 的作用域结束，自动调用析构函数：关闭文件
```

输出：
```
文件打开
文件关闭
```

**关键点**：`f` 是栈对象，离开作用域时自动销毁，析构函数自动调用。你不需要写 `f.close()`，不需要 `try-finally`，不需要担心异常跳过时漏掉清理。

这就是 **RAII**：

> **Resource Acquisition Is Initialization**
> 资源获取即初始化

资源（文件、内存、锁、网络连接）的获取发生在构造函数里，资源的释放绑定在析构函数里，而析构函数的调用绑定在对象的生命周期上。

---

## 5.2 为什么 RAII 是 C++ 的根基？

想象没有 RAII 的世界：

```cpp
// C 语言风格：手动管理，步步惊心
void bad_example() {
    FILE* f = fopen("data.txt", "r");
    if (some_error) {
        return;  // 糟糕！忘记 fclose 了，资源泄漏
    }
    // ... 更多代码 ...
    if (another_error) {
        return;  // 又忘了！
    }
    fclose(f);  // 终于关了，但前面两条 return 都漏了
}
```

每一个提前 return、每一个异常抛出，都是潜在的泄漏点。

C++ 的 RAII 解决方式：

```cpp
void good_example() {
    File f("data.txt");  // 构造：获取资源
    if (some_error) {
        return;  // 没问题！f 的析构自动调用，文件关闭
    }
    // ... 更多代码 ...
    if (another_error) {
        return;  // 没问题！析构照样调用
    }
}  // 正常结束，析构也调用
```

无论函数怎么退出——正常结束、return、异常抛出——栈对象的析构函数都会被调用。

这就是 C++ 的**栈展开（stack unwinding）**机制：异常发生时，编译器会沿着调用栈，依次销毁所有已构造的局部对象。

---

## 5.3 析构函数什么时候被调用？

| 场景 | 析构调用时机 |
|:---|:---|
| 栈对象离开作用域 | 自动调用 |
| 栈对象在 return 之前 | 自动调用 |
| 异常抛出，栈展开时 | 自动调用 |
| `delete` 堆对象 | 调用一次 |
| `delete[]` 堆数组 | 每个元素调用一次 |
| 对象包含成员对象 | 先析构成员，再析构自己 |
| 继承体系中 | 先析构派生类，再析构基类 |

---

## 5.4 成员对象与继承的析构顺序

```cpp
class Member {
public:
    Member()  { std::cout << "Member 构造\n"; }
    ~Member() { std::cout << "Member 析构\n"; }
};

class Base {
public:
    Base()  { std::cout << "Base 构造\n"; }
    ~Base() { std::cout << "Base 析构\n"; }
};

class Derived : public Base {
    Member m;
public:
    Derived()  { std::cout << "Derived 构造\n"; }
    ~Derived() { std::cout << "Derived 析构\n"; }
};

int main() {
    Derived d;
}
```

输出：
```
Base 构造      ← 先构造基类
Member 构造    ← 再构造成员
Derived 构造   ← 最后构造自己
Derived 析构   ← 先析构自己
Member 析构    ← 再析构成员
Base 析构      ← 最后析构基类
```

**规律**：构造和析构严格对称、顺序相反。

先构造的后析构，后构造的先析构。就像搭积木：先搭地基，最后拆地基。

---

## 5.5 虚析构函数：多态对象的生死

这是一个极易踩的坑：

```cpp
class Base {
public:
    ~Base() { std::cout << "~Base\n"; }
};

class Derived : public Base {
    int* data;
public:
    Derived() : data(new int[100]) {}
    ~Derived() { 
        delete[] data; 
        std::cout << "~Derived\n"; 
    }
};

Base* p = new Derived();
delete p;  // 输出什么？
```

输出：
```
~Base
```

`~Derived` 没调用！`data` 泄漏了！

**为什么？** 因为 `p` 的类型是 `Base*`，编译器看到 `Base` 的析构函数不是 `virtual`，就静态绑定了 `Base::~Base()`，根本没去查虚表。

**解决方案**：基类的析构函数必须是 `virtual`：

```cpp
class Base {
public:
    virtual ~Base() = default;  // 虚析构，允许派生类正确析构
};
```

**规则**：任何作为多态基类的类，析构函数必须是 `virtual`。否则通过基类指针 `delete` 派生类对象时，会泄漏资源。

---

## 5.6 默认析构与 =default / =delete

```cpp
class Good {
public:
    ~Good() = default;  // 编译器生成默认析构，逐个析构成员
};

class NoCopy {
public:
    NoCopy() = default;
    ~NoCopy() = default;
    NoCopy(const NoCopy&) = delete;       // 禁止拷贝
    NoCopy& operator=(const NoCopy&) = delete;  // 禁止赋值
};
```

`= delete` 是 C++11 的特性：显式禁止某个函数。上面的 `NoCopy` 类不可拷贝、不可赋值，但可以正常构造和析构。这是管理稀缺资源（文件描述符、锁、唯一标识）时的常用模式。

---

## 本课重点

1. **析构函数**：`~类名()`，无参无返回值，对象销毁时自动调用
2. **RAII**：资源获取放在构造函数，资源释放放在析构函数，绑定对象生命周期
3. **栈展开**：异常发生时，局部对象自动析构，保证资源不泄漏
4. **析构顺序**：与构造严格相反——先构后析，后构先析
5. **虚析构**：多态基类必须有 `virtual ~Base()`，否则 `delete` 派生类对象会泄漏
6. **`= delete`**：显式禁止拷贝/赋值，构造不可复制的资源管理类

---

## 课后练习与答案

### 练习题1：代码找错

```cpp
class Buffer {
    char* data;
    size_t size;
public:
    Buffer(size_t n) : size(n) {
        data = new char[n];
    }

    ~Buffer() {
        delete data;  // 注意这里
    }
};
```

### 答案

`delete data;` 应该是 `delete[] data;`。

`new char[n]` 使用了数组形式的 `new[]`，对应的释放必须用 `delete[]`。用 `delete` 代替 `delete[]` 是未定义行为（UB），可能导致：
- 只调用第一个元素的析构（对 `char` 来说没影响，但对有析构函数的类型会泄漏）
- 内存管理元数据不匹配，导致堆损坏或程序崩溃

**正确写法**：

```cpp
~Buffer() {
    delete[] data;
}
```

更好的做法是用 `std::vector<char>` 或 `std::unique_ptr<char[]>`，彻底避免手动管理。

### 练习题2：构造异常与析构

```cpp
class SafeFile {
    std::FILE* handle;
public:
    SafeFile(const char* path) {
        handle = fopen(path, "r");
        if (!handle) throw std::runtime_error("打开失败");
    }
    ~SafeFile() { if (handle) fclose(handle); }
};

void test() {
    SafeFile f("not_exist.txt");  // 文件不存在，构造抛异常
}
```

`test()` 函数里，如果构造抛异常了，`~SafeFile` 会被调用吗？为什么？

### 答案

**不会被调用。**

原因：C++ 对象的生命周期从构造函数**成功返回**开始。如果构造函数抛出异常，对象从未真正"诞生"，析构函数自然不会调用。

在这个具体例子里：
- `fopen` 返回 `nullptr`
- `if (!handle) throw ...` 抛出异常
- 构造函数没有成功完成，所以 `f` 这个对象从未存在过
- `~SafeFile()` 不会被调用

**但这里没有资源泄漏**：因为 `fopen` 失败了，没有打开任何文件，`handle` 是 `nullptr`，所以即使析构不调用，也没有已获取的资源需要释放。

**但如果构造函数里先成功获取了资源，然后后续操作抛异常呢？**

```cpp
class Dangerous {
    int* a;
    int* b;
public:
    Dangerous() {
        a = new int[100];      // 成功分配
        b = new int[100];      // 假设这里抛了 bad_alloc
        // 构造函数未完成，~Dangerous 不调用，a 泄漏了！
    }
    ~Dangerous() {
        delete[] a;
        delete[] b;
    }
};
```

**解决方案**：
1. 用智能指针（`std::unique_ptr`）—— 它自己就是 RAII，即使后续抛异常，之前构造好的智能指针会析构并释放资源
2. 用 `try-catch` 在构造函数里手动清理（不推荐，容易遗漏）
3. 更好的设计：每个资源单独封装成 RAII 类，不要在一个构造函数里获取多个裸资源

```cpp
class Safe {
    std::unique_ptr<int[]> a;
    std::unique_ptr<int[]> b;
public:
    Safe() : a(new int[100]), b(new int[100]) {}
    // 如果 b 的 new 抛异常，a 的 unique_ptr 析构会自动释放 a
};
```


