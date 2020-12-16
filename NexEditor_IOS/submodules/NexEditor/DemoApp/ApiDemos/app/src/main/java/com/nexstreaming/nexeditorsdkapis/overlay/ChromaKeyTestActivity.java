/******************************************************************************
 * File Name        : ChromaKeyTestActivity.java
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
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;

import java.util.ArrayList;

public class ChromaKeyTestActivity extends Activity {
    private static final String TAG = "ChromaKeyTest";
    private nexEngineListener mEditorListener = null;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngin;
    private nexEngineView m_editorView;
    private boolean mIsExport;
    private Button mPlayButton;
    private Button mChromaColor;
    private TextView mChromaFG;
    private TextView mChromaBG;

    private SeekBar mChromaFGSeek;
    private SeekBar mChromaBGSeek;


    private int mChromaClipTotalTime;
    private int [] mChromaRecommendedColors;
    private int mSelectChromaRecommendedColor = -1;
    nexOverlayItem mChromaitem;
    private Switch mChromakeySet;
    private Switch mChromakeyMask;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chroma_key_test);

        nexProject project = new nexProject();
        final Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        String path;
        try {
            UtilityCode.raw2file(getApplicationContext(), R.raw.chromakey_sample, "chromakey_sample.mp4");
        } catch (Exception e) {
            e.printStackTrace();
        }

        path = getFilesDir().getAbsolutePath()+"/chromakey_sample.mp4";
        nexClip chromarkeyClip = nexClip.getSupportedClip(path);

        if( chromarkeyClip == null ){
            Toast.makeText(getApplicationContext(), "chromarkeyClip load fail.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        mChromaClipTotalTime = chromarkeyClip.getTotalTime();

        nexClip backGroundClip = new nexClip(m_listfilepath.get(0));

        if (backGroundClip == null) {
            Toast.makeText(getApplicationContext(), "Not supported Clips.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }


        if( mChromaClipTotalTime > 10000 ){
            mChromaClipTotalTime = 10000;
        }

        nexOverlayImage chromaVideo = new nexOverlayImage("chromaVideo",chromarkeyClip);
        mChromaitem = new nexOverlayItem(chromaVideo,1280/2,720/2,0, mChromaClipTotalTime);
        mChromaitem.setScale(1280f / (float) chromarkeyClip.getWidth() * 0.5f, 720f / (float) chromarkeyClip.getHeight() * 0.5f);

        project.addOverlay(mChromaitem);
        project.add(backGroundClip);

        mChromakeySet = (Switch)findViewById(R.id.switch_chromakey_set);
        mChromakeyMask = (Switch)findViewById(R.id.switch_chromakey_mask);

        mChromaFGSeek = (SeekBar)findViewById(R.id.seekBar_chromakey_fg);
        mChromaFGSeek.setMax(100);
        mChromaBGSeek = (SeekBar)findViewById(R.id.seekBar_chromakey_bg);
        mChromaBGSeek.setMax(100);

        mPlayButton = (Button)findViewById(R.id.button_chromakey_playstop);
        mPlayButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //mEngin.updateProject();
                updateUI(false);
                mEngin.play();
            }
        });

        mPlayButton.setVisibility(View.INVISIBLE);

        mChromaColor = (Button)findViewById(R.id.button_chromakey_color);
        mChromaColor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder alertBuilder = new AlertDialog.Builder(
                        ChromaKeyTestActivity.this);
                //alertBuilder.setIcon(R.drawable.ic_launcher);
                alertBuilder.setTitle("Select ChromaKey Color");


                final ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                        ChromaKeyTestActivity.this,
                        android.R.layout.select_dialog_singlechoice);

                for (int color : mChromaRecommendedColors) {
                    adapter.add(String.format("0x%08X", color));
                }


                alertBuilder.setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int which) {
                                dialog.dismiss();
                            }
                        });

                alertBuilder.setAdapter(adapter, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mSelectChromaRecommendedColor = which;
                    }
                });

                alertBuilder.show();
            }
        });

        mChromaColor.setVisibility(View.INVISIBLE);

        mChromaFG = (TextView)findViewById(R.id.textView_chromakey_fg);
        mChromaBG = (TextView)findViewById(R.id.textView_chromakey_bg);

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEditorListener); // null
        m_editorView = (nexEngineView) findViewById(R.id.engineview_chromakey);
        m_editorView.setBlackOut(true);
        mEngin.setView(m_editorView);
        mEngin.setProject(project);

        chromarkeyClip.getVideoClipDetailThumbnails(640, 360, 1000, mChromaClipTotalTime, 1, 0, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
            @Override
            public void onGetDetailThumbnailResult(int i, Bitmap bitmap, int i1, int i2, int i3) {
                if (i == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                    mChromaRecommendedColors = nexOverlayItem.ChromaKey.getChromaKeyRecommendedColors(bitmap);
                    if (mChromaRecommendedColors != null && mChromaRecommendedColors.length > 0) {
                        mChromaitem.getChromaKey().setChromaKeyEnabled(true);

                        mChromaitem.getChromaKey().setChromaKeyColor(mChromaRecommendedColors[0]);
                        mSelectChromaRecommendedColor = 0;
                        for (int color : mChromaRecommendedColors) {
                            Log.d(TAG, "ChromaRecommendedColor=" + String.format("0x%08X", color));
                        }
                        updateUI(true);
                    }
                }
            }
        });
    }

    void updateUI(boolean toUi){
        if( toUi ) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (mChromaRecommendedColors != null && mChromaRecommendedColors.length > 0) {
                        mPlayButton.setVisibility(View.VISIBLE);
                        mChromaColor.setVisibility(View.VISIBLE);
                        mChromaColor.setBackgroundColor(mChromaRecommendedColors[mSelectChromaRecommendedColor]);
                        int progress = (int)(mChromaitem.getChromaKey().getChromaKeyFGClip()*100);
                        mChromaFGSeek.setProgress(progress);
                        mChromaFG.setText(String.format("FG:%03d", progress));

                        progress = (int)(mChromaitem.getChromaKey().getChromaKeyBGClip()*100);
                        mChromaBGSeek.setProgress(progress);
                        mChromaBG.setText(String.format("BG:%03d",progress));

                    }
                    mChromakeySet.setChecked(mChromaitem.getChromaKey().getChromaKeyEnabled());
                }
            });
        }else{
            mChromaitem.getChromaKey().setChromaKeyEnabled(mChromakeySet.isChecked());
            mChromaitem.getChromaKey().setChromaKeyMaskEnabled(mChromakeyMask.isChecked());
            mChromaitem.getChromaKey().setChromaKeyColor(mChromaRecommendedColors[mSelectChromaRecommendedColor]);
            float clipval = (float)mChromaFGSeek.getProgress()/100f;
            mChromaitem.getChromaKey().setChromaKeyFGClip(clipval);
            clipval = (float)mChromaBGSeek.getProgress()/100f;
            mChromaitem.getChromaKey().setChromaKeyBGClip(clipval);
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
