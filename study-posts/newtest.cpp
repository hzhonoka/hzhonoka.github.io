#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

#define int long long

struct Point
{
    int x,y;
};

int polygonArea(vector<Point>& p) {
    int n = p.size();
    int ans = 0;
    for (int i = 0; i < n; i++)
    {
        int j = (i + 1) % n;
        ans += p[i].x * p[j].y - p[j].x * p[i].y;
    }
    return abs(ans) / 2;
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    cin>>n;
    vector<Point> a(n);
    for (int i = 0; i < n; i++)
    {
        cin>>a[i].x>>a[i].y;
    }
    cout<<polygonArea(a)<<endl;
}