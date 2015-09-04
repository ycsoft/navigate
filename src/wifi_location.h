#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H


#include <map>
#include <cstring>

#include "wifi_location_defines.h"

using namespace std;

class WifiLocation
{
public:
    WifiLocation();
    ~WifiLocation();

    // load the wifi file
    int LoadWifiFile(const char* filepath);

private:
    // 保存楼宇中每个楼层的wifi数据
    map<string, FloorWifiInfo> m_building_wifi_info;


};
#endif // WIFI_LOCATION_H
