#include <iostream>
#include <vector>
#include <list>


#include "../src/navigate.h"

using namespace std;

Navigate    nav;

void test( int id1, int id2 )
{
    cout<<"From: "<<id1<<" To: "<<id2<<endl;
    Node *start = nav.GetPoint(id1); //根据ID获取点
    Node *end = nav.GetPoint(id2);
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
}

int main()
{
        //加载地图路径点文件
        nav.LoadPointsFile("../data/mg.i2");
        //设置导航起点与终点
        test(30001,30045);
        cout<<endl;

        test(30021,30043);
        cout<<endl;

        test(30011,30025);
        cout<<endl;

        for ( int i = 0 ; i < 10; i++)
        {
            test(30085,30086);
            cout<<endl;
        }
    return 0;
}

