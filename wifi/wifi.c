
#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BYTES_IN_BUFFER 128

#define  GETS(buf,f)   fgets(buf,MAX_BYTES_IN_BUFFER,f)

#define  Write32Buf(buf,f)  fwrite(buf,1,32,f)
#define  WriteInt(buf,f)    fwrite(buf,sizeof(int),1,f)

typedef float Real;

wifi_finger* readWifiMap(const char *fname, int *nump, int *numwp)
{
    wifi_finger* result;
    FILE *wf = fopen(fname,"r");
    char buf[MAX_BYTES_IN_BUFFER] = {0};
    char *seps = "\t ";
    char * cstr = NULL;
    int i , j;
    int num = 0,innernum = 0;

    if ( NULL == wf )
        return NULL;
    fgets(buf,MAX_BYTES_IN_BUFFER,wf);
    fgets(buf,MAX_BYTES_IN_BUFFER,wf);
    //提取点数
    cstr = strtok(buf,seps);
    num = atoi(cstr);
    innernum = atoi(strtok(NULL,seps));

    (*nump) = num;
    (*numwp) = innernum;

    result = (wifi_finger*)malloc(sizeof(wifi_finger) * num);
    fgets(buf,MAX_BYTES_IN_BUFFER,wf);
    for ( i = 0 ; i < num; ++i)
    {
        fgets(buf,MAX_BYTES_IN_BUFFER,wf);
        for ( j = 0 ; j < innernum;++j)
        {
            GETS(buf,wf);
            cstr = strtok(buf,seps);
            result[i].wifi[j].id = atoi(cstr);
            cstr = strtok(NULL,seps);
            result[i].wifi[j].rssi = atoi(cstr);
        }
    }
    fclose(wf);
    return result;
}

wifi_list * readWifiList(const char *fname,int *count)
{
    FILE *wf = fopen(fname,"r");
    int  num,i;
    char *seps = " \t";
    char buf[MAX_BYTES_IN_BUFFER]= {0};
    char *str;
    wifi_list *result;

    if ( NULL == wf )
        return NULL;

    GETS(buf,wf);
    GETS(buf,wf);

    num = atoi(buf);

    if ( num <=0 )
        return NULL;
    result = (wifi_list*)malloc(sizeof(wifi_list)*num);
    (*count) = num;

    GETS(buf,wf);

    for ( i = 0 ; i<num; ++i )
    {
        GETS(buf,wf);
        str = strtok(buf,seps);
        strncpy(result[i].mac,str,strlen(str)+1);
        str = strtok(NULL,seps);
        result[i].rssi = atoi(str);
        str = strtok(NULL,seps);
        result[i].pt_max = atoi(str);
    }
    return result;

}

tagPoint* readTagPoints(const char *fname)
{
    FILE  *wf = fopen(fname,"r");
    int   num = 0,i = 0;
    char  *seps = " \t"
          ,*str =NULL;
    char  buf[MAX_BYTES_IN_BUFFER] = {0};

    tagPoint *result = NULL;

    if ( NULL == wf )
        return NULL;

    GETS(buf,wf);
    GETS(buf,wf);
    num = atoi(buf);
    if ( num <= 0)
        return NULL;

    result = (tagPoint*)malloc( sizeof(tagPoint) * num );
    GETS(buf,wf);
    for( i = 0 ; i < num ; ++i)
    {
        GETS(buf,wf);
        str = strtok(buf,seps);
        result[i].x = atoi(str);

        str = strtok(NULL,seps);
        result[i].y = atoi(str);

        printf("Point Tag:[%7.1lf,%7.1lf]\n",result[i].x,result[i].y);
    }

    return result;
}

point *readCurrentWifi(const char *fname)
{
    FILE *wf = fopen( fname,"r");
    char *seps = " \t";
    char buf[ MAX_BYTES_IN_BUFFER ] = {0};
    char *str = NULL;
    int i = 0;
    point* results = (point*)malloc(sizeof(point) * FINGER_COUNT);

    if ( wf == NULL)
        return NULL;
    GETS(buf,wf);

    for ( i = 0 ; i < FINGER_COUNT; ++i)
    {
        GETS( buf,wf );
        memset( results+i,0,sizeof(point) );
        str = strtok( buf,seps );
        strncpy( results[i].mac,str,strlen(str)+1);
        str = strtok(NULL,seps);
        results[i].rssi = atoi(str);
    }
    return results;

}

void flushfile(const char *fname, wifi_finger *finger, tagPoint *tag, int fcount, wifi_list *list, int count)
{
    FILE * f = fopen(fname,"wb");
    char buf[32] = "WIFI Data";
    char *cstr = "building-code-12342";
    int  t;

    if ( NULL == f)
        return;
    //title : 32 Bytes
    Write32Buf(buf,f);
    //build code: 32 Bytes
    Write32Buf(cstr,f);
    //floor count
    t = 1;
    WriteInt(&t,f);
    //cur floor code :32 bytes
    cstr = "floor-code-111";
    Write32Buf(cstr,f);
    //
}

void wifi()
{

    int     ncwfc = 0 , npmax = 0, ii = 0;
    int     i = 0 , j = 0,nump,numwp,numwf;
    int     npmaxi,nwfmax,rssmax;
    double  drarb = 0.0, sumx = 0.0, sumy = 0.0
            ,x = 0.0,y = 0.0;

    wifi_list       *list = readWifiList("list.out",&numwf);
    tagPoint        *tag = readTagPoints("ZB.IN");
    point           *pts = readCurrentWifi("data.in");
    wifi_finger     wfpmax;
    wifi_xy         xy_now[FINGER_COUNT];
    wifi_finger     *map = readWifiMap("map.out",&nump,&numwp);

    //计算指纹重合度,确定是否在当前楼层

    wifi_now *wfnow = (wifi_now*)malloc(sizeof(wifi_now) * numwp);
    for ( i = 0 ; i < numwp;++i)
    {

        for ( j = 0 ; j < numwf; ++j)
        {
            //MAC地址匹配，重合度累加
            if ( strcmp(pts[i].mac, list[j].mac) == 0 )
            {
                ++ncwfc;
                wfnow[i].listid = j;
                wfnow[i].rssi_cur = pts[i].rssi;
                wfnow[i].rssi_list = list[j].rssi;
                //strncpy(wfnow[i].mac,list[i].mac,strlen(list[i].mac)+1);
                wfnow[i].pt_max = list[j].pt_max;
                if ( ncwfc == 1)
                {
                    //基准指纹编号
                    npmax = list[j].pt_max;
                }
                break;
            }
        }
        //未找到匹配的MAC
        if ( j >= numwf )
        {
            wfnow[i].listid = -1;
            wfnow[i].rssi_cur = pts[i].rssi;
            wfnow[i].pt_max = 0;
            wfnow[i].rssi_list = 0;
        }
    }

    printf("当前点WIFI重合度: %5d\n",ncwfc);
    printf("当前点WIFI指纹转换-----OK\n");
    printf("npmax=%5d\n",npmax);

    if ( ncwfc == 0)
    {
        printf("当前位置不在支持区域\n");
    }

    ///////////////////////////////////////////////
    //计算当前点坐标
    printf("========================================\n");
    memcpy(&wfpmax,&map[npmax-1],sizeof(wifi_finger));
    for ( i = 0 ; i < numwp ;++i)
    {
        for ( j = 0; j < numwp; ++j)
        {
            if ( wfnow[i].listid == wfpmax.wifi[j].id-1)
            {

                nwfmax = wfnow[i].listid;
                rssmax = list[nwfmax].rssi;
                npmaxi = list[nwfmax].pt_max;
                xy_now[ii].listid = wfnow[i].listid;
                xy_now[ii].rssi_cur = wfnow[i].rssi_cur - rssmax;
                xy_now[ii].rssi_list = wfpmax.wifi[j].rssi - rssmax;
                xy_now[ii].x = tag[npmaxi-1].x;
                xy_now[ii].y = tag[npmaxi-1].y;

                printf("%8d,%8d\n",xy_now[ii].rssi_cur,xy_now[ii].rssi_list);
                ii += 1;
                break;
            }
        }
    }
    printf("========================================\n");
    sumx = xy_now[0].x;
    sumy = xy_now[0].y;
    for ( i = 1; i < ii; ++i)
    {
        if (  xy_now[i].rssi_list != 0 )
        {
            drarb = (double)xy_now[i].rssi_cur/xy_now[i].rssi_list;
            printf("drarb = %8.1lf\n",drarb);
            xy_now[i].x = (1.0-drarb)*xy_now[i].x + drarb * xy_now[0].x;
            xy_now[i].y = (1.0-drarb)*xy_now[i].y + drarb * xy_now[0].y;
        }
        sumx += xy_now[i].x;
        sumy += xy_now[i].y;
    }
    printf("%lf,%lf\n",sumx,sumy);
    x = sumx / ii;
    y = sumy / ii;

    printf("当前位置:[%8.1lf,%-8.1lf]\n",x,y);

    free(map);
    free(list);
    free(pts);
    free(tag);
    free(wfnow);
}
