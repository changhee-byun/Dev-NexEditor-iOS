/******************************************************************************
 * File Name        : AudioTrackActivity.java
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

import android.app.ActionBar;
import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Color;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexAudioItem;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.util.ArrayList;


public class AudioTrackActivity extends Activity {
    private static final String TAG = "AudioTrack";
    private LinearLayout timeline1;
    private LinearLayout timeline2;
    private LinearLayout timeline3;
    private Button btAdd;
    private Button btPlay;
    private Button btExport;

    private nexEngine mEngine;
    private nexEngineListener mEditorListener = null;
    private nexProject project = new nexProject();

    private ArrayList<ClipOnTimeline> m_listfilepath = new ArrayList<>();

    private int mStartTime;
    private int mEndTime;
    private int mStartTrim;
    private int mEndTrim;
    private int index = 0;
    private ArrayList<String> mEnv_Time;
    private ArrayList<String> mEnv_Vol;
//    private ImageView mIV;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_audio_track);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        nexClip clip = nexClip.getSolidClip(Color.RED);
        if( clip != null ) {
            project.add(clip);
        }
        btAdd = (Button)findViewById(R.id.button_audio_track_add);
        btAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,false);
                intent.setAction(Intent.ACTION_GET_CONTENT);

                startActivityForResult(intent, 100);
            }
        });
        btPlay = (Button)findViewById(R.id.button_audio_track_play);
        btPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(project.getTotalClipCount(true) == 0) {
                    Toast.makeText(getApplicationContext(), "clip is 0", Toast.LENGTH_SHORT).show();
                    return ;
                }

                mEngine.stop();
                project.setBackgroundConfig(0, false, false);
                int duration = getAudioTrackDuration(project);
                project.getClip(0,true).setImageClipDuration(duration);

                mEngine.play();
            }
        });
        btExport = (Button)findViewById(R.id.button_audio_track_export);
        btExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(project.getTotalClipCount(true) == 0) {
                    Toast.makeText(getApplicationContext(), "clip is 0", Toast.LENGTH_SHORT).show();
                    return ;
                }

                mEngine.stop();
                if(project.getTotalClipCount(false) > 0) {
                    project.setBackgroundConfig(0, false, false);
                    int duration = getAudioTrackDuration(project);
                    project.getClip(0, true).setImageClipDuration(duration);
                    mEngine.updateProject();

                    Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                    startActivity(intent);
                }else{
                    Toast.makeText(getApplicationContext(), String.format("Project is not set"), Toast.LENGTH_SHORT).show();
                }
            }
        });
        timeline1 = (LinearLayout) findViewById(R.id.linear_audio_track_timeline1);
        timeline2 = (LinearLayout) findViewById(R.id.linear_audio_track_timeline2);
        timeline3 = (LinearLayout) findViewById(R.id.linear_audio_track_timeline3);

        mEditorListener = new nexEngineListener(){
            private int mLastTimeSec = 0;
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
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

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setProject(project);
        mEngine.setEventHandler(mEditorListener);

    }
    protected int getAudioTrackDuration(nexProject mProject){
        int duration=0;
        for(int i = 0; i<mProject.getTotalClipCount(false); i++){
            nexAudioItem audioClip = mProject.getAudioItem(i);
            if(duration < audioClip.getEndTime()){
                duration = audioClip.getEndTime();
            }
        }

        return duration;
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100){
            // for KMSA-280
            if( data == null ) return;

            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                m_listfilepath.add(new ClipOnTimeline(filepath, 1));
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    m_listfilepath.add(new ClipOnTimeline(filepath, 1));
                }
            }
            index = m_listfilepath.size()-1;

            Intent intent = new Intent(this, AudioTrackInfoActivity.class);
            intent.putExtra("filepath", filepath);
            startActivityForResult(intent, 200);
        }else if(requestCode==200) {
            if (data != null) {
                if(project.getTotalClipCount(false)>=3){
                    Toast.makeText(getApplicationContext(), String.format("Max audio clip count is 3"), Toast.LENGTH_SHORT).show();
                    return;
                }
                mStartTime = data.getIntExtra("StartTime", 0);
                mEndTime = data.getIntExtra("EndTime", 60000);
                mStartTrim = data.getIntExtra("StartTrim", 0);
                mEndTrim = data.getIntExtra("EndTrim", 60000);

                nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(index).getClipPath());

                if( clip == null || clip.getClipType() != nexClip.kCLIP_TYPE_AUDIO)
                {
                    Toast.makeText(getApplicationContext(), String.format("Not supported clips"), Toast.LENGTH_SHORT).show();
                    return;
                }

                int id = project.addAudio(clip, mStartTime, mEndTime);
                project.findAudioItem(id).setTrim(mStartTrim, mEndTrim);
                m_listfilepath.get(index).setID(id);
                for (int i = 0; i < project.getTotalClipCount(false); i++) {
                    m_listfilepath.get(i).setTimeline(1);
                }
//            project.changeAudio(project.getAudioItem(index), mStartTime, mStartTime + project.getAudioItem(index).getEndTime());
                //project.getAudioItem(index).setTrim(mTrimTime, project.getClip(index, false).getTotalTime());
                //project.getClip(index, false).setClipVolume(mBgmVol);
                //for(int i=0; i<mEnv_Time.size(); i++){
//                project.getClip(index,false).getAudioEnvelop().addVolumeEnvelope(Integer.parseInt(mEnv_Time.get(i)),Integer.parseInt(mEnv_Vol.get(i)));
//            }
                timeline1.removeAllViews();
                timeline2.removeAllViews();
                timeline3.removeAllViews();
                project.setBackgroundConfig(0, false, false);

                int timeline1_endtime = 0;
                int timeline2_endtime = 0;
                for (int i = 0; i < project.getTotalClipCount(false); i++) {
                    boolean gototimeline2 = false;
                    boolean gototimeline3 = false;

                    float weight = (project.getAudioItem(i).getEndTime() - project.getAudioItem(i).getStartTime()) / 1000;
                    TextView textView = new TextView(AudioTrackActivity.this);
                    textView.setLayoutParams(new LinearLayout.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT, weight));

                    MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
                    metadataRetriever.setDataSource(project.getClip(i, false).getPath());

                    String title = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
                    String artist = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);

                    if (title == null) {
                        title = "Unknown";
                    }
                    if (artist == null) {
                        artist = "Unknown";
                    }

                    textView.setTextSize(13);
                    textView.setText(title + "(" + artist + ")");
                    if (timeline1.getChildCount() == 0) {
                        textView.setBackgroundColor(Color.parseColor("#00FFFFFF"));
                        textView.setTextColor(Color.parseColor("#FF7200"));
                        timeline1.addView(textView);
                        timeline1_endtime = project.getAudioItem(i).getEndTime();
                    } else {
                        switch (timeline1.getChildCount()) {
                            case 1:
                                textView.setBackgroundColor(Color.parseColor("#555555FF"));
                                textView.setTextColor(Color.parseColor("#FF7200"));
                                break;
                            case 2:
                                textView.setBackgroundColor(Color.parseColor("#999999FF"));
                                textView.setTextColor(Color.parseColor("#FF7200"));
                                break;
                        }
                        for (int j = 0; j < project.getTotalClipCount(false); j++) {
                            if (project.getClip(j, false) != clip) {
                                if (project.getClip(i, false).getProjectStartTime() < timeline1_endtime) {
                                    gototimeline2 = true;
                                    break;
                                }
                            }
                        }
                        if (gototimeline2 == false) {
                            timeline1.addView(textView);
                            timeline1_endtime = project.getAudioItem(i).getEndTime();
                        } else {
                            for (int j = 0; j < m_listfilepath.size(); j++) {
                                if (m_listfilepath.get(j).getID() == id) {
                                    m_listfilepath.get(j).setTimeline(2);
                                }
                            }
                            if (timeline2.getChildCount() == 0) {
                                TextView textView2 = new TextView(AudioTrackActivity.this);
                                weight = project.getAudioItem(i).getStartTime() / 1000;
                                textView2.setLayoutParams(new LinearLayout.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT, weight));
                                timeline2.addView(textView2);
                                textView.setBackgroundColor(Color.parseColor("#00FFFFFF"));
                                textView.setTextColor(Color.parseColor("#FF7200"));
                                timeline2.addView(textView);
                                timeline2_endtime = project.getAudioItem(i).getEndTime();
                            } else {
                                switch (timeline2.getChildCount()) {
                                    case 2:
                                        textView.setBackgroundColor(Color.parseColor("#555555FF"));
                                        textView.setTextColor(Color.parseColor("#FF7200"));
                                        break;
                                    case 3:
                                        textView.setBackgroundColor(Color.parseColor("#999999FF"));
                                        textView.setTextColor(Color.parseColor("#FF7200"));
                                        break;
                                }
                                for (int j = 0; j < project.getTotalClipCount(false); j++) {
                                    if (project.getClip(j, false).getPath() != clip.getPath()) {
                                        if (project.getClip(i, false).getProjectStartTime() < timeline2_endtime) {
                                            gototimeline3 = true;
                                            break;
                                        }
                                    }
                                }
                                if (gototimeline3 == false) {
                                    timeline2.addView(textView);
                                    timeline2_endtime = project.getAudioItem(i).getEndTime();
                                } else {
                                    m_listfilepath.get(index).setTimeline(3);
                                    if (timeline3.getChildCount() == 0) {
                                        TextView textView3 = new TextView(AudioTrackActivity.this);
                                        weight = project.getAudioItem(i).getStartTime() / 1000;
                                        textView3.setLayoutParams(new LinearLayout.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT, weight));
                                        timeline3.addView(textView3);
                                        textView.setBackgroundColor(Color.parseColor("#00FFFFFF"));
                                        textView.setTextColor(Color.parseColor("#FF7200"));
                                        timeline3.addView(textView);
                                    } else {
                                        switch (timeline3.getChildCount()) {
                                            case 2:
                                                textView.setBackgroundColor(Color.parseColor("#555555FF"));
                                                textView.setTextColor(Color.parseColor("#FF7200"));
                                                break;
                                            case 3:
                                                textView.setBackgroundColor(Color.parseColor("#999999FF"));
                                                textView.setTextColor(Color.parseColor("#FF7200"));
                                                break;
                                        }
                                        timeline3.addView(textView);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    protected int getTimeLine(String path){
        for(int i=0; i<m_listfilepath.size(); i++){
            if(m_listfilepath.get(i).getTimeline(path) != -1){
                return m_listfilepath.get(i).getTimeline(path);
            }
        }
        return -1;
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
    
    class ClipOnTimeline{
        private  String filepath;
        private  int timeline;
        private  int id;

        ClipOnTimeline(String path, int time){
            filepath = path;
            timeline = time;
        }
        void setTimeline(int time){
            timeline = time;
        }

        void setClipPath(String path){
            filepath = path;
        }
        void setID(int mid){id = mid;}
        int getID(){return id;}
        String getClipPath(){
            return filepath;
        }

        int getTimeline(String path){
            if(filepath.equals(path)) {
                return timeline;
            }else{
                return -1;
            }
        }
    }
}

