/******************************************************************************
 * File Name        : ThemeViewTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.surface;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.util.ArrayList;

public class ThemeViewTestActivity extends Activity {
    private final static String TAG="ThemeViewTestActivity";
    private nexEngineView engineView;
    private nexEngine mEngine;
    private nexEngineListener mListener;
    private int lnitAspectMode;
    private boolean isEngineViewShow = true;
    private boolean EngineViewAvailable = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_theme_view_test);

        lnitAspectMode = nexApplicationConfig.getAspectRatioMode();

        engineView = (nexEngineView)findViewById(R.id.engineview_test);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setProject(new nexProject());
        mEngine.setView(engineView);

        engineView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int i, int i1) {
                EngineViewAvailable = true;
                Log.d(TAG,"onEngineViewAvailable="+i+"X"+i1);
                if( mEngine.getProject() != null ){
                    if( mEngine.getProject().getTotalClipCount(true) > 0 ){
                        mEngine.seek(500);
                    }
                }
            }

            @Override
            public void onEngineViewSizeChanged(int i, int i1) {
                Log.d(TAG,"onEngineViewSizeChanged="+i+"X"+i1);
            }

            @Override
            public void onEngineViewDestroyed() {
                EngineViewAvailable = false;
                Log.d(TAG,"onEngineViewDestroyed");
            }
        });

        mListener =  new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

            }

            @Override
            public void onTimeChange(int currentTime) {

            }

            @Override
            public void onSetTimeDone(int currentTime) {
                showEngineView();
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

            }

            @Override
            public void onPlayFail(int err, int iClipID) {

            }

            @Override
            public void onPlayStart() {
                showEngineView();
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
        };

        mEngine.setEventHandler(mListener);

        final Button btContent = (Button)findViewById(R.id.button_themeview_content);
        btContent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();

                Intent intent = new Intent( );
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent,100);

            }
        });

    }

    private void showEngineView(){
        if( !isEngineViewShow ){
            engineView.setVisibility(View.VISIBLE);
            isEngineViewShow = true;
        }
    }

    private void hideEngineView(){
        if( isEngineViewShow ) {
            engineView.setVisibility(View.GONE);
            isEngineViewShow = false;
        }
    }


    private void setClip(String path){
        nexClip clip = nexClip.getSupportedClip(path);
        if( clip == null ){
            Toast.makeText(getApplicationContext(), "Not supported clip : " + path, Toast.LENGTH_SHORT).show();
            return ;
        }

        hideEngineView();

        mEngine.getProject().allClear(true);

        int rotate = 0;
        int width = 0;
        int height = 0;
        rotate = clip.getRotateInMeta();
        width = clip.getWidth();
        height = clip.getHeight();

        if( clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270 ){
            nexApplicationConfig.setAspectProfile(new nexAspectProfile(height,width)); // note : set Aspect ratio
            Log.d(TAG,"Port Mode="+height+" X "+width);
        }else{
            nexApplicationConfig.setAspectProfile(new nexAspectProfile(width,height)); // note : set Aspect ratio
            Log.d(TAG,"Land Mode="+width+" X "+height);
        }

        mEngine.getProject().add(clip);
        mEngine.getProject().getLastPrimaryClip().setRotateDegree(rotate);
        mEngine.getProject().getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        mEngine.updateProject();
        if( EngineViewAvailable ) {
            mEngine.seek(500);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ArrayList<String> listFilePath = new ArrayList<>();
            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                listFilePath.add(filepath);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    listFilePath.add(filepath);
                }
            }
            setClip(listFilePath.get(0));
        }
    }

    @Override
    protected void onDestroy() {
        nexApplicationConfig.setAspectMode(lnitAspectMode);
        super.onDestroy();
    }

    @Override
    protected void onResume() {

        if( mEngine.getProject() != null ){
            if( mEngine.getProject().getTotalClipCount(true) > 0 ){
                if( EngineViewAvailable ) {
                    mEngine.seek(500);
                }
            }
        }
        super.onResume();
    }
}
