#ifndef COMMON_H
#define COMMON_H

#include "navigate_defines.h"


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


#endif // COMMON_H

