#ifndef DEFINES_H
#define DEFINES_H

#include <iostream>
#include <cstdlib>
#include <list>
#include <vector>




#define     ISEndian                1
#define     NotEndian               0

#define     TurnLeft                3   //左转
#define     TurnRight               4   //右转
#define     WalkDirect              5   //直行

#define     WalkAlong               6   //沿弯路继续前行


#define     Arrive                  11


#define     RightHandSys            1
#define     LeftHandSys             -1

#define     Direct_Mask             0xffffffff

#define     INVALID                 9999999.9f

//用于浮点数比较
#define     ZERO                    0.000001f
#define     EQUAL(x,y)              (x - y <= ZERO && x - y >= 0-ZERO )

#define     Distance(nx,ny)         ((nx->x - ny->x)*(nx->x - ny->x) \
                                    +(nx->y - ny->y)*(nx->y - ny->y))

#define     FloorFromID( ID )       ( (ID)/10000 )

using namespace std;

typedef double  real;


typedef struct Node
{
    Node()
    {
        id = 0;
        x = 0.0f;
        y = 0.0f;
        g = 0.0f;
        h = 0.0f;
        parent = NULL;
        type = 0;

        attr = ISEndian;
        neighborcount = 0;
    }

    bool operator == ( const Node &nd) const
    {
        return ( EQUAL(x,nd.x) && EQUAL(y,nd.y) );
    }
    bool operator != ( const Node &nd) const
    {
        return !( EQUAL(x,nd.x) && EQUAL(y,nd.y) );
    }

    Node(real fx,real fy)
    {
        x = fx;
        y = fy;
        g = 0.0f;
        h = 0.0f;
        parent = NULL;
        attr = ISEndian;
        id = 0;
        type = 0;
        neighborcount=0;
    }
    ~Node()
    {
        //cout<<"Node Destroy"<<endl;
    }

    //坐标
    real x;
    real y;

    //权值
    real g;
    real h;

    //附加属性
    int   attr;
    int   id; //点的编号
    int   floor; //楼层号
    int   type;
    //岔路数
    int   neighborcount;
    ///
    /// \brief neigbours
    /// 存储点的ID
    ///
    vector<int>  neigbours;

    Node *parent;
}Point,*Point_ptr;

struct Node_hash
{
    size_t operator()( const Node &nd) const
    {
        return nd.id;
    }
};

typedef struct Line
{
    Line()
    {
        num = 0;
    }

    Line(int n)
    {
        num = n;
    }
    ~Line()
    {
        //cout<<"Line Destroy"<<endl;
    }
    int num;        //点数
    list<Point*> pts;     //路径上的点集
}Path;


typedef struct SortValue
{
    real key;
    Node *res;
}SortValue;



#endif // DEFINES_H
