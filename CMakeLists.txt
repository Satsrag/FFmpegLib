cmake_minimum_required(VERSION 3.4.1)

add_library(
        FFmpegLib
        SHARED
        src/main/cpp/FFmpegUtil.cpp
)

add_library(
        tempLib
        STATIC

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

include_directories(src/main/cpp/include)

find_library(log-lib log)
find_library(z z)


target_link_libraries(tempLib libavformat libavfilter libavcodec libswresample libswscale libavutil ${z} ${log-lib})
target_link_libraries(FFmpegLib tempLib ${log-lib})

