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
    PointArray paths = loadPathInfo("mg(916_2326).i2");
    outputFloors(paths);

    NavPoint start =  paths.pts[0]
            ,end =  paths.pts[20];

//    start.x += 10;
//    start.y += 5;
//    start.id = -1;
//    start.floor = 2;
    end.x += 10;
    end.y += 5;
    end.id = -1;
    end.floor = 3;

    start = *(GetPoint(20021));
    PointArray results = getBestPath(&end,&start);
    int i = 0;
    for( ; i < results.num; i++)
    {
        cout<<results.pts[i].id<<"\t"<<GetTips(results.pts[i].attr)<<endl;
    }
    return 0;
}

