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
    scanf("(%d,%d)",&a.x,&a.y);
    getchar();
    scanf("(%d,%d)",&b.x,&b.y);
    getchar();
    scanf("(%d,%d)",&c.x,&c.y);
    getchar();
    scanf("(%d,%d)",&p.x,&p.y);
    getchar();
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