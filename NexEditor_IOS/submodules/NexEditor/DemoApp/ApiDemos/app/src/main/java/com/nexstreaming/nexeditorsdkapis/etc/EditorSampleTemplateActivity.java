/******************************************************************************
 * File Name        : EditorSampleTemplateActivity.java
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
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexAssetMediaManager;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCollageManager;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexFont;
//import com.nexstreaming.nexeditorsdk.nexBeatTemplateManager;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterText;
import com.nexstreaming.nexeditorsdk.nexOverlayManager;
import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdk.service.nexAssetService;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.util.ArrayList;
import java.util.List;

public class EditorSampleTemplateActivity extends Activity {
    private final static String TAG = "EditorSampleTemplate";
    private ListView mTemplateList;
    private ListView mAudioList;
    private ListView mStickerList;
    private ListView mTextList;
    private ListView mFontList;
    private ListView mCollageList;

    private ArrayList<String> mListFilePath;
    private nexTemplateManager mTman;
    private nexOverlayManager mTitle;
    private nexAssetMediaManager mMedia;
    private nexCollageManager mCollage;
//    private nexBeatTemplateManager mMTman;
    private nexEngineView mView;

    private nexProject mOriginalProject;

    private int mlnitAspectMode = 0;
    private int mCurrentTSelect = 0;
    private long backKeyPressedTime = 0;
    ArrayList<String> templateList = null;
    CustomViewAdapter templateAdapter = null;
    ArrayList<String> audioList = null;
    CustomViewAdapter audioAdapter = null;
    ArrayList<String> stickerList = null;
    CustomViewAdapter stickerAdapter = null;

    ArrayList<String> fontList = null;
    CustomViewAdapter fontAdapter = null;

    ArrayList<String> textList = null;
    CustomViewAdapter textAdapter = null;

    ArrayList<String> collageList = null;
    CustomViewAdapter collageAdapter = null;


    private nexEngine mEngine = null;
    private boolean mPlaying = false;
    private TabHost tabHost;
    private String mCurrentStickerID;
    private int mCurrentOverlayID;

    private String mCurrentFontID;
    private int mCurrentTextID;

    private boolean localAssetStore;
    private boolean autoAspect;
    private ProgressDialog progressDialog;
    private boolean mEngineViewAvailable;
    private nexAspectProfile contentAspectProfile;
    private nexEngineListener mEngineListener = new nexEngineListener() {
        @Override
        public void onStateChange(int i, int i1) {

        }

        @Override
        public void onTimeChange(int i) {

        }

        @Override
        public void onSetTimeDone(int i) {

        }

        @Override
        public void onSetTimeFail(int i) {

        }

        @Override
        public void onSetTimeIgnored() {

        }

        @Override
        public void onEncodingDone(boolean b, int i) {

        }

        @Override
        public void onPlayEnd() {
            Log.d(TAG,"onPlayEnd");
            mPlaying = false;
            mEngine.seek(0);
        }

        @Override
        public void onPlayFail(int i, int i1) {
            Log.d(TAG,"onPlayFail="+i);
            mPlaying = false;
            mEngine.seek(0);
        }

        @Override
        public void onPlayStart() {
            Log.d(TAG,"onPlayStart");
            mPlaying = true;
        }

        @Override
        public void onClipInfoDone() {

        }

        @Override
        public void onSeekStateChanged(boolean b) {

        }

        @Override
        public void onEncodingProgress(int i) {

        }

        @Override
        public void onCheckDirectExport(int i) {

        }

        @Override
        public void onProgressThumbnailCaching(int i, int i1) {

        }

        @Override
        public void onFastPreviewStartDone(int i, int i1, int i2) {

        }

        @Override
        public void onFastPreviewStopDone(int i) {

        }

        @Override
        public void onFastPreviewTimeDone(int i) {

        }

        @Override
        public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

        }
    };

    //1. create BroadcastReceiver
    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                if (nexAssetService.ACTION_ASSET_INSTALL_COMPLETED.equals(action)) {
                    int idx = intent.getIntExtra("index", -1);
                    String categoryName = intent.getStringExtra("category.alias");
                    Log.d(TAG, "installed Asset, categoryName=" + categoryName + ", index=" + idx);

                    //TODO : update UI
                    if( idx != -1 ) {
                        updateAssets(true, idx);
                    }else{
                        updateList();
                    }
                } else if (nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED.equals(action)) {
                    int idx = intent.getIntExtra("index", -1);
                    Log.d(TAG, "uninstalled Asset, index=" + idx);

                    //TODO : update UI
                    if( idx != -1 ) {
                        updateAssets(false, idx);
                    }else{
                        updateList();
                    }
                } else if( nexAssetService.ACTION_ASSET_FEATUREDLIST_COMPLETED.equals(action) ){
                    boolean update = intent.getBooleanExtra("update", true);
                    Log.d(TAG, "get featured list update="+update);
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_editor_sample_template);

        //2. BroadcastReceiver was set intent filter.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(nexAssetService.ACTION_ASSET_INSTALL_COMPLETED);
        intentFilter.addAction(nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED);
        intentFilter.addAction(nexAssetService.ACTION_ASSET_FEATUREDLIST_COMPLETED);
        registerReceiver(broadcastReceiver, intentFilter);


        if( mListFilePath == null ) {
            Intent intent = getIntent();
            mListFilePath = intent.getStringArrayListExtra("filelist");
            localAssetStore = intent.getBooleanExtra("localAssetStore", false);
            autoAspect = intent.getBooleanExtra("autoAspect", true);
        }

        if (!localAssetStore) {
            if (nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext())) {
                Log.d(TAG, "Asset store installed");
                nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
            } else {
                Log.d(TAG, "Asset store was not installed");
            }
        }

        if( mListFilePath == null ){
            Toast.makeText(getApplicationContext(), "no clip!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        Log.d(TAG, "nexEditor init end. and make clip start");
        nexClip clip = nexClip.getSupportedClip(mListFilePath.get(0));
        if (clip == null) {
            Toast.makeText(getApplicationContext(), "not supported clip!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mlnitAspectMode = nexApplicationConfig.getAspectRatioMode();

        if (clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {

            if (autoAspect) {
                int rotate = 0;
                int width = 0;
                int height = 0;
                rotate = clip.getRotateInMeta();
                width = clip.getWidth();
                height = clip.getHeight();

                int mode = nexAspectProfile.getSimilarAspectMode(width, height, rotate);
                contentAspectProfile = nexAspectProfile.getAspectProfile(mode);

                nexApplicationConfig.setAspectMode(contentAspectProfile.getAspectMode());
                Log.d(TAG, "aspect ratio setting end.");
            }else{
                contentAspectProfile = nexAspectProfile.getAspectProfile(mlnitAspectMode);;
            }
        } else if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
            contentAspectProfile = nexAspectProfile.getAspectProfile(mlnitAspectMode);;
        }

        setContentView(R.layout.activity_editor_sample_template);

        tabHost = (TabHost) findViewById(R.id.tabhost);
        tabHost.setup();

        // Tab1 Setting
        TabHost.TabSpec tabSpec1 = tabHost.newTabSpec("Tab1");
        tabSpec1.setIndicator("Template"); // Tab Subject
        tabSpec1.setContent(R.id.listview_template_man); // Tab Content
        tabHost.addTab(tabSpec1);

        // Tab2 Setting
        TabHost.TabSpec tabSpec2 = tabHost.newTabSpec("Tab2");
        tabSpec2.setIndicator("Audio"); // Tab Subject
        tabSpec2.setContent(R.id.listview_audio_man); // Tab Content
        tabHost.addTab(tabSpec2);

        // Tab3 Setting
        TabHost.TabSpec tabSpec3 = tabHost.newTabSpec("Tab3");
        tabSpec3.setIndicator("Title"); // Tab Subject
        tabSpec3.setContent(R.id.listview_text_man); // Tab Content
        tabHost.addTab(tabSpec3);

        // Tab4 Setting
        TabHost.TabSpec tabSpec4 = tabHost.newTabSpec("Tab4");
        tabSpec4.setIndicator("Sticker"); // Tab Subject
        tabSpec4.setContent(R.id.listview_sticker_man); // Tab Content
        tabHost.addTab(tabSpec4);


        // Tab5 Setting
        TabHost.TabSpec tabSpec5 = tabHost.newTabSpec("Tab5");
        tabSpec5.setIndicator("Font"); // Tab Subject
        tabSpec5.setContent(R.id.listview_font_man); // Tab Content
        tabHost.addTab(tabSpec5);

        // Tab6 Setting
        TabHost.TabSpec tabSpec6 = tabHost.newTabSpec("Tab6");
        tabSpec6.setIndicator("Collage"); // Tab Subject
        tabSpec6.setContent(R.id.listview_collage_man); // Tab Content
        tabHost.addTab(tabSpec6);


        // show First Tab Content
        tabHost.setCurrentTab(0);

        mTemplateList = (ListView) findViewById(R.id.listview_template_man);
        mAudioList = (ListView) findViewById(R.id.listview_audio_man);
        mStickerList = (ListView) findViewById(R.id.listview_sticker_man);
        mTextList = (ListView) findViewById(R.id.listview_text_man);
        mFontList = (ListView) findViewById(R.id.listview_font_man);
        mCollageList = (ListView) findViewById(R.id.listview_collage_man);

        mTman = nexTemplateManager.getTemplateManager(getApplicationContext(), getApplicationContext());
        templateList = new ArrayList<String>();

        mMedia = nexAssetMediaManager.getAudioManager(getApplicationContext());
//        mMTman = nexBeatTemplateManager.getBeatTemplateManager(getApplicationContext());
        audioList = new ArrayList<String>();

        stickerList = new ArrayList<String>();

        mTitle = nexOverlayManager.getOverlayManager(getApplicationContext(), getApplicationContext());
        textList = new ArrayList<String>();

        fontList = new ArrayList<String>();

        mCollage = nexCollageManager.getCollageManager(getApplicationContext(),getApplicationContext());
        collageList = new ArrayList<String>();

        mTemplateList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mCurrentTSelect != position) {
                    if (mPlaying) {
                        mEngine.stop();
                        mPlaying = false;
                    }

                    reloadClipInEngine();

                    int h = mView.getHeight();
                    int w = (int)((float)h * contentAspectProfile.getAspectRatio());
                    nexApplicationConfig.setAspectMode(contentAspectProfile.getAspectMode());
                    mView.setLayoutParams(new FrameLayout.LayoutParams(w,h, Gravity.CENTER));

                    if (position == 0) {
                        mEngine.updateProject();
                        mEngine.seek(0);
                        mCurrentTSelect = position;

                    } else {
                        boolean isOk = true;
                        /*
                        Log.d(TAG,"validateAssetPackage Asset idx="+templateList.get(position).getAssetIdx() );
                        if( templateList.get(position).getAssetIdx() > 3 ){
                            Log.d(TAG,"validateAssetPackage start");
                            isOk = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).validateAssetPackage(templateList.get(position).getAssetIdx());
                            Log.d(TAG,"validateAssetPackage end");
                        }
                        */
                        if (isOk) {
                            boolean asyncMode = false;
                            if (asyncMode) {
                                if (progressDialog == null)
                                    progressDialog = new ProgressDialog(EditorSampleTemplateActivity.this);
                                progressDialog.show();

				                if ( checkAsset(templateList.get(position)) == false ) {
                                    Toast.makeText(getApplicationContext(), "This is expired asset!", Toast.LENGTH_SHORT).show();
                                    return;
                                }
                                try {
                                    mTman.applyTemplateAsyncToProjectById(mEngine.getProject(), templateList.get(position), new Runnable() {
                                        @Override
                                        public void run() {
                                            mEngine.updateProject();
                                            progressDialog.dismiss();
                                            mEngine.seek(mEngine.getDuration()/2);
                                        }
                                    });
                                } catch (ExpiredTimeException e) {
                                    e.printStackTrace();
                                }
                                mCurrentTSelect = position;
                            } else {
                            	if ( checkAsset(templateList.get(position)) == false ) {
                                    Toast.makeText(getApplicationContext(), "This is expired asset!", Toast.LENGTH_SHORT).show();
                                    return;
                                }
                                try {
                                    mTman.applyTemplateToProjectById(mEngine.getProject(), templateList.get(position));
                                } catch (ExpiredTimeException e) {
                                    e.printStackTrace();
                                }
                                mEngine.updateProject();
                                mEngine.seek(mEngine.getDuration()/2);
                                mCurrentTSelect = position;
                            }
                        } else {
                            Toast.makeText(getApplicationContext(), "Template miss resource!", Toast.LENGTH_SHORT).show();
                        }
                    }
                } else {
                    if (mPlaying) {
                        mEngine.stop();
                        mPlaying = false;
                    } else {
                        mEngine.seek(0);
                        mEngine.resume();
                        mPlaying = true;
                    }
                }
            }
        });


        mAudioList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }

                if (position == 0) {
                    mEngine.getProject().setBackgroundMusicPath(null);
                    mEngine.play();
                    mPlaying = true;
                } else {
                    if ( checkAsset(audioList.get(position)) == false ) {
                        Toast.makeText(getApplicationContext(), "This is expired asset!", Toast.LENGTH_SHORT).show();
                        return;
                    }
                    Log.d(TAG,"audio select id="+audioList.get(position));
                    nexAssetMediaManager.AssetMedia am = mMedia.getAssetMedia(audioList.get(position));
                    if( am != null ) {
                        mEngine.getProject().setBackgroundMusicPath(audioList.get(position));
                    }
                    /*
                    else{
                        nexBeatTemplateManager.BeatTemplate mt = mMTman.getBeatTemplate(audioList.get(position));
                        if( mt != null){
                            reloadClipInEngine();
                            mMTman.applyTemplateToProjectById(mEngine.getProject(),mt.id());
                        }
                    }
                    */
                    mEngine.play();
                    mPlaying = true;
                }
            }
        });


        mStickerList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }

                if (position == 0) {
                    if (mCurrentStickerID != null) {
                        mCurrentStickerID = null;
                        mEngine.getProject().removeOverlay(mCurrentOverlayID);
                        mEngine.seek(0);
                        mCurrentOverlayID = 0;
                    }
                    return;
                }

                boolean setOverlay = false;
                if (mCurrentStickerID == null) {
                    mCurrentStickerID = stickerList.get(position);
                    setOverlay = true;
                } else if (mCurrentStickerID.compareTo(stickerList.get(position)) != 0) {
                    mEngine.getProject().removeOverlay(mCurrentOverlayID);
                    mCurrentStickerID = stickerList.get(position);
                    setOverlay = true;
                }

                if (setOverlay) {
                    nexOverlayImage overlayImage = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getOverlayImage(mCurrentStickerID);
                    Log.d(TAG, "overlayImage duration="+overlayImage.getDefaultDuration());
                    nexOverlayItem overlayItem = new nexOverlayItem(overlayImage, nexOverlayItem.AnchorPoint_MiddleMiddle, false, 0, 0, 0, 6000);
                    mCurrentOverlayID = overlayItem.getId();
                    mEngine.getProject().addOverlay(overlayItem);
                    mEngine.seek(0);
                    mEngine.play();
                }
            }
        });


        mTextList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                    return;
                }

                mTitle.clearOverlayToProject(mEngine.getProject());

                if (position == 0) {
                    mEngine.seek(0);
                    mEngine.resume();
                    mPlaying = true;
                    return;
                } else if ( checkAsset(textList.get(position)) == false ) {
                    Toast.makeText(getApplicationContext(), "This is expired asset!", Toast.LENGTH_SHORT).show();
                    return;
                }

                List<nexOverlayManager.nexTitleInfo> overlays = new ArrayList<nexOverlayManager.nexTitleInfo>();
                mTitle.parseOverlay(textList.get(position), overlays);

                if (overlays.size() == 0) {
                    return;
                }

                String TitleEffectText = "new text for overlay 1@!Ab Ta.";

                for (nexOverlayManager.nexTitleInfo title : overlays) {
                    title.setText("");
                }

                nexOverlayManager.nexTitleInfo title = overlays.get(0);
                int fontSize = 200;

                if (fontSize > title.getMaxFontSize()) {
                    fontSize = title.getMaxFontSize();
                }

                Log.d(TAG, "getFontSize default size=" + title.getFontSize() + ", set size=" + fontSize + ", max=" + title.getMaxFontSize());
                title.setFontSize(fontSize);
                title.setFontID("system.cursiveb");

                int i = 0;
                String sub = null;
                for (i = 0; i < TitleEffectText.length(); i++) {
                    sub = TitleEffectText.substring(0, i);

                    int textWidth = title.getTextWidth(sub);
                    int textOverlayWidth = title.getOverlayWidth();
                    if (textOverlayWidth < textWidth) {
                        if (i != 0)
                            sub = TitleEffectText.substring(0, i - 1);
                        break;
                    }
                }
                if (sub != null) {
                    title.setText(sub);
                }

                mTitle.applyOverlayToProjectById(mEngine.getProject(), textList.get(position), overlays);
                mEngine.seek(0);
                mEngine.resume();
                mPlaying = true;
            }
        });

        mFontList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                    return;
                }
                if (position == 0) {
                    if (mCurrentFontID != null) {
                        mCurrentFontID = null;
                        mEngine.getProject().removeOverlay(mCurrentTextID);
                        mEngine.seek(0);
                        mCurrentTextID = 0;
                    }
                    return;
                }

                if (mCurrentFontID == null) {
                    mCurrentFontID = fontList.get(position);
                } else if (mCurrentFontID.compareTo(fontList.get(position)) != 0) {
                    mEngine.getProject().removeOverlay(mCurrentTextID);
                    mCurrentFontID = fontList.get(position);
                }


                int x = mEngine.getLayerWidth() / 2;
                ;
                int y = mEngine.getLayerHeight() - 160;

                nexOverlayKineMasterText textOverlay = new nexOverlayKineMasterText(getApplicationContext(), "NexStreaming", 50);

                textOverlay.setFontId(mCurrentFontID);

                nexOverlayItem overlayItem = new nexOverlayItem(textOverlay, x, y, 0, 6000);

                mCurrentTextID = overlayItem.getId();

                mEngine.getProject().addOverlay(overlayItem);
                mEngine.seek(0);
            }
        });

        mCollageList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                    return;
                }

                //

                if (position == 0) {
                    reloadClipInEngine();
                    int h = mView.getHeight();
                    int w = (int)((float)h * contentAspectProfile.getAspectRatio());
                    nexApplicationConfig.setAspectMode(contentAspectProfile.getAspectMode());
                    mView.setLayoutParams(new FrameLayout.LayoutParams(w,h, Gravity.CENTER));
                    mCurrentTSelect = 0;
                    mEngine.updateProject();
                    mEngine.seek(0);
                    return;
                    /*
                    if (mCurrentCollageID != null) {
                        mCurrentCollageID = null;
                    }
                    return;
                    */
                }else{
                    nexCollageManager.Collage collage = mCollage.getCollage(collageList.get(position));
                    if( collage != null ){
                        mCurrentTSelect = -1;
                        reloadClipInEngine();
                        try {
                            collage.applyCollage2Project(mEngine.getProject(),mEngine,0,getApplicationContext());
                        } catch (ExpiredTimeException e) {
                            e.printStackTrace();
                        }
                        int h = mView.getHeight();
                        int w = (int)((float)h * collage.getRatio());
                        nexApplicationConfig.setAspectMode(collage.getRatioMode());
                        mView.setLayoutParams(new FrameLayout.LayoutParams(w,h, Gravity.CENTER));
                        if( collage.getType() == nexCollageManager.CollageType.StaticCollage ){
                            mEngine.seek(collage.getEditTime());
                        }else{
                            mEngine.seek(0);
                            mEngine.resume();
                            mPlaying = true;
                        }
                    }else{
                        Toast.makeText(getApplicationContext(), "collage not found!="+collageList.get(position), Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });

        Button bt_export = (Button) findViewById(R.id.button_template_man_export);
        bt_export.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine.getProject() == null) {
                    Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                    return;
                }
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });
        bt_export.setEnabled(true);

        Button bt_intent = (Button) findViewById(R.id.button_template_man_intent);
        bt_intent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (nexAssetStoreAppUtils.isInstalledKineMaster(getApplicationContext())) {
                    if (mEngine.getProject() == null) {
                        Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                        return;
                    }
                    if (mPlaying) {
                        mEngine.stop();
                        mPlaying = false;
                    }
                    if( mOriginalProject != null ) {
                        if( mOriginalProject.getTotalClipCount(true) > 0 ) {
                            //startActivity(mOriginalProject.makeKineMasterIntent().setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK));
                            UtilityCode.launchKineMaster(EditorSampleTemplateActivity.this, mListFilePath);
                        }
                    }
                } else {
                    Toast.makeText(getApplicationContext(), "KineMaster not found.", Toast.LENGTH_LONG).show();
                    nexAssetStoreAppUtils.moveGooglePlayKineMaster(getApplicationContext());
                }
            }
        });


        Button bt_new = (Button) findViewById(R.id.button_template_man_feat);
        bt_new.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }

                if (nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext())) {
                    showDialogFeaturedList();
                } else {
                    Toast.makeText(getApplicationContext(), "Asset store not found.", Toast.LENGTH_LONG).show();
                    nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                }
            }
        });

        if (localAssetStore) {
            bt_new.setEnabled(false);
        }

        Button bt_install = (Button) findViewById(R.id.button_template_man_install);
        bt_install.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }

                if (localAssetStore) {
                    Intent intent = new Intent(getApplicationContext(), LocalAssetStoreSampleActivity.class);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);

                    startActivity(intent);
                } else {
                    if (nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext())) {
                        String extra = null;
                        int selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_Template;
                        switch (tabHost.getCurrentTab()) {
                            case 1: //audio
                                selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_Audio;
                                break;
                            case 2: //title
                                selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_TitleTemplate;
                                break;
                            case 5://collage
                                selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage;
                                extra = "{\"UserSources\": "+1+" }";
                                break;
                        }

                        nexAssetStoreAppUtils.setMimeType(selectMainPage);
                        nexAssetStoreAppUtils.setMimeTypeExtra(extra);
                        nexAssetStoreAppUtils.runAssetStoreApp(EditorSampleTemplateActivity.this, null);
                    } else {
                        Toast.makeText(getApplicationContext(), "Asset store not found.", Toast.LENGTH_LONG).show();
                        nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                    }
                }
            }
        });

        Button bt_uninstall = (Button) findViewById(R.id.button_template_man_uninstall);
        bt_uninstall.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mPlaying) {
                    mEngine.stop();
                    mPlaying = false;
                }

                int index = tabHost.getCurrentTab();
                int last;
                nexAssetPackageManager.Item item = null;
                switch (index) {
                    case 0:
                        if (templateList.size() < 2) {
                            return;
                        }

                        for (nexTemplateManager.Template template : mTman.getTemplates()) {
                            if (template.isDelete()) {
                                reloadClipInEngine();
                                mEngine.updateProject();
                                mTman.uninstallPackageById(template.id());
                                updateAssets(false,template.packageInfo().assetIdx());
                                mCurrentTSelect = 0;
                                break;
                            }
                        }
                        break;
                    case 1:
                        if (audioList.size() < 2) {
                            return;
                        }

                        for (nexAssetMediaManager.AssetMedia media : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO)) {
                            if (media.isDelete()) {
                                int assetIndex = media.packageInfo().assetIdx();
                                nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(media.id());
                                updateAssets(false,assetIndex);
                                reloadClipInEngine();
                                mEngine.updateProject();
                                break;
                            }
                        }

                        break;
                    case 2:
                        if (textList.size() < 2) {
                            return;
                        }

                        for (nexOverlayManager.Overlay overlay : mTitle.getOverlays()) {
                            if (overlay.isDelete()) {
                                int assetIndex = overlay.packageInfo().assetIdx();
                                mTitle.uninstallPackageById(overlay.getId());
                                updateAssets(false,assetIndex);
                                reloadClipInEngine();
                                mEngine.updateProject();
                                break;
                            }
                        }

                        break;

                    case 3:
                        if (stickerList.size() < 2) {
                            return;
                        }
                        last = stickerList.size() - 1;
                        item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(stickerList.get(last));

                        if (item != null && item.isDelete()) {
                            reloadClipInEngine();
                            mEngine.updateProject();
                            int assetIndex = item.packageInfo().assetIdx();
                            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(stickerList.get(last));
                            updateAssets(false,assetIndex);
                        } else {
                            Toast.makeText(getApplicationContext(), "can not uninstall.", Toast.LENGTH_LONG).show();
                        }

                        break;
                    case 5:
                        if( collageList.size() <2){
                            return;
                        }

                        for(nexCollageManager.Collage collage : mCollage.getCollages(mOriginalProject.getTotalClipCount(true),nexCollageManager.CollageType.ALL) ){
                            if (collage.isDelete()) {

                                reloadClipInEngine();
                                int assetIdx = collage.packageInfo().assetIdx();
                                Log.d(TAG,"uninstall collage id="+collage.id()+", idx="+assetIdx);
                                mEngine.updateProject();
                                mCollage.uninstallPackageById(collage.id());
                                updateAssets(false,assetIdx);
                                break;
                            }
                        }
                    default:
                        break;
                }
            }
        });

        mView = (nexEngineView) findViewById(R.id.engineview_template_man);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(mView);
        reloadClipInEngine();
        mEngine.updateScreenMode();
        mEngine.updateProject();
        mView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                mEngineViewAvailable = true;
                Log.d(TAG,"onEngineViewAvailable() seek = "+mEngine.getCurrentPlayTimeTime());
                mEngine.seek(mEngine.getCurrentPlayTimeTime());
            }

            @Override
            public void onEngineViewSizeChanged(int width, int height) {

            }

            @Override
            public void onEngineViewDestroyed() {
                mEngineViewAvailable = false;
            }
        });
        updateList();
    }

    private void updateAssets(boolean installed , int assetIdx){

        nexAssetPackageManager.PreAssetCategoryAlias[] cats = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).updateAssetInManager(installed, assetIdx);

        for (nexAssetPackageManager.PreAssetCategoryAlias c : cats) {
            Log.d(TAG,"updateAssets("+installed+","+assetIdx+", update module="+c.toString());
            if (c == nexAssetPackageManager.PreAssetCategoryAlias.Template) {
                if(installed ) {
                    templateList.add(mTman.getTemplateItemId(false, assetIdx));
                }else {
                    templateList.clear();
                    templateList.add("none");
                    for (nexTemplateManager.Template template : mTman.getTemplates()) {
                        templateList.add(template.id());
                    }
                }
                templateAdapter.notifyDataSetChanged();
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.TextEffect) {
                textList.clear();
                textList.add("none");

                for (nexOverlayManager.Overlay overlay : mTitle.getOverlays()) {
                    nexAssetPackageManager.Item item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(overlay.getId());
                    textList.add(overlay.getId());
                }
                textAdapter.notifyDataSetChanged();
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Overlay) {
                stickerList.clear();
                stickerList.add("none");
                String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                for (String id : stickers) {
                    stickerList.add(id);
                }
                stickerAdapter.notifyDataSetChanged();
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Audio || c == nexAssetPackageManager.PreAssetCategoryAlias.BeatTemplate) {
                audioList.clear();
                audioList.add("none");
                /*
                for( nexBeatTemplateManager.BeatTemplate mt : mMTman.getBeatTemplates()){
                    audioList.add(mt.id());
                }
*/
                for (nexAssetMediaManager.AssetMedia info : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO)) {
                    audioList.add(info.id());
                }
                audioAdapter.notifyDataSetChanged();

            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Font) {
                fontList.clear();
                fontList.add("none");
                for (nexFont font : nexFont.getPresetList()) {
                    fontList.add(font.getId());
                }
                fontAdapter.notifyDataSetChanged();
            } else if( c == nexAssetPackageManager.PreAssetCategoryAlias.Collage || c == nexAssetPackageManager.PreAssetCategoryAlias.StaticCollage || c == nexAssetPackageManager.PreAssetCategoryAlias.DynamicCollage ){

                if( installed ){
                    //Todo:
                    String id = mCollage.getCollageItemId(false,assetIdx,mOriginalProject.getTotalClipCount(true));
                    if( id != null)
                        collageList.add(id);
                }else {
                    collageList.clear();
                    collageList.add("none");
                    for( nexCollageManager.Collage collage : mCollage.getCollages(mOriginalProject.getTotalClipCount(true), nexCollageManager.CollageType.ALL) ){
                        collageList.add(collage.id());
                    }
                }
                collageAdapter.notifyDataSetChanged();
            }
        }
    }

    private void updateList() {
        mTman.loadTemplate();
        templateList.clear();
        templateList.add("none");
        for (nexTemplateManager.Template template : mTman.getTemplates()) {
            templateList.add(template.id());
        }

        if (templateAdapter == null) {
            templateAdapter = new CustomViewAdapter(this, templateList);
            mTemplateList.setAdapter(templateAdapter);
        } else {
            templateAdapter.notifyDataSetChanged();
        }

        mMedia.loadMedia(nexClip.kCLIP_TYPE_AUDIO);
        //mMTman.loadTemplate();
        audioList.clear();
        audioList.add("none");
        /*
        for( nexBeatTemplateManager.BeatTemplate mt : mMTman.getBeatTemplates()){
            audioList.add(mt.id());
        }
*/
        for (nexAssetMediaManager.AssetMedia info : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO)) {
            audioList.add(info.id());
        }

        if (audioAdapter == null) {
            audioAdapter = new CustomViewAdapter(this, audioList);
            mAudioList.setAdapter(audioAdapter);
        } else {
            audioAdapter.notifyDataSetChanged();
        }

        stickerList.clear();
        String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
        stickerList.add("none");
        for (String id : stickers) {
            stickerList.add(id);
        }

        if (stickerAdapter == null) {
            stickerAdapter = new CustomViewAdapter(this, stickerList);
            mStickerList.setAdapter(stickerAdapter);
        } else {
            stickerAdapter.notifyDataSetChanged();
        }

        textList.clear();
        textList.add("none");
        mTitle = nexOverlayManager.getOverlayManager(getApplicationContext(), getApplicationContext());
        mTitle.loadOverlay();

        for (nexOverlayManager.Overlay overlay : mTitle.getOverlays()) {
            nexAssetPackageManager.Item item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(overlay.getId());
            textList.add(overlay.getId());
        }
        if (textAdapter == null) {
            textAdapter = new CustomViewAdapter(this, textList);
            mTextList.setAdapter(textAdapter);
        } else {
            textAdapter.notifyDataSetChanged();
        }

        fontList.clear();
        fontList.add("none");
        for (nexFont font : nexFont.getPresetList()) {
            fontList.add(font.getId());
        }

        if (fontAdapter == null) {
            fontAdapter = new CustomViewAdapter(this, fontList);
            mFontList.setAdapter(fontAdapter);
        } else {
            fontAdapter.notifyDataSetChanged();
        }
        nexColorEffect.updatePluginLut();

        mCollage.loadCollage();
        collageList.clear();
        collageList.add("none");

        for( nexCollageManager.Collage collage : mCollage.getCollages(mOriginalProject.getTotalClipCount(true), nexCollageManager.CollageType.ALL) ){
            collageList.add(collage.id());
        }

        if( collageAdapter == null ){
            collageAdapter =new CustomViewAdapter(this, collageList);
            mCollageList.setAdapter(collageAdapter);
        }else{
            collageAdapter.notifyDataSetChanged();
        }
    }

    private void reloadClipInEngine() {
        if (mOriginalProject == null) {
            mOriginalProject = new nexProject();
            for (String path : mListFilePath) {
                nexClip clip = nexClip.getSupportedClip(path);
                if (clip != null) {
                    mOriginalProject.add(clip);
                    if (clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        mOriginalProject.getLastPrimaryClip().setRotateDegree(clip.getRotateInMeta());
                        mOriginalProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    } else if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                        mOriginalProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
                    }
                }
            }
        }
        mEngine.setProject(nexProject.clone(mOriginalProject));
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause...");
        if( mEngine != null ){
            mEngine.stop();
            mPlaying = false;
        }

        super.onPause();
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart...");
        if (mEngine != null) {
            mEngine.setEventHandler(mEngineListener);
            if( mEngineViewAvailable ) {
                Log.d(TAG, "onStart seek=" + mEngine.getCurrentPlayTimeTime());
                mEngine.seek(mEngine.getCurrentPlayTimeTime());
            }

        }

        super.onStart();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop...");
        if (mEngine != null) {

            mEngine.stop();
            mPlaying = false;
        }
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");

        //3.  BroadcastReceiver was unregisterReceiver
        unregisterReceiver(broadcastReceiver);
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        //super.onBackPressed();
        if (System.currentTimeMillis() > backKeyPressedTime + 2000) {
            backKeyPressedTime = System.currentTimeMillis();
            Toast.makeText(getApplicationContext(), "finish, push on back button.", Toast.LENGTH_SHORT).show();

            return;
        }

        if (System.currentTimeMillis() <= backKeyPressedTime + 2000) {

            if (mTman != null) {
                mTman.cancel();
            }

            if (mEngine != null) {
                mEngine.stop();
                mPlaying = false;

                mEngine = null;
            }
            if( mlnitAspectMode > 0 ) {
                nexApplicationConfig.setAspectMode(mlnitAspectMode);
            }
            finish();
        }
    }

    public class AdaptorFeaturedListItem extends BaseAdapter {
        private nexAssetPackageManager.RemoteAssetInfo [] hotList;
        private nexAssetPackageManager.RemoteAssetInfo [] newList;
        private int hotCount = 0;
        private int newCount = 0;

        public AdaptorFeaturedListItem(){
            hotList = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getRemoteAssetInfos(nexAssetPackageManager.Mode_Hot);
            if( hotList != null ){
                hotCount = hotList.length;
            }
            newList = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getRemoteAssetInfos(nexAssetPackageManager.Mode_New);
            if( newList != null ){
                newCount = newList.length;
            }
        }

        @Override
        public int getCount() {
            return  hotCount+newCount;
        }

        @Override
        public Object getItem(int arg0) {
            if( hotCount > arg0 ){
                return hotList[arg0];
            }else{
                return newList[arg0-hotCount];
            }
        }

        @Override
        public long getItemId(int arg0) {
            if( hotCount > arg0 ){
                return hotList[arg0].idx();
            }else{
                return newList[arg0-hotCount].idx();
            }
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            nexAssetPackageManager.RemoteAssetInfo remoteAssetInfo = (nexAssetPackageManager.RemoteAssetInfo)getItem(arg0);

            if( remoteAssetInfo == null )
                return null;


            LayoutInflater inflater = EditorSampleTemplateActivity.this.getLayoutInflater();
            View rowView = inflater.inflate(R.layout.listitem_assets, null, true);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);
            TextView status = (TextView)rowView.findViewById(R.id.status);

            image.setImageBitmap(remoteAssetInfo.icon());
            name.setText(remoteAssetInfo.name());

            if( hotCount > arg0 ){
                status.setText("Hot");
            }else{
                status.setText("New");
            }
            return rowView;
        }
    }

    private AlertDialog featuredDialog;
    public void showDialogFeaturedList()
    {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(EditorSampleTemplateActivity.this);

        LayoutInflater inflater = EditorSampleTemplateActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_sel_overlay_title, null);

        final LinearLayout ll = (LinearLayout)dialogView.findViewById(R.id.layout_title_info);

        ListView featuredList = (ListView)dialogView.findViewById(R.id.listview_title);
        final AdaptorFeaturedListItem adt = new AdaptorFeaturedListItem();
        featuredList.setAdapter(adt);
        featuredList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                int assetIdx = (int)id;
                Log.d(TAG,"select asset idx="+assetIdx);
                nexAssetStoreAppUtils.runAssetStoreApp(EditorSampleTemplateActivity.this,""+assetIdx);
                featuredDialog.dismiss();
            }
        });

        dialogBuilder.setView(dialogView).
                setTitle("Select Asset").
                setPositiveButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {

                    }
                });
        featuredDialog = dialogBuilder.create();
        featuredDialog.show();
    }

    public boolean checkAsset(String itemId){
        final nexAssetPackageManager.Item item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(itemId);
        if( item != null ){
            if( item.packageInfo() != null ){
                if( nexAssetPackageManager.checkExpireAsset(item.packageInfo()) ){
                    // expire asset
                    //TODO: popup dialog
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Asset Expired!");
                    builder.setMessage("Asset expired ");
                    builder.setNeutralButton("uninstall", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            //uninstall asset
                            int assetIdx = item.packageInfo().assetIdx();
                            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageByAssetIdx(assetIdx);
                            //update ui
                            updateAssets(false,assetIdx);
                        }
                    });
                    builder.setNegativeButton("re-install", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            //move Vasset for re-install asset.
                            nexAssetStoreAppUtils.runAssetStoreApp(EditorSampleTemplateActivity.this,""+item.packageInfo().assetIdx());
                            //finish();
                        }
                    });
                    builder.show();
                }else{
                    return true;
                }
            }
            return false;
        }
        return false;
    }
}
