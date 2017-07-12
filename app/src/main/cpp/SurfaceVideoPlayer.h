//
// Created by zhicheng.huang on 6/14/17.
//

#ifndef HELLOFFMPEG_SURFACEVIDEOPLAYER_H
#define HELLOFFMPEG_SURFACEVIDEOPLAYER_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*RGBAFrameCallback)(uint8_t **rgbaFrame);

int svPlayVideo(JNIEnv* env, jobject surface, RGBAFrameCallback callback);
void surfaceVideoPlayer_Destroy();

#ifdef __cplusplus
}
#endif

#endif //HELLOFFMPEG_SURFACEVIDEOPLAYER_H
