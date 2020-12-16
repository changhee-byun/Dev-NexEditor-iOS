/******************************************************************************
 * File Name        : EffectPreviewListTestActivity.java
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
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.Constants;

import java.util.ArrayList;
import java.util.Random;


public class EffectPreviewListTestActivity extends Activity {
    private static final String TAG = "EffectPreviewListTest";

    private nexEngine mEngin;
    private nexEngineListener mEditorListener = null;
    private String mThemeID;
    private ArrayList<String> mListFilePath;
    private nexEngineView mEditorView;
    private Button mButtonExport;

    private boolean mSurfaceInit = false;
    private boolean mStartPlay = false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_effect_preview_list_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = new ArrayList<String>();

        Intent intent = getIntent();
        mListFilePath = intent.getStringArrayListExtra("filelist");

        if(mListFilePath.size() == 0) {
            Toast.makeText(getApplicationContext(),"No Clips.",Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        String Theme = intent.getStringExtra(Constants.THEME_ID);
        if(Theme.compareTo("none")  != 0 ) {
            mThemeID = Theme;
        }

        String transitionId = intent.getStringExtra(Constants.TRANSITION_ID);

        nexProject project = new nexProject(mThemeID, "Open my Create", "End my Close");

        for(int i = 0; i < mListFilePath.size(); i++) {
            nexClip clip = nexClip.getSupportedClip(mListFilePath.get(i));
            if( clip != null ){
                if(clip.getClipType()==nexClip.kCLIP_TYPE_VIDEO){
                    if(clip.getTotalTime() < 2000){
                        Toast.makeText(getApplicationContext(),"Clip("+i+") duration is lower than 2000ms.",Toast.LENGTH_LONG).show();
                        continue;
                    }
                }

                project.add(clip);

                //if( nexApplicationConfig.getAspectRatioMode() ==  nexApplicationConfig.kAspectRatio_Mode_9v16 &&
                if( !nexApplicationConfig.getAspectProfile().isLandscapeMode() &&
                        clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                    project.getClip(i, true).setRotateDegree(nexClip.kClip_Rotate_270);
                }

                if(project.getClip(i, true).getRotateInMeta() == 270 || project.getClip(i, true).getRotateInMeta() == 90) {
                    if(project.getClip(i, true).getClipType() != nexClip.kCLIP_TYPE_IMAGE) {
                        project.getClip(i, true).setRotateDegree(clip.getRotateInMeta());
                    }
                }
                project.getClip(i, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
            }else {
                Toast.makeText(getApplicationContext(), "Not Supported Clips.", Toast.LENGTH_LONG).show();
            }
        }

        // single content
        if(project.getTotalClipCount(true) == 1) {
            nexClip clip = project.getClip(0,true);
            if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                if(clip.getTotalTime() > 12000) {
                    int dupCount = 3;
                    int interval = clip.getTotalTime() / (dupCount + 1);
                    int startTime = 0;

                    for(int i = 0; i < dupCount; i++) {
                        project.add(nexClip.dup(clip));
                        project.getClip(i, true).getVideoClipEdit().setTrim(startTime, startTime + interval);

                        if(project.getClip(i, true).getRotateInMeta() == 270 || project.getClip(i, true).getRotateInMeta() == 90) {
                            project.getClip(i, true).setRotateDegree(clip.getRotateInMeta());
                        }
                        startTime += interval;
                    }
                    project.getClip(dupCount, true).getVideoClipEdit().setTrim(startTime, startTime + interval);

                    if (project.getClip(dupCount, true).getRotateInMeta() == 270 || project.getClip(dupCount, true).getRotateInMeta() == 90) {
                        project.getClip(dupCount, true).setRotateDegree(clip.getRotateInMeta());
                    }
                    project.getClip(dupCount, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    project.getClip(1, true).setBrightness(100);
                    project.getClip(1, true).setContrast(-25);
                    project.getClip(1, true).setSaturation(-25);
                    project.getClip(1, true).getClipEffect().setTitle("Clip Middle 1");
                    project.getClip(1, true).setColorEffect(nexColorEffect.ORANGE);


                    project.getClip(2, true).setColorEffect(nexColorEffect.DEEP_BLUE);
                }
                else {
                    Log.d(TAG, "single content total time < 12000");
                }
            } else if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                project.add(nexClip.dup(clip));
                project.add(nexClip.dup(clip));

                project.getClip(1, true).getClipEffect().setTitle("Clip Middle 1");
            }
        }

        Log.e(TAG,"Auto Theme Time = "+project.getTotalTime());

        if( transitionId.compareTo("random") == 0 ) {
            nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
            nexTransitionEffect[] transitionEffects = fxlib.getTransitionEffects();

            for (int i = 0; i < project.getTotalClipCount(true); i++) {
                nexClip clip = project.getClip(i, true);
                int rand = new Random().nextInt(transitionEffects.length);
                clip.getTransitionEffect().setTransitionEffect(transitionEffects[rand].getId());
                clip.getTransitionEffect().setDuration(2000);
            }
            Log.e(TAG,"random transition Time = " + project.getTotalTime());
        } else if( transitionId.compareTo("none") != 0 ) {
            nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(getApplicationContext());
            nexEffectOptions options = fxlib.getEffectOptions(getApplicationContext(), transitionId);
            if( options != null ) {

                int index = 0;
                for (int i = 0; i < project.getTotalClipCount(true); i++) {
                    project.getClip(i, true).getTransitionEffect().setTransitionEffect(transitionId);
                    project.getClip(i, true).getTransitionEffect().setDuration(2000);
                    if (options.getSelectOptions().size() > 0) {
                        options.getSelectOptions().get(0).setSelectIndex(index);

                        int selectMaxIndex = options.getSelectOptions().get(0).getItems().length;
                        if (index >= selectMaxIndex)
                            index = 0;
                        else
                            index++;

                        project.getClip(i, true).getTransitionEffect().updateEffectOptions(options, true);
                    }
                }
            }

        }

        mEditorView = (nexEngineView)findViewById(R.id.engineview_effect_preview_list_test);

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setView(mEditorView);

        mEngin.setProject(project);

        mEngin.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {

                if( !mSurfaceInit ){
                    mSurfaceInit = true;
                }

                if( !mStartPlay ){
                    Log.d(TAG,"onSurfaceChanged play call");
                    mStartPlay = true;
                    mEngin.play();
                }

            }
        });


        mButtonExport = (Button)findViewById(R.id.btn_effectpreview_set);
        mButtonExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngin.stop();
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });
    }

    @Override
    protected void onPause() {
        if( mEngin != null) {
            mSurfaceInit = false;
            mStartPlay = false;
            mEngin.stop();
            mEngin.clearTrackCache();
            Log.d(TAG,"clearTrackCache Call!!!");
        }
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");

    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart");
        super.onStart();
        if(mEngin != null) {
            if( mSurfaceInit ) {
                if( !mStartPlay ) {
                    Log.d(TAG,"onStart play call");
                    mStartPlay = true;
                    mEngin.play();
                }
            }
        }
    }
}
