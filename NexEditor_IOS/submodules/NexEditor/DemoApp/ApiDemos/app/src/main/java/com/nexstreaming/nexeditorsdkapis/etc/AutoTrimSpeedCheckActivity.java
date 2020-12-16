/******************************************************************************
 * File Name        : AutoTrimSpeedCheckActivity.java
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

package com.nexstreaming.nexeditorsdkapis.etc;

import android.app.Activity;
import android.content.Intent;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class AutoTrimSpeedCheckActivity extends Activity {
    public static final String TAG = "AutoTrimSpeedCheck";

    enum State {
        IDLE,
        STOP,
        AUTOTRIM,
        EXPORT,
        PLAY;
    }
    private File mFile;

    private ArrayList<String> mListFilePath;
    private nexEngine mEngine;
    private nexEngineView mEditorView;
    private nexClip mClip;
    private nexProject mProject;
    private TextView mTextViewResult;
    private TextView mTextViewDuration;
    private TextView mTextViewInfo;
    private ProgressBar mProgressAutoTrim;
    private Switch mSwitchIDRFrameMode;

    private int mTrimCount = 15;
    private int mTrimInterval = 1000;
    private long mAutoTrimStartTime = 0;
    private long mAutoTrimEndTime = 0;
    private boolean isIDRFrameOn = false;

    // if this feature is 1, autotrim speed will be high. default is 0
    private int mDecodeMode = 0;
    private StringBuilder mAutoTrimResult = null;


    private State mEngineState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_auto_trim_speed_check);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = new ArrayList<String>();

        Intent intent = getIntent();
        mListFilePath = intent.getStringArrayListExtra("filelist");
        mClip = nexClip.getSupportedClip(mListFilePath.get(0));

        if( mClip == null ){
            finish();

            String skip = "do not support this Clip";
            Toast.makeText(getApplicationContext(), skip, Toast.LENGTH_LONG).show();
            Log.e(TAG, "do not support this Clip");

            return;
        }
        if(mClip.getTotalTime() < 30000) {
            Toast.makeText(getBaseContext(), "Content must be at least 30 seconds!!", Toast.LENGTH_SHORT).show();
            finish();
        }

        mProject = new nexProject();
        mProject.add(mClip);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        mEngine.setProject(mProject);
        mEngine.setEventHandler(mEngineListener);
        mEditorView = (nexEngineView)findViewById(R.id.engineview_autotrim_speedcheck);
        mEngine.setView(mEditorView);

        mProgressAutoTrim = (ProgressBar)findViewById(R.id.progressbar_autotrim_speedcheck);
        mProgressAutoTrim.setMax(100);
        mProgressAutoTrim.setProgress(0);

        mTextViewResult = (TextView)findViewById(R.id.textview_autotrim_speedcheck_result);
        mTextViewDuration = (TextView)findViewById(R.id.textview_autotrim_speedcheck_duration);
        mTextViewInfo = (TextView)findViewById(R.id.textview_autotrim_speedcheck_info);

        mSwitchIDRFrameMode = (Switch) findViewById(R.id.switch_autotrim_speedcheck_idrframe_on);
        mSwitchIDRFrameMode.setChecked(true);

        final Button play = (Button)findViewById(R.id.button_autotrim_speedcheck_playstop);
        play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mEngineState == State.STOP || mEngineState == State.IDLE ) {
                    Log.d(TAG, "rooney 1 : " + mEngineState);
                    mEngineState = State.PLAY;
                    play.setText("STOP");
                    mProgressAutoTrim.setMax(mProject.getTotalTime());
                    mEngine.play();
                } else if(mEngineState == State.AUTOTRIM) {
                    Toast.makeText(getApplicationContext(), "during autotrim, you can`t play.", Toast.LENGTH_SHORT).show();
                    return ;
                } else {
                    Log.d(TAG, "rooney 2 : " + mEngineState);
                    play.setText("PLAY");
                    mEngine.stop();
                    mEngineState = State.STOP;
                }
            }
        });

        final Button export = (Button) findViewById(R.id.button_autotrim_speedcheck_export);
        export.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mEngineState == State.PLAY)
                {
                    mEngine.stop();
                    play.setText("PLAY");
                    mEngineState = State.STOP;
                }

                mEngine.stop();
                mEngineState = State.EXPORT;
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });

        final Button autotrim = (Button)findViewById(R.id.button_autotrim_speedcheck_autotrim);
        autotrim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mEngineState == State.AUTOTRIM) {
                    Toast.makeText(getApplicationContext(), "already autotrimming...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if( mEngineState == State.PLAY) {
                    mEngine.stop();
                    mEngineState = State.STOP;
                    play.setText("PLAY");
                }

                mProgressAutoTrim.setProgress(0);
                mProgressAutoTrim.setMax(100);
                mAutoTrimStartTime = System.currentTimeMillis();

                mEngineState = State.AUTOTRIM;

                Log.d(TAG, "isIDRFrameOn: " + isIDRFrameOn);
                Log.d(TAG, "mTrimInterval: " + mTrimInterval);
                Log.d(TAG, "mTrimCount: " + mTrimCount);

                isIDRFrameOn = mSwitchIDRFrameMode.isChecked();
                mEngine.autoTrim(mClip.getPath(), isIDRFrameOn, mTrimInterval, mTrimCount, mDecodeMode, new nexEngine.OnAutoTrimResultListener() {
                    @Override
                    public void onAutoTrimResult(int errcode, int[] arrayData) {
                        if(arrayData == null || arrayData.length == 0) {
                            if(errcode !=0) {
                                updateTextViewResult("Error !!! code(" + errcode + ")");
                                mEngine.autoTrimStop();
                            }
                        } else {
                            mProject.allClear(true);

                            mAutoTrimResult = null;
                            mAutoTrimResult = new StringBuilder();

                            for (int i = 0; i < arrayData.length; i++) {
                                mAutoTrimResult.append("CTS : " + arrayData[i] + "\n");
                                mProject.add(nexClip.dup(mClip));
//                                Log.d(TAG, "arrayData[start]: " + arrayData[i] + ", arrayData[end]: " + arrayData[i] + mTrimInterval);
                                mProject.getClip(i, true).getVideoClipEdit().setTrim(arrayData[i], arrayData[i] + mTrimInterval);
                            }

                            mAutoTrimEndTime = System.currentTimeMillis();
                            mTextViewResult.setText(mAutoTrimResult.toString());
                            mTextViewInfo.setText("default interval: " + mTrimInterval + ", count: " + mTrimCount + " time: " + (mAutoTrimEndTime - mAutoTrimStartTime));
                        }
                        mEngineState = State.STOP;
                    }
                });
            }
        });

        final Button autotrimNExport = (Button) findViewById(R.id.button_autotrim_speedcheck_autotrim_n_export);
        autotrimNExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mEngineState == State.AUTOTRIM) {
                    Toast.makeText(getApplicationContext(), "already autotrimming...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if( mEngineState == State.PLAY) {
                    mEngine.stop();
                    mEngineState = State.STOP;
                    play.setText("PLAY");
                }

                mProgressAutoTrim.setProgress(0);
                mProgressAutoTrim.setMax(200);
                mAutoTrimStartTime = System.currentTimeMillis();

                isIDRFrameOn = mSwitchIDRFrameMode.isChecked();
                mEngineState = State.AUTOTRIM;
                mEngine.autoTrim(mClip.getPath(), isIDRFrameOn, mTrimInterval, mTrimCount, mDecodeMode, new nexEngine.OnAutoTrimResultListener() {
                    @Override
                    public void onAutoTrimResult(int errcode, int[] arrayData) {
                        if(arrayData == null || arrayData.length == 0) {
                            updateTextViewResult("Error !!! code("+errcode+")");
                            mEngine.autoTrimStop();
                        } else {
                            mProject.allClear(true);

                            mAutoTrimResult = null;
                            mAutoTrimResult = new StringBuilder();


                            for (int i = 0; i < arrayData.length; i++) {
                                mAutoTrimResult.append("CTS : " + arrayData[i] + "\n");
                                mProject.add(nexClip.dup(mClip));
                                mProject.getClip(i, true).getVideoClipEdit().setTrim(arrayData[i], arrayData[i] + mTrimInterval);
                            }

                            mEngine.stop();
                            mEngine.updateProject();
                            if(isIDRFrameOn == true) {
                                mEngine.checkDirectExport(0);
                            } else {
                                mFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), 3);
                                mEngine.export(mFile.getAbsolutePath(),
                                        mProject.getClip(0, true).getWidth(),
                                        mProject.getClip(0, true).getHeight(),
                                        mProject.getClip(0, true).getVideoBitrate(),
                                        Long.MAX_VALUE,
                                        0,
                                        44100);
                            }
                        }
                    }
                });
            }
        });
    }

    /**
     * Listener
     */
    nexEngineListener mEngineListener = new nexEngineListener(){
        @Override
        public void onCheckDirectExport(int result) {
            Log.d(TAG, "onCheckDirectExport: " + result);
            if(result != 0) {
                Toast.makeText(getApplicationContext(), "check direct export fail!!", Toast.LENGTH_SHORT).show();
                Log.d(TAG, "checkDriectExport false: " + result);

            }
            else {
                Log.d(TAG, "checkDriectExport ok: " + result);
                mFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), 1);
                mEngine.directExport(mFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime(), 0);

            }
        }

        @Override
        public void onStateChange(int oldState, int newState) {

        }

        @Override
        public void onTimeChange(int currentTime) {
            mTextViewDuration.setText(currentTime + "");
            mProgressAutoTrim.setProgress(currentTime);
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
            Log.d(TAG, "onEncodingDone-> iserror: " + iserror + ", result: " + result);
            if(iserror) {
                Log.d(TAG, "onEncodingDone error: " + iserror);
            } else {
                if(mFile != null) {
                    MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                }
            }

            mAutoTrimEndTime = System.currentTimeMillis();
            mTextViewResult.setText(mAutoTrimResult.toString());
            mTextViewInfo.setText("default interval: " + mTrimInterval + ", count: " + mTrimCount + " time: " + (mAutoTrimEndTime - mAutoTrimStartTime));
            mEngineState = State.STOP;
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
            Log.d(TAG, "onEncodingProgress percent: " + percent);
            mProgressAutoTrim.setProgress(percent + 100);
            mTextViewDuration.setText((percent + 100) + "");
        }

        @Override
        public void onProgressThumbnailCaching(int progress, int maxValue) {
            mProgressAutoTrim.setProgress(progress);
            mTextViewDuration.setText(progress + "");
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

    @Override
    protected void onStop() {
        super.onStop();
        if( mEngineState == State.PLAY ) {
            mEngine.stop();
            mEngineState = State.STOP;
        }
    }

    @Override
    protected void onStart() {

        Log.d(TAG, "onStart");
        if(mEngineState == State.EXPORT)
        {
            mEngineState = State.IDLE;
            mEngine.setEventHandler(mEngineListener);
        }
        super.onStart();

    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        mEngine.autoTrimStop();
        if( mEngineState == State.PLAY ) {
            mEngine.stop();
            mEngineState = State.STOP;
        }
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }


    /**
     * Private Function
     */
    void updateTextViewResult(final String str)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTextViewResult.setText(str);
            }
        });
    }


    private File getExportFile(int wid , int hei, int mode ) {
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

}
