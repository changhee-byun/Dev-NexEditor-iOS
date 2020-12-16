/******************************************************************************
 * File Name        : ClipTrimTestActivity.java
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
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
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
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;


public class ClipTrimTestActivity extends Activity {
    private static final String TAG = "ClipTrimTestActivity";
    private nexEngine mEngine;
    private nexProject mProject;
    private nexEngineView m_editorView;
    private nexEngineListener mEditorListener = null;

    private ProjectEditItemAdapter mItemAdp;
    private SeekBar mStartSeekBar;
    private TextView mStartTextview;
    private SeekBar mEndSeekBar;
    private TextView mEndTextview;
    private Spinner mSpinnerSpeed1;
    private Button btplay;
    private Button mExport;
    private Button mButtonDirectExport;
    private TextView mTextViewDirectExport;
    private ProgressBar mProgressBarDirectExport;

    private EditText mEdittextStart;
    private EditText mEdittextEnd;
    private int index;
    private int starttrimtime;
    private int endtrimtime;
    private boolean isPlay;
    private Switch mSwitchIDRSeek;

    private File mFile;
    private Long mTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_clip_trim_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mProject = new nexProject();
        isPlay = false;
        mStartSeekBar = (SeekBar) findViewById(R.id.seekbar_clip_trim_test_start);

        mStartSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            private int seek;
            private boolean isSeeking = false;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }
                if (isSeeking) {
                    seek = progress * 33;
                    if (!mSwitchIDRSeek.isChecked()) {
                        mStartTextview.setText(String.valueOf(progress * 33));
                        mEdittextStart.setText(String.valueOf(progress * 33));
                        starttrimtime = seek;
                        Log.d(TAG, "starttrimtime = " + starttrimtime);

                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }

                if (!isSeeking) {
                    mEngine.pause();
                    isSeeking = true;
                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }
            }
        });
        mStartTextview = (TextView) findViewById(R.id.textview_clip_trim_test_starttrimtime);
        mEdittextStart = (EditText) findViewById(R.id.edittext_clip_test_starttrimtime);
        mEdittextStart.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                try {
                    if(UtilityCode.isNumeric(s.toString())) {
                        if (Integer.parseInt(s.toString()) <= mProject.getClip(0, true).getTotalTime()) {
                            mStartSeekBar.setProgress(Integer.parseInt(s.toString()) / 33);
                            mStartTextview.setText(s);
                            starttrimtime = Integer.parseInt(s.toString());
                        }
                    } else if (s.length() == 0) {
                        mStartSeekBar.setProgress(0);
                        mStartTextview.setText("0");
                        starttrimtime = 0;
                    }
                } catch (NumberFormatException e) {
                    Log.e(TAG, e.toString());
                }

                Log.d(TAG, "onTextChanged: " + s);
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });



        mEndSeekBar = (SeekBar) findViewById(R.id.seekbar_clip_trim_test_end);

        mEndSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            private int seek;
            private boolean isSeeking = false;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }

                if (isSeeking) {
                    seek = progress * 33;
                    if (!mSwitchIDRSeek.isChecked()) {
                        mEndTextview.setText(String.valueOf(progress * 33));
                        mEdittextEnd.setText(String.valueOf(progress * 33));
                        endtrimtime = seek;
                        Log.d(TAG, "endtrimtime = " + endtrimtime);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }

                if (!isSeeking) {
                    mEngine.pause();
                    isSeeking = true;
                }
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }
            }
        });
        mEndTextview = (TextView) findViewById(R.id.textview_clip_trim_test_endtrimtime);
        mEdittextEnd = (EditText) findViewById(R.id.edittext_clip_test_endtrimtime);
        mEdittextEnd.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                try {
                    if(UtilityCode.isNumeric(s.toString())) {
                        if (Integer.parseInt(s.toString()) <= mProject.getClip(0, true).getTotalTime()) {
                            mEndSeekBar.setProgress(Integer.parseInt(s.toString()) / 33);
                            mEndTextview.setText(s);
                            endtrimtime = Integer.parseInt(s.toString());
                        }
                    } else if (s.length() == 0) {
                        mEndSeekBar.setProgress(0);
                        mEndTextview.setText("0");
                        endtrimtime = 0;
                    }
                } catch (NumberFormatException e) {
                    Log.e(TAG, e.toString());
                }

                Log.d(TAG, "onTextChanged: " + s);
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });
        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {
                Log.d(TAG, "onTimeChange = " + currentTime);
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                Log.d(TAG, "onSetTimeDone = " + currentTime);
            }

            @Override
            public void onSetTimeFail(int err) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean iserror, int result) {
                if (iserror) {
                    Log.i(TAG, "Error result=" + result);
                    if (mFile != null) {
                        mFile.delete();
                        mFile = null;
                    }
                    // TODO ; Error
                    Log.d(TAG, "Export Fail!");
                    return;
                } else {
                    if (mFile != null) {
                        // To update files at Gallery
                        mProgressBarDirectExport.setProgress(100);
                        mTextViewDirectExport.setText("100");
                        Log.d(TAG, "To update files at Gallery" + result);
                        Long timediff = System.currentTimeMillis() - mTime;
                        Log.d(TAG, "spend time =" + timediff);
                        Toast.makeText(ClipTrimTestActivity.this, "spend time =" + timediff, Toast.LENGTH_LONG).show();
                        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                    }
                    for(int i = 0; i<mProject.getTotalClipCount(true); i++){
                        mProject.getClip(i, true).getVideoClipEdit().setSpeedControl(100);
                    }
                    if(mProject.getTotalClipCount(true) > 1) {
                        mProject.getClip(0, true).getVideoClipEdit().clearTrim();
                        mProject.getClip(1, true).getVideoClipEdit().clearTrim();
                        mEngine.updateProject();
                        mProject.getClip(0, true).getVideoClipEdit().setTrim(starttrimtime, endtrimtime);
                        mProject.getClip(1, true).getVideoClipEdit().setTrim(endtrimtime, mProject.getClip(1, true).getTotalTime());
                    }
                }
            }


            @Override
            public void onPlayEnd() {
                mEngine.stop();
                isPlay = false;
                btplay.setText("Play");
//                        if(mProject.getTotalClipCount(true) > 1) {
//                            mProject.getClip(0, true).getVideoClipEdit().clearTrim();
//                            mProject.getClip(1, true).getVideoClipEdit().clearTrim();
//                            mEngine.updateProject();
//                            mProject.getClip(0, true).getVideoClipEdit().setTrim(starttrimtime, endtrimtime);
//                            mProject.getClip(1, true).getVideoClipEdit().setTrim(endtrimtime, mProject.getClip(1, true).getTotalTime());
//                        }
            }

            @Override
            public void onPlayFail(int err, int iClipID) {
                Log.d(TAG, "onPlayFail : err=" + err + " iClipID=" + iClipID);
            }

            @Override
            public void onPlayStart() {
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
                Log.d(TAG, "onEncodingProgress: " + percent);
                mTextViewDirectExport.setText(percent + "");
                mProgressBarDirectExport.setProgress(percent);

            }

            @Override
            public void onCheckDirectExport(int result) {
                Log.d(TAG, "onCheckDirectExport(" + result + ") ");

                if (result == 0) {
                    Toast.makeText(ClipTrimTestActivity.this, "direct export start!!", Toast.LENGTH_SHORT).show();

                    mEngine.directExport(mFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime(), 0);
                } else
                    Toast.makeText(ClipTrimTestActivity.this, "direct export can`t start checkexport: " + result, Toast.LENGTH_SHORT).show();

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

        btplay = (Button) findViewById(R.id.button_clip_trim_test_play);
        btplay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mProject.getTotalClipCount(true) == 0) {
                    Toast.makeText(getApplicationContext(), "clip count is 0", Toast.LENGTH_SHORT).show();
                    return ;
                }
                if (isPlay) {
                    mEngine.stop();
                    isPlay = false;
                    btplay.setText("Play");
                } else {
                    mEngine.setView(m_editorView);
                    setData();
                    isPlay = true;
                    mEngine.play();
                    btplay.setText("Stop");
                }
            }
        });

        Button btadd = (Button) findViewById(R.id.button_clip_trim_test_add);
        btadd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(isPlay == true)
                {
                    Toast.makeText(getApplicationContext(), "retry after playing...", Toast.LENGTH_LONG).show();
                    return;
                }

                if (mProject.getTotalClipCount(true) == 0) {
                    Intent intent = new Intent();
                    intent.setType("video/*");
                    intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                    intent.setAction(Intent.ACTION_GET_CONTENT);
                    startActivityForResult(intent, 100);
                }
                else
                {
                    Toast.makeText(getApplicationContext(), "content is already added to project.", Toast.LENGTH_SHORT).show();
                    return ;
                }
            }
        });

        Button bttrim = (Button) findViewById(R.id.button_clip_trim_test_trim);
        bttrim.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(isPlay == true)
                {
                    Toast.makeText(getApplicationContext(), "retry after playing...", Toast.LENGTH_SHORT).show();
                    return ;
                }

                mEngine.stop();

                if( mProject.getTotalVisualClipCount() <= 0 )
                {
                    Toast.makeText(getApplicationContext(), "Video clip was not selected", Toast.LENGTH_SHORT).show();
                    return;
                }




                if ((endtrimtime - starttrimtime > 500)) {

                    mProject.getClip(0, true).getVideoClipEdit().setTrim(starttrimtime, endtrimtime);
                } else {
                    Toast.makeText(getApplicationContext(), "Trim duration is small in 500ms", Toast.LENGTH_SHORT).show();
                }

                m_editorView.setBlackOut(true);
            }
        });

        mSwitchIDRSeek = (Switch) findViewById(R.id.switch_clip_trim_test_idrseek);
        mSwitchIDRSeek.setChecked(false);
        mSwitchIDRSeek.setEnabled(false);

        m_editorView = (nexEngineView) findViewById(R.id.engineview_clip_trim_test);
        m_editorView.setBlackOut(true);

        Integer[] items = new Integer[]{12, 25, 50, 75, 100, 125, 150, 175, 200, 400};
        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);

        mSpinnerSpeed1 = (Spinner) findViewById(R.id.spinner_clip_trim_test_speed1);

        mSpinnerSpeed1.setAdapter(adapter);
        mSpinnerSpeed1.setSelection(4);
        mSpinnerSpeed1.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.stop();
                isPlay = false;
                btplay.setText("Play");
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mExport = (Button) findViewById(R.id.button_clip_trim_test_export);
        mExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }

                setData();
                nexClip clip = mProject.getClip(0, true);
                mEngine.stop();
                mEngine.setView(m_editorView);
                mProgressBarDirectExport.setMax(100);
                mProgressBarDirectExport.setProgress(0);
                mTextViewDirectExport.setText(String.valueOf(0));

                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });

        mProgressBarDirectExport = (ProgressBar) findViewById(R.id.progressbar_trim_test_direct_export);
        mProgressBarDirectExport.setMax(100);
        mProgressBarDirectExport.setProgress(0);

        mTextViewDirectExport = (TextView) findViewById(R.id.textview_trim_test_drecit_export);

        mButtonDirectExport = (Button) findViewById(R.id.button_trim_test_direct_export);
        mButtonDirectExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonDirectExport Click!!");
                if(mProject.getTotalClipCount(true) == 0) {
                    return ;
                }

                setData();

                mEngine.stop();
                mEngine.setView(m_editorView);
                mTime = System.currentTimeMillis();
                mFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), 1);

                mProgressBarDirectExport.setProgress(0);
                mTextViewDirectExport.setText("0");

                Log.d(TAG, "mFile Path: " + mFile.getAbsolutePath());

                mEngine.checkDirectExport(0);
            }
        });


        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setEventHandler(mEditorListener);
        mEngine.setProject(mProject);
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
        mTime = System.currentTimeMillis();
        File exportFile = new File(exportDir, "KineMaster_"+wid+"X"+hei +"_"+ export_time+".mp4");
        return exportFile;
    }

    private void setData() {
        Integer i =  (Integer) mSpinnerSpeed1.getSelectedItem();
        mEngine.getProject().getClip(0,true).getVideoClipEdit().setSpeedControl(i.intValue());
        

        mEngine.updateProject();
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
                mEngine.updateProject();
                mStartSeekBar.setMax(mProject.getTotalTime() / 33);
                mStartSeekBar.setProgress(0);
                mEndSeekBar.setMax(mProject.getTotalTime() / 33);
                mEndSeekBar.setProgress(0);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);


                    nexClip clip = nexClip.getSupportedClip(filepath);
                    if( clip != null ) {
                        if (mProject.getTotalClipCount(true) !=0)
                            mProject.allClear(true);

                        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                            clip.setRotateDegree(clip.getRotateInMeta());
                        }

                        mProject.add(new nexClip(filepath));
                        mEngine.updateProject();
                        mStartSeekBar.setMax(mProject.getTotalTime() / 33);
                        mStartSeekBar.setProgress(0);
                        mEndSeekBar.setMax(mProject.getTotalTime() / 33);
                        mEndSeekBar.setProgress(0);
                    }else{
                        Toast.makeText(ClipTrimTestActivity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                    }
                }
            }
        }
    }

    @Override
    protected void onStop() {
        mEngine.stop();
        super.onStop();
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        ApiDemosConfig.getApplicationInstance().getEngine();
        if(mEditorListener != null)
            mEngine.setEventHandler(mEditorListener);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mEngine.stop();
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        if(mProject!=null) {
            for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                mProject.getClip(i, true).getVideoClipEdit().setSpeedControl(100);
            }
            mEngine.updateProject();
        }
        super.onResume();
    }
}
