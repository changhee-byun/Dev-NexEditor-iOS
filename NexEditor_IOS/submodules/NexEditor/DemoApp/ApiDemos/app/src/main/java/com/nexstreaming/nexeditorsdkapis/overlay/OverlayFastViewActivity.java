/******************************************************************************
 * File Name        : OverlayFastViewActivity.java
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
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayFilter;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;

import java.util.List;
public class OverlayFastViewActivity extends Activity {
    static private String TAG = "OverlayEdit";
    SeekBar mXposSeekBar;
    SeekBar mYposSeekBar;
    SeekBar mRotateSeekBar;
    SeekBar mRotateXSeekBar;
    SeekBar mRotateYSeekBar;
    SeekBar mAlphaSeekBar;
    SeekBar mScaleXSeekBar;
    SeekBar mScaleYSeekBar;

    TextView mXposTextView;
    TextView mYposTextView;
    TextView mAlphaTextView;
    TextView mRotateTextView;
    TextView mRotateXTextView;
    TextView mRotateYTextView;
    TextView mScaleXTextView;
    TextView mScaleYTextView;

    RadioButton mResourceRadio;
    RadioButton mFilterRadio;
    RadioButton mPresetRadio;
    RadioButton mImageRadio;

    int mRotateX = 0;
    int mRotateY = 0;
    int mRotateZ = 0;
    private nexEngine mEngine;
    private nexEngineListener mEditorListener = null;
    private nexEngineView m_editorView;

    private int mOverlayId;
    private int mRadioIndex = 0;
    private String mExternalImagePath;

    private double oldDist = -1;
    private double newDist = -1;
    private double Dist = 1;

    private enum TouchMode {
        ROTATE,
        SCALE,
        MOVE,
        NONE
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_overlay_fast_view);

        // set screen timeout to never
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
        }else{
            project.add(nexClip.getSolidClip(0xffff00ff));
        }

        nexOverlayItem.setOutLine();
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_RightTop, R.drawable.layer_rotate);

        m_editorView = (nexEngineView) findViewById(R.id.engineview_overlay_fastview);
        m_editorView.setOnTouchListener(new View.OnTouchListener() {
            nexOverlayItem.HitPoint mPosition = new nexOverlayItem.HitPoint();
            TouchMode mode;
            double beforeScale = -1;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (mEngine == null) {
                    return false;
                }
                int act = event.getAction();
                if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_DOWN) {
                    mPosition.mTime = 0;
                    mPosition.mViewX = event.getX();
                    mPosition.mViewY = event.getY();
                    mPosition.mViewWidth = v.getWidth();
                    mPosition.mViewHeight = v.getHeight();
                    if (mEngine.getOverlayHitPoint(mPosition)) {
                        Log.d(TAG, "ACTION_DOWN=(" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was ID = " + mPosition.getID() + ",Pos= " + mPosition.getHitInPosition());
                        if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_RightTop) { //rotate
                            mode = TouchMode.ROTATE;
                        } else { //move
                            mode = TouchMode.MOVE;
                        }
                    } else {
                        Log.d(TAG, "ACTION_DOWN = (" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was not hit.");
                        mode = TouchMode.NONE;
                        return true;
                    }
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP) {
                    mode = TouchMode.NONE;
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_MOVE) {
                    Log.d(TAG, "ACTION_MOVE = (" + event.getX() + "," + event.getY() + "), mode = " + mode + ", view width: " + v.getWidth() + ", height: " + v.getHeight());
                    if (mode == TouchMode.MOVE) { // move
                        mEngine.buildOverlayPreview(mOverlayId).setOutline(true)
                                .setPositionX((int) (event.getX() * nexApplicationConfig.getAspectProfile().getWidth() / v.getWidth()))
                                .setPositionY((int) (event.getY() * nexApplicationConfig.getAspectProfile().getHeight() / v.getHeight()))
                                .display();
                        mXposSeekBar.setProgress((int) (event.getX() * nexApplicationConfig.getAspectProfile().getWidth() / v.getWidth()));
                        mYposSeekBar.setProgress((int) (event.getY() * nexApplicationConfig.getAspectProfile().getHeight() / v.getHeight()));

                    } else if (mode == TouchMode.ROTATE) { // rotate
                        int angle = (int) getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY());
                        Log.d(TAG, "View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getAngle(" + getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) + ")");
                        mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateZ(angle).display();
                        mRotateSeekBar.setProgress(angle);
                    } else if (mode == TouchMode.SCALE) { // scale
                        newDist = spacing(event);
                        //double distance = getDistance((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) * 0.003f; /* 0.003f is standard distance */
                        double distance = (newDist - oldDist) * 0.003f;/* 0.003f is standard distance */
                        if (distance == 0) {
                            return true;
                        }
                        float scale = Float.parseFloat(String.format("%.2f", (float) (Dist + distance)));
                        if (scale <= 0.25f)
                            scale = 0.25f;

                        if (beforeScale != scale) {
                            Log.d(TAG, "TouchMode.SCALE -> View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getDistance(" + distance + "), scale(" + scale + "), Dist(" + Dist + ")");
                            mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleX(scale).display();
                            mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleY(scale).display();
                            mScaleXSeekBar.setProgress((int) (scale / 0.25));
                            mScaleYSeekBar.setProgress((int) (scale / 0.25));
                        }
                        beforeScale = scale;
                    }
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_POINTER_DOWN) {
                    mode = TouchMode.SCALE;
                    oldDist = spacing(event);
                    //nexOverlayItem.setOutLineIcon(getApplicationContext(), nexOverlayItem.kOutLine_Pos_RightBottom, R.drawable.layer_scale);
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_POINTER_UP) {
                    mode = TouchMode.NONE;
                    Dist = Dist + (newDist - oldDist) * 0.003f;
                    if (Dist <= 0.25) {
                        Dist = 0.25;
                    }
                }
                return true;
            }
        });

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.updateProject();

        m_editorView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                mEngine.seek(1000);
            }

            @Override
            public void onEngineViewSizeChanged(int width, int height) {

            }

            @Override
            public void onEngineViewDestroyed() {

            }
        });

        mXposTextView = (TextView) findViewById(R.id.textview_overlay_fastview_x);
        mYposTextView = (TextView) findViewById(R.id.textview_overlay_fastview_y);
        mRotateTextView = (TextView) findViewById(R.id.textview_overlay_fastview_rotate);
        mRotateXTextView = (TextView) findViewById(R.id.textview_overlay_fastview_rotate_x);
        mRotateYTextView = (TextView) findViewById(R.id.textview_overlay_fastview_rotate_y);
        mAlphaTextView = (TextView) findViewById(R.id.textview_overlay_fastview_alpha);
        mScaleXTextView = (TextView) findViewById(R.id.textview_overlay_fastview_scalex);
        mScaleYTextView = (TextView) findViewById(R.id.textview_overlay_fastview_scaley);

        mXposSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_x);
        mXposSeekBar.setMax(nexApplicationConfig.getAspectProfile().getWidth());
        mXposSeekBar.setProgress(nexApplicationConfig.getAspectProfile().getWidth() / 2);
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
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setPosition(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getPositionY());
            }
        });


        mYposSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_y);
        mYposSeekBar.setMax(nexApplicationConfig.getAspectProfile().getHeight());
        mYposSeekBar.setProgress(nexApplicationConfig.getAspectProfile().getHeight() / 2);
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
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setPosition(mEngine.getProject().getOverlay(mOverlayId).getPositionX(), lastProgress);
            }
        });

        mRotateSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_rotate);
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
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                //mEngine.getProject().getOverlay(mOverlayId).setRotate(lastProgress);
                mRotateZ = lastProgress;
                mEngine.getProject().getOverlay(mOverlayId).setRotate(mRotateX,mRotateY,mRotateZ);
            }
        });

        mRotateXSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_rotate_x);
        mRotateXSeekBar.setMax(359);

        mRotateXSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mRotateXTextView.setText("" + progress);
                lastProgress = progress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateX(progress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mRotateX = lastProgress;
                //mEngine.getProject().getOverlay(mOverlayId).setRotate(lastProgress);
                mEngine.getProject().getOverlay(mOverlayId).setRotate(mRotateX,mRotateY,mRotateZ);
            }
        });


        mRotateYSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_rotate_y);
        mRotateYSeekBar.setMax(359);

        mRotateYSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mRotateYTextView.setText("" + progress);
                lastProgress = progress;
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateY(progress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mRotateY = lastProgress;
                //mEngine.getProject().getOverlay(mOverlayId).setRotate(lastProgress);
                mEngine.getProject().getOverlay(mOverlayId).setRotate(mRotateX,mRotateY,mRotateZ);
            }
        });


        mAlphaSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_alpha);
        mAlphaSeekBar.setMax(100);
        mAlphaSeekBar.setProgress(100);
        mAlphaSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            float lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lastProgress = (float) progress / 100f;
                mAlphaTextView.setText("" + lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setAlpha(lastProgress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setAlpha(lastProgress);
            }
        });

        mScaleXSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_scalex);
        mScaleXSeekBar.setMax(10);
        mScaleXSeekBar.setProgress(4);
        mScaleXSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            float lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                lastProgress = progress * 0.25f;
                mScaleXTextView.setText("" + lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleX(lastProgress).display();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                mEngine.getProject().getOverlay(mOverlayId).setScale(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getScaledY());
            }
        });

        mScaleYSeekBar = (SeekBar) findViewById(R.id.seekbar_overlay_fastview_scaley);
        mScaleYSeekBar.setMax(10);
        mScaleYSeekBar.setProgress(4);
        mScaleYSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            float lastProgress;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                lastProgress = progress * 0.25f;
                mScaleYTextView.setText("" + lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleY(lastProgress).display();

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        mResourceRadio = (RadioButton) findViewById(R.id.radioButton_resource);
        mResourceRadio.setChecked(true);
        mResourceRadio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked){
                    setImageSource(0);
                }
            }
        });

        mFilterRadio = (RadioButton) findViewById(R.id.radioButton_filter);
        mFilterRadio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked){
                    setImageSource(1);
                }
            }
        });

        mPresetRadio = (RadioButton) findViewById(R.id.radioButton_preset);
        mPresetRadio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked){
                    setImageSource(2);
                }
            }
        });

        mImageRadio = (RadioButton) findViewById(R.id.radioButton_image);
        mImageRadio.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("image/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });
        setImageSource(0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String title = null;
            String artist = null;
            int duration = 0;

            if (clipData == null) {
                mExternalImagePath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                item = clipData.getItemAt(0);
                uri = item.getUri();
                mExternalImagePath = ConvertFilepathUtil.getPath(this, uri);
            }
            setImageSource(3);
        }
    }
    /**
     * ----------------------------------------------
     * Private Function               *
     * ----------------------------------------------
     **/
    private void reset(){
        mXposSeekBar.setProgress(nexApplicationConfig.getAspectProfile().getWidth() / 2);
        mYposSeekBar.setProgress(nexApplicationConfig.getAspectProfile().getHeight() / 2);
        mRotateSeekBar.setProgress(0);
        mAlphaSeekBar.setProgress(100);
        mScaleXSeekBar.setProgress(4);
        mScaleYSeekBar.setProgress(4);
        if( mEngine.getProject() != null ){
            mEngine.getProject().clearOverlay();
        }

    }

    private void setImageSource(int index) {
        reset();
        mRadioIndex = index;
        if(nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited))
        {
            if( mRadioIndex == 0 ) {
                nexOverlayImage overlay = new nexOverlayImage("resource", this, R.drawable.ilove0001);
                nexOverlayItem overlayItem = new nexOverlayItem(overlay, nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2, 0, 0 + 6000);
                mOverlayId = overlayItem.getId();

                mEngine.getProject().addOverlay(overlayItem);
            }else if( mRadioIndex == 1 ){
                nexOverlayFilter[] filters = nexEffectLibrary.getEffectLibrary(getApplicationContext()).getOverlayFilters();
                for( nexOverlayFilter filter : filters ){
                    if( filter.getId().compareTo("com.nexstreaming.editor.blurall") == 0 ){
                        List<nexEffectOptions.RangeOpt> opts= filter.getEffectOption().getRangeOptions();
                        for( nexEffectOptions.RangeOpt ro : opts ){
                            ro.setValue(5);
                        }
                        filter.setWidth(300);
                        filter.setHeight(300);
                        nexOverlayItem overlayItem = new nexOverlayItem(filter,nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2,0,6000);
                        mOverlayId = overlayItem.getId();
                        mEngine.getProject().addOverlay(overlayItem);
                    }
                }
            }else if( mRadioIndex == 2 ){
                nexOverlayPreset ovlib = nexOverlayPreset.getOverlayPreset();
                String[] preset = ovlib.getIDs();
                if (preset != null) {
                    nexOverlayItem overlayItem = new nexOverlayItem(preset[0], nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2, 0, 6000);
                    mOverlayId = overlayItem.getId();
                    mEngine.getProject().addOverlay(overlayItem);
                } else {
                    Log.d(TAG, "preset is none");
                }
            }else if( mRadioIndex == 3 ){
                if( mExternalImagePath != null ) {
                    nexOverlayImage overlay = new nexOverlayImage("external", mExternalImagePath);
                    nexOverlayItem overlayItem = new nexOverlayItem(overlay,nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2, 0, 6000);
                    mOverlayId = overlayItem.getId();
                    mEngine.getProject().addOverlay(overlayItem);
                }
            }
        }
        else
        {
            nexOverlayPreset ovlib = nexOverlayPreset.getOverlayPreset();
            String[] preset = ovlib.getIDs();
            if (preset != null) {
                Log.d(TAG, "preset id0 =" + preset[0]);
                nexOverlayItem overlayItem = new nexOverlayItem(preset[0], nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2, 0, 6000);
                mOverlayId = overlayItem.getId();
                mEngine.getProject().addOverlay(overlayItem);
            } else {
                Log.d(TAG, "preset is none");
            }
        }
        mEngine.updateProject();
        mEngine.seek(1000);
    }

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
    private double spacing(MotionEvent event) {
        double x = event.getX(0) - event.getX(1);
        double y = event.getY(0) - event.getY(1);
        return Math.sqrt(x * x + y * y);
    }
    protected void onStop() {
        mEngine.stop();

        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
