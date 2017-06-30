#include "libffmpegthumbnailer/videothumbnailer.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include "libffmpegthumbnailer/grayscalefilter.h"
#include "libffmpegthumbnailer/filmstripfilter.h"
#include <android/log.h>
#include <jni.h>

#define LOG_TAG "System.out.c"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace std;
using namespace ffmpegthumbnailer;

std::string jstringTostring(JNIEnv *env, jstring jstr);

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

std::string jstringTostring(JNIEnv *env, jstring jstr) {
    const char *c_str = NULL;
    c_str = env->GetStringUTFChars(jstr, NULL);
    std::string stemp(c_str);
    env->ReleaseStringUTFChars(jstr, c_str);
    return stemp;
}

