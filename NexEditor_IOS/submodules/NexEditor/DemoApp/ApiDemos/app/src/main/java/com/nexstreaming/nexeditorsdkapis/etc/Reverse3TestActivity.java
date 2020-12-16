/******************************************************************************
 * File Name        : Reverse3TestActivity.java
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
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
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


public class Reverse3TestActivity extends Activity {
    public static final String TAG = "Reverse3Test";

    private ArrayList<String> mListFilePath;

    private nexEngineView mEditorView;
    private nexProject mProject;
    private nexEngine mEngin;

    private EditText m_StartEditText;
    private EditText m_EndEditText;

    private int mStartT = 0;
    private int mEndT = 0;
    private String mDstFilePath;

    private boolean isReversing = false;
    private int mDecodeMode = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_reverse3_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = getIntent().getStringArrayListExtra("filelist");

        nexClip clip = new nexClip(mListFilePath.get(0));
        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
            clip.setRotateDegree(clip.getRotateInMeta());
        }

        mEditorView = (nexEngineView) findViewById(R.id.engineview_reverse3);

        mProject = new nexProject();

        mProject.add(clip);

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEditorListener);
        mEngin.setView(mEditorView);
        mEngin.setProject(mProject);
        mEngin.updateProject();

        Log.d(TAG, "mListFilePath 0: " + mListFilePath.get(0));

        // UI init
        ((ProgressBar) findViewById(R.id.progress_reverse3)).setMax(100);

        Button btReverseStart = (Button)findViewById(R.id.button_reverse3_start);
        btReverseStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(false);
                try {
                    mStartT = Integer.parseInt(m_StartEditText.getText().toString());
                    mEndT = Integer.parseInt(m_EndEditText.getText().toString());
                } catch(NumberFormatException e) {
                    ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(true);
                    Toast.makeText(getApplicationContext(), "Please set the start & end values", Toast.LENGTH_SHORT).show();
                    return;
                }

                String srcPath = mListFilePath.get(0);
                mDstFilePath = getReverseDstFilePath();
                String temppath = getReverseTempFilePath();

                if (mProject.getTotalClipCount(true) == 0) {
                    nexClip clip = new nexClip(mListFilePath.get(0));
                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }
                    mProject.add(clip);
                }

                int width = mProject.getClip(0, true).getWidth();
                int height = mProject.getClip(0, true).getHeight();
                int bitrate = mProject.getClip(0, true).getVideoBitrate();
                //int bitrate = 6 * 1024 * 1024;
                long maxFileSize = Long.MAX_VALUE;

                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(false);

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

                if(mStartT >= mEndT || mEndT - mStartT < 500) {
                    Toast.makeText(getApplicationContext(), "endTime - startTime <= 1000, retry again!!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                isReversing = true;
                mEngin.reverseStart(srcPath, mDstFilePath, temppath, width, height, bitrate, maxFileSize, mStartT, mEndT, mDecodeMode);

                ((ProgressBar) findViewById(R.id.progress_reverse3)).setProgress(0);
            }
        });

        m_StartEditText = (EditText) findViewById(R.id.edittext_reverse3_start);
        m_EndEditText = (EditText) findViewById(R.id.edittext_reverse3_end);

        Button btReversePlay = (Button) findViewById(R.id.button_reverse3_play);
        btReversePlay.setEnabled(false);
        btReversePlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }
                ((Button)findViewById(R.id.button_reverse3_play)).setEnabled(false);
                mEngin.stop();
                mEngin.setView(mEditorView);
                mEngin.play();
            }
        });

        Button btReverseExport = (Button) findViewById(R.id.button_reverse3_export);
        btReverseExport.setEnabled(false);
        btReverseExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }
                mEngin.stop();
                mEngin.setView(mEditorView);

                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);

            }
        });

        Button btReverseClear = (Button) findViewById(R.id.button_reverse3_clear);
        btReverseClear.setEnabled(false);
        btReverseClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                mProject.allClear(true);
                mEngin.updateProject();

                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(true);
                ((Button)findViewById(R.id.button_reverse3_export)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse3_clear)).setEnabled(false);

                deleteTempFile();

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


    private boolean deleteTempFile() {
        boolean bRet = false;
        try {
            if (!mDstFilePath.equals(null)) {
                File file = new File(mDstFilePath);
                if (file != null && file.delete())
                    bRet = true;
            }
        } catch(NullPointerException e) {
            Log.d(TAG, "deleteTempFile NullPointerException");
        }
        return bRet;
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        ApiDemosConfig.getApplicationInstance().getEngine();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        mEngin.setEventHandler(mEditorListener);
        super.onResume();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop");
        ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(true);
        ((Button)findViewById(R.id.button_reverse3_play)).setEnabled(false);
        ((Button)findViewById(R.id.button_reverse3_export)).setEnabled(false);
        ((Button)findViewById(R.id.button_reverse3_clear)).setEnabled(false);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        deleteTempFile();
        mEngin.stop();
        super.onDestroy();
        Log.d(TAG, "onDestroy");
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
            Log.d(TAG, "iserror: " + iserror + ", result: " + result);

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

                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(true);
                ((ProgressBar) findViewById(R.id.progress_reverse3)).setProgress(0);
                deleteTempFile();
                return ;
            }

            if(isReversing == true) {
                int iCount = 0;

                isReversing = false;
                mEngin.reverseStop();

                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse3_play)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse3_export)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse3_clear)).setEnabled(false);

                mProject.getClip(iCount, true).getVideoClipEdit().setTrim(mStartT, mEndT);
                iCount++;

                nexClip nClip1 = nexClip.dup(mProject.getClip(0, true));
                if( nClip1!= null ) {
                    if (nClip1.getRotateInMeta() == 270 || nClip1.getRotateInMeta() == 90)
                        nClip1.setRotateDegree(nClip1.getRotateInMeta());

                    mProject.add(nClip1);
                    mProject.getClip(iCount, true).getVideoClipEdit().setTrim(mStartT, mEndT);
                    mProject.getClip(iCount, true).getVideoClipEdit().setSpeedControl(50); //	1/2x
                    iCount++;
                }

                File files = new File(mDstFilePath);
                nexClip nDstClip;
                if(files.exists()==true) {
                    nDstClip = new nexClip(mDstFilePath);
                    if( nDstClip!= null ) {
                        if (nDstClip.getTotalTime() > 0) {
                            if (nDstClip.getRotateInMeta() == 270 || nDstClip.getRotateInMeta() == 90)
                                nDstClip.setRotateDegree(nDstClip.getRotateInMeta());

                            mProject.add(nDstClip);
                            iCount++;
                        } else {
                            mEngin.stop();
                            mProject.allClear(true);
                            mEngin.updateProject();

                            ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(true);
                            ((Button)findViewById(R.id.button_reverse3_export)).setEnabled(false);
                            ((Button)findViewById(R.id.button_reverse3_clear)).setEnabled(false);

                            deleteTempFile();
                            Log.e(TAG," Reverse Fail(Result file did not exist)");
                            Toast.makeText(getApplicationContext(), "Reverse Fail", Toast.LENGTH_SHORT).show();
                            return;
                        }
                    }
                }

                nexClip nClip2 = nexClip.dup(mProject.getClip(0, true));
                if( nClip2!= null ) {
                    if (nClip2.getRotateInMeta() == 270 || nClip2.getRotateInMeta() == 90)
                        nClip2.setRotateDegree(nClip2.getRotateInMeta());

                    mProject.add(nClip2);
                    mProject.getClip(iCount, true).getVideoClipEdit().setTrim(mStartT, mEndT);
                    mProject.getClip(iCount, true).getVideoClipEdit().setSpeedControl(13); //  1/8x
                    iCount++;
                }

                nexClip nClip3 = nexClip.dup(mProject.getClip(0, true));
                if( nClip3!= null ) {
                    if (nClip3.getRotateInMeta() == 270 || nClip3.getRotateInMeta() == 90)
                        nClip3.setRotateDegree(nClip3.getRotateInMeta());

                    mProject.add(nClip3);
                    mProject.getClip(iCount, true).getVideoClipEdit().setTrim(mStartT, mEndT);
                    iCount++;
                }

                mProject.setProjectAudioFadeInTime(0);
                mProject.setProjectAudioFadeOutTime(0);

                mEngin.setProject(mProject);
                mEngin.updateProject();
                mEngin.play();

                ((ProgressBar) findViewById(R.id.progress_reverse3)).setProgress(0);
                ((Button)findViewById(R.id.button_reverse3_start)).setEnabled(false);
                ((Button)findViewById(R.id.button_reverse3_export)).setEnabled(true);
                ((Button)findViewById(R.id.button_reverse3_clear)).setEnabled(true);
            }
        }

        @Override
        public void onPlayEnd() {
            Log.d(TAG," playEnd");
            ((Button)findViewById(R.id.button_reverse3_play)).setEnabled(true);
            Toast.makeText(getApplicationContext(), "onPlayEnd", Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onPlayFail(int err, int iClipID) {
            Toast.makeText(getApplicationContext(), "onPlayFail", Toast.LENGTH_SHORT).show();
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
            ((ProgressBar) findViewById(R.id.progress_reverse3)).setProgress(percent);
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
