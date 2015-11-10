#ifndef NAVEXCEPTION_H
#define NAVEXCEPTION_H

#include <exception>
#include <iostream>
#include <string.h>

using namespace std;

const int MaxMsgLen = 128;

class NavException:public exception
{
public:

    NavException():exception()
    {
    }

    NavException(const char *m):exception( m )
    {
    }

    ~NavException()
    {

    }
};

#endif // NAVEXCEPTION_H
