/******************************************************************************
 * File Name        : OverlayFilterPreViewActivity.java
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

package com.nexstreaming.nexeditorsdkapis.overlay;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayFilter;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;

import java.util.ArrayList;
import java.util.List;

public class OverlayFilterPreViewActivity extends Activity {
    static private String TAG = "OverlayFilterPreView";
    SeekBar mXposSeekBar;
    SeekBar mYposSeekBar;
    SeekBar mRotateSeekBar;
    SeekBar mScaleXSeekBar;
    SeekBar mScaleYSeekBar;

    TextView mXposTextView;
    TextView mYposTextView;
    TextView mRotateTextView;
    TextView mScaleXTextView;
    TextView mScaleYTextView;

    SeekBar mOption1SeekBar;
    TextView mOption1TextView;
    TextView mOpt1TitleTextView;
    int option1Max;
    int option1Min;
    int option1Val;

    SeekBar mOption2SeekBar;
    TextView mOption2TextView;
    TextView mOpt2TitleTextView;
    int option2Max;
    int option2Min;
    int option2Val;


    Spinner mOverlayFilterIdSpinner;
    private ArrayList mFilterItems;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private String mSelectedOverlayFilterId;
    private int mOverlayId;
    nexOverlayFilter mFilter;
    int overlayWidth = 1280;
    int overlayHeight = 720;

    int mX;
    int mY;
    int mRotate;
    int mWidth;
    int mHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_overlay_filter_pre_view);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        nexProject project = new nexProject();
        String path;
        try {
            UtilityCode.raw2file(getApplicationContext(), R.mipmap.dessert, "desert.jpg");
        } catch (Exception e) {
            e.printStackTrace();
        }
        path = getFilesDir().getAbsolutePath()+"/desert.jpg";
        nexClip clip=nexClip.getSupportedClip(path);
        if( clip != null ){
            project.add(clip);
            project.getClip(0,true).setImageClipDuration(6000);
            project.getClip(0,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
        }

        overlayWidth = nexApplicationConfig.getAspectProfile().getWidth();
        overlayHeight = nexApplicationConfig.getAspectProfile().getHeight();

        mX = overlayWidth/2;
        mY = overlayHeight/2;
        mRotate = 0;
        mWidth = overlayWidth/2;
        mHeight = overlayHeight/2;

        nexOverlayItem.setOutLine();

        m_editorView = (nexEngineView)findViewById(R.id.engineview_overlay_fastview);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.updateProject();
        mEngine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {
                mEngine.seek(1000);
            }
        });

        mOverlayFilterIdSpinner = (Spinner)findViewById(R.id.spinner_overlayfilter_ids);
        nexOverlayFilter [] overlayFilters = nexEffectLibrary.getEffectLibrary(getApplicationContext()).getOverlayFilters();
        if( overlayFilters == null ){
            finish();
            return;
        }

        if( overlayFilters.length == 0 ){
            finish();
            return;
        }

        mFilterItems = new ArrayList();
        for( nexOverlayFilter filter : overlayFilters ){
            mFilterItems.add(filter.getId());
        }
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, mFilterItems);

        mOverlayFilterIdSpinner.setAdapter(adapter);

        mOverlayFilterIdSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.getProject().clearOverlay();
                mSelectedOverlayFilterId = mFilterItems.get(position).toString();
                mFilter = nexEffectLibrary.getEffectLibrary(getApplicationContext()).findOverlayFilterById(mSelectedOverlayFilterId);
                mFilter.setWidth(mWidth);
                mFilter.setHeight(mHeight);
                nexOverlayItem overlayItem = new nexOverlayItem(mFilter,nexOverlayItem.AnchorPoint_LeftTop,false,mX,mY,0,6000);
                mEngine.getProject().addOverlay(overlayItem);
                mEngine.seek(3000);
                mOverlayId = overlayItem.getId();
                Log.d(TAG,"mOverlayId="+mOverlayId);
                updateUI();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        mXposTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_x);
        mYposTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_y);
        mRotateTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_rotate);
        mScaleXTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_width);
        mScaleYTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_height);

        mXposSeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_x);
        mXposSeekBar.setMax(overlayWidth);
        mXposSeekBar.setProgress(mX);
        mXposTextView.setText(""+mX);
        mXposSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mXposTextView.setText("" + progress);
                lastProgress = progress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setPositionX(progress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mX = lastProgress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setPosition(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getPositionY());
            }
        });



        mYposSeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_y);
        mYposSeekBar.setMax(overlayHeight);
        mYposSeekBar.setProgress(mY);
        mYposTextView.setText("" + mY);
        mYposSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mYposTextView.setText("" + progress);
                lastProgress = progress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setPositionY(progress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mY = lastProgress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setPosition(mEngine.getProject().getOverlay(mOverlayId).getPositionX(), lastProgress);
            }
        });

        mRotateSeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_rotate);
        mRotateSeekBar.setMax(359);
        mRotateSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mRotateTextView.setText("" + progress);
                lastProgress = progress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateZ(progress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mRotate = lastProgress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setRotate(lastProgress);
            }
        });

        mScaleXSeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_width);
        mScaleXSeekBar.setMax(overlayWidth);
        mScaleXSeekBar.setProgress(mWidth);
        mScaleXTextView.setText("" + mWidth);
        mScaleXSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                lastProgress = progress;
                mScaleXTextView.setText("" + progress);
                mFilter.setWidth(lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mWidth = lastProgress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                //mEngine.getProject().getOverlay(mOverlayId).setScale(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getScaledY());
            }
        });

        mScaleYSeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_height);
        mScaleYSeekBar.setMax(overlayHeight);
        mScaleYSeekBar.setProgress(mHeight);
        mScaleYTextView.setText("" + mHeight);
        mScaleYSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                lastProgress = progress;
                mScaleYTextView.setText("" + progress);
                mFilter.setHeight(lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).display();

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mHeight = lastProgress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
            }
        });

        mOption1SeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_option1) ;
        mOption1SeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                option1Val = option1Min+progress;
                mOption1TextView.setText(""+option1Val);
                mFilter.getEffectOption().getRangeOptions().get(0).setValue(option1Val);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
            }
        });
        mOption1TextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_option1);
        mOpt1TitleTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_option1_title);
        mOption1SeekBar.setVisibility(View.GONE);
        mOption1TextView.setVisibility(View.GONE);
        mOpt1TitleTextView.setVisibility(View.GONE);
        mOption2SeekBar = (SeekBar)findViewById(R.id.seekbar_overlayfilter_preview_option2) ;
        mOption2SeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                option2Val = option2Min+progress;
                mOption2TextView.setText(""+option2Val);
                mFilter.getEffectOption().getRangeOptions().get(1).setValue(option2Val);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
            }
        });
        mOption2TextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_option2);
        mOpt2TitleTextView = (TextView)findViewById(R.id.textview_overlayfilter_preview_option2_title);
        mOption2SeekBar.setVisibility(View.GONE);
        mOption2TextView.setVisibility(View.GONE);
        mOpt2TitleTextView.setVisibility(View.GONE);
    }

    private void updateUI(){
        mOption1SeekBar.setVisibility(View.GONE);
        mOption1TextView.setVisibility(View.GONE);
        mOption2SeekBar.setVisibility(View.GONE);
        mOption2TextView.setVisibility(View.GONE);
        mOpt1TitleTextView.setVisibility(View.GONE);
        mOpt2TitleTextView.setVisibility(View.GONE);
        if( mFilter != null ){
            nexEffectOptions options = mFilter.getEffectOption();
            if( options != null ) {
                List<nexEffectOptions.RangeOpt> rangeOpts = options.getRangeOptions();
                if( rangeOpts != null ){
                    int count = 0;
                    for( nexEffectOptions.RangeOpt op : rangeOpts){
                        if( count == 0 ){
                            option1Max = op.max();
                            option1Min = op.min();
                            option1Val = op.getValue();
                            mOption1SeekBar.setVisibility(View.VISIBLE);
                            mOption1SeekBar.setMax(option1Max - option1Min);
                            mOption1SeekBar.setProgress(option1Val - option1Min);
                            mOption1TextView.setVisibility(View.VISIBLE);
                            mOption1TextView.setText(""+option1Val);
                            mOpt1TitleTextView.setText(op.getLabel());
                            mOpt1TitleTextView.setVisibility(View.VISIBLE);


                        }else if(count == 1 ){
                            option2Max = op.max();
                            option2Min = op.min();
                            option2Val = op.getValue();
                            mOption2SeekBar.setVisibility(View.VISIBLE);
                            mOption2SeekBar.setMax(option2Max - option2Min);
                            mOption2SeekBar.setProgress(option2Val - option2Min);
                            mOption2TextView.setVisibility(View.VISIBLE);
                            mOption2TextView.setText(""+option2Val);
                            mOpt2TitleTextView.setText(op.getLabel());
                            mOpt2TitleTextView.setVisibility(View.VISIBLE);
                        }
                        count++;
                    }
                }
            }
        }
    }
}
