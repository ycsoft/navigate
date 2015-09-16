#ifndef WIFI_LOCATION_DEFINES_H
#define WIFI_LOCATION_DEFINES_H

#include <iostream>
#include <cstring>
#include <list>
#include <map>

using namespace std;

#define MAC_LENTH 12


// 每层楼wifi列表的列表项结构体
typedef struct Mac_List_Item_ST {
    int id;
    string mac;
    int max_rssi;           // 信号强度最大值
    int gather_point_code;  // 信号强度最大值的测量点
    Mac_List_Item_ST() {
        memset(this, 0, sizeof(Mac_List_Item_ST));
    }
    Mac_List_Item_ST(int _id, string _mac, int _max_rssi, int _gather_point_code) {
        this->id = _id;
        this->mac = _mac;
        this->max_rssi = _max_rssi;
        this->gather_point_code = _gather_point_code;
    }
}MacListItem;

typedef struct Gather_Finger_Item_ST {
    string mac;
    int rssi;
    Gather_Finger_Item_ST() {
        rssi = 0;
    }
    Gather_Finger_Item_ST(string _mac, int _rssi) {
        this->mac = _mac;
        this->rssi = _rssi;
    }
}GatherFingerItem, RealTimeFinger;

typedef struct Point_ST {
    int pcode;
    double x;
    double y;
    const char* floor_code;
    Point_ST() {
        pcode = -1;
        x = -1.0f;
        y = -1.0f;
        floor_code = NULL;
    }
} LPoint;

// 相似度排序的时候，用到的临时结构体
typedef struct Point_Similar_Temp_ST {
    int pcode;
    double x;
    double y;
    float simi;
} SPointTemp;

typedef struct Gather_Finger_Info_ST {
    LPoint p;
    map<string, GatherFingerItem> fingers_map;
} GatherFingerInfo;

// used to store one wifi data of one floor
typedef struct Wifi_Floor_ST {
    string floor_code;
    int floor_number;
    map<string, MacListItem> all_mac_map;           // 所有的wifi map
    map< int, GatherFingerInfo > finger_map;        // 每个测量点的wifi指纹，key为测量点编号，value为测量点的wifi指纹
    map<string, list<int> > mac_point_code_map;     // mac地址在哪些采集点曾出现过
}FloorWifiInfo;

// 相似度计算结果结构体，同时返回每次不同的相似度
typedef struct Similarity_Result_ST {
    int s1;
    float s2;
    float s3;
    Similarity_Result_ST() {
        s1 = 0;
        s2 = 0.0f;
        s3 = 0.0f;
    }
    Similarity_Result_ST(int s1, float s2, float s3) {
        this->s1 = s1;
        this->s2 = s2;
        this->s3 = s3;
    }
} Similarities;

// 相似度算法类型
enum SimilarityCalType {
    enum_Normal = 1,
    enum_M2,
    enum_M3,
};

/*
 * eror code defines
 */
#define LOC_WIFI_ERR_FILE_OPEN_FAILED   -1
#define LOC_WIFI_ERR_MAC_ID_INVALID     -2


#endif // WIFI_LOCATION_DEFINES_H

