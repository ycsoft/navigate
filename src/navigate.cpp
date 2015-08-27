﻿#include "navigate.h"

#include <assert.h>
#include <cstdio>
#include <string.h>
#include <cmath>

#define  REAL_TYPE_SIZE  (sizeof(real))

union charDouble
{
    char c[ REAL_TYPE_SIZE ];
    real db;
};

/**
 * @brief toDouble
 * 转换double型数据的字节序
 *
 * @param x
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
/**
 * @brief toBigEndian
 * 转换int型数据字节序
 * @param x
 */
void  toBigEndian(int& x)
{
    x = ( (x & 0x000000ff) << 24 ) | ( (x & 0x0000ff00) << 8)
            | ((x & 0x00ff0000) >> 8) | ( (x & 0xff000000) >> 24);
}


Navigate::Navigate()
{
    _coordsys = LeftHandSys;
}

Navigate::~Navigate()
{
    vector<Node*>::iterator ptiter= __points.begin();

    while ( ptiter != __points.end() )
    {
        delete *ptiter;
        ptiter = __points.erase(ptiter);
    }

    ClearResult();
}

list<Node*> Navigate::GetNeighbor(Node *center)
{
    vector<int> ids = center->neigbours;
    list<Node*> res;
    for ( size_t i = 0 ; i < ids.size(); i++ )
    {
        if (_id2points[ids[i]]->type != 1)
            res.push_back(_id2points[ids[i]]);
    }
    return res;

}

void Navigate::LoadPointsFile(const char *file)
{

    //二进制文件读取
    FILE *f = fopen(file,"rb");
    char buf[32] = {0};
    int floorcount = 0;

    if ( NULL == f )
        return;

    //楼层编号
    fread(buf,1,32,f);
    //楼层数
    fread(&floorcount,sizeof(int),1,f);
    toBigEndian(floorcount);
    if ( floorcount <= 0) return;

    //for 楼层
    for ( int k = 0 ; k < floorcount; ++k);
    {
        int count = 0, floor = 0;
        //楼层编号
        fread(buf,1,32,f);
        fread(&floor,sizeof(int),1,f);
        toBigEndian(floor);
        //总点数
        fread(&count,sizeof(int),1,f);
        toBigEndian(count);
        list<int>  lnd;
        _floor_binds[count] = lnd;
        //for 楼层点
        for ( int i = 0 ; i < count; ++i)
        {
            Node *nd = new Node;

            //楼层
            nd->floor = floor;
            fread(&nd->id,sizeof(int),1,f);
            toBigEndian(nd->id);

            //点类型
            fread(&nd->type,sizeof(int),1,f);
            toBigEndian(nd->type);
            fread(&nd->x,sizeof(real),1,f);
            toDouble(nd->x);

            fread(&nd->y,sizeof(real),1,f);
            toDouble(nd->y);
            //相邻路口数
            int nbcount;
            fread(&nbcount,sizeof(int),1,f);
            toBigEndian(nbcount);
            nd->neigbours.resize(nbcount);
            for ( int j = 0 ; j < nbcount; ++j)
            {
                fread(&nd->neigbours[j],sizeof(int),1,f);
                toBigEndian(nd->neigbours[j]);
            }
            _id2points[nd->id] = nd;
            __points.push_back(nd);

            if ( nd->type == Bind )
            {
                _binds.push_back(nd);
                lnd.push_back(nd->id);
                _floor_binds[nd->floor] = lnd;
            }
        }//end for 楼层点
    }//end for 楼层
    fclose(f);
}

list<Node*> Navigate::GetBestPath(Node *start, Node *end)
{
    ClearResult();
    bool    bfinded = true;
    list<Node*> result;
    //起点终点相同，不予导航
    if ( EQUAL(start->x , end->x) && EQUAL(start->y,end->y))
        return result;
    //未给定路径，不予导航
    if (__points.size() <= 0)
        return result;

    assert(start != NULL && end != NULL);

    start = GetNearPathNode(start);
    end = GetNearPathNode(end);

    _start = start;
    _end = end;

    list<Node*> neighbor = GetNeighbor(start);
    list<Node*>::iterator iter = neighbor.begin();

    for ( ; iter != neighbor.end(); ++iter)
    {
        AddToOpenList( start,*iter );
    }
    AddToCloseList( start );


    Node *nstart = start;
    //memcpy(&nstart,start,sizeof(Node));
    while ( !IsInPath(_closeList,end) )
    {
        if ( _openList.empty())
        {
            bfinded = false;
            break;
        }
        //从开放集中获取最小F值的数据点并加入封闭集中
        Node *min_nd = GetMiniFNode(nstart);
//        cout<<"Min Node:"<<min_nd->x<<" ,"<<min_nd->y<<end;
        nstart = min_nd;
        AddToCloseList(min_nd);
        RemoveFromOpenList(min_nd);

        //检查并更新该点邻域数值
        list<Node*> newneighbor = GetNeighbor(min_nd);
        list<Node*> ::iterator nbiter = newneighbor.begin();
        for ( ; nbiter != newneighbor.end(); ++nbiter )
        {
            // 分两种情况
            if ( IsInPath(_closeList,*nbiter))
                continue;
            //1: 未在开放集中,更新父节点及g值，放入开放集中
            if ( !IsInPath(_openList,*nbiter))
            {
                (*nbiter)->parent = min_nd;
                (*nbiter)->g = min_nd->g + GetGValue(min_nd,*nbiter);
                AddToOpenList(min_nd,*nbiter);
            }
            //2: 已在开放集中,发掘更优路径
            else
            {
                real g = min_nd->g + GetGValue(min_nd,*nbiter);
                if ( g < (*nbiter)->g )
                {
                    (*nbiter)->parent = min_nd;
                    (*nbiter)->g = g;
                }
            }
        }
    }

    if ( !bfinded ){

        cout<<"Can not Find"<<endl;
        return result;

    }
    else{
        cout<<"Finded and Path node cout:"<<_closeList.size()<<endl;
    }
    Node *nd = _closeList.front();
    result.push_front(end);
    while ( (*nd)!=(*start) )
    {
        result.push_front(nd->parent);
        nd = nd->parent;
    }
    //result.push_front(start);
    _closeList.clear();
    _openList.clear();
    UpdateDirect(result);
    return result;
}


//为尽可能简化程序，提高运行效率
//该方法认为要添加的点在开放集中不存在，直接添加
void Navigate::AddToOpenList(Node *parent,Node *center)
{
    Node *nd = new Node;

    memcpy(nd,center,sizeof(Node));
    nd->parent = parent;
    _openList.push_front(nd);
}

void Navigate::AddToCloseList(Node *center)
{
    Node *nd = new Node;

    memcpy(nd,center,sizeof(Node));
    nd->parent = center->parent;
    _closeList.push_front(nd);
}

void Navigate::ClearResult()
{
    list<Node*> ::iterator it1 = _openList.begin();
    while ( it1 != _openList.end())
    {
        delete *it1;
        it1 = _openList.erase(it1);
    }

    list<Node*> ::iterator it2 = _closeList.begin();
    while ( it2!= _closeList.end() )
    {
        delete *it2;
        it2 = _openList.erase(it2);
    }
}

real Navigate::GetGValue(Node *n1, Node *n2)
{
    real dx = n1->x - n2->x;
    real dy = n2->y - n1->y;

    return sqrt( dx * dx + dy * dy );
}
bool Navigate::IsInPath(list<Node *> path, Node *nd)
{
    if ( path.size() <= 0 || nd == NULL )
        return false;
    list<Node*>::iterator it = path.begin();
    for ( ; it != path.end() ; ++it )
    {
        if ( EQUAL( (*it)->x,nd->x) && EQUAL( (*it)->y,nd->y) )
            return true;
    }
    return false;
}
void Navigate::RemoveFromCloseList(Node *nd)
{
    list<Node*>::iterator iter = _closeList.begin();
    while ( iter != _closeList.end())
    {
        if ( EQUAL((*iter)->x,nd->x) && EQUAL((*iter)->y,nd->y) )
        {
            iter = _closeList.erase(iter);
        }else
            ++iter;
    }
}
void Navigate::RemoveFromOpenList(Node *nd)
{
    list<Node*>::iterator iter = _openList.begin();
    while ( iter != _openList.end())
    {
        if ( EQUAL((*iter)->x,nd->x) && EQUAL((*iter)->y,nd->y) )
        {
            iter = _openList.erase(iter);
        }else
            ++iter;
    }
}
Node* Navigate::GetMiniFNode(Node *cur)
{
    list<Node*>::iterator iter = _openList.begin();
    real minf = INVALID;
    bool flag = false;
    Node *res;

    for ( ; iter != _openList.end(); ++iter )
    {
        real g = cur->g + GetGValue(cur,*iter);
        (*iter)->h = GetGValue(*iter,_end);
        real f = g + (*iter)->h;
        if ( f < minf )
        {
            flag = true;
            minf = f;
            res = (*iter);
        }
    }

    return res;
}

//
//根据路径更新路径点的方向信息
//路径中至少需包含三个数据点
void Navigate::UpdateDirect(list<Node *> &path)
{
    if ( path.size() < 3)
        return;

    list<Node*>::iterator iter = path.begin();
    Node *nd1 = *iter;
    Node *nd2 = NULL;
    Node *nd3 = NULL;

    nd2 = *(++iter);
    nd3 = *(++iter);
    --iter;
    --iter;

    nd1->attr = WalkDirect;
    for( iter ; iter != path.end(); ++iter )
    {

        Vec v12(nd2->x - nd1->x, nd2->y - nd1->y),
            v13(nd3->x - nd1->x, nd3->y - nd1->y);

        real cross = VectorCrossMulti(&v12,&v13);
        if ( cross < 0)
        {
            nd2->attr = TurnRight;
        }else if ( cross > 0 )
        {
            nd2->attr = TurnLeft;
        }else
            nd2->attr = WalkDirect;

        if ( nd2->neighborcount <= 2)
            nd2->attr = WalkAlong;

        nd1 = nd2;
        nd2 = nd3;
        ++iter; //2
        ++iter; //3
        ++iter; //下一个要处理的点
        if ( iter == path.end())
        {
            (*(--iter))->attr = Arrive;
            break;
        }
        nd3 = *(iter);
        --iter;
        --iter;
        --iter;
    }


}

Node *Navigate::GetNearPathNode(Node *nd)
{
    real mindis = INVALID;
    Node *res = NULL;
    int i = 0, count = __points.size();
    for ( i = 0 ; i < count; ++i)
    {
        real dis = Distance(nd,__points[i]);
        if ( dis < mindis)
        {
            mindis = dis;
            res = __points[i];
        }
    }
    return res;
}

Node *Navigate::getNearestBind(Node *ndsrc)
{
    real    mindis = INVALID;
    int     floor = ndsrc->floor;
    Node    *res = NULL;

    if ( _floor_binds.find(floor) == _floor_binds.end() )
        return NULL;
    list<int>   lbs = _floor_binds[floor];

    list<int>::iterator it = lbs.begin();
    while ( it != lbs.end() )
    {
        Node *nd = GetPoint(*it);
        real dis = Distance(ndsrc,nd);
        if ( dis <= mindis)
        {
            mindis = dis;
            res = nd;
        }
        ++it;
    }

    return res;
}