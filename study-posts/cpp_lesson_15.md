# C++进阶笔记：第15课——异常处理与noexcept

> 前面学了RAII、智能指针、模板，今天讲C++的错误处理机制——异常。理解什么时候用异常、什么时候不用，以及如何写出异常安全的代码。

---

## 15.1 错误处理的困境：错误码的痛点

C 语言风格：

```cpp
FILE* f = fopen("data.txt", "r");
if (!f) {
    return -1;  // 错误码
}

char* buffer = malloc(1000);
if (!buffer) {
    fclose(f);
    return -2;  // 另一个错误码
}

// ... 更多操作，每层都要检查错误码 ...
```

**问题**：
- 错误码容易被忽略：调用者可以假装没看见返回值
- 错误码污染逻辑：正常逻辑和错误处理混在一起，嵌套层级爆炸
- **构造函数无法返回错误码**：对象构造失败怎么办？

异常就是解决这些问题的机制。

---

## 15.2 try-catch-throw 基本语法

```cpp
double divide(double a, double b) {
    if (b == 0) {
        throw std::runtime_error("除零错误");  // 抛出异常
    }
    return a / b;
}

int main() {
    try {
        double result = divide(10, 0);
        std::cout << result << std::endl;
    } catch (const std::runtime_error& e) {   // 捕获异常
        std::cout << "捕获: " << e.what() << std::endl;
    }
}
```

**流程**：
1. `throw` 创建一个异常对象（通常是某个 `std::exception` 的派生类）
2. 当前函数立刻停止，控制权交给最近的匹配的 `catch`
3. 如果当前函数没有 `catch`，就沿着调用栈向上传播
4. 传播过程中，**栈展开（stack unwinding）**发生：局部对象自动析构

---

## 15.3 栈展开与 RAII 的完美配合

```cpp
class Guard {
    std::string name;
public:
    Guard(const std::string& n) : name(n) {
        std::cout << name << " 上锁\n";
    }
    ~Guard() {
        std::cout << name << " 解锁\n";
    }
};

void risky() {
    Guard g1("A");          // A 上锁
    Guard g2("B");          // B 上锁

    throw std::runtime_error("boom");  // 抛异常！

    Guard g3("C");          // 不会执行
}

int main() {
    try {
        risky();
    } catch (...) {         // ... 捕获所有异常
        std::cout << "异常已处理\n";
    }
}
```

输出：
```
A 上锁
B 上锁
B 解锁      ← 栈展开，先析构 g2
A 解锁      ← 再析构 g1
异常已处理
```

**关键**：即使 `risky()` 因为异常突然中断，`g2` 和 `g1` 的析构函数仍然被调用。这就是栈展开保证 RAII 资源不泄漏。

---

## 15.4 异常安全等级

C++ 标准库对函数有明确的异常安全承诺：

| 等级 | 含义 | 例子 |
|:---|:---|:---|
| 不抛保证 | 承诺绝不抛异常 | `swap`、`push_back`（有时） |
| 强保证 | 异常发生时，对象状态完全回滚到调用前 | `vector::insert`（有时） |
| 基本保证 | 异常发生时，对象处于有效但未指定状态 | 大多数标准库操作 |
| 无保证 | 异常可能导致对象腐败 | 不要这样写 |

---

## 15.5 强保证的实现：copy-and-swap

```cpp
class Buffer {
    std::unique_ptr<int[]> data;
    size_t size;
public:
    Buffer& operator=(const Buffer& other) {
        // 强保证：先在新内存上操作，成功后再交换
        auto new_data = std::make_unique<int[]>(other.size);
        std::copy(other.data.get(), other.data.get() + other.size, new_data.get());

        // 到这里，new_data 构造成功，没有抛异常
        // 现在交换，不会失败
        data = std::move(new_data);
        size = other.size;

        return *this;
    }
};
```

如果 `new int[]` 或 `copy` 抛异常，`this` 的状态完全不变。只有一切成功后，才用 `move` 更新状态（move 不抛异常）。

---

## 15.6 noexcept：承诺与优化

```cpp
void foo() noexcept;           // 承诺不抛异常
void bar() noexcept(true);     // 同上
void baz() noexcept(false);    // 可能抛异常（默认）
```

**为什么重要？**

**编译器优化**：调用 `noexcept` 函数时，编译器不需要生成异常处理的栈展开代码

**标准库契约**：`vector` 只在元素的移动构造是 `noexcept` 时，才用移动代替拷贝

```cpp
class Safe {
public:
    Safe(Safe&&) noexcept = default;  // 移动不抛异常
};

class Unsafe {
public:
    Unsafe(Unsafe&&) { /* 可能抛 */ }
};

std::vector<Safe> v1;
v1.reserve(100);
v1.push_back(Safe());
v1.push_back(Safe());  // 扩容时：Safe 的移动是 noexcept，用移动！O(1)

std::vector<Unsafe> v2;
// 扩容时：Unsafe 的移动不是 noexcept，只能用拷贝！O(n)
```

**条件 noexcept**：

```cpp
template<typename T>
void swap(T& a, T& b) noexcept(noexcept(T(std::move(a)))) {
    // 只有当 T 的移动构造不抛异常时，这个 swap 才标记 noexcept
}
```

---

## 15.7 不要滥用异常

**异常不是控制流工具**：

```cpp
// ❌ 烂代码：用异常做正常逻辑
try {
    while (true) {
        try {
            auto val = queue.pop();  // 队列空时抛异常？
            process(val);
        } catch (const EmptyException&) {
            break;  // 用异常表示"队列空"
        }
    }
}
```

正确做法：用 `optional` 或返回布尔值。

```cpp
// ✅ 好代码
while (auto val = queue.try_pop()) {  // 返回 optional
    process(*val);
}
```

**异常应该用于**：
- 真正的异常情况（文件打不开、内存不足、除零）
- 构造函数失败（无法返回错误码）
- 跨越多层调用栈的错误传播（避免每层都传错误码）

---

## 本课重点

1. **异常替代错误码**：构造函数无法返回错误码时的唯一选择
2. **栈展开**：异常传播时，局部对象自动析构，RAII 资源不泄漏
3. **异常安全等级**：不抛 > 强保证 > 基本保证 > 无保证
4. **强保证技巧**：copy-and-swap，先在新资源上操作，成功后再提交
5. **noexcept**：承诺不抛异常，让编译器优化，让标准库敢用移动
6. **不要滥用异常**：正常控制流用返回值，异常留给真正的异常

---

## 课后练习与答案

### 练习题1：vector扩容与noexcept

```cpp
std::vector<std::string> v;
v.reserve(2);
v.push_back("hello");
v.push_back("world");
v.push_back("!");  // 扩容！
```

假设 `std::string` 的移动构造是 `noexcept`，`vector` 扩容时会：移动旧元素，还是拷贝旧元素？

### 答案

**移动旧元素。**

分析：
- `std::string` 的移动构造标记了 `noexcept`
- `vector` 在扩容时检查元素类型的移动构造是否是 `noexcept`
- 如果是，用**移动**转移旧元素：旧元素的资源直接"偷"走，不需要复制字符串内容
- 如果不是，只能用**拷贝**：每个字符串都要复制一份内容

**性能差异**：
- 移动：O(1) 每个元素，只是指针交换
- 拷贝：O(L) 每个元素，L 是字符串长度

对于长字符串，这个差异是巨大的。所以自定义类的移动构造一定要标记 `noexcept`，否则标准库不敢用移动。

### 练习题2：异常安全分析

```cpp
class Stack {
    int* data;
    size_t size;
    size_t capacity;
public:
    void push(int x) {
        if (size == capacity) {
            int* new_data = new int[capacity * 2];  // 可能抛 bad_alloc
            std::copy(data, data + size, new_data);
            delete[] data;
            data = new_data;
            capacity *= 2;
        }
        data[size++] = x;
    }
};
```

这段代码是异常安全的吗？提供什么级别的保证？

### 答案

**不是异常安全的。只提供无保证级别。**

分析各种失败场景：

**场景A：`new int[capacity * 2]` 抛 `bad_alloc`**
- `new_data` 分配失败
- 异常抛出，`push` 提前返回
- `data` 仍然指向旧内存（没问题）
- `size` 没有增加（没问题）
- ✅ 对象状态不变

**场景B：`std::copy` 抛异常**
- 对于 `int` 数组，`copy` 不会抛异常（拷贝 int 不抛）
- 但如果 `Stack` 存的是复杂类型，`copy` 可能抛异常
- 假设抛异常：
  - `new_data` 已经分配了
  - 异常抛出，`push` 提前返回
  - `data` 仍然指向旧内存（没问题）
  - 但 `new_data` 泄漏了！没有 `delete[] new_data`
  - ⚠️ **内存泄漏**

**场景C：`new` 成功后，`delete[] data` 之后、`data = new_data` 之前抛异常**
- 对于 `int` 数组，这中间没有操作会抛异常
- 但如果存的是复杂类型，中间可能有异常
- 假设抛异常：
  - `data` 已经被 `delete[]` 了，变成悬空指针！
  - `new_data` 也泄漏了
  - `size` 和 `capacity` 还是旧值
  - ⚠️ **对象处于腐败状态**：`data` 是悬空指针，后续任何操作都是 UB

**正确写法（强保证）**：

```cpp
class Stack {
    std::unique_ptr<int[]> data;  // 用智能指针管理内存
    size_t size;
    size_t capacity;
public:
    void push(int x) {
        if (size == capacity) {
            auto new_data = std::make_unique<int[]>(capacity * 2);
            std::copy(data.get(), data.get() + size, new_data.get());

            // 到这里，所有可能抛异常的操作都完成了
            // 下面用 noexcept 操作提交状态
            data = std::move(new_data);  // unique_ptr 的 move 是 noexcept
            capacity *= 2;
        }
        data[size++] = x;
    }
};
```

用 `unique_ptr` 后：
- `new_data` 是 `unique_ptr`，即使抛异常也会自动释放
- `data = std::move(new_data)` 是 `noexcept`，不会失败
- 在提交状态前，所有可能抛异常的操作都完成了
- 提交状态的操作是 `noexcept`，保证不会中途失败
- ✅ **强保证**：要么全部成功，要么对象状态完全不变


