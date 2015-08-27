#include "navigatelib.h"

#include <iostream>

using namespace std;

int main()
{
    PointArray pt =   loadPathInfo("gemo.bin");

    NavPoint start,end;

    start.x = 375;
    start.y = 1005;

    end.x = 375;
    end.y = 405;

    pt = getBestPath(&start,&end);

    for ( int i = 0 ; i < pt.num; ++i)
    {
        cout<<pt.pts[i].id<<endl;
    }

    return 0;
}
