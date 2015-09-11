#ifndef NAVIGATELIB_H
#define NAVIGATELIB_H

#include <stdio.h>
#include <string.h>

typedef double real;

typedef struct _Point
{
    real    x;
    real    y;
    int     attr;
    int     id;
    int     type;
    _Point() {
        x = -1.0f;
        y = -1.0f;
        id = -1;
        attr = -1;
        type = -1;
    }
}NavPoint;


#define LEN_FLOOR_CODE 32
typedef struct _Point_Wifi_ST
{
    double x;
    double y;
    int id;
    char floor_code[LEN_FLOOR_CODE];
    _Point_Wifi_ST() {
        x = -1.0f;
        y = -1.0f;
        id = -1;
        memset(floor_code, 0, LEN_FLOOR_CODE);
    }
} WifiPoint;


typedef struct
{
    int     num;
    NavPoint  *pts;
}PointArray;

class Navigate;

extern "C"
{

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
    WifiPoint doLocate(const char* bssids);

}


#endif // NAVIGATELIB_H
