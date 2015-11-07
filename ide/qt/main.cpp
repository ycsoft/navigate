#include <iostream>

#include "../../src/location_defines.h"
#include "../../src/navigate_defines.h"
#include "../../src/paka_api.h"

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
void outputNeig( vector<int> &nei)
{
    cout<<" [ ";
    for ( size_t i = 0 ; i < nei.size(); ++i)
    {
        cout<<nei[i]<<" ";
    }
    cout<<" ]";
}

int main()
{

    PointArray paths = loadPathInfo("mg(cross_floor).i2");
    outputFloors(paths);

    NavPoint start =  paths.pts[39]
            ,end =  paths.pts[20];

    cout<<"Start id:"<<start.id<<"  End Id:"<<end.id<<endl;
//    end.x += 10;
//    end.y += 5;
//    end.id = -1;
//    end.floor = 3;

//    NavPoint *pnav = GetPoint(20021);
//    start = (pnav == NULL ? NavPoint():(*(pnav)));
//    end.id = -1;
    start.x += 1;
    start.y += 1;
    start.id = -1;
    end.x += 1;
    end.y += 1;
    end.id=-1;
    PointArray results = getBestPath(&end,&start);
    int i = 0;
    for( ; i < results.num; i++)
    {
        cout<<results.pts[i].id<<"\t";
        outputNeig(GetNeighor(results.pts[i].id));
        cout<<GetTips(results.pts[i].attr)<<endl;
    }
    return 0;
}

