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


#endif // COMMON_H

