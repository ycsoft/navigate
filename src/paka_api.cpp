#include "navigate.h"
#include "paka_api.h"
#include "rssi_location.h"
#include "navigate_defines.h"
#include "location_master.h"
#include "common.h"
#include "processroadpoint.hpp"

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

using namespace std;

static Navigate& global_nav = Navigate::ref();
static RssiLocation global_wifi;
static LocationMaster global_location;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 路径规划
PointArray loadPathInfo(const char *filepath)
{
    global_nav.LoadPointsFile(filepath);
    vector<Node*> res = global_nav.GetAllPoints();
    static PointArray    result;
    if ( result.num > 0 )
    {
        delete [] result.pts;
        result.num = 0;
    }
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
    Node n1,n2;
    static PointArray result;
    if ( result.num > 0)
    {
        delete [] result.pts;
    }
    if ( start->id != INVALID_ID )
    {
        start->floor = FloorFromID(start->id);
    }
    if ( end->id != INVALID_ID )
    {
        end->floor = FloorFromID(end->id);
    }
    memset(&result,0,sizeof(PointArray));
    typeTrans(n1,*start);
    typeTrans(n2,*end);
    start->floor = n1.floor;
    end->floor = n2.floor;
    //判断传入的点是否为标记点，进行必要的定位起始点工作
    ProcessRoadPoint<Node> proc(global_nav._id2points,global_nav.__points,global_nav._floor_binds);
    nstart  = proc.findNearTagPoint(&n1,&n2,ProcessRoadPoint<Node>::Start);
    nend    = proc.findNearTagPoint(&n1,&n2,ProcessRoadPoint<Node>::End);

    if ( nend == NULL || nstart == NULL)
    {
        cerr<<"导航点为空"<<endl;
        return result;
    }

    int stfloor = FloorFromID(nstart->id);
    int edfloor = FloorFromID(nend->id);
    int sign = ( edfloor > stfloor ? 1:-1);
    list<Node*> path;


    if ( stfloor != edfloor )
    {
        list<Node*> ltmp;
        Node        *dest1 = NULL;

        dest1 = proc.getNearBindPointToFloor(edfloor,nstart);
        if ( NULL == dest1)
        {
            dest1 = proc.getNearBindPointToFloor(stfloor + sign,nstart);
        }
        Node *dest2 ;
        if ( dest1 == NULL  )
        {
            return result;
        }

        list<Node*>  tmppath = global_nav.GetBestPath(nstart,dest1),
                     tmppath2 = proc.getFloorPath(edfloor,dest1);//global_nav.GetBestPath(dest2,nend);
        dest2 = tmppath2.back();
        ltmp = global_nav.GetBestPath(dest2,nend);
        listMerge(tmppath2,ltmp);
        if ( tmppath.empty() || tmppath2.empty() )
        {
            return result;
        }

        list<Node*>::iterator it = tmppath.begin();
        int  attr = (stfloor < edfloor ? 1:-1) * UpStairs;
        tmppath.back()->attr = attr;
        while ( it != tmppath.end())
        {
            path.push_back(*it);
            ++it;
        }
        if ( isInList(tmppath2.front(),tmppath) )
        {
            tmppath2.pop_front();
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

    FloorBasicInfo floorinfo = global_wifi.LocationBuildingFloor(fingers, count);

    LPoint p = global_wifi.LocationFloorPoint_SCM_11(floorinfo.floor_code.c_str(), fingers, count);
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
    FloorBasicInfo floorinfo = global_wifi.LocationBuildingFloor(fingers, count);

    // 不同的计算方式
    LPoint p1 = global_wifi.LocationFloorPoint_SCM_11(floorinfo.floor_code.c_str(), fingers, count);
    LPoint p2 = global_wifi.LocationFloorPoint_SCM_12(floorinfo.floor_code.c_str(), fingers, count);
    LPoint p3 = global_wifi.LocationFloorPoint_SCM_21(floorinfo.floor_code.c_str(), fingers, count);
    LPoint p4 = global_wifi.LocationFloorPoint_SCM_22(floorinfo.floor_code.c_str(), fingers, count);

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
    if ( NULL == nd || NULL == pt )
    {
        return NULL;
    }
    pt->x = nd->x;
    pt->y = nd->y;
    pt->id = nd->id;
    pt->attr = nd->attr;
    delete nd;
    return pt;
}

vector<int>& GetNeighor(int &id)

{
//    Node *nd = global_nav.GetPoint(id);
    return global_nav.GetNei(id);
}
