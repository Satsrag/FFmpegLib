LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libavcodec
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavformat
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavutil
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libavfilter
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libswresample
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libswscale
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := x264
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := aac
LOCAL_SRC_FILES := ../cpp/lib/${TARGET_ARCH_ABI}/libfdk-aac.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := native-lib
LOCAL_SRC_FILES :=../cpp/thumbnail_demo.c \
                         ../cpp/Thumb.cpp \
                         ../cpp/Thumb.h \
                         ../cpp/libffmpegthumbnailer/filmstrip.h \
                         ../cpp/libffmpegthumbnailer/filmstripfilter.cpp \
                         ../cpp/libffmpegthumbnailer/filmstripfilter.h \
                         ../cpp/libffmpegthumbnailer/grayscalefilter.h \
                         ../cpp/libffmpegthumbnailer/histogram.h \
                         ../cpp/libffmpegthumbnailer/ifilter.h \
                         ../cpp/libffmpegthumbnailer/imagetypes.h \
                         ../cpp/libffmpegthumbnailer/imagewriter.h \
                         ../cpp/libffmpegthumbnailer/imagewriterfactory.h \
                         ../cpp/libffmpegthumbnailer/moviedecoder.cpp \
                         ../cpp/libffmpegthumbnailer/moviedecoder.h \
                         ../cpp/libffmpegthumbnailer/pngwriter.cpp \
                         ../cpp/libffmpegthumbnailer/pngwriter.h \
                         ../cpp/libffmpegthumbnailer/stringoperations.cpp \
                         ../cpp/libffmpegthumbnailer/stringoperations.h \
                         ../cpp/libffmpegthumbnailer/videoframe.h \
                         ../cpp/libffmpegthumbnailer/videothumbnailer.cpp \
                         ../cpp/libffmpegthumbnailer/videothumbnailer.h \
                         ../cpp/libffmpegthumbnailer/videothumbnailerc.cpp \
                         ../cpp/libffmpegthumbnailer/videothumbnailerc.h \
                         ../cpp/libpng/png.c \
                         ../cpp/libpng/png.h \
                         ../cpp/libpng/pngconf.h \
                         ../cpp/libpng/pngerror.c \
                         ../cpp/libpng/pnggccrd.c \
                         ../cpp/libpng/pngget.c \
                         ../cpp/libpng/pngmem.c \
                         ../cpp/libpng/pngpread.c \
                         ../cpp/libpng/pngread.c \
                         ../cpp/libpng/pngrio.c \
                         ../cpp/libpng/pngrtran.c \
                         ../cpp/libpng/pngrutil.c \
                         ../cpp/libpng/pngset.c \
                         ../cpp/libpng/pngtest.c \
                         ../cpp/libpng/pngtrans.c \
                         ../cpp/libpng/pngvcrd.c \
                         ../cpp/libpng/pngwio.c \
                         ../cpp/libpng/pngwrite.c \
                         ../cpp/libpng/pngwtran.c \
                         ../cpp/libpng/pngwutil.c
LOCAL_LDLIBS    := -llog -lz
LOCAL_C_INCLUDES := ../cpp/include
LOCAL_STATIC_LIBRARIES := libavcodec libavformat libavutil libswscale libswresample libavfilter x264 aac
include $(BUILD_SHARED_LIBRARY)


