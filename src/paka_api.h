#ifndef NAVIGATELIB_H
#define NAVIGATELIB_H

#include <stdio.h>
#include <string.h>

#include "location_defines.h"

#define PRIVATE



class Navigate;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 路径规划
/*!
 * \brief 加载路径文件,返回路径点数组
 * \param filepath
 * \return
 */
PointArray loadPathInfo(const char *filepath);

/*!
 * \brief 根据给定的起点和终点进行路径规划
 * \param start
 * \param end
 * \return
 */
PointArray getBestPath(NavPoint *start, NavPoint *end);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 室内定位

/*!
 * \brief 初始化楼层定位数据, 需要传入比例尺, 正北与Y夹角, 数据文件路径等
 * \param scale 比例尺, 一米对应多少像素
 * \param nyAngle 正北与Y夹角
 * \param datapath 数据文件data目录地址
 * \return
 */
bool initFloorLocationData(double scale, double nyAngle, const char *datapath);

/*!
 * \brief 加入了惯性导航的定位
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
SidPoint doLocation(double x0, double y0,
                    double almx, double almy, double almz,
                    double rotx, double roty, double rotz,
                    const char* signal_ids,
                    SignalType sig_type,
                    LocationCalType cal_type);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 仅供WIFI测试用的函数

/*!
 * \brief 加载wifi数据文件
 * \param filepath
 * \return 加载结果
 */
int loadWifiInfo(const char *filepath);

/*!
 * \brief 进行定位，返回定位结果
 * \param bssids mac地址字符串：格式为 {mac字符串}，{信号强度}；{mac字符串}，{信号强度}；
 * \return
 */
SidPoint doLocate(const char* bssids);

/*!
 * \brief doLocateTest 测试函数，使用多种方法进行相似度计算定位
 * \param bssids
 * \return
 */
WifiMultiPoint doLocateTest(const char* bssids);

/*!
 * \brief GetPoint
 * 该函数仅供内部测试使用
 * \param id
 * \return
 */
PRIVATE NavPoint *GetPoint(int id);


#endif // NAVIGATELIB_H
