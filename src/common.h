#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>
#include "navigate_defines.h"

using namespace std;

#define GLOBAL_FLOOR_CODE_BASE          10000
#define REAL_TYPE_SIZE                  (sizeof(real))

union charDouble
{
    char c[REAL_TYPE_SIZE];
    real db;
};

// convert real to double
void toDouble(real &x);

// make int to bigendian
void toBigEndian(int& x);

// floor point code to building globe code
int floorPCodeToGlobePCode(int floorCode, int floorNubmer);

// get floor number from globe point code
int globePointToFloorNumber(int globePointCode);

// split the string
vector<string> split(string str, string pattern);

// 计算两点距离
double calTwoPointDistance(double x1, double y1, double x2, double y2);

template<typename T>
bool isInVector(T &value, vector<T> dest)
{
    typename vector<T>::iterator iter = dest.begin();
    while ( iter!=dest.end())
    {
        if ( (*iter) == value )
        {
//          printf("ID:%d Value=%d\n",*iter,value);
            return true;
        }else
        {
            ++iter;
        }
    }
    return false;
}

template<typename T>
bool isInList(T &value, list<T> dest)
{
    list<T>::iterator iter = dest.begin();
    while ( iter!=dest.end())
    {
        if ( (*iter) == value )
        {
//          printf("ID:%d Value=%d\n",*iter,value);
            return true;
        }else
        {
            ++iter;
        }
    }
    return false;
}

template<typename T1,typename T2>
void typeTrans(T1 &to, const T2 &from)
{
    to.floor    = from.floor;
    to.x        = from.x;
    to.y        = from.y;
    to.id       = from.id;
}


template<typename T>
void listMerge(list<T> &dest, list<T> &src)
{

    if ( src.empty() )
    {
        return;
    }
    T back = src.front();
    if ( isInList(back,dest) )
    {
        src.pop_front();
    }
    list<T>::iterator it = src.begin();
    while ( it != src.end() )
    {
        dest.push_back(*it);
        ++it;
    }
}




#endif // COMMON_H

