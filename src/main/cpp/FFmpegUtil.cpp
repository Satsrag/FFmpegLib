
#include <jni.h>
#include <android/log.h>
#include "Log.h"

extern "C" {
#include "libavutil/log.h"
#include "cmd/ffmpeg.h"
#include "video_edit/Compress.h"
#include "video_edit/Progress.h"
}
using namespace std;

void log_callback(void *ptr, int level, const char *fmt,
                  va_list vl) {
//    __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_initFFmpeg(JNIEnv *env, jclass type, jboolean debug) {
    JNI_DEBUG = debug;
    if (JNI_DEBUG) {
//        av_log_set_callback(log_callback);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_ffmpegRun(JNIEnv *env, jclass type,
                                          jobjectArray commands) {
    int argc = env->GetArrayLength(commands);
    char *argv[argc];
    int i;
    for (i = 0; i < argc; i++) {
        jstring js = (jstring) env->GetObjectArrayElement(commands, i);
        argv[i] = (char *) env->GetStringUTFChars(js, 0);
    }
    return cmdRun(argc, argv);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_ffmegCompress(JNIEnv *env, jclass type, jint inFd, jlong offset,
                                              jlong length, jint outFd, jlong videoBitRate,
                                              jlong audioBitRate, jint width, jint height,
                                              jint videoID) {
    setJni(env);
    int ret = compress(inFd, offset, length, outFd, videoBitRate, audioBitRate, width, height,
                       videoID);
    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_cancelFFmpegCompress(JNIEnv *env, jclass instance) {
    cancelCompress();
}