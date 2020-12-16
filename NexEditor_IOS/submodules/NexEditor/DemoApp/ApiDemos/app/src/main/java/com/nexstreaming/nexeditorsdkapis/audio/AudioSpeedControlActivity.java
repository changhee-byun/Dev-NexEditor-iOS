/******************************************************************************
 * File Name        : AudioSpeedControlActivity.java
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

package com.nexstreaming.nexeditorsdkapis.audio;

import android.app.Activity;
import android.content.Intent;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexVideoClipEdit;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.util.ArrayList;


public class AudioSpeedControlActivity extends Activity {
    private static final String TAG = "AudioSpeedControl";
    private Spinner mSpinner;
    private Spinner mSpinner2;
    private Button mPlay;
    private Button mExport;
    private nexEngine mEngin;
    private nexEngineListener mEditorListener = null;
    private TextView mTime;
    private ArrayList<String> m_listfilepath;
    private nexEngineView m_editorView;
    //private SurfaceView m_surfaceView;
    private boolean mExportMode;
    private SeekBar mBGMVol;
    private SeekBar mClipVol;
    private Switch mBGMOnOff;
    private Switch mClipOnOff;

    private ProgressBar mProgress;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_audio_speed_control);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        nexProject project = new nexProject();
        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if(clip != null) {
            if (clip.getClipType() != nexClip.kCLIP_TYPE_VIDEO) {
                Toast.makeText(getApplicationContext(), "No Video Clips.", Toast.LENGTH_LONG).show();
                finish();
                return;
            }

            if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                clip.setRotateDegree(clip.getRotateInMeta());
            }

            clip.setAudioOnOff(false);

            project.add(clip);
/*
                    project.add(nexClip.dup(clip));
                    project.add(nexClip.dup(clip));
                    project.add(nexClip.dup(clip));
                    project.add(nexClip.dup(clip));

                    project.getClip(0,true).getVideoClipEdit().setTrim(0,1498);
                    project.getClip(1,true).getVideoClipEdit().setTrim(1498,10893);
                    project.getClip(1,true).getVideoClipEdit().setSpeedControl(400);
                    project.getClip(2,true).getVideoClipEdit().setTrim(10893,13559);
                    project.getClip(3,true).getVideoClipEdit().setTrim(13559,28464);
                    project.getClip(3,true).getVideoClipEdit().setSpeedControl(25);
                    project.getClip(4,true).getVideoClipEdit().setTrim(28464,30000);
*/
        }
        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();

        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                //Log.d(TAG, "onTimeChange: " + currentTime);
                mTime.setText(String.valueOf(currentTime));
                mProgress.setProgress(currentTime);
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

        mEngin.setEventHandler(mEditorListener); // null

        m_editorView = (nexEngineView) findViewById(R.id.engineview_audio_speed_control_editorview);
//                m_surfaceView = (SurfaceView) findViewById(R.id.engineview_audio_speed_control_editorview);

        //m_editorView.setBlackOut(true);
        mEngin.setView(m_editorView);
        mEngin.setProject(project);
        mEngin.updateProject();

        mBGMVol = (SeekBar) findViewById(R.id.seekbar_audio_speed_control_bgm);
        mBGMVol.setMax(200);
        mBGMVol.setProgress(100);
        mBGMVol.setEnabled(false);

        mClipVol = (SeekBar) findViewById(R.id.seekbar_audio_speed_control_clip);
        mClipVol.setMax(200);
        mClipVol.setProgress(100);

        mBGMOnOff = (Switch) findViewById(R.id.switch_audio_speed_control_bgm);
        mBGMOnOff.setChecked(true);
        mBGMOnOff.setEnabled(false);
        mClipOnOff = (Switch) findViewById(R.id.switch_audio_speed_control_clip);
        mClipOnOff.setChecked(true);

        mTime = (TextView) findViewById(R.id.textview_audio_speed_control_time);

        mSpinner = (Spinner) findViewById(R.id.spinner_audio_speed_control);

	Integer[] items = new Integer[]{12, 25, 50, 75, 100, 125, 150, 175, 200, 400,800, nexVideoClipEdit.kSpeedControl_MaxValue};
	ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);

        mSpinner.setAdapter(adapter);
        mSpinner.setSelection(4);

        mSpinner2 = (Spinner) findViewById(R.id.spinner_audio_speed_pitch);

        String[] items2 = new String[]{"Pitch on", "Pitch off" };
        ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items2);

        mSpinner2.setAdapter(adapter2);
        mSpinner2.setSelection(0);

        mProgress = (ProgressBar) findViewById(R.id.progressbar_audio_speed_control_speed);

        mProgress.setProgress(0);
        mProgress.setMax(mEngin.getProject().getTotalTime());

        mPlay = (Button) findViewById(R.id.button_audio_speed_control_play);
        mPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                if (setData() != false) {

                    mEngin.play();
                }
            }
        });

        mExport = (Button) findViewById(R.id.button_audio_speed_control_export);
        mExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                mExportMode = true;
                setData();
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });

        final Button btIntent = (Button) findViewById(R.id.button_audio_speed_control_intent);
        btIntent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                //startActivity(mEngin.getProject().makeKineMasterIntent());
                UtilityCode.launchKineMaster(AudioSpeedControlActivity.this, m_listfilepath);
            }
        });

    }

    private boolean setData(){
        Integer i =  (Integer) mSpinner.getSelectedItem();
        try {
            mEngin.getProject().getClip(0,true).getVideoClipEdit().setSpeedControl(i.intValue());
        } catch(InvalidRangeException e) {
            if(i.intValue() != 100) {
                Toast.makeText(getApplicationContext(), "this content is invalid range so you can`t assign speed control, retry again.", Toast.LENGTH_SHORT).show();
                return false;
            }
        }

        String s =  (String) mSpinner2.getSelectedItem();
        try {
            int val = 1;

            if(s.equals("Pitch on"))
                val = 1;
            else if (s.equals("Pitch off"))
                val = 0;

            mEngin.getProject().getClip(0,true).getVideoClipEdit().setKeepPitch(val);
        } catch(InvalidRangeException e) {
                Toast.makeText(getApplicationContext(), "this content is invalid pitch range, retry again.", Toast.LENGTH_SHORT).show();
                return false;
        }

        mEngin.getProject().getClip(0, true).setAudioOnOff(mClipOnOff.isChecked());
        mEngin.getProject().getClip(0, true).setClipVolume(mClipVol.getProgress());
        if( mBGMOnOff.isChecked() ) {
            mEngin.getProject().getClip(0, true).setBGMVolume(mBGMVol.getProgress());
        }else{
            mEngin.getProject().getClip(0, true).setBGMVolume(0);
        }
        mEngin.updateProject();
        mProgress.setMax(mEngin.getProject().getTotalTime());
        return true;
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart");
        if( mEngin != null){
            mEngin.setEventHandler(mEditorListener);
        }
        mExportMode = false;
        super.onStart();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop");
        if( !mExportMode ){
            mEngin.stop();
        }
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        mEngin.stop();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
