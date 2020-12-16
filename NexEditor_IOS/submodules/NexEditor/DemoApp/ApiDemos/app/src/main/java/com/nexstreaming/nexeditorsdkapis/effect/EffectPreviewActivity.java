/******************************************************************************
 * File Name        : EffectPreviewActivity.java
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

package com.nexstreaming.nexeditorsdkapis.effect;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;

import com.nexstreaming.nexeditorsdk.nexClipEffect;
import com.nexstreaming.nexeditorsdk.nexEffect;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEffectPreviewView;
import com.nexstreaming.nexeditorsdk.nexOverlayFilter;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.nexeditorsdkapis.dialog.ColorPickerDialog;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Constants;

import java.util.ArrayList;
import java.util.List;

public class EffectPreviewActivity extends Activity  {
    private static final String TAG = "EffectPreviewActivity";
    private nexEffectPreviewView mPreviewView;
    private ArrayList mEffectItem;
    private EditText mEditTextLine1;
    private EditText mEditTextLine2;
    private EditText mEditTextLine3;
    private EditText mEditTextLine4;
    private int mEffectType;
    private nexEffectOptions mOptions;
    private Spinner mSpinnerSelectOption1;
    private Spinner mSpinnerSelectOption2;
    private Button mBtnColor1;
    private Button mBtnColor2;
    private Button mBtnColor3;
    private Button mBtnSet;

    private SeekBar mSeekRange1;
    private SeekBar mSeekRange2;

    private String mSelectedEffectId;

    private int mColor1;
    private int mColor2;
    private int mColor3;
    private int mSelectedColor = 1;
    private ColorPickerDialog.OnColorChangedListener mColorChangedListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_effect_preview);
        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Intent intent = getIntent();
        mEffectType = intent.getIntExtra(Constants.EFFECT_TYPE, 0);

        mPreviewView = (nexEffectPreviewView)findViewById(R.id.effectpreview_effectpreview);
        mEditTextLine1 = (EditText)findViewById(R.id.edittext_thumbnail_line1);
        mEditTextLine2 = (EditText)findViewById(R.id.edittext_effectpreview_line2);
        mEditTextLine3 = (EditText)findViewById(R.id.edittext_effectpreview_line3);
        mEditTextLine4 = (EditText)findViewById(R.id.edittext_effectpreview_line4);
        mEditTextLine1.setText("Text Line 1");
        mEditTextLine2.setText("Text Line 2");
        mEditTextLine3.setText("Text Line 3");
        mEditTextLine4.setText("Text Line 4");
        nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(getApplicationContext());
        mBtnSet = (Button)findViewById(R.id.btn_effectpreview_set);
        mBtnSet.setVisibility(View.INVISIBLE);
        mEffectItem = new ArrayList<String>();

        if(mEffectType == Constants.EFFECT_TYPE_TRANSITION) {
            nexTransitionEffect[] transitionEffects = fxlib.getTransitionEffects();
            for (nexTransitionEffect e : transitionEffects) {
                mEffectItem.add(e.getId());
            }
        } else if( mEffectType == Constants.EFFECT_TYPE_CLIP) {
            nexClipEffect[] clipEffects = fxlib.getClipEffects();
            for (nexClipEffect e : clipEffects) {
                mEffectItem.add(e.getId());
            }
        }else if(mEffectType == Constants.EFFECT_TYPE_SELECT_CLIP) {
            mBtnSet.setVisibility(View.VISIBLE);
            nexClipEffect[] clipEffects = fxlib.getClipEffects();
            for (nexClipEffect e : clipEffects) {
                mEffectItem.add(e.getId());
            }
        }else if(mEffectType == Constants.EFFECT_TYPE_SELECT_TRANSITION) {
            mBtnSet.setVisibility(View.VISIBLE);
            nexTransitionEffect[] transitionEffects = fxlib.getTransitionEffects();
            for (nexTransitionEffect e : transitionEffects) {
                mEffectItem.add(e.getId());
            }
        }else if (mEffectType == Constants.EFFECT_TYPE_SELECT_OVERLAYFILTER) {
            mBtnSet.setVisibility(View.VISIBLE);
            nexOverlayFilter[] overlayEffects = fxlib.getOverlayFilters();
            for (nexOverlayFilter e : overlayEffects) {
                mEffectItem.add(e.getId());
            }
        }

        mSpinnerSelectOption1 = (Spinner) EffectPreviewActivity.this.findViewById(R.id.spinner_effectpreview_selectoption1);
        mSpinnerSelectOption1.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //mOptions.getSelectOpt(0).setSelectIndex(position);
                mOptions.getSelectOptions().get(0).setSelectIndex(position);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSpinnerSelectOption2 = (Spinner) EffectPreviewActivity.this.findViewById(R.id.spinner_effectpreview_selectoption2);
        mSpinnerSelectOption2.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //mOptions.getSelectOpt(1).setSelectIndex(position);
                mOptions.getSelectOptions().get(1).setSelectIndex(position);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSeekRange1 = (SeekBar) EffectPreviewActivity.this.findViewById(R.id.seekBar__clip_effectpreview_range1);

        mSeekRange1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lastProgress = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //mOptions.getRangeOptions().get(0).max()
                mOptions.getRangeOptions().get(0).setValue(lastProgress + mOptions.getRangeOptions().get(0).min());
            }
        });

        mSeekRange2 = (SeekBar) EffectPreviewActivity.this.findViewById(R.id.seekBar__clip_effectpreview_range2);
        mSeekRange2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mOptions.getRangeOptions().get(1).setValue(lastProgress + mOptions.getRangeOptions().get(1).min());
            }
        });

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, mEffectItem);

        Spinner spinnerSelect = (Spinner) findViewById(R.id.spinner_thumbnail_select);
        spinnerSelect.setAdapter(adapter);
        spinnerSelect.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedEffectId = mEffectItem.get(position).toString();
                //nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(EffectPreviewActivity.this);
                //fxlib.info(mSelectedEffectId);
                updateData(EffectPreviewActivity.this, mSelectedEffectId);
                parent.invalidate();
            }

            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        Button btnOk = (Button)findViewById(R.id.button_effectpreview_ok);
        btnOk.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mEffectType == Constants.EFFECT_TYPE_SELECT_OVERLAYFILTER ){
                    return;
                }

                if (mEditTextLine1.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(0).setText(mEditTextLine1.getText().toString());
                }

                if (mEditTextLine2.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(1).setText(mEditTextLine2.getText().toString());
                }

                if (mEditTextLine3.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(2).setText(mEditTextLine3.getText().toString());
                }

                if (mEditTextLine4.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(3).setText(mEditTextLine4.getText().toString());
                }
                mPreviewView.setEffectOptions(nexEffect.getTitleOptions(mOptions));
                mPreviewView.setEffect(mSelectedEffectId);
                mPreviewView.setEffectTime(3000);
            }
        });

        mBtnSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEditTextLine1.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(0).setText(mEditTextLine1.getText().toString());
                }

                if (mEditTextLine2.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(1).setText(mEditTextLine2.getText().toString());
                }

                if (mEditTextLine3.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(2).setText(mEditTextLine3.getText().toString());
                }

                if (mEditTextLine4.getVisibility() == View.VISIBLE) {
                    mOptions.getTextOptions().get(3).setText(mEditTextLine4.getText().toString());
                }

                if (mEffectType == Constants.EFFECT_TYPE_SELECT_CLIP) {
                    Intent intent = new Intent();
                    intent.putExtra(Constants.EFFECT_OPTION, mOptions);
                    intent.putExtra(Constants.EFFECT, mSelectedEffectId);
                    setResult(Activity.RESULT_OK, intent);
                    finish();
                } else if (mEffectType == Constants.EFFECT_TYPE_SELECT_TRANSITION) {
                    Intent intent = new Intent();
                    intent.putExtra(Constants.TRANSITION_EFFECT_OPTION, mOptions);
                    intent.putExtra(Constants.TRANSITION_EFFECT, mSelectedEffectId);
                    setResult(Activity.RESULT_OK, intent);
                    finish();
                } else if(mEffectType == Constants.EFFECT_TYPE_SELECT_OVERLAYFILTER) {
                    Intent intent = new Intent();
                    intent.putExtra(Constants.OVERLAY_FILTER_OPTION, mOptions);
                    intent.putExtra(Constants.OVERLAY_FILTER_ID, mSelectedEffectId);
                    setResult(Activity.RESULT_OK, intent);
                    finish();
                }
            }
        });
        mBtnColor1 = (Button)findViewById(R.id.button_effectpreview_color1);
        mBtnColor1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 1;
                new ColorPickerDialog(EffectPreviewActivity.this, mColorChangedListener, mColor1).show();
            }
        });
        mBtnColor2 = (Button)findViewById(R.id.button_effectpreview_color2);
        mBtnColor2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 2;
                new ColorPickerDialog(EffectPreviewActivity.this, mColorChangedListener, mColor2).show();
            }
        });
        mBtnColor3 = (Button)findViewById(R.id.button_effectpreview_color3);
        mBtnColor3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 3;
                new ColorPickerDialog(EffectPreviewActivity.this, mColorChangedListener, mColor3).show();
            }
        });
        mColorChangedListener = new ColorPickerDialog.OnColorChangedListener(){
            @Override
            public void colorChanged(int color) {
                if( mSelectedColor == 1 ) {
                    mColor1 = color;
                    mOptions.getColorOptions().get(0).setARGBColor(mColor1);
                }else if( mSelectedColor == 2 ){
                    mColor2 = color;
                    mOptions.getColorOptions().get(1).setARGBColor(mColor2);
                }else if( mSelectedColor == 3 ){
                    mColor3 = color;
                    mOptions.getColorOptions().get(2).setARGBColor(mColor3);
                }
            }
        };

    }

    private void updateData(Context context ,String effectId){
        nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(context);
        mOptions = fxlib.getEffectOptions(context,effectId);
        mEditTextLine1.setVisibility(View.GONE);
        mEditTextLine2.setVisibility(View.GONE);
        mEditTextLine3.setVisibility(View.GONE);
        mEditTextLine4.setVisibility(View.GONE);
        mSpinnerSelectOption1.setVisibility(View.GONE);
        mSpinnerSelectOption2.setVisibility(View.GONE);
        mBtnColor1.setVisibility(View.GONE);
        mBtnColor2.setVisibility(View.GONE);
        mBtnColor3.setVisibility(View.GONE);
        mSeekRange1.setVisibility(View.GONE);
        mSeekRange2.setVisibility(View.GONE);

        for( int line = 0; line < mOptions.getTextFieldCount() ; line++ ){
            switch (line){
                case 0:
                    mEditTextLine1.setVisibility(View.VISIBLE);
                    break;

                case 1:
                    mEditTextLine2.setVisibility(View.VISIBLE);
                    break;

                case 2:
                    mEditTextLine3.setVisibility(View.VISIBLE);
                    break;

                case 3:
                    mEditTextLine4.setVisibility(View.VISIBLE);
                    break;

                default:
                    break;
            }
        }

        List <nexEffectOptions.ColorOpt> color = mOptions.getColorOptions();
        int colorCount = 0;
        if( color != null ){
            for( nexEffectOptions.ColorOpt opt : color ){
                if( colorCount == 0 ){
                    mBtnColor1.setVisibility(View.VISIBLE);
                    mBtnColor1.setText(opt.getLabel());
                    mColor1 = opt.getARGBformat();
                }else if( colorCount == 1 ){
                    mBtnColor2.setVisibility(View.VISIBLE);
                    mBtnColor2.setText(opt.getLabel());
                    mColor2 = opt.getARGBformat();
                }else if( colorCount == 2 ){
                    mBtnColor3.setVisibility(View.VISIBLE);
                    mBtnColor3.setText(opt.getLabel());
                    mColor3 = opt.getARGBformat();
                }
                colorCount++;
            }
        }

        List <nexEffectOptions.SelectOpt> select =  mOptions.getSelectOptions();
        int selectCount = 0;
        if( select != null ) {
            for (nexEffectOptions.SelectOpt opt : select) {

                if (selectCount == 0) {
                    mSpinnerSelectOption1.setVisibility(View.VISIBLE);
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                            R.layout.simple_spinner_dropdown_item_1, opt.getItems());
                    mSpinnerSelectOption1.setAdapter(adapter);
                } else if (selectCount == 1) {
                    mSpinnerSelectOption2.setVisibility(View.VISIBLE);
                    ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(this,
                            R.layout.simple_spinner_dropdown_item_1, opt.getItems());
                    mSpinnerSelectOption2.setAdapter(adapter2);
                }
                selectCount++;
            }
        }

        List<nexEffectOptions.RangeOpt> rangeOpts = mOptions.getRangeOptions();
        if( rangeOpts != null){
            int count = 0;
            for( nexEffectOptions.RangeOpt opt : rangeOpts ){
                if( count == 0 ){
                    int range = opt.max() - opt.min() ;
                    int pos = opt.getValue()  - opt.min();
                    mSeekRange1.setVisibility(View.VISIBLE);
                    mSeekRange1.setMax(range);
                    mSeekRange1.setProgress(pos);
                }else if( count == 1 ){
                    int range = opt.max() - opt.min();
                    int pos = opt.getValue() - opt.min();
                    mSeekRange2.setVisibility(View.VISIBLE);
                    mSeekRange2.setMax(range);
                    mSeekRange2.setProgress(pos);
                }
            }
        }
    }
}
