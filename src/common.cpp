
#include <cmath>
#include "common.h"

/*!
 * \brief 转换double型数据的字节序
 * \param x
 */
void toDouble(real &x)
{
    charDouble cd;
    cd.db = x;

    char c[REAL_TYPE_SIZE] = {0};
    int  tpsize = REAL_TYPE_SIZE;

    for ( int i = 0; i < tpsize; ++i)
        c[i] = cd.c[tpsize-i-1];
    x = *(real*)c;
}

/*!
 * \brief 转换int型数据字节序
 * \param x
 */
void  toBigEndian(int& x)
{
    x = ( (x & 0x000000ff) << 24 ) | ( (x & 0x0000ff00) << 8)
            | ((x & 0x00ff0000) >> 8) | ( (x & 0xff000000) >> 24);
}

/*!
 * \brief 将楼层点编号，转换为楼宇全局编号
 * \param floorCode
 * \param floorNubmer
 * \return
 */
int floorPCodeToGlobePCode(int floorCode, int floorNumber)
{
    if (floorCode < 0)
    {
        return floorCode;
    }

    if (floorNumber < 0)
    {
        return GLOBAL_FLOOR_CODE_BASE * floorNumber - floorCode;
    }
    else
    {
        return GLOBAL_FLOOR_CODE_BASE * floorNumber + floorCode;
    }
}

/*!
 * \brief 从全局点编号得到楼层的自然序号
 * \param globePointCode 全局点编号
 * \return 楼层自然序号
 */
int globePointToFloorNumber(int globePointCode)
{
    return globePointCode / GLOBAL_FLOOR_CODE_BASE;
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

// 计算两点的距离
double calTwoPointDistance(double x1, double y1, double x2, double y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
