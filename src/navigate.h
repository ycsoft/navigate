#ifndef NAVIGATE_H
#define NAVIGATE_H

#include "defines.h"

#include <vector>
#include <list>
#include <map>

using namespace std;

class Navigate
{
public:
    typedef Node Vec;

    enum PtTyle {Common = 1,Endian,Bind};
    Navigate();
    ~Navigate();

    ///
    /// \brief GetBestPath
    ///
    /// 根据给定的起点和终点，为用户选择一条便捷的可行路线,
    /// 并且在适当位置提示用户左转右转等息息
    ///
    /// \param start    导航起始点
    /// \param end      导航终点
    /// \return         导航计算出的路径
    ///
    ///
    list<Node*>  GetBestPath(Node *start, Node *end);


    ///
    /// \brief LoadPointsFile
    /// 加载二进制地理信息点文件
    ///
    /// \param file 文件名
    ///
    void LoadPointsFile(const char *file);

    ///
    /// \brief GetNeighbor
    ///
    /// 从地图路径中获取当前点联通的邻域点
    /// \param center 当前点
    /// \return
    ///
    list<Node*> GetNeighbor(Node *center);

    ///
    /// \brief SetCoordSys
    /// \param v
    ///  设置坐标系：左手系-1  右手系1
    void SetCoordSys( int v )
    {
        _coordsys = v;
    }
    ///
    /// \brief GetPoint
    /// 根据点的ID查找点
    ///
    /// \param id   要查找的点的ID
    /// \return
    ///
    Node *GetPoint(int id)
    {
        return _id2points[id];
    }

    ///
    /// \brief GetAllPoints
    ///
    /// 获取所有已加载的路径点
    /// \return
    ///
    vector<Node*>   GetAllPoints()
    {
        return __points;
    }

    ///
    /// \brief getNearestBind
    /// 查找距离点nd最近的楼层连接点，
    /// 在不同楼层间导航时用到此函数
    /// \param nd
    /// \return
    ///
    Node* getNearestBind(Node *nd);

    ///
    /// \brief locateBinds
    /// 寻找从点src到目标楼层destfloor的可行连接点
    /// \param src
    /// \param destfloor
    /// \return
    ///
    //list<Node*> locateBinds(Node *src, int destfloor);
protected:
    //ClearResult
    void ClearResult();
    //将center附近的点添加至 开放/封闭 列表
    void AddToOpenList(Node *parent,Node *center);
    void AddToCloseList(Node *center);
    real GetGValue( Node * n1,Node *n2);
    bool IsInPath(list<Node*> path, Node *nd);
    //从开放集中获取F值最小的数据点
    Node *GetMiniFNode(Node *cur);
    real VectorCrossMulti(Vec *v1, Vec *v2)
    {
        real cros = ( v1->x * v2->y - v2->x * v1->y) * _coordsys;
        return cros;
    }


    real VecMol(Vec *v)
    {
        return sqrt( v->x * v->x + v->y*v->y );
    }

    real VectorAngle(Vec *v1, Vec *v2)
    {
        real beta = acos((v1->x*v2->x + v1->y*v2->y)/(VecMol(v1)*VecMol(v2)));
        beta = beta/3.1415926*180.0;

        beta = beta > 0 ? beta: (180 + beta);
        return beta;
    }

    //得到与选定点距离最近的路径点
    Node  *GetNearPathNode( Node *nd);


    //删除点
    void RemoveFromOpenList(Node *nd);
    void RemoveFromCloseList(Node*nd);
    //跟新方向信息
    void UpdateDirect(list<Node*> &path);
private:
    vector<Node*>  __points;

    //开放列表
    list<Node*>  _openList;

    //封闭列表
    list<Node*>  _closeList;

    //ID --Points对应表
    map<int,Node*> _id2points;
    map<int,list<int> >  _floor_binds;

    //楼层间的连接点
    list<Node*>  _binds;

    //起始点和终点
    Node*       _start;
    Node*       _end;

    int         _coordsys;
};

#endif // NAVIGATE_H
