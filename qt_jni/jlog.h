#include <android/log.h>

#ifndef PAKA_LOG_H
#define PAKA_LOG_H

#define  LOG_TAG    "paka_log"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif
