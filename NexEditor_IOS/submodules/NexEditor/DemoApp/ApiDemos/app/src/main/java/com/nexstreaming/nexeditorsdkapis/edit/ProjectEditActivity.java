/******************************************************************************
 * File Name        : ProjectEditActivity.java
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
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayFilter;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.effect.EffectPreviewActivity;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;


public class ProjectEditActivity extends Activity {
    private static final String TAG = "ProjectEditActivity";
    private nexEngine mEngine;
    private nexProject mProject;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;
    private ListView m_ListView;
    private ProjectEditItemAdapter mItemAdp;
    private SeekBar seekBar;
    private TextView mTextview;
    private int index;
    private boolean clipPlay = false;
    private int clipPlayStartTime = 0;
    private int clipPlayLastTime = 0x80000000;
    public enum State {
        IDLE,
        PLAY,
        STOP;
    }

    private State mEngineState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_project_edit);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mProject = new nexProject();

        m_ListView = (ListView)findViewById(R.id.listview_project_edit);
        mItemAdp = new ProjectEditItemAdapter(mProject, true);
        m_ListView.setAdapter(mItemAdp);

        m_ListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Toast.makeText(ProjectEditActivity.this, "Seek(" + 0 + ") : " + position, Toast.LENGTH_SHORT).show();
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

        seekBar = (SeekBar) findViewById(R.id.seekbar_project_edit);
        seekBar.setMax(0);
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

        mTextview = (TextView) findViewById(R.id.textview_project_edit_time);

        final Button btplay = (Button)findViewById(R.id.button4_play);
        btplay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mProject.getTotalClipCount(true) == 0) {
                    Toast.makeText(getApplicationContext(), "clip count is 0", Toast.LENGTH_SHORT).show();
                    return ;
                }
                clipPlay = false;
                if(mEngineState == State.IDLE || mEngineState == State.STOP) {
                    mEngine.setView(m_editorView);
                    clipPlayLastTime = mEngine.getProject().getTotalTime();
                    seekBar.setMax(mEngine.getProject().getTotalTime()/33);
                    mEngine.play();
                    mEngineState = State.PLAY;
                    btplay.setText("Stop");
                } else {
                    seekBar.setProgress(0);
                    mEngine.stop();
                    mEngineState = State.STOP;
                    btplay.setText("Play");
                }
            }
        });

        Button btadd = (Button)findViewById(R.id.button4_add);
        btadd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngineState == State.PLAY) {
                    Toast.makeText(getApplicationContext(), "try after play...", Toast.LENGTH_SHORT).show();
                    return;
                }

                Intent intent = new Intent();
                intent.setType("*/*");
                String[] mimetypes = {"image/*", "video/*"};
                intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        Button btclear = (Button)findViewById(R.id.button4_clear);
        btclear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mEngineState == State.PLAY) {
                    Toast.makeText(getApplicationContext(), "try after play...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                mProject.allClear(true);
                mEngine.updateProject();
                m_editorView.setBlackOut(true);
                mItemAdp.notifyDataSetChanged();
            }
        });

        Button btSolid = (Button)findViewById(R.id.button4_solid);
        btSolid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngineState == State.PLAY) {
                    Toast.makeText(getApplicationContext(), "try after play...", Toast.LENGTH_SHORT).show();
                    return;
                }
                mProject.add(0, true, nexClip.getSolidClip(0xff000000));
                mProject.getClip(0, true).setImageClipDuration(3500);
                mProject.getClip(0,true).getTransitionEffect().setTransitionEffect("com.nexstreaming.kinemaster.builtin.transition.crossfade");
                mProject.getClip(0,true).getTransitionEffect().setDuration(1500);
                mItemAdp.notifyDataSetChanged();
                nexProject cloneProject = nexProject.clone(mProject);
                cloneProject.add(0, true, nexClip.getSolidClip(0xff000000));
                cloneProject.getClip(0, true).setImageClipDuration(3500);
                cloneProject.getClip(0, true).getTransitionEffect().setTransitionEffect("com.nexstreaming.kinemaster.builtin.transition.crossfade");
                cloneProject.getClip(0,true).getTransitionEffect().setDuration(1500);
                cloneProject.updateProject();
                mEngine.updateProject();

            }
        });

        Button btExport = (Button)findViewById(R.id.button4_export);
        btExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine.getProject() == null ) {
                    Toast.makeText(getApplicationContext(), "you must add clip!", Toast.LENGTH_SHORT).show();
                    return;
                }

                if( mEngine.getProject().getTotalClipCount(true) == 0 ){
                    Toast.makeText(getApplicationContext(), "you must add clip!", Toast.LENGTH_SHORT).show();
                    return;
                }

                mEngine.stop();
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                intent.putExtra("AutoPauseResume",true);
                startActivity(intent);

            }
        });

        mEditorListener = new nexEngineListener(){
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                if(mProject.getTotalClipCount(true)>0) {
                    mTextview.setText(String.valueOf(currentTime));
                    seekBar.setProgress(currentTime / 33);
                    if( clipPlay ) {
                        if (clipPlayLastTime < (currentTime + 33)) {
                            mEngine.stop();
                            mEngineState = State.STOP;
                        }
                    }
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
                seekBar.setProgress(0);
                mEngine.seek(0);
                mEngineState = State.STOP;
                btplay.setText("Play");
            }

            @Override
            public void onPlayFail(int err, int iClipID) {
                Log.d(TAG, "onPlayFail : err=" + err + " iClipID=" + iClipID);
                Toast.makeText(getApplicationContext(), "Play Fail", Toast.LENGTH_SHORT).show();
                mEngine.stop();
                mEngineState = State.STOP;
                btplay.setText("Play");
            }

            @Override
            public void onPlayStart() {
                seekBar.setMax(mProject.getTotalTime()/33);
                seekBar.setProgress(0);
                Log.d(TAG,"Project Duration="+mProject.getTotalTime());
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

        m_editorView = (nexEngineView)findViewById(R.id.engineview_project_edit);
        m_editorView.setBlackOut(true);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setEventHandler(mEditorListener);
        mEngine.setProject(mProject);
        mEngine.setView(m_editorView);

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                nexClip clip = nexClip.getSupportedClip(filepath);
                if(clip != null) {
                    if (clip.getTotalTime() < 2000 && clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        Toast.makeText(getApplicationContext(), "select content`s duration 2000 over", Toast.LENGTH_SHORT).show();
                        return;
                    }

                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }

                    if( !clip.ableToDecoding() ){
                        Toast.makeText(getApplicationContext(), "ableToDecoding fail", Toast.LENGTH_LONG).show();
                        return;
                    }

                    mProject.add(clip);
                    mEngine.updateProject();
                    mItemAdp.notifyDataSetChanged();
                }else{
                    Toast.makeText(ProjectEditActivity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                }
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);

                    nexClip clip = nexClip.getSupportedClip(filepath);

                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }

                    if( clip != null ) {
                        mProject.add(clip);

                        mItemAdp.notifyDataSetChanged();
                    }else{
                        Toast.makeText(ProjectEditActivity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                    }
                }
            }
            mProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
            seekBar.setMax(mProject.getTotalTime()/33);
        }
        if(requestCode == 200 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.EFFECT_OPTION);
            String effect = data.getStringExtra(Constants.EFFECT);
            mEngine.getProject().getClip(index,true).getClipEffect().setEffect(effect);
            mEngine.getProject().getClip(index,true).getClipEffect().updateEffectOptions(opt, true);

        }
        if(requestCode == 300 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.TRANSITION_EFFECT_OPTION);
            String effect = data.getStringExtra(Constants.TRANSITION_EFFECT);
            mEngine.getProject().getClip(index,true).getTransitionEffect().setTransitionEffect(effect);
            mEngine.getProject().getClip(index,true).getTransitionEffect().updateEffectOptions(opt, true);
            mEngine.getProject().getClip(index,true).getTransitionEffect().setDuration(1000);
        }

        if(requestCode == 400 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.OVERLAY_FILTER_OPTION);
            String effect = data.getStringExtra(Constants.OVERLAY_FILTER_ID);

            nexOverlayFilter filter =nexEffectLibrary.getEffectLibrary(getApplicationContext()).findOverlayFilterById(effect);

            filter.setWidth(300);
            filter.setHeight(300);
            nexOverlayItem overlayItem = new nexOverlayItem(filter,nexOverlayItem.AnchorPoint_MiddleMiddle,false,0,0,0,6000);
            mEngine.getProject().clearOverlay();
            mEngine.getProject().addOverlay(overlayItem);

        }
        mEngine.updateProject();

    }
    public void getClipeffect(int pos)
    {
        index = pos;
        mEngine.stop();
        Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
        intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_CLIP);
        startActivityForResult(intent, 200);
    }
    public void getTransition(int pos)
    {
        index = pos;
        mEngine.stop();
        Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
        intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_TRANSITION);
        startActivityForResult(intent, 300);
    }

    public void getOverlayFilter(int pos){
        index = pos;
        mEngine.stop();
        Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
        intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_OVERLAYFILTER);
        startActivityForResult(intent, 400);
    }

    public boolean isEnginePlaying() {
        if(mEngineState == State.PLAY)
            return true;
        return false;
    }

    void playClip(int index ){
        if(mEngineState == State.PLAY) {
            mEngine.stop();
            mEngineState = State.STOP;
            return;
        }

        mEngine.updateProject();
        int timePositions[] = mEngine.getProject().getClipPositionTime(index);
        if( timePositions != null ) {
            mEngineState = State.PLAY;
            Log.d(TAG, "playClip index="+index+", startTime="+timePositions[0]+", endTime="+timePositions[1]);
            clipPlay = true;
            clipPlayStartTime = timePositions[0];
            clipPlayLastTime = timePositions[1];
            mEngine.seek(timePositions[0]);
            mEngine.resume();

        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        mEngine.setEventHandler(mEditorListener);
    }
}
