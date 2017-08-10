//
// Created by Saqrag Borgn on 10/08/2017.
//

#include "Progress.h"
#include "../Log.h"
#include <jni.h>

JNIEnv *compressEnv;
jclass pJclass;
jmethodID method;

void updateProgress(char *inPath, float progress) {
    jstring inFile = (*compressEnv)->NewStringUTF(compressEnv, inPath);
    (*compressEnv)->CallStaticVoidMethod(compressEnv, pJclass, method, inFile, progress);
    (*compressEnv)->DeleteLocalRef(compressEnv, inFile);
}

void setJni(JNIEnv *env) {
    compressEnv = env;
    pJclass = (*compressEnv)->FindClass(compressEnv, "com/zuga/ffmpeg/FFmpegUtil");
    if (pJclass == 0) return;
    method = (*compressEnv)->GetStaticMethodID(
            compressEnv, pJclass, "updateCompress",
            "(Ljava/lang/String;F)V"
    );
    if (method == 0) return;
}

