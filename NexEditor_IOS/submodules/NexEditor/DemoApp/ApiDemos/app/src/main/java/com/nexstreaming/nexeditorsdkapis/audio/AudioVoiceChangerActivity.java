/******************************************************************************
 * File Name        : AudioVoiceChangerActivity.java
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
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;


public class AudioVoiceChangerActivity extends Activity {
    private static final String TAG = "AudioVoiceChanger";
    private nexEngine mEngine;
    private nexProject mProject;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;
    private ListView m_ListView;
    private AudioVoiceChangerItemAdapter mItemAdp;
    private SeekBar seekBar;
    private TextView mTextview;
    private int index;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_audio_voice_changer);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mProject = new nexProject();

        m_ListView = (ListView) findViewById(R.id.listview_audio_voice_changer);
        mItemAdp = new AudioVoiceChangerItemAdapter(mProject, true);
        m_ListView.setAdapter(mItemAdp);

        m_ListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Toast.makeText(AudioVoiceChangerActivity.this, "Seek(" + 0 + ") : " + position, Toast.LENGTH_SHORT).show();
                if (mEngine != null && mProject != null) {
                    if (mProject.getTotalClipCount(true) > 0) {
//                                mEngine.updateProject();
//                                int seekTime = mProject.getClip(position, true).getProjectStartTime() + 3000;
//                                Toast.makeText(ProjectEditActivity.this, "Seek(" + seekTime + ") : "+position , Toast.LENGTH_SHORT).show();
//                                mEngine.seek(seekTime);
                    }
                }
            }
        });

        seekBar = (SeekBar) findViewById(R.id.seekBar);

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            private int seek;
            private boolean isSeeking = false;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (isSeeking) {
                    seek = progress * 33;
                    mTextview.setText(String.valueOf(progress * 33));
                    mEngine.seek(seek);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if (!isSeeking) {
                    mEngine.pause();
                    isSeeking = true;
                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (isSeeking) {
                    isSeeking = false;
                }
            }
        });
        seekBar.setMax(0);
        mTextview = (TextView) findViewById(R.id.textview_audio_voice_changer_time);

        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                if(mProject.getTotalClipCount(true)>0) {
                    mTextview.setText(String.valueOf(currentTime));
                    seekBar.setProgress(currentTime / 33);
                }
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                if(mProject.getTotalClipCount(true)>0) {
                    seekBar.setProgress(currentTime / 33);
                    mTextview.setText(String.valueOf(currentTime));
                }
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
                Log.d(TAG, "onPlayFail : err=" + err + " iClipID=" + iClipID);
            }

            @Override
            public void onPlayStart() {
                seekBar.setMax(mProject.getTotalTime() / 33);
                seekBar.setProgress(0);
                Log.d(TAG, "Project Duration" + mProject.getTotalTime());
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {
                Log.d(TAG, "onSeekStateChanged : isSeeking=" + isSeeking);
                /*
                if( isSeeking )
                    mEngine.pause();
                else
                    mEngine.resume();
                    */
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


        Button btplay = (Button) findViewById(R.id.button_audio_voice_changer_play);
        btplay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mProject.getTotalClipCount(true) == 0) {
                    Toast.makeText(getApplicationContext(), "clip count is 0", Toast.LENGTH_SHORT).show();
                    return;
                }
                mEngine.stop();
                if (m_editorView == null) {
                    Toast.makeText(getApplicationContext(), "View is not ready. Please restart VoiceChanger", Toast.LENGTH_SHORT).show();
                    finish();
                }
                mEngine.setView(m_editorView);
                seekBar.setMax(mEngine.getProject().getTotalTime() / 33);
                mEngine.play();
            }
        });

        Button btadd = (Button) findViewById(R.id.button_audio_voice_changer_add);
        btadd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        Button btclear = (Button) findViewById(R.id.button_audio_voice_changer_clear);
        btclear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mProject.allClear(true);
                mEngine.updateProject();
                if(m_editorView == null){
                    Toast.makeText(getApplicationContext(), "View is not ready. Please restart VoiceChanger", Toast.LENGTH_SHORT).show();
                    finish();
                }
                m_editorView.setBlackOut(true);
                mItemAdp.notifyDataSetChanged();
            }
        });
        m_editorView = (nexEngineView) findViewById(R.id.engineview_audio_voice_changer);
        m_editorView.setBlackOut(true);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setEventHandler(mEditorListener);
        mEngine.setProject(mProject);

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                nexClip clip = new nexClip(filepath);
                if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                    clip.setRotateDegree(clip.getRotateInMeta());
                }
                mProject.add(clip);
                mItemAdp.notifyDataSetChanged();
                seekBar.setMax(mProject.getTotalTime() / 33);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);

                    nexClip clip = nexClip.getSupportedClip(filepath);
                    if( clip != null ) {
                        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                            clip.setRotateDegree(clip.getRotateInMeta());
                        }

                        mProject.add(new nexClip(filepath));
                        mItemAdp.notifyDataSetChanged();
                        seekBar.setMax(mProject.getTotalTime() / 33);
                    }else{
                        Toast.makeText(AudioVoiceChangerActivity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                    }
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

}
