package com.sunny.helloffmpeg;

import android.content.pm.PackageManager;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class Main2Activity extends AppCompatActivity implements SurfaceHolder.Callback {
    SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Thread mThread;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.addCallback(this);

        verifyPermissions();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("NativeFFplayer", "UI tid=" + Thread.currentThread().getId() + " name=" + Thread.currentThread().getName());

        //NativeFFplayer.native_initFFplayer("http://61.55.145.199/live/hls/1048.m3u8");
        NativeFFplayer.native_initFFplayer("/storage/emulated/0/DCIM/Camera/VID_20170616_173646.3gp");
        NativeFFplayer.native_playAudio();
        Log.d("NativeFFplayer", "prepare play video ...");
        NativeFFplayer.native_playVideo(mSurfaceHolder.getSurface());

        /*mThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d("NativeFFplayer", "new thread tid=" + Thread.currentThread().getId() + " name=" + Thread.currentThread().getName());
                /**
                 * CCTV1 http://14.18.17.142:9009/live/chid=1
                 * CCTV6 http://14.18.17.142:9009/live/chid=10
                 * CCTV10 http://14.18.17.142:9009/live/chid=4
                 * CCTV13 http://14.18.17.142:9009/live/chid=13
                 * http://cctv1.vtime.cntv.dnion.com:8000/live/no/18_/seg0/index.m3u8
                 * http://183.251.61.207/PLTV/88888888/224/3221225800/index.m3u8
                 *
                NativeFFplayer.native_playVideo(mSurfaceHolder.getSurface());
                Log.d("NativeFFplayer", "myvideo1 thread exit ...");
            }
        }, "myvideo1");
        mThread.start();*/
        //NativeFFplayer.native_initFFplayer("http://14.18.17.142:9009/live/chid=1");
        //NativeFFplayer.native_playAudio();
        //NativeFFplayer.native_playVideo(mSurfaceHolder.getSurface());
        Log.d("NativeFFplayer", "surfaceCreated finish ...");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("NativeFFplayer", "surfaceDestroyed finish ...");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (View.VISIBLE != mSurfaceView.getVisibility()) { return; }
        NativeFFplayer.native_stopAudio();
        NativeFFplayer.native_stopVideo();
        NativeFFplayer.native_releaseFFplayer();
        /*while (Thread.State.TERMINATED != mThread.getState()) {
            Log.d("NativeFFplayer", "waiting for thread " + mThread.getName() + " exit ...");
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }*/
    }

    public void verifyPermissions() {
        Log.d("NativeFFplayer", "call verifyPermissions ...");
        if (Build.VERSION.SDK_INT >= 23) {
            //检测是否有权限
            int checkPermission = checkSelfPermission("android.permission.WRITE_EXTERNAL_STORAGE");
            if (checkPermission != PackageManager.PERMISSION_GRANTED) {
                mSurfaceView.setVisibility(View.INVISIBLE);
                // 没有权限，去申请权限，会弹出对话框
                requestPermissions(PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
            }

            int checkPermission2 = checkSelfPermission("android.permission.CALL_PHONE");
            if (checkPermission2 == PackageManager.PERMISSION_GRANTED) { Log.d("NativeFFplayer", "permission.CALL_PHONE granted ..."); }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            Log.d("NativeFFplayer", "permission granted ...");
            if (View.VISIBLE != mSurfaceView.getVisibility()) { mSurfaceView.setVisibility(View.VISIBLE); }

        } else {
            Log.d("NativeFFplayer", "permission refused ...");
        }
    }
}
