cmake_minimum_required(VERSION 3.4.1)

add_library(
    FFmpegLib
    SHARED
    src/main/cpp/FFmpegUtil.cpp
    )

add_library(
    tempLib
    STATIC

    ####### ffmpeg get thumb util #######
    src/main/cpp/libffmpegthumbnailer/filmstrip.h
    src/main/cpp/libffmpegthumbnailer/filmstripfilter.cpp
    src/main/cpp/libffmpegthumbnailer/filmstripfilter.h
    src/main/cpp/libffmpegthumbnailer/grayscalefilter.h
    src/main/cpp/libffmpegthumbnailer/histogram.h
    src/main/cpp/libffmpegthumbnailer/ifilter.h
    src/main/cpp/libffmpegthumbnailer/imagetypes.h
    src/main/cpp/libffmpegthumbnailer/imagewriter.h
    src/main/cpp/libffmpegthumbnailer/imagewriterfactory.h
    src/main/cpp/libffmpegthumbnailer/moviedecoder.cpp
    src/main/cpp/libffmpegthumbnailer/moviedecoder.h
    src/main/cpp/libffmpegthumbnailer/pngwriter.cpp
    src/main/cpp/libffmpegthumbnailer/pngwriter.h
    src/main/cpp/libffmpegthumbnailer/stringoperations.cpp
    src/main/cpp/libffmpegthumbnailer/stringoperations.h
    src/main/cpp/libffmpegthumbnailer/videoframe.h
    #src/main/cpp/libffmpegthumbnailer/videothumbnailer.cpp
    #src/main/cpp/libffmpegthumbnailer/videothumbnailer.h
    #src/main/cpp/libffmpegthumbnailer/videothumbnailerc.cpp
    #src/main/cpp/libffmpegthumbnailer/videothumbnailerc.h

    ####### libpng #######
    src/main/cpp/libpng/png.c
    src/main/cpp/libpng/png.h
    src/main/cpp/libpng/pngconf.h
    src/main/cpp/libpng/pngerror.c
    src/main/cpp/libpng/pnggccrd.c
    src/main/cpp/libpng/pngget.c
    src/main/cpp/libpng/pngmem.c
    src/main/cpp/libpng/pngpread.c
    src/main/cpp/libpng/pngread.c
    src/main/cpp/libpng/pngrio.c
    src/main/cpp/libpng/pngrtran.c
    src/main/cpp/libpng/pngrutil.c
    src/main/cpp/libpng/pngset.c
    src/main/cpp/libpng/pngtest.c
    src/main/cpp/libpng/pngtrans.c
    src/main/cpp/libpng/pngvcrd.c
    src/main/cpp/libpng/pngwio.c
    src/main/cpp/libpng/pngwrite.c
    src/main/cpp/libpng/pngwtran.c
    src/main/cpp/libpng/pngwutil.c

    ######## cmd ########
    src/main/cpp/cmd/ffmpeg.h
    src/main/cpp/cmd/config.h
    src/main/cpp/cmd/cmdutils.h
    src/main/cpp/cmd/cmdutils.c
    src/main/cpp/cmd/cmdutils_common_opts.h
    src/main/cpp/cmd/ffmpeg.c
    src/main/cpp/cmd/ffmpeg_filter.c
    src/main/cpp/cmd/ffmpeg_opt.c


    ##### Log #########
    src/main/cpp/Log.c

    ####### VideoEdit ########
    src/main/cpp/video_edit/Cut.c
    src/main/cpp/video_edit/Compress.c
    src/main/cpp/video_edit/Progress.c
    src/main/cpp/video_edit/Progress.h
)

add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavcodec.so)
add_library(libavfilter SHARED IMPORTED)
set_target_properties(libavfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavfilter.so)
add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavformat.so)
add_library(libswresample SHARED IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libswresample.so)
add_library(libavutil SHARED IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavutil.so)
add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libswscale.so)
add_library(lib264 STATIC IMPORTED)
set_target_properties(lib264 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/armeabi/libx264.a)
add_library(aac SHARED IMPORTED)
set_target_properties(aac PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libfdk-aac.so)

include_directories(ffmpeg-3.4.9)

find_library(log-lib log)
find_library(z z)


target_link_libraries(tempLib libavformat libavfilter libavcodec libswresample libswscale libavutil lib264 aac ${z} ${log-lib})
target_link_libraries(FFmpegLib tempLib ${log-lib})

