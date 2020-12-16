/******************************************************************************
 * File Name        : ReverseTestActivity.java
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
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
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
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;


public class ReverseTestActivity extends Activity {
    public static final String TAG = "ReverseTest";

    private ArrayList<String> mListFilePath;

    private nexEngineView mEditorView;
    private nexProject mProject;
    private nexEngine mEngin;
    private Stopwatch watch = new Stopwatch();

    private int mStartT = 0;
    private int mEndT = 0;
    private String mDstFilePath;

    private boolean isReversing = false;

    // if this feature is 1, autotrim speed will be high. default is 0
    private int mDecodeMode = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_reverse_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = getIntent().getStringArrayListExtra("filelist");

        nexClip clip = new nexClip(mListFilePath.get(0));
        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
            clip.setRotateDegree(clip.getRotateInMeta());
        }

        mEditorView = (nexEngineView) findViewById(R.id.engineview_reverse_test);

        mProject = new nexProject();

        mProject.add(clip);

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEditorListener);
        mEngin.setView(mEditorView);
        mEngin.setProject(mProject);
        mEngin.updateProject();

        Log.d(TAG, "mListFilePath 0: " + mListFilePath.get(0));

        // UI init
        ((ProgressBar) findViewById(R.id.progress_reverse_test)).setMax(100);


        final Button btPlay = (Button)findViewById(R.id.button_reverse_test_play);
        btPlay.setEnabled(false);
        btPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "btPlay onClick");
                ((Button) findViewById(R.id.button_reverse_test_reverse)).setEnabled(false);
                mEngin.play();
            }
        });

        Button btReverseStart = (Button)findViewById(R.id.button_reverse_test_reverse);
        btReverseStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String srcPath = mListFilePath.get(0);
                mDstFilePath = getReverseDstFilePath();
                String temppath = getReverseTempFilePath();

                int width = mProject.getClip(0, true).getWidth();
                int height = mProject.getClip(0, true).getHeight();
                int bitrate = mProject.getClip(0, true).getVideoBitrate();
//                        int bitrate = 6 * 1024 * 1024;
                long maxFileSize = Long.MAX_VALUE;

                Log.d(TAG, "=====================ReverseStartInfo=====================");
                Log.d(TAG, "srcPath: " + srcPath);
                Log.d(TAG, "dstPath: " + mDstFilePath);
                Log.d(TAG, "temppath: " + temppath);
                Log.d(TAG, "width: " + width);
                Log.d(TAG, "height: " + height);
                Log.d(TAG, "bitrate: " + bitrate);
                Log.d(TAG, "maxFileSize: " + maxFileSize);
                Log.d(TAG, "startT: " + mStartT);
                Log.d(TAG, "endT: " + mEndT);
                Log.d(TAG, "==========================================================");

                if(mStartT >= mEndT) {
                    Toast.makeText(getApplicationContext(), "startTime >= endTime, retry again!!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(mEndT - mStartT <= 1000) {
                    Toast.makeText(getApplicationContext(), "endTime - startTime <= 1000, retry again!!", Toast.LENGTH_SHORT).show();
                    return ;
                }


                isReversing = true;
                watch.reset();
                watch.start();
                mEngin.reverseStart(srcPath, mDstFilePath, temppath, width, height, bitrate, maxFileSize, mStartT, mEndT, mDecodeMode);

                ((Button)findViewById(R.id.button_reverse_test_reverse)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse_test_play)).setEnabled(false);
                ((ProgressBar) findViewById(R.id.progress_reverse_test)).setProgress(0);
            }
        });

        Button btReverseStop = (Button) findViewById(R.id.button_reverse_test_stop);
        btReverseStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isReversing = false;
                mEngin.reverseStop();

            }
        });


        SeekBar sbStartT = (SeekBar) findViewById(R.id.seekbar_reverse_test_start_time);
        sbStartT.setMax(mProject.getClip(0, true).getTotalTime());
        sbStartT.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                ((TextView) findViewById(R.id.textview_reverse_test_start_time)).setText(value + "");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mStartT = value;
                ((TextView) findViewById(R.id.textview_reverse_test_start_time)).setText(value + "");
            }
        });


        SeekBar sbEndT = (SeekBar) findViewById(R.id.seekbar_reverse_test_end_time);
        sbEndT.setMax(mProject.getClip(0, true).getTotalTime());
        sbEndT.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                ((TextView) findViewById(R.id.textview_reverse_test_end_time)).setText(value + "");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEndT = value;
                ((TextView) findViewById(R.id.textview_reverse_test_end_time)).setText(value + "");
            }
        });

    }


    private String getReverseDstFilePath() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String reverse = "Reverse";
        String reverseDirPath = sdCardPath + File.separator + "KM" + File.separator + reverse;

        File exportDir = new File(reverseDirPath);
        exportDir.mkdirs();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String filePath = ".KineMaster_" + reverse + "_" + export_time+".mp4";

        return reverseDirPath + File.separator + filePath;
    }

    private String getReverseTempFilePath() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String reversed = "Reverse";
        String reverseDirPath = sdCardPath + File.separator + "KM" + File.separator + reversed;

        File exportDir = new File(reverseDirPath);
        exportDir.mkdirs();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String filePath = "KineMaster_" + reversed + "_TEMP_" + export_time+".mp4";

        return reverseDirPath + File.separator + filePath;
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
        if(isReversing == true) {
            mEngin.reverseStop();
        }
        ApiDemosConfig.getApplicationInstance().releaseEngine();
    }

    /**------------------------------------------------------------------------
     *                          Event Listener                                *
     ------------------------------------------------------------------------**/
    nexEngineListener mEditorListener = new nexEngineListener() {
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
            Log.d(TAG, "onEncodeingDone() iserror: " + iserror + ", result: " + result + ", Elapsed="+watch.toString());

            if(result == nexEngine.nexErrorCode.TRANSCODING_USER_CANCEL.getValue() ||
                    result == nexEngine.nexErrorCode.CODEC_INIT.getValue() ||
                    result == nexEngine.nexErrorCode.FILEREADER_CREATE_FAIL.getValue() ||
                    result == nexEngine.nexErrorCode.TRANSCODING_CODEC_FAILED.getValue())
            {
                Log.d(TAG, "Reverse Stop");

                if(result == nexEngine.nexErrorCode.CODEC_INIT.getValue())
                    Toast.makeText(getApplicationContext(), "error: CODEC_INIT", Toast.LENGTH_SHORT).show();
                else if(result == nexEngine.nexErrorCode.FILEREADER_CREATE_FAIL.getValue())
                    Toast.makeText(getApplicationContext(), "error: FILEREADER_CREATE_FAIL", Toast.LENGTH_SHORT).show();
                else if(result == nexEngine.nexErrorCode.TRANSCODING_CODEC_FAILED.getValue())
                    Toast.makeText(getApplicationContext(), "error: TRANSCODING_CODEC_FAILED", Toast.LENGTH_SHORT).show();

                ((Button)findViewById(R.id.button_reverse_test_reverse)).setEnabled(true);
                ((ProgressBar) findViewById(R.id.progress_reverse_test)).setProgress(0);
                //((SeekBar) findViewById(R.id.seekbar_reverse_test_start_time)).setProgress(0);
                //((SeekBar) findViewById(R.id.seekbar_reverse_test_end_time)).setProgress(0);

                return ;
            }

            // btPlay set Enable
            ((Button) findViewById(R.id.button_reverse_test_play)).setEnabled(true);

            // if receive onEncodingDone in ReverseTestActivity, this event is ReverseDone

            // so remove srcClip and add dstClip
            mProject.remove(mProject.getClip(0, true));

            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mDstFilePath}, null, null);
            mProject.add(new nexClip(mDstFilePath));

            isReversing = false;
//            // play
//            mEngin.play();

            ((Button)findViewById(R.id.button_reverse_test_reverse)).setEnabled(true);
            ((Button)findViewById(R.id.button_reverse_test_play)).setEnabled(true);
        }

        @Override
        public void onPlayEnd() {
            Log.d(TAG," playEnd");
            ((Button) findViewById(R.id.button_reverse_test_reverse)).setEnabled(true);
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
            Log.d(TAG, "progress: " + percent);
            ((ProgressBar) findViewById(R.id.progress_reverse_test)).setProgress(percent);
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


}
