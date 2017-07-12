//
// Created by zhicheng.huang on 6/9/17.
//

#ifndef HELLOFFMPEG_LOG_H
#define HELLOFFMPEG_LOG_H

#include <stdio.h>

#define LOG_TAG "NativeFFplayer"

//pirnt log
#ifdef ANDROID
#include <android/log.h>
#define LOGI(msg...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, msg)
#define LOGE(msg...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, msg)
#define LOGD(msg...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, msg)
#else
#define LOGI(format, ...) printf("(>_<) " format "\n", __VA_ARGS__)
#define LOGE(format, ...) printf("(>_<) " format "\n", __VA_ARGS__)
#define LOGE(...) printf("(>_<)\n", __VA_ARGS__)
#define LOGD(...) printf("(>_<)\n", __VA_ARGS__)
#endif

#endif //HELLOFFMPEG_LOG_H
