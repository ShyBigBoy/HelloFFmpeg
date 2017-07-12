package com.sunny.helloffmpeg;

/**
 * Created by Sunny on 5/26/17.
 */

public class NativeFFplayer {
    static {
        try {
            System.loadLibrary("avcodec-57");
            System.loadLibrary("avfilter-6");
            System.loadLibrary("avformat-57");
            System.loadLibrary("avutil-55");
            System.loadLibrary("swresample-2");
            System.loadLibrary("swscale-4");
            System.loadLibrary("nativeffplayer-lib");
        } catch (UnsatisfiedLinkError ule) {}
    }

    public native static String native_avcodecInfo();
    public native static String native_stringFromJNI();
    public native static String native_avcodecConfiguration();
    public native static int native_playVideo(Object surface);
    public native static void native_stopVideo();
    public native static void native_playAudio();
    public native static void native_stopAudio();
    public native static int native_initFFplayer(String url);
    public native static int native_releaseFFplayer();
}
