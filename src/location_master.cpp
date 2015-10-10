#include "location_master.h"

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

using namespace std;

// 用于数据处理的点队列长度
const int kQueSize = 15;
// 蓝牙文件和wifi文件的文件名
const string kWifiFilename = "wfinger.f";
const string kBleFilename = "bfinger.f";

LocationMaster::LocationMaster()
{
    m_scale = 0;
    m_nyAngle = 0;
    memset(&m_last_floor_code, 0, LEN_FLOOR_CODE);
    m_last_floor_number = 0;
}

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
    if (cal_type == enum_pcal_type_location)
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
        m_last_floor_number = globePointToFloorNumber(ret.id);
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
    }
    // 将点加入到队列中, 以作为后期粗大点剔除使用

    QuePoint pp;
    pp.x = ret.x;
    pp.y = ret.y;
    pp.floor_num = m_last_floor_number;
    addPoints(pp);
    return ret;
}

bool LocationMaster::initData(double scale, double nyAngle, const char *wifidatapath, const char *bledatapath)
{
    m_scale = scale;
    m_nyAngle = nyAngle;
    // 分别加载蓝牙数据和wifi数据
    load_wifi_file(wifidatapath);
    load_ble_file(bledatapath);
    return true;
}

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
    string floor_code = m_wifi_location.LocationBuildingFloor(fingers, count);

    LPoint p;
    switch (simi_type)
    {
    case enum_simi_type_11:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_11(floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_12:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_12(floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_21:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_21(floor_code.c_str(), fingers, count);
        break;
    }
    case enum_simi_type_22:
    {
        p = m_wifi_location.LocationFloorPoint_SCM_22(floor_code.c_str(), fingers, count);
        break;
    }
    }

    SidPoint pp;
    pp.id = p.pcode; // 此处的CODE是一个全局点ID
    pp.x = p.x;
    pp.y = p.y;
    memcpy(pp.floor_code, p.floor_code, strlen(p.floor_code));

    return pp;
}

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

void LocationMaster::realDistanceToPixel(double rdx, double rdy, double *pdx, double *pdy)
{
    // 比例尺数据是一米对应多少像素, 直接乘以上去即可
    *pdx = rdx * m_scale;
    *pdy = rdy * m_scale;
}

void LocationMaster::addPoints(QuePoint p)
{
    m_quePoints.push(p);
    // 保证只有15个
    while (m_quePoints.size() > kQueSize)
    {
        m_quePoints.pop();
    }
}
