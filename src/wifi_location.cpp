#include "wifi_location.h"

#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>

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

                // 这里已经可以知道在某个mac地址在某个采集点上出现过, 处理此信息
                addMacToAppearPointCodeMapList(_gather_mac, _gather_point_code, fwi.mac_point_code_map);
            }
            fwi.finger_map.insert(make_pair(_gather_point_code, finfo));
        }
        m_building_wifi_info.insert(make_pair(fwi.floor_code, fwi));
    }
    return 0;
}


/*!
 * \brief 将mac地址加入到曾经出现过的点列表中
 * \param mac mac地址
 * \param point_code 采集点编号
 * \param mac_point_code_map 关系表
 * \return mac地址出现过的采集点个数
 */
int WifiLocation::addMacToAppearPointCodeMapList(string mac, int point_code, map<string, list<int> > &mac_point_code_map)
{
    map< string, list<int> >::iterator iter = mac_point_code_map.find(mac);
    if (iter == mac_point_code_map.end())
    {
        list<int> plist;
        plist.push_back(point_code);
        mac_point_code_map.insert(make_pair(mac, plist));
        return plist.size();
    }
    else
    {
        list<int> &plist = iter->second;
        plist.push_back(point_code);
        return plist.size();
    }
}

/*!
 * \brief 根据传入的指纹，确定在哪一个楼层
 * \param fingers 传入的指纹
 * \param size 指纹个数
 * \return 楼层编号
 */
string WifiLocation::LocationBuildingFloor(InputFinger* fingers[], int size)
{
    // 分别计算每一个楼层，取匹配数目最高
    // 遍历map
    string max_matched_floor_code;
    int max_matched = -1;
    map<string, FloorWifiInfo>::iterator iter = m_building_wifi_info.begin();
    for ( ; iter != m_building_wifi_info.end(); ++iter)
    {
        string floor_code = iter->first;
        FloorWifiInfo &finfo = iter->second;
        int matched = calSimilarityInBuildingFloor(fingers, size, finfo);
        if (matched > max_matched)
        {
            max_matched = matched;
            max_matched_floor_code = floor_code;
        }
    }
    return max_matched_floor_code;
}


/*!
 * \brief 计算传入的wifi指纹与楼层wifi列表的相似度
 * \param fingers 传入的指纹
 * \param size 指纹个数
 * \param finfo 单个楼层的wifi信息引用
 * \return 相似度(匹配个数)
 */
int WifiLocation::calSimilarityInBuildingFloor(InputFinger* fingers[], int size, FloorWifiInfo &finfo)
{
    map<string, MacListItem> &all_mac_map = finfo.all_mac_map;
    if (all_mac_map.size() == 0)
    {
        return 0;
    }
    int matched = 0;
    for (int i = 0; i < size; i++)
    {
        InputFinger* f = fingers[i];
        string fmac = f->mac;
        map<string, MacListItem>::iterator iter = all_mac_map.find(fmac);
        if (iter != all_mac_map.end())
        {
            matched++;
        }
    }
    return matched;
}

/*!
 * \brief judge which point am I in the floor
 * \param floor_code 楼层编码
 * \param fingers 传入的wifi data
 * \param size 指纹容量
 * \return 定位点结构体
 */
LPoint WifiLocation::LocationFloorPoint(const char* floor_code, InputFinger* fingers[], int size)
{
    LPoint p;
    p.x = -255.0;
    p.y = -255.0;
    p.pcode = -1;

    // 根据楼层编号得到wifi数据
    string fcode = floor_code;
    map<string, FloorWifiInfo>::iterator iter = m_building_wifi_info.find(fcode);
    if (iter == m_building_wifi_info.end())
    {
        // location error
        return p;
    }

    FloorWifiInfo &floor_wifi_info = iter->second;
    set<int> possible_pcode_set = calPossiblePoints(fingers, size, floor_wifi_info.mac_point_code_map);

    vector<SPointTemp> sptl = getSimilarPointCodeList(fingers, size, floor_wifi_info.finger_map, possible_pcode_set);

    p = calFloorPointLocation(sptl);
    p.pcode = floorPCodeToGlobePCode(p.pcode, floor_wifi_info.floor_number);
    return p;
}

/*!
 * \brief 计算可能的点，缩小进行相似度比较的采集点范围
 * \param fingers
 * \param size
 * \param finger_map
 * \return
 */
set<int> WifiLocation::calPossiblePoints(InputFinger* fingers[], int size, map<string, list<int> > &mac_point_code_map)
{
    set<int> ret;
    for (int i = 0; i < size; i++)
    {
        InputFinger* f = fingers[i];
        map<string, list<int> >::iterator iter =  mac_point_code_map.find(f->mac);
        if (iter != mac_point_code_map.end())
        {
            list<int> &l = iter->second;
            for (list<int>::iterator iter2 = l.begin(); iter2 != l.end(); ++iter2)
            {
                int code = *iter2;
                ret.insert(code);
            }
        }
    }
    return ret;
}


/*!
 * \brief 计算传入的wifi数据与采集点指纹的相似度
 * \param fingers 传入的wifi data
 * \param size input wifi data length
 * \param ginfo gather point wifi info
 * \return similarity
 */
int WifiLocation::calSimilarityInFloorGatherCode(InputFinger* fingers[], int size, GatherFingerInfo &ginfo)
{
    map<string, GatherFingerItem> &fingers_map = ginfo.fingers_map;
    if (fingers_map.size() == 0)
    {
        return 0;
    }

    int matched = 0;
    for (int i = 0; i < size; i++)
    {
        InputFinger* f = fingers[i];
        string fmac = f->mac;
        map<string, GatherFingerItem>::iterator iter = fingers_map.find(fmac);
        if (iter != fingers_map.end())
        {
            matched++;
        }
    }
    return matched;
}

/*!
 * \brief 得到最相似点
 * \param fingers 传入的wifi data
 * \param size input wifi data length
 * \param finger_map 指纹map
 * \param points 待比较的目标点
 * \return
 */
int WifiLocation::getMostSimilarPointCode(InputFinger* fingers[], int size, map< int, GatherFingerInfo > &finger_map, set<int> &points)
{
    set<int>::iterator iter = points.begin();
    int most_simi_code = -1;
    int max_simi = -1;

    for ( ; iter != points.end(); ++iter)
    {
        int pcode = *iter;
        map< int, GatherFingerInfo >::iterator iter2 = finger_map.find(pcode);
        if (iter2 != finger_map.end())
        {
            GatherFingerInfo &info = iter2->second;
            int simi = calSimilarityInFloorGatherCode(fingers, size, info);
            if (simi > max_simi)
            {
                max_simi = simi;
                most_simi_code = pcode;
            }
        }
    }
    return most_simi_code;
}

/*!
 * \brief 按照相似度排序函数
 * \param v1
 * \param v2
 * \return
 */
bool SortBySimi( const SPointTemp &v1, const SPointTemp &v2)//注意：本函数的参数的类型一定要与vector中元素的类型一致
{
    return v1.simi > v2.simi; //升序排列
}

/*!
 * \brief 得到相似点列表，按照相似度排序
 * \param fingers 传入的wifi data
 * \param size input wifi data length
 * \param finger_map 指纹map
 * \param points 待比较的目标点
 * \return
 */
vector<SPointTemp> WifiLocation::getSimilarPointCodeList(InputFinger* fingers[], int size, map< int, GatherFingerInfo > &finger_map, set<int> &points)
{
    vector<SPointTemp> fv;
    set<int>::iterator iter = points.begin();
    for ( ; iter != points.end(); ++iter)
    {
        int pcode = *iter;
        map< int, GatherFingerInfo >::iterator iter2 = finger_map.find(pcode);
        if (iter2 != finger_map.end())
        {
            GatherFingerInfo &info = iter2->second;
            int simi = calSimilarityInFloorGatherCode(fingers, size, info);
            SPointTemp spt;
            spt.x = info.p.x;
            spt.y = info.p.y;
            spt.simi = simi;
            spt.pcode = info.p.pcode;
            fv.push_back(spt);
        }
    }
    sort(fv.begin(), fv.end(), SortBySimi);
    return fv;
}


/*!
 * \brief 得到最相似点后计算XY坐标
 * \param vecSpt 相似点，按相似度排序
 * \return
 */
LPoint WifiLocation::calFloorPointLocation(vector<SPointTemp> vecSpt)
{
    LPoint ret;
    ret.pcode = -1;
    if (vecSpt.size() == 0)
    {
        return ret;
    }

    // 只取第一个点
    SPointTemp &p1 = vecSpt[0];
    ret.pcode = p1.pcode;
    ret.x = p1.x;
    ret.y = p1.y;
    return ret;

    /*
    // 取前三个点，求平均坐标，然后看平均坐标离哪个最近，则取最近的点
    double sum_x = 0.0;
    double sum_y = 0.0;
    int count = 0;
    for (int i= 0; i < vecSpt.size() && i < 3; ++i)
    {
        sum_x = sum_x + vecSpt[i].x;
        sum_y = sum_y + vecSpt[i].y;
        ++count;
    }

    double avg_x = sum_x / count;
    double avg_y = sum_y / count;

    SPointTemp mp;
    double min = 99999.0f;
    // 求两点间距离，得到最近距离，即为选取的点
    for (int i = 0; i < count; ++i)
    {
        double x = vecSpt[i].x;
        double y = vecSpt[i].y;

        double dis = calTwoPointDistance(x, y, avg_x, avg_y);
        if (dis < min)
        {
            min = dis;
            mp = vecSpt[i];
        }

    }

    ret.pcode = mp.pcode;
    ret.x = mp.x;
    ret.y = mp.y;
    return ret;
    */
}

// 计算两点距离
double WifiLocation::calTwoPointDistance(double x1, double y1, double x2, double y2)
{
   return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
