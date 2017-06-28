cmake_minimum_required(VERSION 3.4.1)

add_library(
    FFmpegLib
    SHARED
    src/main/cpp/FFmpegUtil.cpp

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
    src/main/cpp/libffmpegthumbnailer/videothumbnailer.cpp
    src/main/cpp/libffmpegthumbnailer/videothumbnailer.h
    src/main/cpp/libffmpegthumbnailer/videothumbnailerc.cpp
    src/main/cpp/libffmpegthumbnailer/videothumbnailerc.h

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
)

add_library(libavcodec STATIC IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavcodec.a)
add_library(libavfilter STATIC IMPORTED)
set_target_properties(libavfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavfilter.a)
add_library(libavformat STATIC IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavformat.a)
add_library(libswresample STATIC IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libswresample.a)
add_library(libavutil STATIC IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libavutil.a)
add_library(libswscale STATIC IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libswscale.a)
add_library(lib264 STATIC IMPORTED)
set_target_properties(lib264 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libx264.a)
add_library(aac STATIC IMPORTED)
set_target_properties(aac PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/lib/${ANDROID_ABI}/libfdk-aac.a)


include_directories(src/main/cpp/include)

find_library(log-lib log)
find_library(z z)

target_link_libraries(FFmpegLib libavformat libavcodec libswscale libavutil libswresample libavfilter lib264 aac ${z} ${log-lib})

