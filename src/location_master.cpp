#include "location_master.h"

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include "common.h"

using namespace std;

// 惯性定位点和接下来的WIFI定位点, 最大的距离差, 单位米
const double kMaxDiff = 5;
// 蓝牙文件和wifi文件的文件名
const string kWifiFilename = "wfinger.f";
const string kBleFilename = "bfinger.f";

LocationMaster::LocationMaster()
{
    m_scale = 1;
    memset(&m_last_floor_code, 0, LEN_FLOOR_CODE);
    m_last_floor_number = 0;
    m_lastPoint.x = -99.0f;
    m_lastPoint.y = -99.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 初始化数据

bool LocationMaster::initData(double scale, double nyAngle, const char *wifidatapath, const char *bledatapath)
{
    m_scale = scale;
    m_guidance.m_nyAngle = nyAngle;
    // 分别加载蓝牙数据和wifi数据
    if (wifidatapath != NULL)
    {
        load_wifi_file(wifidatapath);
    }
    if (wifidatapath != NULL)
    {
        load_ble_file(bledatapath);
    }
    return true;
}

// 初始化数据结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*!
 * \brief 定位主函数, 试用与各种情况
 * \param x0 惯性导航用初始的X坐标
 * \param y0 惯性导航用初始的Y坐标
 * \param almx 线性加速度传感器值
 * \param almy 线性加速度传感器值
 * \param almz 线性加速度传感器值
 * \param rotx 方位角
 * \param roty 方位角
 * \param rotz 方位角
 * \param signal_ids 信号列表
 * \param sig_type 信号类型
 * \param cal_type 计算类型
 * \return
 */
SidPoint LocationMaster::do_lacation_master(double x0, double y0,
                                           double almx, double almy, double almz,
                                           double rotx, double roty, double rotz,
                                           const char *signal_ids, SignalType sig_type,
                                           LocationCalType cal_type)
{
    SidPoint ret;
    // 根据蓝牙信号和WIFI信号进行定位
    if (cal_type == enum_pcal_type_location || cal_type == enum_pcal_type_forcelocation)
    {
        // wifi
        if (sig_type == enum_sigtype_wifi)
        {
            ret = do_wifi_location(signal_ids, enum_simi_type_22);
        }
        // ble
        else if (sig_type == enum_sigtype_ble)
        {
            ret = do_ble_location(signal_ids, enum_simi_type_22);
        }

        // 保存楼层序号和自然楼层编号
        memcpy(&m_last_floor_code, ret.floor_code, LEN_FLOOR_CODE);
        m_last_floor_number = ret.floor_number;
        if (cal_type == enum_pcal_type_location && m_lastPoint.x > 0.0f && m_lastPoint.y > 0.0f)
        {
            // 此处的XY已经是像素
            // 点校正
            // 1. 求新旧两点的像素距离
            double dis = calTwoPointDistance(ret.x, ret.y, m_lastPoint.x, m_lastPoint.y);
            if (dis > kMaxDiff * m_scale)
            {
                // 舍弃WIFI得到的点坐标
                ret.x = m_lastPoint.x;
                ret.y = m_lastPoint.y;
            }
            else
            {
                ret.x = (ret.x + m_lastPoint.x) / 2.0f;
                ret.y = (ret.y + m_lastPoint.y) / 2.0f;
            }
        }
    }
    else // 惯性导航
    {
        // dx dy 单位米
        double dx = 0.0f;
        double dy = 0.0f;
        m_guidance.doProcess(almx, almy, almz, rotx, roty, rotz, &dx, &dy);

        // 换算成像素
        double pdx = 0.0f;
        double pdy = 0.0f;
        realDistanceToPixel(dx, dy, &pdx, &pdy);
        double xn = x0 + pdx;
        double yn = y0 + pdy;

        ret.x = xn;
        ret.y = yn;
        ret.id = 0;
        memcpy(&ret.floor_code, m_last_floor_code, LEN_FLOOR_CODE);
        ret.floor_number = m_last_floor_number;

        // 将点加入到队列中, 以作为后期粗大点剔除使用
        // 保存上一个点
        m_lastPoint.x = ret.x;
        m_lastPoint.y = ret.y;
        m_lastPoint.floor_num = m_last_floor_number;
    }

    // 在这个地方，拉到路上
    // 相似点列表为 rssi的public字段m_sptl
    SPointTemp ppp = calMinDistancePointInRoad(m_wifi_location.m_sptl, ret.x, ret.y);
    ret.x = ppp.x;
    ret.y = ppp.y;
    ret.id = ppp.pcode;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WIFI相关
//

int LocationMaster::load_wifi_file(const char *filepath)
{
    return m_wifi_location.LoadSignalFile(filepath);
}

SidPoint LocationMaster::do_wifi_location(const char *bssids, SimilarityCalType simi_type)
{
    // 处理输入的实时信号数据
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

    // 判断在哪个楼层
    FloorBasicInfo floorinfo = m_wifi_location.LocationBuildingFloor(fingers, count);

    LPoint p;
    switch (simi_type)
    {
    case enum_simi_type_11:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_11(floorinfo.floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_12:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_12(floorinfo.floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_21:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_21(floorinfo.floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_22:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_22(floorinfo.floor_code.c_str(), fingers, count);
        break;
    }
    }

    // 此处p.pcode, 是没有编号的，因为直接计算出的是XY点，此处并不存在点编号，在master层，可考虑返回一个离XY最近的点的点编号

    SidPoint pp;
    pp.id = p.pcode;
    pp.x = p.x;
    pp.y = p.y;
    pp.floor_number = p.floor_number;
    memcpy(pp.floor_code, p.floor_code, strlen(p.floor_code));
    return pp;
}

// WIFI相关
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 蓝牙相关

int LocationMaster::load_ble_file(const char *filepath)
{
    return m_ble_location.LoadSignalFile(filepath);
}

SidPoint LocationMaster::do_ble_location(const char *bleids, SimilarityCalType simi_type)
{
    SidPoint ret;
    if (simi_type == enum_simi_type_11)
    {
        int i = 0;
        i++;
    }
    // 处理输入的实时信号数据
    string bssidstr = bleids;
    return ret;
}

// 蓝牙相关结束
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void LocationMaster::realDistanceToPixel(double rdx, double rdy, double *pdx, double *pdy)
{
    // 比例尺数据是一米对应多少像素, 直接乘以上去即可
    *pdx = rdx * m_scale;
    *pdy = rdy * m_scale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 对RSSI原始的的定位结果的处理

SPointTemp LocationMaster::calMinDistancePoint(vector<SPointTemp> &points, double x, double y)
{
    SPointTemp ret;
    double dmin = -99.0f;
    for (int i = 0; i < (int)points.size(); ++i)
    {
        SPointTemp &item = points[i];
        double d = calTwoPointDistance(item.x, item.y, x, y);
        if (dmin < 0 || dmin > d)
        {
            dmin = d;
            ret = points[i];
        }
    }
    return ret;
}

// 根据距离的排序函数, 小到大
bool disCompareMinToMax(const SPointTemp &arg1, const SPointTemp &arg2)
{
    return arg1.simi < arg2.simi;
}

SPointTemp LocationMaster::calMinDistancePointInRoad(vector<SPointTemp> &points, double x0, double y0)
{
    SPointTemp ret;
    if (points.size() < 2)
    {
        ret.x = x0;
        ret.y = y0;
        return ret;
    }

    // 首先计算距离最近的三个点
    // 分别计算每个点的距离，然后进行排序
    vector<SPointTemp> vectemp;
    for (int i = 0; i < (int)points.size(); ++i)
    {
        SPointTemp ttmp;
        ttmp.x = points[i].x;
        ttmp.y = points[i].y;
        ttmp.pcode = points[i].pcode;
        // 计算距离，并且把距离临时保存在ttmp是simi字段里面
        double d = calTwoPointDistance(ttmp.x, ttmp.y, x0, y0);
        ttmp.simi = d;
        vectemp.push_back(ttmp);
    }

    // 进行到这里，listtemp里面的每个SPointTemp的simi字段，都有距离了，啊
    // 排序，选择最近三个
    sort(vectemp.begin(), vectemp.end(), disCompareMinToMax);

    // 原始点x0 y0
    // 最近的两个点
    double x1 = vectemp.at(0).x;
    double y1 = vectemp.at(0).y;
    double x2 = vectemp.at(1).x;
    double y2 = vectemp.at(1).y;

    double x = 0.0f;
    double y = 0.0f;

    // 然后把点弄在路上，三种情况
    if (x1 == x2)
    {
        x = x1;
        y = y0;
    }
    else if (y1 == y2)
    {
        x = x0;
        y = y1;
    }
    else
    {
        double k = (y2 - y1) / (x2 - x1);
        double kk = 1.0f / k;
        x = (y0 + kk * x0 - (y1 - k * x1)) / (k + kk);
        y = k * (x - x1) + y1;
    }
    ret.x = x;
    ret.y = y;

    // 把最近点的点编号放进去
    ret.pcode = vectemp.at(0).pcode;
    return ret;
}
