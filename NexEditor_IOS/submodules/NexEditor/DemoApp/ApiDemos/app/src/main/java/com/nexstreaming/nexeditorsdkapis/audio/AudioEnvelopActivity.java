/******************************************************************************
 * File Name        : AudioEnvelopActivity.java
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
import android.graphics.Color;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
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

import java.util.ArrayList;


public class AudioEnvelopActivity extends Activity implements nexEngineView.NexViewListener {
    private static final String TAG = "AudioEnvelopActivity";
    private ArrayList<String> m_soundfilepath;
    private nexEngine mEngine;
    private nexEngineListener mEditorListener = null;

    private ProgressBar progressBar_playtime;
    private TextView textView_playtime;

    private TextView bgmInfoTitle;
    private TextView bgmInfoArtist;
    private TextView bgmInfoDuration;
    private SeekBar seekBar_startTime;
    private TextView textView_startTime;

    private SeekBar seekBar_Trimtime;
    private TextView textView_Trimtime;

    private SeekBar seekBar_BGMVol;
    private TextView textView_BGMVol;

    private TextView startTime;
    private TextView trimTime;
    private TextView bgmVolume;
    private ListView env_list;
    private AudioEnvelopAdapter mItemAdp;
    private ArrayList<String> mTime = new ArrayList<String>();
    private ArrayList<String> mVol = new ArrayList<String>();
    private EditText envTime;
    private EditText envVol;
    private Button setEnv;

    private int mStartTime = 0;
    private int mBGMVol = 100;
    private int mTrimTime = 0;

    nexEngineView m_editorView;

    String title = null;
    String artist = null;
    String duration = null;

    private nexProject project = new nexProject();

    @Override
    public void onEngineViewAvailable(int width, int height) {

        Log.d(TAG,"onEngineViewAvailable ("+width+"X"+height+")");

    }

    @Override
    public void onEngineViewSizeChanged(int width, int height) {
        Log.d(TAG,"onEngineViewSizeChanged ("+width+"X"+height+")");
    }

    @Override
    public void onEngineViewDestroyed() {

        Log.d(TAG,"onEngineViewDestroyed ()");
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_audio_envelop);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        final Intent intent = getIntent();
        m_soundfilepath = intent.getStringArrayListExtra("filelist");
        String path;
        try {
            UtilityCode.raw2file(getApplicationContext(), R.mipmap.dessert, "desert.jpg");
        } catch (Exception e) {
            e.printStackTrace();
        }

//                path = getFilesDir().getAbsolutePath()+"/desert.jpg";
//                kmClip clip=kmClip.getSupportedClip(path);
        nexClip clip = nexClip.getSolidClip(Color.RED);
        nexClip clip2= nexClip.getSupportedClip(m_soundfilepath.get(0));
        if( clip2 != null ){
            if(clip2.getClipType()!=nexClip.kCLIP_TYPE_AUDIO){
                Toast.makeText(getApplicationContext(), "Video is not supported!", Toast.LENGTH_SHORT).show();
                finish();
            }
            clip.setImageClipDuration(clip2.getTotalTime());
            project.add(clip);
            project.addAudio(clip2, 0, clip2.getTotalTime());

//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));
//                    project.add(kmClip.dup(clip));//40 seconds
        }else{
            Toast.makeText(getApplicationContext(), "Audio not supported!", Toast.LENGTH_SHORT).show();
            finish();
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
                mEngine.stop();
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

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setProject(project);
        mEngine.setEventHandler(mEditorListener);


        textView_playtime = (TextView)findViewById(R.id.textview_audio_envelop_playtime);
        progressBar_playtime = (ProgressBar)findViewById(R.id.progressbar_audio_envelop_playtime);
        progressBar_playtime.setMax(project.getTotalTime() / 1000);
        progressBar_playtime.setProgress(0);


        bgmInfoTitle = (TextView) findViewById(R.id.textview_audio_envelop_bgminfo_title);
        bgmInfoArtist = (TextView) findViewById(R.id.textview_audio_envelop_bgminfo_artist);
        bgmInfoDuration = (TextView) findViewById(R.id.textview_audio_envelop_bgminfo_duration);

        startTime = (TextView) findViewById(R.id.textview_audio_envelop_starttime);
        trimTime = (TextView) findViewById(R.id.textview_audio_envelop_trimtime);
        bgmVolume = (TextView) findViewById(R.id.textview_audio_envelop_volume);

        env_list = (ListView)findViewById(R.id.list_audio_envelop);
        mItemAdp = new AudioEnvelopAdapter(AudioEnvelopActivity.this, mTime, mVol);
        env_list.setAdapter(mItemAdp);

        seekBar_startTime = (SeekBar)findViewById(R.id.seekbar_audio_envelop_bgm_starttime);
        textView_startTime = (TextView)findViewById(R.id.textview_audio_envelop_bgm_starttime);
        seekBar_startTime.setMax(project.getTotalTime());
        seekBar_startTime.setProgress(0);
        textView_startTime.setText("0");

        m_editorView = (nexEngineView)findViewById(R.id.engineview_media_play);
        m_editorView.setListener(this);
        mEngine.setView(m_editorView);
        seekBar_startTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_startTime.setText("" + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_startTime.setText("" + value);
                mStartTime = value;
                startTime.setText(String.valueOf(value));
            }
        });

        seekBar_Trimtime = (SeekBar)findViewById(R.id.seekbar_audio_envelop_trim_time);
        textView_Trimtime = (TextView)findViewById(R.id.textview_audio_envelop_trim_time);
        seekBar_Trimtime.setMax(project.getTotalTime());
        seekBar_Trimtime.setProgress(0);
        textView_Trimtime.setText("0");

        seekBar_Trimtime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 100;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_Trimtime.setText("" + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Trimtime.setText("" + value);
                mTrimTime = value;

                if( project.getClip(0, false).getTotalTime() - mTrimTime<=0 ){
                    mTrimTime = project.getClip(0, false).getTotalTime() - 1000;
                }
                trimTime.setText(String.valueOf(mTrimTime));
                textView_Trimtime.setText("" + mTrimTime);
            }
        });

        seekBar_BGMVol = (SeekBar)findViewById(R.id.seekbar_audio_envelop_bgm_vol);
        textView_BGMVol = (TextView)findViewById(R.id.textview_audio_envelop_bgm_vol);
        seekBar_BGMVol.setMax(200);
        seekBar_BGMVol.setProgress(100);
        textView_BGMVol.setText("" + 100);
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
                bgmVolume.setText(String.valueOf(value));
            }
        });
        envTime = (EditText)findViewById(R.id.edittext_audio_envelop_env_time);
        envVol = (EditText)findViewById(R.id.edittext_audio_envelop_env_vol);
        setEnv = (Button)findViewById(R.id.button_audio_envelop_set);
        setEnv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(envTime.getText().length()==0 || envVol.getText().length()==0){
                    Toast.makeText(getApplicationContext(), "input time/vol value", Toast.LENGTH_SHORT).show();
                    return;
                }
                if(Integer.parseInt(envTime.getText().toString()) < 0 || Integer.parseInt(envTime.getText().toString()) > project.getClip(0, false).getTotalTime()){
                    Toast.makeText(getApplicationContext(), "Input Valid Time Range 0~"+project.getClip(0, false).getTotalTime() , Toast.LENGTH_SHORT).show();
                    return;
                }
                if(Integer.parseInt(envVol.getText().toString()) < 0 || Integer.parseInt(envVol.getText().toString()) > 200){
                    Toast.makeText(getApplicationContext(), "Input Valid Vol Range 0~200", Toast.LENGTH_SHORT).show();
                    return;
                }
                mTime.add(envTime.getText().toString());
                mVol.add(envVol.getText().toString());
                mItemAdp.notifyDataSetChanged();
            }
        });

        Button bt = (Button)findViewById(R.id.button_audio_envelop_bgm_play);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                project.changeAudio(project.getAudioItem(0), mStartTime, mStartTime+project.getClip(0, false).getTotalTime());
                project.getAudioItem(0).setTrim(mTrimTime, project.getClip(0, false).getTotalTime());
                project.getClip(0, false).setClipVolume(mBGMVol);
                for(int i=0; i<mTime.size(); i++){
                    project.getClip(0,false).getAudioEnvelop().addVolumeEnvelope(Integer.parseInt(mTime.get(i)),Integer.parseInt(mVol.get(i)));
                }
                if(mTrimTime != 0){
                    project.getClip(0,false).getAudioEnvelop().addVolumeEnvelope(mTrimTime, 100);
                }
                project.setBackgroundConfig(0, false, false);
                project.getClip(0,true).setImageClipDuration(project.getClip(0, false).getTotalTime() + mStartTime - mTrimTime);
                progressBar_playtime.setMax((project.getClip(0,false).getTotalTime() +mStartTime -mTrimTime)/1000);
                mEngine.play();
            }
        });
        getContentInfo(m_soundfilepath.get(0));
    }


    protected void getContentInfo(String path) {
            MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
            metadataRetriever.setDataSource(path);

            title = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
            artist = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
            //duration = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            nexClip clip = nexClip.getSupportedClip(path);
            duration = String.valueOf(clip.getAudioDuration());

            if (title == null) {
                title = "Unknown";
            }
            if (artist == null) {
                artist = "Unknown";
            }

            bgmInfoTitle.setText(title);
            bgmInfoArtist.setText(artist);
            int minute = Integer.parseInt(duration) / 60000;
            int second = (Integer.parseInt(duration) % 60000) / 1000;
            bgmInfoDuration.setText("Play time\r\n" + minute + ":" + second);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
