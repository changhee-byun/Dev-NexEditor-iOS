/******************************************************************************
 * File Name        : TranscodingTestActivity.java
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
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;


public class TranscodingTestActivity extends Activity {
    public static final String TAG = "TranscodingTest";

    private ArrayList<String> mListFilePath;

    private nexProject mProject;
    private nexEngine mEngin;

    private int mDstWidth = 0;
    private int mDstHeight = 0;
    private int mDstDisplayWidth = 0;
    private int mDstDisplayHeight = 0;
    private int mDstFPS = 0;
    private String mDstFilePath;

    private boolean isTranscoding = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_transcoding_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = getIntent().getStringArrayListExtra("filelist");

        nexClip clip = new nexClip(mListFilePath.get(0));

        if( clip != null ) {

            TextView tvFPS = (TextView) findViewById(R.id.textview_transcoding_test_clip_fps);
            tvFPS.setText(String.format("FPS : %d", clip.getFramesPerSecond()));

            mDstWidth = clip.getWidth();
            mDstHeight = clip.getHeight();
            mDstDisplayWidth = clip.getDisplayWidth();
            mDstDisplayHeight = clip.getDisplayHeight();
            TextView tvResolution = (TextView) findViewById(R.id.textview_transcoding_test_clip_resolution);
            tvResolution.setText(String.format("Resolution : %d x %d", mDstWidth, mDstHeight));
        }

        SeekBar sbFPS = (SeekBar) findViewById(R.id.seekbar_transcoding_test_fps);
        sbFPS.setMax(120);
        sbFPS.setProgress(clip.getFramesPerSecond());
        sbFPS.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                TextView tvFPS = (TextView) findViewById(R.id.textview_transcoding_test_fps);
                tvFPS.setText(String.format("(%d)", value));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mDstFPS = value;
                TextView tvFPS = (TextView) findViewById(R.id.textview_transcoding_test_fps);
                tvFPS.setText(String.format("(%d)", value));
            }
        });


        mProject = new nexProject();
        mProject.add(clip);

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEditorListener);
        mEngin.setProject(mProject);
        mEngin.updateProject();

        Log.d(TAG, "mListFilePath 0: " + mListFilePath.get(0));

        Button btReverseStart = (Button)findViewById(R.id.button_transcoding_test_start);
        btReverseStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String srcPath = mListFilePath.get(0);
                mDstFilePath = getTranscodingDstFilePath();
                String temppath = getTranscodingTempFilePath();

                int width = mProject.getClip(0, true).getWidth();
                int height = mProject.getClip(0, true).getHeight();
                int bitrate = mProject.getClip(0, true).getVideoBitrate();
                int framerate = mProject.getClip(0, true).getFramesPerSecond();
//                        int bitrate = 6 * 1024 * 1024;
                long maxFileSize = Long.MAX_VALUE;

                Log.d(TAG, "=====================Transcoding StartInfo=====================");
                Log.d(TAG, "srcPath: " + srcPath);
                Log.d(TAG, "dstPath: " + mDstFilePath);
                Log.d(TAG, "temppath: " + temppath);
                Log.d(TAG, "width: " + width);
                Log.d(TAG, "height: " + height);
                Log.d(TAG, "bitrate: " + bitrate);
                Log.d(TAG, "framerate: " + framerate);
                Log.d(TAG, "maxFileSize: " + maxFileSize);

                Log.d(TAG, "DstWidth: " + mDstWidth);
                Log.d(TAG, "DstHeight: " + mDstHeight);
                Log.d(TAG, "DstWidth: " + mDstDisplayWidth);
                Log.d(TAG, "DstHeight: " + mDstDisplayHeight);
                Log.d(TAG, "DstFPS: " + mDstFPS);
                Log.d(TAG, "==========================================================");


                isTranscoding = true;
                mEngin.transcodingStart(srcPath, mDstFilePath, mDstWidth, mDstHeight, mDstDisplayWidth, mDstDisplayHeight, bitrate, maxFileSize, mDstFPS, 0);

                ((Button) findViewById(R.id.button_transcoding_test_start)).setEnabled(false);
                ((Button) findViewById(R.id.button_transcoding_test_stop)).setEnabled(true);
            }
        });

        Button btReverseStop = (Button) findViewById(R.id.button_transcoding_test_stop);
        btReverseStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isTranscoding = false;
                mEngin.transcodingStop();

            }
        });

        btReverseStop.setEnabled(false);

    }


    private String getTranscodingDstFilePath() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String reverse = "transcoding";
        String reverseDirPath = sdCardPath + File.separator + "KM" + File.separator + reverse;

        File exportDir = new File(reverseDirPath);
        exportDir.mkdirs();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String filePath = "NexEditor_" + reverse + "_" + export_time+".mp4";

        return reverseDirPath + File.separator + filePath;
    }

    private String getTranscodingTempFilePath() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String reversed = "transcoding";
        String reverseDirPath = sdCardPath + File.separator + "KM" + File.separator + reversed;

        File exportDir = new File(reverseDirPath);
        exportDir.mkdirs();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String filePath = "NexEditor_" + reversed + "_TEMP_" + export_time+".mp4";

        return reverseDirPath + File.separator + filePath;
    }

    @Override
    protected void onDestroy() {

        Log.d(TAG, "onDestroy");
        if(isTranscoding == true) {
            mEngin.transcodingStop();
        }
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
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
            Log.d(TAG, "iserror: " + iserror + ", result: " + result);

            if(result == nexEngine.nexErrorCode.TRANSCODING_USER_CANCEL.getValue() ||
                    result == nexEngine.nexErrorCode.CODEC_INIT.getValue() ||
                    result == nexEngine.nexErrorCode.FILEREADER_CREATE_FAIL.getValue())
            {
                Log.d(TAG, "Transcoding Stop");

                if(result == nexEngine.nexErrorCode.CODEC_INIT.getValue())
                    Toast.makeText(getApplicationContext(), "error: CODEC_INIT", Toast.LENGTH_SHORT).show();
                else if(result == nexEngine.nexErrorCode.FILEREADER_CREATE_FAIL.getValue())
                    Toast.makeText(getApplicationContext(), "error: FILEREADER_CREATE_FAIL", Toast.LENGTH_SHORT).show();

                ((Button) findViewById(R.id.button_transcoding_test_start)).setEnabled(true);
                ((Button) findViewById(R.id.button_transcoding_test_stop)).setEnabled(false);

                return ;
            }

            // if receive onEncodingDone in ReverseTestActivity, this event is ReverseDone

            // so remove srcClip and add dstClip
            mProject.remove(mProject.getClip(0, true));

            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mDstFilePath}, null, null);
            mProject.add(new nexClip(mDstFilePath));

            isTranscoding = false;

            ((Button) findViewById(R.id.button_transcoding_test_start)).setEnabled(true);
            ((Button) findViewById(R.id.button_transcoding_test_stop)).setEnabled(false);
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
            TextView tvProgress = (TextView)findViewById(R.id.textview_progress);
            tvProgress.setText(String.format("progress : %d %%", percent));
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
