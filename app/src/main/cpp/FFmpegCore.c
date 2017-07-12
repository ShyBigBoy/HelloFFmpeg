//
// Created by zhicheng.huang on 6/9/17.
//

#include "FFmpegCore.h"
#include "log.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>

static AVFormatContext *aFormatCtx;
static uint8_t *outputBuffer;
static size_t outputBufferSize;

static AVPacket packet;
static AVPacket vpacket;
static int audioStreamIndex;
static int videoStreamIndex;
static AVFrame *aFrame;
static AVFrame* pFrame;
static AVFrame* pFrameRGBA;
static SwrContext *swr;
struct SwsContext* sws_ctx;
static AVCodecContext *aCodecCtx;
static AVCodecContext* pCodecCtx;
static uint8_t* rgbaBuffer;
static int numBytes;

static int init_audio_stream(/*int *rate, int *channel*/) {
    // Find the first audio stream
    int i;
    audioStreamIndex = -1;
    for (i = 0; i < aFormatCtx->nb_streams; i++) {
        if (aFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
            audioStreamIndex < 0) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        LOGE("Couldn't find audio stream!");
        return -1;
    }

    // Get a pointer to the codec context for the audio stream
    aCodecCtx = aFormatCtx->streams[audioStreamIndex]->codec;

    // Find the decoder for the audio stream
    AVCodec *aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (!aCodec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0) {
        LOGE("Could not open codec.");
        return -1; // Could not open codec
    }

    aFrame = av_frame_alloc();

    // 设置格式转换
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_layout",  aCodecCtx->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", aCodecCtx->channel_layout,  0);
    av_opt_set_int(swr, "in_sample_rate",     aCodecCtx->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate",    aCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  aCodecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
    swr_init(swr);

    // 分配PCM数据缓存
    outputBufferSize = 8196;
    outputBuffer = (uint8_t *) malloc(sizeof(uint8_t) * outputBufferSize);

    // 返回sample rate和channels
    //*rate = aCodecCtx->sample_rate;
    //*channel = aCodecCtx->channels;

    return 0;
}

int getAudioStreamParameter(int *rate, int *channel) {
    // 返回sample rate和channels
    *rate = aCodecCtx->sample_rate;
    *channel = aCodecCtx->channels;

    return 0;
}

// 获取PCM数据, 自动回调获取
int getPCM(void **pcm, size_t *pcmSize) {
    LOGD(">> getPcm");
    LOGD("FFmpegCore11------------tid=%ld", pthread_self());
    while (av_read_frame(aFormatCtx, &packet) >= 0) {

        int frameFinished = 0;
        // Is this a packet from the audio stream?
        if (packet.stream_index == audioStreamIndex) {
            avcodec_decode_audio4(aCodecCtx, aFrame, &frameFinished, &packet);

            if (frameFinished) {
                // data_size为音频数据所占的字节数
                int data_size = av_samples_get_buffer_size(
                        aFrame->linesize, aCodecCtx->channels,
                        aFrame->nb_samples, aCodecCtx->sample_fmt, 1);
                LOGD(">> getPcm data_size=%d", data_size);
                // 这里内存再分配可能存在问题
                if (data_size > outputBufferSize) {
                    outputBufferSize = data_size;
                    outputBuffer = (uint8_t *) realloc(outputBuffer,
                                                       sizeof(uint8_t) * outputBufferSize);
                }

                // 音频格式转换
                swr_convert(swr, &outputBuffer, aFrame->nb_samples,
                            (uint8_t const **) (aFrame->extended_data),
                            aFrame->nb_samples);

                // 返回pcm数据
                *pcm = outputBuffer;
                *pcmSize = data_size;
                return 0;
            }
        }
    }
    return -1;
}

static int init_video_stream() {
    // Find the first video stream
    //获取视频流的索引位置
    int  j;
    videoStreamIndex = -1;
    for (j = 0; j < aFormatCtx->nb_streams; j++) {
        if (aFormatCtx->streams[j]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStreamIndex < 0) {
            videoStreamIndex = j;
            break;
        }
    }
    //play_video(env, surface);

    if(videoStreamIndex==-1) {
        LOGE("Didn't find a video stream.");
        return -1; // Didn't find a video stream
    }
    // Get a pointer to the codec context for the video stream
    //pCodecCtx = aFormatCtx->streams[videoStreamIndex]->codec;
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, aFormatCtx->streams[videoStreamIndex]->codecpar);
    // Find the decoder for the video stream
    AVCodec * pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        LOGE("Codec not found.");
        return -1; // Codec not found
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Could not open codec.");
        return -1; // Could not open codec
    }

    // Allocate video frame,yuv格式
    pFrame = av_frame_alloc();
    // 用于渲染
    pFrameRGBA = av_frame_alloc();
    if(pFrameRGBA == NULL || pFrame == NULL) {
        LOGE("Could not allocate video frame.");
        return -1;
    }
    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height, 1);
    rgbaBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    //av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, rgbaBuffer, AV_PIX_FMT_RGBA,
    //                     pCodecCtx->width, pCodecCtx->height, 1);
    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    sws_ctx = sws_getContext(pCodecCtx->width,
                             pCodecCtx->height,
                             pCodecCtx->pix_fmt,
                             pCodecCtx->width,
                             pCodecCtx->height,
                             AV_PIX_FMT_RGBA,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL);

    return 0;
}

int getVideoStreamParameter(int *width, int *height, int *buffersize) {
    *width = pCodecCtx->width;
    *height = pCodecCtx->height;
    *buffersize = numBytes;

    return 0;
}

int getRGBA(/*JNIEnv* env, jobject surface, */uint8_t** rgbaFrameBuffer) {
    LOGD(">> getRGBA");
    LOGD("FFmpegCore22------------tid=%ld", pthread_self());
    //int frameFinished;
    // 获取native window
    /*ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (NULL == nativeWindow) {
        LOGE("ANativeWindow_fromSurface error");
        return -1;
    }
    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,  pCodecCtx->width, pCodecCtx->height, WINDOW_FORMAT_RGBA_8888);
    //绘制时候的缓冲区
    ANativeWindow_Buffer windowBuffer;*/
    while(av_read_frame(aFormatCtx, &vpacket)>=0) {
        // Is this a packet from the video stream?
        if(vpacket.stream_index==videoStreamIndex) {
            // Decode video frame
            //avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            avcodec_send_packet(pCodecCtx, &vpacket);

            //从解码器返回解码输出数据，并不是decode一次就可解码出一帧
            if (0 == avcodec_receive_frame(pCodecCtx, pFrame)) {
                // lock native window buffer
                //ANativeWindow_lock(nativeWindow, &windowBuffer, 0);
                // 格式转换
                sws_scale(sws_ctx, (const uint8_t *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
                av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, rgbaBuffer, AV_PIX_FMT_RGBA,
                                     pCodecCtx->width, pCodecCtx->height, 1);
                // 获取stride
                /*uint8_t * dst = (uint8_t*) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t * src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < pCodecCtx->height; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }*/

                *rgbaFrameBuffer = rgbaBuffer;

                //ANativeWindow_unlockAndPost(nativeWindow);
                return 0;
            }
        }
        av_packet_unref(&vpacket);
    }

    return -1;
}

int getRGBAFrame(JNIEnv* env, jobject surface) {
    //int frameFinished;
    // 获取native window
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (NULL == nativeWindow) {
        LOGE("ANativeWindow_fromSurface error");
        return -1;
    }
    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,  pCodecCtx->width, pCodecCtx->height, WINDOW_FORMAT_RGBA_8888);
    //绘制时候的缓冲区
    ANativeWindow_Buffer windowBuffer;
    while(av_read_frame(aFormatCtx, &vpacket)>=0) {
        // Is this a packet from the video stream?
        if(vpacket.stream_index==videoStreamIndex) {
            // Decode video frame
            //avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            avcodec_send_packet(pCodecCtx, &vpacket);

            //从解码器返回解码输出数据，并不是decode一次就可解码出一帧
            if (0 == avcodec_receive_frame(pCodecCtx, pFrame)) {
                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);
                // 格式转换
                sws_scale(sws_ctx, (const uint8_t *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);
                av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, (const uint8_t *) windowBuffer.bits, AV_PIX_FMT_RGBA,
                                     pCodecCtx->width, pCodecCtx->height, 1);
                // 获取stride
                /*uint8_t * dst = (uint8_t*) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t * src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];
                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < pCodecCtx->height; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }*/

                //*rgbaFrameBuffer = rgbaBuffer;

                ANativeWindow_unlockAndPost(nativeWindow);
                //return 0;
            }
        }
        av_packet_unref(&vpacket);
    }

    av_free(rgbaBuffer);
    av_packet_unref(&vpacket);
    av_frame_free(&pFrameRGBA);
    // Free the YUV frame
    av_frame_free(&pFrame);
    // Close the codecs
    avcodec_close(pCodecCtx);
    sws_freeContext(sws_ctx);
    // Close the video file
    avformat_close_input(&aFormatCtx);
    ANativeWindow_release(nativeWindow);
    LOGD("all destroy ...");

    return 0;
}

int initFFmpeg(const char* url) {
    //注册所有编解码器
    av_register_all();
    //封装格式上下文
    aFormatCtx = avformat_alloc_context();
    LOGD("ffmpeg get url=%s", url);
    const char *file_name = url;

    //打开输入流并读取头文件。此时编解码器还没有打开
    if (avformat_open_input(&aFormatCtx, file_name, NULL, NULL) != 0) {
        LOGE("Couldn't open file:%s\n", file_name);
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(aFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    init_audio_stream();
    init_video_stream();
    LOGD("init FFmpeg finish ...");

    return 0;
}

int init_FFmpeg(const char* url, int *rate, int *channel) {
    //注册所有编解码器
    av_register_all();
    //封装格式上下文
    aFormatCtx = avformat_alloc_context();
    LOGD("ffmpeg get url=%s", url);
    const char *file_name = url;

    //打开输入流并读取头文件。此时编解码器还没有打开
    if (avformat_open_input(&aFormatCtx, file_name, NULL, NULL) != 0) {
        LOGE("Couldn't open file:%s\n", file_name);
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(aFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return -1;
    }

    //init_audio_stream(rate, channel);
    //init_video_stream();

    return 0;
}

// 释放相关资源
int releaseFFmpeg() {
    av_free(outputBuffer);
    av_packet_unref(&packet);
    av_frame_free(&aFrame);
    avcodec_close(aCodecCtx);
    swr_free(&swr);
    //avformat_close_input(&aFormatCtx);

    av_free(rgbaBuffer);
    av_packet_unref(&vpacket);
    av_frame_free(&pFrameRGBA);
    // Free the YUV frame
    av_frame_free(&pFrame);
    // Close the codecs
    avcodec_close(pCodecCtx);
    sws_freeContext(sws_ctx);
    // Close the video file
    avformat_close_input(&aFormatCtx);
    //ANativeWindow_release(nativeWindow);
    LOGD("all destroy ...");

    return 0;
}

void avcodec_info(char **avcodecinfo) {
    char info[40000] = {0};
    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        if (c_temp->decode != NULL) { sprintf(info, "%s[Dec]", info); }
        else { sprintf(info, "%s[Enc]", info); }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    LOGD("%s", info);
    *avcodecinfo = info;
}

void avcodec_configuration_info(char* confinfo) {
    char info[10000] = { 0 };
    sprintf(info, "%s\n", avcodec_configuration());
    confinfo = info;
}