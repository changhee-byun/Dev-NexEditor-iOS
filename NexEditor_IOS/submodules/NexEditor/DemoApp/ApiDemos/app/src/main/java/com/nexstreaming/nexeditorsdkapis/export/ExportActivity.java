/******************************************************************************
 * File Name        : ExportActivity.java
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

package com.nexstreaming.nexeditorsdkapis.export;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.module.nexExternalExportProvider;
import com.nexstreaming.nexeditorsdk.module.nexModuleProvider;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexExportFormat;
import com.nexstreaming.nexeditorsdk.nexExportFormatBuilder;
import com.nexstreaming.nexeditorsdk.nexExportListener;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;


public class ExportActivity extends Activity {
    private enum State {
        NONE(0),
        IDLE(1),
        RUN(2),
        RECORD(3),
        PAUSE(4),
        RESUME(5);

        int value;
        State(int v) {
            value = v;
        }

        int getValue() {
            return value;
        }
    }

    public static final int AVCProfileBaseline	= 0x01;
    public static final int AVCProfileMain	= 0x02;
    public static final int AVCProfileExtended	= 0x03;
    public static final int AVCProfileHigh	= 0x04;
    public static final int AVCProfileHigh10	= 0x5;
    public static final int AVCProfileHigh422	= 0x6;
    public static final int AVCProfileHigh444	= 0x7;

    public static final int AVCLevel1		= 0x01;
    public static final int AVCLevel1b		= 0x02;
    public static final int AVCLevel11		= 0x04;
    public static final int AVCLevel12		= 0x08;
    public static final int AVCLevel13		= 0x10;
    public static final int AVCLevel2		= 0x20;
    public static final int AVCLevel21		= 0x40;
    public static final int AVCLevel22		= 0x80;
    public static final int AVCLevel3		= 0x100;
    public static final int AVCLevel31		= 0x200;
    public static final int AVCLevel32		= 0x400;
    public static final int AVCLevel4		= 0x800;
    public static final int AVCLevel41		= 0x1000;
    public static final int AVCLevel42		= 0x2000;
    public static final int AVCLevel5		= 0x4000;
    public static final int AVCLevel51		= 0x8000;
    public static final int AVCLevel52		= 0x10000;

    private static final String TAG = "ExportActivity";
    private nexEngine mEngine;
    private nexEngineListener mExportListener = null;
    private File mFile;


    private Spinner mResolutionSpinner;
    private Spinner mVideoBitrate1Spinner;
    private Spinner mFPSSpinner;
    private Spinner mSamplingRateSpinner;
    private Spinner mProfileSpinner;
    private Spinner mLevelSpinner;
    private Spinner mCodecSpinner;
    private Spinner mExportModuleSpinner;

    private HashMap<String, Integer> profilemap;
    private HashMap<String, Integer> levelmap;
    private HashMap<String, Integer> codecmap;

    private ArrayAdapter<String> adapterProfile;
    private ArrayAdapter<String> adapterLevele;

    private List<nexModuleProvider> imageExports;

    private ProgressBar mProgressBar;

    private Button mExportButton;
    private Button mExportStopButton;
    private Button mExportSaveStopButton;
    private Button mPauseResumeButton;
    private int mAspectMode = 1;
    private int mPreviewFaceMode = 0;

    private boolean mExportCancel;
    private State mState = State.IDLE;

    private int mEngineState = nexEngine.nexPlayState.NONE.getValue();
    ProgressDialog progressDlg=null;

    private boolean mAutoPauseResume;

    private Rect[] oriStartRect;
    private Rect[] oriEndRect;

    private Stopwatch watch = new Stopwatch();

    private class ExportResolution{
        public String display;
        public int width;
        public int height;
        public int video_bitrate;
    }

    private List<ExportResolution> mExportResolution;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_export);

        Intent intent = getIntent();
        mPreviewFaceMode = intent.getIntExtra("FaceMode", 0);
        mAutoPauseResume = intent.getBooleanExtra("AutoPauseResume",false);
        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Log.d(TAG, "ExportActivity start!");

        profilemap = new HashMap<>();
        levelmap = new HashMap<>();
        codecmap = new HashMap<>();
        profilemap.put("AVCProfileBaseline",nexEngine.ExportProfile_AVCBaseline);
        profilemap.put("AVCProfileMain",nexEngine.ExportProfile_AVCMain);
        profilemap.put("AVCProfileExtended",nexEngine.ExportProfile_AVCExtended);
        profilemap.put("AVCProfileHigh",nexEngine.ExportProfile_AVCHigh);
        profilemap.put("AVCProfileHigh10",nexEngine.ExportProfile_AVCHigh10);
        profilemap.put("AVCProfileHigh422",nexEngine.ExportProfile_AVCHigh422);
        profilemap.put("AVCProfileHigh444",nexEngine.ExportProfile_AVCHigh444);
        profilemap.put("HEVCProfileMain", nexEngine.ExportProfile_HEVCMain);
        profilemap.put("HEVCProfileMain10", nexEngine.ExportProfile_HEVCMain10);
        profilemap.put("MPEG4VSimple", nexEngine.ExportProfile_MPEG4VSimple);
        profilemap.put("MPEG4VASP", nexEngine.ExportProfile_MPEG4VASP);

        levelmap.put("AVCLevel1",nexEngine.ExportAVCLevel1);
        levelmap.put("AVCLevel1b",nexEngine.ExportAVCLevel1b);
        levelmap.put("AVCLevel11",nexEngine.ExportAVCLevel11);
        levelmap.put("AVCLevel12",nexEngine.ExportAVCLevel12);
        levelmap.put("AVCLevel13",nexEngine.ExportAVCLevel13);
        levelmap.put("AVCLevel2",nexEngine.ExportAVCLevel2);
        levelmap.put("AVCLevel21",nexEngine.ExportAVCLevel21);
        levelmap.put("AVCLevel22",nexEngine.ExportAVCLevel22);
        levelmap.put("AVCLevel3",nexEngine.ExportAVCLevel3);
        levelmap.put("AVCLevel31",nexEngine.ExportAVCLevel31);
        levelmap.put("AVCLevel32",nexEngine.ExportAVCLevel32);
        levelmap.put("AVCLevel4",nexEngine.ExportAVCLevel4);
        levelmap.put("AVCLevel41",nexEngine.ExportAVCLevel41);
        levelmap.put("AVCLevel42",nexEngine.ExportAVCLevel42);
        levelmap.put("AVCLevel5",nexEngine.ExportAVCLevel5);
        levelmap.put("AVCLevel51",nexEngine.ExportAVCLevel51);
        levelmap.put("AVCLevel52",nexEngine.ExportAVCLevel52);

        levelmap.put("HEVCMainTierLevel1", nexEngine.ExportHEVCMainTierLevel1);
        levelmap.put("HEVCHighTierLevel1", nexEngine.ExportHEVCHighTierLevel1);
        levelmap.put("HEVCMainTierLevel2", nexEngine.ExportHEVCMainTierLevel2);
        levelmap.put("HEVCHighTierLevel2", nexEngine.ExportHEVCHighTierLevel2);
        levelmap.put("HEVCMainTierLevel21", nexEngine.ExportHEVCMainTierLevel21);
        levelmap.put("HEVCHighTierLevel21", nexEngine.ExportHEVCHighTierLevel21);
        levelmap.put("HEVCMainTierLevel3", nexEngine.ExportHEVCMainTierLevel3);
        levelmap.put("HEVCHighTierLevel3 ", nexEngine.ExportHEVCHighTierLevel3 );
        levelmap.put("HEVCMainTierLevel31", nexEngine.ExportHEVCMainTierLevel31);
        levelmap.put("HEVCHighTierLevel31", nexEngine.ExportHEVCHighTierLevel31);
        levelmap.put("HEVCMainTierLevel4", nexEngine.ExportHEVCMainTierLevel4);
        levelmap.put("HEVCHighTierLevel4", nexEngine.ExportHEVCHighTierLevel4);
        levelmap.put("HEVCMainTierLevel41", nexEngine.ExportHEVCMainTierLevel41);
        levelmap.put("HEVCHighTierLevel41", nexEngine.ExportHEVCHighTierLevel41);
        levelmap.put("HEVCMainTierLevel5", nexEngine.ExportHEVCMainTierLevel5);
        levelmap.put("HEVCHighTierLevel5", nexEngine.ExportHEVCHighTierLevel5);
        levelmap.put("HEVCMainTierLevel51", nexEngine.ExportHEVCMainTierLevel51);
        levelmap.put("HEVCHighTierLevel51", nexEngine.ExportHEVCHighTierLevel51);
        levelmap.put("HEVCMainTierLevel52", nexEngine.ExportHEVCMainTierLevel52);
        levelmap.put("HEVCHighTierLevel52", nexEngine.ExportHEVCHighTierLevel52);
        levelmap.put("HEVCMainTierLevel6", nexEngine.ExportHEVCMainTierLevel6);
        levelmap.put("HEVCHighTierLevel6", nexEngine.ExportHEVCHighTierLevel6);
        levelmap.put("HEVCMainTierLevel61", nexEngine.ExportHEVCMainTierLevel61);
        levelmap.put("HEVCHighTierLevel61", nexEngine.ExportHEVCHighTierLevel61);
        levelmap.put("HEVCMainTierLevel62", nexEngine.ExportHEVCMainTierLevel62);
        levelmap.put("HEVCHighTierLevel62", nexEngine.ExportHEVCHighTierLevel62);

        levelmap.put("MPEG4Level0", nexEngine.ExportMPEG4Level0);
        levelmap.put("MPEG4Level0b", nexEngine.ExportMPEG4Level0b);
        levelmap.put("MPEG4Level1", nexEngine.ExportMPEG4Level1);
        levelmap.put("MPEG4Level2", nexEngine.ExportMPEG4Level2);
        levelmap.put("MPEG4Level3", nexEngine.ExportMPEG4Level3);
        levelmap.put("MPEG4Level4", nexEngine.ExportMPEG4Level4);
        levelmap.put("MPEG4Level4a", nexEngine.ExportMPEG4Level4a);
        levelmap.put("MPEG4Level5", nexEngine.ExportMPEG4Level5);

        codecmap.put("AVC",nexEngine.ExportCodec_AVC);
        codecmap.put("HEVC",nexEngine.ExportCodec_HEVC);
        codecmap.put("MPEG4V", nexEngine.ExportCodec_MPEG4V);

        mProgressBar = (ProgressBar) findViewById(R.id.progressbar_export);
        mProgressBar.setProgress(0);
        mProgressBar.setMax(100);

        mExportButton = (Button) findViewById(R.id.button_direct_export_test_export);
        mExportButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                watch.reset();
                watch.start();
                if(mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
                    Log.d(TAG, "State is " + mState);
                    Toast.makeText(getApplicationContext(), "already exporting..." , Toast.LENGTH_SHORT).show();
                    return ;
                }
                if(mEngineState != nexEngine.nexPlayState.IDLE.getValue()) {
                    Log.d(TAG, "Invalid engine state! State is " + mEngineState);
                    Toast.makeText(getApplicationContext(), "Engine is running! Try again when engine is stopped." , Toast.LENGTH_SHORT).show();
                    return ;
                }

                int iSelResolution = mResolutionSpinner.getSelectedItemPosition();

                if( mExportResolution.size() < iSelResolution )
                {
                    Toast.makeText(getApplicationContext(), "Resolution not selected" , Toast.LENGTH_LONG).show();
                    return ;
                }

                ExportResolution er = mExportResolution.get(iSelResolution);

                int width = er.width;
                int height = er.height;

                if( er.display.equals("Original resolution") )
                {
                    int baseIndex=0;
                    for(int i = 0; i < mEngine.getProject().getTotalClipCount(true); i++) {
                        if(mEngine.getProject().getClip(i, true).isAssetResource() == true) {
                            if ( baseIndex == i && mEngine.getProject().getTotalClipCount(true)>(baseIndex+1) ) {
                                baseIndex++;
                            }
                            continue;
                        }
                        if(mEngine.getProject().getClip(baseIndex, true).getWidth() != mEngine.getProject().getClip(i, true).getWidth() ||
                                mEngine.getProject().getClip(baseIndex, true).getHeight() != mEngine.getProject().getClip(i, true).getHeight()){
                            Toast.makeText(getBaseContext(), "Contents resolution is different", Toast.LENGTH_LONG).show();
                            return;
                        }
                    }

                    width = mEngine.getProject().getClip(0, true).getWidth();
                    height = mEngine.getProject().getClip(0, true).getHeight();

                    //Aspect Ratio check
                    if ( nexApplicationConfig.getAspectRatio() != (float)width/(float)height ) {
                        Toast.makeText(getApplicationContext(), String.format("Invalid export resolution! Ratio="+nexApplicationConfig.getAspectRatio()+", Width/Height="+(float)width/(float)height) , Toast.LENGTH_LONG).show();
                        return;
                    }
                }

                mState = State.RECORD;
                mProgressBar.setProgress(0);

                if( ExternalExport(width,height) == 1 ){
                    return;
                }

                Integer samplingRate =  (Integer) mSamplingRateSpinner.getSelectedItem();
                int fps = (int)((Float)mFPSSpinner.getSelectedItem()*100);
                int iBitrate = (int)((float)(mVideoBitrate1Spinner.getSelectedItem()) * 1024 * 1024);
                String profile =  (String) mProfileSpinner.getSelectedItem();
                String level =  (String) mLevelSpinner.getSelectedItem();
                String videocodectype =  (String) mCodecSpinner.getSelectedItem();

                int profile_value = profilemap.get(profile);
                int level_value = levelmap.get(level);
                int codec_value = codecmap.get(videocodectype);
                mFile = getExportFile(width, height, samplingRate,"mp4");

                Toast.makeText(getApplicationContext(), String.format("Export start with(%d %d %d %d %d %d %d)", width, height, iBitrate, samplingRate, fps , profile_value , level_value) , Toast.LENGTH_LONG).show();
                Log.d(TAG, "exporting infor : codec="+videocodectype+codec_value+", profile="+profile+profile_value+", level="+level+level_value);

                int ret = mEngine.faceDetect(true , 1 , nexEngine.nexUndetectedFaceCrop.fromValue(mPreviewFaceMode));
                if(ret ==0){
                    Log.d(TAG, "Disable FaceDetector");
                }else{
                    Log.d(TAG, "Enable FaceDetector");
                }
                int iRet = mEngine.exportNoException(mFile.getAbsolutePath(), width, height, iBitrate, Long.MAX_VALUE, 0, samplingRate, profile_value, level_value, fps, codec_value);
                if( iRet == 0 )
                {
                } else if ( iRet == -1 ) {
                    Toast.makeText(getApplicationContext(), "Already exported", Toast.LENGTH_SHORT).show();

                } else if ( iRet == -2 ) {
                    Toast.makeText(getApplicationContext(), "Effect error while export", Toast.LENGTH_SHORT).show();
                    mEngine.stop();
                    mState = State.IDLE;
                    finish();
                } else {
                    Toast.makeText(getApplicationContext(), "Export error : " + iRet, Toast.LENGTH_SHORT).show();
                }
            }
        });

        mExportStopButton = (Button) findViewById(R.id.button_direct_export_test_export_stop);
        mExportStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine != null) {
                    if (mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
                        mExportCancel = true;
                        mEngine.stop();
                    }else{
                        finish();
                    }
                }
            }
        });

        mExportSaveStopButton = (Button) findViewById(R.id.button_export_test_export_save_stop);
        mExportSaveStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine != null) {
                    if (mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
                        mExportCancel = false;
                        mEngine.exportSaveStop(new nexEngine.OnCompletionListener() {
                            @Override
                            public void onComplete(int resultCode) {
                                Log.d(TAG, "Save export : " + resultCode);
                            }
                        });
                    }else{
                        finish();
                    }
                }
            }
        });


        mPauseResumeButton = (Button) findViewById(R.id.button_export_test_pause_resume);
        if( mAutoPauseResume ){
            mPauseResumeButton.setVisibility(View.INVISIBLE);
        }
        //mPauseResumeButton.setEnabled(false);
        mPauseResumeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPauseResumeButton.setEnabled(false);
                mExportSaveStopButton.setEnabled(false);
                mExportStopButton.setEnabled(false);
                mExportButton.setEnabled(false);
                if (mState == State.RECORD || mState == State.RESUME) {
                    mEngine.exportPause();
                    mState = State.PAUSE;
                    mPauseResumeButton.setText("Resume");
                } else if (mState == State.PAUSE) {
                    mEngine.exportResume();
                    mState = State.RESUME;
                    mPauseResumeButton.setText("Pause");
                }
                mPauseResumeButton.setEnabled(true);
                mExportSaveStopButton.setEnabled(true);
                mExportStopButton.setEnabled(true);
                mExportButton.setEnabled(true);
            }
        });


        mExportResolution = new ArrayList<>();
        mAspectMode  = nexApplicationConfig.getAspectRatioMode();

        for(int i = 0; i < 5; i++ )
        {
            mExportResolution.add(getExportResolution(i));
        }

        ExportResolution item = new ExportResolution();
        item.width = 0;
        item.height = 0;
        item.video_bitrate = 4;
        item.display = "Original resolution";
        mExportResolution.add(item);

        mExportModuleSpinner = (Spinner)findViewById(R.id.spinner_export_module);

        List<String> module_items = new ArrayList<String>();
        module_items.add("internal");
        imageExports = nexApplicationConfig.getExternalModuleManager().getModules(nexExternalExportProvider.class);
        for( nexModuleProvider m : imageExports ){
            module_items.add("external-"+m.name());
        }

        ArrayAdapter<String> module_adapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, module_items);
        mExportModuleSpinner.setAdapter(module_adapter);

        mSamplingRateSpinner = (Spinner)findViewById(R.id.spinner_export_test_samplingrate);

        Integer[] items = new Integer[]{8000, 16000, 22050, 24000, 32000, 44100, 48000};
        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);

        mSamplingRateSpinner.setAdapter(adapter);
        mSamplingRateSpinner.setSelection(5);

        mVideoBitrate1Spinner = (Spinner)findViewById(R.id.spinner_export_test_bitrate1);
        List<Float> bitrate1_item = new ArrayList<Float>();
        for(int i = 5; i <= 300; i++ ) {
            bitrate1_item.add(((float)i)/10);
        }

        final ArrayAdapter<Float> bitrate1_adapter = new ArrayAdapter<Float>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, bitrate1_item);

        mVideoBitrate1Spinner.setAdapter(bitrate1_adapter);
        mVideoBitrate1Spinner.setSelection(0);


        mFPSSpinner = (Spinner)findViewById(R.id.spinner_export_test_fps);

        Float[] fps_items = new Float[]{20.00f, 24.00f, 24.97f, 25.00f, 29.97f, 30.00f};
        ArrayAdapter<Float> fps_adapter = new ArrayAdapter<Float>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, fps_items);

        mFPSSpinner.setAdapter(fps_adapter);
        mFPSSpinner.setSelection(5);

        mResolutionSpinner = (Spinner)findViewById(R.id.spinner_export_test_resolution);

        List<String> resolutios = new ArrayList<>();

        for(ExportResolution er : mExportResolution )
            resolutios.add(er.display);

        final ArrayAdapter<String> res_adapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, resolutios);

        mResolutionSpinner.setAdapter(res_adapter);
        mResolutionSpinner.setSelection(0);

        mResolutionSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                if( mExportResolution.size() < i )
                    return;

                ExportResolution er = mExportResolution.get(i);

                mVideoBitrate1Spinner.setSelection(0);
                for(int idx = 0; idx < bitrate1_adapter.getCount(); idx++ )
                {
                    if( er.video_bitrate == bitrate1_adapter.getItem(idx) )
                    {
                        mVideoBitrate1Spinner.setSelection(idx);
                        break;
                    }
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });
        setExportProfile(0);

         mProfileSpinner = (Spinner)findViewById(R.id.spinner_export_test_profile);

        //String [] itemssp = new String[]{"AVCProfileBaseline", "AVCProfileMain", "AVCProfileExtended", "AVCProfileHigh", "AVCProfileHigh10", "AVCProfileHigh422", "AVCProfileHigh444"};
        adapterProfile = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, listProfileAdapter);

        mProfileSpinner.setAdapter(adapterProfile);
        //mProfileSpinner.setSelection(0);
        mProfileSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                setExportProfile(2);
                adapterLevele.notifyDataSetChanged();
                mLevelSpinner.setSelection(listLevelAdapter.size()/2);
                //mLevelSpinner.no
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        mLevelSpinner= (Spinner)findViewById(R.id.spinner_export_test_level);

        //String[] itemsle = new String[]{"AVCLevel1", "AVCLevel1b", "AVCLevel11", "AVCLevel12", "AVCLevel13", "AVCLevel2", "AVCLevel21" ,"AVCLevel22" ,"AVCLevel3" ,"AVCLevel31","AVCLevel32" ,"AVCLevel4" ,"AVCLevel41" ,"AVCLevel42","AVCLevel5","AVCLevel51","AVCLevel52"};
        adapterLevele = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, listLevelAdapter);
        mLevelSpinner.setAdapter(adapterLevele);
        mLevelSpinner.setSelection(listLevelAdapter.size()/2);

        mCodecSpinner= (Spinner)findViewById(R.id.spinner_export_test_codec);

        //String[] itemsco = new String[]{"AVC", "HEVC"};
        ArrayAdapter<String> adapterco = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, listCodecAdapter);
        mCodecSpinner.setAdapter(adapterco);

        if ( mCodecSpinner.getItemAtPosition(0) == "AVC" )
            mCodecSpinner.setSelection(0);
        else if ( listCodecAdapter.size() > 1 && mCodecSpinner.getItemAtPosition(1) == "AVC" )
            mCodecSpinner.setSelection(1);
        else
            mCodecSpinner.setSelection(0);

        mCodecSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                setExportProfile(1);
                adapterProfile.notifyDataSetChanged();
                adapterLevele.notifyDataSetChanged();
                mProfileSpinner.setSelection(0);
                mLevelSpinner.setSelection(listLevelAdapter.size()/2);
                if ( mCodecSpinner.getSelectedItem().toString().equalsIgnoreCase("MPEG4V") ) {
                    Log.d(TAG, "onItemSelected = " + mCodecSpinner.getSelectedItem().toString());
                    mResolutionSpinner.setSelection(1);
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        mExportListener = new nexEngineListener(){
            @Override
            public void onStateChange(int oldState, int newState) {
                Log.d(TAG, "onStateChange() old="+ oldState+", new= "+ newState);
                mEngineState = newState;
//                if( oldState == State.RECORD.getValue() || oldState == State.RESUME.getValue() ) {
//                    if( newState == State.IDLE.getValue() ) {
//                        mState = State.IDLE;
//                    }
//                }
            }

            @Override
            public void onTimeChange(int currentTime) {
                Log.d(TAG, "onTimeChange currentTime: " + currentTime);
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                Log.d(TAG, "onSetTimeDone() =" + currentTime);
            }

            @Override
            public void onSetTimeFail(int err) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean iserror, int result) {
                Log.d(TAG, "onEncodingDone() error =" + iserror);

                Toast.makeText(ExportActivity.this, "Encoding elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();
                Log.d(TAG,"Encoding elapsed = "+watch.toString());

                if(iserror) {
                    // for KMSA - 303
                    Log.d(TAG, "Error result=" + result);

                    if(mExportCancel == false)
                    {
                        if(mFile != null) {
                            // To update files at Gallery
                            Log.i(TAG, "To update files at Gallery" + result);
                            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                        }

                        if(nexEngine.nexErrorCode.EXPORT_USER_CANCEL.getValue() == result) {
                            Toast.makeText(getBaseContext(), "Export User Cancel without delete", Toast.LENGTH_SHORT).show();
                        }
                    }
                    else
                    {
                        Toast.makeText(getBaseContext(),"Export Fail!",Toast.LENGTH_SHORT).show();
                        if(mFile!=null) {
                            mFile.delete();
                            mFile = null;
                        }
                    }

                } else {
                    if(mFile != null) {
                        if(oriStartRect != null && oriEndRect != null) {
                            for (int i = 0; i < mEngine.getProject().getTotalClipCount(true); i++) {
                                mEngine.getProject().getClip(i, true).getCrop().setStartPosition(oriStartRect[i]);
                                mEngine.getProject().getClip(i, true).getCrop().setEndPosition(oriEndRect[i]);
                            }
                        }
                        // To update files at Gallery
                        Log.i(TAG, "To update files at Gallery" + result);
                        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                    }
                }

                mProgressBar.setProgress(0);
                mPauseResumeButton.setEnabled(false);
                mState = State.IDLE;
                finish();
            }

            @Override
            public void onPlayEnd() {
                Log.i(TAG, "onPlayEnd() ");
            }

            @Override
            public void onPlayFail(int err, int iClipID) {
                Log.d(TAG,"onPlayFail : err=" + err + " iClipID=" + iClipID );
            }

            @Override
            public void onPlayStart() {
                Log.i(TAG, "onPlayStart() ");
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {
                Log.d(TAG, "onSeekStateChanged() ");
            }

            @Override
            public void onEncodingProgress(int percent) {
                Log.d(TAG, "onEncodingProgress precent: " + percent);
                if(percent > 0)
                {
                    mPauseResumeButton.setEnabled(true);
                }

                mProgressBar.setProgress(percent);
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
        mEngine.setEventHandler(mExportListener);
        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if ( progressDlg != null ) {
                    progressDlg.dismiss();
                    progressDlg = null;
                }
                mEngineState = nexEngine.nexPlayState.IDLE.getValue();
                Log.d(TAG, "nexEngine.stop() done! state=" + mEngineState);
            }
        });
        if ( mEngineState != nexEngine.nexPlayState.IDLE.getValue() ) {
            showProgressDialog();
        }

        if( mEngine.getProject() == null ) {
            finish();
            return;
        }

        oriStartRect = new Rect[mEngine.getProject().getTotalClipCount(true)];
        oriEndRect = new Rect[mEngine.getProject().getTotalClipCount(true)];
        for(int i = 0; i<mEngine.getProject().getTotalClipCount(true); i++){
            oriStartRect[i] = new Rect();
            mEngine.getProject().getClip(i, true).getCrop().getStartPosition(oriStartRect[i]);
            oriEndRect[i] = new Rect();
            mEngine.getProject().getClip(i, true).getCrop().getEndPosition(oriEndRect[i]);
        }
    }

    private File getExportFile(int wid , int hei, int samplingRate, String ext) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NexEditor_"+wid+"X"+hei + "_" + samplingRate + "_" + export_time+"."+ext);
        return exportFile;
    }

    private ArrayList<String> listCodecAdapter = new ArrayList<>();
    private ArrayList<String> listProfileAdapter = new ArrayList<>();
    private ArrayList<String> listLevelAdapter = new ArrayList<>();

    private void setExportLevel(int codec ,int maxLevel){
        listLevelAdapter.clear();
        for( String key : levelmap.keySet() ){
            int level = levelmap.get(key);
            if( level <= maxLevel ){
                if ((codec==nexEngine.ExportCodec_AVC && !key.contains("AVC")) || (codec!=nexEngine.ExportCodec_AVC && key.contains("AVC"))) continue;
                else if ((codec==nexEngine.ExportCodec_HEVC && !key.contains("HEVC")) || (codec!=nexEngine.ExportCodec_HEVC && key.contains("HEVC"))) continue;
                else if ((codec==nexEngine.ExportCodec_MPEG4V && !key.contains("MPEG4")) || (codec!=nexEngine.ExportCodec_MPEG4V && key.contains("MPEG4"))) continue;

                if (!listLevelAdapter.contains(key)) listLevelAdapter.add(key);
                if ( codec!=nexEngine.ExportCodec_HEVC ) Collections.sort(listLevelAdapter);// Don't sort level name for HEVC
            }
        }
    }

    private void setExportProfile( int selectMode ){
        nexEngine.ExportProfile[] profiles = nexEngine.getExportProfile();
        String selProfile = null;
        String selCodec = null;

        if( profiles.length == 0 ){
            if( selectMode == 0 ) {
                listCodecAdapter.clear();
                listCodecAdapter.add("AVC");
            }else if( selectMode == 1 ){
                listProfileAdapter.clear();
                listProfileAdapter.add("AVCProfileBaseline");
            }else if( selectMode == 2 ) {
                setExportLevel(nexEngine.ExportCodec_AVC, nexEngine.ExportAVCLevel31);
            }
            return;
        }
        if( selectMode != 0 ) {
            selProfile = (String) mProfileSpinner.getSelectedItem();
            selCodec = (String) mCodecSpinner.getSelectedItem();
        }
        if ( selectMode == 1) listProfileAdapter.clear();

        for (nexEngine.ExportProfile p : profiles) {
            System.out.println("setExportProfile="+Integer.toHexString(p.getMimeType()));
            if (p.getMimeType() == nexEngine.ExportCodec_AVC ) {
                if( selectMode == 0 ) { //init
                    if ( !listCodecAdapter.contains("AVC") ) listCodecAdapter.add("AVC");
                }else if( selectMode == 1  ){ //codec select
                    if( selCodec.compareTo("AVC") != 0 ){
                        continue;
                    }
                }else if( selectMode == 2 ){//profile select
                    if( selCodec.compareTo("AVC") != 0 ){
                        continue;
                    }
                }
                for( nexEngine.ProfileAndLevel pl : p.getProFileAndLevel() ){
                    if( pl.getProfile() == nexEngine.ExportProfile_AVCBaseline ){
                        if( selectMode == 1 ) {
                            if ( !listProfileAdapter.contains("AVCProfileBaseline")) listProfileAdapter.add("AVCProfileBaseline");
                            setExportLevel(nexEngine.ExportCodec_AVC, pl.getLevel());
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("AVCProfileBaseline") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_AVC, pl.getLevel());
                        }
                    }else if( pl.getProfile() == nexEngine.ExportProfile_AVCMain ){
                        if( selectMode == 1 ) {
                            if (!listProfileAdapter.contains("AVCProfileMain")) listProfileAdapter.add("AVCProfileMain");
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("AVCProfileMain") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_AVC, pl.getLevel());
                        }
                    }else if( pl.getProfile() == nexEngine.ExportProfile_AVCHigh ){
                        if( selectMode == 1 ) {
                            if (!listProfileAdapter.contains("AVCProfileHigh")) listProfileAdapter.add("AVCProfileHigh");
                        }else if( selectMode == 2 ){
                            if( selProfile.compareTo("AVCProfileHigh") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_AVC, pl.getLevel());
                        }
                    }
                }
            }else if( p.getMimeType() == nexEngine.ExportCodec_HEVC ){
                if( selectMode == 0 ) {
                    if (!listCodecAdapter.contains("HEVC")) listCodecAdapter.add("HEVC");
                }else if( selectMode == 1  ){ //codec select
                    if( selCodec.compareTo("HEVC") != 0 ){
                        continue;
                    }
                }else if( selectMode == 2 ){//profile select
                    if( selCodec.compareTo("HEVC") != 0 ){
                        continue;
                    }
                }
		        for( nexEngine.ProfileAndLevel pl : p.getProFileAndLevel() ){
                    if( pl.getProfile() == nexEngine.ExportProfile_HEVCMain ){
                        if( selectMode == 1 ) {
                            if ( !listProfileAdapter.contains("HEVCProfileMain")) listProfileAdapter.add("HEVCProfileMain");
                            setExportLevel(nexEngine.ExportCodec_HEVC, pl.getLevel());
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("HEVCProfileMain") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_HEVC, pl.getLevel());
                        }
                    }else if( pl.getProfile() == nexEngine.ExportProfile_HEVCMain10 ){
                        if( selectMode == 1 ) {
                            if (!listProfileAdapter.contains("HEVCProfileMain10")) listProfileAdapter.add("HEVCProfileMain10");
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("HEVCProfileMain10") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_HEVC, pl.getLevel());
                        }
                    }
                }
            }else if( p.getMimeType() == nexEngine.ExportCodec_MPEG4V ){
                if ( selectMode == 0 ) {
                    if (!listCodecAdapter.contains("MPEG4V")) listCodecAdapter.add("MPEG4V");
                } else if( selectMode == 1  ){ //codec select
                    if( selCodec.compareTo("MPEG4V") != 0 ){
                        continue;
                    }
                }else if( selectMode == 2 ){//profile select
                    if( selCodec.compareTo("MPEG4V") != 0 ){
                        continue;
                    }
                }

                for( nexEngine.ProfileAndLevel pl : p.getProFileAndLevel() ){
                    if( pl.getProfile() == nexEngine.ExportProfile_MPEG4VSimple){
                        if( selectMode == 1 ) {
                            if (!listProfileAdapter.contains("MPEG4VSimple")) listProfileAdapter.add("MPEG4VSimple");
                            setExportLevel(nexEngine.ExportCodec_MPEG4V, pl.getLevel());
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("MPEG4VSimple") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_MPEG4V, pl.getLevel());
                        }
                    }else if( pl.getProfile() == nexEngine.ExportProfile_MPEG4VASP){
                        if( selectMode == 1 ) {
                            if (!listProfileAdapter.contains("MPEG4VASP")) listProfileAdapter.add("MPEG4VASP");
                        }else if(  selectMode == 2 ){
                            if( selProfile.compareTo("MPEG4VASP") != 0){
                                continue;
                            }
                            setExportLevel(nexEngine.ExportCodec_MPEG4V, pl.getLevel());
                        }
                    }
                }


						
            }
        }
    }

    private int ExternalExport(int width , int height ){
        int index = mExportModuleSpinner.getSelectedItemPosition();
        if( index == 0 ){
            return 0;
        }
        index--;
        String uuid = imageExports.get(index).uuid();
        String type = "external-"+imageExports.get(index).name();
        String ext = imageExports.get(index).format();
        if( uuid == null){
            return 0;
        }

        float ratio = 480f / (float)height;
        int w = (int)(width * ratio);

        mFile = getExportFile(width, height, 0,ext);
        nexExportFormat format = nexExportFormatBuilder.Builder()
                .setType(type)
                .setWidth(w)
                .setHeight(480)
                .setPath(mFile.getAbsolutePath())
                .setIntervalTime(200)
                .setStartTime(0)
                .setEndTime(8000)
                .setUUID(uuid)
                .build();
        mEngine.updateProject();
        mEngine.export(format, new nexExportListener() {
            @Override
            public void onExportFail(nexEngine.nexErrorCode err) {
                Log.d(TAG, "onExportFail: " + err.toString());


                if(mExportCancel == false)
                {
                    if(mFile != null) {
                        // To update files at Gallery
                        Log.i(TAG, "To update files at Gallery: " + err.toString());
                        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                    }

                    if( err == nexEngine.nexErrorCode.EXPORT_USER_CANCEL) {
                        Toast.makeText(getBaseContext(), "Export User Cancel without delete", Toast.LENGTH_SHORT).show();
                    }
                }
                else
                {
                    Toast.makeText(getBaseContext(),"Export Fail!",Toast.LENGTH_SHORT).show();
                    if(mFile!=null) {
                        mFile.delete();
                        mFile = null;
                    }
                }

                mProgressBar.setProgress(0);
                mPauseResumeButton.setEnabled(false);
                mState = State.IDLE;
                finish();
            }

            @Override
            public void onExportProgress(int persent) {
                if( mProgressBar != null ) {
                    mProgressBar.setProgress(persent);
                }
            }

            @Override
            public void onExportDone(Bitmap bitmap) {
                Log.d(TAG, "onExportDone: ");

                Toast.makeText(ExportActivity.this, "Encoding elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();

                if(mFile != null) {
                    if(oriStartRect != null && oriEndRect != null) {
                        for (int i = 0; i < mEngine.getProject().getTotalClipCount(true); i++) {
                            mEngine.getProject().getClip(i, true).getCrop().setStartPosition(oriStartRect[i]);
                            mEngine.getProject().getClip(i, true).getCrop().setEndPosition(oriEndRect[i]);
                        }
                    }
                    // To update files at Gallery
                    MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                }
                mProgressBar.setProgress(0);
                mPauseResumeButton.setEnabled(false);
                mState = State.IDLE;
                finish();
            }
        });
        return 1;
    }

    private ExportResolution getExportResolution(int iMode)
    {
        ExportResolution item = new ExportResolution();

        int val[][][] = {   {{1920, 1080, 6}, {1280, 720, 4},   {960, 540, 3},  {864, 486, 2}, {640, 360, 1}},
                            {{1080, 1920, 6}, {720, 1280, 4},   {540, 960, 3},  {486, 864, 2}, {360, 640, 1}},
                            {{1080, 1080, 6}, {720, 720, 4},    {540, 540, 3},  {480, 480, 2}, {360, 360, 1}},
                            {{2160, 1080, 6}, {1440, 720, 4},   {1080, 540, 3}, {960, 480, 2}, {720, 360, 1}},
                            {{1080, 2160, 6}, {720, 1440, 4},   {540, 1080, 3}, {480, 960, 2}, {360, 720, 1}},
                            {{1920, 1440, 5}, {1280, 960, 4},   {640, 480, 3}, {480, 360, 2}, {320, 240, 1}},
                            {{1440, 1920, 5}, {960, 1280, 4},   {480, 640, 3}, {360, 480, 2}, {240, 320, 1}},
        };

        if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_1v1 ) {
            item.width = val[2][iMode][0];
            item.height = val[2][iMode][1];
            item.video_bitrate = val[2][iMode][2];
        }else if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_9v16 ){
            item.width = val[1][iMode][0];
            item.height = val[1][iMode][1];
            item.video_bitrate = val[1][iMode][2];
        }else if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_2v1 ) {
            item.width = val[3][iMode][0];
            item.height = val[3][iMode][1];
            item.video_bitrate = val[3][iMode][2];
        }else if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_1v2 ) {
            item.width = val[4][iMode][0];
            item.height = val[4][iMode][1];
            item.video_bitrate = val[4][iMode][2];
        }else if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_3v4 ) {
            item.width = val[6][iMode][0];
            item.height = val[6][iMode][1];
            item.video_bitrate = val[6][iMode][2];
        }else if( mAspectMode == nexApplicationConfig.kAspectRatio_Mode_4v3 ) {
            item.width = val[5][iMode][0];
            item.height = val[5][iMode][1];
            item.video_bitrate = val[5][iMode][2];
        }else{
            item.width = val[0][iMode][0];
            item.height = val[0][iMode][1];
            item.video_bitrate = val[0][iMode][2];
        }

        item.display = ""+item.width+"x"+item.height;

        return item;
    }

    @Override
    public void onBackPressed() {
        //super.onBackPressed();
        Toast.makeText(getBaseContext(), "Please use cancel button if you want finish", Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onPause() {
        if( mAutoPauseResume ){
            if (mState == State.RECORD || mState == State.RESUME) {
                mEngine.exportPause();
                mState = State.PAUSE;
            }
        }
        super.onPause();
    }

    @Override
    protected void onResume() {
        if( mAutoPauseResume ){
            if (mState == State.PAUSE) {
                mEngine.exportResume();
                mState = State.RESUME;
            }
        }
        super.onResume();
    }

    private void showProgressDialog() {
        progressDlg = new ProgressDialog(ExportActivity.this);
        progressDlg.setTitle("Engine stopping ...");
        progressDlg.setMessage("Waiting...");
        //progressDlg.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
        progressDlg.setCanceledOnTouchOutside(false);
        progressDlg.setProgress(0);
        progressDlg.setMax(100);
        progressDlg.show();
    }
}
