//
// Created by Saqrag Borgn on 04/07/2017.
//

#ifndef SMALLVIDEOVIEW_LOG_H
#define SMALLVIDEOVIEW_LOG_H

#define TAG "FFmpegUtil"

#include <android/log.h>

#define LOGD(format, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, format, ## __VA_ARGS__)
#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, TAG, format, ## __VA_ARGS__)
#define LOGW(format, ...) __android_log_print(ANDROID_LOG_WARN, TAG, format, ## __VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, format, ## __VA_ARGS__)

//#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__)

extern int JNI_DEBUG;

#endif //SMALLVIDEOVIEW_LOG_H
