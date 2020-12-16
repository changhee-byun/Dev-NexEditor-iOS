/******************************************************************************
 * File Name        : OverlayMaskTestActivity.java
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
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.nexstreaming.nexeditorsdk.nexAnimate;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;

public class OverlayMaskTestActivity extends Activity {
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private nexProject mProject = new nexProject();
    private final static int max_input_count =4;
    private final static int play_duration = 6000;
    private final static int frame_margin = 5;
    private Bitmap mMaskImage;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_overlay_mask_test);

        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        mProject.add(nexClip.getSolidClip(0xff000000));
        mProject.getClip(0,true).setImageClipDuration(play_duration);

        int overlayWidth = nexApplicationConfig.getAspectProfile().getWidth();
        int overlayHeight = nexApplicationConfig.getAspectProfile().getHeight();

        makeMaskImage();

        for(  int i = 0 ; i < m_listfilepath.size() ; i++ ) {
            if( i == max_input_count )
                break;
            nexOverlayImage overlayImage = new nexOverlayImage("image"+i,m_listfilepath.get(i));
            //overlayImage.resizeBitmap(overlayWidth,overlayHeight);
            nexOverlayItem overlayItem = new nexOverlayItem(overlayImage,nexOverlayItem.AnchorPoint_MiddleMiddle,false,0,0,0,play_duration);
            overlayItem.getMask().setState(true);
            overlayItem.getMask().setMaskImage(mMaskImage);
            if( i == 0 ) {
                overlayItem.addAnimate(nexAnimate.getMove(0, play_duration, new nexAnimate.MoveTrackingPath() {

                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        int overlayWidth = nexApplicationConfig.getAspectProfile().getWidth();
                        int overlayHeight = nexApplicationConfig.getAspectProfile().getHeight();

                        if(coordinate == nexAnimate.kCoordinateX){
                            return (overlayWidth*timeRatio/2)*-1;
                        }
                        return 0;
                    }
                }));
                overlayItem.getMask().setPosition(0+frame_margin,0+frame_margin,overlayWidth/2-frame_margin,overlayHeight/2-frame_margin );
            }else if( i == 1 ){
                overlayItem.getMask().setPosition(overlayWidth/2+frame_margin,0+frame_margin,overlayWidth-frame_margin,overlayHeight/2-frame_margin );
                overlayItem.addAnimate(nexAnimate.getScale(0,play_duration,2,2));
            }else if( i == 2 ){
                overlayItem.getMask().setPosition(0+frame_margin,overlayHeight/2+frame_margin,overlayWidth/2-frame_margin,overlayHeight-frame_margin );
                overlayItem.addAnimate(nexAnimate.getRotate(0,play_duration,true,360,null));
            }else if( i == 3 ){
                overlayItem.getMask().setPosition(overlayWidth/2+frame_margin,overlayHeight/2+frame_margin,overlayWidth-frame_margin,overlayHeight-frame_margin );
            }
            mProject.addOverlay(overlayItem);

        }

        final Button btPlay = (Button)findViewById(R.id.button_overlay_mask_play);
        btPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.play();
            }
        });

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        m_editorView = (nexEngineView) findViewById(R.id.engineview_overlay_mask);
        m_editorView.setBlackOut(true);
        mEngine.setView(m_editorView);
        mEngine.setProject(mProject);
        mEngine.updateProject();

        mEngine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {
                mEngine.seek(0);
            }
        });
    }

    private void makeMaskImage(){
        int overlayWidth = nexApplicationConfig.getAspectProfile().getWidth();
        int overlayHeight = nexApplicationConfig.getAspectProfile().getHeight();

        if( mMaskImage != null ){
            mMaskImage.recycle();
        }
        mMaskImage = Bitmap.createBitmap(overlayWidth/2, overlayHeight/2, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(mMaskImage);

        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL_AND_STROKE);
        paint.setColor(0xff000000);
        canvas.drawRect(0,0,overlayWidth/2,overlayHeight/2,paint);
        paint.setColor(0xffffffff);
        canvas.drawCircle(overlayWidth/4,overlayHeight/4,overlayHeight/4,paint);
    }

    @Override
    protected void onDestroy() {
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
