
#include "libffmpegthumbnailer/videothumbnailer.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include <jni.h>
#include <android/log.h>
#include "Log.h"

extern "C" {
#include "libavutil/log.h"
#include "cmd/ffmpeg.h"
#include "video_edit/Compress.h"
#include "video_edit/Cut.h"
}
using namespace std;
using namespace ffmpegthumbnailer;

std::string jstringTostring(JNIEnv *env, jstring jstr);


void log_callback(void *ptr, int level, const char *fmt,
                  va_list vl) {
    __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_initFFmpeg(JNIEnv *env, jobject type, jboolean debug) {
    JNI_DEBUG = debug;
    if (JNI_DEBUG) {
        av_log_set_callback(log_callback);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_ffmpegRun(JNIEnv *env, jobject type,
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
JNIEXPORT jint JNICALL Java_com_zuga_ffmpeg_FFmpegUtil_getThumb(
        JNIEnv *env, jobject /* Jni object */,
        jstring inputFile, jstring outputFile, jstring seekTime) {
    int thumbnailSize = 128;
    int imageQuality = 3;
    bool workaroundIssues = false;
    bool maintainAspectRatio = true;
    bool smartFrameSelection = false;
    try {
        VideoThumbnailer videoThumbnailer(thumbnailSize, workaroundIssues,
                                          maintainAspectRatio, imageQuality,
                                          smartFrameSelection);
        videoThumbnailer.setSeekTime(jstringTostring(env, seekTime));
        videoThumbnailer.generateThumbnail(jstringTostring(env, inputFile), Jpeg,
                                           jstringTostring(env, outputFile));
    } catch (exception &e) {
        return (jint) -1;
    } catch (...) {
        return (jint) -1;
    }
    return 0;
}

//Java调用剪切
extern "C"
JNIEXPORT void JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_clipVideo(JNIEnv *env, jobject instance,
                                          jdouble startTime, jdouble endTime,
                                          jstring inFileName_, jstring outFileName_) {

    const char *inFileName = env->GetStringUTFChars(inFileName_, 0);
    const char *outFileName = env->GetStringUTFChars(outFileName_, 0);

    int result = cut_video((float) startTime, (float) endTime, inFileName, outFileName);
    if (result == 0) {
        LOGD("Clip video finished");
    } else {
        LOGD("Clip video failed");
    }
    env->ReleaseStringUTFChars(inFileName_, inFileName);
    env->ReleaseStringUTFChars(outFileName_, outFileName);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zuga_ffmpeg_FFmpegUtil_compress(JNIEnv *env, jobject instance, jstring inFile,
                                         jstring outFile) {
    const char *inPath = env->GetStringUTFChars(inFile, 0);
    const char *outPath = env->GetStringUTFChars(outFile, 0);
    int i = compress(inPath, outPath);
}

std::string jstringTostring(JNIEnv *env, jstring jstr) {
    const char *c_str = NULL;
    c_str = env->GetStringUTFChars(jstr, NULL);
    std::string stemp(c_str);
    env->ReleaseStringUTFChars(jstr, c_str);
    return stemp;
}


