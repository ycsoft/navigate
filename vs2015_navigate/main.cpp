// vs2015_navigate.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include "../src/navigatelib.h"
#include "../src/defines.h"

using namespace std;

string parseAtrr(int arrt) {

	/*
	#define     TurnLeft                3   //左转
	#define     TurnRight               4   //右转
	#define     WalkDirect              5   //直行
	#define     WalkAlong               6   //沿弯路继续前行
	#define     Arrive                  11  //到达

	#define     UpStairs                12  //到楼上
	#define     DownStairs              -12  //到楼下
	*/

	switch (arrt)
	{
	case TurnLeft:
		return "左转";
	case TurnRight:
		return "右转";
	case WalkDirect:
		return "直行";
	case WalkAlong:
		return  "沿弯路继续前行";
	case Arrive:
		return "到达";
	case UpStairs:
		return "上楼";
	case DownStairs:
		return "下楼";
	default:
		return "随便走";
	}
}


int main()
{
	PointArray pointarr = loadPathInfo("e:\\mg.i2");
	int num = pointarr.num;
	/*NavPoint  *p = pointarr.pts;
	for (int i = 0; i < num; i++)
	{
		printf("i=%d, x=%f, y=%f, attr=%d, id=%d, type=%d\n", i, p->x, p->y, p->attr, p->id, p->type);
		p++;
	}
	getchar();*/

	while (1)
	{
		cout << "------------------------------------" << endl;
		int sid;
		int eid;
		cout << "input start id: ";
		cin >> sid;
		cout << "input end id: ";
		cin >> eid;

		NavPoint start;
		NavPoint end;

		start.id = sid;
		end.id = eid;

		PointArray path = getBestPath(&start, &end);
		num = path.num;
		NavPoint *p = path.pts;
		for (int i = 0; i < num; i++)
		{
			printf("i=%d, x=%f, y=%f, attr=%d, id=%d, type=%d\n", i, p->x, p->y, p->attr, p->id, p->type);
			p++;
		}
		cout << "==========" << endl;
		p = path.pts;
		for (int i = 0; i < num; i++)
		{
			cout << i << ". " << "在点" << p->id << parseAtrr(p->attr) << endl;
			p++;
		}

		cout << "input 1 fot continue, 0 to exit: ";
		int q;
		cin >> q;
		if (q == 0)
		{
			break;
		}
	}
	return 0;
}


