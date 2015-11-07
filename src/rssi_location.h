#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H

#include <map>
#include <set>
#include <vector>
#include <cstring>

#include "location_defines.h"

using namespace std;

class RssiLocation
{
public:

    RssiLocation();

    ~RssiLocation();

    // load the wifi file
    int LoadSignalFile(const char* filepath);

    // judge which floor am I
    FloorBasicInfo LocationBuildingFloor(RealTimeSignal* realdata[], int size);

    // 根据不同的方法判断在哪个点
    LPoint LocationFloorPoint_SCM_11(const char* floor_code, RealTimeSignal* realdata[], int size);
    LPoint LocationFloorPoint_SCM_12(const char* floor_code, RealTimeSignal* realdata[], int size);
    LPoint LocationFloorPoint_SCM_21(const char* floor_code, RealTimeSignal* realdata[], int size);
    LPoint LocationFloorPoint_SCM_22(const char* floor_code, RealTimeSignal* realdata[], int size);

private:

    // judge which point am I in the floor
    LPoint LocationFloorPoint(const char* floor_code, RealTimeSignal* realdata[], int size, SimilarityCalType calType);

    // 将mac地址加入到曾经出现过的点列表中
    int addMacToAppearPointCodeMapList(string mac, int point_code, map<string, list<int> > &mac_point_code_map);

    // 计算传入的wifi数据与楼层wifi列表的相似度
    int calSimilarityInBuildingFloor(RealTimeSignal* fingers[], int size, FloorSignalInfo &finfo);

    // 计算传入的wifi数据与采集点指纹的相似度
    Similarities calSimilarityInFloorGatherCode(RealTimeSignal* realdata[], int size, GatherFingerInfo &ginfo);

    // 相似度比较方法2
    inline float calSimilarity_M2(int &rssi_in, int &rssi_f);

    // 相似度比较方法3
    inline float calSimilarity_M3(int &rssi_in, int &rssi_f, int index_f);

    // 计算可能的点，缩小进行相似度比较的采集点范围
    set<int> calPossiblePoints(RealTimeSignal* realdata[], int size, map<string, list<int> > &mac_point_code_map);

    // 得到最相似点
    int getMostSimilarPointCode(RealTimeSignal* realdata[], int size,
                                map< int, GatherFingerInfo > &signal_map, set<int> &points,
                                SimilarityCalType calType);

    // 得到相似点列表，按照相似度排序
    vector<SPointTemp> getSimilarPointCodeList(RealTimeSignal* realdata[], int size,
                                               map< int, GatherFingerInfo > &signal_map,
                                               set<int> &points,
                                               SimilarityCalType calType);

    // 得到最相似点后计算XY坐标
    LPoint calFloorPointLocation(vector<SPointTemp> vecSpt);

    // 比较复杂的得到相似点后计算XY坐标
    LPoint calFloorPointLocation(RealTimeSignal* fingers[], int size, vector<SPointTemp> vecSpt,
                                 map< int, GatherFingerInfo > &signal_map,
                                 map<string, MacListItem> &max_rssi_point_map);

private:
    double calDistanceStandardDeviation(vector<CalTemp> &);

private:
    // 保存楼宇中每个楼层的wifi数据
    map<string, FloorSignalInfo> m_building_signal_info;

public:
    // 对于输入的rssi指纹，包含相似指纹的点
    vector<SPointTemp> m_sptl;

};
#endif // WIFI_LOCATION_H
