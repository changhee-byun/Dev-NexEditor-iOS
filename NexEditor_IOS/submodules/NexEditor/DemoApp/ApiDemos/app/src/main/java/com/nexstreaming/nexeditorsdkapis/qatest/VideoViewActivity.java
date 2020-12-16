package com.nexstreaming.nexeditorsdkapis.qatest;

//import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.app.Activity;
import android.app.KeyguardManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.PowerManager;
import android.view.View;
import android.widget.ImageView;
import android.widget.MediaController;
import android.widget.VideoView;

import com.nexstreaming.nexeditorsdkapis.R;

public class VideoViewActivity extends Activity {
    private VideoView mVView;
    private ImageView mIViewBack;

    private String mPath;

    private boolean mIsLCDoff;
    private boolean mIsSecured;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_view);

        mVView = (VideoView) findViewById(R.id.videoview);
        mIViewBack = (ImageView) findViewById(R.id.preview_back);
        mIViewBack.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        mPath = getIntent().getStringExtra("path");

        mVView.setVideoPath(mPath);
        mVView.setMediaController(new MediaController(this));
        mVView.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                mVView.seekTo(0);
            }
        });
        mVView.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mp) {
                if(!mIsSecured) {
                    mVView.start();
                }
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();

        KeyguardManager keyguardManager = (KeyguardManager) getSystemService(KEYGUARD_SERVICE);
        if(keyguardManager.inKeyguardRestrictedInputMode()) mIsSecured = true;

        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        boolean isScreenOn = powerManager.isScreenOn();
        if(!isScreenOn) {
            mIsLCDoff = true;
        }
        mVView.pause();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if(!mIsLCDoff && !mIsSecured) {
            mVView.resume();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mVView.stopPlayback();
    }
}
