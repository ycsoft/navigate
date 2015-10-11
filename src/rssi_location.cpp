#include "rssi_location.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <string>

RssiLocation::RssiLocation()
{

}

RssiLocation::~RssiLocation()
{

}

/*!
 * \brief 加载信号数据文件, 信号则包括了wifi信号和蓝牙信号
 * 数据文件 ,文件形式:二进制文件，每栋楼一个文件
 *
 * 楼宇编号(32字节) 楼层数
 *
 * 楼层编号(32字节) 楼层的自然层序号 楼层总的SID数 (SID指wifi的mac地址,或者蓝牙的唯一序号)
 * （ 首先是每个楼层总的 SID 列表 ）
 * SID序号 SID地址(mac地址是12字节, 蓝牙.. 暂时还没有处理蓝牙的啊) rssi最强强度值（有符号int型4字节）最强强度点编号（有符号int型4字节）
 * ......
 * （然后是每个采集点的信息）
 * 楼层总的采集点数
 * 采集点点编号 x坐标 y坐标 采集点采集的SID个数
 * SID序号 rssi强度值
 *
 * （接下来再进行楼层循环）
 *
 * 其中数量类为4字节,int类型;坐标点为double类型
 *
 * \param filepath 文件路径
 */
int RssiLocation::LoadSignalFile(const char *filepath)
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
        FloorSignalInfo fwi;
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
                    m_building_signal_info.clear();
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
        m_building_signal_info.insert(make_pair(fwi.floor_code, fwi));
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
int RssiLocation::addMacToAppearPointCodeMapList(string mac, int point_code, map<string, list<int> > &mac_point_code_map)
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
string RssiLocation::LocationBuildingFloor(RealTimeSignal* fingers[], int size)
{
    // 分别计算每一个楼层，取匹配数目最高
    // 遍历map
    string max_matched_floor_code;
    int max_matched = -1;
    map<string, FloorSignalInfo>::iterator iter = m_building_signal_info.begin();
    for ( ; iter != m_building_signal_info.end(); ++iter)
    {
        string floor_code = iter->first;
        FloorSignalInfo &finfo = iter->second;
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
int RssiLocation::calSimilarityInBuildingFloor(RealTimeSignal* fingers[], int size, FloorSignalInfo &finfo)
{
    map<string, MacListItem> &all_mac_map = finfo.all_mac_map;
    if (all_mac_map.size() == 0)
    {
        return 0;
    }
    int matched = 0;
    for (int i = 0; i < size; i++)
    {
        RealTimeSignal* f = fingers[i];
        string fmac = f->mac;
        map<string, MacListItem>::iterator iter = all_mac_map.find(fmac);
        if (iter != all_mac_map.end())
        {
            matched++;
        }
    }
    return matched;
}


LPoint RssiLocation::LocationFloorPoint_SCM_11(const char* floor_code, RealTimeSignal* realdata[], int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_simi_type_11);
}

LPoint RssiLocation::LocationFloorPoint_SCM_12(const char* floor_code, RealTimeSignal* realdata[], int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_simi_type_12);
}

LPoint RssiLocation::LocationFloorPoint_SCM_21(const char* floor_code, RealTimeSignal* realdata[], int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_simi_type_21);
}

LPoint RssiLocation::LocationFloorPoint_SCM_22(const char* floor_code, RealTimeSignal* realdata[], int size)
{
    return LocationFloorPoint(floor_code, realdata, size, enum_simi_type_22);
}

/*!
 * \brief judge which point am I in the floor
 * \param floor_code 楼层编码
 * \param fingers 传入的wifi data
 * \param size 指纹容量
 * \return 定位点结构体
 */
LPoint RssiLocation::LocationFloorPoint(const char* floor_code, RealTimeSignal* realdata[], int size, SimilarityCalType calType)
{
    LPoint p;
    p.x = -255.0;
    p.y = -255.0;
    p.pcode = -1;

    // 根据楼层编号得到wifi数据
    string fcode = floor_code;
    map<string, FloorSignalInfo>::iterator iter = m_building_signal_info.find(fcode);
    if (iter == m_building_signal_info.end())
    {
        // location error
        return p;
    }
    FloorSignalInfo &floor_wifi_info = iter->second; // 楼层wifi

    // 先挑选出可能的点，缩小比较范围
    set<int> possible_pcode_set = calPossiblePoints(realdata, size, floor_wifi_info.mac_point_code_map);

    // 得到按照相似度排列的点列表，可选择不同的相似度计算方式
    vector<SPointTemp> sptl = getSimilarPointCodeList(realdata, size, floor_wifi_info.finger_map, possible_pcode_set, calType);

    switch (calType) {
    case enum_simi_type_11:
    case enum_simi_type_21:
        p = calFloorPointLocation(sptl);
        break;
    case enum_simi_type_12:
    case enum_simi_type_22:
        p =  calFloorPointLocation(realdata, size, sptl, floor_wifi_info.finger_map, floor_wifi_info.all_mac_map);
    default:
        break;
    }

    // 上面得到的只有XY坐标了，根据XY坐标，得到离哪个关键点最近
    SPointTemp mindp = calMinDistancePoint(sptl, p.x, p.y);
    p.pcode = mindp.pcode;
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
set<int> RssiLocation::calPossiblePoints(RealTimeSignal* realdata[], int size, map<string, list<int> > &mac_point_code_map)
{
    set<int> ret;
    for (int i = 0; i < size; i++)
    {
        RealTimeSignal* f = realdata[i];
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
Similarities RssiLocation::calSimilarityInFloorGatherCode(RealTimeSignal* realdata[], int size, GatherFingerInfo &ginfo)
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
        RealTimeSignal* f = realdata[i];
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
float RssiLocation::calSimilarity_M2(int &rssi_in, int &rssi_f)
{
    float k1 = (float)rssi_in - (float)rssi_f;
    float k2 = k1 / (float)rssi_f;
    float k3 = abs(k2);
    float k4 = 1.0f - k3;
    return k4;
}

// 相似度比较方法3
// 在方法2的基础上 s3 = s2 * (now mac 顺序序号)^(-1/3)
float RssiLocation::calSimilarity_M3(int &rssi_in, int &rssi_f, int index_f)
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
int RssiLocation::getMostSimilarPointCode(RealTimeSignal* fingers[],
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
            case enum_simi_type_11:
            case enum_simi_type_12:
                simi = ret.s1;
                break;
            case enum_simi_type_21:
            case enum_simi_type_22:
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
vector<SPointTemp> RssiLocation::getSimilarPointCodeList(RealTimeSignal* fingers[],
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
            case enum_simi_type_11:
                spt.simi = ret.s1;
                break;
            case enum_simi_type_12:
                spt.simi = ret.s1;
                break;
            case enum_simi_type_21:
                spt.simi = ret.s3;
                break;
            case enum_simi_type_22:
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
LPoint RssiLocation::calFloorPointLocation(vector<SPointTemp> vecSpt)
{
    LPoint ret;
    ret.pcode = -1;
    if (vecSpt.size() == 0)
    {
        return ret;
    }

    double x = 0.0f;
    double y = 0.0f;

    int i = 0;
    // 前三点求平均
    for ( ; i < 3 && i < (int)vecSpt.size(); ++i)
    {
        x = x + vecSpt[i].x;
        y = y + vecSpt[i].y;
    }

    ret.x = x / (float)i;
    ret.y = y / (float)i;

    return ret;
}


// 根据距离的排序函数
bool disCompare(const CalTemp &arg1, const CalTemp &arg2)
{
    return arg1.d > arg2.d;
}


/*!
 * \brief 比较复杂的计算坐标点的方法
 * \param fingers 当前实时采集的指纹
 * \param size 实时采集指纹的数组大小
 * \param vecSpt 上一步计算出的，按相似度大小排列的点信息
 * \param finger_map 所有的指纹信息
 * \param max_rssi_point_map 每个mac地址的最强强度点信息
 * \return
 */
LPoint RssiLocation::calFloorPointLocation(RealTimeSignal *fingers[], int size, vector<SPointTemp> vecSpt,
                                           map< int, GatherFingerInfo > &finger_map_info,
                                           map<string, MacListItem> &max_rssi_point_map)
{
    LPoint ret;
    ret.pcode = -1;
    if (vecSpt.size() == 0 || size == 0)
    {
        return ret;
    }

    // 相似度最高点的X坐标和Y坐标
    double x0 = 0.0f;
    double y0 = 0.0f;

    // 相似度最高点
    SPointTemp& bestSimPoint = vecSpt[0];
    // 得到相似度最高点的指纹信息
    map< int, GatherFingerInfo >::iterator iter = finger_map_info.find(bestSimPoint.pcode);
    if (iter == finger_map_info.end())
    {
        return ret;
    }

    GatherFingerInfo& bestSimPointFingerInfo = iter->second;
    x0 = bestSimPointFingerInfo.p.x;
    y0 = bestSimPointFingerInfo.p.y;
    map<string, GatherFingerItem> &fingers_map = bestSimPointFingerInfo.fingers_map;

    // 现在实时的指纹列表，和相似度最高点的指纹列表，提取出一样的mac地址

    vector<CalTemp> calTempVec;

    // 遍历当前采集到的指纹列表
    for (int i = 0; i < size; ++i)
    {
        RealTimeSignal *cf = fingers[i];
        // 判断mac地址，在最相似点上是否出现过
        map<string, GatherFingerItem>::iterator iter2 = fingers_map.find(cf->mac);
        if (iter2 == fingers_map.end())
        {
            continue;
        }
        GatherFingerItem &bestSimPointFingerItem = iter2->second;

        // 最强强度点信息
        map<string, MacListItem>::iterator iter3 = max_rssi_point_map.find(cf->mac);
        if (iter3 == max_rssi_point_map.end())
        {
            continue;
        }
        MacListItem &maxMacRssiPointInfo = iter3->second;

        // 最强强度点编号已获得，拿到XY坐标
        iter = finger_map_info.find(maxMacRssiPointInfo.gather_point_code);
        if (iter == finger_map_info.end())
        {
            continue;
        }
        GatherFingerInfo &maxMacRssiPointFingerInfo = iter->second;

        // 这个mac地址，在相似度最高点的指纹列表中
        CalTemp caltemp;
        caltemp.mac = cf->mac;
        caltemp.curRssi = cf->rssi;
        caltemp.bestSimPointRssi = bestSimPointFingerItem.rssi;
        caltemp.maxRssi = maxMacRssiPointInfo.max_rssi;
        caltemp.maxX = maxMacRssiPointFingerInfo.p.x;
        caltemp.maxY = maxMacRssiPointFingerInfo.p.y;

        // 计算
        //  比值1 = rssi now 1 / rssi 最相似 1
        //  x1 = （1 - 比值1） * （x1）+ (比值1 * x0)
        //  y1 = （1 - 比值1） * （y1）+ (比值1 * y0)
        caltemp.ft = (float)cf->rssi / (float)bestSimPointFingerItem.rssi;
        caltemp.hd_x = (1 - caltemp.ft) * caltemp.maxX + (caltemp.ft * x0);
        caltemp.hd_y = (1 - caltemp.ft) * caltemp.maxY + (caltemp.ft * y0);
        caltemp.d = calTwoPointDistance(caltemp.hd_x, caltemp.hd_y, x0, y0);
        calTempVec.push_back(caltemp);
    }

    // 如果i<=3直接做平均, xi yi分别求平均，得到坐标。。 end

    // 如果i>3
    // 求出d1 d2 d3..di （xi,yi) (x0, y0)距离，并排序
    // 排序  大到小
    sort(calTempVec.begin(), calTempVec.end(), disCompare);

    // 此时已排序好
    // 避免死循环
    for(int i = 0; i < 100; ++i)
    {
        int c = calTempVec.size();
        if (c <= 3)
        {
            break;
        }

        // 拿出最大和最小，求平均值
        CalTemp &tempDMax = calTempVec[0];
        CalTemp &tempDMin = calTempVec[c - 1];

        double d_avg = (tempDMax.d + tempDMin.d) / 2.0;

        // 对d求标准差 σd
        double e = calDistanceStandardDeviation(calTempVec);
        double t1 = abs(tempDMin.d - d_avg) / e;
        double tn = abs(tempDMax.d - d_avg) / e;
        double ta = 0.00104 * pow(c, 3.0) - 0.03903 * pow (c, 2.0)+ 0.54014 * c - 0.12437;

        if (ta > t1 && ta > tn)
        {
            break;
        }

        if (t1 >= ta && tn >= ta)
        {
            calTempVec.erase(calTempVec.begin());
        }
        else if ( t1 >= ta)
        {
            calTempVec.erase(calTempVec.end() - 1);

        }
        else if (tn >= ta)
        {
            calTempVec.erase(calTempVec.begin());
        }

        if (t1 < ta && tn > ta)
        {
            break;
        }
    }

    int vecSize = calTempVec.size();
    double sumX = 0.0f;
    double sumY = 0.0f;

    vector<CalTemp>::const_iterator vecIter = calTempVec.begin();
    for ( ; vecIter != calTempVec.end(); ++vecIter)
    {
        CalTemp item = *vecIter;
        sumX += item.hd_x;
        sumY += item.hd_y;
    }

    ret.x = sumX / (float)vecSize;
    ret.y = sumY / (float)vecSize;
    return ret;
}

// 计算距离的标准偏差
double RssiLocation::calDistanceStandardDeviation(vector<CalTemp> &data)
{
    // 求D平均值
    double sum = 0.0f;
    int n = data.size();

    for (int i = 0; i < n; ++i)
    {
        sum = sum + data[i].d;
    }

    double avg = sum / (float)n;

    double k = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        k = k + (data[i].d - avg) * (data[i].d - avg);
    }
    double k2 = k / (float)n;
    double k3 = sqrt(k2);
    return k3;
}

SPointTemp RssiLocation::calMinDistancePoint(vector<SPointTemp> &points, double x, double y)
{
    SPointTemp ret;
    double dmin = -99.0f;
    for (int i = 0; i < (int)points.size(); ++i)
    {
        SPointTemp &item = points[i];
        double d = calTwoPointDistance(item.x, item.y, x, y);
        if (dmin < 0 || dmin > d)
        {
            dmin = d;
            ret = points[i];
        }
    }
    return ret;
}


