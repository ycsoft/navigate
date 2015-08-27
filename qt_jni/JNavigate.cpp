
//#include "defines.h"
//#include "navigate.h"
#include "navigate_Navigate.h"

#include "navigatelib.h"

//static Navigate global_nav;

JNIEXPORT jobjectArray JNICALL Java_navigate_Navigate_loadPathInfo
  (JNIEnv *env, jobject, jstring filename)
{
    jboolean bcopy = JNI_FALSE;
    const char *fname = (env)->GetStringUTFChars(filename,&bcopy);
    //global_nav.LoadPointsFile(fname);
    PointArray res = loadPathInfo(fname);

    //vector<Node*> points = global_nav.GetAllPoints();

    int  pointnum = res.num;

    jobjectArray    pointArray;

    jclass objclass = (env)->FindClass("java/lang/Object");
    //新建object数组
    pointArray = (env)->NewObjectArray(pointnum,objclass,0);

    //获取java中的实例类
    jclass pointclass = (env)->FindClass("navigate/Point");

    //ToDo : add id attribute of the point
    jfieldID xid = (env)->GetFieldID(pointclass,"x","F");
    jfieldID yid = (env)->GetFieldID(pointclass,"y","F");
    jfieldID attrid = (env)->GetFieldID(pointclass,"attr","I");
    jfieldID type = (env)->GetFieldID(pointclass,"type","I");


    for ( int i = 0 ; i < pointnum; ++i)
    {
        jobject point  = (env)->AllocObject(pointclass);
        (env)->SetFloatField(point,xid,res.pts[i].x);
        (env)->SetFloatField(point,yid,res.pts[i].y);
        (env)->SetIntField(point,attrid,res.pts[i].attr);
        (env)->SetIntField(point,type,res.pts[i].type);
        (env)->SetObjectArrayElement(pointArray,i,point);
    }
    delete res.pts;
    res.pts = NULL;
    return pointArray;

}


JNIEXPORT jobjectArray JNICALL Java_navigate_Navigate_getBestPath
  (JNIEnv *env, jobject, jobject start, jobject end)

{
    jobjectArray    resultPath = 0;

    //static Node nstart,nend;
    static NavPoint nstart,nend;
    //获取起点和终点
    jclass pointClass = (env)->FindClass("navigate/Point");

    jfieldID  xid = (env)->GetFieldID(pointClass,"x","F");
    jfieldID  yid = (env)->GetFieldID(pointClass,"y","F");
    jfieldID  attrId = (env)->GetFieldID(pointClass,"attr","I");
    jfieldID  type = (env)->GetFieldID(pointClass,"type","I");

    float fsx = (env)->GetFloatField(start,xid),
          fsy = (env)->GetFloatField(start,yid),
          fex = (env)->GetFloatField(end,xid),
          fey = (env)->GetFloatField(end,yid);
    int   sattr = (env)->GetIntField(start,attrId),
          eattr = (env)->GetIntField(end,attrId);

    nstart.x = fsx;
    nstart.y = fsy;
    nstart.attr = sattr;

    nend.x = fex;
    nend.y = fey;
    nend.attr = eattr;

    //list<Node*> res = global_nav.GetBestPath(&nstart,&nend);

    PointArray res = getBestPath(&nstart,&nend);

    jclass objclass = (env)->FindClass("java/lang/Object");
    resultPath = (env)->NewObjectArray(res.num,objclass,0);

    //list<Node*>::iterator it = res.begin();
    int count = res.num;

    int i = 0;
    while ( i < count )
    {
        jobject node = (env)->AllocObject(pointClass);

        (env)->SetFloatField(node,xid,res.pts[i].x);
        (env)->SetFloatField(node,yid,res.pts[i].y);
        (env)->SetIntField(node,attrId,res.pts[i].attr);
        (env)->SetIntField(node,type,res.pts[i].type);
        (env)->SetObjectArrayElement(resultPath,i,node);
        ++i;
    }
    delete res.pts;
    res.pts = NULL;
    return resultPath;

}
