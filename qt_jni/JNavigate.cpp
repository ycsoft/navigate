//#include "defines.h"
//#include "navigate.h"
#include "../src/navigatelib.h"
#include "navigate_Navigate.h"
#include "jlog.h"

// static Navigate global_nav;

JNIEXPORT jint JNICALL Java_me_bukp_pakamaplib_business_PakaCoreJniBridge_just4T(
		JNIEnv *env, jobject) {
	int t = 1;
	return (jint) t;
}

JNIEXPORT jobjectArray JNICALL Java_me_bukp_pakamaplib_business_PakaCoreJniBridge_loadPathInfo(
		JNIEnv *env, jobject, jstring filename) {
	jboolean bcopy = JNI_FALSE;
	const char *fname = (env)->GetStringUTFChars(filename, &bcopy);
	//global_nav.LoadPointsFile(fname);
	PointArray res = loadPathInfo(fname);

	//vector<Node*> points = global_nav.GetAllPoints();

	int pointnum = res.num;

	jobjectArray pointArray;

	jclass objclass = (env)->FindClass("java/lang/Object");
	//新建object数组
	pointArray = (env)->NewObjectArray(pointnum, objclass, 0);

	//获取java中的实例类
	jclass pointclass = (env)->FindClass("me/bukp/pakamaplib/business/Point");

	//ToDo : add id attribute of the point
	jfieldID xid = (env)->GetFieldID(pointclass, "x", "F");
	jfieldID yid = (env)->GetFieldID(pointclass, "y", "F");
	jfieldID attrid = (env)->GetFieldID(pointclass, "attr", "I");
	jfieldID type = (env)->GetFieldID(pointclass, "type", "I");

	for (int i = 0; i < pointnum; ++i) {
		jobject point = (env)->AllocObject(pointclass);
		(env)->SetFloatField(point, xid, res.pts[i].x);
		(env)->SetFloatField(point, yid, res.pts[i].y);
		(env)->SetIntField(point, attrid, res.pts[i].attr);
		(env)->SetIntField(point, type, res.pts[i].type);
		(env)->SetObjectArrayElement(pointArray, i, point);
		(env)->DeleteLocalRef(point);
	}

	(env)->DeleteLocalRef(pointclass);
	(env)->DeleteLocalRef(objclass);
	(env)->ReleaseStringUTFChars(filename, fname);
	delete res.pts;
	res.pts = 0;
	return pointArray;

}

JNIEXPORT jobjectArray JNICALL Java_me_bukp_pakamaplib_business_PakaCoreJniBridge_getBestPath(
		JNIEnv *env, jobject, jobject start, jobject end) {
	LOGD("ENTER");
	jobjectArray resultPath;
	//static Node nstart,nend;
	static NavPoint nstart, nend;
	LOGD("11111");
	//获取起点和终点
	jclass pointClass = (env)->FindClass("me/bukp/pakamaplib/business/Point");

	jfieldID idId = (env)->GetFieldID(pointClass, "id", "I");
	jfieldID xid = (env)->GetFieldID(pointClass, "x", "F");
	jfieldID yid = (env)->GetFieldID(pointClass, "y", "F");
	jfieldID attrId = (env)->GetFieldID(pointClass, "attr", "I");
	jfieldID type = (env)->GetFieldID(pointClass, "type", "I");

	int sid = (env)->GetIntField(start, idId);
	int eid = (env)->GetIntField(end, idId);
	float fsx = (env)->GetFloatField(start, xid);
	float fsy = (env)->GetFloatField(start, yid);
	float fex = (env)->GetFloatField(end, xid);
	float fey = (env)->GetFloatField(end, yid);
	int sattr = (env)->GetIntField(start, attrId);
	int eattr = (env)->GetIntField(end, attrId);

	nstart.id = sid;
	nstart.x = fsx;
	nstart.y = fsy;
	nstart.attr = sattr;

	nend.id = eid;
	nend.x = fex;
	nend.y = fey;
	nend.attr = eattr;
	LOGD("22222, start %d-%f-%f-%d, end %d-%f-%f-%d", sid, fsx, fsy, sattr, eid,
			fex, fey, eattr);
	PointArray res = getBestPath(&nstart, &nend);
	LOGD("33333");
	jclass objclass = (env)->FindClass("java/lang/Object");

	resultPath = (env)->NewObjectArray(res.num, objclass, 0);

	//list<Node*>::iterator it = res.begin();
	int count = res.num;
	LOGD("44444 %d", count);
	int i = 0;
	while (i < count) {
		jobject node = (env)->AllocObject(pointClass);

		(env)->SetIntField(node, idId, res.pts[i].id);
		(env)->SetFloatField(node, xid, res.pts[i].x);
		(env)->SetFloatField(node, yid, res.pts[i].y);
		(env)->SetIntField(node, attrId, res.pts[i].attr);
		(env)->SetIntField(node, type, res.pts[i].type);
		(env)->SetObjectArrayElement(resultPath, i, node);
		(env)->DeleteLocalRef(node);
		++i;
	}
	LOGD("55555");
	(env)->DeleteLocalRef(pointClass);
	(env)->DeleteLocalRef(objclass);
	delete res.pts;
	res.pts = 0;
	return resultPath;

}
