#include "libffmpegthumbnailer/videothumbnailer.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include <android/log.h>
#include <jni.h>

#define LOG_TAG "System.out.c"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace std;
using namespace ffmpegthumbnailer;

std::string jstringTostring(JNIEnv *env, jstring jstr);

string int2str(const int &int_temp);

extern "C"
JNIEXPORT jint JNICALL Java_com_zuga_ffmpeg_FFmpegUtil_getThumbs(
        JNIEnv *env, jobject /* Jni object */,
        jstring inputFile, jstring outputDir, jint totalTime) {
    LOGD("start time");
    int thumbnailSize = 128;
    int imageQuality = 8;
    bool workaroundIssues = false;
    bool maintainAspectRatio = true;
    bool smartFrameSelection = false;
    int h = 0;
    int m = 0;
    int s = -1;
    for (int i = 0; i < totalTime; ++i) {
        s++;
        if (s >= 60) {
            s = 0;
            m++;
        }
        if (m >= 60) {
            m = 0;
            h++;
        }
        string time = "";
        time.append(int2str(h) + ":" + int2str(m) + ":" + int2str(s));
        LOGD("time: %s", time.c_str());
        try {
            VideoThumbnailer videoThumbnailer(thumbnailSize, workaroundIssues,
                                              maintainAspectRatio, imageQuality,
                                              smartFrameSelection);
            videoThumbnailer.setSeekTime(time);
            string outFile = jstringTostring(env, outputDir);
            std::ostringstream o;
            o << "/" << i << ".jpg";
            outFile += o.str();
            videoThumbnailer.generateThumbnail(jstringTostring(env, inputFile), Jpeg, outFile);
        } catch (exception &e) {
            return (jint) -1;
        } catch (...) {
            return (jint) -1;
        }
    }
    LOGD("end time");
    return (jint) 0;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_zuga_ffmpeg_FFmpegUtil_getThumb(
        JNIEnv *env, jobject /* Jni object */,
        jstring inputFile, jstring outputFile, jstring seekTime) {
    int thumbnailSize = 128;
    int imageQuality = 8;
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
        LOGD("start");
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

string int2str(const int &int_temp) {
    stringstream stream;
    stream << int_temp;
    return stream.str();
}

