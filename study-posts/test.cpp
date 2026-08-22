#include <iostream>
using namespace std;
struct point{
    int x,y;
};
int main(){
    int n; 
    cin>>n;
    point p[n];
    for (int i=0;i<n;i++){
        cin>>p[i].x>>p[i].y;
    }
    int max=0;
    for (int i=0;i<n-1;i++){
        for (int j=i+1;j<n;j++){//任取两点i，j，姑且记为I、J吧
            int cnt=2;//IJ上本就有I、J两点
            point vec1;//用点坐标代替向量IJ（此处应该有“→”）
            vec1.x=p[j].x-p[i].x;
            vec1.y=p[j].y-p[i].y;//IJ=(xj-xi,yj-yi)
            for (int h=0;h<n;h++){//枚举异于i、j的所有点
                if (h==i || h==j) continue;
                point vec2;//同样的方法计算向量IH
                vec2.x=p[h].x-p[i].x;
                vec2.y=p[h].y-p[i].y;
                if (vec1.x*vec2.y==vec1.y*vec2.x){//如果IJ与IH共线，即I、J、H三点共线
                    cnt++;//直线IJ上的点+1
                }
            }
            if (cnt>max) max=cnt;
        }
    }
    cout<<max;
    return 0;
}