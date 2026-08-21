# C++进阶笔记：第16课——IO流与文件操作

> 前面学了模板、异常、继承多态，今天讲C++的IO系统——iostream的继承体系、格式化控制、文件流和字符串流。

---

## 16.1 iostream 的继承体系

C++ 的流库是一个经典的继承结构：

```
ios_base
    └── ios
         ├── istream  ←── ifstream / istringstream / cin
         └── ostream  ←── ofstream / ostringstream / cout
              └── iostream ←── fstream / stringstream
```

- `istream`：输入流，重载了 `>>` 提取运算符
- `ostream`：输出流，重载了 `<<` 插入运算符
- `iostream`：同时继承两者，可输入可输出

`std::cin` 的类型是 `std::istream&`，`std::cout` 的类型是 `std::ostream&`。

---

## 16.2 格式化输出

```cpp
#include <iomanip>

double pi = 3.1415926535;

std::cout << std::fixed;           // 固定小数位，不用科学计数法
std::cout << std::setprecision(2); // 保留两位小数
std::cout << pi << std::endl;      // 输出：3.14

std::cout << std::setw(10) << 42 << std::endl;  // 占10个字符宽度，右对齐
std::cout << std::left << std::setw(10) << 42;  // 左对齐

std::cout << std::hex << 255 << std::endl;      // 输出：ff
std::cout << std::dec;                           // 恢复十进制
```

**注意**：`setw` 是"一次性"的，只对下一次 `<<` 有效；`fixed`、`hex`、`left` 是"持续性"的，直到你改回来。

---

## 16.3 文件流 fstream

```cpp
#include <fstream>

// 写文件
std::ofstream out("data.txt");   // 打开文件用于输出
out << "hello" << " " << 42 << std::endl;
// out 离开作用域自动关闭

// 读文件
std::ifstream in("data.txt");
if (!in) {                       // 检查是否成功打开
    std::cerr << "打开失败
";
    return;
}

std::string word;
int num;
in >> word >> num;               // 和 cin 用法一样
std::cout << word << " " << num << std::endl;
```

**打开模式**：

| 模式 | 含义 |
|:---|:---|
| `std::ios::in` | 读 |
| `std::ios::out` | 写（默认截断文件） |
| `std::ios::app` | 追加写 |
| `std::ios::binary` | 二进制模式（不转换换行符） |
| `std::ios::trunc` | 截断已有内容 |

```cpp
// 二进制追加写
std::ofstream log("app.log", std::ios::app | std::ios::binary);
```

---

## 16.4 字符串流 sstream

在内存里模拟文件操作，常用于格式转换和字符串拼接：

```cpp
#include <sstream>

// 拼接字符串
std::ostringstream oss;
oss << "score: " << 95 << ", rank: " << 3;
std::string result = oss.str();   // "score: 95, rank: 3"

// 解析字符串
std::string line = "Alice 20 95.5";
std::istringstream iss(line);
std::string name;
int age;
double score;
iss >> name >> age >> score;      // 和 cin 一样方便
```

场景：读取一行文本，然后按字段拆分。

---

## 16.5 流的状态位

每个流对象内部有四个状态位：

| 状态 | 含义 | 触发条件 |
|:---|:---|:---|
| `goodbit` | 一切正常 | 初始状态 |
| `eofbit` | 读到文件尾 | `get()` 到 EOF，`>>` 读到末尾 |
| `failbit` | 逻辑错误 | 类型不匹配（想读 int 却遇到字母） |
| `badbit` | 严重错误 | 磁盘损坏、流缓冲区崩溃 |

**检查方法**：

```cpp
std::ifstream in("data.txt");
int x;

while (in >> x) {        // 隐式转换为 bool，检查 !fail()
    std::cout << x << "
";
}

if (in.eof()) {
    std::cout << "正常读到文件尾
";
} else if (in.fail()) {
    std::cout << "数据格式错误
";
}
```

**重要**：`while (in >> x)` 是最地道的 C++ 读取循环。流对象在 `fail()` 时转为 `false`，循环自然结束。

**清空错误状态**：

```cpp
in.clear();           // 清除错误标志位
in.seekg(0);          // 回到文件开头（如果需要重读）
```

---

## 16.6 自定义类型的流操作

结合第8课的运算符重载：

```cpp
struct Point {
    double x, y;

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "(" << p.x << ", " << p.y << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Point& p) {
        char left, comma, right;
        is >> left >> p.x >> comma >> p.y >> right;
        // 实际生产代码应该检查格式
        return is;
    }
};

Point p{3.0, 4.0};
std::cout << p << std::endl;   // 输出：(3, 4)
```

---

## 16.7 二进制读写

```cpp
struct Record {
    int id;
    double score;
};

// 写
std::ofstream out("data.bin", std::ios::binary);
Record r{42, 99.5};
out.write(reinterpret_cast<const char*>(&r), sizeof(r));

// 读
std::ifstream in("data.bin", std::ios::binary);
Record r2;
in.read(reinterpret_cast<char*>(&r2), sizeof(r2));
```

**注意**：二进制读写不可移植（不同平台结构体对齐、字节序不同）。跨平台建议用序列化库（如 Protocol Buffers）。

---

## 本课重点

1. `cin`/`cout` 是 `istream&`/`ostream&`，继承体系清晰
2. `iomanip` 控制格式：`setw`（一次性）、`fixed`/`hex`（持续性）
3. `fstream` 文件读写，`ifstream`/`ofstream` 自动 RAII 关闭
4. `sstream` 内存字符串流，做格式转换和拆分很方便
5. 流状态位：`good`/`fail`/`eof`/`bad`，`while (in >> x)` 是地道写法
6. 二进制读写：用 `read`/`write`，但要注意不可移植性

---

## 课后练习与答案

### 练习题1：格式化输出

```cpp
std::cout << std::setw(5) << std::setfill('0') << 42 << std::endl;
std::cout << 7 << std::endl;
```

第一行输出什么？第二行输出什么？`setfill` 的效果是持续的吗？

### 答案

**第一行输出**：`00042`（5个字符宽度，右对齐，不足用'0'填充）

**第二行输出**：`7`（没有 setw，所以不控制宽度）

**`setfill` 是持续的**，`setw` 是一次性的。

分析：
- `std::setfill('0')` 设置了填充字符为 `'0'`，这个设置会一直生效，直到你改成别的
- `std::setw(5)` 只影响**下一个** `<<` 操作，即 `42`
- 第二行 `std::cout << 7` 没有 `setw`，所以不控制宽度，直接输出 `7`
- 如果想第二行也有填充，需要再写 `std::setw(5)`

```cpp
std::cout << std::setw(5) << std::setfill('0') << 42 << std::endl;  // 00042
std::cout << std::setw(5) << 7 << std::endl;                          // 00007
```

### 练习题2：EOF 读取循环

```cpp
std::ifstream in("data.txt");
std::string line;
while (!in.eof()) {
    std::getline(in, line);
    std::cout << line << std::endl;
}
```

这段代码有什么问题？

提示：`eof()` 在什么时候变成 `true`？如果最后一行没有换行符会怎样？

### 答案

**这段代码会多输出一行空行。**

分析 `eof()` 的时机：
- `eof()` 只有在**尝试读取但发现已经到文件末尾**时才变成 `true`
- 不是"读到文件末尾的前一行就停"

执行流程：
1. 假设文件有3行，最后一行有换行符
2. 读取第1行 → `getline` 成功，`eof()` 还是 `false`
3. 读取第2行 → `getline` 成功，`eof()` 还是 `false`
4. 读取第3行 → `getline` 成功，`eof()` 还是 `false`（因为读到换行符，还没到EOF）
5. 检查 `!in.eof()` → `true`，进入循环
6. 再次 `getline` → 发现到文件末尾了，`getline` 失败，`line` 被清空
7. 输出一个空行！
8. 检查 `!in.eof()` → `false`，退出循环

**如果最后一行没有换行符**：
- 读取最后一行时，`getline` 读到文件末尾但没有换行符，它仍然成功读取了内容
- `eof()` 在 `getline` 返回后才变成 `true`
- 但循环条件在 `getline` 之前检查，所以还是会多执行一次

**正确写法**：

```cpp
// 方法一：检查 getline 的返回值
while (std::getline(in, line)) {
    std::cout << line << std::endl;
}

// 方法二：先读再检查
while (true) {
    if (!std::getline(in, line)) break;
    std::cout << line << std::endl;
}
```

`std::getline` 返回的是 `istream&`，在 `fail()`（包括读到 EOF）时转为 `false`。所以 `while (std::getline(in, line))` 会在读取失败时自然退出，不会多输出空行。

**更深层的问题**：如果文件中间有格式错误（比如某行太长导致缓冲区问题），`getline` 可能设置 `failbit` 而不是 `eofbit`，此时 `eof()` 仍然是 `false`，循环会继续尝试读取，可能进入无限循环或重复输出。用 `while (std::getline(...))` 会在任何失败时退出，更安全。


