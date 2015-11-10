#include <iostream>
#include <ctime>
#include <cmath>

#include "../../src/location_defines.h"
#include "../../src/navigate_defines.h"
#include "../../src/paka_api.h"
#include "../../src/navigate.h"
#include "../../src/common.h"
#include "../../src/navexception.hpp"

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
    cout<<" ] ";
}


void unit_test(PointArray &paths, int id1,int id2)
{
    paths = loadPathInfo("mg.i2");
    NavPoint start =  paths.pts[id1]
            ,end =  paths.pts[id2];

    cout<<"from id:"<<start.id<<"  to Id:"<<end.id;
    start.floor = FloorFromID(start.id);
    start.x += 1;
    start.y += 1;
    start.id = -1;
    end.floor = FloorFromID(end.id);
    end.x += 1;
    end.y += 1;
    end.id=-1;
    PointArray results = getBestPath(&start,&end);
    if ( results.num > 1 )
    {
        cout<<"\t Pass!"<<endl;
    }else
    {
        cout<<"\t Fail!"<<endl;
        system("pause");
    }
}

void suit_test()
{
    int testtimes = 10000;
    srand(time(NULL));
    PointArray paths = loadPathInfo("mg.i2");
    int count = paths.num;
    int i = 0;
    cout<<"Test Times:"<<testtimes;
    while( i++ < testtimes )
    {
        unit_test(paths,rand()%count,rand()%count);
    }
}

int main()
{

    PointArray paths = loadPathInfo("mg.i2");
    Navigate nav;
    PointArray results;
    nav.LoadPointsFile("mg.i2");
    outputFloors(paths);

    NavPoint start
            ,end;
    try
    {
        typeTrans(start,*(nav._id2points[20084]));
        typeTrans(end,*(nav._id2points[40019]));
        results = getBestPath(&start,&end);
    }catch(NavException &nav)
    {
        cout<<nav.what()<<endl;
        return EXIT_FAILURE;
    }

    int i = 0;
    cout<<"From:"<<start.id<<" To:"<<end.id<<endl;
    for( ; i < results.num; i++)
    {
        cout<<results.pts[i].id<<"\t";
        cout<<GetTips(results.pts[i].attr)<<endl;
    }
        if( results.num <= 0 )
        {
            cout<<"Can not find"<<endl;
        }

  //suit_test();


    return EXIT_SUCCESS;
}

