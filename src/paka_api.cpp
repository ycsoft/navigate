
#include "navigate.h"
#include "paka_api.h"
#include "rssi_location.h"
#include "navigate_defines.h"
#include "location_master.h"
#include "common.h"

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

using namespace std;

static Navigate global_nav;
static RssiLocation global_wifi;
static LocationMaster global_location;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 路径规划

PointArray loadPathInfo(const char *filepath)
{
    global_nav.LoadPointsFile(filepath);
    vector<Node*> res = global_nav.GetAllPoints();
    PointArray    result;

    result.num = res.size();
    result.pts = new NavPoint[res.size()];

    vector<Node*>::iterator it = res.begin();
    int i = 0;
    while ( it != res.end() )
    {
        result.pts[i].x = (*it)->x;
        result.pts[i].y = (*it)->y;
        result.pts[i].id = (*it)->id;
        result.pts[i].attr = (*it)->attr;
        result.pts[i].type = (*it)->type;
        ++it;++i;
    }
    return result;
}

PointArray getBestPath(NavPoint *start, NavPoint *end)
{
    Node *nstart = NULL, *nend = NULL;
    //判断传入的点是否为标记点，进行必要的定位起始点工作
    if ( INVALID_ID ==  start->id)
    {
        nstart = global_nav.FindTagPoint(start,end);
    }else
    {
        nstart = global_nav.GetPoint(start->id);
    }
    nend = global_nav.GetPoint(end->id);


    int stfloor = FloorFromID(nstart->id);
    int edfloor = FloorFromID(nend->id);
    list<Node*> path;

    if ( stfloor != edfloor )
    {
        Node *dest1  = global_nav.getNearestBind(global_nav.GetPoint(nstart->id));
        Node *dest2  = global_nav.getNearestBind(global_nav.GetPoint(nend->id));
        list<Node*>  tmppath = global_nav.GetBestPath(nstart,dest1),
                     tmppath2 = global_nav.GetBestPath(dest2,nend);
        list<Node*>::iterator it = tmppath.begin();
        int  attr = (stfloor < edfloor ? 1:-1) * UpStairs;
        tmppath.back()->attr = attr;
        while ( it != tmppath.end())
        {
            path.push_back(*it);
            ++it;
        }
        it = tmppath2.begin();
        while ( it != tmppath2.end() )
        {
            path.push_back(*it);
            ++it;
        }
    }
    else
    {
        path = global_nav.GetBestPath(nstart,nend);
    }

    PointArray result;
    result.num = path.size();
    result.pts = new NavPoint[result.num];
    list<Node*>::iterator it = path.begin();
    size_t i = 0;
    while ( it != path.end() )
    {
        result.pts[i].x = (*it)->x;
        result.pts[i].y = (*it)->y;
        result.pts[i].id = (*it)->id;
        result.pts[i].attr = (*it)->attr;
        result.pts[i].type = (*it)->type;
        ++it;++i;
    }
    return result;
}

// 路径规划结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 加入了惯性导航的室内定位

bool initFloorLocationData(double scale, double nyAngle, const char *wifidatapath, const char *bledatapath)
{
    return global_location.initData(scale, nyAngle, wifidatapath, bledatapath);
}

SidPoint doLocation(double x0, double y0, double almx, double almy, double almz, double rotx, double roty, double rotz, const char *signal_ids, int sig_type, int cal_type)
{
    return global_location.do_lacation_master(x0, y0, almx, almy, almz, rotx, roty, rotz, signal_ids, (SignalType)sig_type, (LocationCalType)cal_type);
}

// 加入了惯性导航的室内定位结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 旧的WIFI文件

int loadWifiInfo(const char *filepath)
{
    return global_wifi.LoadSignalFile(filepath);
}


SidPoint doLocate(const char* bssids)
{
    string bssidstr = bssids;
    vector<string> macs = split(bssidstr, ";");
    RealTimeSignal** fingers = new RealTimeSignal*[macs.size()];
    int count = 0;
    for (size_t i = 0; i < macs.size(); ++i)
    {
        string mac = macs[i];
        vector<string> item = split(mac, ",");
        if (item.size() == 2)
        {
            RealTimeSignal* ff = new RealTimeSignal(item[0], atoi(item[1].c_str()));
            fingers[count] = ff;
            count++;
        }
    }
    string floor_code = global_wifi.LocationBuildingFloor(fingers, count);
    LPoint p = global_wifi.LocationFloorPoint_SCM_11(floor_code.c_str(), fingers, count);
    SidPoint pp;
    pp.id = p.pcode;
    pp.x = p.x;
    pp.y = p.y;
    memcpy(pp.floor_code, p.floor_code, strlen(p.floor_code));
    return pp;
}

WifiMultiPoint doLocateTest(const char* bssids)
{
    string bssidstr = bssids;
    vector<string> macs = split(bssidstr, ";");
    RealTimeSignal** fingers = new RealTimeSignal*[macs.size()];
    int count = 0;
    for (size_t i = 0; i < macs.size(); ++i)
    {
        string mac = macs[i];
        vector<string> item = split(mac, ",");
        if (item.size() == 2)
        {
            RealTimeSignal* ff = new RealTimeSignal(item[0], atoi(item[1].c_str()));
            fingers[count] = ff;
            count++;
        }
    }
    string floor_code = global_wifi.LocationBuildingFloor(fingers, count);

    // 不同的计算方式
    LPoint p1 = global_wifi.LocationFloorPoint_SCM_11(floor_code.c_str(), fingers, count);
    LPoint p2 = global_wifi.LocationFloorPoint_SCM_12(floor_code.c_str(), fingers, count);
    LPoint p3 = global_wifi.LocationFloorPoint_SCM_21(floor_code.c_str(), fingers, count);
    LPoint p4 = global_wifi.LocationFloorPoint_SCM_22(floor_code.c_str(), fingers, count);

    WifiMultiPoint ppp;
    memcpy(ppp.floor_code, p1.floor_code, strlen(p1.floor_code));

    ppp.id1 = p1.pcode;
    ppp.x1 = p1.x;
    ppp.y1 = p1.y;

    ppp.id2 = p2.pcode;
    ppp.x2 = p2.x;
    ppp.y2 = p2.y;

    ppp.id3 = p3.pcode;
    ppp.x3 = p3.x;
    ppp.y3 = p3.y;

    ppp.id4 = p4.pcode;
    ppp.x4 = p4.x;
    ppp.y4 = p4.y;

    return ppp;
}

// 旧的wifi文件结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NavPoint *GetPoint(int id)
{
    Node *nd = global_nav.GetPoint(id);
    NavPoint *pt = new NavPoint;
    pt->x = nd->x;
    pt->y = nd->y;
    pt->id = nd->id;
    pt->attr = nd->attr;
    return pt;
}

