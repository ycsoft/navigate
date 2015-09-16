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
    string LocationBuildingFloor(RealTimeFinger* realdata[], int size);


    // 根据不同的方法判断在哪个点
    LPoint LocationFloorPoint_SCM_Normal(const char* floor_code,
                              RealTimeFinger* realdata[],
                              int size);
    LPoint LocationFloorPoint_SCM_M2(const char* floor_code,
                              RealTimeFinger* realdata[],
                              int size);
    LPoint LocationFloorPoint_SCM_M3(const char* floor_code,
                              RealTimeFinger* realdata[],
                              int size);

private:

    // judge which point am I in the floor
    LPoint LocationFloorPoint(const char* floor_code,
                              RealTimeFinger* realdata[],
                              int size,
                              SimilarityCalType calType);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the data input
    // 将mac地址加入到曾经出现过的点列表中
    int addMacToAppearPointCodeMapList(string mac, int point_code, map<string, list<int> > &mac_point_code_map);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the loc judge which floor in building
    // 计算传入的wifi数据与楼层wifi列表的相似度
    int calSimilarityInBuildingFloor(RealTimeFinger* fingers[], int size, FloorWifiInfo &finfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // about the loc judge which point in floor
    // 计算传入的wifi数据与采集点指纹的相似度
    Similarities calSimilarityInFloorGatherCode(RealTimeFinger* fingers[], int size, GatherFingerInfo &ginfo);

    // 相似度比较方法2
    inline float calSimilarity_M2(int &rssi_in, int &rssi_f);

    // 相似度比较方法3
    inline float calSimilarity_M3(int &rssi_in, int &rssi_f, int index_f);

    // 计算可能的点，缩小进行相似度比较的采集点范围
    set<int> calPossiblePoints(RealTimeFinger* fingers[], int size, map<string, list<int> > &mac_point_code_map);

    // 得到最相似点
    int getMostSimilarPointCode(RealTimeFinger* fingers[], int size,
                                map< int, GatherFingerInfo > &finger_map, set<int> &points,
                                SimilarityCalType calType);

    // 得到相似点列表，按照相似度排序
    vector<SPointTemp> getSimilarPointCodeList(RealTimeFinger* fingers[], int size,
                                               map< int, GatherFingerInfo > &finger_map,
                                               set<int> &points,
                                               SimilarityCalType calType);

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
