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
string WifiLocation::LocationBuildingFloor(RealTimeFinger* fingers[], int size)
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
int WifiLocation::calSimilarityInBuildingFloor(RealTimeFinger* fingers[], int size, FloorWifiInfo &finfo)
{
    map<string, MacListItem> &all_mac_map = finfo.all_mac_map;
    if (all_mac_map.size() == 0)
    {
        return 0;
    }
    int matched = 0;
    for (int i = 0; i < size; i++)
    {
        RealTimeFinger* f = fingers[i];
        string fmac = f->mac;
        map<string, MacListItem>::iterator iter = all_mac_map.find(fmac);
        if (iter != all_mac_map.end())
        {
            matched++;
        }
    }
    return matched;
}

// 常规方式计算相似度
LPoint WifiLocation::LocationFloorPoint_SCM_Normal(const char* floor_code,
                          RealTimeFinger* realdata[],
                          int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_Normal);
}

// 相似度计算方法2
LPoint WifiLocation::LocationFloorPoint_SCM_M2(const char* floor_code,
                          RealTimeFinger* realdata[],
                          int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_M2);
}

// 相似度计算方法3
LPoint WifiLocation::LocationFloorPoint_SCM_M3(const char* floor_code,
                          RealTimeFinger* realdata[],
                          int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_M3);
}


/*!
 * \brief judge which point am I in the floor
 * \param floor_code 楼层编码
 * \param fingers 传入的wifi data
 * \param size 指纹容量
 * \return 定位点结构体
 */
LPoint WifiLocation::LocationFloorPoint(const char* floor_code,
                                        RealTimeFinger* realdata[],
                                        int size,
                                        SimilarityCalType calType)
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
    FloorWifiInfo &floor_wifi_info = iter->second; // 楼层wifi


    // 先挑选出可能的点，缩小比较范围
    set<int> possible_pcode_set = calPossiblePoints(realdata, size, floor_wifi_info.mac_point_code_map);

    // 得到按照相似度排列的点列表，可选择不同的相似度计算方式
    vector<SPointTemp> sptl = getSimilarPointCodeList(realdata, size,
                                                      floor_wifi_info.finger_map,
                                                      possible_pcode_set,
                                                      calType);
    // 根据相似的点列表，计算定位点
    p = calFloorPointLocation(sptl);

    // 一些其他工作
    p.pcode = floorPCodeToGlobePCode(p.pcode, floor_wifi_info.floor_number);
    p.floor_code = floor_wifi_info.floor_code.c_str();
    return p;
}

/*!
 * \brief 计算可能的点，缩小进行相似度比较的采集点范围
 *          先挑选出可能的点，缩小比较范围
 *          例如，实时的wifi列表中有A B C D四个mac地址
 *          mac A 在 p1 p2 p3点出现过
 *          mac B 在 p1 p3 p4点出现过
 *          mac C 在 p2 p3 p5点出现过
 *          mac D 在 p3 p5 p6出现过
 *          那么，则返回上述的点
 *          下一步进行相似度计算的时候，则只需要在这些点中进行比较
 * \param fingers
 * \param size
 * \param finger_map
 * \return
 */
set<int> WifiLocation::calPossiblePoints(RealTimeFinger* realdata[], int size, map<string, list<int> > &mac_point_code_map)
{
    set<int> ret;
    for (int i = 0; i < size; i++)
    {
        RealTimeFinger* f = realdata[i];
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
 *          方法1：仅比较mac地址是否匹配，匹配个数越多，相似度越高
 * \param fingers 传入的wifi data
 * \param size input wifi data length
 * \param ginfo gather point wifi info
 * \return Similarities
 */
Similarities WifiLocation::calSimilarityInFloorGatherCode(RealTimeFinger* realdata[], int size, GatherFingerInfo &ginfo)
{
    map<string, GatherFingerItem> &fingers_map = ginfo.fingers_map;
    Similarities ret;
    if (fingers_map.size() == 0)
    {
        return ret;
    }

    // 分别为三种方式求得的相似度
    int s1 = 0; // 方法1，有匹配的mac地址则加1
    float s2 = 0.0f; // 方法2，有匹配的mac地址则加  1 - |((rssi_now - rssi_old) / rssi_old)|
    float s3 = 0.0f; // 方法3，在方法2的基础上 s3 = s2 * (now mac 顺序序号)^(-1/3)

    for (int i = 0; i < size; i++)
    {
        RealTimeFinger* f = realdata[i];
        string fmac = f->mac;
        map<string, GatherFingerItem>::iterator iter = fingers_map.find(fmac);
        if (iter != fingers_map.end())
        {
            GatherFingerItem &item = iter->second;
            // 找到了是相同的mac
            ++s1;
            s2 = s2 + calSimilarity_M2(f->rssi, item.rssi);
            s3 = s3 + calSimilarity_M3(f->rssi, item.rssi, i+1);
        }
    }

    ret.s1 = s1;
    ret.s2 = s2;
    ret.s3 = s3;

    return ret;
}

// 相似度比较方法2
// 在mac地址一致的情况下，matched 加入式子中给的值
// 有匹配的mac地址则加  1 - |((rssi_now - rssi_old) / rssi_old)|
float WifiLocation::calSimilarity_M2(int &rssi_in, int &rssi_f)
{
    float k1 = (float)rssi_in - (float)rssi_f;
    float k2 = k1 / (float)rssi_f;
    float k3 = abs(k2);
    float k4 = 1.0f - k3;
    return k4;
}

// 相似度比较方法3
// 在方法2的基础上 s3 = s2 * (now mac 顺序序号)^(-1/3)
float WifiLocation::calSimilarity_M3(int &rssi_in, int &rssi_f, int index_f)
{
    float k1 = calSimilarity_M2(rssi_in, rssi_f);
    double k2 = pow(index_f, 1.0f / 3.0f);
    return (float)(k1*k2);
}

/*!
 * \brief 得到最相似点
 * \param fingers 传入的wifi data
 * \param size input wifi data length
 * \param finger_map 指纹map
 * \param points 待比较的目标点
 * \return
 */
int WifiLocation::getMostSimilarPointCode(RealTimeFinger* fingers[],
                                          int size,
                                          map< int, GatherFingerInfo > &finger_map,
                                          set<int> &points,
                                          SimilarityCalType calType
                                          )
{
    set<int>::iterator iter = points.begin();
    int most_simi_code = -1;
    float max_simi = -1.0;

    for ( ; iter != points.end(); ++iter)
    {
        int pcode = *iter;
        map< int, GatherFingerInfo >::iterator iter2 = finger_map.find(pcode);
        if (iter2 != finger_map.end())
        {
            GatherFingerInfo &info = iter2->second;
            // 得到最相似点，此处需按照不同的算法得到不同的点

            Similarities ret = calSimilarityInFloorGatherCode(fingers, size, info);
            float simi = 0.0f;
            switch (calType) {
            case enum_Normal:
                simi = ret.s1;
                break;
            case enum_M2:
                simi = ret.s2;
                break;
            case enum_M3:
                simi = ret.s3;
                break;
            default:
                break;
            }
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
vector<SPointTemp> WifiLocation::getSimilarPointCodeList(RealTimeFinger* fingers[],
                                                         int size, map< int, GatherFingerInfo > &finger_map,
                                                         set<int> &points,
                                                         SimilarityCalType calType)
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

            Similarities ret = calSimilarityInFloorGatherCode(fingers, size, info);
            SPointTemp spt;
            spt.x = info.p.x;
            spt.y = info.p.y;

            spt.simi = 0;
            switch (calType) {
            case enum_Normal:
                spt.simi = ret.s1;
                break;
            case enum_M2:
                spt.simi = ret.s2;
                break;
            case enum_M3:
                spt.simi = ret.s3;
                break;
            default:
                break;
            }
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

    // m1 最相似点坐标

    // m2 前三点平均

    // m3  x0,y0 相似度最高点的坐标

    //  now 指纹列表     相似度最高点的指纹列表
    //  提取出一样的mac地址

    //  mac1 now rssi 最相似点rssi mac1地址最强强度 最强强度出现的xy坐标 (x1,y1)
    //  mac2 now rssi 最相似点rssi mac2地址最强强度 最强强度出现的xy坐标 (x2,y2)
    //  mac3 now rssi 最相似点rssi mac3地址最强强度 最强强度出现的xy坐标 (x3,y3)

    //  比值1 = rssi now 1 / rssi 最相似 1
    //  比值2 = rssi now 2 / rssi 最相似 2
    //  比值3 = rssi now 3 / rssi 最相似 3

    //  x1 = （1 - 比值1） * （x1）+ (比值1 * x0)
    //  y1 = （1 - 比值1） * （y1）+ (比值1 * y0)

    //  x2 = （1 - 比值2） * （x2）+ (比值2 * x0)
    //  y2 = （1 - 比值2） * （y2）+ (比值2 * y0)

    //  x3 = （1 - 比值3） * （x3）+ (比值3 * x0)
    //  y3 = （1 - 比值3） * （y3）+ (比值3 * y0)

    // 如果i<=3直接做平均, xi yi分别求平均，得到坐标。。 end

    // 如果i>3

    // 求出d1 d2 d3..di （xi,yi) (x0, y0)距离

    // 排序  大到小
    //  d2   (x2,y2)
    //  d3   (x3,y3)
    //  d1   (x1,y1)
    //  d4   (x4,y4)
    //  d5   (x5,y5)


// do
    // 拿出最大 d2, 最小d5
    // 平均值 davg
    // 求统计标准差
    // 对d求标准差 σd
    //  t1 = abs(d_min - davg)/σd
     // tn = abs(d_max - davg)/σd
    // count_d
    // ta = 0.00104*count_d**3.0 - 0.03903*count_d**2.0
    //     + 0.54014*count_d   - 0.12437

       // if(t1 >= ta) {
            // 删d2
       // } else if (tn >=ta) {
       //   // 删d5
       //}
    //

    //if (t1 < ta && tn < ta) break;
    //if(count_d<=3) break;
// while

 // 得到x y列表，求平均

}

// 计算两点距离
double WifiLocation::calTwoPointDistance(double x1, double y1, double x2, double y2)
{
   return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
