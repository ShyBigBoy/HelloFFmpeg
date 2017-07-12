package com.sunny.helloffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.util.Log;
import android.widget.EditText;

public class Main3Activity extends AppCompatActivity {
    private EditText mInput;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);

        mInput = (EditText) findViewById(R.id.etUrl);
        //mInput.setText("http://qzone.60dj.com/huiyuan/201704/19/201704190533197825_35285.mp3");
        mInput.setText("http://61.55.145.199/live/hls/1048.m3u8");

        Log.d("NativeFFplayer", "avcodecInfo=" + NativeFFplayer.native_avcodecInfo());
        Log.d("NativeFFplayer", "stringFromJNI=" + NativeFFplayer.native_stringFromJNI());


        findViewById(R.id.btPlay).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Log.d("NativeFFplayer", "UI tid=" + Thread.currentThread().getId() + " name=" + Thread.currentThread().getName());
                NativeFFplayer.native_initFFplayer(mInput.getText().toString().trim());
                //NativeFFplayer.native_playAudio(mInput.getText().toString().trim());
                NativeFFplayer.native_playAudio();

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        Log.d("NativeFFplayer", "new thread tid=" + Thread.currentThread().getId() + " name=" + Thread.currentThread().getName());
                        Log.d("NativeFFplayer", "avcodecConfiguration=" + NativeFFplayer.native_avcodecConfiguration());

                        Log.d("NativeFFplayer", "myaudio1 thread exit ...");
                    }
                }, "myaudio1").start();

                Log.d("NativeFFplayer", "btPlay finish ...");
            }
        });
        findViewById(R.id.btPause).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NativeFFplayer.native_stopAudio();
                NativeFFplayer.native_releaseFFplayer();
            }
        });


    }
}
