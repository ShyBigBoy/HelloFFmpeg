//
// Created by zhicheng.huang on 6/14/17.
//

#include "SurfaceVideoPlayer.h"
#include "log.h"
#include "FFmpegCore.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>

static ANativeWindow *nativeWindow;
//绘制时候的缓冲区
ANativeWindow_Buffer windowBuffer;

static uint8_t *rgbaFrameBuffer;
static int windowBufferSize;
static RGBAFrameCallback callBack;

void *getRGBAFrameAndShow(void *args) {
    LOGD("SurfaceVideoPlayer22------------tid=%ld", pthread_self());
    while (0 == callBack(&rgbaFrameBuffer)) {
        // lock native window buffer
        ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

        if (NULL != rgbaFrameBuffer) {
            LOGD("rgbaFrameBuffer size=%d  windowBuffer.bits size=%d", sizeof(rgbaFrameBuffer), sizeof(windowBuffer.bits));
            LOGD("windowBufferSize=%d  %d", windowBufferSize, windowBufferSize * sizeof(uint8_t));
            //windowBuffer.bits = rgbaFrameBuffer;
            memcpy(windowBuffer.bits, rgbaFrameBuffer, windowBufferSize * sizeof(uint8_t));

        }

        ANativeWindow_unlockAndPost(nativeWindow);
    }
    LOGD("thread %ld exit ...............", pthread_self());
}

int svPlayVideo(JNIEnv* env, jobject surface, RGBAFrameCallback callback) {
    LOGD("SurfaceVideoPlayer11------------tid=%ld", pthread_self());
    int width, height;
    callBack = callback;

    getVideoStreamParameter(&width, &height, &windowBufferSize);

// 获取native window
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (NULL == nativeWindow) {
        LOGE("ANativeWindow_fromSurface error");
        return -1;
    }
// 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

    pthread_t thread;
    pthread_create(&thread, NULL, getRGBAFrameAndShow, (void*)NULL);

    /*while (0 == callback(&rgbaFrameBuffer)) {
        // lock native window buffer
        ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

        if (NULL != rgbaFrameBuffer) {
            LOGD("rgbaFrameBuffer size=%d  windowBuffer.bits size=%d", sizeof(rgbaFrameBuffer), sizeof(windowBuffer.bits));
            LOGD("windowBufferSize=%d  %d", windowBufferSize, windowBufferSize * sizeof(uint8_t));
            //windowBuffer.bits = rgbaFrameBuffer;
            memcpy(windowBuffer.bits, rgbaFrameBuffer, windowBufferSize * sizeof(uint8_t));

        }

        ANativeWindow_unlockAndPost(nativeWindow);
    }*/
    LOGD("svPlayVideo finish ...");
    return 0;
}

void surfaceVideoPlayer_Destroy() {
    ANativeWindow_release(nativeWindow);
}