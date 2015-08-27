#ifndef WIFI_H
#define WIFI_H

#define FINGER_COUNT    15
#define MAC_MAX_LEN         32

//WI-FI AP
typedef struct wifimap
{
    int id;
    int apid;
    int rssi;
    double x;
    double y;
}
wifi_map;

//WIFI 指纹
typedef struct _wifi_finger
{
    wifi_map wifi[FINGER_COUNT];
}wifi_finger;

//WI-FI列表
typedef struct wifilist
{
    char mac[MAC_MAX_LEN];
    int  rssi;
    int  pt_max;
}
wifi_list;

//标记点坐标
typedef struct tag
{
    double x;
    double y;
}tagPoint;

//当前点AP
typedef struct _point
{
    char mac[MAC_MAX_LEN];
    int  rssi;
}point;

//当前点
typedef struct _wfnow
{
    int listid;
    int rssi_cur;
    //char mac[MAC_MAX_LEN];
    int rssi_list;
    int pt_max;
}
wifi_now;

//计算点坐标数据结构
typedef struct _xy
{
    int listid;
    int rssi_cur;
    int rssi_list;
    double x;
    double y;
}wifi_xy;

wifi_finger* readWifiMap(const char *fname, int *nump, int *numwp);
wifi_list*  readWifiList(const char *fname, int *count);
tagPoint *  readTagPoints(const char* fname);
point*      readCurrentWifi(const char* fname);


void        flushfile(const char *fname, wifi_finger *finger,tagPoint* tag, int fcount
                      , wifi_list *list, int count);


void        wifi();


#endif // WIFI_H
