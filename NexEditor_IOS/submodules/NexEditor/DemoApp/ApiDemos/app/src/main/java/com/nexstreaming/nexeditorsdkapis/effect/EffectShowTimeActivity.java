/******************************************************************************
 * File Name        : EffectShowTimeActivity.java
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

package com.nexstreaming.nexeditorsdkapis.effect;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.Constants;

import java.util.ArrayList;


public class EffectShowTimeActivity extends Activity {
    private static final String TAG = "EffectShowTimeActivity";
    private SeekBar seekBar_startTime;
    private TextView textView_startTime;


    private SeekBar seekBar_endTime;
    private TextView textView_endTime;

    private ProgressBar progressBar_playtime;
    private TextView textView_playtime;

    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;

    private int mStartTime = 0;
    private int mEndTime = 0;

    private int kClipDuration;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_effect_show_time);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
            kClipDuration = 10000;
        }else{
            kClipDuration = clip.getTotalTime();
        }

        if( clip != null ){
            project.add(clip);
            project.getClip(0,true).setImageClipDuration(kClipDuration);
            project.getClip(0,true).getClipEffect().setEffect("com.nexstreaming.kinemaster.builtin.title.bam");
            project.getClip(0,true).getClipEffect().setTitle("Title test");
            project.getClip(0,true).getClipEffect().setShowStartTime(0);
            project.getClip(0,true).getClipEffect().setShowEndTime(kClipDuration);
//            project.getClip(0,true).getClipEffect().setRandomCropMode(nexClipEffect.CropMode.FIT);
        }

        mEditorListener = new nexEngineListener(){
            private int mLastTimeSec = 0;
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                textView_playtime.setText(String.valueOf(currentTime/1000));
                progressBar_playtime.setProgress(currentTime/1000);
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

            }

            @Override
            public void onPlayEnd() {

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
        };

        m_editorView = (nexEngineView)findViewById(R.id.editorview_effect_show_time);
        m_editorView.setBlackOut(true);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.setEventHandler(mEditorListener);

        textView_playtime = (TextView)findViewById(R.id.textview_effect_show_time_playtime);
        progressBar_playtime = (ProgressBar)findViewById(R.id.progressbar_effect_show_time_playtime);
        progressBar_playtime.setMax(kClipDuration/1000);
        progressBar_playtime.setProgress(0);

        seekBar_startTime = (SeekBar)findViewById(R.id.seekbar_effect_show_time_starttime);
        textView_startTime = (TextView)findViewById(R.id.textview_effect_show_time_starttime);
        seekBar_startTime.setMax(kClipDuration/1000);
        seekBar_startTime.setProgress(0);
        textView_startTime.setText("0");

        seekBar_startTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_startTime.setText(""+ progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_startTime.setText("" + value);
                mStartTime = value * 1000;
            }
        });

        seekBar_endTime = (SeekBar)findViewById(R.id.seekbar_effect_show_time_endtime);
        textView_endTime = (TextView)findViewById(R.id.textview_effect_show_time_endtime);
        seekBar_endTime.setMax(kClipDuration/1000);
        seekBar_endTime.setProgress(kClipDuration/1000);
        textView_endTime.setText(""+kClipDuration/1000);
        mEndTime = kClipDuration;
        seekBar_endTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = kClipDuration/1000;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_endTime.setText(""+ progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_endTime.setText("" + value);
                mEndTime = value * 1000;
            }
        });

        Button bt = (Button)findViewById(R.id.button_effect_show_time_play);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mEndTime <= mStartTime) {
                    Toast.makeText(getApplicationContext(), "startTime >= endTime, retry again!!", Toast.LENGTH_SHORT).show();
                    return ;
                }
                //mEngine.stop();
                mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        mEngine.getProject().getClip(0,true).getClipEffect().setEffectShowTime(mStartTime, mEndTime);
                        mEngine.play();
                    }
                });
            }
        });

        ((Button)findViewById(R.id.button_effect_show_time_set)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
                intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_CLIP);
                startActivityForResult(intent, 100);
            }
        });

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.EFFECT_OPTION);
            String effect = data.getStringExtra(Constants.EFFECT);
            mEngine.getProject().getClip(0,true).getClipEffect().setEffect(effect);
            mEngine.getProject().getClip(0,true).getClipEffect().updateEffectOptions(opt, true);
            mEngine.updateProject();
        }

    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        ApiDemosConfig.getApplicationInstance().getEngine();
    }

    @Override
    protected void onPause() {
        mEngine.stop();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
    }

}
