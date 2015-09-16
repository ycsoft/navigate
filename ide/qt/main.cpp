#include <iostream>

#include "../../src/paka_api.h"
#include "../../src/navigate_defines.h"

using namespace std;

void outputFloors( PointArray paths )
{
    int i = 1,fl = FloorFromID(paths.pts[0].id);
    cout<<fl<<endl;
    for ( ; i < paths.num; ++i )
    {
        int cf = FloorFromID(paths.pts[i].id);
        if ( cf != fl )
        {
            cout<< FloorFromID(paths.pts[i].id)<<endl;
            fl = cf;
        }
    }
}

int main()
{
    cout << "Hello World!" << endl;
    PointArray paths = loadPathInfo("mg(cross_floor).i2");
    outputFloors(paths);

    NavPoint start =  paths.pts[0]
            ,end =  paths.pts[20];

    start.x += 10;
    start.y += 5;
    start.id = -1;
    start.floor = 2;
    PointArray results = getBestPath(&start,&end);

    cout<<results.num<<endl;
    return 0;
}

