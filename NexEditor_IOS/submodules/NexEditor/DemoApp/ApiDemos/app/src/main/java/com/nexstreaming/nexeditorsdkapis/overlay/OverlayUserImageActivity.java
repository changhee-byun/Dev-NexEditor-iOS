/******************************************************************************
 * File Name        : OverlayUserImageActivity.java
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
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.BounceInterpolator;
import android.view.animation.Interpolator;
import android.view.animation.OvershootInterpolator;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexAnimate;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterExpression;
import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterText;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexUtils;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.util.ArrayList;


public class OverlayUserImageActivity extends Activity {
    private static final String TAG = "UserImageOverlay";
    private nexEngineListener mEditorListener = null;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngin;
    private nexEngineView m_editorView;
    private Spinner mSpin;
    private int mOverlayItemId;
    private boolean mIsExport;
    private Button mPlayButton;
    private static final boolean sLayerExpressionInternal = false;

    private static String sLayerExpressions[] = {
            "None", "Fade", "Pop" ,"Slide", "Spin" ,"Drop","Scale","Floating","Drifting","Squishing","FreeType"
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_user_image_overlay);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        nexProject project = new nexProject();
        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexClip clip = new nexClip(m_listfilepath.get(0));

        if (clip.getClipType() != nexClip.kCLIP_TYPE_VIDEO) {
            Toast.makeText(getApplicationContext(), "No Video Clips.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if (clip.getTotalTime() < 6000) {
            Toast.makeText(getApplicationContext(), "Video is short.( more 6 second)", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        int maxcount = 2;
        for (int i = 0; i < maxcount; i++) {
            project.add(nexClip.dup(clip));
            project.getClip(i, true).getVideoClipEdit().setTrim(2000, 5000);
            project.getClip(i, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        }

//        nexEffectLibrary fxlib =  nexEffectLibrary.getEffectLibrary(this);
        mSpin = (Spinner)findViewById(R.id.spinner_user_image_overlay);
        ArrayAdapter<String> adapter = null;
        if( sLayerExpressionInternal ){
            adapter = new ArrayAdapter<String>(getApplicationContext(),
                    R.layout.simple_spinner_dropdown_item_1, nexOverlayKineMasterExpression.getNames());
        }else {
            adapter = new ArrayAdapter<String>(getApplicationContext(),
                    R.layout.simple_spinner_dropdown_item_1, sLayerExpressions);
        }
        mSpin.setAdapter(adapter);
        mSpin.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngin.stop();
                if( sLayerExpressionInternal ){
                    mEngin.getProject().getOverlay(mOverlayItemId).setLayerExpression(nexOverlayKineMasterExpression.getExpression(position));

                }else {
                    presetExpression(position);
                }
                //mEngin.play();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        final Button bt = (Button) findViewById(R.id.button_user_image_overlay_export);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                mIsExport = true;
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);

            }
        });

        final Button send = (Button)findViewById(R.id.button_user_image_overlay_intent);
        send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                mIsExport = true;
                //startActivity(mEngin.getProject().makeKineMasterIntent());
                UtilityCode.launchKineMaster(OverlayUserImageActivity.this, m_listfilepath);
            }
        });

        nexOverlayImage overlay = new nexOverlayImage("testOverlay", OverlayUserImageActivity.this, R.drawable.myavatar_17004763_1);
        nexOverlayItem overlayItem = new nexOverlayItem(overlay, 200, 200, 0, 0 + 6000);

        if (nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayAnimateLimited)) {
            nexAnimate ani = nexAnimate.getAnimateImages(
                    0, 5000, R.drawable.myavatar_17004763_1
                    , R.drawable.myavatar_17004763_2
                    , R.drawable.myavatar_17004763_3
                    , R.drawable.myavatar_17004763_4
                    , R.drawable.myavatar_17004763_5
                    , R.drawable.myavatar_17004763_6
                    , R.drawable.myavatar_17004763_7
                    , R.drawable.myavatar_17004763_8
                    , R.drawable.myavatar_17004763_9
                    , R.drawable.myavatar_17004763_10
            );
            overlayItem.addAnimate(ani);
        }

        nexAnimate mv = nexAnimate.getMove(0, 5000, new nexAnimate.MoveTrackingPath() {
            @Override
            public float getTranslatePosition(int coordinate, float timeRatio) {
                if (coordinate == nexAnimate.kCoordinateX) {
                    return (nexApplicationConfig.getAspectProfile().getWidth()-320) * timeRatio;
                } else if (coordinate == nexAnimate.kCoordinateY) {
                    return (nexApplicationConfig.getAspectProfile().getHeight()-320) * timeRatio;
                }
                return 0;
            }
        });
        overlayItem.addAnimate(mv);

        nexAnimate ap = nexAnimate.getAlpha(4000, 2000, 1, 0);
        overlayItem.addAnimate(ap);

        nexAnimate ro = nexAnimate.getRotate(2000, 3000, true, 360, null);
        overlayItem.addAnimate(ro);

        nexAnimate sc = nexAnimate.getScale(0, 3000, 2, 2);
        overlayItem.addAnimate(sc);

        project.addOverlay(overlayItem);


        if (nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
            nexOverlayImage overlay2 = new nexOverlayImage("testOverlay2", OverlayUserImageActivity.this, R.drawable.ilove0001);
            nexOverlayItem overlayItem2 = new nexOverlayItem(overlay2, nexOverlayItem.AnchorPoint_RightTop, false,overlay2.getWidth()*-1, overlay2.getHeight(), 0, 0 + 6000);

            if (nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayAnimateLimited)) {
                nexAnimate ani2 = nexAnimate.getAnimateImages(
                        0, 5000, R.drawable.ilove0001
                        , R.drawable.ilove0002
                        , R.drawable.ilove0003
                        , R.drawable.ilove0004
                        , R.drawable.ilove0005
                        , R.drawable.ilove0006
                        , R.drawable.ilove0007
                        , R.drawable.ilove0008
                        , R.drawable.ilove0009
                        , R.drawable.ilove0010
                );
                overlayItem2.addAnimate(ani2);
            }

            project.addOverlay(overlayItem2);
        }

        int width = nexApplicationConfig.getAspectProfile().getWidth();
        int height = nexApplicationConfig.getAspectProfile().getHeight();
        int x = width / 2;
        int y = height / 2;

        if (nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
            nexOverlayImage textOverlay = new nexOverlayImage("text", new OverlayText(getApplicationContext(), "Nexstreaming", 10002, width, height, 1));
            nexOverlayItem overlayItem3 = new nexOverlayItem(textOverlay, x, y, 0, 6000);

            project.addOverlay(overlayItem3);
        }

        if (nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
            nexOverlayImage textOverlay2 = new nexOverlayImage("text2", new OverlayText(getApplicationContext(), "Yoon OK?", 10003, width, height, 0.5f));
            nexOverlayItem overlayItem4 = new nexOverlayItem(textOverlay2, x, y + 100, 0, 6000);
            project.addOverlay(overlayItem4);
        }

        nexOverlayKineMasterText builtinText = new nexOverlayKineMasterText(getApplicationContext(),"nexOverlayStandardText", 50);
        builtinText.setTextColor(0xffff0000);
        String[] fontIds = nexFont.getFontIds();
        if( fontIds.length > 0 ) {
            builtinText.setFontId(fontIds[0]);
        }
        nexOverlayItem overlayItem6 = new nexOverlayItem(builtinText,640,180,0,6000);
        overlayItem6.setRotate(45);
        overlayItem6.setScale(2,2);

        project.addOverlay(overlayItem6);

        mOverlayItemId = overlayItem6.getId();

        nexOverlayPreset ovlib = nexOverlayPreset.getOverlayPreset();
        String[] preset = ovlib.getIDs();
        if(preset != null) {
            Log.d(TAG, "preset id0 ="+preset[0]);
            nexOverlayItem overlayItem7 = new nexOverlayItem(preset[0],nexOverlayItem.AnchorPoint_LeftBottom,true,0.25f,-0.25f,0,6000);
            project.addOverlay(overlayItem7);
        } else {
            Log.d(TAG, "preset is none");
        }

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEditorListener); // null
        m_editorView = (nexEngineView) findViewById(R.id.engineview_user_image_overlay);
        mEngin.setView(m_editorView);
        mEngin.setProject(project);

        mPlayButton = (Button) findViewById(R.id.button_user_image_overlay_playstop);
        mPlayButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mEngin.play();
            }
        });
    }

    class FreeTypeExpress extends nexAnimate {
        boolean binit;
        int itemDuration;
        Interpolator interp;

        FreeTypeExpress(){
            super(0, 3000);
        }

        @Override
        protected boolean onFreeTypeAnimate(int time, nexOverlayItem overlay) {
            if( !binit ) {
                resetFreeTypeAnimate();
                mAlpha = overlay.getAlpha();
                mRotateDegreeZ = overlay.getRotateZ();
                mScaledX = overlay.getScaledX();
                mScaledY = overlay.getScaledY();
                itemDuration = overlay.getEndTime() - overlay.getStartTime();
                interp = new AccelerateDecelerateInterpolator();
                binit = true;
            }

            float timeRatio  = (float)time/(float)3000;
            if( timeRatio > 1 ){
                timeRatio = 1;
            }

            float p = interp.getInterpolation(timeRatio);
            mAlpha = p;
            mRotateDegreeZ = 360 *p;
            mScaledX = p;
            return  true;
        }
    }

    void presetExpression(int index){
        nexOverlayItem item = mEngin.getProject().getOverlay(mOverlayItemId);

        final int duration = item.getEndTime() - item.getStartTime();
        int expressDuration = 1000;
        if( duration < expressDuration*2 ){
            expressDuration = duration/2;
        }
        switch (index ){
            case 0 :
                item.clearAnimate();
                break;
            case 1 :
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration-expressDuration, expressDuration,1,0).setInterpolator(new AccelerateDecelerateInterpolator()));
                break;
            case 2 :
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration,0,1).setInterpolator(new OvershootInterpolator()));
                item.addAnimate(nexAnimate.getScale(0, expressDuration, 0, 0, 1, 1).setInterpolator(new OvershootInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateInterpolator()));
                item.addAnimate(nexAnimate.getScale(duration - expressDuration, expressDuration, 1, 1, 2, 2).setInterpolator(new AccelerateInterpolator()));
                break;

            case 3 :
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getMove(0, expressDuration, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return 100 - (100 * timeRatio);
                        }
                        return 0;
                    }
                }).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getMove(duration - expressDuration, expressDuration, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return 100 * timeRatio;
                        }
                        return 0;
                    }
                }).setInterpolator(new AccelerateDecelerateInterpolator()));

                break;

            case 4:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getRotate(0, 1000, true, 360 * 2, null).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getRotate(duration - expressDuration, 1000, false, 360 * 2, null).setInterpolator(new AccelerateDecelerateInterpolator()));
                break;

            case 5:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new BounceInterpolator()));
                item.addAnimate(nexAnimate.getMove(0, expressDuration, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateY) {
                            return 1200 - (1200 * timeRatio);
                        }
                        return 0;
                    }
                }).setInterpolator(new BounceInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));

                break;

            case 6:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getScale(0, expressDuration, 0, 0, 1, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getScale(duration - expressDuration, expressDuration, 1, 1, 0, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                break;
            case 7:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getMove(0, duration, new nexAnimate.MoveTrackingPath() {
                    private float _cx = 0;
                    private float _cy = 0;
                    private float _dx = 0;
                    private float _dy = 0;
                    private int time = 0;
                    private boolean _cal = true;

                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (_cal) {
                            float cx = _cx / 500 + duration * 10;
                            float cy = _cy / 500 + duration * 10;
                            double dx = 40 * nexUtils.noise(cx, cy, (float) time / 500);
                            double dy = 40 * nexUtils.noise(cx + 7534.5431, cy + 123.432, (float) time / 500);
                            double dx2 = 10 * nexUtils.noise(cx + 543, cy + 823.25, (float) time / 300 + 234);
                            double dy2 = 10 * nexUtils.noise(cx + 734.5431, cy + 13.432, (float) time / 300 + 567);

                            _cx = cx;
                            _cy = cy;
                            time += 33;
                            _dx = (float) (dx + dx2);
                            _dy = (float) (dy + dy2);
                            _cal = false;
                            //Log.d(TAG, "noise move dx=" + _dx + ", dy=" + _dy);
                        }
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return _dx;

                        } else if (coordinate == nexAnimate.kCoordinateY) {
                            return _dy;
                        } else {
                            _cal = true;
                        }
                        return 0;
                    }
                }).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));

                break;

            case 8:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getMove(0, duration, new nexAnimate.MoveTrackingPath() {
                    private float _cx = 0;
                    private float _cy = 0;
                    private float _dx = 0;
                    private float _dy = 0;
                    private int time = 0;
                    private boolean _cal = true;
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if( _cal ) {
                            float cx = _cx / 500 + duration*10;
                            float cy = _cy / 500 + duration*10;
                            double dx = 40 * nexUtils.noise(cx, cy, (float) time / 1000);
                            double dy = 40 * nexUtils.noise(cx + 7534.5431, cy + 123.432, (float) time / 1000);
                            double dx2 = 10 * nexUtils.noise(cx + 543, cy + 823.25, (float) time / 500 + 234);
                            double dy2 = 10 * nexUtils.noise(cx + 734.5431, cy + 13.432, (float) time / 500 + 567);

                            _cx = cx;
                            _cy = cy;
                            time += 33;
                            _dx = (float) (dx + dx2);
                            _dy = (float) (dy + dy2);
                            _cal = false;
                            //Log.d(TAG,"noise move dx="+_dx+", dy="+_dy);
                        }
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return _dx;

                        }else if( coordinate == nexAnimate.kCoordinateY ) {
                            return _dy;
                        }else{
                            _cal = true;
                        }
                        return 0;
                    }
                }).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));

                break;
            case 9:
                item.clearAnimate();
                item.addAnimate(nexAnimate.getAlpha(0, expressDuration, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                item.addAnimate(nexAnimate.getScale(0, duration, new nexAnimate.MoveTrackingPath() {
                    int time = 0;
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX){
                            return (float) (Math.sin((float)time/400f)*0.2f );
                        }else if( coordinate == nexAnimate.kCoordinateY ){
                            return (float) (Math.cos((float)time/400f)*0.2f );
                        }else{
                            time += 33;
                        }
                        return 1;
                    }
                }).setInterpolator(new AccelerateDecelerateInterpolator()));

                item.addAnimate(nexAnimate.getAlpha(duration - expressDuration, expressDuration, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                break;
            case 10:
                item.clearAnimate();
                item.addAnimate(new FreeTypeExpress());

                break;
            default:
                break;
        }
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart");
        if( mEngin != null) {
            mEngin.setEventHandler(mEditorListener); // null
        }
        mIsExport = false;
        super.onStart();
    }

    @Override
    protected void onStop() {
        mEngin.stop();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
