/******************************************************************************
 * File Name        : ClipCropTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.edit;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.util.ArrayList;


public class ClipCropTestActivity extends Activity {
    private static final String TAG = "ClipCropTestActivity";
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private float mRatio = 1;
    private ClipDragRectView mStartRect;
    private ClipDragRectView mEndRect;
    private boolean mSeek ;
    private Rect startrect;
    private Rect endrect;
    private nexCrop.CropMode mode = nexCrop.CropMode.PAN_RAND;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_clip_crop_test);

        m_listfilepath = new ArrayList<String>();
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        int wid = 640, hei = 360;
        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if (clip != null) {
            if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                clip.setRotateDegree(clip.getRotateInMeta());
            }
            project.add(clip);
        }

        m_editorView = (nexEngineView) findViewById(R.id.engineview_color_crop_test);
        m_editorView.setBlackOut(true);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);

        mSeek = false;
        mEngine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {
                if( !mSeek ){
                    mSeek = true;
                    mEngine.updateProject();
                    mEngine.seek(1000);
                }
            }
        });

        mStartRect = (ClipDragRectView) findViewById(R.id.dragrectview_clip_crop_test_start);
        if (null != mStartRect) {
            mStartRect.setOnUpCallback(new ClipDragRectView.OnUpCallback() {
                @Override
                public void onRectFinished(final Rect rect) {
                    Toast.makeText(getApplicationContext(), "Rect is (" + rect.left + ", " + rect.top + ", " + rect.right + ", " + rect.bottom + ")",
                            Toast.LENGTH_LONG).show();
                    rect.left = (int) (rect.left / mRatio);
                    rect.top = (int) (rect.top / mRatio);
                    rect.right = (int) (rect.right / mRatio);
                    rect.bottom = (int) (rect.bottom / mRatio);
                    mEngine.getProject().getClip(0, true).getCrop().setStartPosition(rect);
                    startrect = rect;
                    Rect raw = new Rect();
                    mEngine.getProject().getClip(0, true).getCrop().getStartPositionRaw(raw);
                    mEngine.fastPreviewCrop(raw);
                }
            });
        } else {
            Toast.makeText(getApplicationContext(), "mstartRect is null" , Toast.LENGTH_SHORT).show();
        }

        mEndRect = (ClipDragRectView) findViewById(R.id.dragrectview_clip_crop_test_end);
        if (null != mEndRect) {
            mEndRect.setOnUpCallback(new ClipDragRectView.OnUpCallback() {
                @Override
                public void onRectFinished(final Rect rect) {
                    Toast.makeText(getApplicationContext(), "Rect is (" + rect.left + ", " + rect.top + ", " + rect.right + ", " + rect.bottom + ")",
                            Toast.LENGTH_LONG).show();
                    rect.left = (int) (rect.left / mRatio);
                    rect.top = (int) (rect.top / mRatio);
                    rect.right = (int) (rect.right / mRatio);
                    rect.bottom = (int) (rect.bottom / mRatio);
                    mEngine.getProject().getClip(0, true).getCrop().setEndPosition(rect);
                    endrect = rect;
                    Rect raw = new Rect();
                    mEngine.getProject().getClip(0, true).getCrop().getEndPositionRaw(raw);
                    mEngine.fastPreviewCrop(raw);
                }
            });
        }

        setDimension();

        final Button bt = (Button) findViewById(R.id.button_clip_crop_test_play);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                if(mode == nexCrop.CropMode.PAN_RAND) {
                    mEngine.getProject().getClip(0, true).getCrop().randomizeStartEndPosition(false, mode);
                }else{
                    if(startrect == null || endrect == null) {
                        mEngine.getProject().getClip(0, true).getCrop().randomizeStartEndPosition(false, mode);
                    } else {
                        mEngine.getProject().getClip(0, true).getCrop().setStartPosition(startrect);
                        mEngine.getProject().getClip(0, true).getCrop().setEndPosition(endrect);
                    }

                }
                mEngine.play();
            }
        });

        final Button rt = (Button) findViewById(R.id.button_clip_crop_test_rotate);
        rt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.getProject().getClip(0, true).setRotateDegree(mEngine.getProject().getClip(0, true).getRotateDegree() + 90);
                setDimension();
            }
        });

        String[] items = {"FIT", "FILL", "PAN_RAND", "PANORAMA"};
        final Spinner sp = (Spinner) findViewById(R.id.spinner_clip_crop_test_mode);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, items);
        sp.setAdapter(adapter);
        sp.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (position == 0)
                    mode = nexCrop.CropMode.FIT;
                else if (position == 1)
                    mode = nexCrop.CropMode.FILL;
                else if (position == 2)
                    mode = nexCrop.CropMode.PAN_RAND;
                else if (position == 3)
                    mode = nexCrop.CropMode.PANORAMA;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

    private void setDimension() {
        int wid = mEngine.getProject().getClip(0, true).getCrop().getWidth();
        int hei = mEngine.getProject().getClip(0, true).getCrop().getHeight();
        int rot = mEngine.getProject().getClip(0, true).getCrop().getRotate();

        if( rot == 90 || rot == 270 ) {
            mRatio = 360f / (float) wid;
            int tmp = wid;
            wid = hei;
            hei = tmp;
        }else{
            mRatio = 360f / (float) hei;
        }
        wid = (int) (wid * mRatio);
        hei = (int) (hei * mRatio);

        mStartRect.resizeDimension(wid, hei);
        mEndRect.resizeDimension(wid, hei);

        Rect startRect = new Rect();
        Rect endRect = new Rect();
        mEngine.getProject().getClip(0, true).getCrop().getStartPosition(startRect);

        startRect.left = (int) (startRect.left * mRatio);
        startRect.top = (int) (startRect.top * mRatio);
        startRect.right = (int) (startRect.right * mRatio);
        startRect.bottom = (int) (startRect.bottom * mRatio);

        mEngine.getProject().getClip(0, true).getCrop().getEndPosition(endRect);

        endRect.left = (int) (endRect.left * mRatio);
        endRect.top = (int) (endRect.top * mRatio);
        endRect.right = (int) (endRect.right * mRatio);
        endRect.bottom = (int) (endRect.bottom * mRatio);

        mStartRect.initRect(startRect);
        mEndRect.initRect(endRect);

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
