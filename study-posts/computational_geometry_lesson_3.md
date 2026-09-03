## 3.1 为什么要看线段相交

据说是计算几何里**最经典、最基础的算法**

而且推而广之到游戏里的碰撞判断，地图导航的路径规划，都是需要线段相交判定的啦。

而且——后面学的凸包、半平面交、扫描线，都要调用它。

总之开始！

## 3.2 研究直线的表示

高中的解析几何教过我们，直线的斜截式方程标准形式是 `y = kx + b` ,但是我们第一节课就提到了斜率无穷的特判问题和除法的精度问题，所以直接套用并不现实。

那么怎么办呢？

平面上直线如何确定？——两点确定一条直线！

那么类比向量的做法，我们用两个点也可以表示一条直线！

从而我们有：

> 一条直线/线段，我们就用两个端点 A 和 B 表示。需要方向向量时，现场算 B - A。

## 3.3 跨立实验（Two Crosses Test）

受过高中解析几何虐待的我们显然写腻了直线相交的问题，所以我们现在在计算几何中研究它。

### 问题

给定两条线段 `AB` 和 `CD` ，判断它们是否相交。

### 思路

假设AB和CD相交，显然交点在内部

什么意思呢？就是说A和B分居线段CD的两侧。同理，C和D分居线段AB的两侧。

```
          C         
A------B   \          如图，AB和CD不相交，且AB位于CD同侧。
            \         因此，上述两个条件同时成立才能确定相交。
             D
```

在两侧怎么判断呢……嘶……两侧，某种意义上就是相对线段的旋转方向相反……你是……不能忘记的

对啦！**叉积**！

```cpp
double c1 = cross(B - A, C - A);  // C 在 AB 的哪一侧？
double c2 = cross(B - A, D - A);  // D 在 AB 的哪一侧？
```
如果c1和c2符号相反，那也就说明C和D在AB的异侧。同理判断AB即可。

最终的判断相交的代码如下：

```cpp
//规范相交
if (sgn(c1) * sgn(c2) < 0 && sgn(c3) * sgn(c4) < 0)
    return true;
```

p.s. 规范相交的定义是：交点严格在两条线段的内部，不包括端点。 

在规范相交中，我们不考虑边界上的情况。所以只要判断乘积<0即可。至于边界等特殊情况，我们统一纳入下一节考虑。毕竟我们其实都不喜欢过多的分类讨论~

## 3.4 非规范相交（边界情况）
解析几何中，直线有斜率不存在，和圆锥曲线会有相切，临界情况比比皆是，这里也不例外。

```
A------C------D------B  共线

A------C------B  交点是端点
        \
         \
          D         
```

饿啊，好恶心。

### 处理非规范相交

既然两种情况都有端点在线上的情况，我们不妨构造一个辅助函数onSegment，来判断点P是否在线段AB上。前提是我们用叉积知道了P在直线AB上。

```cpp
bool onSegment(Point a, Point b, Point p) 
{
    return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x)
        && min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}
```

诶，不是直线吗，为什么不能只判断x或者只判断y的范围是否合法嘛。

```
          A
          |
          |
C-------D B
```

垂直：请输入文本。如图所示，CDB共线。如果我们只判断B的纵坐标，显然是合规的。然而B的横坐标并不落在CD上。

所以不要在这些地方偷懒啦。

## 3.5 完整模板

把规范相交和非规范相交合起来：


```cpp
bool intersect(Point a1, Point a2, Point b1, Point b2) {
    Point a = a2 - a1;  // 向量 AB
    Point b = b2 - b1;  // 向量 CD
    
    double c1 = cross(a, b1 - a1);  // C 在 AB 的哪一侧
    double c2 = cross(a, b2 - a1);  // D 在 AB 的哪一侧
    double c3 = cross(b, a1 - b1);  // A 在 CD 的哪一侧
    double c4 = cross(b, a2 - b1);  // B 在 CD 的哪一侧
    
    // 规范相交：互相跨立
    if (sgn(c1) * sgn(c2) < 0 && sgn(c3) * sgn(c4) < 0)
        return true;
    
    // 非规范相交：某个端点落在另一条线段上
    if (sgn(c1) == 0 && onSegment(a1, a2, b1)) return true;
    if (sgn(c2) == 0 && onSegment(a1, a2, b2)) return true;
    if (sgn(c3) == 0 && onSegment(b1, b2, a1)) return true;
    if (sgn(c4) == 0 && onSegment(b1, b2, a2)) return true;
    
    return false;
}
```

## 3.6 练习

[P5428 [USACO19OPEN] Cow Steeplechase II S](https://www.luogu.com.cn/problem/P5428)

>### P5428 [USACO19OPEN] Cow Steeplechase II S
>#### 题目描述
>在过去，Farmer John 曾经构思了许多新式奶牛运动项目的点子，其中就包括奶牛障碍赛，是奶牛们在赛道上跑越障碍栏架的竞速项目。他之前对推广这项运动做出的努力结果喜忧参半，所以他希望在他的农场上建造一个更大的奶牛障碍赛的场地，试着让这项运动更加普及。
>Farmer John 为新场地精心设计了 $ N $ 个障碍栏架，编号为 $ 1 \ldots 
 N $ （ $ 2 \leq N \leq 10^5 $ ），每一个栏架都可以用这一场地的二维地图中的一条线段来表示。这些线段本应两两不相交，包括端点位置。
>不幸的是，Farmer John 在绘制场地地图的时候不够仔细，现在发现线段之间出现了交点。然而，他同时注意到只要移除一条线段，这张地图就可以恢复到预期没有相交线段的状态（包括端点位置）。
>请求出 Farmer John 为了恢复没有线段相交这一属性所需要从他的计划中删去的一条线段。如果有多条线段移除后均可满足条件，请输出在输入中出现最早的线段的序号。
>#### 输入格式
>输入的第一行包含 $ N $ 。余下 $ N $ 行每行用四个整数 $ x_1,y_1,x_2,y_2 $ 表示一条线段，均为至多 $ 10^9 $ 的非负整数。这条线段的端点为 $ (x_1,y_1) $ 和 $ (x_2,y_2) $ 。所有线段的端点各不相同。
>#### 输出格式
>输出在输入中出现最早的移除之后可以使得余下线段各不相交的线段序号。
>#### 输入输出样例 #1
>##### 输入 #1
>```
>4
>2 1 6 1
>4 0 1 5
>5 6 5 5
>2 7 1 3
>```
>##### 输出 #1
>```
>2
>```
>#### 说明/提示
>注意：由于线段端点坐标数值的大小，在这个问题中你可能需要考虑整数类型溢出的情况。

题意其实很简单，因为只要删除一条线段就可以让所有线段不相交，因此我们只要找到一对相交的线段即可。这一对线段中必然有一条就是所求的线段（若否，假设所求线段为EF，则删去EF后这两条线段仍然相交，矛盾！因此所求线段必然是其中之一）

那么到底是哪一条呢……我们记两条线段是AB，CD，且AB编号更小。

如果CD还和AB以外的线段GH相交，那么所求为CD（否则，删除AB，则CD和GH仍然相交，矛盾！）;否则，所求为AB。

于是我们按照标号从小到大暴力枚举第一组相交的线段。然后暴力枚举后一条线段是否还和其他线段有交点，代码如下：（注意 最后输出的编号从1开始，而我们读入的数组是0开始的，因此记得+1）

```cpp
#include <iostream>
#include <cmath>
using namespace std;

#define int long long

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
    if(x == 0) return 0;
    return x > 0 ? 1 : -1;
}

Point operator-(Point a, Point b) 
{
    return {a.x - b.x, a.y - b.y};
}

bool onSegment(Point a, Point b, Point p) 
{
    return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x)
        && min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}

bool intersect(Point a1, Point a2, Point b1, Point b2) {
    Point a = a2 - a1;
    Point b = b2 - b1;
    
    int c1 = cross(a, b1 - a1);
    int c2 = cross(a, b2 - a1);
    int c3 = cross(b, a1 - b1);
    int c4 = cross(b, a2 - b1);
    
    if (sgn(c1) * sgn(c2) < 0 && sgn(c3) * sgn(c4) < 0)
        return true;
    
    if (sgn(c1) == 0 && onSegment(a1, a2, b1)) return true;
    if (sgn(c2) == 0 && onSegment(a1, a2, b2)) return true;
    if (sgn(c3) == 0 && onSegment(b1, b2, a1)) return true;
    if (sgn(c4) == 0 && onSegment(b1, b2, a2)) return true;
    
    return false;
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    cin>>n;
    Point a[n],b[n];
    for (size_t i = 0; i < n; i++)
    {
        cin>>a[i].x>>a[i].y>>b[i].x>>b[i].y;
    }
    bool tag = 1;
    int ans1 = 0,ans2 = 0;
    int ans = 0;
    for (size_t i = 0; i < n && tag; i++)
    {
        for (size_t j = i + 1; j < n; j++)
        {
            if (intersect(a[i], b[i], a[j], b[j]))
            {
                ans1 = i;
                ans2 = j;
                tag = 0;
                break;
            }
            
        }
        
    }
    ans = ans1;
    for (int i = 0; i < n; i++)
    {
        if (i == ans2 || i == ans1)
        {
            continue;
        }
        if (intersect(a[ans2], b[ans2], a[i], b[i]))
        {
            ans = ans2;
            break;
        }
    }
    ans++;
    cout<<ans<<endl;
    return 0;
}
```

那么这段代码可以AC吗！

并不能，O(n*n)对于n=10^5显然TLE了，但是我们至少通过了将近一半的测试点，至少我们的总体逻辑是没有问题的，线段相交的判定也写的很好！

唯一美中不足的就是找出第一组相交线段的时间复杂度实在是太！大！了！有没有不暴力一点的做法呢。

p.s. 如果只是为了掌握线段相交的判定，其实到这里已经足够啦！

其实搜索相交线段的过程可以优化到NlogN，不过需要后续的扫描线算法就是了。等到学到那边的时候再callback一下www。
