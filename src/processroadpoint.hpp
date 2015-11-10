#ifndef PROCESSROADPOINT_H
#define PROCESSROADPOINT_H


#include "navigate_defines.h"
#include "navigate.h"
#include "location_defines.h"
#include "common.h"

#include <map>
#include <list>
#include <iostream>
using namespace std;

/**
 *使用模板做数据点的处理，防止日后对数据结构进行修改
 *而造成程序适配复杂
 */

template<typename ElemType>
class ProcessRoadPoint
{
    friend class Navigate;

public:
    //寻找点的依据，离起点近 or 离终点近
    enum  FindType{Start, End};
    explicit ProcessRoadPoint( map<int,ElemType*> &data ,vector<ElemType*> &pts,map<int,list<int> >  &floor_binds)
        :__points(pts),_id2points(data),_floor_binds(floor_binds)
    {

    }

    explicit ProcessRoadPoint(ProcessRoadPoint &p)
    {
        __points = p.__points;
        _id2points = p._id2points;
        _floor_binds = p._floor_binds;
    }

    ProcessRoadPoint& operator = ( ProcessRoadPoint &p)
    {
        __points = p.__points;
        _id2points = p._id2points;
        _floor_binds = p._floor_binds;
        return *this;
    }
    void    sortMiddlePoint(ElemType *start,list<ElemType*> &midpts)
    {
        if ( midpts.empty())
        {
            return;
        }
        list<ElemType*>::iterator it = midpts.begin();
        //ToDo:解决路径中间点顺序问题
    }

    list<ElemType*> Process(list<ElemType*> &result)
    {
        list<ElemType*> finalResult;
        typename list<ElemType*>::iterator riter = result.begin();
        ++riter;
        finalResult.push_back(*result.begin());

        while ( riter != result.end())
        {
            ElemType *cur = *riter;
            ElemType *pre = *(--riter);
            //
            //扫描所有点，邻域点中同时包含pre和cur的点被添加进路径中
            //
            list<ElemType>   midpts;
            typename vector<ElemType*> ::iterator viter = __points.begin();
            while( viter != __points.end() )
            {
                if ( isInVector(pre->id,(*viter)->neigbours)
                     && isInVector(cur->id,(*viter)->neigbours)
                     &&(*viter)->type == PtTyle_Common)
                {
                    ElemType *nd = (*viter);
                    nd->g = Distance(nd,pre);
                    midpts.push_back(*(_id2points[(*viter)->id]));
                }
                ++viter;
            }

            if ( midpts.size() > 0)
            {
                midpts.sort();
                typename list<ElemType>::iterator it = midpts.begin();
                while( it != midpts.end())
                {
                    finalResult.push_back(_id2points[(*it).id]);
                    ++it;
                }
            }

            finalResult.push_back(cur);
            ++riter;
            ++riter;
        }
        return finalResult;
    }

    /**
     * @brief findNearTagPoint
     * 在起点、终点间查询可作为导航起始点的路口点
     *
     * @param start
     * @param end
     * @param tp
     * @return
     */
    ElemType *findNearTagPoint(ElemType *start,ElemType *end,FindType tp)
    {

        ElemType *res = NULL;
        int floor;
        typename vector<ElemType*>::iterator piter = __points.begin();
        real mindis = INVALID;
        ElemType *dest = (tp == Start? start : end);
        bool    flag = (INVALID_ID == dest->id);

        if ( flag )
        {
            floor = dest->floor;
            while ( piter != __points.end() )
            {
                if ( FloorFromID((*piter)->id) != floor )
                {
                    ++piter;
                    continue;
                }
                real dis = Distance(dest,*piter) ;//+ Distance(start,end);
                if ( dis < mindis )
                {
                    mindis = dis;
                    res = (*piter);
                }
                ++piter;
            }

        }else
        {
            res = _id2points[dest->id];
        }
        return res;
    }



    /**
     * @brief getNextBind
     * 计算tofloor中可与curpt互通的连接点
     *
     * @param tofloor
     * @param curpt
     * @return
     */
    ElemType *getBindInFloor(int tofloor, ElemType *curpt)
    {
        ElemType *res = NULL;
        vector<int> nbs = curpt->neigbours;
        //当前楼层的所有链接点
        vector<int>::iterator liter = nbs.begin();
        while ( liter != nbs.end())
        {
            if ( _id2points[*liter]->floor == tofloor)
            {
                res = _id2points[*liter];
                return _id2points[*liter];
            }
            ++liter;
        }

    }

    ElemType *getNearBindPointToFloor(int dflr, ElemType *curp)
    {
        int curflr = curp->floor;

        if ( isDirectLinkWith(dflr,curp) )
        {
            return curp;
        }
        //all bind points in current floor
        list<int> binds = _floor_binds[curflr];
        list<int>::iterator liter = binds.begin();
        real  mindis = INVALID;
        ElemType *result = NULL;
        //find nearest bind point to curp which can lead to dflr
        while ( liter != binds.end() )
        {

            if ( isDirectLinkWith(dflr,_id2points[*liter]))
            {
                real dis = Distance(_id2points[*liter],curp);
                if ( dis < mindis)
                {
                    mindis = dis;
                    result = _id2points[*liter];
                }
            }
            ++liter;
        }
        return result;
    }
    bool isDirectLinkWith(int flr,ElemType *curp)
    {
        if (curp->type == PtTyle_Bind )
        {
            vector<int> nbs = curp->neigbours;
            vector<int>::iterator it = nbs.begin();
            while ( it != nbs.end())
            {
                if ( _id2points[*it]->floor == flr )
                {
                    return true;
                }
                ++it;
            }
        }
        return false;
    }

    list<ElemType*> getFloorPath(int destfloor, ElemType *curpoint)
    {
        int cfloor = curpoint->floor;
        int sign = ( destfloor - cfloor > 0 ? 1: -1);
        int floor = cfloor;
        list<ElemType*>  results;

        do{
            //Specific elevator that direct to the destniation floor
            // exists, we can go there directly
            if( isDirectLinkWith(destfloor,curpoint))
            {
                curpoint->attr = sign * UpStairs;
                results.push_back(curpoint);
                ElemType *em = getBindInFloor(destfloor,curpoint);
                results.push_back(em);
                break;
            }
            //one floor at a time
            floor  +=  sign;
            ElemType *em = getNearBindPointToFloor(floor,curpoint);

            if ( em == NULL )
            {
                cerr<<"Can not find Bind to next floor"<<endl;
                results.clear();
                return results;
            }else
            {
                //find the bind point to next floor,now we shoud judge
                //whether the point is different from the given point or no
                //and then we can know if additional navigate are needed
                if ( em == curpoint )
                {
                    results.push_back(em);
                }else
                {
                    list<Node*> ret = Navigate::ref().GetBestPath(curpoint,em);
                    ret.back()->attr = UpStairs * sign;
                    listMerge(results,ret);
                }
                curpoint = getBindInFloor(floor,em);
                if( floor == destfloor)
                {
                    results.push_back(curpoint);
                }else{
                    curpoint->attr = UpStairs*sign;
                }
            }

        }while(floor != destfloor);
        return results;
    }

    ~ProcessRoadPoint()
    {

    }

private:

    vector<ElemType*>  &__points;

    //ID --Points对应表
    map<int,ElemType*> &_id2points;

    map<int,list<int> >  _floor_binds;
};

#endif // PROCESSROADPOINT_H
