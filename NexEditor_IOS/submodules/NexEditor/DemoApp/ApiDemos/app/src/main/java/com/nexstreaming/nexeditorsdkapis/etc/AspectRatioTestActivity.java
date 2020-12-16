/******************************************************************************
 * File Name        : AspectRatioTestActivity.java
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
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import java.util.ArrayList;

public class AspectRatioTestActivity extends Activity {
    private final static String TAG = "AspectRatioTest";
    private nexEngine mEngine;
    private ArrayList<String> m_listfilepath;
    private nexEngineView m_editorView;
    private int lnitAspectMode;
    private TextView mTextViewResult;
    private StringBuilder mResult;
    private boolean mStartPlay = false;
    private boolean mEngineViewAvailable = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mResult = new StringBuilder();
        setResultMessage("onCreate start");

        lnitAspectMode = nexApplicationConfig.getAspectRatioMode();
        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");


        setResultMessage("clip create start");
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if( clip == null ){
            Toast.makeText(getApplicationContext(), "Not supported clip : " + m_listfilepath.get(0), Toast.LENGTH_SHORT).show();
            return ;
        }
        setResultMessage("clip create end");
        int rotate = 0;
        int width = 0;
        int height = 0;
        rotate = clip.getRotateInMeta();
        width = clip.getWidth();
        height = clip.getHeight();

        setResultMessage("clip get Meta data = ("+width +" X "+height+" , RM : "+ rotate);

        if( clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270 ){
            nexApplicationConfig.setAspectProfile(new nexAspectProfile(height,width)); // note : set Aspect ratio
        }else{
            nexApplicationConfig.setAspectProfile(new nexAspectProfile(width,height)); // note : set Aspect ratio
        }

        setResultMessage("Layer resolution = "+ nexApplicationConfig.getAspectProfile().getWidth()+" X "+nexApplicationConfig.getAspectProfile().getHeight());

        nexProject project = new nexProject();
        project.add(clip);
        project.getLastPrimaryClip().setRotateDegree(clip.getRotateInMeta());
        project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        int layerWidth = nexApplicationConfig.getAspectProfile().getWidth();
        int layerHeight = nexApplicationConfig.getAspectProfile().getHeight();

        nexOverlayImage overlayImage = new nexOverlayImage("solid1",0xff00ffff);
        nexOverlayItem overlayItem = new nexOverlayItem(overlayImage,nexOverlayItem.AnchorPoint_MiddleMiddle,true,0,0.2f,0,6000);
        overlayItem.setScale(2,2);
        overlayItem.setRotate(45);
        project.addOverlay(overlayItem);
        setResultMessage("add solid overlay.");

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.updateScreenMode(); //note : update aspect ratio.

        setContentView(R.layout.activity_aspect_ratio_test); // note : called after nexApplicationConfig.setAspectProfile()

        mTextViewResult = (TextView)findViewById(R.id.textview_aspect_result);
        updateTextViewResult();
        mEngine.setProject(project);

        m_editorView = (nexEngineView)findViewById(R.id.engineview_aspect);
        mEngine.setView(m_editorView);
        m_editorView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                mEngineViewAvailable = true;
                setResultMessage("onEngineViewAvailable("+width+","+height+"). startPlay="+mStartPlay);

                if( !mStartPlay ) {
                    mStartPlay = true;
                    mEngine.play();

                }
            }

            @Override
            public void onEngineViewSizeChanged(int width, int height) {

            }

            @Override
            public void onEngineViewDestroyed() {
                setResultMessage("onEngineViewDestroyed().");
                mEngineViewAvailable = false;
            }
        });


        mEngine.setEventHandler(new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {
                if( newState == nexEngine.nexPlayState.IDLE.getValue() ) {
                    mStartPlay = false;
                }
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

            }

            @Override
            public void onPlayEnd() {
                setResultMessage("onPlayEnd");
                updateTextViewResult();
            }

            @Override
            public void onPlayFail(int err, int iClipID) {

            }

            @Override
            public void onPlayStart() {
                setResultMessage("onPlayStart");
                updateTextViewResult();
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {

            }

            @Override
            public void onEncodingProgress(int percent) {

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
        });
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        if( mStartPlay ) {
            mStartPlay = false;
            mEngine.stop();
        }

        super.onPause();
    }

    @Override
    protected void onResume() {
        setResultMessage("onResume mEngineViewAvailable="+mEngineViewAvailable);
        if( mEngineViewAvailable ){
            mStartPlay = true;
            mEngine.play();
        }
        super.onResume();
    }


    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        nexApplicationConfig.setAspectMode(lnitAspectMode);
        super.onDestroy();
    }

    void setResultMessage(String str){
        mResult.append("["+System.currentTimeMillis()+"]"+str+"\n");
    }

    void updateTextViewResult()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTextViewResult.setText(mResult.toString());
            }
        });
    }
}
