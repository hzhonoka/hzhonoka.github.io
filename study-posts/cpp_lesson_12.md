# C++进阶笔记：第12课——智能指针

> 前面学了RAII、拷贝控制、移动语义，今天把这些全部整合起来——用标准库的智能指针彻底告别手动内存管理。

---

## 12.1 裸指针的罪

先回忆一下 RAII 的精神：资源跟着对象走，对象死资源释放。

但裸指针完全背叛了这个精神：

```cpp
void leak() {
    int* p = new int[1000];
    if (some_error) {
        return;     // 糟糕，没 delete
    }
    // ... 更多代码 ...
    delete[] p;     // 如果前面抛异常，这里也执行不到
}
```

每条提前的 return、每个抛出的异常，都是潜在的泄漏。

智能指针就是把裸指针包进一个类，让这个类帮你 `delete`。因为类对象有析构函数，而析构函数一定会执行。

---

## 12.2 unique_ptr：独占所有权

`std::unique_ptr` 表示这块内存只属于我，别人不能碰。

```cpp
#include <memory>

std::unique_ptr<int> p1(new int(42));
// std::unique_ptr<int> p2 = p1;   // ❌ 编译错误！不能拷贝

std::unique_ptr<int> p3 = std::move(p1);  // ✅ 可以移动
// 现在 p1 是 nullptr，p3 拥有那块内存
```

**核心规则**：
- 不能拷贝（= 拷贝构造会编译失败）
- 可以移动（`std::move` 转移所有权）
- 离开作用域自动 `delete`

```cpp
void foo() {
    std::unique_ptr<int> p(new int(42));
    // 出了 foo，p 自动销毁，自动 delete
}
```

**为什么禁止拷贝？**

如果允许拷贝，两个 `unique_ptr` 指向同一块内存，析构时会 double free。禁止拷贝就是在编译期杜绝这种错误。

---

## 12.3 unique_ptr 的数组版本

```cpp
std::unique_ptr<int[]> arr(new int[100]);  // 注意 <int[]>
arr[0] = 1;                                // 支持下标访问
// 析构时自动调用 delete[]，不是 delete
```

---

## 12.4 把 unique_ptr 交给函数

```cpp
void process(std::unique_ptr<int> p) {
    std::cout << *p << std::endl;
}   // p 在这里销毁，内存释放

std::unique_ptr<int> p(new int(42));
process(std::move(p));   // 把所有权交给 process
// 现在 p 是 nullptr
```

如果函数只是看看，不想拿走所有权：

```cpp
void peek(const std::unique_ptr<int>& p) {  // 传引用，不转移
    std::cout << *p << std::endl;
}
// 或者更地道的：
void peek(const int* p) {                   // 直接传裸指针，只看不用
    std::cout << *p << std::endl;
}
```

---

## 12.5 shared_ptr：共享所有权

`std::shared_ptr` 表示这块内存大家共享，最后一个走的人关灯。

```cpp
std::shared_ptr<int> p1(new int(42));
{
    std::shared_ptr<int> p2 = p1;  // 拷贝，引用计数 +1
    std::cout << *p2;              // 42
}   // p2 销毁，引用计数 -1

// p1 还在，引用计数是 1
// p1 销毁时，引用计数变 0，自动 delete
```

**底层机制**：`shared_ptr` 内部有一个**控制块（control block）**，保存：
- 引用计数（多少个 shared_ptr 指向这块内存）
- 弱引用计数（weak_ptr 用的，后面讲）
- 删除器（可选）

```
[p1] ──→ [控制块: 引用计数=2] ←── [p2]
              ↓
           [实际对象: int(42)]
```

---

## 12.6 循环引用：shared_ptr 的噩梦

```cpp
struct Node {
    std::shared_ptr<Node> next;
};

auto a = std::make_shared<Node>();
auto b = std::make_shared<Node>();

a->next = b;   // b 的引用计数 +1
b->next = a;   // a 的引用计数 +1

// 现在 a 和 b 互相指着，引用计数都是 2
// 出了作用域，a 和 b 各销毁一次，引用计数各变 1
// 但 1 ≠ 0，所以内存永远不会释放！
// 循环引用 = 内存泄漏
```

---

## 12.7 weak_ptr：旁观者

`std::weak_ptr` 指向 `shared_ptr` 管理的对象，但**不增加引用计数**。

### 12.7.1 为什么需要 weak_ptr？

`shared_ptr` 的循环引用问题本质上是：**两个对象互相持有对方的强引用，导致谁都不敢先死**。`weak_ptr` 的设计就是打破这种僵局——它让你能"看到"对象，但不阻止对象被销毁。

想象一个场景：一个缓存系统，缓存项持有对原始数据的引用。如果缓存项用 `shared_ptr` 持有数据，那即使原始数据的使用者都退出了，缓存项还在，数据就永远不会释放。用 `weak_ptr` 则不同——缓存项只是"弱引用"数据，当所有真正的使用者都退出后，数据可以正常销毁，缓存项的 `weak_ptr` 自动变成空。

### 12.7.2 用 weak_ptr 打破循环引用

```cpp
struct Node {
    std::weak_ptr<Node> next;   // 用 weak_ptr 代替 shared_ptr
};

auto a = std::make_shared<Node>();
auto b = std::make_shared<Node>();

a->next = b;   // b 的弱引用计数 +1，强引用计数不变
b->next = a;   // a 的弱引用计数 +1，强引用计数不变

// 出了作用域，a 和 b 的强引用计数都是 1，各自销毁后变 0
// 内存正确释放！
```

**关键区别**：

| | shared_ptr | weak_ptr |
|:---|:---|:---|
| 是否增加引用计数 | ✅ 强引用计数 +1 | ❌ 不影响强引用计数 |
| 是否阻止对象销毁 | ✅ 是 | ❌ 否 |
| 能否直接解引用 | ✅ `*p` 或 `p->` | ❌ 必须先用 `lock()` 升级 |
| 适用场景 | 真正的所有权 | 观察者、缓存、打破循环引用 |

### 12.7.3 lock()：把 weak_ptr 升级成 shared_ptr

`weak_ptr` 不能直接解引用（没有 `operator*` 和 `operator->`），因为指向的对象可能已经死了。使用时必须先"升级"：

```cpp
std::weak_ptr<Node> w = a;

// w.lock() 返回一个 shared_ptr（如果对象还活着）
if (auto s = w.lock()) {
    // s 是有效的 shared_ptr，可以安全使用
    std::cout << "对象还活着\n";
    s->doSomething();   // ✅ 安全
} else {
    std::cout << "对象已经死了\n";
}
```

`w.lock()` 的行为：
- **对象还存在** → 返回一个 `shared_ptr`，强引用计数 +1，然后你就可以安全使用
- **对象已销毁** → 返回空的 `shared_ptr`（相当于 `nullptr`）

**为什么叫 "lock"？**

因为 `lock()` 的语义是"锁定对象的生命周期"——它返回的 `shared_ptr` 保证在 `s` 的作用域内对象不会突然销毁。这和多线程里的锁概念类似：你拿到了一个保证。

### 12.7.4 expired()：快速检查对象是否还活着

如果你只是想检查对象是否还存在，不需要升级：

```cpp
std::weak_ptr<Node> w = a;

if (w.expired()) {
    std::cout << "对象已经死了\n";
} else {
    std::cout << "对象还活着\n";
    // 注意：expired() 返回 true 后，对象可能立刻就死了
    // 所以如果要使用，还是要 lock()
}
```

**注意**：`expired()` 和 `lock()` 之间有一个经典的竞态条件（race condition）：

```cpp
if (!w.expired()) {          // 检查：对象还活着
    // 但在这里，另一个线程可能让最后一个 shared_ptr 销毁了
    auto s = w.lock();       // 现在 lock() 可能返回空！
}
```

**正确做法**：永远直接用 `lock()`，然后检查返回值：

```cpp
if (auto s = w.lock()) {     // 原子地检查和升级
    // 安全使用 s
}
```

### 12.7.5 控制块的生存期

`weak_ptr` 虽然不增加强引用计数，但它会增加**弱引用计数**。控制块只有在强引用计数和弱引用计数都归零时才会被释放。

```cpp
auto a = std::make_shared<Node>();
std::weak_ptr<Node> w = a;

a.reset();   // 强引用计数变 0，Node 对象销毁

// 但控制块还在！因为 w 持有弱引用
// w.expired() 返回 true
// w.lock() 返回空 shared_ptr

w.reset();   // 弱引用计数也变 0，控制块才释放
```

这意味着即使对象已经销毁，`weak_ptr` 仍然可以安全地调用 `expired()` 和 `lock()`——因为控制块还活着。

### 12.7.6 一个完整的例子：观察者模式

```cpp
class Subject;  // 前向声明

class Observer {
public:
    virtual void onNotify(Subject* s) = 0;
    virtual ~Observer() = default;
};

class Subject {
    std::vector<std::weak_ptr<Observer>> observers;  // 弱引用观察者
public:
    void addObserver(const std::shared_ptr<Observer>& o) {
        observers.push_back(o);  // 只增加弱引用计数
    }

    void notify() {
        for (auto it = observers.begin(); it != observers.end(); ) {
            if (auto sp = it->lock()) {     // 升级成 shared_ptr
                sp->onNotify(this);          // 安全调用
                ++it;
            } else {
                // 观察者已经死了，清理掉
                it = observers.erase(it);
            }
        }
    }
};
```

这个设计的优雅之处：
- 观察者可以随时被销毁，不需要通知 Subject
- Subject 不会阻止观察者的销毁（因为用的是 weak_ptr）
- Subject 在通知时自动清理已经死亡的观察者

---

## 12.8 make_unique 和 make_shared

C++14 引入 `std::make_unique`，C++11 已有 `std::make_shared`。

```cpp
// 老写法：两次分配（一次给对象，一次给控制块）
std::shared_ptr<int> p(new int(42));

// 新写法：一次分配，异常安全，更优雅
auto p = std::make_shared<int>(42);

auto arr = std::make_unique<int[]>(100);  // C++14，数组版本
```

**为什么推荐 make_*？**

**异常安全**：

```cpp
foo(std::shared_ptr<int>(new int(42)), bar());
```

如果 `bar()` 抛异常，而 `new int(42)` 已经执行但 `shared_ptr` 还没包好，内存泄漏。`make_shared` 把分配和包装原子化，避免这个问题。

**性能**：`make_shared` 把对象和控制块放在同一块内存里，减少一次分配。

---

## 12.9 自定义删除器

默认智能指针用 `delete` 或 `delete[]`，但你可以指定自己的清理方式：

```cpp
// 关闭文件描述符
auto file = std::unique_ptr<FILE, decltype(&fclose)>(
    fopen("data.txt", "r"),
    &fclose
);

// 或者用 lambda（C++11）
auto buffer = std::shared_ptr<int>(
    new int[100],
    [](int* p) { delete[] p; std::cout << "自定义删除\n"; }
);
```

---

## 本课重点

1. **unique_ptr**：独占所有权，不能拷贝只能移动，离开作用域自动释放
2. **shared_ptr**：共享所有权，引用计数归零时释放
3. **循环引用**：shared_ptr 互相指向导致泄漏，用 weak_ptr 打破
4. **weak_ptr**：不增加强引用计数，使用时用 `lock()` 升级成 shared_ptr
5. **`expired()` 只用于快速检查**，真正使用必须走 `lock()`
6. **控制块在强引用和弱引用都归零时才释放**
7. **make_unique / make_shared**：异常安全、性能更好、语法更简洁
8. **自定义删除器**：用 `unique_ptr<T, Deleter>` 或 `shared_ptr` 的构造函数

---

## 课后练习与答案

### 练习题1：weak_ptr 的 lock()

```cpp
std::shared_ptr<int> a = std::make_shared<int>(10);
std::weak_ptr<int> w = a;

a.reset();   // 显式释放 a

if (auto s = w.lock()) {
    std::cout << *s << std::endl;
} else {
    std::cout << "gone\n";
}
```

输出什么？`w.lock()` 返回的 `s` 是空还是有值？

### 答案

输出 `gone`。

分析：
1. `a.reset()` 把 `a` 置空，强引用计数从 1 变 0
2. 强引用计数归零，`int(10)` 被销毁，内存释放
3. `w` 是 weak_ptr，不影响强引用计数，所以对象正常销毁
4. `w.lock()` 检查到对象已死，返回空的 `shared_ptr`
5. `if (auto s = w.lock())` 中 `s` 是空，`bool` 转换为 `false`
6. 进入 `else` 分支，输出 `gone`

**注意**：虽然对象销毁了，但控制块还活着（因为 `w` 持有弱引用）。所以 `w.lock()` 可以安全调用，不会崩溃——它只是返回空而已。

### 练习题2：unique_ptr 的返回值优化

```cpp
std::unique_ptr<int> foo() {
    std::unique_ptr<int> p(new int(42));
    return p;   // 这里发生了什么？
}

std::unique_ptr<int> q = foo();
```

`unique_ptr` 不能拷贝，但这里为什么能编译？

### 答案

**这里发生了移动构造，不是拷贝构造。**

`foo()` 返回的 `p` 是一个局部变量，在返回时它被视为将亡值（xvalue）。编译器会匹配移动构造函数：

```cpp
std::unique_ptr(std::unique_ptr&& other) noexcept;
```

把 `p` 的资源转移给 `foo()` 的返回值（一个临时对象），然后 `q` 再用移动构造从临时对象转移过来。

实际上，现代编译器会应用 RVO/NRVO，直接把 `q` 当作 `p` 来构造，**零开销**——甚至连移动构造都不会调用。

**关键理解**：`unique_ptr` 禁止的是拷贝（两个对象同时拥有一块内存），但移动是允许的（资源从将死的对象转移给新对象）。返回值优化让这个过程完全没有性能损失。


