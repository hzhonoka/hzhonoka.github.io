> 开一个计算几何的小坑（） 当然我不是竞赛生 所以只能作为一点兴趣先学学 大佬见谅QAQ 欢迎批评指正！

---

## 1.1 诶多，计算几何是什么

**计算几何**（Computational Geometry）说人话就是：**教计算机怎么处理"形状"和"位置"的学问**。

但是反正我们不是数学系，不需要严谨证明，只需要让代码跑对就行（大概）（真的对吗啊喂！）。

那就，开始吧！

---

## 1.2 让我们从向量谈起

回顾高中初识解析几何的美好时光，对于最简单的直线，可以用斜率（和截距）来描述它。

但是斜率的计算会发生：无穷大（总不能每个地方都特判吧……）、浮点数精度误差……

而且斜率只告诉你方向的比例，没告诉你走了多远。如果你想知道"从 A 到 B 走了多少距离"，斜率帮不了。

所以在计算几何中使用向量的说。

---

## 1.3 点与向量的表示

在平面直角坐标系中，一个点拥有x轴坐标与y轴坐标两种属性，即(x, y)，因此，我们可以使用结构体来表示一个点：

```cpp
struct Point 
{
    double x,y;
    Point(double x = 0, double y = 0) : x(x),y(y){}
};
```

那么如何将其视作一个向量呢？

从原点向这个点连接一条有向线段，我们就得到了向量~

反过来，任何一个向量 v = (dx, dy)，如果我们把它固定起点在原点，那它的终点就是一个点 (dx, dy)。

在代码层面，数据结构是完全一样哒。

---

## 1.4 向量计算

利用我们高中的数学知识，我们可以得到以下代码。

p.s. 从点 A 指向点 B 的向量，记作 B - A：

```cpp
// 向量加法
Point operator + (Point a, Point b)
{
    return Point(a.x + b.x, a.y + b.y);
}

// 向量减法
Point operator - (Point a, Point b)
{
    return Point(a.x - b.x, a.y - b.y);
}

// 向量数乘
Point operator * (Point a, double p)
{
    return Point(a.x * p, a.y * p);
}

// 向量数除
Point operator / (Point a, double p)
{
    return Point(a.x / p, a.y / p);
}

// 向量模长
double len(Point a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}
```
---

## 1.5 浮点精度问题

利用计算系统基础与cpl课程的所学我们知道，浮点数double是由二进制表示的，很多小数其实不是完全精确表示。

```cpp
#include <iostream>
using namespace std;
int main() {
    double a = 0.1;
    double b = 0.2;
    if (a + b == 0.3) {
        cout << "a + b == 0.3" << endl;
    } else {
        cout << "a + b != 0.3" << endl;
        printf("a + b = %.20f\n", a + b);
    }
    return 0;
}
```

运行结果

```
a + b != 0.3
a + b = 0.30000000000000004441
```

因此，这样的误差使得我们不能作出诸如“叉积为零所以共线”这样的判断。

所以，如何解决呢……

我们可以看到在当前情形中，上述的计算误差发生在相当靠后的位数。

解决方案：**eps(epsilon)**

类似于微积分（雾），我们定义一个很小的数：

```cpp
const double eps = 1e-10;
```
然后写一个符号函数sgn：

```cpp
int sgn(double x) {
    if (fabs(x) < eps) return 0;   // 认为是 0
    return x > 0 ? 1 : -1;         // 正或负
}
```

因此，我们的浮点数比较就可以这么写

~~if (a == b)~~

if (sgn(a - b) == 0)

~~if(a < b)~~

if (sgn(a - b) < 0)

---

## 1.6 偷跑一下叉积

利用我们（剩的不多）的微积分知识，我们艰难的回忆起了叉积的定义！：

```cpp
double cross(Point a, Point b)
{
    return a.x * b.y - a.y * b.x;
}
```

从几何意义上，我们知道，cross(a, b)的绝对值就是以a，b为邻边的平行四边形面积。

并且，由于cross(a, b) = |a| · |b| · sin(θ)（其中 θ 是从向量 a 转到向量 b 所经过的角度（逆时针为正，顺时针为负）），我们可以利用叉积来判断方向：

- cross > 0：b 在 a 的逆时针方向（左转）
- cross < 0：b 在 a 的顺时针方向（右转）
- cross == 0：a 和 b 共线

---

## 1.7 练习

[轰炸(1142)](https://www.luogu.com.cn/problem/P1142)

粘贴一下

> #### P1142 题目描述
> “我该怎么办？”飞行员 klux 向你求助。
> 事实上，klux 面对的是一个很简单的问题，但是他实在太菜了。
> klux 要想轰炸某个区域内的一些地方，它们是位于平面上的一些点，但是（显然地）klux 遇到了抵抗，所以 klux 只能飞一次，而且由于飞机比较破，一但起飞就只能沿直线飞行，无法转弯。现在他想一次轰炸最多的地方。
>##### 输入格式
>第一行一个整数 $n$。
>接下来 $n$ 行，每行有一对整数，表示一个点的坐标。没有一个点会出现两次。
>##### 输出格式
>一个整数，表示一条直线能覆盖的最多的点数。
>##### 输入输出样例 #1
>##### 输入 #1
>```
>5
>1 1
>2 2
>3 3
>9 10
>10 11
>```
>##### 输出 #1
>```
>3
>```
>#### 说明/提示
>##### 数据范围
>对于全部数据，保证 $1\le n\le 700$。
>本题翻译并改编自 uva270，数据及解答由 uva 提供。
>

值得欣喜的有两点

1. 输入整数，我们有望规避掉使用eps了（好耶）
2. 数据范围很小，可以放心大胆使用暴力（雾）

假设A，B，C三点共线，我们可以得到等价条件：向量AB与向量AC共线，即叉积为0。因此，选取一个点A为起始点，再从剩下的点中选择一个点B，得到一个向量AB，再从剩下的n-2个点中找出符合AC//AB的点的个数即可，逐个遍历的最大值。

代码如下

```cpp
#include <iostream>
using namespace std;

struct Point {
    int x, y;
};

int main() {
    int n;
    cin>>n;
    Point points [1000];
    for(int i=0;i<n;i++){
        int x, y;
        cin>>x>>y;
        points[i] = {x, y};
    }
    int ans = 0;
    for (int i = 0; i < n; i++) // 取点A
    {
        for (int j = i + 1; j < n; j++) // 取点B
        {
            Point vec = {points[j].x - points[i].x, points[j].y - points[i].y}; // 向量AB
            int count = 2; // AB上本来就有A和B两个点
            for (int k = 0; k < n; k++)
            {
                if (k == i || k == j) //不能是被选择过的A，B
                {
                    continue;
                }
                Point vec2 = {points[k].x - points[i].x, points[k].y - points[i].y};
                if (vec.x * vec2.y == vec.y * vec2.x) // 叉乘=0 共线 如果AB与AC共线 说明A，B，C共线
                {
                    count++; // 在直线AB上的点+1
                }
            }
            ans = max(ans, count);
        }
    }
    if (n == 1)
    {
        cout<<1<<endl; // 特判一下n=1
    }
    else
    {
        cout<<ans<<endl;
    }
    return 0;
}
```
