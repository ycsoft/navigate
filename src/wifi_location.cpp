#include "wifi_location.h"

#include <stdlib.h>
#include <stdio.h>

#include "common.h"

WifiLocation::WifiLocation()
{

}

WifiLocation::~WifiLocation()
{

}

/*!
 * \brief load the wifi file
 * wifi数据文件 ,文件形式:二进制文件，每栋楼一个文件
 * 从数据库中获取每个楼层的wifi数据，并生成文件
 *
 * 楼宇编号(32字节) 楼层数
 *
 * 楼层编号(32字节) 楼层的自然层序号 楼层总的mac地址数
 * （首先是每个楼层总的mac列表 ）
 * mac序号 mac地址12字节 rssi最强强度值（有符号int型4字节）最强强度点编号（有符号int型4字节）
 * ......
 * （然后是每个采集点的信息）
 * 楼层总的采集点数
 * 采集点点编号 x坐标 y坐标 采集点采集的mac个数
 * mac序号 rssi强度值
 *
 * （接下来再进行楼层循环）
 *
 * 其中数量类为4字节,int类型;坐标点为double类型
 * \param filepath
 */

int WifiLocation::LoadWifiFile(const char *filepath)
{
    // read the file
    //二进制文件读取
    FILE *f = fopen(filepath,"rb");
    if (NULL == f)
    {
        return LOC_WIFI_ERR_FILE_OPEN_FAILED;
    }

    char _buf_32[32] = {0};
    int _floor_count = 0;
    int _floor_number = 0;
    int _floor_mac_number = 0;

    //楼宇编号
    fread(_buf_32, 1, 32, f);
    //楼层数
    fread(&_floor_count, sizeof(int), 1, f);
    toBigEndian(_floor_count);


    // 开始处理每个楼层
    for (int i = 0; i < _floor_count; i++)
    {
        FloorWifiInfo fwi;
        // 楼层编号
        fread(_buf_32, 1, 32, f);
        // 楼层自然层序号
        fread(&_floor_number, sizeof(int), 1, f);
        toBigEndian(_floor_number);
        // 楼层总的mac地址数
        fread(&_floor_mac_number, sizeof(int), 1, f);
        toBigEndian(_floor_mac_number);

        fwi.floor_number = _floor_number;
        fwi.floor_code = _buf_32;

        // 开始处理每个楼层总的mac地址列表
        // mac序号 mac地址12字节 rssi最强强度值（有符号int型4字节）最强强度点编号（有符号int型4字节）

        char _buf_12[12+1] = {0};
        int _mac_id = 0;
        int _rssi_max = 0;
        int _rssi_max_point_code = 0;

        map<int, string> _id_mac_map;
        for (int j = 0; j < _floor_mac_number; j++)
        {
            // mac序号
            fread(&_mac_id, sizeof(int), 1, f);
            toBigEndian(_mac_id);
            // mac地址12字节
            fread(_buf_12, 1, 12, f);
            // rssi最强强度值
            fread(&_rssi_max, sizeof(int), 1, f);
            toBigEndian(_rssi_max);
            // 最强强度点编号
            fread(&_rssi_max_point_code, sizeof(int), 1, f);
            toBigEndian(_rssi_max_point_code);
            string _mac_string = _buf_12;

            // 填充楼层wifi列表
            MacListItem item(_mac_id, _mac_string, _rssi_max, _rssi_max_point_code);
            fwi.all_mac_map.insert(make_pair(_mac_string, item));
            // 写入mac与序号对应关系
            _id_mac_map.insert(make_pair(_mac_id, _mac_string));
        }

        // 然后是每个采集点的信息
        // 采集点点编号 x坐标 y坐标 采集点采集的mac个数
        // mac序号 rssi强度值
        int _gather_count = 0;
        int _gather_point_code = 0;
        double _x = 0.0;
        double _y = 0.0;
        int _gather_mac_count = 0;
        int _gather_mac_id = 0;
        int _gather_mac_rssi = 0;

        // 楼层总的采集点数
        fread(&_gather_count, sizeof(int), 1, f);
        toBigEndian(_gather_count);

        for (int j = 0; j < _gather_count; j++)
        {
            GatherFingerInfo finfo;
            // 采集点点编号
            fread(&_gather_point_code, sizeof(int), 1, f);
            toBigEndian(_gather_point_code);
            // 采集点x坐标
            fread(&_x, sizeof(double), 1, f);
            toDouble(_x);
            // 采集点y坐标
            fread(&_y, sizeof(double), 1, f);
            toDouble(_y);

            finfo.p.pcode = _gather_point_code;
            finfo.p.x = _x;
            finfo.p.y = _y;

            // 采集点采集的mac个数
            fread(&_gather_mac_count, sizeof(int), 1, f);
            toBigEndian(_gather_mac_count);

            for (int k = 0; k < _gather_mac_count; k++)
            {
                // mac序号
                fread(&_gather_mac_id, sizeof(int), 1, f);
                toBigEndian(_gather_mac_id);
                // rssi
                fread(&_gather_mac_rssi, sizeof(int), 1, f);
                toBigEndian(_gather_mac_rssi);

                // 从mac序号中找mac地址
                map<int, string>::iterator iter = _id_mac_map.find(_gather_mac_id);
                if (iter == _id_mac_map.end())
                {
                    // 没有在序号中找到mac地址肯定是不对的
                    m_building_wifi_info.clear();
                    return LOC_WIFI_ERR_MAC_ID_INVALID;
                }
                string _gather_mac = iter->second;
                GatherFingerItem item(_gather_mac, _gather_mac_rssi);
                finfo.fingers_map.insert(make_pair(_gather_mac, item));
            }
            fwi.finger_map.insert(make_pair(_gather_point_code, finfo));
        }
        m_building_wifi_info.insert(make_pair(fwi.floor_code, fwi));
    }
    return 0;
}





