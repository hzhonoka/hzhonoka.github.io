## 2.1 正式从叉积说起

上一章的末尾，出于写题的必要性，粗略引入了一下叉积，尽管我们用高中知识也可以推导出向量平行的条件，~~但是这样会显得写的内容比较多（雾）~~。

那么再次把代码抄一遍吧

```cpp
double cross(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}
```

利用上章所说的几何意义，叉积的绝对值即为两个向量为邻边张出的平行四边形的面积；对于同样的两个向量为两边的三角形ABC，其面积就是平行四边形ABCD的一半

```
            ______
  /\       /     /
 /  \     /     /
/____\   /____ /   
```

因此对于三角形ABC，我们可以利用两边的向量叉积来计算其面积：

```cpp
double area(Point a, Point b, Point c) {
    // 三角形 ABC 的面积（带符号）
    return cross(b - a, c - a) / 2.0; //向量AB与向量AC的叉积
}
```

除此之外，我们还可以利用叉积来判断两个向量的方向关系


让我们把昨天的内容直接搬过来吧（）

> 并且，由于cross(a, b) = |a| · |b| · sin(θ)（其中 θ 是从向量 a 转到向量 b 所经过的角度（逆时针为正，顺时针为负）），我们可以利用叉积来判断方向：
> - cross > 0：b 在 a 的逆时针方向（左转）
> - cross < 0：b 在 a 的顺时针方向（右转）
> - cross == 0：a 和 b 共线

```
        A
       /   逆时针（左转）
    a /  
     /    b
    O --------> B
         顺时针（右转）
```

## 2.2 再来谈谈点积

既然叉积可以考察两个向量的相对旋转关系，那么我们不得不看看高中就已经熟悉的点积的意义的说。

对于向量a和b的点积，我们有：

```cpp
double dot(Point a, Point b) 
{
    return a.x * b.x + a.y * b.y;
}
```
**几何意义**

> dot(a, b) = |a| * |b| * cos(θ)

其中θ是向量a，b的夹角。

回忆我们高中学习的向量知识，我们可以知道：

| 点积结果   | 夹角             | 含义     |
| ------ | -------------- | ------ |
| `> 0`  | 锐角（0° ~ 90°）   | 方向大致相同 |
| `== 0` | 直角（90°）        | **垂直** |
| `< 0`  | 钝角（90° ~ 180°） | 方向大致相反 |

主播主播，你的点积确实很有说法，但是你的应用在哪里。

有的兄弟有的（）

给定一个点P和线段AB，我们就可以利用点积判断一下点和直线的相对关系。

```cpp
Point ap = p - a; // 向量AP
Point ab = b - a; // 向量AB

double t = dot(ap, ab);  // 点积

if (sgn(t) < 0) 
{
    // P 在 A 的"后面"，离 A 最近
} 
else if (sgn(t - dot(ab, ab)) > 0) 
{
    // P 在 B 的"前面"，离 B 最近
} 
else 
{
    // P 的投影落在线段 AB 上
}
```

## 2.3 简单对比一下

|          | 叉积 `cross`          | 点积 `dot`            |   
| :------- | :------------------ | :------------------ | 
| **公式**   | `a.x*b.y - a.y*b.x` | `a.x*b.x + a.y*b.y` |      
| **为正**   | 逆时针 / 左侧            | 夹角为锐角               |  
| **为负**   | 顺时针 / 右侧            | 夹角为钝角               |   
| **为零**   | **共线**              | **垂直**              |   
| **用途**   | 判方向、求面积             | 判夹角、求投影             |   

## 2.4 点到直线距离

当然，这个不是点到直线距离公式（）

```
        P
       /|
      / |
     /  | h
    /   |
   A -------- B
```

根据前文我们知道，三角形ABP的面积是 `abs(cross(b - a, p - a) / 2.0)`，同时，又可以利用`AB*h/2.0`来计算，因此我们得到：

> h = |cross(B-A, P-A)| / |B-A|

代码如下：

```cpp
double pointToLine(Point p, Point a, Point b) 
{
    Point v1 = b - a;      // 向量 AB
    Point v2 = p - a;      // 向量 AP
    return fabs(cross(v1, v2)) / len(v1);
}
```

## 2.5 练习

[神秘大三角(P1355)](https://www.luogu.com.cn/problem/P1355)

>### P1355 神秘大三角
>#### 题目描述
>判断一个点与已知三角形的位置关系。
>#### 输入格式
>前三行，每行一个坐标，表示该三角形的三个顶点。
>第四行，一个点的坐标，试判断该点与前三个点围成三角形的位置关系。
>所有坐标值均为整数。
>#### 输出格式
>- 若点在三角形内（不含边界），输出 $1$；
>- 若点在三角形外（不含边界），输出 $2$；
>- 若点在三角形边界上（不含顶点），输出 $3$；
>- 若点在三角形顶点上，输出 $4$。
>#### 输入输出样例 #1
>##### 输入 #1
>```
>(0,0)
>(3,0)
>(0,3)
>(1,1)
>```
>##### 输出 #1
>```
>1
>```
>#### 说明/提示
>##### 数据规模与约定
>对于 $100\%$ 数据，$0\le x_i,y_i\le 100$。
>

我们可以轻松地看出——在三角形顶点上是最好判断哒！（

其次是边界的情况。利用向量平行也可以轻松得出。

如果点在三角形内……如果考虑点到三边的距离和对应的高作比较,只要都小于……似乎可行，但是实际上并不能。

```
|\·P  P到三边的距离也小于对应的高……
| \   但是人家在三角形外头……
|__\
```
那如果我们考虑旋转关系呢？叉积的说。

```
        C
       / \
      /   \
     /  P  \
    /       \
   A -------- B
```
考虑`cross(B-A,P-A)`,`cross(A-C,P-C)`,`cross(C-B,P-B)`的**值的正负性**！

如果在内部，三个式子应该同号；如果在外部，必定存在异号；甚至还能判断边界：如果有叉积=0，说明P在边界上。

**真的这么简单的吗**

并非并非 请注意题目说的是在**三角形边界**上，然而叉积为零只能说明点在三角形某一条边的所在直线上的说。

所以我们需要写一个onsegment来额外判断：叉积为零 并且P的坐标应该在两个端点之间。

代码如下：

```cpp
#include <iostream>
#include <cmath>
using namespace std;


struct Point
{
    int x,y;
};

int cross(Point a, Point b) 
{
    return a.x * b.y - a.y * b.x;
}

int sgn(int x) 
{
    return x > 0 ? 1 : -1;         // 正或负
}

Point operator-(Point a, Point b) 
{
    return {a.x - b.x, a.y - b.y};
}

bool operator==(Point a, Point b) 
{
    return a.x == b.x && a.y == b.y;
}

bool onsegment(Point a, Point b, Point c) 
{
    return cross(b - a, c - a) == 0 && (c.x - a.x) * (c.x - b.x) <= 0 && (c.y - a.y) * (c.y - b.y) <= 0;
}

int main(){
    Point a,b,c,p;
    char ch;
    cin>>ch>>a.x>>ch>>a.y>>ch>>ch>>b.x>>ch>>b.y>>ch>>ch>>c.x>>ch>>c.y>>ch>>ch>>p.x>>ch>>p.y;
    Point ab = b - a;
    Point ca = a - c;
    Point bc = c - b;
    Point ap = p - a;
    Point bp = p - b;
    Point cp = p - c;
    if (a==p||b==p||c==p)
    {
        cout<<4<<endl;
    }
    else if (onsegment(a,b,p)||onsegment(b,c,p)||onsegment(c,a,p))
    {
        cout<<3<<endl;
    }
    else if (sgn(cross(ab,ap))==sgn(cross(ca,cp))&&sgn(cross(ca,cp))==sgn(cross(bc,bp)))
    {
        cout<<1<<endl;
    }
    
    else
    {
        cout<<2<<endl;
    }
    return 0;
}
```

然后就可以AC了

## 2.6 题外话

唔啊啊啊上学好累 一学期为什么要学这么多东西。更新慢见谅（）



