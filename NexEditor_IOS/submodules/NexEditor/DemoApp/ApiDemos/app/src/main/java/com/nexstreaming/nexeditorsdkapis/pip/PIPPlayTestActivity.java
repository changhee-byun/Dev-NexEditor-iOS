/******************************************************************************
 * File Name        : PIPPlayTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.pip;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class PIPPlayTestActivity extends Activity {
    public static final String TAG = "PIPDisplayTest";
    public static final int MAX_SELECTED_PIP_COUNT = 4;

    private ArrayList<nexOverlayItem> mSelectedPIP;
    private ArrayList<nexOverlayItem> mSelectedPIP2;
    private Button mButtonAdd;
    private Button mButtonPlay;
    private Button mButtonExport;

    private ProgressBar mProgressBar;

    private ListView mListViewSelctedPIP;
    private ArrayAdapter<String> mListAdapterSelectedPIP;

    private nexEngine mEngine;
    private nexProject mDisplayProject;
    private nexProject mExportProject;
    private nexEngineView mEngineView;

    private File mCurrentExportFile;

    private enum State {
        IDLE,
        STOP,
        PLAY,
        EXPORT
    }

    private State mState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_pipplay_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if(!nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
            Toast.makeText(getApplicationContext(),"API Level is not supported!",Toast.LENGTH_SHORT).show();
            finish();
        }


        nexClip clip = nexClip.getSolidClip(Color.RED);
        clip.setImageClipDuration(20000);

        mDisplayProject = new nexProject();
        mDisplayProject.add(clip);
        //if( nexApplicationConfig.getAspectRatioMode() ==  nexApplicationConfig.kAspectRatio_Mode_9v16
        if( !nexApplicationConfig.getAspectProfile().isLandscapeMode() &&
                clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            mDisplayProject.getClip(0, true).setRotateDegree(nexClip.kClip_Rotate_270);
        }

        mEngineView = (nexEngineView) findViewById(R.id.engineview_pip_play_test);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(mEngineView);
       // mEngine.setEventHandler(mEngineListener);

        mProgressBar = (ProgressBar) findViewById(R.id.progress_pip_play_test);
        mProgressBar.setProgress(0);

        mSelectedPIP = new ArrayList<nexOverlayItem>();
        mSelectedPIP2 = new ArrayList<nexOverlayItem>();

        mListViewSelctedPIP = (ListView) findViewById(R.id.listview_pip_play_test);
        mListAdapterSelectedPIP = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_list_item_1);
        mListViewSelctedPIP.setAdapter(mListAdapterSelectedPIP);
        mListViewSelctedPIP.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Log.d(TAG, "mListViewSelctedPIP click -> ");
            }
        });

        mButtonAdd = (Button) findViewById(R.id.button_pip_play_test_add);
        mButtonAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mState == State.PLAY) {
                    mProgressBar.setProgress(0);
                    mState = State.STOP;
                    mEngine.stop();
                }

                Intent intent = new Intent();
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);

                startActivityForResult(intent, 100);
            }
        });

        mButtonPlay = (Button) findViewById(R.id.button_pip_play_test_play);
        mButtonPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonPlay click ->");
                mEngine.setProject(mDisplayProject);

                if(mState == State.PLAY) {
                    mEngine.stop();
                    mButtonPlay.setText("Play");
                } else {
                    mState = State.PLAY;
                    mProgressBar.setProgress(0);
                    mProgressBar.setMax(mDisplayProject.getTotalTime() / 33);
                    mEngine.play();
                    mButtonPlay.setText("Stop");
                }
            }
        });

        mButtonExport = (Button) findViewById(R.id.button_pip_play_test_export);
        mButtonExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonExport -> ");


                // for KMSA-286
                if( mDisplayProject.getOverlayItems().size() < 4 )
                {
                    Toast.makeText(getApplicationContext(), String.format("Please select 4 videos for export testing."), Toast.LENGTH_SHORT).show();
                    return;
                }

                mState = State.EXPORT;
                mProgressBar.setProgress(0);
                mProgressBar.setMax(200);

                mEngine.stop();

                int width = nexApplicationConfig.getAspectProfile().getWidth();
                int height = nexApplicationConfig.getAspectProfile().getHeight();

/*
                        if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16 ){
                            width = 720;
                            height = 1280;
                        }
*/
                mCurrentExportFile = getExportFile(width, height);

                mExportProject = nexProject.clone(mDisplayProject);
                mSelectedPIP2.addAll(mExportProject.getOverlayItems());

                mExportProject.removeOverlay(mSelectedPIP2.get(2).getId());
                mExportProject.removeOverlay(mSelectedPIP2.get(3).getId());

                mEngine.setProject(mExportProject);
                mEngine.updateProject();

                // export 1
                mEngine.export(mCurrentExportFile.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, 44100);
                Log.d(TAG, "first export export start !!!");
            }
        });
        mEngine.setEventHandler(mEngineListener);
    }



    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            if(mSelectedPIP.size() >= MAX_SELECTED_PIP_COUNT) {
                Toast.makeText(getApplicationContext(), "selected list size > " + MAX_SELECTED_PIP_COUNT + ", so fail add video to list", Toast.LENGTH_SHORT).show();
                return ;
            }

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                nexClip clip = nexClip.getSupportedClip(filepath);

                if( clip != null ) {

                    mListAdapterSelectedPIP.add(filepath + "(" + clip.getTotalTime() + ")");
                    int id = mSelectedPIP.size();
                    nexOverlayImage ov = new nexOverlayImage("video" + id, clip);

                    int x = 320 + 640 * (id % 2);
                    int y = 180 + 360 * (id / 2);

                    nexOverlayItem ovitem = new nexOverlayItem(ov, x, y, 0, mDisplayProject.getTotalTime());
                    ovitem.setScale(1280f / (float) clip.getWidth() * 0.5f, 720f / (float) clip.getHeight() * 0.5f);

                    mSelectedPIP.add(ovitem);
                    mDisplayProject.addOverlay(ovitem);
                }
            }

            mEngine.updateProject();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        ApiDemosConfig.getApplicationInstance().getEngine();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
    }


    /**-----------------------------*
     *         Event Listener       *
     * ----------------------------**/
    nexEngineListener mEngineListener = new nexEngineListener() {
        boolean firstExportDone = false;


        @Override
        public void onStateChange(int oldState, int newState) {
            if(newState == 1) {
                if(mState == State.EXPORT) {
                    return ;
                }
                mState = State.STOP;
                mProgressBar.setProgress(0);
                mButtonPlay.setText("Play");
            } else if(newState == 2) {
                mState = State.PLAY;
            } else if(newState == 3) {
                mState = State.EXPORT;
            }
        }

        @Override
        public void onTimeChange(int currentTime) {
            Log.i(TAG, "onTimeChange: currentTime: " + currentTime + "mState: " + mState);
            if(mState != State.EXPORT)
                mProgressBar.setProgress(currentTime / 33);
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
            Log.d(TAG, "onEncodingDone -> iserror: " + iserror + " result: " + result);
            if(firstExportDone) {
                Log.d(TAG, "first export export done !!!");
                mEngine.stop();

                // create clip
                nexClip clip = new nexClip(mCurrentExportFile.getAbsolutePath());
                if (clip == null) {
                    Log.d(TAG, "mButtonExport -> clip is null(" + mCurrentExportFile.getAbsolutePath() + ")");
                    return;
                }

                // add clip to project
                mExportProject.add(clip);

                // remove solid(red) clip
                mExportProject.remove(mExportProject.getClip(0, true));

                // remove pip 0, 1
                mExportProject.removeOverlay(mSelectedPIP2.get(0).getId());
                mExportProject.removeOverlay(mSelectedPIP2.get(1).getId());

                // add pip 2, 3
                mExportProject.addOverlay(mSelectedPIP2.get(2));
                mExportProject.addOverlay(mSelectedPIP2.get(3));

                // set property
                int width = nexApplicationConfig.getAspectProfile().getWidth();
                int height = nexApplicationConfig.getAspectProfile().getHeight();

/*
                if (nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
                    width = 720;
                    height = 1280;
                }
*/
                mEngine.updateProject();

                // export 2
                Log.d(TAG, "second export start !!!");
                mEngine.export(getExportFile(width, height).getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, 44100);
            }
        }

        @Override
        public void onPlayEnd() {
            Log.i(TAG, "onPlayEnd");
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
            if(firstExportDone)
                mProgressBar.setProgress(100 + percent);
            else
                mProgressBar.setProgress(percent);

            if(percent == 100) {
                if(firstExportDone) {
                    Log.d(TAG, "first export done -> false");
                    firstExportDone = false;
                    mState = State.IDLE;
                    return ;
                }

                firstExportDone = true;
            }

            Log.d(TAG, "progress value: " + mProgressBar.getProgress() + " max: " + mProgressBar.getMax());
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

    private File getExportFile(int wid , int hei) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "PIP_"+wid+"X"+hei +"_"+ export_time+".mp4");
        return exportFile;
    }
}
