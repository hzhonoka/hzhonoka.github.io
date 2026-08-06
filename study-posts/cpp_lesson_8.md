# C++进阶笔记：第8课——运算符重载

> 前面七课讲了对象模型、生命周期、资源管理。今天进入C++的另一个核心特性：让自定义类型像内置类型一样自然——运算符重载。

---

## 8.1 为什么要重载运算符？

C++ 允许你给自定义类型定义 `+`、`-`、`*`、`[]`、`<<` 这些运算符的行为，让代码更直观。

```cpp
Complex a(1, 2);   // 1 + 2i
Complex b(3, 4);   // 3 + 4i

Complex c = a + b;  // 直观！如果没有运算符重载，你得写：
// Complex c = a.add(b);  // 不符合数学直觉
```

---

## 8.2 成员函数 vs 非成员函数

运算符重载有两种写法：

### 写法一：成员函数

```cpp
class Complex {
    double real, imag;
public:
    Complex(double r, double i) : real(r), imag(i) {}

    // 成员函数：左操作数是 *this
    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
};

Complex a(1, 2);
Complex b(3, 4);
Complex c = a + b;  // 等价于 a.operator+(b)
```

### 写法二：非成员函数

```cpp
class Complex {
    double real, imag;
public:
    Complex(double r, double i) : real(r), imag(i) {}

    // 声明为友元，允许访问私有成员
    friend Complex operator+(const Complex& lhs, const Complex& rhs);
};

Complex operator+(const Complex& lhs, const Complex& rhs) {
    return Complex(lhs.real + rhs.real, lhs.imag + rhs.imag);
}

Complex c = a + b;  // 等价于 operator+(a, b)
```

---

## 8.3 什么时候用成员，什么时候用非成员？

| 运算符 | 推荐方式 | 原因 |
|:---|:---|:---|
| `=` `[]` `()` `->` | 必须是成员 | C++ 语法规定 |
| `+=` `-=` `*=` 等复合赋值 | 推荐成员 | 修改自身，和 `this` 绑定自然 |
| `+` `-` `*` 等对称二元 | 推荐非成员 | 允许左操作数隐式转换 |
| `<<` `>>` (IO) | 必须非成员 | 左操作数是 `ostream`，不是你的类 |

对称运算符用非成员的例子：

```cpp
Complex a(1, 2);
Complex b = a + 3.0;   // 如果 operator+ 是成员：a.operator+(3.0) ✅
Complex c = 3.0 + a;   // 如果 operator+ 是成员：3.0.operator+(a) ❌ 编译错误！

// 非成员版本：operator+(3.0, a) ✅ 两边都能隐式转换
```

---

## 8.4 输入输出运算符 << 和 >>

这是最常见的非成员运算符重载：

```cpp
class Complex {
    double real, imag;
public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    friend std::ostream& operator<<(std::ostream& os, const Complex& c);
    friend std::istream& operator>>(std::istream& is, Complex& c);
};

std::ostream& operator<<(std::ostream& os, const Complex& c) {
    os << c.real << " + " << c.imag << "i";
    return os;  // 返回引用，支持链式：cout << a << b
}

std::istream& operator>>(std::istream& is, Complex& c) {
    is >> c.real >> c.imag;
    return is;
}

// 使用
Complex a(1, 2);
std::cout << a << std::endl;   // 输出：1 + 2i
```

**为什么返回引用？**

为了支持：

```cpp
std::cout << a << b << c;
// 等价于
// operator<<(operator<<(operator<<(cout, a), b), c);
```

如果返回值而不是引用，第二次 `<< b` 就是在临时对象上操作，会出问题。

---

## 8.5 下标运算符 []

必须是成员函数，通常成对提供 const 和非 const 版本：

```cpp
class Array {
    int* data;
    size_t len;
public:
    // 非 const 版本：允许修改
    int& operator[](size_t i) {
        if (i >= len) throw std::out_of_range("越界");
        return data[i];
    }

    // const 版本：只读
    const int& operator[](size_t i) const {
        if (i >= len) throw std::out_of_range("越界");
        return data[i];
    }
};

Array arr(10);
arr[0] = 5;          // 调用非 const 版本，返回 int&，可以赋值

const Array& cref = arr;
int x = cref[0];     // 调用 const 版本，返回 const int&，只读
```

**为什么要两个版本？**
- const 对象只能调用 const 成员函数
- 如果只有非 const 版本，`const Array` 就不能用 `[]`
- const 版本返回 `const int&`，防止通过下标修改常量对象

---

## 8.6 自增运算符 ++：前置 vs 后置

这是一个经典坑。C++ 用参数个数区分前置和后置：

```cpp
class Counter {
    int value;
public:
    // 前置 ++i：返回引用，效率更高
    Counter& operator++() {
        ++value;
        return *this;
    }

    // 后置 i++：返回旧值的拷贝，效率低
    Counter operator++(int) {  // 注意：多了一个 int 参数，只是标记
        Counter old = *this;   // 保存旧值
        ++value;               // 自增
        return old;            // 返回旧值
    }
};
```

使用：

```cpp
Counter c;
++c;    // 调用 operator++()
c++;    // 调用 operator++(int)
```

**最佳实践**：能用前置就别用后置。后置需要保存旧值再返回，多一次拷贝。

---

## 8.7 类型转换运算符

让自定义类型可以隐式转换成其他类型：

```cpp
class Fraction {
    int num, den;
public:
    Fraction(int n, int d) : num(n), den(d) {}

    // 转换成 double
    operator double() const {
        return static_cast<double>(num) / den;
    }
};

Fraction f(3, 4);
double d = f;           // 隐式转换：0.75
double e = f + 0.5;     // f 先转 double，再相加
```

**注意**：隐式转换有时候太"积极"了，会导致意外匹配。可以用 `explicit` 限制：

```cpp
explicit operator bool() const {  // 必须显式转 bool
    return num != 0;
}
```

---

## 本课重点

1. **运算符重载让自定义类型像内置类型一样自然**
2. **对称二元运算符（`+` `-` `*`）推荐非成员**：支持左右隐式转换
3. **`<<` `>>` 必须非成员**：左操作数是流对象
4. **`[]` 必须成员**，且通常提供 const + 非 const 双版本
5. **前置 `++` 返回引用**，后置 `++` 返回拷贝（多一个 `int` 哑参数区分）
6. **类型转换运算符**可以实现隐式转换，但谨慎使用 `explicit`

---

## 课后练习与答案

### 练习题1：成员 vs 非成员

```cpp
class String {
    char* data;
public:
    String(const char* s);

    // 下面两种写法，哪种更好？为什么？
    String operator+(const String& other) const;           // A：成员
};

// B：非成员
String operator+(const String& lhs, const String& rhs);
```

提示：考虑 `String s = "hello" + s2;` 能不能编译。

### 答案

**B（非成员）更好。**

原因：

如果 `operator+` 是成员函数（A），调用形式是 `lhs.operator+(rhs)`，要求 `lhs` 必须是 `String` 类型。

```cpp
String s1 = "hello";
String s2 = "world";

String a = s1 + s2;        // ✅ s1 是 String，调用 s1.operator+(s2)
String b = s1 + "world";   // ✅ s1 是 String，"world" 隐式转 String
// String c = "hello" + s2; // ❌ "hello" 是 const char*，不是 String，没有 operator+
```

如果 `operator+` 是非成员函数（B），两边都可以隐式转换：

```cpp
String a = s1 + s2;        // ✅ operator+(s1, s2)
String b = s1 + "world";   // ✅ operator+(s1, String("world"))
String c = "hello" + s2;   // ✅ operator+(String("hello"), s2)
```

对于对称运算符（`+`、`-`、`*` 等），非成员版本让两边操作数地位平等，都支持隐式转换。这是C++惯用法。

### 练习题2：异常安全

下面这个 `operator=` 有什么问题？

```cpp
class Array {
    int* data;
    size_t size;
public:
    Array& operator=(const Array& other) {
        if (this != &other) {
            delete[] data;
            data = new int[other.size];
            size = other.size;
            for (size_t i = 0; i < size; ++i)
                data[i] = other.data[i];
        }
        return *this;
    }
};
```

提示：如果 `new int[other.size]` 抛异常（内存不足），会发生什么？

### 答案

**异常安全问题：如果 `new` 失败，原数据已经丢失。**

执行顺序：
1. `delete[] data;` —— 释放旧内存 ✅
2. `data = new int[other.size];` —— 假设这里抛 `std::bad_alloc`
3. 异常抛出，`operator=` 提前返回
4. `data` 现在是**悬空指针**（指向已释放的内存）！
5. 后续如果有人用 `data`，就是未定义行为
6. 更严重：析构时会 `delete[] data`，对悬空指针二次释放

**正确写法（拷贝并交换惯用法，copy-and-swap idiom）**：

```cpp
class Array {
    int* data;
    size_t size;
public:
    // 先写一个正确的拷贝构造
    Array(const Array& other) : size(other.size), data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
    }

    // 再写一个 noexcept 的 swap
    void swap(Array& other) noexcept {
        using std::swap;
        swap(data, other.data);
        swap(size, other.size);
    }

    // 拷贝赋值：异常安全 + 自赋值安全
    Array& operator=(Array other) {  // 注意：传值，不是引用！
        swap(other);                // 和临时对象交换
        return *this;
    }
    // other 在这里析构，自动释放原来的旧内存
};
```

**原理**：
- 参数 `Array other` 是传值，调用拷贝构造创建临时副本
- 如果拷贝构造抛异常（`new` 失败），异常在 `operator=` 外部发生，原对象不受影响
- `swap` 是 `noexcept`，不会抛异常
- 和临时对象交换后，`*this` 拥有新数据，临时对象 `other` 拥有旧数据
- `other` 离开作用域析构，自动释放旧内存

这个写法同时解决了：
- 自赋值安全（不需要 `if (this != &other)`）
- 异常安全（拷贝失败不影响原对象）
- 代码简洁（不需要手动 `delete` 和 `new`）


