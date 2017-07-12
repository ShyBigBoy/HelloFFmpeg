//
// Created by zhicheng.huang on 6/9/17.
//

#ifndef HELLOFFMPEG_FFMPEGCORE_H
#define HELLOFFMPEG_FFMPEGCORE_H

//#include <stddef.h>
//#include <stdint.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

int initFFmpeg(const char* url);
int init_FFmpeg(const char* url, int *rate, int *channel);
int releaseFFmpeg();
int getAudioStreamParameter(int *rate, int *channel);
int getPCM(void **pcm, size_t *pcmSize);
int getVideoStreamParameter(int *width, int *height, int *buffersize);
int getRGBA(uint8_t **rgbaFrameBuffer);
int getRGBAFrame(JNIEnv* env, jobject surface);
void avcodec_info(char **avcodecinfo);
void avcodec_configuration_info(char* confinfo);

#ifdef __cplusplus
}
#endif

#endif //HELLOFFMPEG_FFMPEGCORE_H
