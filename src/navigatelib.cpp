#include "navigatelib.h"
#include "navigate.h"

#include "defines.h"

#include <map>
#include <list>
#include <vector>
#include <math.h>

using namespace std;

static Navigate global_nav;

PointArray      loadPathInfo(const char *filepath)
{
    global_nav.LoadPointsFile(filepath);
    vector<Node*> res = global_nav.GetAllPoints();
    PointArray    result;

    result.num = res.size();
    result.pts = new NavPoint[res.size()];

    vector<Node*>::iterator it = res.begin();
    int i = 0;
    while ( it != res.end() )
    {
        result.pts[i].x = (*it)->x;
        result.pts[i].y = (*it)->y;
        result.pts[i].id = (*it)->id;
        result.pts[i].attr = (*it)->attr;
        result.pts[i].type = (*it)->type;
        ++it;++i;
    }
    return result;
}

PointArray  getBestPath(NavPoint *start, NavPoint *end)
{
    Node *nstart = global_nav.GetPoint(start->id)
         ,*nend = global_nav.GetPoint(end->id);
    int  stfloor = FloorFromID(start->id)
            ,edfloor = FloorFromID(start->id);
    list<Node*> path;

    if ( stfloor != edfloor )
    {
        Node *dest1  = global_nav.getNearestBind(global_nav.GetPoint(start->id));
        Node *dest2  = global_nav.getNearestBind(global_nav.GetPoint(end->id));
        list<Node*>  tmppath = global_nav.GetBestPath(nstart,dest1),
                     tmppath2 = global_nav.GetBestPath(dest2,nend);
        list<Node*>::iterator it = tmppath.begin();
        int  attr = (stfloor < edfloor ? 1:-1) * UpStairs;
        tmppath.back()->attr = attr;
        while ( it != tmppath.end())
        {
            path.push_back(*it);
            ++it;
        }
        it = tmppath2.begin();
        while ( it != tmppath2.end() )
        {
            path.push_back(*it);
            ++it;
        }
    }
    else
        path = global_nav.GetBestPath(nstart,nend);
    PointArray result;
    result.num = path.size();
    result.pts = new NavPoint[result.num];
    list<Node*>::iterator it = path.begin();
    size_t i = 0;
    while ( it != path.end() )
    {
        result.pts[i].x = (*it)->x;
        result.pts[i].y = (*it)->y;
        result.pts[i].id = (*it)->id;
        result.pts[i].attr = (*it)->attr;
        result.pts[i].type = (*it)->type;
        ++it;++i;
    }
    return result;

}
