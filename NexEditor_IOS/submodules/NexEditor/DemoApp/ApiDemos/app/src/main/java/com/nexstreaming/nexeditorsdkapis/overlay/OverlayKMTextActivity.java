/******************************************************************************
 * File Name        : OverlayKMTextActivity.java
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
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterExpression;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterText;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.dialog.ColorPickerDialog;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class OverlayKMTextActivity extends Activity {
    private static final String TAG = "KMTextOverlay";
    private nexEngineListener mEditorListener = null;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private EditText mText;
    private Spinner mSpinFont;
    private Spinner mSpinSize;
    private Spinner mSpinExpression;
    private boolean mExportStop;
    private Button mPlayButton;
    private String mDefaultFont;
    private TextView textView_width;
    private TextView textView_height;
    private TextView textView_X;
    private TextView textView_Y;
    private TextView textView_degree;
    private TextView textView_scale;

    private CheckBox mChkShadow;
    private CheckBox mChkOutline;
    private CheckBox mChkGlow;

    private nexOverlayKineMasterText mOverlayKMText;
    private int mOverlayItemId;
    private List<String> mFontIDs = new ArrayList<>();
    private static Integer[] mSizes = {50, 100, 150 , 200, 250 };

    private int mColorText;
    private int mColorOutLine;
    private int mColorShadow;
    private int mColorGlow;
    private int mSelectedColor = 1;
    private ColorPickerDialog.OnColorChangedListener mColorChangedListener;
    private double mLastDistance;

    private enum TouchMode {
        HIT,
        ROTATE,
        SCALE,
        MOVE,
        NONE
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_overlay_kmtext);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        nexProject project = new nexProject();
        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexClip clip = new nexClip(m_listfilepath.get(0));

        if( clip.getSupportedResult() != 0 ){
            Log.e(TAG,"getSupportedResult="+ clip.getSupportedResult());
            Toast.makeText(getApplicationContext(), "Not supported Clips.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if( clip.getClipType() != nexClip.kCLIP_TYPE_VIDEO  ){
            Toast.makeText(getApplicationContext(), "No Video Clips.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if( clip.getTotalTime() < 3000 ){
            Toast.makeText(getApplicationContext(), "Video is short.( more 3 second)", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        int maxcount = 2;
        for( int i = 0 ; i < maxcount ; i++){
            project.add(nexClip.dup(clip));
            project.getClip(i,true).getVideoClipEdit().setTrim(0, 3000);
            project.getClip(i,true).setRotateDegree(project.getClip(i,true).getRotateInMeta());
        }
        File ffile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+"gallery"+File.separator+"knewave.ttf");
        if( ffile.isFile() ) {
            mDefaultFont = ffile.getAbsolutePath();
        }

        mOverlayKMText = new nexOverlayKineMasterText(getApplicationContext(),"", 50);
        mOverlayKMText.setFontId(mDefaultFont);
        int width = nexApplicationConfig.getAspectProfile().getWidth()/2;
        int height = nexApplicationConfig.getAspectProfile().getHeight()/2;
        /*
        if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16 ){
            width = 360;
            height = 640;
        }
*/
        nexOverlayItem overlayItem = new nexOverlayItem(mOverlayKMText,width,height,0,6000);
        project.addOverlay(overlayItem);
        mOverlayItemId = overlayItem.getId();

        nexOverlayItem.setOutLine();
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_RightTop, R.drawable.layer_rotate);
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_RightBottom, R.drawable.layer_scale);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setEventHandler(mEditorListener); // null
        m_editorView = (nexEngineView)findViewById(R.id.engineview_overlay_kmtext_test);

        m_editorView.setOnTouchListener(new View.OnTouchListener() {
            nexOverlayItem.HitPoint mPosition = new nexOverlayItem.HitPoint();
            TouchMode mode = TouchMode.NONE;
            double beforeScale = -1;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (mEngine == null) {
                    return false;
                }

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    mPosition.mTime = mEngine.getCurrentPlayTimeTime();
                    mPosition.mViewX = event.getX();
                    mPosition.mViewY = event.getY();
                    mPosition.mViewWidth = v.getWidth();
                    mPosition.mViewHeight = v.getHeight();

                    if (mode == TouchMode.NONE) {
                        if (mEngine.getOverlayHitPoint(mPosition)) {
                            mode = TouchMode.HIT;
                            mOverlayItemId = mPosition.getID();
                            mLastDistance = mEngine.getProject().getOverlay(mOverlayItemId).getScaledX() -1;
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).display();
                        }
                        return true;
                    }

                    if (mEngine.getOverlayHitPoint(mPosition)) {
                        Log.d(TAG, "ACTION_DOWN=(" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was ID = " + mPosition.getID() + ",Pos= " + mPosition.getHitInPosition());
                        if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_RightTop) { //rotate
                            mode = TouchMode.ROTATE;
                        } else if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_RightBottom) { //scale
                            mode = TouchMode.SCALE;
                        } else { //move
                            mode = TouchMode.MOVE;
                        }
                    } else {
                        Log.d(TAG, "ACTION_DOWN = (" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was not hit.");
                        mode = TouchMode.NONE;
                        return true;
                    }
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    if (mode != TouchMode.HIT) {
                        mode = TouchMode.NONE;
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(false).display();
                    }
                } else if (event.getAction() == MotionEvent.ACTION_MOVE) {
                    Log.d(TAG, "ACTION_MOVE = (" + event.getX() + "," + event.getY() + "), mode = " + mode + ", view width: " + v.getWidth() + ", height: " + v.getHeight());
                    if (mode == TouchMode.MOVE) { // move
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true)
                                .setPositionX((int) (event.getX() * 1280 / v.getWidth()))
                                .setPositionY((int) (event.getY() * 720 / v.getHeight()))
                                .display();
                        mEngine.getProject().getOverlay(mOverlayItemId).setPosition((int) event.getX() * 1280 / v.getWidth(), (int) event.getY() * 720 / v.getHeight());
                        textView_X.setText("X : " + (int) event.getX() * 1280 / v.getWidth());
                        textView_Y.setText("Y : "+(int) event.getY() * 720 / v.getHeight());
                    } else if (mode == TouchMode.ROTATE) { // rotate
                        int angle = (int) getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY());
                        Log.d(TAG, "View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getAngle(" + getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) + ")");
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setRotateZ(angle).display();
                        mEngine.getProject().getOverlay(mOverlayItemId).setRotate(angle);
                        textView_degree.setText("degree : "+angle);
                    } else if (mode == TouchMode.SCALE) { // scale
                        double distance = getDistance((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) * 0.003f; /* 0.003f is standard distance */
                        distance += mLastDistance;
                        if(distance==0){
                            return true;
                        }
                        float scale = Float.parseFloat(String.format("%.2f", (float) (1 + distance)));
                        if (scale <= 0.25f)
                            scale = 0.25f;

                        if (beforeScale != scale) {
                            Log.d(TAG, "TouchMode.SCALE -> View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getDistance(" + distance + "), scale(" + scale + ")");
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setScaleX(scale).display();
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setScaleY(scale).display();

                            mEngine.getProject().getOverlay(mOverlayItemId).setScale(scale, scale);
                        }
                        beforeScale = scale;
                        textView_width.setText("Width : " + mOverlayKMText.getWidth());
                        textView_height.setText("Height : " + mOverlayKMText.getHeight());
                        textView_scale.setText("scale : "+scale);
                    }
                }
                return true;
            }
        });


        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.updateProject();
        mEngine.seek(0);

        mText = (EditText)findViewById(R.id.editText_overlay_kmtext_title);

        mText.setImeOptions(EditorInfo.IME_ACTION_DONE);

        mText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    hideSoftKeyboard(mText);
                    mOverlayKMText.setText(mText.getText().toString());
                    mEngine.buildOverlayPreview(mOverlayItemId).display();
                    textView_width.setText("Width : " + mOverlayKMText.getWidth());
                    textView_height.setText("Height : " + mOverlayKMText.getHeight());
                    textView_X.setText("X : " + mEngine.getProject().getOverlay(mOverlayItemId).getPositionX());
                    textView_Y.setText("Y : "+mEngine.getProject().getOverlay(mOverlayItemId).getPositionY());
                    textView_degree.setText("degree : "+mEngine.getProject().getOverlay(mOverlayItemId).getRotate());
                    textView_scale.setText("scale : "+mEngine.getProject().getOverlay(mOverlayItemId).getScaledX());
                    return true;
                }
                return false;
            }
        });
        mColorText = mOverlayKMText.getTextColor();
        final Button btTextColor = (Button)findViewById(R.id.button_overlay_kmtext_text_color);
        btTextColor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 1;
                new ColorPickerDialog(OverlayKMTextActivity.this, mColorChangedListener, mColorText).show();
            }
        });

        mColorChangedListener = new ColorPickerDialog.OnColorChangedListener() {
            @Override
            public void colorChanged(int color) {
                if( mSelectedColor == 1 ) {
                    mColorText = color;
                    mOverlayKMText.setTextColor(color);
                }else if( mSelectedColor == 2 ){
                    mColorShadow = color;
                    mOverlayKMText.setShadowColor(color);
                }else if( mSelectedColor == 3 ){
                    mColorOutLine = color;
                    mOverlayKMText.setOutlineColor(color);
                }else if( mSelectedColor == 4 ){
                    mColorGlow = color;
                    mOverlayKMText.setGlowColor(color);
                }
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }
        };

        String[] ids = nexFont.getFontIds();
        mFontIDs.add("Default");
        for(int i = 0 ; i < ids.length ; i++ ){
            mFontIDs.add(ids[i]);
        }

        mSpinFont = (Spinner)findViewById(R.id.spinner_overlay_kmtext_fontid);
        ArrayAdapter<String> adapterFont = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, mFontIDs);
        mSpinFont.setAdapter(adapterFont);
        mSpinFont.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.stop();
                if (position == 0) {
                    mOverlayKMText.setFontId(mDefaultFont);
                } else {
                    mOverlayKMText.setFontId(mFontIDs.get(position));
                }
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSpinSize = (Spinner)findViewById(R.id.spinner_overlay_kmtext_size);
        ArrayAdapter<Integer> adapterSize = new ArrayAdapter<Integer>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, mSizes);
        mSpinSize.setAdapter(adapterSize);
        mSpinSize.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.stop();
                mOverlayKMText.setTextSize(mSizes[position]);
                textView_width.setText("Width : " + mOverlayKMText.getWidth());
                textView_height.setText("Height : " + mOverlayKMText.getHeight());
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mChkShadow = (CheckBox)findViewById(R.id.checkBox_kmtext_shadow);
        mChkShadow.setChecked(mOverlayKMText.isEnableShadow());
        mChkShadow.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mOverlayKMText.EnableShadow(isChecked);
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }
        });

        mColorShadow = mOverlayKMText.getShadowColor();
        final Button btShadowColor = (Button)findViewById(R.id.button_kmtext_shadow_color);
        btShadowColor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 2;
                new ColorPickerDialog(OverlayKMTextActivity.this, mColorChangedListener, mColorShadow).show();
            }
        });

        mChkOutline = (CheckBox)findViewById(R.id.checkBox_kmtext_outline);
        mChkOutline.setChecked(mOverlayKMText.isEnableOutline());
        mChkOutline.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mOverlayKMText.EnableOutline(isChecked);
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }
        });

        mColorOutLine = mOverlayKMText.getOutlineColor();
        final Button btOutLineColor = (Button)findViewById(R.id.button_kmtext_outline_color);
        btOutLineColor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 3;
                new ColorPickerDialog(OverlayKMTextActivity.this, mColorChangedListener, mColorOutLine).show();
            }
        });

        mChkGlow = (CheckBox)findViewById(R.id.checkBox_kmtext_glow);
        mChkGlow.setChecked(mOverlayKMText.isEnableGlow());
        mChkOutline.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mOverlayKMText.EnableGlow(isChecked);
                mEngine.buildOverlayPreview(mOverlayItemId).display();
            }
        });

        mColorGlow = mOverlayKMText.getGlowColor();
        final Button btGlowColor = (Button)findViewById(R.id.button_kmtext_glow_color);
        btGlowColor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 4;
                new ColorPickerDialog(OverlayKMTextActivity.this, mColorChangedListener, mColorGlow).show();
            }
        });


        mSpinExpression = (Spinner)findViewById(R.id.spinner_overlay_kmtext_experssion);
        ArrayAdapter<String> adapterExpress = new ArrayAdapter<String>(getApplicationContext(),
                    R.layout.simple_spinner_dropdown_item_1, nexOverlayKineMasterExpression.getNames());
        mSpinExpression.setAdapter(adapterExpress);
        mSpinExpression.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.stop();
                mEngine.getProject().getOverlay(mOverlayItemId).setLayerExpression(nexOverlayKineMasterExpression.getExpression(position));
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        final Button bt = (Button)findViewById(R.id.button_overlay_kmtext_export);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                mExportStop = true;
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);

            }
        });

        final Button send = (Button)findViewById(R.id.button_overlay_kmtext_intent);
        send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                mExportStop = true;
                //startActivity(mEngine.getProject().makeKineMasterIntent());
                UtilityCode.launchKineMaster(OverlayKMTextActivity.this, m_listfilepath);
            }
        });


        mPlayButton = (Button)findViewById(R.id.button_overlay_kmtext_playstop);
        mPlayButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mEngine.play();
            }
        });

        final Button reset = (Button)findViewById(R.id.button_reset);
        reset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                mExportStop = true;
                //mEngine.getProject().getOverlay(mOverlayItemId).setLayerExpression(nexOverlayKineMasterExpression.getExpression(0));
                mEngine.getProject().getOverlay(mOverlayItemId).setScale( 1,  1);
                mEngine.getProject().getOverlay(mOverlayItemId).setRotate(0);
                mEngine.getProject().getOverlay(mOverlayItemId).setPosition(640, 360);
                mEngine.buildOverlayPreview(mOverlayItemId).display();
                textView_width.setText("Width : " + mOverlayKMText.getWidth());
                textView_height.setText("Height : " + mOverlayKMText.getHeight());
                textView_X.setText("X : " + mEngine.getProject().getOverlay(mOverlayItemId).getPositionX());
                textView_Y.setText("Y : " + mEngine.getProject().getOverlay(mOverlayItemId).getPositionY());
                textView_degree.setText("degree : " + mEngine.getProject().getOverlay(mOverlayItemId).getRotate());
                textView_scale.setText("scale : " + mEngine.getProject().getOverlay(mOverlayItemId).getScaledX());
            }
        });

        textView_width = (TextView)findViewById(R.id.textView_w);
        textView_width.setText("0");
        textView_height = (TextView)findViewById(R.id.textView_h);
        textView_height.setText("0");
        textView_X = (TextView)findViewById(R.id.textView_x);
        textView_X.setText("0");
        textView_Y = (TextView)findViewById(R.id.textView_y);
        textView_Y.setText("0");
        textView_degree = (TextView)findViewById(R.id.textView_degree);
        textView_degree.setText("0");
        textView_scale = (TextView)findViewById(R.id.textView_scale);
        textView_scale.setText("1");
    }

    /**----------------------------------------------
     *               Private Function               *
     ----------------------------------------------**/
    public double getAngle(int startX, int startY, int endX, int endY) {
        double dy = endY - startY;
        double dx = endX - startX;
        double angle = Math.atan(dy/dx) * (180.0/Math.PI);

        if(dx < 0.0) {
            angle += 180.0;
        } else {
            if(dy < 0.0) angle += 360.0;
        }

        return angle;
    }

    public double getDistance(int startX, int startY, int endX, int endY) {
        double distance = Math.sqrt(Math.pow(Math.abs(endX - startX), 2) + Math.pow(Math.abs(endY - startY), 2));

        // 1 quadrant
        if(startX < endX && startY > endY) {
            return 1;
        }

        // 3 quadrant
        if(startX > endX && startY < endY) {
            return 1;
        }

        // 2 quadrant
        if(startX > endX && startY > endY) {
            distance *= -1;
        }

        return distance;
    }

    void hideSoftKeyboard(EditText input) {
        input.setInputType(0);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(input.getWindowToken(), 0);
    }

}
