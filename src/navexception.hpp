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
        memset(msg,0,MaxMsgLen);
        strcpy(msg,"NavException");
    }

    NavException(const char *m):exception()
    {
        strcpy(msg,m);
    }

    const char *what() const
    {
        return msg;
    }
    ~NavException()
    {


    }
private:
    char        msg[MaxMsgLen];
};

#endif // NAVEXCEPTION_H
