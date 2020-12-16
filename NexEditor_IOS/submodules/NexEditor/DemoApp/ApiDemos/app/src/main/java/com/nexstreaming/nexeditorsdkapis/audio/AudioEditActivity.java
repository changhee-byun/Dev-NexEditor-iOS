/******************************************************************************
 * File Name        : AudioEditActivity.java
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
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
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
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.util.ArrayList;

public class AudioEditActivity extends Activity {

    private static final String TAG = "AudioEdit";
    private nexEngine mEngine;
    private nexProject mProject;
    private nexClip mClip;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;
    private ListView m_ListView;
    private AudioEditItemAdapter mItemAdp;
    private SeekBar mSeekBarProgress;
    private SeekBar mSeekBarVolume;
    private TextView mTextview;
    private int mManualVolume = -1;

    private ArrayList<String> m_listfilepath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_audio_edit);

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        mClip = nexClip.getSupportedClip(m_listfilepath.get(0));

        if( mClip == null ) {
            finish();
            return;
        }

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mProject = new nexProject();
        mProject.add(mClip);

        m_ListView = (ListView) findViewById(R.id.listview_audio_edit);
        mItemAdp = new AudioEditItemAdapter(mProject);
        m_ListView.setAdapter(mItemAdp);
        mItemAdp.notifyDataSetChanged();

        m_ListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Toast.makeText(AudioEditActivity.this, "Seek(" + 0 + ") : " + position, Toast.LENGTH_SHORT).show();
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



        mSeekBarVolume = (SeekBar) findViewById(R.id.seekbar_audio_edit_volume);
        mSeekBarVolume.setMax(200);
        mSeekBarVolume.setProgress(100);
        mSeekBarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            private boolean isSeeking = false;
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (isSeeking) {
                    value = progress;
                    ((TextView) findViewById(R.id.textview_audio_edit_volume)).setText("" + value);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if (!isSeeking) {

                    isSeeking = true;

                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (isSeeking) {
                    isSeeking = false;
                    mProject.setProjectVolume(value);
                }
            }
        });

        mSeekBarProgress = (SeekBar) findViewById(R.id.seekbar_audio_edit_progress);
        mSeekBarProgress.setMax(mProject.getTotalTime() / 33);

        mSeekBarProgress.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
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
        mSeekBarProgress.setMax(0);
        mTextview = (TextView) findViewById(R.id.textview_audio_edit_time);

        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                if(mProject.getTotalClipCount(true)>0) {
                    mTextview.setText(String.valueOf(currentTime));
                    mSeekBarProgress.setProgress(currentTime / 33);
                }
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                if(mProject.getTotalClipCount(true)>0) {
                    mSeekBarProgress.setProgress(currentTime / 33);
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
                mSeekBarProgress.setMax(mProject.getTotalTime() / 33);
                mSeekBarProgress.setProgress(0);
                Log.d(TAG, "Project Duration" + mProject.getTotalTime());
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {
                Log.d(TAG, "onSeekStateChanged : isSeeking=" + isSeeking);
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


        Button btplay = (Button) findViewById(R.id.button_audio_edit_play);
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
                mSeekBarProgress.setMax(mEngine.getProject().getTotalTime() / 33);
                mEngine.play();
            }
        });

        Button btadd = (Button) findViewById(R.id.button_audio_edit_add);
        btadd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        Button btclear = (Button) findViewById(R.id.button_audio_edit_clear);
        btclear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mProject.allClear(false);
                mEngine.updateProject();
                if(m_editorView == null){
                    Toast.makeText(getApplicationContext(), "View is not ready. Please restart VoiceChanger", Toast.LENGTH_SHORT).show();
                    finish();
                }
                m_editorView.setBlackOut(true);
                mItemAdp.notifyDataSetChanged();
            }
        });

        Button btnSetAllManualVolume = (Button)findViewById(R.id.button_audio_edit_set_all_manual_volume);
        btnSetAllManualVolume.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final CharSequence menuManualVolume[] = new CharSequence[]{"enable", "disable"};

                AlertDialog.Builder builder = new AlertDialog.Builder(AudioEditActivity.this);
                builder.setTitle("change clip all manual volume");
                builder.setSingleChoiceItems(menuManualVolume, mManualVolume, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mProject.setManualVolumeControl(which == 0 ? 1 : 0);
                        mManualVolume = which;
                        dialog.dismiss();
                    }
                });

                builder.show();
            }
        });

        m_editorView = (nexEngineView) findViewById(R.id.engineview_audio_edit);
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
                mProject.add(clip);
                mItemAdp.notifyDataSetChanged();
                mSeekBarProgress.setMax(mProject.getTotalTime() / 33);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);

                    nexClip clip = nexClip.getSupportedClip(filepath);
                    if( clip != null ) {

                        mProject.add(new nexClip(filepath));
                        mItemAdp.notifyDataSetChanged();
                        mSeekBarProgress.setMax(mProject.getTotalTime() / 33);
                    }else{
                        Toast.makeText(AudioEditActivity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
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
