# C++进阶笔记：第18课——多线程基础

> 暑假C++进阶系列的最后一课。前面学了对象模型、STL、智能指针，今天进入并发编程——理解C++11标准库提供的多线程工具。

---

## 18.1 为什么需要多线程？

现代 CPU 都是多核的，单线程只能跑在一个核上，浪费其他核。多线程让程序同时做多件事：

- 游戏：主线程渲染画面，后台线程加载资源
- 服务器：一个线程处理一个客户端连接
- 算法：把大任务拆成小块，并行计算

C++11 引入了标准线程库 `<thread>`，之前只能用平台相关的 API（Windows CreateThread / POSIX pthread）。

---

## 18.2 创建线程

```cpp
#include <thread>
#include <iostream>

void hello() {
    std::cout << "Hello from thread\n";
}

int main() {
    std::thread t(hello);   // 创建线程，执行 hello()
    t.join();               // 等待线程结束
    return 0;
}
```

`join()`：主线程阻塞在这里，直到 `t` 执行完毕。**必须 join 或 detach，否则线程对象销毁时程序终止**（`std::terminate`）。

`detach()`：让线程独立运行，主线程不再管它。适合"放出去就不管"的后台任务。

```cpp
std::thread t(hello);
t.detach();   // t 独立运行，主线程继续
// 注意：detach 后不要再访问 t
```

---

## 18.3 用 lambda 传参

```cpp
void worker(int id, int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << "Thread " << id << ": " << i << "\n";
    }
}

int main() {
    std::thread t1(worker, 1, 5);   // 参数直接跟在函数后面
    std::thread t2([](int id) {
        std::cout << "Lambda thread " << id << "\n";
    }, 2);

    t1.join();
    t2.join();
}
```

**注意**：默认参数是拷贝给线程的。如果要传引用，必须用 `std::ref`：

```cpp
int result = 0;
std::thread t([&result]() {   // 按引用捕获 lambda
    result = 42;
});
t.join();
std::cout << result;  // 42
```

---

## 18.4 数据竞争与互斥锁

多个线程同时读写同一变量，结果不可预期：

```cpp
int counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        ++counter;   // 不是原子操作！
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join(); t2.join();
    std::cout << counter;  // 大概率不是 200000！
}
```

**原因**：`++counter` 在底层是三步：读内存 → 加1 → 写回内存。两个线程可能同时读到同一个值，都加1，结果只加了1次。

`std::mutex`：互斥锁，保证同一时刻只有一个线程进入临界区。

```cpp
#include <mutex>

int counter = 0;
std::mutex mtx;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);  // 构造时加锁，析构时解锁
        ++counter;
    }
}
```

`std::lock_guard`：RAII 封装 mutex。构造时 `lock()`，析构时 `unlock()`。即使抛异常，析构也会解锁，不会死锁。

---

## 18.5 死锁：锁的噩梦

两个线程互相等待对方释放锁：

```cpp
std::mutex m1, m2;

void threadA() {
    std::lock_guard<std::mutex> lock1(m1);
    std::lock_guard<std::mutex> lock2(m2);  // 等 B 释放 m2
}

void threadB() {
    std::lock_guard<std::mutex> lock1(m2);
    std::lock_guard<std::mutex> lock2(m1);  // 等 A 释放 m1
}
```

A 拿着 m1 等 m2，B 拿着 m2 等 m1，永远等下去。

**解决方案**：`std::lock` 同时锁多个锁，避免死锁：

```cpp
void safe() {
    std::unique_lock<std::mutex> lock1(m1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(m2, std::defer_lock);
    std::lock(lock1, lock2);   // 原子地同时锁住两个，内部排序避免死锁
}
```

---

## 18.6 条件变量：线程间通信

一个线程等另一个线程的通知：

```cpp
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });  // 等 ready 变成 true
    // 被唤醒后，自动持有锁
    std::cout << "开始工作\n";
}

void boss() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();  // 唤醒一个等待的线程
}

int main() {
    std::thread t(worker);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    boss();
    t.join();
}
```

**wait 的陷阱**：必须放在 while 里检查条件，防止虚假唤醒。

```cpp
while (!ready) {   // 必须用 while，不能用 if！
    cv.wait(lock);
}
```

---

## 18.7 原子操作：无锁的极致性能

如果只是一个整数加减，用 `std::atomic` 比 mutex 快得多：

```cpp
#include <atomic>

std::atomic<int> counter{0};

void increment() {
    for (int i = 0; i < 100000; ++i) {
        ++counter;   // 原子自增，线程安全，无锁实现
    }
}
```

`std::atomic` 底层用 CPU 的原子指令（如 x86 的 `LOCK INC`），不需要操作系统介入，比 mutex 快几十倍。

---

## 18.8 async：更高级的异步

```cpp
#include <future>

int heavy_computation(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

int main() {
    // 启动异步任务
    std::future<int> result = std::async(std::launch::async, heavy_computation, 5);

    // 主线程继续做其他事...

    int val = result.get();  // 阻塞等待结果，返回 25
}
```

`std::async` 自动管理线程，返回 `std::future` 获取结果。比手动 thread + mutex 更简洁。

---

## 本课重点

1. **std::thread**：创建线程，必须 `join()` 或 `detach()`
2. **std::mutex + std::lock_guard**：RAII 加锁，防止数据竞争
3. **死锁**：多个锁嵌套时可能发生，用 `std::lock` 同时加锁避免
4. **std::condition_variable**：线程等待/通知机制，注意虚假唤醒
5. **std::atomic**：无锁原子操作，适合简单计数器
6. **std::async / std::future**：高级异步接口，自动管理线程

---

## 课后练习与答案

### 练习题1：std::lock 与死锁

```cpp
std::mutex m1, m2;

void f1() {
    std::lock(m1, m2);
    std::lock_guard<std::mutex> a(m1, std::adopt_lock);
    std::lock_guard<std::mutex> b(m2, std::adopt_lock);
}

void f2() {
    std::lock(m2, m1);
    std::lock_guard<std::mutex> a(m2, std::adopt_lock);
    std::lock_guard<std::mutex> b(m1, std::adopt_lock);
}
```

`f1` 和 `f2` 会死锁吗？为什么？

### 答案

**不会死锁。**

`std::lock(m1, m2)` 和 `std::lock(m2, m1)` 不会死锁，因为 `std::lock` 的内部实现会**对所有锁进行全局排序**，然后按统一顺序加锁。

具体机制：
- `std::lock` 接收多个锁，内部用某种排序算法（比如按锁的地址排序）确定加锁顺序
- 无论调用者传入锁的顺序是什么，`std::lock` 都会按统一顺序逐个加锁
- 所以即使 `f1` 传 `(m1, m2)`，`f2` 传 `(m2, m1)`，内部实际加锁顺序是一样的

**`std::adopt_lock` 的含义**：

`std::lock_guard` 的构造函数默认会调用 `mutex.lock()`。但 `std::lock` 已经帮我们锁好了，所以用 `std::adopt_lock` 告诉 `lock_guard`："这个锁已经被锁住了，你直接接管所有权，析构时负责解锁就行。"

**对比**：如果不使用 `std::lock`，直接嵌套 `lock_guard`：

```cpp
void bad() {
    std::lock_guard<std::mutex> a(m1);  // 锁 m1
    std::lock_guard<std::mutex> b(m2);  // 锁 m2
}
```

这种情况下，如果另一个线程以相反顺序加锁，就会死锁。`std::lock` 的价值就在于它内部排序，消除了这种风险。

### 练习题2：atomic 的 memory_order

```cpp
std::atomic<int> a{0};

void foo() {
    for (int i = 0; i < 1000; ++i) {
        a.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::thread t1(foo);
    std::thread t2(foo);
    t1.join(); t2.join();
    std::cout << a << std::endl;
}
```

输出什么？`fetch_add` 和普通的 `++a` 有什么区别？`memory_order_relaxed` 是什么意思？

### 答案

**输出 2000。**

分析：
- `fetch_add` 是原子操作，保证每次自增是不可分割的
- 两个线程各自增 1000 次，最终结果是 2000
- 即使使用 `memory_order_relaxed`，`fetch_add` 本身的原子性仍然保证

**`fetch_add` vs `++a` 的区别**：

| | `++a` | `a.fetch_add(1)` |
|:---|:---|:---|
| 返回值 | 自增后的值 | 自增**前**的值 |
| 灵活性 | 无 | 可以指定 memory_order |
| 底层 | 调用 `fetch_add(1)` 然后返回新值 | 直接原子加，返回旧值 |

```cpp
int x = ++a;           // x = a.fetch_add(1) + 1，返回新值
int y = a.fetch_add(1); // y 是自增前的值
```

**`memory_order_relaxed` 的含义**：

C++ 内存模型有六种 memory order，从最强约束到最弱约束：

| memory_order | 含义 | 性能 |
|:---|:---|:---|
| `seq_cst` | 顺序一致性，所有线程看到的操作顺序一致 | 最慢 |
| `acquire` | 获取语义，用于读操作 | 中等 |
| `release` | 释放语义，用于写操作 | 中等 |
| `acq_rel` | acquire + release，用于读改写 | 中等 |
| `relaxed` | 无同步约束，只保证原子性 | 最快 |
| `consume` | 类似 acquire，但更弱（很少用） | 快 |

`memory_order_relaxed` 只保证操作的原子性（不会被其他线程看到中间状态），但不保证操作之间的顺序对其他线程可见。

在这个具体例子里：
- 只是简单计数，不需要和其他变量同步
- `relaxed` 就够了，性能最好
- 如果 `foo()` 里除了 `fetch_add` 还有其他共享变量的读写，可能需要更强的 memory order

```cpp
// 需要同步的场景：flag 和 data 的读写顺序必须对其他线程可见
std::atomic<int> data{0};
std::atomic<bool> flag{false};

// 线程1写
void writer() {
    data.store(42, std::memory_order_relaxed);
    flag.store(true, std::memory_order_release);  // release：之前的写对 acquire 可见
}

// 线程2读
void reader() {
    while (!flag.load(std::memory_order_acquire));  // acquire：看到 release 之前的所有写
    std::cout << data.load(std::memory_order_relaxed);  // 保证看到 42
}
```

