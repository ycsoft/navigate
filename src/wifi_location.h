#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H

#include <map>
#include <set>
#include <vector>
#include <cstring>

#include "wifi_location_defines.h"

using namespace std;

class WifiLocation
{
public:

    WifiLocation();

    ~WifiLocation();

    // load the wifi file
    int LoadWifiFile(const char* filepath);

    // judge which floor am I
    string LocationBuildingFloor(InputFinger* fingers[], int size);

    // judge which point am I in the floor
    LPoint LocationFloorPoint(const char* floor_code, InputFinger* fingers[], int size);

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the data input
    // 将mac地址加入到曾经出现过的点列表中
    int addMacToAppearPointCodeMapList(string mac, int point_code, map<string, list<int> > &mac_point_code_map);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the loc judge which floor in building
    // 计算传入的wifi数据与楼层wifi列表的相似度
    int calSimilarityInBuildingFloor(InputFinger* fingers[], int size, FloorWifiInfo &finfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the loc judge which point in floor
    // 计算传入的wifi数据与采集点指纹的相似度
    int calSimilarityInFloorGatherCode(InputFinger* fingers[], int size, GatherFingerInfo &ginfo);

    // 计算可能的点，缩小进行相似度比较的采集点范围
    set<int> calPossiblePoints(InputFinger* fingers[], int size, map<string, list<int> > &mac_point_code_map);

    // 得到最相似点
    int getMostSimilarPointCode(InputFinger* fingers[], int size,
                                map< int, GatherFingerInfo > &finger_map, set<int> &points);

    // 得到相似点列表，按照相似度排序
    vector<SPointTemp> getSimilarPointCodeList(InputFinger* fingers[], int size,
                                               map< int, GatherFingerInfo > &finger_map, set<int> &points);

    // 得到最相似点后计算XY坐标
    LPoint calFloorPointLocation(vector<SPointTemp> vecSpt);

private:
    // 计算两点距离
    inline double calTwoPointDistance(double x1, double y1, double x2, double y2);

private:
    // 保存楼宇中每个楼层的wifi数据
    map<string, FloorWifiInfo> m_building_wifi_info;

};
#endif // WIFI_LOCATION_H
