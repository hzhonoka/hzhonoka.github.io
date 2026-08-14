# C++进阶笔记：第13课——继承与多态

> 前面课程讲了对象的生命周期、资源管理、STL和智能指针。今天进入面向对象的核心：继承和多态——理解C++如何通过虚函数实现运行时多态。

---

## 13.1 继承的基本语法

```cpp
class Animal {
protected:
    std::string name;   // protected：子类能访问，外部不能
public:
    Animal(const std::string& n) : name(n) {}

    void speak() const {
        std::cout << name << " makes a sound\n";
    }
};

class Dog : public Animal {   // public 继承
public:
    Dog(const std::string& n) : Animal(n) {}  // 调用基类构造

    void speak() const {
        std::cout << name << " barks\n";  // 可以访问 protected 成员
    }
};
```

`public` 继承的含义：基类的 public 成员在子类还是 public，protected 还是 protected。

如果写 `class Dog : protected Animal`，基类的 public 成员在子类变成 protected。通常用 public 继承。

---

## 13.2 名字隐藏（Name Hiding）

子类定义了同名函数，会隐藏基类的所有重载版本：

```cpp
class Base {
public:
    void foo(int) {}
    void foo(double) {}
};

class Derived : public Base {
public:
    void foo(const std::string&) {}  // 隐藏了 Base::foo(int) 和 Base::foo(double)
};

Derived d;
d.foo(42);      // ❌ 编译错误！Derived 里的 foo 只接受 string
d.Base::foo(42); // ✅ 显式调用基类版本
```

**规则**：子类一旦定义同名函数，基类的重载全部不可见。除非用 `using`：

```cpp
class Derived : public Base {
public:
    using Base::foo;  // 把基类的 foo 全部引进来
    void foo(const std::string&) {}
};

Derived d;
d.foo(42);       // ✅ 现在可以了，调用 Base::foo(int)
d.foo("hello");  // ✅ 调用 Derived::foo(string)
```

---

## 13.3 虚函数与动态绑定

没有 virtual，调用的是指针的静态类型：

```cpp
Animal* p = new Dog("Buddy");
p->speak();  // 调用 Animal::speak()，输出 "Buddy makes a sound"
```

加了 virtual：

```cpp
class Animal {
public:
    virtual void speak() const {
        std::cout << name << " makes a sound\n";
    }
};

class Dog : public Animal {
public:
    void speak() const override {   // override 关键字（C++11）
        std::cout << name << " barks\n";
    }
};

Animal* p = new Dog("Buddy");
p->speak();  // 调用 Dog::speak()，输出 "Buddy barks"
```

`virtual` 告诉编译器："这个函数要到运行时查虚表，看实际对象是什么类型。"

---

## 13.4 override 和 final（C++11）

```cpp
class Base {
public:
    virtual void foo(int);
    virtual void bar();
};

class Derived : public Base {
public:
    void foo(int) override;      // ✅ 明确声明"我在重写基类虚函数"
    void foo(double) override;   // ❌ 编译错误！基类没有 virtual foo(double)
    void bar() final;            // ✅ 重写，且禁止子类再重写
};

class GrandChild : public Derived {
public:
    void bar() override;  // ❌ 编译错误！Derived::bar 标记了 final
};
```

- `override`：让编译器帮你检查——"我真的重写了基类虚函数吗？如果没对上，报错。"
- `final`：到此为止，后面的人不许再重写。

---

## 13.5 纯虚函数与抽象类

```cpp
class Shape {
public:
    virtual double area() const = 0;  // = 0 表示纯虚函数，没有实现
    virtual ~Shape() = default;
};

// Shape s;  // ❌ 编译错误！抽象类不能实例化

class Circle : public Shape {
    double radius;
public:
    Circle(double r) : radius(r) {}
    double area() const override {
        return 3.14159 * radius * radius;
    }
};
```

`= 0` 表示这个函数没有默认实现，子类必须实现，否则子类也是抽象类。

---

## 13.6 虚函数表（vtable）机制

每个有虚函数的类，编译器偷偷生成一张虚函数表（vtable），里面存着函数地址。每个对象偷偷藏一个指针 vptr，指向这张表。

```
Animal 对象: [vptr] ──→ Animal_vtable: [0: Animal::speak]
Dog 对象:    [vptr] ──→ Dog_vtable:    [0: Dog::speak]
```

调用 `p->speak()` 时：
1. 通过 `p` 找到对象
2. 通过对象找到 vptr
3. 通过 vptr 找到 vtable
4. 通过偏移量找到实际函数地址
5. 调用

这就是运行时多态的代价：一次间接寻址（比直接调用慢一点点，但通常可忽略）。

---

## 13.7 多继承与菱形继承

C++ 允许一个类继承多个基类：

```cpp
class Camera {
public:
    void takePhoto() {}
};

class Phone {
public:
    void call() {}
};

class SmartPhone : public Camera, public Phone {
    // 既有 Camera 的功能，也有 Phone 的功能
};
```

**菱形继承问题**：

```cpp
class Device {
public:
    int id;
};

class Camera : public Device { ... };
class Phone : public Device { ... };

class SmartPhone : public Camera, public Phone {
    // 现在 SmartPhone 里有两个 Device！
    // 两个 id，分别来自 Camera 和 Phone 的继承路径
};
```

```
        Device(id)
        /            Camera(id)  Phone(id)
        \        /
        SmartPhone(两个id)
```

**解决方案：虚继承**

```cpp
class Camera : virtual public Device { ... };
class Phone : virtual public Device { ... };
```

`virtual` 继承表示："Device 只保留一份，大家共享。"

```
        Device(id) ←── 只有一份
        /            Camera      Phone
        \        /
        SmartPhone
```

虚继承的代价：对象布局更复杂，访问虚基类成员需要额外间接寻址。

---

## 本课重点

1. **public 继承**：基类的 public/protected 在子类保持原访问级别
2. **名字隐藏**：子类同名函数会隐藏基类所有重载，用 `using` 引进
3. **virtual**：运行时查虚表，调用实际类型的函数
4. **override**：显式声明重写，编译器帮你检查签名匹配
5. **final**：禁止后续子类重写
6. **纯虚函数 `= 0`**：抽象类，不能实例化，强制子类实现
7. **虚函数表**：运行时多态的底层机制，每个对象一个 vptr
8. **多继承**：C++ 支持，但小心菱形继承；用 `virtual` 继承共享基类

---

## 课后练习与答案

### 练习题1：虚函数动态绑定

```cpp
class Base {
public:
    virtual void foo() { std::cout << "Base\n"; }
};

class Derived : public Base {
public:
    void foo() { std::cout << "Derived\n"; }
};

Base b;
Derived d;
Base* p1 = &b;
Base* p2 = &d;

p1->foo();  // 输出？
p2->foo();  // 输出？
```

提示：`Derived::foo` 有没有 `override` 不影响结果，但写 `override` 是好习惯。

### 答案

- `p1->foo()` 输出 `Base`
- `p2->foo()` 输出 `Derived`

分析：
- `p1` 指向 `Base` 对象 `b`，`p1->foo()` 查虚表找到 `Base::foo`
- `p2` 的静态类型是 `Base*`，但指向的是 `Derived` 对象 `d`。`foo` 是虚函数，所以运行时查虚表，找到 `Derived::foo`

**注意**：虽然 `Derived::foo` 没有写 `override`，但它确实重写了基类的虚函数（因为签名完全匹配）。`override` 关键字只是让编译器帮你做静态检查，不写不影响运行时行为，但写了更安全。

### 练习题2：非虚函数的调用

```cpp
class A {
public:
    virtual ~A() = default;
};

class B : public A {
public:
    void bar() {}
};

A* p = new B();
p->bar();  // 能编译吗？
```

提示：`bar()` 在基类里声明了吗？虚函数机制需要什么前提？

### 答案

**不能编译。**

原因：
- `p` 的静态类型是 `A*`，编译器在编译期只看 `A` 的接口
- `A` 类里没有声明 `bar()` 函数
- 所以 `p->bar()` 在编译期就报错了

**关键理解**：虚函数机制只在**基类已经声明了虚函数**的前提下工作。如果基类根本没有这个函数，编译器不会到运行时才去查——它直接拒绝编译。

**正确做法**：如果需要通过基类指针调用子类特有的方法，有两种选择：

1. **在基类里声明虚函数**：

```cpp
class A {
public:
    virtual void bar() {}  // 空实现或纯虚
    virtual ~A() = default;
};
```

2. **向下转型（dynamic_cast）**：

```cpp
if (B* bp = dynamic_cast<B*>(p)) {
    bp->bar();  // 安全调用
}
```

