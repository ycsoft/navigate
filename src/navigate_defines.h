#ifndef NAVIGATE_DEFINES_H
#define NAVIGATE_DEFINES_H

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
#define     AlongLeft               63  //靠左行走
#define     AlongRight              64  //靠右行走


#define     Arrive                  11  //到达

#define     UpStairs                12  //到楼上
#define     DownStairs              -12 //到楼下

#define     CloseToU               13  //就在跟前


#define     RightHandSys            1
#define     LeftHandSys             -1

#define     Direct_Mask             0xffffffff

#define     INVALID                 999999999.9

//用于浮点数比较
#define     ZERO                    0.000001f
#define     EQUAL(x,y)              (x - y <= ZERO && x - y >= 0-ZERO )

#define     Distance(nx,ny)         sqrt((((nx)->x - (ny)->x)*((nx)->x - (ny)->x) \
                                    +((nx)->y - (ny)->y)*((nx)->y - (ny)->y)))

#define     FloorFromID(ID)       ( (ID)/10000 )

#define     GetTips(ID)           (ID == TurnLeft ? "左转": \
                                        (ID == UpStairs ? "上楼": \
                                        (ID == DownStairs ? "下楼": \
                                        (ID == TurnRight ? "右转": \
                                        (ID == WalkDirect? "直行" : \
                                        (ID == WalkAlong ? "继续前行" : \
                                        (ID == Arrive ? "到达": \
                                        (ID == AlongLeft ? "靠左行走": \
                                        (ID == AlongRight ? "靠右行走":"无法判断方向"))))))))   \
                                   )

#define     INVALID_ID              -1  //无效的楼层编号

using namespace std;

typedef double real;
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

	bool operator == (const Node &nd) const
	{
		return (EQUAL(x, nd.x) && EQUAL(y, nd.y));
	}
	bool operator != (const Node &nd) const
	{
		return !(EQUAL(x, nd.x) && EQUAL(y, nd.y));
	}

	Node(real fx, real fy)
	{
		x = fx;
		y = fy;
		g = 0.0f;
		h = 0.0f;
		parent = NULL;
		attr = ISEndian;
		id = 0;
		type = 0;
		neighborcount = 0;
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
}Point, *Point_ptr;

struct Node_hash
{
	size_t operator()(const Node &nd) const
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

#endif // NAVIGATE_DEFINES_H
