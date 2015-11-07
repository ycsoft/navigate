#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <list>

#include "../src/paka_api.h"
#include "../src/navigate_defines.h"
#include "../src/common.h"
using namespace std;

void test( int id1, int id2 )
{
    cout<<"From: "<<id1<<" To: "<<id2<<endl;

    NavPoint start;
    NavPoint end;

    start.id = id1;
    end.id = id2;

    PointArray ret  = getBestPath(&start, &end);

    int num = ret.num;
    NavPoint* p = ret.pts;
    for (int i = 0; i < num; ++i) {
        NavPoint e = p[i];
        cout << e.id << " " << GetTips(e.attr) << endl;
    }
    cout<<endl;
}

void test_any(int x, int y, int floornumber, int id2)
{

    NavPoint start;
    NavPoint end;

    start.x = x;
    start.y = y;
    start.floor = floornumber;
    start.id = -1;

    end.id = id2;

    PointArray ret  = getBestPath(&start, &end);

    int num = ret.num;
    NavPoint* p = ret.pts;
    for (int i = 0; i < num; ++i) {
        NavPoint e = p[i];
        cout << e.id << " " << GetTips(e.attr) << endl;
    }
    cout<<endl;
}


void test_navigate() {
    //加载地图路径点文件
    loadPathInfo("F:\\_temp\\mg_xnrf.i2");
    // test_any(800, 700, 1, 10249);
    test(30083,30010);
    cout<<endl;
    /*
    //设置导航起点与终点
    test(30001,30045);
    cout<<endl;

    test(30021,30043);
    cout<<endl;

    test(30011,30025);
    cout<<endl;

    test(30085,30086);
    cout<<endl;
*/
}


void test_wifi_location_read_file() {
   // wloc.LoadWifiFile("/home/bukp/temp/wfinger.f");
    // wloc.LoadWifiFile("F:\\_temp\\wfinger.f");
}


void test_wifi_location_jduge_which_floor() {
    /*
    RealTimeFinger *f1 = new RealTimeFinger("d0c7c06f7194", -54);
    RealTimeFinger *f2 = new RealTimeFinger("147590e10142", -64);
    RealTimeFinger *f3 = new RealTimeFinger("78d38dbcbc74", -64);
    RealTimeFinger *f4 = new RealTimeFinger("bcd17750b46e", -67);
    RealTimeFinger *f5 = new RealTimeFinger("a8574e0590c0", -69);
    RealTimeFinger *f6 = new RealTimeFinger("a8ad3dc0f998", -70);
    RealTimeFinger *f7 = new RealTimeFinger("78d38dbcbc78", -70);
    RealTimeFinger *f8 = new RealTimeFinger("8089177e8038", -72);
    RealTimeFinger *f9 = new RealTimeFinger("08107692b1a9", -73);
    RealTimeFinger *f10 = new RealTimeFinger("e4d332bdad46", -73);
    RealTimeFinger *f11 = new RealTimeFinger("ec26ca38f68e", -74);
    RealTimeFinger *f12 = new RealTimeFinger("80891724fed8", -75);
    RealTimeFinger *f13 = new RealTimeFinger("60bb0c1f38ec", -75);
    RealTimeFinger *f14 = new RealTimeFinger("d4ee070f9736", -75);
    RealTimeFinger *f15 = new RealTimeFinger("38e595b271a9", -76);

    RealTimeFinger** fingers = new RealTimeFinger*[15] {
          f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15
    };
    // string floorcode =  wloc.LocationBuildingFloor(fingers, 15);
    //wloc.LocationFloorPoint(floorcode.c_str(), fingers, 15);
    */
}

void test_guidance() {
    ifstream in("d:\\dg-5-log-in.txt");
    string line;
    double x0 = 1222.886841f;
    double y0 = 733.241088867187f;

    ofstream out("d:\\dg-5-log-out.txt");
    while(getline(in, line)) {
        // cout << line << endl;
        vector<string> s =  split(line, ",");

        double almx = atof(s[0].c_str());
        double almy = atof(s[1].c_str());
        double almz = atof(s[2].c_str());
        double rotx = atof(s[3].c_str());
        double roty = atof(s[4].c_str());
        double rotz = atof(s[5].c_str());
        SidPoint p = doLocation(x0, y0, almx, almy, almz, rotx, roty, rotz, "", -1, 2);
        cout << p.x << "  " << p.y << endl;
        out << p.x << "\t" << p.y << endl;
        x0 = p.x;
        y0 = p.y;
    }
}


int main()
{
   //test_navigate();
   //return 1;
   //test_wifi_location_read_file();
   //test_wifi_location_jduge_which_floor();
   // loadWifiInfo("F:\\_temp\\data\\wfinger_xnrf.f");
   //const char* bssids = "14759071f8b4,-44;bcd17798c642,-46;08107692b1a9,-48;20c9d0188cdc,-48;78d38dbcbc74,-55;9c216ae323a0,-55;80891724fed8,-59;78d38dbcbc78,-59;d0c7c00bbb5a,-59;54e6fc22046a,-59;202bc19f3639,-61;147590dc8346,-61;d0c7c08a440f,-79;b85510716928,-94;b85510716929,-94";
   const char* bssids = "882593775bcf,-42;88259339171e,-51;48022af67a2b,-57;1cfa68c0f916,-72;88259372ac80,-72;bcd17750b46e,-86";
   // SidPoint pp = doLocate(bssids);
   initFloorLocationData(1, 90, "F:\\_temp\\data\\wfinger_xnrf.f", 0);
   SidPoint pp = doLocation(0, 0, 0, 0, 0, 0, 0, 0, bssids, 1, 3);
   SidPoint pp2 = doLocation(pp.x, pp.y , 0.1, 0.2, 0.3, -32, 23, 43, "", 1, 2);

   // printf("%d", pp.id1);
   //test_navigate();
   return 0;
}
