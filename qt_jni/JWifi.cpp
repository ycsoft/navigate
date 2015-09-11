//#include "defines.h"
//#include "navigate.h"
#include "paka_api.h"
#include "me_bukp_testrssi_JniBridge.h"
#include "jlog.h"

JNIEXPORT jint JNICALL Java_me_bukp_testrssi_JniBridge_loadWifiFinger(
		JNIEnv *env, jclass, jstring filename) {
	LOGD("Enter load wifi finger info");
	jboolean bcopy = JNI_FALSE;
	const char *fname = (env)->GetStringUTFChars(filename, &bcopy);
	LOGD("Get input param filename done, begin do load");
	jint ret = loadWifiInfo(fname);
	(env)->ReleaseStringUTFChars(filename, fname);
	LOGD("Load data with return value %d", ret);
	return ret;
}

jstring char2jstring(JNIEnv* pEnv, char* pChars, int Length, char* szCharset) {
	jclass clsString = pEnv->FindClass("java/lang/String");
	/// <init>属于构造函数的范畴
	jmethodID construct = pEnv->GetMethodID(clsString, "<init>",
			"([BLjava/lang/String;)V");
	jbyteArray byteArray = pEnv->NewByteArray(Length);
	pEnv->SetByteArrayRegion(byteArray, 0, Length, (jbyte*) pChars);
	jstring charset = pEnv->NewStringUTF(szCharset);
	jstring strDst = (jstring) pEnv->NewObject(clsString, construct, byteArray,
			charset);
	pEnv->DeleteLocalRef(byteArray);
	return strDst;
}

JNIEXPORT jobject JNICALL Java_me_bukp_testrssi_JniBridge_doLocate(
		JNIEnv *env, jclass, jstring jbssids) {
	LOGD("Enter do locate");
	jboolean bcopy = JNI_FALSE;
	const char *bssids = (env)->GetStringUTFChars(jbssids, &bcopy);
	LOGD("Get input param bssids done, begin do locate");
	WifiPoint wp = doLocate(bssids);
	(env)->ReleaseStringUTFChars(jbssids, bssids);
	LOGD("Do locate done, return p(id=%d,x=%f,y=%f)", wp.id, wp.x, wp.y);

	LOGD("Jni data prepare for return");
	// 构造数据返回
	jclass pointClass = (env)->FindClass("me/bukp/testrssi/WifiLocation");
	jobject retobj = (env)->AllocObject(pointClass);
	jfieldID idId = (env)->GetFieldID(pointClass, "id", "I");
	jfieldID xid = (env)->GetFieldID(pointClass, "x", "F");
	jfieldID yid = (env)->GetFieldID(pointClass, "y", "F");
	jfieldID floorCodeId = (env)->GetFieldID(pointClass, "fc", "Ljava/lang/String;");

	(env)->SetIntField(retobj, idId, wp.id);
	(env)->SetFloatField(retobj, xid, wp.x);
	(env)->SetFloatField(retobj, yid, wp.y);
	jstring jfloorcode = char2jstring(env, wp.floor_code, strlen(wp.floor_code), "UTF-8");

	(env)->SetObjectField(retobj, floorCodeId, jfloorcode);
	LOGD("Done.. return...");
	return retobj;
}
