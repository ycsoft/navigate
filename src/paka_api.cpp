#include "paka_api.h"
#include "navigate.h"
#include "wifi_location.h"
#include "navigate_defines.h"
#include "navigate_defines.h"

#include <map>
#include <list>
#include <vector>
#include <cstring>
#include <iostream>
#include <cmath>

using namespace std;

static Navigate global_nav;
static WifiLocation global_wifi;

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
    Node *nstart = global_nav.GetPoint(start->id);
    Node *nend = global_nav.GetPoint(end->id);
    int stfloor = FloorFromID(start->id);
    int edfloor = FloorFromID(start->id);
    list<Node*> path;

    if ( stfloor != edfloor )
    {
        Node *dest1  = global_nav.getNearestBind(global_nav.GetPoint(start->id));
        Node *dest2  = global_nav.getNearestBind(global_nav.GetPoint(end->id));
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

int loadWifiInfo(const char *filepath)
{
    return global_wifi.LoadWifiFile(filepath);
}


// split the string
vector<string> split(string str, string pattern)
{
     string::size_type pos;
     vector<string> result;
     str += pattern;//扩展字符串以方便操作
     size_t size=str.size();

     for(size_t i = 0; i < size; i++)
     {
         pos = str.find(pattern,i);
         if (pos < size)
         {
             std::string s=str.substr(i, pos-i);
             result.push_back(s);
             i = pos+pattern.size() - 1;
         }
     }
     return result;
}


WifiPoint doLocate(const char* bssids)
{
    string bssidstr = bssids;
    vector<string> macs = split(bssidstr, ";");
    RealTimeFinger** fingers = new RealTimeFinger*[macs.size()];
    int count = 0;
    for (size_t i = 0; i < macs.size(); ++i)
    {
        string mac = macs[i];
        vector<string> item = split(mac, ",");
        if (item.size() == 2)
        {
            RealTimeFinger* ff = new RealTimeFinger(item[0], atoi(item[1].c_str()));
            fingers[count] = ff;
            count++;
        }
    }
    string floor_code = global_wifi.LocationBuildingFloor(fingers, count);
    LPoint p = global_wifi.LocationFloorPoint_SCM_Normal(floor_code.c_str(), fingers, count);
    WifiPoint pp;
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
    RealTimeFinger** fingers = new RealTimeFinger*[macs.size()];
    int count = 0;
    for (size_t i = 0; i < macs.size(); ++i)
    {
        string mac = macs[i];
        vector<string> item = split(mac, ",");
        if (item.size() == 2)
        {
            RealTimeFinger* ff = new RealTimeFinger(item[0], atoi(item[1].c_str()));
            fingers[count] = ff;
            count++;
        }
    }
    string floor_code = global_wifi.LocationBuildingFloor(fingers, count);

    // 不同的计算方式
    LPoint p1 = global_wifi.LocationFloorPoint_SCM_Normal(floor_code.c_str(), fingers, count);
    LPoint p2 = global_wifi.LocationFloorPoint_SCM_M2(floor_code.c_str(), fingers, count);
    LPoint p3 = global_wifi.LocationFloorPoint_SCM_M3(floor_code.c_str(), fingers, count);

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

    return ppp;
}

