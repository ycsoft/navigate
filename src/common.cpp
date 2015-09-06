
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
int floorPCodeToGlobePCode(int floorCode, int floorNubmer)
{
    return GLOBAL_FLOOR_CODE_BASE * floorNubmer + floorCode;
}
