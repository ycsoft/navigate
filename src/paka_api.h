#ifndef NAVIGATELIB_H
#define NAVIGATELIB_H

#include <stdio.h>
#include <string.h>

#define     PRIVATE

typedef double real;

typedef struct _Point
{
    real    x;
    real    y;
    int     attr;
    int     id;
    int     type;
    int     floor;
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

typedef struct _Multi_Point_Wifi_ST
{
    char floor_code[LEN_FLOOR_CODE];
    int id1;
    double x1;
    double y1;
    int id2;
    double x2;
    double y2;
    int id3;
    double x3;
    double y3;
    _Multi_Point_Wifi_ST() {
        memset(this, 0, sizeof(_Multi_Point_Wifi_ST));
    }
} WifiMultiPoint;


typedef struct
{
    int       num;
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
    PRIVATE NavPoint    *GetPoint(int id);

}


#endif // NAVIGATELIB_H
