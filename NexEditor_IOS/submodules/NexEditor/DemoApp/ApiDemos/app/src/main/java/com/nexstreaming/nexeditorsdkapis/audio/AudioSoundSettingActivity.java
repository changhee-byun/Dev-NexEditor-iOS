/******************************************************************************
 * File Name        : AudioSoundSettingActivity.java
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
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;


public class AudioSoundSettingActivity extends Activity {
    private static final String TAG = "AudioSoundSetting";
    private String m_soundfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;

    private ProgressBar progressBar_playtime;
    private TextView textView_playtime;

    private TextView bgmInfo;
    private TextView soundInfoTitle;
    private TextView soundInfoArtist;
    private TextView soundInfoDuration;
    private Button soundButton;
    private SeekBar seekBar_startTime;
    private TextView textView_startTime;

    private SeekBar seekBar_endTime;
    private TextView textView_endTime;

    private int mStartTime = 0;
    private int mEndTime = 0;
    private nexProject project = new nexProject();
    private static final int kClipDuration = 10000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_audio_sound_setting);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        String path;
        try {
            UtilityCode.raw2file(getApplicationContext(), R.mipmap.dessert, "desert.jpg");
        } catch (Exception e) {
            e.printStackTrace();
        }
        path = getFilesDir().getAbsolutePath()+"/desert.jpg";
        nexClip clip=nexClip.getSupportedClip(path);
        if( clip != null ){
            project.add(clip);
            //project.setThemeId("com.nexstreaming.kinemaster.travel");
            project.getLastPrimaryClip().setImageClipDuration(kClipDuration);
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

        m_editorView = (nexEngineView)findViewById(R.id.engineview_audio_sound_setting_editorview);
        m_editorView.setBlackOut(true);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.setEventHandler(mEditorListener);

        textView_playtime = (TextView)findViewById(R.id.textview_audio_sound_setting_playtime);
        progressBar_playtime = (ProgressBar)findViewById(R.id.progressbar_audio_sound_setting_playtime);
        progressBar_playtime.setMax(kClipDuration / 1000);
        progressBar_playtime.setProgress(0);

        bgmInfo = (TextView) findViewById(R.id.textview_audio_sound_setting_bgminfo);
        bgmInfo.setText("Travel Theme BGM");

        soundInfoTitle = (TextView) findViewById(R.id.textview_audio_sound_setting_soundinfo_title);
        soundInfoArtist = (TextView) findViewById(R.id.textview_audio_sound_setting_soundinfo_artist);
        soundInfoDuration = (TextView) findViewById(R.id.textview_audio_sound_setting_soundinfo_duration);

        soundButton = (Button) findViewById(R.id.button_audio_sound_setting_sound);
        soundButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        seekBar_startTime = (SeekBar)findViewById(R.id.seekbar_audio_sound_setting_starttime);
        textView_startTime = (TextView)findViewById(R.id.textview_audio_sound_setting_starttime);
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

        seekBar_endTime = (SeekBar)findViewById(R.id.seekbar_audio_sound_setting_endtime);
        textView_endTime = (TextView)findViewById(R.id.textview_audio_sound_setting_endtime);
        seekBar_endTime.setMax(kClipDuration/1000);
        seekBar_endTime.setProgress(kClipDuration/1000);
        textView_endTime.setText(""+kClipDuration/1000);
        mEndTime = kClipDuration;
        seekBar_endTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = kClipDuration / 1000;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_endTime.setText("" + progress);
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

        Button bt = (Button)findViewById(R.id.button_audio_sound_setting_play);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                if(project.getAudioItems().size() != 0)
                    if(mEndTime - mStartTime <=0 ) {
                        Toast.makeText(AudioSoundSettingActivity.this, "End time low less than start time ", Toast.LENGTH_LONG).show();
                        project.changeAudio(project.getAudioItem(0), 0, mEndTime);
                        seekBar_startTime.setProgress(0);
                        textView_startTime.setText("" + 0);
                    }
                    else project.changeAudio(project.getAudioItem(0), mStartTime, mEndTime);
                mEngine.play();
            }
        });

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String title = null;
            String artist = null;
            int duration = 0;

            if(clipData == null) {
                m_soundfilepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                item = clipData.getItemAt(0);
                uri = item.getUri();
                m_soundfilepath = ConvertFilepathUtil.getPath(this, uri);
            }
            MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
            metadataRetriever.setDataSource(m_soundfilepath);

            title = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
            artist = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);

            nexClip clip = nexClip.getSupportedClip(m_soundfilepath);
            if( clip == null ){
                Toast.makeText(getApplicationContext(),"Audio not supported!", Toast.LENGTH_SHORT).show();
            }
            else {
                duration = clip.getAudioDuration();
                if(project.getTotalClipCount(false) !=0) {
                    project.getClip(0, false).replaceClip(m_soundfilepath);
                }else {
                    if(mEndTime - mStartTime <=0 ) {
                        mStartTime = 0;
                        seekBar_startTime.setProgress(0);
                        textView_startTime.setText("" + 0);
                    }
                    project.addAudio(clip, mStartTime, mEndTime);
                }
                project.updateProject();

                if (title != null) {
                    soundInfoTitle.setText(title);
                } else {
                    soundInfoTitle.setText("Unknown");
                }
                if (artist != null) {
                    soundInfoArtist.setText(artist);
                } else {
                    soundInfoArtist.setText("Unknown");
                }
                int minute = duration / 60000;
                int second = (duration % 60000) / 1000;
                soundInfoDuration.setText("Play time\r\n" + minute + ":" + second);
            }
        }

    }

    @Override
    protected void onDestroy() {
        mEngine.stop();
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        mEngine.stop();
        super.onPause();
    }
}
