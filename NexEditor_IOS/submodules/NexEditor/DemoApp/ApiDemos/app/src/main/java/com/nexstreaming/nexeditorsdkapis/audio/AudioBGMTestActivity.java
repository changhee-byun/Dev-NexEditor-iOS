/******************************************************************************
 * File Name        : AudioBGMTestActivity.java
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
import android.content.ClipData;
import android.content.Intent;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.util.ArrayList;

public class AudioBGMTestActivity extends Activity {
    private static final String TAG = "AudioBGMTest";
    private String m_soundfilepath;
    private nexEngine mEngine = null;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;

    private ProgressBar progressBar_playtime;
    private TextView textView_playtime;

    private TextView bgmInfoTitle;
    private TextView bgmInfoArtist;
    private TextView bgmInfoDuration;
    private Button bgmButton;
    private Button exportButton;
    private Button intentButton;
    private SeekBar seekBar_startTime;
    private TextView textView_startTime;

    private SeekBar seekBar_BGMLevel;
    private TextView textView_BGMLevel;

    private SeekBar seekBar_BGMVol;
    private TextView textView_BGMVol;

    private EditText fadeIn;
    private EditText fadeOut;
    private Switch loop;
    private Switch trim;
    private ArrayList<String> m_listfilepath;
    private int mStartTime = 0;
    private int mBGMVol = 100;
    private int mBGMLevel = 100;
    private int mBGMDuration;
    private nexProject project = new nexProject();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_audio_bgm_test);


        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        if(m_listfilepath.size() == 0) {
            Log.d(TAG, "m_listfilepath size is 0");
            return;
        }

        nexClip clip=nexClip.getSupportedClip(m_listfilepath.get(0));
        if( clip != null ){

            clip.setRotateDegree(clip.getRotateInMeta());
            project.add(clip);
        }

        for( int i = 0 ; i < project.getTotalClipCount(true) ; i++ ){
            project.getClip(i, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        }
        mEditorListener = new nexEngineListener(){
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

        m_editorView = (nexEngineView)findViewById(R.id.engineview_audio_bgmtest);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.setEventHandler(mEditorListener);

        textView_playtime = (TextView)findViewById(R.id.textView_audio_bgmtest_playtime);
        progressBar_playtime = (ProgressBar)findViewById(R.id.progressbar_audio_bgmtest_playtime);
        progressBar_playtime.setMax(project.getTotalTime() / 1000);
        progressBar_playtime.setProgress(0);


        bgmInfoTitle = (TextView) findViewById(R.id.textview_audio_bgmtest_info_title);
        bgmInfoArtist = (TextView) findViewById(R.id.textview_audio_bgmtest_info_artist);
        bgmInfoDuration = (TextView) findViewById(R.id.textview_audio_bgmtest_info_duration);

        bgmButton = (Button) findViewById(R.id.button_audio_bgmtest_change);
        bgmButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        intentButton = (Button) findViewById(R.id.button_audio_bgmtest_intent);
        intentButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                //startActivity(project.makeKineMasterIntent());
                UtilityCode.launchKineMaster(AudioBGMTestActivity.this, m_listfilepath);
            }
        });

        exportButton = (Button)findViewById(R.id.button_audio_bgmtest_export);
        exportButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(fadeIn.getText().length() !=0 ) {
                    project.setProjectAudioFadeInTime(Integer.parseInt(fadeIn.getText().toString()));
                }else {
                    project.setProjectAudioFadeInTime(200);
                }

                if(fadeOut.getText().length() !=0 ) {
                    project.setProjectAudioFadeOutTime(Integer.parseInt(fadeOut.getText().toString()));
                }else{
                    project.setProjectAudioFadeOutTime(5000);
                }
                if(m_soundfilepath != null) {

                    if( trim.isChecked() ) {
                        if( mBGMDuration > 10000 ) {
                            project.setBackgroundTrim(4000,9000);
                        }else{
                            project.setBackgroundTrim(0,0);
                        }
                    }else{
                        project.setBackgroundTrim(0,0);
                    }
                    project.setBackgroundConfig(mStartTime, false, loop.isChecked());
                    project.setBackgroundMusicPath(m_soundfilepath);
                }else {
                    if( trim.isChecked() ) {
                        project.setBackgroundTrim(4000,9000);
                    }else{
                        project.setBackgroundTrim(0,0);
                    }
                    project.setBackgroundConfig(mStartTime, true, loop.isChecked());
                }
                //for( int i = 0 ; i < project.getTotalClipCount(true) ; i++) {
                project.getClip(0, true).setClipVolume(mBGMLevel);
                //}
                project.setBGMMasterVolumeScale(mBGMVol / 200f);
                project.updateProject();

                mEngine.stop();
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });
        seekBar_startTime = (SeekBar)findViewById(R.id.seekbar_audio_bgmtest_starttime);
        textView_startTime = (TextView)findViewById(R.id.textView_audio_bgmtext_starttime);
        seekBar_startTime.setMax(project.getTotalTime() / 1000);
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

        seekBar_BGMLevel = (SeekBar)findViewById(R.id.seekbar_audio_bgmtest_clip_bgm_lv);
        textView_BGMLevel = (TextView)findViewById(R.id.textView_audio_bgmtest_clip_bgm_lv);
        seekBar_BGMLevel.setMax(200);
        seekBar_BGMLevel.setProgress(100);
        textView_BGMLevel.setText("" + 100);
        mBGMLevel = 100;

        seekBar_BGMLevel.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 100;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_BGMLevel.setText("" + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_BGMLevel.setText("" + value);
                mBGMLevel = value;
            }
        });

        seekBar_BGMVol = (SeekBar)findViewById(R.id.seekbar_audio_bgmtest_vol);
        textView_BGMVol = (TextView)findViewById(R.id.textview_audio_bgmtest_vol);
        seekBar_BGMVol.setMax(200);
        seekBar_BGMVol.setProgress(100);
        textView_BGMVol.setText(""+100);
        mBGMVol = 100;
        seekBar_BGMVol.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 100;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_BGMVol.setText("" + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_BGMVol.setText("" + value);
                mBGMVol = value;
            }
        });
        fadeIn = (EditText)findViewById(R.id.edittext_audio_bgmtest_fadein);
        fadeOut = (EditText)findViewById(R.id.edittext_audio_bgmtest_fadeout);

        loop = (Switch)findViewById(R.id.switch_audio_bgmtest_loop);
        loop.setChecked(true);

        trim = (Switch)findViewById(R.id.switch_audio_bgmtest_trim);
        trim.setChecked(false);

        Button bt = (Button)findViewById(R.id.button_audio_bgmtest_play);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                if(fadeIn.getText().length() !=0 ) {
                    project.setProjectAudioFadeInTime(Integer.parseInt(fadeIn.getText().toString()));
                }else {
                    project.setProjectAudioFadeInTime(200);
                }

                if(fadeOut.getText().length() !=0 ) {
                    project.setProjectAudioFadeOutTime(Integer.parseInt(fadeOut.getText().toString()));
                }else{
                    project.setProjectAudioFadeOutTime(5000);
                }

                if(m_soundfilepath != null) {

                    if( trim.isChecked() ) {
                        if( mBGMDuration > 10000 ) {
                            project.setBackgroundTrim(4000,9000);
                        }else{
                            project.setBackgroundTrim(0,0);
                        }
                    }else{
                        project.setBackgroundTrim(0,0);
                    }
                    project.setBackgroundConfig(mStartTime, false, loop.isChecked());
                    project.setBackgroundMusicPath(m_soundfilepath);
                }else {
                    if( trim.isChecked() ) {
                        project.setBackgroundTrim(4000,9000);
                    }else{
                        project.setBackgroundTrim(0,0);
                    }
                    project.setBackgroundConfig(mStartTime, true, loop.isChecked());
                }

                project.getClip(0, true).setClipVolume(mBGMLevel);

                project.setBGMMasterVolumeScale(mBGMVol / 200f);
                project.updateProject();

                mEngine.play();
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String title = null;
            String artist = null;
            int duration = 0;

            if (clipData == null) {
                m_soundfilepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                item = clipData.getItemAt(0);
                uri = item.getUri();
                m_soundfilepath = ConvertFilepathUtil.getPath(this, uri);
            }
            nexClip clip = nexClip.getSupportedClip(m_soundfilepath);
            if( clip == null ){
                Toast.makeText(getApplicationContext(),"Audio not supported!", Toast.LENGTH_SHORT).show();
            }
            else {
                MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
                metadataRetriever.setDataSource(m_soundfilepath);

                title = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
                artist = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
                duration = clip.getAudioDuration();

                if (title == null) {
                    title = "Unknown";
                }
                if (artist == null) {
                    artist = "Unknown";
                }

                mBGMDuration = duration;
                bgmInfoTitle.setText(title);
                bgmInfoArtist.setText(artist);
                int minute = duration / 60000;
                int second = (duration % 60000) / 1000;
                bgmInfoDuration.setText("Play time\r\n" + minute + ":" + second);
            }
        }
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart");

        if(mEngine != null) {
            mEngine.setEventHandler(mEditorListener);
        }
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
