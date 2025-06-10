cmake_minimum_required(VERSION 3.22.1)

project("smartzplayer")

# 设置 FFmpeg 库路径
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/../jniLibs)

# 添加 FFmpeg 库
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
    ${FFMPEG_DIR}/${ANDROID_ABI}/libswscale.so)

# 添加你的原生库
add_library(${CMAKE_PROJECT_NAME} SHARED
    native-lib.cpp
    player.cpp
)

# 包含头文件目录
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${FFMPEG_DIR}/include)

# 链接 FFmpeg 库
target_link_libraries(${CMAKE_PROJECT_NAME}
    avcodec
    avfilter
    avformat
    avutil
    swresample
    swscale
    android
    log)
