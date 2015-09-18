#include <assert.h>
#include <cstdio>
#include <string.h>
#include <cmath>


#include "common.h"
#include "navigate.h"
#include "paka_api.h"
#include "navigate_defines.h"

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
    int floorcount = 0,k = 0;

    if ( NULL == f )
    {
        cerr<<"无法读取文件"<<endl;
        return;
    }

    //楼宇编号
    fread(buf,1,32,f);
    //楼层数
    fread(&floorcount,sizeof(int),1,f);
    toBigEndian(floorcount);
    if ( floorcount <= 0) { return; }

    //for 楼层
    for ( k = 0 ; k < floorcount;k++)
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
            nd->neighborcount = nbcount;
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
    if ( start == NULL || end == NULL )
    {
        return result;
    }

    //起点终点相同，不予导航
    if (start->id == end->id)
    {
        return result;
    }
    //未给定路径，不予导航
    if (__points.size() <= 0)
    {
        return result;
    }
    _start = start;
    _end = end;

//    Node *start_2 = start;//GetNearPathNode(start);
//    Node *end_2 = end;//GetNearPathNode(end);

    list<Node*>  endNeighbor = GetNeighbor(end);
    if ( endNeighbor.size() < 1)
    {
        cerr<<"目标点不可到达"<<endl;
        return result;
    }

    list<Node*> neighbor = GetNeighbor(start);
    list<Node*>::iterator iter = neighbor.begin();

    for ( ; iter != neighbor.end(); ++iter)
    {
        AddToOpenList( start,*iter );
    }
    AddToCloseList( start );
    Node *nstart = start;

    while ( !IsInPath(_closeList,end) && !IsInPath(endNeighbor,_closeList.front()))
    {
        if ( _openList.empty())
        {
            bfinded = false;
            break;
        }
        //从开放集中获取最小F值的数据点并加入封闭集中
        Node *min_nd = GetMiniFNode(nstart);
        nstart = min_nd;
        AddToCloseList(min_nd);
        RemoveFromOpenList(min_nd);
        if ( IsInPath(endNeighbor,min_nd) )
        {
            break;
        }
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
        //        cout<<"Finded and Path node cout:"<<_closeList.size()<<endl;
    }
    Node *nd = _closeList.front();
    if ( nd != end )
    {
        result.push_front(end);
    }
    while ( (*nd)!=(*start) )
    {
        result.push_front(nd);
        nd = nd->parent;
    }
    result.push_front(start);
    _closeList.clear();
    _openList.clear();
    UpdateDirect(result);
    return result;
}



void Navigate::AddToCloseList(Node *center)
{
    //    Node *nd = new Node;

    //    memcpy(nd,center,sizeof(Node));
    //    nd->parent = center->parent;
    //    _closeList.push_front(nd);
    _closeList.push_front(center);
}
//为尽可能简化程序，提高运行效率
//该方法认为要添加的点在开放集中不存在，直接添加
void Navigate::AddToOpenList(Node *parent,Node *center)
{
    //    Node *nd = new Node;

    //    memcpy(nd,center,sizeof(Node));
    //    nd->parent = parent;
    //    _openList.push_front(nd);
    center->parent = parent;
    _openList.push_front(center);
}


void Navigate::ClearResult()
{
    _openList.clear();
    _closeList.clear();
    //    list<Node*> ::iterator it1 = _openList.begin();
    //    while ( it1 != _openList.end())
    //    {
    //        delete *it1;
    //        it1 = _openList.erase(it1);
    //    }

    //    list<Node*> ::iterator it2 = _closeList.begin();
    //    while ( it2!= _closeList.end() )
    //    {
    //        delete *it2;
    //        it2 = _closeList.erase(it2);
    //    }
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
        }
        else
        {
            ++iter;
        }

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
            //          res->parent = cur;
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
    if ( path.size() == 2)
    {
        if ( FloorFromID(path.front()->id) < FloorFromID(path.back()->id))
        {
            path.front()->attr = UpStairs;
        }else if ( FloorFromID(path.front()->id) > FloorFromID(path.back()->id))
        {
            path.front()->attr = DownStairs;
        }else
        {
            path.front()->attr = CloseToU;
        }
        path.back()->attr = Arrive;
        return;
    }

    list<Node*>::iterator iter = path.begin();
    Node *nd1 = *iter;
    Node *nd2 = NULL;
    Node *nd3 = NULL;

    nd2 = *(++iter);
    nd3 = *(++iter);
    --iter;
    --iter;

    if ( FloorFromID(nd1->id) < FloorFromID(nd2->id))
    {
        nd1->attr = UpStairs;
    }else if ( FloorFromID(nd1->id) > FloorFromID(nd2->id))
    {
        nd1->attr = DownStairs;
    }else
    {
        nd1->attr = WalkDirect;
    }

    for( ; iter != path.end(); ++iter )
    {
        if ( FloorFromID(nd2->id) < FloorFromID(nd3->id))
        {
            nd2->attr = UpStairs;
        }else if ( FloorFromID(nd2->id) > FloorFromID(nd3->id) )
        {
            nd2->attr = DownStairs;
        }else
        {
            Vec v12(nd2->x - nd1->x, nd2->y - nd1->y);
            Vec v13(nd3->x - nd1->x, nd3->y - nd1->y);
            Vec v23(nd3->x - nd2->x, nd3->y - nd2->y);
            real angle = VectorAngle(&v12,&v23);
            real cross = VectorCrossMulti(&v12,&v13);

            if ( cross < 0)
            {
                if ( angle > 15 )
                {
                    nd2->attr = TurnRight;
                }else
                {
                    nd2->attr = AlongRight;
                }
            }
            else if ( cross > 0 )
            {
                if ( angle > 15 )
                {
                   nd2->attr = TurnLeft;
                }else
                {
                    nd2->attr = AlongLeft;
                }

            }
            else
            {
                nd2->attr = WalkDirect;
            }
        }

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
    Node *res = NULL;
    if ( nd->type == Endian )
    {
        return nd;
    }
    else
    {
        int nbcount = nd->neighborcount;
        real mindis = INVALID;
        for ( int i = 0 ; i < nbcount ; ++i)
        {
            Node * nb = GetPoint(nd->neigbours[i]);
            if ( nb->type != Endian )
            {
                continue;
            }
            real dis = Distance(_end,nb);
            if ( dis < mindis )
            {
                mindis = dis;
                res = nb;
            }
        }
    }

    //    real mindis = INVALID;
    //    Node *res = NULL;
    //    int i = 0, count = __points.size();
    //    for ( i = 0 ; i < count; ++i)
    //    {
    //        if (__points[i]->type == Endian)
    //        {
    //            real dis = Distance(nd,__points[i]);
    //            if ( dis < mindis)
    //            {
    //                mindis = dis;
    //                res = __points[i];
    //            }
    //        }

    //    }
    return res;
}

Node *Navigate::getNearestBind(Node *ndsrc)
{
    real    mindis = INVALID;
    int     floor = ndsrc->floor;
    Node    *res = NULL;

    if (_floor_binds.find(floor) == _floor_binds.end())
    {
        return NULL;
    }

    list<int> lbs = _floor_binds[floor];

    list<int>::iterator it = lbs.begin();
    while (it != lbs.end())
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

Node *Navigate::FindTagPoint(void *st, void *ed)
{
    NavPoint *start = (NavPoint*)st;
    NavPoint *end = (NavPoint*)ed;
    Node    *res = NULL;
    int     floor;
    vector<Node*>::iterator piter = __points.begin();
    real    mindis = INVALID;

    if ( INVALID_ID == start->id )
    {
        floor = start->floor;
        while ( piter != __points.end() )
        {
            if ( FloorFromID((*piter)->id) != floor )
            {
                ++piter;
                continue;
            }
            real dis = Distance(start,*piter) + Distance(start,end);
            if ( dis < mindis )
            {
                mindis = dis;
                res = (*piter);
            }
            ++piter;
        }

    }else
    {
        res = _id2points[start->id];
    }
    return res;
}
