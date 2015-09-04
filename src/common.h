#ifndef COMMON_H
#define COMMON_H

#include "navigate_defines.h"

#define  REAL_TYPE_SIZE  (sizeof(real))
union charDouble
{
    char c[REAL_TYPE_SIZE];
    real db;
};

// convert real to double
void toDouble(real &x);

// make int to bigendian
void toBigEndian(int& x);


#endif // COMMON_H

