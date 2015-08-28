#include <iostream>
#include <vector>
#include <list>


#include "../src/navigate.h"

using namespace std;


int main()
{
        Navigate nav;
        //加载地图路径点文件
        nav.LoadPointsFile("../data/mg.i2");
        //设置导航起点与终点
        Node *start = nav.GetPoint(30001); //根据ID获取点
        Node *end = nav.GetPoint(30032);
        //获取路径
        list<Node*> path  = nav.GetBestPath(start,end);
        path = nav.GetBestPath(start,end);
        list<Node*>::iterator iter = path.begin();

        cout<<"最佳路径:"<<endl;
        while ( iter != path.end())
        {
            cout<<(*iter)->id<<"    "<<GetTips((*iter)->attr)<<endl;
            ++iter;
        }
        cout<<endl;


    return 0;
}

