/******************************************************************************
 * File Name        : AutoTrimTestActivity.java
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
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.util.ArrayList;

public class AutoTrimTestActivity extends Activity {
    private enum State {
        AUTOTRIM,
        PLAY,
        STOP,
        AUTOTRIM_STOP,
        IDLE
    };

    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private nexClip mClip;
    private nexProject mProject;
    private nexEngineListener mEditorListener = null;
    private TextView mTextViewResult;
    private TextView mTextViewDuration;
    private ProgressBar mProgressAutoTrim;
    private Button mButtonPlay;
    private Button mButtonExport;
    private Button mButtonAutoTrim;
    private TextView mTV_count;
    private TextView mTV_count_value;
    private SeekBar mSeekbarTrimCount;
    private SeekBar mSeekbarTrimInterval;

    private int mTrimCount = 1;
    private int mAutoTrim_Success = 1;
    private int mTrimInterval = 1000;
    private int mSeekTab[];

    // if this feature is 1, autotrim speed will be high. default is 0
    private int mDecodeMode = 0;

    private State mState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_auto_trim_test);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        mClip = nexClip.getSupportedClip(m_listfilepath.get(0));
        mSeekTab = mClip.getSeekPointsSync();
        if(mSeekTab.length == 1)
            Toast.makeText(getApplicationContext(), "I-Frame only one content ", Toast.LENGTH_SHORT).show();


        if( mClip == null ) {
            finish();
            return;
        }

        mProject = new nexProject();
        mProject.add(mClip);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        mEditorListener = new nexEngineListener() {
            @Override
            public void onCheckDirectExport(int result) {

            }

            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                mTextViewDuration.setText(currentTime + "");
                mProgressAutoTrim.setProgress(currentTime);
            }

            @Override
            public void onSetTimeDone(int currentTime) {

            }

            @Override
            public void onSetTimeFail(int err) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean iserror, int result) {
                mState = State.STOP;
                mButtonPlay.setText("PLAY");
            }

            @Override
            public void onPlayEnd() {
                mState = State.STOP;
                mButtonPlay.setText("PLAY");
            }

            @Override
            public void onPlayFail(int err, int iClipID) {

            }

            @Override
            public void onPlayStart() {

            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {

            }

            @Override
            public void onEncodingProgress(int percent) {

            }

            @Override
            public void onProgressThumbnailCaching(int progress, int maxValue) {
                mProgressAutoTrim.setProgress(progress);
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
        };

        mEngine.setProject(mProject);
        mEngine.setEventHandler(mEditorListener);
        m_editorView = (nexEngineView)findViewById(R.id.videoView_autotrim);
        mEngine.setView(m_editorView);

        mProgressAutoTrim = (ProgressBar)findViewById(R.id.progressBar_autotrim);
        mProgressAutoTrim.setMax(100);
        mProgressAutoTrim.setProgress(0);

        mTextViewResult = (TextView)findViewById(R.id.textview_autotrim_result);
        mTextViewDuration = (TextView)findViewById(R.id.textview_autotrim_duration);
        mTV_count = (TextView)findViewById(R.id.tv_count);
        mTV_count_value = (TextView) findViewById(R.id.textview_autotrim_trimcount);

        mSeekbarTrimCount = (SeekBar)findViewById(R.id.seekbar_autotrim_trimcount);
        mSeekbarTrimCount.setMax(100);
        mSeekbarTrimCount.setProgress(1);
        mSeekbarTrimCount.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            int min = 1;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (progress < min) {
                    mTV_count_value.setText("" + min);
                    value = min;
                    mSeekbarTrimCount.setProgress(value);
                } else {
                    mTV_count_value.setText("" + progress);
                    value = progress;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mTrimCount = value;
            }
        });

        mSeekbarTrimInterval = (SeekBar)findViewById(R.id.seekbar_autotrim_triminterval);
        mSeekbarTrimInterval.setMax(mClip.getTotalTime() / 10);
        mSeekbarTrimInterval.setProgress(1000);
        mSeekbarTrimInterval.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            int min = 1000;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (progress < min) {
                    ((TextView) findViewById(R.id.textview_autotrim_triminterval)).setText("" + min);
                    value = min;
                    mSeekbarTrimInterval.setProgress(value);

                } else {
                    ((TextView) findViewById(R.id.textview_autotrim_triminterval)).setText("" + progress);
                    value = progress;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mTrimInterval = value;
            }
        });


        mButtonAutoTrim = (Button)findViewById(R.id.button_autotrim_trim);
        mButtonAutoTrim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.PLAY) {
                    Toast.makeText(getApplicationContext(), "Playing...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(mState == State.AUTOTRIM) {
                    // autotrim stop event
                    mEngine.autoTrimStop();

                    mState = State.AUTOTRIM_STOP;
                    mButtonAutoTrim.setText("AutoTrim");
                    mButtonPlay.setEnabled(true);
                    mButtonExport.setEnabled(true);
                } else {
                    if(mTrimCount < 1 || mTrimInterval < 1000) {
                        Toast.makeText(getApplicationContext(), "trimCount < 1, trimInterval < 1000, retry again!!", Toast.LENGTH_SHORT).show();
                        return ;
                    }

                    mState = State.AUTOTRIM;
                    mButtonAutoTrim.setText("Stop");
                    mButtonPlay.setEnabled(false);
                    mButtonExport.setEnabled(false);

                    mProgressAutoTrim.setMax(100);
                    mEngine.autoTrim(mClip.getPath(), true, mTrimInterval, mTrimCount, mDecodeMode, new nexEngine.OnAutoTrimResultListener() {
                        @Override
                        public void onAutoTrimResult(int errcode, int[] arrayData) {
                            if (arrayData == null || arrayData.length == 0) {
                                if (mAutoTrim_Success == 1)
                                    if (errcode != 0)
                                        updateTextViewResult("Error !!! code(" + errcode + ")");
                                    else if (mAutoTrim_Success == 0)
                                        updateTextViewResult("Can not auto trim");

                                mEngine.autoTrimStop();
                            } else {
                                mProject.allClear(true);
                                StringBuilder result = new StringBuilder();
                                for (int i = 0; i < arrayData.length; i++) {
                                    if (arrayData[i] < mClip.getTotalTime()) {
                                        result.append("CTS : " + arrayData[i] + "\n");
                                        updateTextViewResult(result.toString());
                                        mProject.add(nexClip.dup(mClip));
                                        mProject.getClip(i, true).getVideoClipEdit().setTrim(arrayData[i], arrayData[i] + mTrimInterval - 1);
                                    } else if (arrayData.length == 1 && arrayData[i] + mTrimInterval > mClip.getTotalTime()) {
                                        mAutoTrim_Success = 0;
                                    }
                                }
                            }

                            mState = State.AUTOTRIM_STOP;
                            mButtonPlay.setEnabled(true);
                            mButtonExport.setEnabled(true);
                            mButtonAutoTrim.setText("AutoTrim");
                        }
                    });
                }
            }
        });

        mButtonPlay = (Button)findViewById(R.id.button_autotrim_playstop);
        mButtonPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.AUTOTRIM) {
                    Toast.makeText(getApplicationContext(), "Autotriming...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(mState == State.STOP || mState == State.IDLE || mState == State.AUTOTRIM_STOP) {

                  if(mAutoTrim_Success == 1){
                            mButtonPlay.setText("STOP");
                            mProgressAutoTrim.setMax(mProject.getTotalTime());
                            mState = State.PLAY;
                            mEngine.play();
                    }else updateTextViewResult("Can not play");
                } else {
                    mButtonPlay.setText("PLAY");
                    mEngine.stop();
                    mState = State.STOP;
                }
            }
        });

        mButtonExport = (Button) findViewById(R.id.button_autotrim_export);
        mButtonExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.AUTOTRIM) {
                    Toast.makeText(getApplicationContext(), "Autotriming...", Toast.LENGTH_SHORT).show();
                    return ;
                }
                if(mAutoTrim_Success == 1) {
                    mEngine.stop();
                    mState = State.STOP;
                    mButtonPlay.setText("PLAY");
                    Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                    startActivity(intent);
                }else updateTextViewResult("Can not export");
            }
        });
    }

    void updateTextViewResult(final String str)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTextViewResult.setText(str);
            }
        });
    }

    @Override
    protected void onStop() {
        if(mState == State.PLAY) {
            mEngine.stop();
            mState = State.STOP;
        }
        super.onStop();
    }


    @Override
    protected void onStart() {
        mState = State.STOP;
        mEngine.setEventHandler(mEditorListener);
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        if(mState == State.AUTOTRIM) {
            // autotrim stop event
            mEngine.autoTrimStop();

            mState = State.AUTOTRIM_STOP;
        }
        mEngine.stop();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
