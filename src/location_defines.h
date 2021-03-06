#ifndef WIFI_LOCATION_DEFINES_H
#define WIFI_LOCATION_DEFINES_H

#include <iostream>
#include <cstring>
#include <list>
#include <map>

using namespace std;

#define MAC_LENTH 12


////////////////////////////////
// struct used for output

typedef double real;

typedef struct _Point
{
    real    x;
    real    y;
    int     attr;
    int     id;
    int     type;
    int     floor;
    _Point() {
        x = -1.0f;
        y = -1.0f;
        id = -1;
        attr = -1;
        type = -1;
    }
}NavPoint;


#define LEN_FLOOR_CODE 32

typedef struct _Point_Sid_ST
{
    double x;
    double y;
    int id;
    int floor_number;
    char floor_code[LEN_FLOOR_CODE];
    _Point_Sid_ST() {
        x = -1.0f;
        y = -1.0f;
        id = -1;
        floor_number = 0;
        memset(floor_code, 0, LEN_FLOOR_CODE);
    }
} SidPoint;

typedef struct _Multi_Point_Wifi_ST
{
    char floor_code[LEN_FLOOR_CODE];
    int id1;
    double x1;
    double y1;
    int id2;
    double x2;
    double y2;
    int id3;
    double x3;
    double y3;
    int id4;
    double x4;
    double y4;
    _Multi_Point_Wifi_ST() {
        memset(this, 0, sizeof(_Multi_Point_Wifi_ST));
    }
} WifiMultiPoint;

typedef struct
{
    int       num;
    NavPoint  *pts;
}PointArray;

// struct used for output end
////////////////////////////////

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
}GatherFingerItem, RealTimeSignal;

typedef struct Point_ST {
    int floor_number;
    int pcode;
    double x;
    double y;
    const char* floor_code;
    Point_ST() {
        floor_number = 0;
        pcode = -1;
        x = -1.0f;
        y = -1.0f;
        floor_code = NULL;
    }
} LPoint;

// 楼层基本信息
typedef struct FloorBasicInfo_ST {
    string floor_code;
    int floor_number;
    FloorBasicInfo_ST() {
        floor_number = 0;
    }
} FloorBasicInfo;

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
    map<string, MacListItem> all_mac_map;           // 所有的wifi map, 里面保存有mac地址出现过的最强强度点等信息
    map< int, GatherFingerInfo > finger_map;        // 每个测量点的wifi指纹，key为测量点编号，value为测量点的wifi指纹
    map<string, list<int> > mac_point_code_map;     // mac地址在哪些采集点曾出现过
}FloorSignalInfo;

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

struct CalTemp
{
    string mac;
    int    curRssi; // mac地址在当前采集点的RSSI
    int    bestSimPointRssi; // mac地址在最相似点的RSSI
    int    maxRssi; // mac地址曾经出现过的最强强度
    double maxX; // mac地址曾经出现过的最强强度点的X坐标
    double maxY; // mac地址曾经出现过的最强强度点的Y坐标
    // 以上是原始的数据
    // 以下是要通过原始数据计算而得的处理后数据
    float  ft; // cur rssi 与 rssi 最相似的比值
    double hd_x; // 处理后的X坐标
    double hd_y; // 处理后的Y坐标
    double d; // (xi, yi) (x0, y0)距离
};

// 相似度算法类型
enum SimilarityCalType {
    enum_simi_type_11,
    enum_simi_type_12,
    enum_simi_type_21,
    enum_simi_type_22,
};

enum SignalType {
    enum_sigtype_wifi = 1,
    enum_sigtype_ble,
};

enum LocationCalType {
    enum_pcal_type_location = 1,
    enum_pcal_type_guidance = 2,
    enum_pcal_type_forcelocation = 3,
};

/*
 * eror code defines
 */
#define LOC_WIFI_ERR_FILE_OPEN_FAILED   -1
#define LOC_WIFI_ERR_MAC_ID_INVALID     -2


#endif // WIFI_LOCATION_DEFINES_H

