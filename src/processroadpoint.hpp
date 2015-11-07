#ifndef PROCESSROADPOINT_H
#define PROCESSROADPOINT_H

#include "../../src/location_defines.h"
#include "../../src/navigate_defines.h"

#include <map>
#include <list>
#include <iostream>
using namespace std;

/**
 *使用模板做数据点的处理，防止日后对数据结构进行修改
 *而造成程序适配复杂
 *
 */

template<typename ElemType>
/**
 * @brief The ProcessRoadPoint class
 *
 *处理地图数据点，导航结果中不应只包含路口点，应将路口之间的数据点
 * 输出至结果路径中，以便于APP使用该数据构建准确的路径
 *
 */
class ProcessRoadPoint
{
public:
    explicit ProcessRoadPoint( map<int,ElemType*> data ,vector<ElemType*> pts)
    {
        __points = pts;
        _id2points = data;
    }

    explicit ProcessRoadPoint(ProcessRoadPoint &p)
    {
        __points = p.__points;
        _id2points = p._id2points;
    }

    ProcessRoadPoint& operator = ( ProcessRoadPoint &p)
    {
        __points = p.__points;
        _id2points = p._id2points;
        return *this;
    }

    list<ElemType*> Process(list<ElemType*> &result)
    {
        list<ElemType*> finalResult;
        list<ElemType*>::iterator riter = result.begin();
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
            vector<ElemType*> ::iterator viter = __points.begin();
            while( viter != __points.end() )
            {
                if ( isInVector(pre->id,(*viter)->neigbours)
                     && isInVector(cur->id,(*viter)->neigbours))
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
                list<ElemType>::iterator it = midpts.begin();
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

    ~ProcessRoadPoint()
    {

    }

private:

    vector<ElemType*>  __points;

    //ID --Points对应表
    map<int,ElemType*> _id2points;
};

#endif // PROCESSROADPOINT_H
