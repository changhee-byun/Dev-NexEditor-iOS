/******************************************************************************
 * File Name        : MediaPlayerActivity.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdkapis.etc;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.Layout;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import java.util.ArrayList;
import java.util.List;


public class MediaPlayerActivity extends Activity implements nexEngineListener , nexEngineView.NexViewListener{
    private static final String TAG = "nexMediaPlayer";

    private nexEngine mEngine;
    private ArrayList<String> m_listfilepath;

    private nexEngineView m_editorView;
    //private SurfaceView m_editorView;

    private SeekBar mSeekBarPlayTime;
    private Button mPlayStop;
    private Button mTrim;
    private Button mThumbnail;
    private TextView mPlayTime;
    private TextView mEndTime;

    private SeekBar mSeekBarSeekAccel;
    private int mSeekAccelSetTime;

    private SeekBar mSeekBarClipVol;
    private TextView mClipVol = null;
    private int mClipVolume;

    private SeekBar mSeekBarBGMVol;
    private TextView mBGMVol = null;
    private int mBGMVolume;

    private SeekBar mSeekBarDeviceLightLevel;
    private TextView mDeviceLightLevelTexView = null;
    private TextView mDeviceLightLevelNameTextView;
    private int mDeviceLightLevel;

    private SeekBar mSeekBarDeviceGamma;
    private TextView mDeviceGammaTexView = null;
    private TextView mDeviceGammaNameTextView;
    private int mDeviceGamma;

    private TextView mTitle;
    private TextView mArtist;
    private TextView mDuration;
    private Button mAddBGM;

    private int mCurrentPlayTime;
    private boolean trackingSeekbarTouch = false;
    private boolean accelSeekbarTouch = false;
    private int trackingSeektime = -1;
    private boolean seekInProgress = false;
    private boolean lastUIisPlaying = false;
    private int playTime = 0;

    private int vv360x = 180;
    private int vv360y = 0;

    private TextView mTextViewvv360x;
    private TextView mTextViewvv360y;

    private boolean mChangeAspect = false;

    private Button mButtonUp;
    private Button mButtonDown;
    private Button mButtonLeft;
    private Button mButtonRight;
    private ImageView mScreenView;
    private boolean isCaptureScreen;

    private boolean backgroundMode;
    private int instNum;
    nexProject project;

    private boolean isAvailableView = false;

    private List<Runnable> postOnAvailableSurfaceRunnables = new ArrayList<>();
    private boolean isHDR = false;
    private boolean Hdr2Sdr = true;

    private FrameLayout frameLayout;
    private int screenWidth;
    private int screenHeight;

    private int scaleMode = 0;

    @Override
    public void onEngineViewAvailable(int width, int height) {
        isAvailableView = true;
        for(Runnable runnable:  postOnAvailableSurfaceRunnables ){
            runnable.run();
        }
        Log.d(TAG,"onEngineViewAvailable ("+width+"X"+height+")");
        postOnAvailableSurfaceRunnables.clear();
        screenWidth = width;
        screenHeight = height;
    }

    @Override
    public void onEngineViewSizeChanged(int width, int height) {
        Log.d(TAG,"onEngineViewSizeChanged ("+width+"X"+height+")");
    }

    @Override
    public void onEngineViewDestroyed() {
        isAvailableView = false;
        Log.d(TAG,"onEngineViewDestroyed ()");
    }

    private enum State {
        IDLE,
        STOP_READY,
        STOPED,
        SEEKING,
        PLAY_READY,
        PLAYING,
    };

    private State mState = State.IDLE;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        instNum = ++debugTag;
        Log.d(TAG,"onCreate = "+instNum);
        if( savedInstanceState!=null ) {
            mCurrentPlayTime = savedInstanceState.getInt("currentTime");
            lastUIisPlaying = savedInstanceState.getBoolean("requestedPlay");
        }

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        project = new nexProject();

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        mEngine.setProperty("supportContentDuration", ""+300);

        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if(clip == null){
            Toast.makeText(getApplicationContext(), "getSupportedClip fail", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
/*
        if( !clip.ableToDecoding() ){
            Toast.makeText(getApplicationContext(), "ableToDecoding fail", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
*/
        if( clip.hasVideo() ) {

            Log.d(TAG, "Video Render Mode=" + clip.getVideoRenderMode());
            if (clip.getVideoRenderMode() == nexClip.kCLIP_VIDEORENDERMODE_360VIDE) {
                if (nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1) {
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
                    mEngine.updateScreenMode();
                    mChangeAspect = true;
                }
            }
            if (clip != null) {

                clip.setRotateDegree(clip.getRotateInMeta());
                if( clip.getVideoHDRType() == nexClip.HDR_TYPE_10PQ ) {
                    isHDR = true;
                }
                project.add(clip);
                project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                Log.d(TAG, "Video SeekPointInterval=" + clip.getSeekPointInterval()+"(ms)");
                    /*
                    if( nexApplicationConfig.getAspectRatioMode() ==  nexApplicationConfig.kAspectRatio_Mode_9v16 &&
                            clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                        project.getClip(0, true).setRotateDegree(nexClip.kClip_Rotate_270);
                    }
                    */
            } else {
                Toast.makeText(getApplicationContext(), "Not supported clip : " + m_listfilepath.get(0), Toast.LENGTH_SHORT).show();
            }

            if (project.getTotalClipCount(true) == 0) {
                Toast.makeText(getApplicationContext(), "No Clips.", Toast.LENGTH_LONG).show();
                finish();
                return;
            }
        }else{
            project.add(nexClip.getSolidClip(0xffff00ff));
            project.getLastPrimaryClip().setImageClipDuration(clip.getAudioDuration());
        }
        setContentView(R.layout.activity_media_player);
        mScreenView = (ImageView) findViewById(R.id.imageView_media_play);
        mScreenView.setVisibility(View.GONE);
        m_editorView = (nexEngineView)findViewById(R.id.engineview_media_play);
        m_editorView.setListener(this);

        frameLayout = (FrameLayout)findViewById(R.id.framelayout_engine_view);
        frameLayout.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_UP:
                        if( scaleMode  == 0 ) {
                            scaleMode = 1;
                            updateTextureViewSize((int) event.getX(), (int) event.getY(), 2f);
                        }else{
                            updateTextureViewSize(screenWidth/2, screenHeight/2,1);
                            scaleMode = 0;
                        }
                        break;
                }
                return true;
            }
        });


        //m_editorView.setVisibility(View.GONE);
        //m_editorView = (SurfaceView) findViewById(R.id.engineview_media_play);
        //m_editorView.setBlackOut(true);

        //mEngine.setPreviewScaleFactor(2);


        mSeekBarPlayTime = (SeekBar)findViewById(R.id.seekbar_media_play);
        mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
        mSeekBarPlayTime.setProgress(0);
        mSeekBarPlayTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                if (trackingSeekbarTouch) {

                    playTime = i * 33;
                    Log.d(TAG, "seekTs:" + mSeekCurrentTS + " progress:" + i + "/" + Math.abs(mSeekCurrentTS - playTime));
                    if (Math.abs(mSeekCurrentTS - playTime) > 100/*Tmp Value*/) {
                        mSeekMode = 2;
                    } else {
                        mSeekMode = 1;
                    }
                    trackingSeektime = playTime;

                    if (!lastUIisPlaying) {
                        seek(trackingSeektime);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                changeEngineView();

                trackingSeekbarTouch = true;
                stop();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                trackingSeekbarTouch = false;
            }
        });

        mSeekBarSeekAccel = (SeekBar)findViewById(R.id.seekbar_seek_accelerate);
        mSeekBarSeekAccel.setMax(200);
        mSeekBarSeekAccel.setProgress(100);

        mSeekBarSeekAccel.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                return false;
            }
        });

        mSeekBarSeekAccel.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int lastSeekTime = -1;
            boolean threadRun;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (accelSeekbarTouch) {

                    if( progress == 0 ){
                        threadRun = true;
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                while(threadRun){
                                    try {
                                        Thread.sleep(100);
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }

                                    if( !threadRun )
                                        break;

                                    mSeekAccelSetTime -= 10;
                                    int seekTime = mSeekAccelSetTime - 1000;

                                    if( seekTime < 0 )
                                        seekTime = 0;
                                    else if( seekTime > mEngine.getProject().getTotalTime())
                                        seekTime = mEngine.getProject().getTotalTime();

                                    if( lastSeekTime != seekTime) {
                                        lastSeekTime = seekTime;
                                        Log.d(TAG, "long seek prev mSeekAccelSetTime=" + mSeekAccelSetTime + ", seekTime=" + lastSeekTime);
                                        mEngine.seekFromCache(seekTime);
                                    }
                                }
                            }
                        }).start();
                    }else if( progress == 200 ){
                        threadRun = true;
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                while(threadRun){
                                    try {
                                        Thread.sleep(100);
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }

                                    if( !threadRun )
                                        break;

                                    mSeekAccelSetTime += 10;
                                    int seekTime = mSeekAccelSetTime + 1000;

                                    if( seekTime < 0 )
                                        seekTime = 0;
                                    else if( seekTime > mEngine.getProject().getTotalTime())
                                        seekTime = mEngine.getProject().getTotalTime();

                                    if( lastSeekTime != seekTime) {
                                        lastSeekTime = seekTime;
                                        Log.d(TAG, "long seek next mSeekAccelSetTime=" + mSeekAccelSetTime + ", seekTime="+ lastSeekTime);
                                        mEngine.seekFromCache(seekTime);
                                    }
                                }
                            }
                        }).start();

                    }else{
                        threadRun = false;
                        int accel = progress - 100;
                        int seekTime = mSeekAccelSetTime + 10 *  accel;
                        if( seekTime < 0 )
                            seekTime = 0;
                        else if( seekTime > mEngine.getProject().getTotalTime())
                            seekTime = mEngine.getProject().getTotalTime();

                        if( lastSeekTime != seekTime ){
                            lastSeekTime = seekTime;
                            Log.d(TAG, "normal seek , mSeekAccelSetTime=" + mSeekAccelSetTime + ", seekTime="+ seekTime);
                            mEngine.seekFromCache(seekTime);
                        }
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                changeEngineView();
                stop();
                accelSeekbarTouch = true;
                lastSeekTime = -1;
                mSeekAccelSetTime = mEngine.getCurrentPlayTimeTime();
                mEngine.seekFromCache(mSeekAccelSetTime);
                Log.d(TAG, "onStartTrackingTouch");
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                accelSeekbarTouch = false;
                threadRun = false;
                //mSeekBarSeekAccel.setProgress(100);
                seekBar.cancelPendingInputEvents();
                trackingSeektime = lastSeekTime;
                seekBar.setProgress(100);
                Log.d(TAG, "onStopTrackingTouch");
            }
        });

        mPlayStop = (Button)findViewById(R.id.button_media_play_stop);
        mPlayStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                changeEngineView();

                if (mState == State.PLAYING) {
                    stop();
                    mPlayStop.setText("Play");
                    lastUIisPlaying = false;
                } else {
                    project.setBGMMasterVolumeScale(100 / 200f);
                    project.getClip(0, true).setClipVolume(100);
                    mEngine.updateProject();
                    resume();
                    mPlayStop.setText("Stop");
                    lastUIisPlaying = true;
                }
            }
        });

        mTrim = (Button) findViewById(R.id.button_media_play_trim);

        if( isHDR ){
            mTrim.setText("HDR");
        }else{
            mTrim.setVisibility(View.INVISIBLE);
            setVisibilityOfUIForHDR(View.INVISIBLE);
        }


        mTrim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( Hdr2Sdr ) {
                    Hdr2Sdr = false;
                    mTrim.setText("SDR");
                }else {
                    Hdr2Sdr = true;
                    mTrim.setText("HDR");
                }
                mEngine.setHDRtoSDR(Hdr2Sdr);
                if (mState == State.STOPED){
                    mEngine.seek(mCurrentPlayTime);
                }
                        /*
                if (playTime > 0) {
                    project.getClip(0, true).getVideoClipEdit().setTrim(0, playTime);
                    Toast.makeText(getApplicationContext(), "trim success", Toast.LENGTH_SHORT).show();
                    mTrim.setEnabled(false);
                }
                */

            }
        });

        mThumbnail =(Button)findViewById(R.id.button_media_play_thumbnail);
        mThumbnail.setVisibility(View.INVISIBLE);
        mThumbnail.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int thumbendtime = 0;
                thumbendtime = project.getClip(0, true).getTotalTime();

                Toast.makeText(getApplicationContext(), "Get thumbnail loading...", Toast.LENGTH_SHORT).show();
                project.getClip(0, true).getVideoClipDetailThumbnails(project.getClip(0, true).getWidth(), project.getClip(0, true).getHeight(), 0, thumbendtime, 100, 0, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
                    @Override
                    public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                        if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                            Log.d(TAG, "Thumbnail loading... index[" + index + "], timestamp[" + timestamp + "]");
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                            Toast.makeText(getApplicationContext(), "Get thumbnail complete", Toast.LENGTH_LONG).show();
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                            Toast.makeText(getApplicationContext(), "Get thumbnail fail", Toast.LENGTH_LONG).show();
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                            Toast.makeText(getApplicationContext(), "system error", Toast.LENGTH_LONG).show();
                        }
                    }
                });
            }
        });

        mPlayTime = (TextView)findViewById(R.id.textview_media_play_time);

        mEndTime = (TextView)findViewById(R.id.textview_media_play_total);
        mEndTime.setText(getTimeString(project.getTotalTime()));

        mClipVol = (TextView) findViewById(R.id.textview_media_play_clipval);
        mClipVol.setText("100");
        mClipVolume = 100;

        mSeekBarClipVol = (SeekBar) findViewById(R.id.seekbar_media_play_clipvol);
        mSeekBarClipVol.setMax(200);
        mSeekBarClipVol.setProgress(100);
        mSeekBarClipVol.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mClipVol.setText(String.valueOf(progress));

                mClipVolume = progress;

                mEngine.setTotalAudioVolumeWhilePlay(mClipVolume, mBGMVolume);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.setTotalAudioVolumeProject(mClipVolume, mBGMVolume);
            }
        });

        mBGMVol = (TextView) findViewById(R.id.textview_media_play_bgmval);
        mBGMVol.setText("100");
        mBGMVolume = 100;

        mSeekBarBGMVol = (SeekBar) findViewById(R.id.seekbar_media_play_bgmvol);
        mSeekBarBGMVol.setMax(200);
        mSeekBarBGMVol.setProgress(100);
        mSeekBarBGMVol.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mBGMVol.setText(String.valueOf(progress));

                mBGMVolume = progress;

                mEngine.setTotalAudioVolumeWhilePlay(mClipVolume, mBGMVolume);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.setTotalAudioVolumeProject(mClipVolume, mBGMVolume);
            }
        });

        mDeviceLightLevelTexView = (TextView) findViewById(R.id.textview_media_play_device_light_level);
        mDeviceLightLevel = 600;
        mEngine.setDeviceLightLevel(mDeviceLightLevel);

        mDeviceLightLevelTexView.setText("600");

        mSeekBarDeviceLightLevel = (SeekBar) findViewById(R.id.seekbar_media_play_device_light_level);
        mSeekBarDeviceLightLevel.setMax(1000);
        mSeekBarDeviceLightLevel.setProgress(600);
        mSeekBarDeviceLightLevel.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mDeviceLightLevelTexView.setText(String.valueOf(progress));

                mDeviceLightLevel = progress;

                mEngine.setDeviceLightLevel(mDeviceLightLevel);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        mDeviceGammaTexView = (TextView) findViewById(R.id.textview_media_play_device_gamma);

        mDeviceGamma = 190;
        mEngine.setDeviceGamma(getConvertedValue(50,mDeviceGamma,100));
        mDeviceGammaTexView.setText(String.valueOf(getConvertedValue(50,mDeviceGamma,100)));
        //Math.floor( )
        mSeekBarDeviceGamma = (SeekBar) findViewById(R.id.seekbar_media_play_device_gamma);
        mSeekBarDeviceGamma.setMax(50 + 350);
        mSeekBarDeviceGamma.setProgress(mDeviceGamma);

        mSeekBarDeviceGamma.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mDeviceGammaTexView.setText(String.valueOf(getConvertedValue(50,mDeviceGamma,100)));
                mDeviceGamma = progress;

                mEngine.setDeviceGamma(getConvertedValue(50,mDeviceGamma,100));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.setDeviceGamma(getConvertedValue(50,mDeviceGamma,100));
            }

        });

        mTitle = (TextView) findViewById(R.id.textview_media_play_title);
        mArtist = (TextView) findViewById(R.id.textview_media_play_artist);
        mDuration = (TextView) findViewById(R.id.textview_media_play_duration);

        mAddBGM = (Button) findViewById(R.id.bt_addbgm);
        mAddBGM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                stop();
                Intent intent = new Intent();
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        mButtonUp = (Button)findViewById(R.id.button_media_play_360up);
        mButtonUp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                vv360y += 10;
                if( vv360y > 90 ){
                    vv360y -= 180;
                }
                mTextViewvv360y.setText("y:"+ vv360y);

                if( mState == State.STOPED ){
                    mEngine.set360VideoViewStopPosition(vv360x, vv360y);
                }
                mEngine.set360VideoViewPosition(vv360x, vv360y);
            }
        });

        mButtonDown = (Button)findViewById(R.id.button_media_play_360down);
        mButtonDown.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                vv360y -= 10;
                if( vv360y < -90 ){
                    vv360y += 180;
                }
                mTextViewvv360y.setText("y:"+ vv360y);

                if( mState == State.STOPED ){
                    mEngine.set360VideoViewStopPosition(vv360x, vv360y);
                }
                mEngine.set360VideoViewPosition(vv360x,vv360y);
            }
        });

        mButtonLeft = (Button)findViewById(R.id.button_media_play_360left);
        mButtonLeft.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                vv360x -= 10;
                if( vv360x < 0 ){
                    vv360x += 360;
                }
                mTextViewvv360x.setText("x:" + vv360x);

                if( mState == State.STOPED ){
                    mEngine.set360VideoViewStopPosition(vv360x, vv360y);
                }
                mEngine.set360VideoViewPosition(vv360x,vv360y);
            }
        });
        mButtonRight = (Button)findViewById(R.id.button_media_play_360right);
        mButtonRight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                vv360x += 10;
                if(  vv360x > 360){
                    vv360x -= 360;
                }
                mTextViewvv360x.setText("x:" + vv360x);

                if( mState == State.STOPED ){
                    mEngine.set360VideoViewStopPosition(vv360x, vv360y);
                }
                mEngine.set360VideoViewPosition(vv360x,vv360y);
            }
        });

        mTextViewvv360x = (TextView)findViewById(R.id.textView_media_play_360x);
        mTextViewvv360x.setText("x:" + vv360x);
        mTextViewvv360y = (TextView)findViewById(R.id.textView_media_play_360y);
        mTextViewvv360y.setText("y:"+ vv360y);

        if( clip.getVideoRenderMode() == nexClip.kCLIP_VIDEORENDERMODE_NORMAL ){
            mButtonUp.setVisibility(View.INVISIBLE);
            mButtonDown.setVisibility(View.INVISIBLE);
            mButtonLeft.setVisibility(View.INVISIBLE);
            mButtonRight.setVisibility(View.INVISIBLE);
            mTextViewvv360x.setVisibility(View.INVISIBLE);
            mTextViewvv360y.setVisibility(View.INVISIBLE);
        }

        mEngine.setView(m_editorView);
        mEngine.set360VideoViewPosition(vv360x, vv360y);
        mEngine.setEventHandler(MediaPlayerActivity.this);
        mEngine.setProject(project);
        mEngine.updateProject();

        lastUIisPlaying = true;
    }

    private float getConvertedValue(int min, int intVal,float digit){
        double result = (min + intVal) / digit;
        //result = Math.floor( result * 100 ) / 100;
        return (float)result;
    }

    private void setVisibilityOfUIForHDR(int flag) {

        if(mSeekBarDeviceLightLevel == null){
            mSeekBarDeviceLightLevel = (SeekBar) findViewById(R.id.seekbar_media_play_device_light_level);
        }
        mSeekBarDeviceLightLevel.setVisibility(flag);

        if(mDeviceLightLevelTexView == null){
            mDeviceLightLevelTexView = (TextView) findViewById(R.id.textview_media_play_device_light_level);
        }
        mDeviceLightLevelTexView.setVisibility(flag);

        if(mDeviceLightLevelNameTextView == null){
            mDeviceLightLevelNameTextView = (TextView) findViewById(R.id.textview_media_play_device_light_level_name);
        }
        mDeviceLightLevelNameTextView .setVisibility(flag);

        if( mDeviceGammaTexView == null) {
            mDeviceGammaTexView = (TextView) findViewById(R.id.textview_media_play_device_gamma);
        }
        mDeviceGammaTexView.setVisibility(flag);

        if(mSeekBarDeviceGamma == null){
            mSeekBarDeviceGamma = (SeekBar)findViewById(R.id.seekbar_media_play_device_gamma);
        }
        mSeekBarDeviceGamma.setVisibility(flag);

        if(mDeviceGammaNameTextView == null){
            mDeviceGammaNameTextView = (TextView) findViewById(R.id.textview_media_play_device_gamma_name);
        }
        mDeviceGammaNameTextView .setVisibility(flag);

    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        String filepath = null;
        if(requestCode == 100) {
            if(data == null) {
                return ;
            }
            ClipData clipData = data.getClipData();
            if (clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                for (int i = 0; i < clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                }
            }
        }

        nexClip clip = nexClip.getSupportedClip(filepath);
        if( clip == null )
        {
            Toast.makeText(getApplicationContext(), "not support audio format for bgm.", Toast.LENGTH_LONG).show();
            return;
        }

        boolean isAudio = project.setBackgroundMusicPath(filepath);
        if(isAudio == true) {
            MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
            metadataRetriever.setDataSource(filepath);

            String title = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
            String artist = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
            int duration = clip.getAudioDuration();

            mTitle.setText(title);
            mArtist.setText(artist);
            int minute = duration / 60000;
            int second = (duration % 60000) / 1000;
            mDuration.setText("Play time\r\n" + minute + ":" + second);
            mAddBGM.setEnabled(false);
            mEngine.updateProject();
            mEngine.set360VideoViewPosition(vv360x, vv360y);
        }else{
            Toast.makeText(getApplicationContext(), String.format("Not supported clips"), Toast.LENGTH_SHORT).show();
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putBoolean("requestedPlay", lastUIisPlaying);
        outState.putInt("currentTime", mCurrentPlayTime);
        super.onSaveInstanceState(outState);
    }

    private static int debugTag = 0;

    @Override
    public void onStateChange(int oldState, int newState) {
        Log.i(TAG, "onStateChange("+instNum+"): oldState=" + oldState + ", newState=" + newState );
        if( newState == 1 ) {

            mState = State.STOPED;
            if( trackingSeektime != - 1) {
                seek(trackingSeektime);
                trackingSeektime = -1;
            }

            // for KMSA 353
            if(mClipVol!=null) {
                mClipVol.setText("100");
                mClipVolume = 100;
                mSeekBarClipVol.setProgress(100);
            }
            if(mBGMVol!=null) {
                mBGMVol.setText("100");
                mBGMVolume = 100;
                mSeekBarBGMVol.setProgress(100);
            }
            if(mPlayStop!=null) {
                mPlayStop.setText("Play");
            }
        }else if( newState == 2 ){
            mEngine.setTotalAudioVolumeWhilePlay(mClipVolume, mBGMVolume);
            mEngine.setTotalAudioVolumeProject(mClipVolume, mBGMVolume);

            mState = State.PLAYING;
            if(mPlayStop!=null) {
                mPlayStop.setText("Stop");
            }
        }
    }

    @Override
    public void onTimeChange(int currentTime) {
        if( !trackingSeekbarTouch ) {
            playTime = currentTime;
            mSeekBarPlayTime.setProgress(currentTime / 33);
            mPlayTime.setText(getTimeString(currentTime));
            mCurrentPlayTime = currentTime;
        }
    }

    @Override
    public void onSetTimeDone(int currentTime) {
        mSeekBarPlayTime.setProgress(currentTime / 33);
        mPlayTime.setText(getTimeString(currentTime));
        mCurrentPlayTime = currentTime;

        Log.i(TAG, String.format("onSetTimeDone("+instNum+"): currentTime=%d, seekInProgress=%b lastUIisPlaying=%b", currentTime, seekInProgress, lastUIisPlaying));

        mState = State.STOPED;

        if( trackingSeektime != - 1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if( !backgroundMode && lastUIisPlaying ){
            resume();
        }

        /*
        if( !seekInProgress && !trackingSeekbarTouch){

            mState = State.STOPED;
            if( lastUIisPlaying ){
                resume();
            }
        }
        */
    }

    @Override
    public void onSetTimeFail(int err) {
        Log.i(TAG,"onSetTimeFail(): err="+err+", seekInProgress="+seekInProgress);

        mState = State.STOPED;

        if( trackingSeektime != - 1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if( !backgroundMode && lastUIisPlaying ){
            resume();
        }

        /*
        if( !seekInProgress && !trackingSeekbarTouch ){

            mState = State.STOPED;

            if( lastUIisPlaying ){
                resume();
            }
        }
        */
    }

    @Override
    public void onSetTimeIgnored() {
        Log.i(TAG,"onSetTimeIgnored(): seekInProgress="+seekInProgress);

        mState = State.STOPED;

        if( trackingSeektime != - 1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if( !backgroundMode && lastUIisPlaying ){
            resume();
        }
        /*
        if( !seekInProgress && !trackingSeekbarTouch ){
            mState = State.STOPED;

            if( trackingSeektime != - 1) {
                seek(trackingSeektime);
                trackingSeektime = -1;
            }

            if( lastUIisPlaying ){
                resume();
            }
        }
        */
    }

    @Override
    public void onEncodingDone(boolean iserror, int result) {

    }

    @Override
    public void onPlayEnd() {
        Log.d(TAG,"onPlayEnd");
        mPlayStop.setText("Play");
        lastUIisPlaying = false;
        mEngine.seek(0);

        mClipVolume = 100;
        mClipVol.setText("100");
        mSeekBarClipVol.setProgress(100);

        mBGMVolume = 100;
        mBGMVol.setText("100");
        mSeekBarBGMVol.setProgress(100);
    }

    @Override
    public void onPlayFail(int err, int iClipID) {
        Log.d(TAG,"onPlayFail");
    }

    @Override
    public void onPlayStart() {
        Log.d(TAG,"onPlayStart");

    }

    @Override
    public void onClipInfoDone() {

    }

    @Override
    public void onSeekStateChanged(boolean isSeeking) {
        Log.i(TAG,"onSeekStateChanged() isSeeking="+isSeeking );
        seekInProgress = isSeeking;
    }

    @Override
    public void onEncodingProgress(int percent) {

    }

    @Override
    public void onCheckDirectExport(int result) {

    }

    @Override
    public void onProgressThumbnailCaching(int progress, int maxValue) {

    }

    @Override
    public void onFastPreviewStartDone(int err, int startTime, int endTime) {

    }

    @Override
    public void onFastPreviewStopDone(int err) {

    }

    @Override
    public void onFastPreviewTimeDone(int err) {

    }

    @Override
    public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {
    }

    private void startPlay(){
        if( !isAvailableView && !backgroundMode){
            postOnAvailableSurfaceRunnables.add(new Runnable() {
                @Override
                public void run() {
                    startPlay();
                }
            });
        }else {
            if (mState == State.STOPED || mState == State.IDLE) {
                mState = State.PLAY_READY;
                mEngine.play();
            } else {
                Log.w(TAG, "startPlay() invaild state=" + mState);
            }
        }
    }

    private void resume(){
        if( !isAvailableView && !backgroundMode){
            postOnAvailableSurfaceRunnables.add(new Runnable() {
                @Override
                public void run() {
                    resume();
                }
            });
        }else {
            if (mState == State.STOPED || mState == State.IDLE) {
                mState = State.PLAY_READY;
                mEngine.resume();
            } else {
                Log.w(TAG, "resume() invaild state=" + mState);
            }
        }
    }

    private void stop(){
        if (mState == State.PLAYING || mState == State.PLAY_READY) {
            mState = State.STOP_READY;
            mEngine.stop();
        } else {
            Log.w(TAG, "stop() invaild state=" + mState);
        }
    }

    private int mSeekMode = 1; /*1: Seek, 2: Seek IDR*/
    private int mSeekCurrentTS;
    private int mSeekms = 0;
    private void seek(int ms){
        if( !isAvailableView && !backgroundMode){
            mSeekms = ms;
            postOnAvailableSurfaceRunnables.add(new Runnable() {
                @Override
                public void run() {
                    seek(mSeekms);
                }
            });
        }else {
            mState = State.SEEKING;
            mSeekCurrentTS = ms;
            if (mSeekMode == 1) {
                mEngine.seek(ms);
            } else {
                mEngine.seekIDRorI(ms);
            }
        }
        /*
        if( mState == State.STOPED )
        {
            mState = State.SEEKING;
            mSeekCurrentTS = ms;
            if(mSeekMode == 1) {
                mEngine.seek(ms);
            } else {
                mEngine.seekIDRorI(ms);
            }
            Log.i(TAG, "seek(): time=" + ms + ", seekInProgress=" + seekInProgress);
        }
        else if( mState == State.SEEKING )
        {
            Log.i(TAG, "seek ignore because current state already seek");
        }else{
            Log.w(TAG,"update() invaild state="+mState );
        }
        */
    }

    private String getTimeString(int time) {

            return
                    String.format("%02d:%02d:%02d.%02d",
                            time / 3600000,
                            (time % 3600000)/60000,
                            (time % 60000)/1000,
                            (time%1000)/10);
    }

    @Override
    protected void onPause() {
        backgroundMode = true;
        Log.d(TAG, "onPause");
        mEngine.stop();
        nexEngine.nexErrorCode errorCode = mEngine.captureCurrentFrame(new nexEngine.OnCaptureListener() {
            @Override
            public void onCapture(Bitmap bitmap) {
                Log.d(TAG,"captureCurrentFrame Ok!=("+bitmap.getWidth()+","+bitmap.getHeight()+")");
                m_editorView.setVisibility(View.GONE);
                mScreenView.setVisibility(View.VISIBLE);
                mScreenView.setImageBitmap(bitmap);

                mEngine.set360VideoForceNormalView();
                if( mChangeAspect ){
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_2v1);
                    mEngine.updateScreenMode();
                    mChangeAspect = false;
                }

                isCaptureScreen = true;
            }

            @Override
            public void onCaptureFail(nexEngine.nexErrorCode nexErrorCode) {
                Log.d(TAG,"captureCurrentFrame fail!="+nexErrorCode);
            }
        });
        super.onPause();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        if( mEngine != null){
            mEngine.set360VideoViewPosition(vv360x,vv360y);
        }
        backgroundMode = false;
        if( lastUIisPlaying ) {
            changeEngineView();
            seek(mCurrentPlayTime);
            resume();
            mPlayStop.setText("Stop");
        }else{
            seek(mCurrentPlayTime);
        }
        super.onResume();
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        nexClip.cancelThumbnails();
        mEngine.set360VideoForceNormalView();
        if( mChangeAspect ){
            nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_2v1);
            mEngine.updateScreenMode();
            mChangeAspect = false;
        }
        if(mEngine!= null)
      	    mEngine.stop();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();

    }

    private void changeEngineView(){
        if( isCaptureScreen ){
            m_editorView.setVisibility(View.VISIBLE);
            mScreenView.setVisibility(View.GONE);
            isCaptureScreen = false;
        }
    }

    private void updateTextureViewSize(int pivotPointX, int pivotPointY, float scale ) {
        Matrix matrix = new Matrix();
        matrix.setScale(scale, scale, pivotPointX, pivotPointY);
        Log.d(TAG,"updateTextureViewSize scaleX="+scale+", pivotPointX="+pivotPointX+" , pivotPointY="+pivotPointY);
        m_editorView.setTransform(matrix);
        m_editorView.setLayoutParams(new FrameLayout.LayoutParams(screenWidth, screenHeight));
    }

}
