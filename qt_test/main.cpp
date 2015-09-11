#include <iostream>
#include <vector>
#include <cstring>
#include <list>


#include "../src/navigate.h"
#include "../src/wifi_location.h"

#include "../src/paka_api.h"

using namespace std;

Navigate nav;
WifiLocation wloc;

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

void test_navigate() {
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
}


void test_wifi_location_read_file() {
    wloc.LoadWifiFile("/home/bukp/temp/wfinger.f");
    // wloc.LoadWifiFile("F:\\_temp\\wfinger.f");
}


void test_wifi_location_jduge_which_floor() {
    InputFinger *f1 = new InputFinger("d0c7c06f7194", -54);
    InputFinger *f2 = new InputFinger("147590e10142", -64);
    InputFinger *f3 = new InputFinger("78d38dbcbc74", -64);
    InputFinger *f4 = new InputFinger("bcd17750b46e", -67);
    InputFinger *f5 = new InputFinger("a8574e0590c0", -69);
    InputFinger *f6 = new InputFinger("a8ad3dc0f998", -70);
    InputFinger *f7 = new InputFinger("78d38dbcbc78", -70);
    InputFinger *f8 = new InputFinger("8089177e8038", -72);
    InputFinger *f9 = new InputFinger("08107692b1a9", -73);
    InputFinger *f10 = new InputFinger("e4d332bdad46", -73);
    InputFinger *f11 = new InputFinger("ec26ca38f68e", -74);
    InputFinger *f12 = new InputFinger("80891724fed8", -75);
    InputFinger *f13 = new InputFinger("60bb0c1f38ec", -75);
    InputFinger *f14 = new InputFinger("d4ee070f9736", -75);
    InputFinger *f15 = new InputFinger("38e595b271a9", -76);

    InputFinger** fingers = new InputFinger*[15] {
          f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15
    };
    string floorcode =  wloc.LocationBuildingFloor(fingers, 15);
    wloc.LocationFloorPoint(floorcode.c_str(), fingers, 15);
}


int main()
{
    //test_wifi_location_read_file();
    //test_wifi_location_jduge_which_floor();
    loadWifiInfo("F:\\_temp\\wfinger.f");
    const char* bssids = "14759071f8b4,-44;bcd17798c642,-46;08107692b1a9,-48;20c9d0188cdc,-48;78d38dbcbc74,-55;9c216ae323a0,-55;80891724fed8,-59;78d38dbcbc78,-59;d0c7c00bbb5a,-59;54e6fc22046a,-59;202bc19f3639,-61;147590dc8346,-61;d0c7c08a440f,-79;b85510716928,-94;b85510716929,-94";
    WifiPoint pp = doLocate(bssids);

    return 0;
}

