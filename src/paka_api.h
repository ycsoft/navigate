#ifndef NAVIGATELIB_H
#define NAVIGATELIB_H

typedef double real;

typedef struct _Point
{
    real    x;
    real    y;
    int     attr;
    int     id;
    int     type;
}NavPoint;

typedef struct
{
    int     num;
    NavPoint  *pts;
}PointArray;

class Navigate;

extern "C"
{
    ///
    /// 加载路径文件
    ///
    /// 返回路径点数组
    ///
    PointArray      loadPathInfo(const char *filepath);

    ///
    /// \brief getBestPath
    ///
    /// 根据给定的起点和终点进行路径规划
    /// \param start
    /// \param end
    /// \return
    ///
    ///
    PointArray getBestPath(NavPoint *start, NavPoint *end);

}


#endif // NAVIGATELIB_H
