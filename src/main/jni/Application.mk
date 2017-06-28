# 指定生成哪些cpu架构的库
APP_ABI := armeabi
# 此变量包含目标 Android 平台的名称
APP_PLATFORM := android-14
APP_STL := gnustl_static
APP_CPPFLAGS += -fexceptions -std=c++11