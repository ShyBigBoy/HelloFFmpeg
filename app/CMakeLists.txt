# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set( LIB_SRC_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI} )
message( STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}" )
message( STATUS "ANDROID_ABI=${ANDROID_ABI}" )
message( STATUS "LIB_SRC_DIR=${LIB_SRC_DIR}" )

include_directories( src/main/jniLibs/include )
include_directories( src/main/cpp )

#编解码(最重要的库)
add_library( avcodec SHARED IMPORTED )
set_target_properties( avcodec
                       PROPERTIES
                       IMPORTED_LOCATION ${LIB_SRC_DIR}/libavcodec-57.so )

# 滤镜特效处理库
add_library( avfilter SHARED IMPORTED )
set_target_properties( avfilter
                       PROPERTIES
                       IMPORTED_LOCATION ${LIB_SRC_DIR}/libavfilter-6.so )

# 封装格式处理库
add_library( avformat SHARED IMPORTED )
set_target_properties( avformat
                      PROPERTIES
                      IMPORTED_LOCATION ${LIB_SRC_DIR}/libavformat-57.so )

# 工具库(大部分库都需要这个库的支持)
add_library( avutil SHARED IMPORTED )
set_target_properties( avutil
                       PROPERTIES
                       IMPORTED_LOCATION ${LIB_SRC_DIR}/libavutil-55.so )

# 音频采样数据格式转换库
add_library( swresample SHARED IMPORTED )
set_target_properties( swresample
                       PROPERTIES
                       IMPORTED_LOCATION ${LIB_SRC_DIR}/libswresample-2.so )

# 视频像素数据格式转换
add_library( swscale SHARED IMPORTED )
set_target_properties( swscale
                       PROPERTIES
                       IMPORTED_LOCATION ${LIB_SRC_DIR}/libswscale-4.so )

add_library( # Sets the name of the library.
             nativeffplayer-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/com_sunny_helloffmpeg_NativeFFplayer.cpp
             src/main/cpp/FFmpegCore.c
             src/main/cpp/OpenSLES_AudioPlayer.c
             src/main/cpp/SurfaceVideoPlayer.c )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
find_library( z-lib z )
find_library( android-lib android )
find_library( OpenSLES-lib OpenSLES )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       nativeffplayer-lib
                       avcodec
                       avfilter
                       avformat
                       avutil
                       swresample
                       swscale

                       ${z-lib}
                       ${android-lib}
                       ${OpenSLES-lib}

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
