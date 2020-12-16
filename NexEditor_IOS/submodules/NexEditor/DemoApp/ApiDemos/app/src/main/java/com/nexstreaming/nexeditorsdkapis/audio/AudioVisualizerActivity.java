/******************************************************************************
 * File Name        : AudioVisualizerActivity.java
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

import android.media.AudioManager;
import android.media.MediaMetadataRetriever;
import android.media.audiofx.Equalizer;
import android.media.audiofx.Visualizer;
import android.net.Uri;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.common.VisualizerView;

import java.util.ArrayList;

public class AudioVisualizerActivity extends Activity implements nexEngineListener {
    private static final String TAG = "AudioVisualizer";

    private nexEngine mEngine;
    private ArrayList<String> m_listfilepath;
    private nexEngineView m_editorView;
    private SeekBar mSeekBarPlayTime;
    private Button mPlayStop;
    private TextView mPlayTime;
    private TextView mEndTime;


    private SeekBar mSeekBarClipVol;
    private TextView mClipVol;
    private int mClipVolume;

    private SeekBar mSeekBarBGMVol;
    private TextView mBGMVol;
    private int mBGMVolume;

    private TextView mTitle;
    private TextView mArtist;
    private TextView mDuration;
    private Button mAddBGM;

    private int mCurrentPlayTime;
    private boolean trackingSeekbarTouch = false;
    private int trackingSeektime = -1;
    private boolean seekInProgress = false;
    private boolean lastUIisPlaying = false;
    private int playTime = 0;

    private Visualizer mVisualizer;
    private VisualizerView mVisualizerView;
    private Equalizer mEqualizer;

    nexProject project;
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
//        setContentView(R.layout.activity_audio_visualizer);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        if( savedInstanceState!=null ) {
            mCurrentPlayTime = savedInstanceState.getInt("currentTime");
            lastUIisPlaying = savedInstanceState.getBoolean("requestedPlay");
        }


        setContentView(R.layout.activity_audio_visualizer);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        project = new nexProject();

        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if (clip != null) {
            clip.setRotateDegree(clip.getRotateInMeta());
            project.add(clip);
            project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        } else {
            Toast.makeText(getApplicationContext(), "Not supported clip : " + m_listfilepath.get(0), Toast.LENGTH_SHORT).show();
        }

        if (project.getTotalClipCount(true) == 0) {
            Toast.makeText(getApplicationContext(), "No Clips.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        m_editorView = (nexEngineView) findViewById(R.id.engineview_audio_visualizer);
        m_editorView.setBlackOut(true);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setEventHandler(AudioVisualizerActivity.this);
        mEngine.setProject(project);
        mEngine.updateProject();

        mSeekBarPlayTime = (SeekBar) findViewById(R.id.seekbar_audio_visualizer);
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

                            /*
                            if (mState == State.STOPED) {
                                playTime = i * 33;
                                Log.d(TAG, "seekTs:" + mSeekCurrentTS + " progress:" + i + "/" + Math.abs(mSeekCurrentTS - playTime));
                                if (Math.abs(mSeekCurrentTS - playTime) > 100) {
                                    mSeekMode = 2;
                                } else {
                                    mSeekMode = 1;
                                }
                                trackingSeektime = playTime;
                                // seek(playTime);
                            }
                            */
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                trackingSeekbarTouch = true;
                stop();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                trackingSeekbarTouch = false;
            }
        });

        mVisualizerView = (VisualizerView)findViewById(R.id.visualizerview_audio_visualizer);

        mPlayStop = (Button) findViewById(R.id.button_audio_visualizer_stop);
        mPlayStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mState == State.PLAYING) {
                    stop();
                    mPlayStop.setText("Play");
                    lastUIisPlaying = false;
                } else {
                    project.setBGMMasterVolumeScale(mBGMVolume / 200f);
                    project.getClip(0, true).setClipVolume(mClipVolume);
                    mEngine.updateProject();
                    resume();
                    mPlayStop.setText("Stop");
                    lastUIisPlaying = true;
                }
            }
        });


        mPlayTime = (TextView) findViewById(R.id.textview_audio_visualizer_time);

        mEndTime = (TextView) findViewById(R.id.textview_audio_visualizer_total);
        mEndTime.setText(getTimeString(project.getTotalTime()));

        mClipVol = (TextView) findViewById(R.id.textview_audio_visualizer_clipval);
        mClipVol.setText("100");
        mClipVolume = 100;

        mSeekBarClipVol = (SeekBar) findViewById(R.id.seekbar_audio_visualizer_clipvol);
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

        mBGMVol = (TextView) findViewById(R.id.textview_audio_visualizer_bgmval);
        mBGMVol.setText("100");
        mBGMVolume = 100;

        mSeekBarBGMVol = (SeekBar) findViewById(R.id.seekbar_audio_visualizer_bgmvol);
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

        mTitle = (TextView) findViewById(R.id.textview_audio_visualizer_title);
        mArtist = (TextView) findViewById(R.id.textview_audio_visualizer_artist);
        mDuration = (TextView) findViewById(R.id.textview_audio_visualizer_duration);

        mAddBGM = (Button) findViewById(R.id.bt_audio_visualizer_addbgm);
        mAddBGM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });
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

    @Override
    public void onStateChange(int oldState, int newState) {
        Log.i(TAG, "onStateChange(): oldState=" + oldState + ", newState=" + newState );
        if( newState == 1 ) {

            mState = State.STOPED;
            if( trackingSeektime != - 1) {
                seek(trackingSeektime);
                trackingSeektime = -1;
            }

            // for KMSA 353
            mClipVol.setText("100");
            mClipVolume = 100;
            mSeekBarClipVol.setProgress(100);

            mBGMVol.setText("100");
            mBGMVolume = 100;
            mSeekBarBGMVol.setProgress(100);
            mPlayStop.setText("Play");
        }else if( newState == 2 ){
            mState = State.PLAYING;
            mPlayStop.setText("Stop");
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

        Log.i(TAG, String.format("onSetTimeDone(): currentTime=%d, seekInProgress=%b lastUIisPlaying=%b", currentTime, seekInProgress, lastUIisPlaying));

        mState = State.STOPED;

        if( trackingSeektime != - 1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if( lastUIisPlaying ){
            resume();
        }
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

        if( lastUIisPlaying ){
            resume();
        }
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

        if( lastUIisPlaying ){
            resume();
        }
    }

    @Override
    public void onEncodingDone(boolean b, int i) {

    }

    @Override
    public void onPlayEnd() {
        mPlayStop.setText("Play");
        lastUIisPlaying = false;
        mEngine.seek(0);

        mClipVolume = 100;
        mClipVol.setText("100");
        mSeekBarClipVol.setProgress(100);

        mBGMVolume = 100;
        mBGMVol.setText("100");
        mSeekBarBGMVol.setProgress(100);

        if( mVisualizer != null ){
            mVisualizer.setEnabled(false);
            mVisualizer.release();
            mVisualizer = null;
        }
    }

    @Override
    public void onPlayFail(int i, int i1) {
        if( mVisualizer != null ){
            mVisualizer.setEnabled(false);
            mVisualizer.release();
            mVisualizer = null;
        }

        if(mEqualizer != null){
            mEqualizer.setEnabled(false);
            mEqualizer.release();
            mEqualizer = null;
        }

    }

    @Override
    public void onPlayStart() {
        int id = mEngine.getAudioSessionID();
        Log.d(TAG,"onPlayStart() getAudioSessionID="+id);
        if(mVisualizer != null){
            mVisualizer.setEnabled(false);
            mVisualizer.release();
        }

        if(mEqualizer != null){
            mEqualizer.setEnabled(false);
            mEqualizer.release();
        }

        mVisualizer = new Visualizer(id);
        mEqualizer = new Equalizer(0,id);
        mEqualizer.setEnabled(true);
        short totalEQ = mEqualizer.getNumberOfPresets();
        if( totalEQ > 0 ) {
            Log.d(TAG,"EQ Preset Name="+mEqualizer.getPresetName((short) (totalEQ - 1)));
            mEqualizer.usePreset((short) (totalEQ - 1));
        }

        mVisualizer.setDataCaptureListener(new Visualizer.OnDataCaptureListener() {
            @Override
            public void onWaveFormDataCapture(Visualizer visualizer, byte[] waveform, int samplingRate) {
                int energy = computeEnergy(waveform,true);
                int time = mEngine.getCurrentPlayTimeTime();
                Log.d(TAG, "[" + time + "]waveform.energy=" + energy + ", samplingRate=" + samplingRate);

                mVisualizerView.updateVisualizer(waveform);
            }

            @Override
            public void onFftDataCapture(Visualizer visualizer, byte[] fft, int samplingRate) {

            }
        }, Visualizer.getMaxCaptureRate()/2,true,false);
        mVisualizer.setEnabled(true);
    }

    @Override
    public void onClipInfoDone() {

    }

    @Override
    public void onSeekStateChanged(boolean isSeeking) {
        Log.i(TAG, "onSeekStateChanged() isSeeking=" + isSeeking);
        seekInProgress = isSeeking;
    }

    @Override
    public void onEncodingProgress(int i) {

    }

    @Override
    public void onCheckDirectExport(int i) {

    }

    @Override
    public void onProgressThumbnailCaching(int i, int i1) {

    }

    @Override
    public void onFastPreviewStartDone(int i, int i1, int i2) {

    }

    @Override
    public void onFastPreviewStopDone(int i) {

    }

    @Override
    public void onFastPreviewTimeDone(int i) {

    }

    @Override
    public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

    }

    private void startPlay(){
        if( mState == State.STOPED  ){
            mState = State.PLAY_READY;
            mEngine.play();
        }else{
            Log.w(TAG,"startPlay() invaild state="+mState );
        }
    }

    private void resume(){
        if( mState == State.STOPED || mState == State.IDLE ) {
            mEngine.resume();
        }else{
            Log.w(TAG,"startPlay() invaild state="+mState );
        }
    }

    private void stop(){
        if( mState == State.PLAYING || mState == State.PLAY_READY ){
            mState = State.STOP_READY;
            mEngine.stop();
        }else{
            Log.w(TAG,"stop() invaild state="+mState );
        }
    }

    private int mSeekMode = 1; /*1: Seek, 2: Seek IDR*/
    private int mSeekCurrentTS;

    private void seek(int ms){
        mState = State.SEEKING;
        mSeekCurrentTS = ms;
        if(mSeekMode == 1) {
            mEngine.seek(ms);
        } else {
            mEngine.seekIDRorI(ms);
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

    private int computeEnergy(byte[] data, boolean pcm) {
        int energy = 0;
        if (data.length != 0) {
            if (pcm) {
                for (int i = 0; i < data.length; i++) {
                    int tmp = ((int)data[i] & 0xFF) - 128;
                    energy += tmp*tmp;
                }
            } else {
                energy = (int)data[0] * (int)data[0];
                for (int i = 2; i < data.length; i += 2) {
                    int real = (int)data[i];
                    int img = (int)data[i + 1];
                    energy += real * real + img * img;
                }
            }
        }
        return energy;
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
        mEngine.stop();
        super.onPause();
    }

    @Override
    protected void onResume() {
        if( lastUIisPlaying ) {
            /*
            if( mCurrentPlayTime > 0 ){
                mEngine.seek(mCurrentPlayTime);
            }
            */
            mEngine.resume();
            mPlayStop.setText("Stop");
        }
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        nexClip.cancelThumbnails();
        mEngine.stop();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
