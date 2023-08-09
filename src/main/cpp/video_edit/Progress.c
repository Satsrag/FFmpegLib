//
// Created by Saqrag Borgn on 10/08/2017.
//

#include "Progress.h"
#include "../Log.h"
#include <jni.h>

JNIEnv *compressEnv;
jclass pJclass;
jmethodID method;

void updateProgress(int videoId, float progress) {
    LOGD("updateProgress 1");
    (*compressEnv)->CallStaticVoidMethod(compressEnv, pJclass, method, videoId, progress);
    LOGD("updateProgress 2");
}

void setJni(JNIEnv *env) {
    compressEnv = env;
    pJclass = (*compressEnv)->FindClass(compressEnv, "com/zuga/ffmpeg/FFmpegUtil");
    if (pJclass == 0) return;
    method = (*compressEnv)->GetStaticMethodID(
            compressEnv, pJclass, "updateCompress",
            "(IF)V"
    );
    if (method == 0) return;
}

