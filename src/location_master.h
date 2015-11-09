#ifndef LOCATION_MASTER_H
#define LOCATION_MASTER_H

#include <vector>

#include "location_defines.h"
#include "rssi_location.h"
#include "guidance.h"

// 定位主文件, 会采用惯性导航\wifi导航等, 得到最终的XY像素坐标

struct QuePoint
{
    double x;
    double y;
    int floor_num;
    QuePoint()
    {
        x = 0.0f;
        y = 0.0f;
        floor_num = -1;
    }
};

class LocationMaster
{

public:

    // 定位主函数
    // 对于每个楼层的定位, 都需要设置比例尺, 及地图Y轴与北边的方位角
    LocationMaster();

    // 定位主函数, 还要进行很多操作
    // x0, y0 , 为惯性的起始坐标, 对于初始的定位, 这两个值并不需要
    // 六个加速度传感器值, 用于惯性导航
    // 支持通过蓝牙和wifi就行导航定位修正, 实时采集的蓝牙数据或者wifi数据, 均通过signal_ids变量传递
    // sig_type为信号类型, 用于区分蓝牙信号或wifi信号
    // cal_type为计算类型, 用于区分是通过蓝牙或者wifi算出一个点, 还是使用惯性导航得到点
    SidPoint do_lacation_master(double x0, double y0,
                              double almx, double almy, double almz,
                              double rotx, double roty, double rotz,
                              const char* signal_ids,
                              SignalType sig_type,
                              LocationCalType cal_type);
    bool initData(double scale, double nyAngle, const char *wifidatapath, const char *bledatapath);

private:

    // 读取wifi数据文件
    int load_wifi_file(const char* filepath);

    // 通过WIFI来进行点的确定
    // simitype为相似度计算的几种方法
    SidPoint do_wifi_location(const char* bssids, SimilarityCalType simi_type);

    // 读取蓝牙数据文件
    int load_ble_file(const char* filepath);

    // 通过蓝牙来进行点定位
    // simitype为相似度计算的几种方法
    SidPoint do_ble_location(const char* bleids, SimilarityCalType simi_type);

    // 真实距离到像素
    void realDistanceToPixel(double rdx, double rdy, double *pdx, double *pdy);

    // 得到一个离XY最近原始点
    SPointTemp calMinDistancePoint(vector<SPointTemp> &points, double x, double y);

    // 根据计算得到离XY最近的一个路上的点
    SPointTemp calMinDistancePointInRoad(vector<SPointTemp> &points, double x0, double y0);

public:
    // 比例尺, 从米到像素
    // 1米对应多少像素
    double m_scale;

private:

    // 需要两个rssi location类
    // 一个用于处理蓝牙, 一个用于处理wifi
    RssiLocation m_wifi_location;
    RssiLocation m_ble_location;
    Guidance m_guidance;

    // 保存上一个点, 用与去除粗大点
    QuePoint m_lastPoint;

    // 上一次进行WIFI定位或者蓝牙定位得到的楼层编号和楼层自然序号信息
    char m_last_floor_code[LEN_FLOOR_CODE];
    int m_last_floor_number;

    // 一般來講，對於每一次rssi的定位數據，都需要跟慣導的進行結合計算
    // 但是，需要結合多少次之後，強制信任一次rssi定位
    int m_rssi_force_count;

};

#endif // LOCATION_MASTER_H
