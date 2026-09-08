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