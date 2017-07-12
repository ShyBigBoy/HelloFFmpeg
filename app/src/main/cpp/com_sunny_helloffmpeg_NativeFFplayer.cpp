//
// Created by Sunny on 5/26/17.
//
#include "com_sunny_helloffmpeg_NativeFFplayer.h"
#include "log.h"
#include <assert.h>
#include <errno.h>
#include <string>
#include <pthread.h>

#include "FFmpegCore.h"
#include "OpenSLES_AudioPlayer.h"
#include "SurfaceVideoPlayer.h"

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

static jstring avcodecInfo(JNIEnv* env, jclass clazz) {
    LOGD("1------------tid=%ld", pthread_self());
    //char info[40000] = {0};
    char* info = NULL;
    avcodec_info(&info);
    if (NULL == info) { LOGD("1 info is NULL"); }
    return env->NewStringUTF(info);
}

static jstring stringFromJNI(JNIEnv* env, jclass clazz) {
#if defined(__arm__)
#if defined(__ARM_ARCH_7A__)
    #if defined(__ARM_NEON__)
            #define ABI "armeabi-v7a/NEON"
        #else
            #define ABI "armeabi-v7a"
        #endif
#else
#define ABI "armeabi"
#endif
#elif defined(__i386__)
    #define ABI "x86"
#elif defined(__mips__)
   #define ABI "mips"
#else
   #define ABI "unknown"
#endif

    LOGD("2------------tid=%ld", pthread_self());
    //std::string hello = "Hello from JNI !";
    std::string hello = "Hello from JNI !  Compiled with ABI=" ABI ".";
    return env->NewStringUTF(hello.c_str());
}

static jstring avcodecConfiguration(JNIEnv* env, jclass clazz) {
    LOGD("3------------tid=%ld", pthread_self());
    //char info[10000] = {0};
    char *info = NULL;
    avcodec_configuration_info(info);
    if (NULL == info) { LOGD("3 info is NULL"); }
    return env->NewStringUTF(info);
}

static jint playVideo(JNIEnv* env, jclass clazz, jobject surface/*, jstring url*/) {
    LOGD("4------------tid=%ld", pthread_self());
    //const char * file_name  = env->GetStringUTFChars(url, JNI_FALSE);
    //initFFmpeg(file_name);
    //return getRGBAFrame(env, surface);
    svPlayVideo(env, surface, getRGBA);
    return 0;
}

static void  stopVideo(JNIEnv *env, jclass clazz) {
    surfaceVideoPlayer_Destroy();
    LOGD("Destroy SurfaceVideoPlayer ...");
}

static void playAudio(JNIEnv *env, jclass clazz/*, jstring url_*/) {
    LOGD("5------------tid=%ld", pthread_self());
    //const char *url = env->GetStringUTFChars(url_, 0);
    //LOGD("start playaudio... url=%s", url);

    //oslPlayAudio(url);
    oslPlayAudio();
    //env->ReleaseStringUTFChars(url_, url);
}

static void  stopAudio(JNIEnv *env, jclass clazz) {
    oslStopAudio();
    LOGD("Destroy OpenSLES_AudioPlayer ...");
}

static jint initFFplayer(JNIEnv *env, jclass clazz, jstring url_) {
    LOGD("6------------tid=%ld", pthread_self());
    const char *url = env->GetStringUTFChars(url_, 0);
    LOGD("initFFplayer... url=%s", url);
    initFFmpeg(url);
    env->ReleaseStringUTFChars(url_, url);

    return 0;
}

static jint releaseFFplayer(JNIEnv *env, jclass clazz) {
    releaseFFmpeg();
    LOGD("Destroy FFmpeg ...");

    return 0;
}

static const char *classPathName = "com/sunny/helloffmpeg/NativeFFplayer";
static JNINativeMethod g_methods[] = {
    { "native_avcodecInfo", "()Ljava/lang/String;", (void*)avcodecInfo },
    { "native_stringFromJNI", "()Ljava/lang/String;", (void*)stringFromJNI },
    { "native_avcodecConfiguration", "()Ljava/lang/String;", (void*)avcodecConfiguration },
    { "native_playVideo", "(Ljava/lang/Object;)I", (void*)playVideo },
    { "native_stopVideo", "()V", (void*)stopVideo },
    { "native_playAudio", "()V", (void*)playAudio },
    { "native_stopAudio", "()V", (void*)stopAudio },
    { "native_initFFplayer", "(Ljava/lang/String;)I", (void*)initFFplayer },
    { "native_releaseFFplayer", "()I", (void*)releaseFFplayer }
};

/*int register_com_eason_helloffmpeg(JNIEnv *env) {
    return registerNativeMethods(env, classPathName, g_methods, NELEM(g_methods));
}*/

static int registerNativeMethods(JNIEnv* env, const char* className, JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE("Error: GetEnv failed in JNI_OnLoad");
        return -1;
    }
    assert(env != NULL);
    if (!registerNativeMethods(env, classPathName, g_methods, NELEM(g_methods))) {
        LOGE("Error: could not register native methods for FFmpegJNI");
        return -1;
    }
    LOGD("7------------tid=%ld", pthread_self());
    return JNI_VERSION_1_6;
}
