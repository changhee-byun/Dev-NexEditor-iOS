/******************************************************************************
 * File Name        : ClipMultiTrimActivity.java
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

package com.nexstreaming.nexeditorsdkapis.edit;

import android.app.Activity;
import android.content.Intent;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.text.format.Time;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.nexstreaming.nexeditorsdk.exception.ProjectNotAttachedException;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexVideoClipEdit;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.scmbsdk.nxSCMBEntry;
import com.nexstreaming.scmbsdk.nxSCMBSDK;
import com.nexstreaming.scmbsdk.ErrorCode;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;


public class ClipMultiTrimActivity extends Activity {
    private static final String TAG = "ClipMultiTrimActivity";
    private nexEngine mEngine;
    private nexProject mProject;
    private nexEngineView m_editorView;
    private ListView m_ListView_Start;
    private ListView m_ListView_End;
    private ListView m_ListView_Speed;
    private ListView m_ListView_PlayMode;
    private EditText m_StartTrim;
    private EditText m_EndTrim;
    private ToggleButton mToggleButton;
    private Button mButtonExport;
    private Button mButtonMix;
    private Button mButtonPlay;
    private Button mButtonDirect;
    private Button mButtonClear;
    private Spinner mSpinnerSpeed;
    private Spinner mSpinnerIframePlayMode;
    private SeekBar mSeekBarAvSync;
    private TextView mTvAvsync;
    private ArrayAdapter<String> m_ListAdapter_Start;
    private ArrayAdapter<String> m_ListAdapter_End;
    private ArrayAdapter<String> m_ListAdapter_Speed;
    private ArrayAdapter<String> m_ListAdapter_Playmode;
    private ArrayList<String> mStartData = new ArrayList<String>();
    private ArrayList<String> mEndData = new ArrayList<String>();
    private ArrayList<String> mSpeedData = new ArrayList<String>();
    private ArrayList<String> mPlaymode = new ArrayList<String>();
    private ArrayList<String> m_listfilepath;
    private ProgressBar progressBar;
    private long time =0;
    private File mFile;
    int ismergeable = -1;
    private int mAVSyncGap = 0;

    private boolean isUHDcontent;
    private boolean mExportMode;
    private  nexEngineListener mEditorListener;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_clip_multi_trim);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        mProject = new nexProject();

        m_ListView_Start = (ListView) findViewById(R.id.listview_clip_multi_trim_start);
        m_ListView_End = (ListView) findViewById(R.id.listView_clip_multi_trim_end);
        m_ListView_Speed = (ListView) findViewById(R.id.listView_clip_multi_trim_speed);
        m_ListView_PlayMode = (ListView) findViewById(R.id.listView_clip_play_mode);

        m_ListAdapter_Start = new ArrayAdapter<String>(getApplicationContext(), android.R.layout.simple_list_item_1, mStartData);
        m_ListAdapter_End = new ArrayAdapter<String>(getApplicationContext(), android.R.layout.simple_list_item_1, mEndData);
        m_ListAdapter_Speed = new ArrayAdapter<String>(getApplicationContext(), android.R.layout.simple_list_item_1, mSpeedData);
        m_ListAdapter_Playmode = new ArrayAdapter<String>(getApplicationContext(), android.R.layout.simple_list_item_1, mPlaymode);

        m_StartTrim = (EditText) findViewById(R.id.edittext_clip_multi_trim_start);
        m_EndTrim = (EditText) findViewById(R.id.edittext_clip_multi_trim_end);
        mSpinnerSpeed = (Spinner)findViewById(R.id.spinner_clip_multi_trim);
        Integer[] items = new Integer[]{3,6,13, 25, 50, 75, 100, 125, 150, 175, 200, 400 , 800 , nexVideoClipEdit.kSpeedControl_MaxValue};

        mSpinnerIframePlayMode = (Spinner)findViewById(R.id.spinner_clip_iframeplay_mode);
        List<String> items_iframe = new ArrayList<String>();
        items_iframe.add("TRUE");
        items_iframe.add("FALSE");

        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);
        mSpinnerSpeed.setAdapter(adapter);
        mSpinnerSpeed.setSelection(6);

        ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items_iframe);
        mSpinnerIframePlayMode.setAdapter(adapter2);
        mSpinnerIframePlayMode.setSelection(0);

        mSpinnerIframePlayMode.getSelectedItem();
        m_ListView_Start.setAdapter(m_ListAdapter_Start);
        m_ListView_End.setAdapter(m_ListAdapter_End);
        m_ListView_Speed.setAdapter(m_ListAdapter_Speed);
        m_ListView_PlayMode.setAdapter(m_ListAdapter_Playmode);

        m_ListView_PlayMode.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                mStartData.remove(position);
                mEndData.remove(position);
                mSpeedData.remove(position);
                mPlaymode.remove(position);
                mProject.remove(mProject.getClip(position, true));
                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();
                m_ListAdapter_Playmode.notifyDataSetChanged();
                return false;
            }
        });

        m_ListView_Start.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                mStartData.remove(position);
                mEndData.remove(position);
                mSpeedData.remove(position);
                mPlaymode.remove(position);
                mProject.remove(mProject.getClip(position, true));
                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();
                m_ListAdapter_Playmode.notifyDataSetChanged();
                return false;
            }
        });
        m_ListView_End.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                mStartData.remove(position);
                mEndData.remove(position);
                mSpeedData.remove(position);
                mPlaymode.remove(position);
                mProject.remove(mProject.getClip(position, true));
                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();
                m_ListAdapter_Playmode.notifyDataSetChanged();
                return false;
            }
        });

        m_ListView_Speed.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                mStartData.remove(position);
                mEndData.remove(position);
                mSpeedData.remove(position);
                mPlaymode.remove(position);
                mProject.remove(mProject.getClip(position, true));
                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();
                m_ListAdapter_Playmode.notifyDataSetChanged();
                return false;
            }
        });

        mToggleButton = (ToggleButton) findViewById(R.id.togglebutton_clip_multi_trim);
        mToggleButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                mProject.allClear(true);
                mEngine.updateProject();
                mStartData.clear();
                mEndData.clear();
                mSpeedData.clear();
                mPlaymode.clear();

                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();
                m_ListAdapter_Playmode.notifyDataSetChanged();

                mButtonDirect.setEnabled(false);
                mButtonPlay.setEnabled(false);
                mButtonMix.setEnabled(false);
                mButtonExport.setEnabled(false);
                mButtonClear.setEnabled(false);
            }
        });

        mButtonPlay = (Button) findViewById(R.id.button_clip_multi_trim_play);
        mButtonPlay.setEnabled(false);
        mButtonPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                if (m_ListView_Start.getAdapter().getCount() != 0) {
                    for (int i = 0; i < m_ListView_Start.getAdapter().getCount(); i++) {
                        int startTime = 0;
                        int endTime = 0;
                        int speed = 100;
                        boolean playmode = true;
                        if (mToggleButton.isChecked()) {
                            startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                            endTime = mProject.getClip(0, true).getTotalTime() - Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                        } else {
                            startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                            endTime = Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                        }
                        speed = Integer.parseInt(m_ListView_Speed.getAdapter().getItem(i).toString());
                        playmode = Boolean.parseBoolean(m_ListView_PlayMode.getAdapter().getItem(i).toString());


                        Log.d(TAG, "startTrimTime = " + startTime);
                        Log.d(TAG, "endTrimTime = " + endTime);
                        Log.d(TAG, "speed = " + speed);
                        mProject.getClip(i, true).getVideoClipEdit().setTrim(startTime, endTime);
                        mProject.getClip(i, true).getVideoClipEdit().setSpeedControl(speed);
                        mProject.getClip(i, true).setAVSyncTime(mAVSyncGap);
                        //mProject.getClip(i, true).getVideoClipEdit().setTrim(Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString()), Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString()));
                        mProject.getClip(i, true).setClipIFamePlayMode(playmode);
                    }
                }
                mEngine.updateProject();

                saveSCMB();

                mEngine.resume();

            }
        });

        mButtonClear = (Button) findViewById(R.id.button_clip_multi_trim_clear);
        mButtonClear.setEnabled(false);
        mButtonClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                mProject.allClear(true);
                mEngine.updateProject();
                mStartData.clear();
                mEndData.clear();
                mSpeedData.clear();
                mPlaymode.clear();

                clearSCMB();
                m_ListAdapter_Start.notifyDataSetChanged();
                m_ListAdapter_End.notifyDataSetChanged();
                m_ListAdapter_Speed.notifyDataSetChanged();

                mButtonDirect.setEnabled(false);
                mButtonPlay.setEnabled(false);
                mButtonMix.setEnabled(false);
                mButtonExport.setEnabled(false);
                mButtonClear.setEnabled(false);
            }
        });

        Button btadd = (Button) findViewById(R.id.button_clip_multi_trim_add);
        btadd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (m_StartTrim.length() != 0 && m_EndTrim.length() != 0) {
                    if (Integer.parseInt(m_EndTrim.getText().toString()) - Integer.parseInt(m_StartTrim.getText().toString()) > 500) {
                        nexClip clip = new nexClip(m_listfilepath.get(0));
                        if (clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                            clip.setRotateDegree(clip.getRotateInMeta());
                        }
                        mProject.add(clip);
                        mProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);

                        isUHDcontent = mProject.getClip(0, true).getWidth() == 3840 ? true : false;
                        mStartData.add(m_StartTrim.getText().toString());
                        mEndData.add(m_EndTrim.getText().toString());
                        int speed = ((Integer)mSpinnerSpeed.getSelectedItem()).intValue();
                        String mode = ((String)mSpinnerIframePlayMode.getSelectedItem()).toString();

                        if( speed < 13 && clip.getFramesPerSecond() < 120 ){
                            speed = 13;
                        }

                        mSpeedData.add(""+speed);
                        mPlaymode.add(mode);

                        m_ListAdapter_Start.notifyDataSetChanged();
                        m_ListAdapter_End.notifyDataSetChanged();
                        m_ListAdapter_Speed.notifyDataSetChanged();
                        m_ListAdapter_Playmode.notifyDataSetChanged();

                        m_StartTrim.getText().clear();
                        m_EndTrim.getText().clear();
                        mSpinnerSpeed.setSelection(6);
                    } else {
                        Toast.makeText(getApplicationContext(), "Trim duration is under 500ms", Toast.LENGTH_SHORT).show();
                    }

                    mButtonDirect.setEnabled(true);
                    mButtonPlay.setEnabled(true);
                    mButtonMix.setEnabled(true);
                    mButtonExport.setEnabled(true);
                    mButtonClear.setEnabled(true);
                }


            }
        });

        mTvAvsync = (TextView)findViewById(R.id.textview_clip_multi_trim_avsync);
        mSeekBarAvSync = (SeekBar) findViewById(R.id.seekbar_clip_multi_trim_avsync);
        mSeekBarAvSync.setMax(1000);
        mSeekBarAvSync.setProgress(500);
        mSeekBarAvSync.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mAVSyncGap = progress - 500;
                mTvAvsync.setText(""+mAVSyncGap);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        progressBar = (ProgressBar) findViewById(R.id.progressbar_clip_multi_trim);
        progressBar.setMax(100);

        mButtonExport = (Button) findViewById(R.id.button_clip_multi_trim_export);
        mButtonExport.setEnabled(false);
        mButtonExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                int startTime = 0;
                int endTime = 0;
                int speed = 100;
                boolean playmode =true;

                for (int i = 0; i < m_ListView_Start.getAdapter().getCount(); i++) {
                    if (mToggleButton.isChecked()) {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = mProject.getClip(0, true).getTotalTime() - Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    } else {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    }
                    speed = Integer.parseInt(m_ListView_Speed.getAdapter().getItem(i).toString());
                    playmode = Boolean.parseBoolean(m_ListView_PlayMode.getAdapter().getItem(i).toString());
                    Log.d(TAG, "startTrimTime = " + startTime);
                    Log.d(TAG, "endTrimTime = " + endTime);
                    Log.d(TAG, "speed = " + speed);
                    mProject.getClip(i, true).getVideoClipEdit().setTrim(startTime, endTime);
                    mProject.getClip(i, true).getVideoClipEdit().setSpeedControl(speed);
                    mProject.getClip(i, true).setAVSyncTime(mAVSyncGap);
                    mProject.getClip(i, true).setClipIFamePlayMode(playmode);
                }
                mEngine.updateProject();
                progressBar.setMax(100);
                progressBar.setProgress(0);

                mFile = getExportFile(720,480,0);
                mExportMode = true;
				try {
					mEngine.export(mFile.getAbsolutePath(), 720, 480, 3 * 1024 * 1024, Long.MAX_VALUE, 0);
				}catch(ProjectNotAttachedException e){

					Toast.makeText(getApplicationContext(), "Fail:Project not attached", Toast.LENGTH_SHORT).show();
				}

            }
        });

        mButtonDirect = (Button) findViewById(R.id.button_clip_multi_trim_direct);
        mButtonDirect.setEnabled(false);
        mButtonDirect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                int startTime = 0;
                int endTime = 0;
                int applySpeedControl = 0;
                for (int i = 0; i < m_ListView_Start.getAdapter().getCount(); i++) {
                    if (mToggleButton.isChecked()) {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = mProject.getClip(0, true).getTotalTime() - Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    } else {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    }
                    int speed = Integer.parseInt(m_ListView_Speed.getAdapter().getItem(i).toString());
                    Log.d(TAG, "startTrimTime = " + startTime);
                    Log.d(TAG, "endTrimTime = " + endTime);
                    Log.d(TAG, "speed = " + speed);
                    if( speed != 100 )
                        applySpeedControl = 1;
                    mProject.getClip(i, true).getVideoClipEdit().setTrim(startTime, endTime);
                    mProject.getClip(i, true).getVideoClipEdit().setSpeedControl(speed);
                    //mProject.getClip(i, true).setClipPropertySlowVideoMode(true); // should test on only LGE version


                }
                mEngine.updateProject();
                mExportMode = true;
                ismergeable = mEngine.checkDirectExport(applySpeedControl);
                if (ismergeable == 8) {
                    Toast.makeText(getApplicationContext(), "Exist Overlay image", Toast.LENGTH_LONG).show();
                    mExportMode = false;
                }
            }
        });


        mButtonMix = (Button) findViewById(R.id.button_clip_multi_trim_mix);
        mButtonMix.setEnabled(false);
        mButtonMix.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                int startTime = 0;
                int endTime = 0;
                for (int i = 0; i < m_ListView_Start.getAdapter().getCount(); i++) {
                    if (mToggleButton.isChecked()) {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = mProject.getClip(0, true).getTotalTime() - Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    } else {
                        startTime = Integer.parseInt(m_ListView_Start.getAdapter().getItem(i).toString());
                        endTime = Integer.parseInt(m_ListView_End.getAdapter().getItem(i).toString());
                    }
                    Log.d(TAG, "startTrimTime = " + startTime);
                    Log.d(TAG, "endTrimTime = " + endTime);
                    mProject.getClip(i, true).getVideoClipEdit().setTrim(startTime, endTime);
                }
                mEngine.updateProject();

				try {
					mFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), 2);
					progressBar.setMax(100);
					progressBar.setProgress(0);
					mExportMode = true;
					if (mEngine.forceMixExport(mFile.getAbsolutePath()) == false) {
						Log.e(TAG, "forceMixExport error");
					}
				}catch(IndexOutOfBoundsException e){

					Toast.makeText(getApplicationContext(), "Fail:Project has not clip", Toast.LENGTH_SHORT).show();

				}
				catch(ProjectNotAttachedException e){

					Toast.makeText(getApplicationContext(), "Fail:Project not attached", Toast.LENGTH_SHORT).show();
				}
            }
        });

        m_editorView = (nexEngineView) findViewById(R.id.engineview_clip_multi_trim);

        mEditorListener = new nexEngineListener() {
            private int mLastTimeSec = 0;

            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {

                if (isUHDcontent) {

                    for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                        int end = mProject.getClip(i, true).getProjectEndTime();

                        if (end < mProject.getTotalTime()) {
                            if ((end - 66) < currentTime && (end > currentTime)) {
                                mEngine.stop();
                                mEngine.seek(end + 33);
                                mEngine.resume();
                            }
                        }
                    }
                }
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
                mExportMode = false;
                progressBar.setProgress(100);
                if(iserror){
                    Toast.makeText(getApplicationContext(), "Exporting fail:"+result, Toast.LENGTH_SHORT).show();
                }else {
                    Log.d(TAG, "Export Done!");
                    MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                }
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
                progressBar.setProgress(percent);
            }

            @Override
            public void onCheckDirectExport(int result) {
                Log.d(TAG, "onCheckDirectExport(" + result + ") ");
                if (result == 0) {
                    Toast.makeText(getApplicationContext(), "Check Direct Export Available", Toast.LENGTH_SHORT).show();
                    mFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), 1);
                    progressBar.setMax(100);
                    progressBar.setProgress(0);

                    mEngine.directExport(mFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime());
                } else {
                    mExportMode = false;
                    Toast.makeText(getApplicationContext(), "Check Direct Export Fail(" + result + ")", Toast.LENGTH_SHORT).show();
                    Log.d(TAG, "Check Direct Export Fail(" + result + ")");
                }
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

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        //m_editorView.s

        mEngine.setView(m_editorView);
        mEngine.setProject(mProject);
        mEngine.setEventHandler(mEditorListener);
        loadSCMB();

    }

    private int speedControlTab(int value) {
        int tab[] = {3,6,13, 25, 50, 75, 100, 125, 150, 175, 200, 400};
        int reval = 400;
        for (int i = 0; i < tab.length; i++) {
            if (tab[i] >= value) {
                reval = tab[i];
                break;
            }
        }
        return reval;
    }

    private void loadSCMB(){
        Log.d(TAG,"loadSCMB()");

        nexClip clip = new nexClip(m_listfilepath.get(0));
        isUHDcontent = clip.getWidth() == 3840 ? true : false;

        nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if( instance != null ){
            nxSCMBEntry[] entry = instance.getEntries();
            if( entry != null ){
                for(int i = 0; i < entry.length ; i++ ) {
                    mStartData.add(""+entry[i].getStartMs());
                    mEndData.add(""+entry[i].getEndMs());
                    mSpeedData.add("" + speedControlTab(entry[i].getSpeed()));
                    mProject.add(nexClip.dup(clip));
                    mProject.getLastPrimaryClip().setRotateDegree(clip.getRotateInMeta());
                    mProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    mProject.getLastPrimaryClip().getVideoClipEdit().setSpeedControl(entry[i].getSpeed());
                    mProject.getLastPrimaryClip().getVideoClipEdit().setTrim((int)entry[i].getStartMs(),(int)entry[i].getEndMs());
                }
                if( entry.length > 0 ) {
                    m_ListAdapter_Start.notifyDataSetChanged();
                    m_ListAdapter_End.notifyDataSetChanged();
                    m_ListAdapter_Speed.notifyDataSetChanged();

                    mButtonDirect.setEnabled(true);
                    mButtonPlay.setEnabled(true);
                    mButtonMix.setEnabled(true);
                    mButtonExport.setEnabled(true);
                    mButtonClear.setEnabled(true);
                }
            }
            instance.destroyInstance();
        }
    }

    private void clearSCMB(){
        nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if (instance != null) {
            instance.setEntries(null);
            instance.destroyInstance();
        }
    }

    private void saveSCMB(){

        if(mProject.getTotalClipCount(true) <= 0)
            return;

        boolean isLinerTrim = true;
        int time = mProject.getClip(0,true).getVideoClipEdit().getStartTrimTime();

        for(int i = 0 ; i < mProject.getTotalClipCount(true) ; i++ ){
            int st = mProject.getClip(i,true).getVideoClipEdit().getStartTrimTime();
            if( time == st ){
                time = mProject.getClip(i,true).getVideoClipEdit().getEndTrimTime();
            }else{
                isLinerTrim = false;
                break;
            }
        }

        if( isLinerTrim ) {
            nxSCMBEntry [] set = new nxSCMBEntry[mProject.getTotalClipCount(true)];

            for(int i = 0 ; i < mProject.getTotalClipCount(true) ; i++ ){
                nxSCMBEntry ent = new nxSCMBEntry(mProject.getClip(i,true).getVideoClipEdit().getSpeedControl()
                        ,mProject.getClip(i,true).getVideoClipEdit().getStartTrimTime()
                        ,mProject.getClip(i,true).getVideoClipEdit().getEndTrimTime()
                );
                set[i] = ent;
            }


            nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
            if (instance != null) {

                instance.setEntries(set);

                instance.destroyInstance();
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    static String getCurrentTime() {
        Time curTime = new Time();
        curTime.setToNow();
        curTime.switchTimezone(Time.getCurrentTimezone());
        return curTime.format("%Y%m%d_%H%M%S");
    }
    private File getExportFile(int wid, int hei, int mode ) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String export;
        if( mode == 1 ) {
            export = "Direct";
        }else if( mode == 2 ){
            export = "KineMix";
        }else{
            export = "Export";
        }
        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + export);
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NexEditor_"+wid+"X"+hei +"_"+ export_time+".mp4");
        return exportFile;
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mEngine.stop();
        mEngine.setProject(null);
        Log.d(TAG, "onDestroy");
        super.onDestroy();
    }
    @Override
    protected void onResume() {
        mEngine.setEventHandler(mEditorListener);
        super.onResume();
    }
}
