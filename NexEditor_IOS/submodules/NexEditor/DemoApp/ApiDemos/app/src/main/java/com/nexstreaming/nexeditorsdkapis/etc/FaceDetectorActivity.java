/******************************************************************************
 * File Name        : FaceDetectorActivity.java
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
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexfacedetection.nexFaceDetector;

import java.util.ArrayList;

public class FaceDetectorActivity extends Activity {
    private static final String TAG = "FaceDetectorActivity";
    private ImageView mView;
    private ArrayList<String> m_listfilepath;
    private int mlistFileIndex;
    private Canvas mCanvas;
    private Bitmap mCanvasBitmap;
    private ProgressDialog mProgressDialog;
    private Button buttonNext;
    private Button buttonPrev;
    private nexEngine mEngin;
    private nexFaceDetector nexFD = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_face_detector);
        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        mlistFileIndex = 0;
        mView = (ImageView)findViewById(R.id.imageView_faceDetector);
        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.set360VideoForceNormalView();

        buttonNext = (Button) findViewById(R.id.button_next);
        buttonNext.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mView.setImageDrawable(null);
                if ( (mlistFileIndex+1) < m_listfilepath.size() ) {
                    mlistFileIndex++;
                }
                drawFace();
                updateButtonActivation();
            }
        });
        buttonPrev = (Button) findViewById(R.id.button_prev);
        buttonPrev.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mView.setImageDrawable(null);

                if ( mlistFileIndex > 0 ) {
                    mlistFileIndex--;
                }
                drawFace();
                updateButtonActivation();
            }
        });

        drawFace();
        updateButtonActivation();
    }

    private void drawFace() {
        String path = m_listfilepath.get(mlistFileIndex);

        nexClip clip = nexClip.getSupportedClip(path);
        if ( clip == null ) {
            Log.d(TAG, "This is unsupported clip! "+path);
            return;
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inMutable = true;
        Bitmap myBitmap = BitmapFactory.decodeFile(path,options);

        mCanvasBitmap = Bitmap.createBitmap(myBitmap.getWidth(), myBitmap.getHeight(), Bitmap.Config.RGB_565);
        mCanvas = new Canvas(mCanvasBitmap);
        mCanvas.drawBitmap(myBitmap, 0, 0, null);

        setNexFaceModule();
        Rect facebounds = new Rect();
        clip.getCrop().getFaceBounds(facebounds, getApplicationContext());
        if ( !facebounds.isEmpty() ) {
            Log.d(TAG, "Face detection is success! "+facebounds);
            Paint rectPaint = new Paint();
            rectPaint.setStrokeWidth(5);
            rectPaint.setColor(Color.RED);
            rectPaint.setStyle(Paint.Style.STROKE);
            mCanvas.drawRect(facebounds, rectPaint);
            mView.setImageDrawable(new BitmapDrawable(getResources(), mCanvasBitmap));
        } else {
            Log.d(TAG, "Face detection fail! "+path);
            mView.setImageDrawable(new BitmapDrawable(getResources(), mCanvasBitmap));
        }
    }

    private void updateButtonActivation() {
        if ( m_listfilepath.size() == 1 ) {
            buttonNext.setEnabled(false);
            buttonPrev.setEnabled(false);
        } else if ( mlistFileIndex == 0 ) {
            buttonPrev.setEnabled(false);
        } else if ( (mlistFileIndex+1) == m_listfilepath.size() ) {
            buttonNext.setEnabled(false);
            buttonPrev.setEnabled(true);
        } else {
            buttonNext.setEnabled(true);
            buttonPrev.setEnabled(true);
        }
    }
    public void setNexFaceModule(){
        mEngin.setFaceModule("6a460d22-cd87-11e7-abc4-cec278b6b50a");
    }
}

