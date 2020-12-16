package com.nexstreaming.editorsimple;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBarActivity;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexAssetMediaManager;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterText;
import com.nexstreaming.nexeditorsdk.nexOverlayManager;
import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdk.service.nexAssetService;

import java.util.ArrayList;
import java.util.List;

public class EditorActivity extends ActionBarActivity {
    private final static String TAG="EditorActivity";
    private ListView mTemplateList;
    private ListView mAudioList;
    private ListView mStickerList;
    private ListView mTextList;
    private ListView mFontList;

    private ArrayList<String> mListFilePath;
    private nexTemplateManager mTman;
    private nexOverlayManager mTitle;
    private nexAssetMediaManager mMedia;
    private nexEngineView mView;
    private nexClip mClip;

    private int mlnitAspectMode;
    private int mCurrentTSelect = 0;
    private long backKeyPressedTime = 0;
    ArrayList<ListViewItem> templateList =null;
    static CustomViewAdapter templateAdapter = null;
    ArrayList<ListViewItem> audioList=null;
    static CustomViewAdapter audioAdapter = null;
    ArrayList<ListViewItem> stickerList=null;
    static CustomViewAdapter stickerAdapter = null;

    ArrayList<ListViewItem> fontList=null;
    static CustomViewAdapter fontAdapter = null;

    ArrayList<ListViewItem> textList=null;
    static CustomViewAdapter textAdapter = null;

    private static nexEngine sEngine = null;
    private static boolean mPlaying = false;
    private TabHost tabHost;
    private String mCurrentStickerID;
    private  int mCurrentOverlayID;

    private String mCurrentFontID;
    private  int mCurrentTextID;

    private boolean localAssetStore;

    private ProgressDialog progressDialog;

    private static nexEngineListener sEngineListener = new nexEngineListener() {
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
            mPlaying = false;
            sEngine.seek(0);
        }

        @Override
        public void onPlayFail(int i, int i1) {
            mPlaying = false;
            sEngine.seek(0);
        }

        @Override
        public void onPlayStart() {
            mPlaying =true;
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
        public void onPreviewPeakMeter(int iCts, int iPeakMeterValue) {

        }
    };

    //1. create BroadcastReceiver
    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                if (nexAssetService.ACTION_ASSET_INSTALL_COMPLETED.equals(action)) {
                    //onUpdateAssetList();
                    int idx = intent.getIntExtra("index",-1);
                    String categoryName = intent.getStringExtra("category.alias");
                    Log.d(TAG,"installed Asset, categoryName="+categoryName+", index="+idx);
                    //TODO : update UI
                    updateList();
                }else if( nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED.equals(action) ){
                    int idx = intent.getIntExtra("index",-1);
                    Log.d(TAG,"uninstalled Asset, index="+idx);
                    updateList();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //2. BroadcastReceiver was set intent filter.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(nexAssetService.ACTION_ASSET_INSTALL_COMPLETED);
        intentFilter.addAction(nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED);
        registerReceiver(broadcastReceiver, intentFilter);

        Intent intent = getIntent();
        mListFilePath = intent.getStringArrayListExtra("filelist");
        localAssetStore = intent.getBooleanExtra("localAssetStore",false);

        if( !localAssetStore ) {
            if (nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext())) {
                Log.d(TAG, "Asset store installed");
                nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
            } else {
                Log.d(TAG, "Asset store was not installed");
            }
        }

        nexApplicationConfig.createApp(getApplicationContext());
        nexApplicationConfig.init(getApplicationContext(), "nexEditorSimple");

        Log.d(TAG, "nexEditor init end. and make clip start");
        mClip = nexClip.getSupportedClip(mListFilePath.get(0));
        if (mClip == null) {
            Toast.makeText(getApplicationContext(), "not supported clip!", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        mlnitAspectMode = nexApplicationConfig.getAspectRatioMode();

        int rotate = 0;
        int width = 0;
        int height = 0;
        rotate = mClip.getRotateInMeta();
        width = mClip.getWidth();
        height = mClip.getHeight();

        int mode = nexAspectProfile.getSimilarAspectMode(width,height,rotate );

        if (mode != mlnitAspectMode) {
            Log.d(TAG, "aspect ratio change mode=" + mode);
            nexApplicationConfig.setAspectMode(mode);
        }
        Log.d(TAG, "aspect ratio setting end.");
        setContentView(R.layout.activity_template_manager);

        tabHost = (TabHost)findViewById(R.id.tabhost);
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

        // show First Tab Content
        tabHost.setCurrentTab(0);

        mTemplateList = (ListView) findViewById(R.id.listview_template_man);
        mAudioList = (ListView) findViewById(R.id.listview_audio_man);
        mStickerList = (ListView) findViewById(R.id.listview_sticker_man);
        mTextList = (ListView) findViewById(R.id.listview_text_man);
        mFontList = (ListView) findViewById(R.id.listview_font_man);

        mTman = nexTemplateManager.getTemplateManager(getApplicationContext(), getApplicationContext());
        mTman.loadTemplate();

        templateList = new ArrayList<ListViewItem>();
        templateList.clear();
        templateList.add(new ListViewItem(null, "Normal", nexApplicationConfig.getAspectRatio(), "none"));
        for (nexTemplateManager.Template template : mTman.getTemplates()) {
            Bitmap icon = template.thumbnail();
            if( icon == null ){
                icon = template.icon();
            }
            Log.d(TAG,"validateAssetPackage Asset1 idx="+template.packageInfo().assetIdx() );
            templateList.add(new ListViewItem(icon, template.name("en"), template.aspect(), template.id(), template.packageInfo().assetIdx()));
        }
        templateAdapter = new CustomViewAdapter(this, templateList);

        mTemplateList.setAdapter(templateAdapter);
        mTemplateList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mCurrentTSelect != position) {
                    if (mPlaying) {
                        sEngine.stop();
                        mPlaying = false;
                    }

                    reloadClipInEngine();
                    if (position == 0) {
                        sEngine.updateProject();
                        sEngine.seek(0);
                        mCurrentTSelect = position;

                    } else {
                        boolean isOk = true;
                        Log.d(TAG,"validateAssetPackage Asset idx="+templateList.get(position).getAssetIdx() );
                        if( templateList.get(position).getAssetIdx() > 3 ){
                            Log.d(TAG,"validateAssetPackage start");
                            isOk = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).validateAssetPackage(templateList.get(position).getAssetIdx());
                            Log.d(TAG,"validateAssetPackage end");
                        }
                        if( isOk ) {
                            boolean asyncMode = mClip.getTotalTime() > 60*60*1000;
                            if( asyncMode ){
                                if( progressDialog == null )
                                    progressDialog = new ProgressDialog(EditorActivity.this);
                                progressDialog.show();

                                mTman.applyTemplateAsyncToProjectById(sEngine.getProject(), templateList.get(position).getAssetID(), new Runnable() {
                                    @Override
                                    public void run() {
                                        sEngine.updateProject();
                                        progressDialog.dismiss();
                                        sEngine.seek(0);
                                    }
                                });
                                mCurrentTSelect = position;
                            }else {
                                mTman.applyTemplateToProjectById(sEngine.getProject(), templateList.get(position).getAssetID());
                                sEngine.updateProject();
                                sEngine.seek(0);
                                mCurrentTSelect = position;
                            }
                        }else{
                            Toast.makeText(getApplicationContext(), "Template miss resource!", Toast.LENGTH_SHORT).show();
                        }
                    }
                } else {
                    if (mPlaying) {
                        sEngine.stop();
                        mPlaying = false;
                    } else {
                        sEngine.resume();
                        mPlaying = true;
                    }
                }
            }
        });

        mMedia = nexAssetMediaManager.getAudioManager(getApplicationContext());
        mMedia.loadMedia(nexClip.kCLIP_TYPE_AUDIO);
        audioList = new ArrayList<ListViewItem>();
        audioList.clear();

        audioList.add(new ListViewItem(null, "None", 0, "none"));

        for( nexAssetMediaManager.AssetMedia info : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO)){
            if( !info.filter() ) {
                Bitmap bmp = info.thumbnail();
                if(bmp == null ){
                    Log.d(TAG,"thumbnail is null");
                    bmp = info.icon();
                }
                audioList.add(new ListViewItem(bmp, info.name("en"), 0, info.id()));
            }
        }
        audioAdapter = new CustomViewAdapter(this, audioList);

        mAudioList.setAdapter(audioAdapter);
        mAudioList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                }

                if( position == 0 ){
                    sEngine.getProject().setBackgroundMusicPath(null);
                    sEngine.play();
                    mPlaying = true;
                }else {
                    sEngine.getProject().setBackgroundMusicPath(audioList.get(position).getAssetID());
                    sEngine.play();
                    mPlaying = true;
                }
            }
        });

        String ids[] = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();

        stickerList = new ArrayList<ListViewItem>();
        stickerList.clear();

        stickerList.add(new ListViewItem(null, "None", 0, "None"));
        for(  String id : ids  ) {
            Bitmap icon = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIcon(id,300,300);
            stickerList.add(new ListViewItem(icon, id, 0, id));

            Log.d(TAG, "stickers id=" + id);
        }
        stickerAdapter = new CustomViewAdapter(this, stickerList);

        mStickerList.setAdapter(stickerAdapter);
        mStickerList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                }

                if( position == 0){
                    if( mCurrentStickerID != null ) {
                        mCurrentStickerID = null;
                        sEngine.getProject().removeOverlay(mCurrentOverlayID);
                        sEngine.seek(0);
                        mCurrentOverlayID = 0;
                    }
                    return;
                }

                boolean setOverlay = false;
                if (mCurrentStickerID == null) {
                    mCurrentStickerID = stickerList.get(position).getAssetID();
                    setOverlay = true;
                } else if (mCurrentStickerID.compareTo(stickerList.get(position).getAssetName()) != 0) {
                    sEngine.getProject().removeOverlay(mCurrentOverlayID);
                    mCurrentStickerID = stickerList.get(position).getAssetID();
                    setOverlay = true;
                }

                if (setOverlay) {
                    int x = sEngine.getLayerWidth()/2;
                    int y = sEngine.getLayerHeight()/2;
                    //Log.d(TAG,"x="+x+"y="+y);
                    nexOverlayImage overlayImage = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getOverlayImage(mCurrentStickerID);
                    nexOverlayItem overlayItem = new nexOverlayItem(overlayImage, x, y, 0, 6000);
                    mCurrentOverlayID = overlayItem.getId();
                    sEngine.getProject().addOverlay(overlayItem);
                    sEngine.seek(0);
                }
            }
        });


        textList = new ArrayList<ListViewItem>();
        textList.clear();

        textList.add(new ListViewItem(null, "None", nexApplicationConfig.getAspectRatio(), "None"));

        mTitle = nexOverlayManager.getOverlayManager(getApplicationContext(),getApplicationContext());
        mTitle.loadOverlay();

        for(nexOverlayManager.Overlay overlay : mTitle.getOverlays()){
            Bitmap bmp = overlay.getThumbnail();
            if( bmp == null ){
                bmp = overlay.getIcon();
            }
                textList.add(new ListViewItem(bmp,overlay.getName("en"),overlay.getRatio(),overlay.getId()));
        }
        textAdapter = new CustomViewAdapter(this, textList);

        mTextList.setAdapter(textAdapter);
        mTextList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                    return;
                }

                mTitle.clearOverlayToProject(sEngine.getProject());

                if( position == 0){
                    sEngine.seek(0);
                    sEngine.resume();
                    mPlaying = true;
                    return;
                }

                List<nexOverlayManager.nexTitleInfo> overlays = new ArrayList<nexOverlayManager.nexTitleInfo>();
                mTitle.parseOverlay(textList.get(position).getAssetID(),overlays);

                String TitleEffectText = "new text for overlay 1@!Ab Ta.";

                for(nexOverlayManager.nexTitleInfo title : overlays ){
                    title.setText("");
                }

                nexOverlayManager.nexTitleInfo title = overlays.get(0);
                int fontSize = 200;

                if( fontSize > title.getMaxFontSize() ){
                    fontSize = title.getMaxFontSize();
                }

                Log.d(TAG,"getFontSize default size="+title.getFontSize()+", set size="+fontSize+", max="+title.getMaxFontSize());
                title.setFontSize(fontSize);
                title.setFontID("system.cursiveb");
                int i = 0;
                String sub = null;
                for(  i= 0 ; i < TitleEffectText.length() ; i++ ) {
                    sub = TitleEffectText.substring(0,i);

                    int textWidth = title.getTextWidth(sub);
                    int textOverlayWidth = title.getOverlayWidth();
                    if (textOverlayWidth < textWidth) {
                        if( i != 0 )
                            sub = TitleEffectText.substring(0,i-1);
                        break;
                    }
                }
                if( sub != null ) {
                    //title.setFontID("system.cursiveb");
                    title.setText(sub);
                }

                mTitle.applyOverlayToProjectById(sEngine.getProject(),textList.get(position).getAssetID(),overlays);
                sEngine.seek(0);
                sEngine.resume();
                mPlaying = true;
            }
        });


        fontList = new ArrayList<ListViewItem>();
        fontList.clear();

        fontList.add(new ListViewItem(null, "None", 0, "None"));


        for( nexFont font : nexFont.getPresetList() ){
            fontList.add(new ListViewItem(font.getSampleImage(getApplicationContext()), font.getId(), 0, font.getId()));
        }

        fontAdapter = new CustomViewAdapter(this, fontList);

        mFontList.setAdapter(fontAdapter);
        mFontList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                    return;
                }
                if( position == 0){
                    if( mCurrentFontID != null ) {
                        mCurrentFontID = null;
                        sEngine.getProject().removeOverlay(mCurrentTextID);
                        sEngine.seek(0);
                        mCurrentTextID = 0;
                    }
                    return;
                }

                if (mCurrentFontID == null) {
                    mCurrentFontID = fontList.get(position).getAssetID();
                } else if (mCurrentFontID.compareTo(fontList.get(position).getAssetID()) != 0) {
                    sEngine.getProject().removeOverlay(mCurrentTextID);
                    mCurrentFontID = fontList.get(position).getAssetID();
                }


                int x = sEngine.getLayerWidth()/2;;
                int y = sEngine.getLayerHeight()- 160;

                nexOverlayKineMasterText textOverlay = new nexOverlayKineMasterText(getApplicationContext(),"NexStreaming",50);

                textOverlay.setFontId(mCurrentFontID);

                nexOverlayItem overlayItem = new nexOverlayItem(textOverlay, x, y, 0, 6000);

                mCurrentTextID = overlayItem.getId();

                sEngine.getProject().addOverlay(overlayItem);
                sEngine.seek(0);
            }
        });

        Button bt_export = (Button) findViewById(R.id.button_template_man_export);
        bt_export.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(sEngine.getProject() == null) {
                    Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                    return ;
                }
                if( mPlaying ) {
                    sEngine.stop();
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
                if( nexAssetStoreAppUtils.isInstalledKineMaster(getApplicationContext()) ) {
                    if (sEngine.getProject() == null) {
                        Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                        return;
                    }
                    if (mPlaying) {
                        sEngine.stop();
                        mPlaying = false;
                    }

                    startActivity(sEngine.getProject().makeKineMasterIntent().setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_MULTIPLE_TASK));
                }else{
                    Toast.makeText(getApplicationContext(), "KineMaster not found.", Toast.LENGTH_LONG).show();
                    nexAssetStoreAppUtils.moveGooglePlayKineMaster(getApplicationContext());
                }
            }
        });

        Button bt_hot = (Button) findViewById(R.id.button_template_man_hot);
        bt_hot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext()) ) {
                    if (mPlaying) {
                        sEngine.stop();
                        mPlaying = false;
                    }

                    nexAssetPackageManager.RemoteAssetInfo[] rInfo = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getRemoteAssetInfos(nexAssetPackageManager.Mode_Hot);
                    if( rInfo.length == 0 ){
                        Toast.makeText(getApplicationContext(), "Hot Item not found.", Toast.LENGTH_LONG).show();

                        nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
                        nexAssetStoreAppUtils.runAssetStoreApp(EditorActivity.this ,"FL"+nexAssetPackageManager.Mode_Hot);
                        return;
                    }

                    Log.d(TAG,"move Hot item="+rInfo[0].idx());
                    nexAssetStoreAppUtils.runAssetStoreApp(EditorActivity.this ,""+rInfo[0].idx());

                }else{
                    Toast.makeText(getApplicationContext(), "Asset store not found.", Toast.LENGTH_LONG).show();
                    nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                }
            }
        });

        if( localAssetStore ) {
            bt_hot.setEnabled(false);
        }

        Button bt_new = (Button) findViewById(R.id.button_template_man_new);
        bt_new.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext()) ) {
                    if (mPlaying) {
                        sEngine.stop();
                        mPlaying = false;
                    }

                    nexAssetPackageManager.RemoteAssetInfo rInfo = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getRemoteAssetInfo(nexAssetPackageManager.Mode_New);
                    if( rInfo == null ){
                        Toast.makeText(getApplicationContext(), "New Item not found.", Toast.LENGTH_LONG).show();

                        nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
                        nexAssetStoreAppUtils.runAssetStoreApp(EditorActivity.this ,"FL"+nexAssetPackageManager.Mode_New);
                        return;
                    }
                    Log.d(TAG,"move New item="+rInfo.idx());
                    nexAssetStoreAppUtils.runAssetStoreApp(EditorActivity.this ,""+rInfo.idx());

                }else{
                    Toast.makeText(getApplicationContext(), "Asset store not found.", Toast.LENGTH_LONG).show();
                    nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                }
            }
        });

        if( localAssetStore ) {
            bt_new.setEnabled(false);
        }

        Button bt_install = (Button) findViewById(R.id.button_template_man_install);
        bt_install.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                }

                int count = mTman.findNewPackages();
                if(  count > 0 ){
                    Toast.makeText(getApplicationContext(), "new package found. start install", Toast.LENGTH_LONG).show();
                    showInstallDialog();
                }else{

                    if( localAssetStore ) {
                        Intent intent = new Intent(getApplicationContext(), LocalAssetStoreSampleActivity.class);
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_MULTIPLE_TASK );

                        startActivity(intent);
                    }else {
                        if (nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext())) {
                            int selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_Template;
                            switch (tabHost.getCurrentTab()) {
                                case 1: //audio
                                    selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_Audio;
                                    break;
                                case 2: //title
                                    selectMainPage = nexAssetStoreAppUtils.AssetStoreMimeType_TitleTemplate;
                                    break;
                            }

                            nexAssetStoreAppUtils.setMimeType(selectMainPage);
                            nexAssetStoreAppUtils.runAssetStoreApp(EditorActivity.this, null);
                        } else {
                            Toast.makeText(getApplicationContext(), "Asset store not found.", Toast.LENGTH_LONG).show();
                            nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                        }
                    }
                }
            }
        });

        Button bt_uninstall = (Button) findViewById(R.id.button_template_man_uninstall);
        bt_uninstall.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mPlaying) {
                    sEngine.stop();
                    mPlaying = false;
                }

                int index = tabHost.getCurrentTab();
                int last;
                nexAssetPackageManager.Item item = null;
                switch (index){
                    case 0:
                        if( templateList.size() < 2 ){
                            return;
                        }

                        for (nexTemplateManager.Template template : mTman.getTemplates()) {
                            if( template.isDelete() ){
                                reloadClipInEngine();
                                sEngine.updateProject();
                                mTman.uninstallPackageById(template.id());
                                updateList();
                                mCurrentTSelect = 0;
                                break;
                            }
                        }
                        break;
                    case 1:
                        if( audioList.size() < 2 ){
                            return;
                        }

                        for(nexAssetMediaManager.AssetMedia media : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO ) ){
                            if( media.isDelete() ){
                                nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(media.id());
                                reloadClipInEngine();
                                sEngine.updateProject();
                                updateList();
                                break;
                            }
                        }

                        break;
                    case 2:
                        if( textList.size() < 3 ){
                            return;
                        }

                        for (nexOverlayManager.Overlay overlay : mTitle.getOverlays()) {
                            if( overlay.isDelete() ){
                                mTitle.uninstallPackageById(overlay.getId());
                                reloadClipInEngine();
                                sEngine.updateProject();
                                updateList();
                                break;
                            }
                        }

                        break;

                    case 3:
                        if( stickerList.size() < 2 ){
                            return;
                        }
                        last = stickerList.size() -1;
                        item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(stickerList.get(last).getAssetID());

                        if( item != null && item.isDelete() ){
                            reloadClipInEngine();
                            sEngine.updateProject();
                            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(stickerList.get(last).getAssetID());
                            updateList();
                        }else{
                            Toast.makeText(getApplicationContext(), "can not uninstall.", Toast.LENGTH_LONG).show();
                        }

                        break;

                    default:
                        break;
                }
            }
        });

        updateBarHandler = new Handler();

        mView = (nexEngineView) findViewById(R.id.engineview_template_man);
        sEngine = getEngine(getApplicationContext());
        sEngine.setView(mView);
        reloadClipInEngine();
        sEngine.updateScreenMode();
        sEngine.updateProject();
    }

    private void updateList(){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTman.loadTemplate();

                templateList.clear();

                templateList.add(new ListViewItem(null, "Normal", nexApplicationConfig.getAspectRatio(), "none"));
                for (nexTemplateManager.Template template : mTman.getTemplates()) {
                    Bitmap icon = template.thumbnail();
                    if( icon == null ){
                        icon = template.icon();
                    }
                    templateList.add(new ListViewItem(icon, template.name("en"), template.aspect(), template.id()));
                }

                templateAdapter.notifyDataSetChanged();

                //List<nexAssetPackageManager.Item> audios = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
                mMedia.loadMedia(nexClip.kCLIP_TYPE_AUDIO);

                audioList.clear();
                audioList.add(new ListViewItem(null, "None", 0, "none"));
                for( nexAssetMediaManager.AssetMedia info : mMedia.getAssetMedias(nexClip.kCLIP_TYPE_AUDIO)){

                    if( !info.filter() ) {
                        Bitmap icon = info.thumbnail();
                        if( icon == null ){
                            icon = info.icon();
                        }

                        audioList.add(new ListViewItem(icon, info.name("en"), 0, info.id()));
                    }
                }

                audioAdapter.notifyDataSetChanged();

                stickerList.clear();
                String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                stickerList.add(new ListViewItem(null, "None", 0, "None"));
                for( String id : stickers  ) {
                    stickerList.add(new ListViewItem(null, id, 0, id));
                    Log.d(TAG, "stickers id=" + id);
                }
                stickerAdapter.notifyDataSetChanged();

                textList.clear();
                textList.add(new ListViewItem(null, "None", nexApplicationConfig.getAspectRatio(), "None"));
                mTitle = nexOverlayManager.getOverlayManager(getApplicationContext(),getApplicationContext());
                mTitle.loadOverlay();

                for(nexOverlayManager.Overlay overlay : mTitle.getOverlays()){
                    Bitmap icon = overlay.getThumbnail();
                    if( icon == null ){
                        icon = overlay.getIcon();
                    }

                    textList.add(new ListViewItem(icon,overlay.getName("en"),overlay.getRatio(),overlay.getId()));
                }

                textAdapter.notifyDataSetChanged();

                fontList.clear();
                fontList.add(new ListViewItem(null, "None", 0, "None"));
                for( nexFont font : nexFont.getPresetList() ){
                    fontList.add(new ListViewItem(font.getSampleImage(getApplicationContext()), font.getId(), 0, font.getId()));
                }
                fontAdapter.notifyDataSetChanged();

                nexColorEffect.updatePluginLut();
            }
        });
    }

    private void reloadClipInEngine(){

        nexProject project = new nexProject();
        project.add(nexClip.dup(mClip));
        project.getClip(0,true).setRotateDegree(project.getClip(0, true).getRotateInMeta());
        project.getClip(0, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        sEngine.setProject(project);
    }

    @Override
    protected void onStart(){
        if( sEngine != null ){
            sEngine.setEventHandler(sEngineListener);
            sEngine.seek(0);

        }
        super.onStart();
    }

    @Override
    protected void onStop() {
        if( sEngine != null ) {
            sEngine.stop();
            mPlaying = false;
        }
        nexFont.clearBuiltinFontsCache();
        super.onStop();
    }
    @Override
    protected void onDestroy() {
        Log.d("nexEditorSimple", "onDestroy()");
        super.onDestroy();
        //3.  BroadcastReceiver was unregisterReceiver
        unregisterReceiver(broadcastReceiver);
    }

    ProgressDialog barProgressDialog;
    Handler updateBarHandler;

    public void showInstallDialog(){
        barProgressDialog = new ProgressDialog(EditorActivity.this);
        barProgressDialog.setTitle("Installing Asset Package ...");
        barProgressDialog.setMessage("ready to install");
        barProgressDialog.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
        barProgressDialog.setProgress(0);
        barProgressDialog.setMax(100);
        barProgressDialog.show();

        mTman.installPackagesAsync(new nexAssetPackageManager.OnInstallPackageListener() {
            private int progress;
            private int count = 0;
            private int max = 0;
            private boolean updateMassage = false;
            @Override
            public void onProgress(int countPackage, int totalPackages, int progressInstalling) {
                Log.d(TAG, "countPackage=" + countPackage + ", totalPackages=" + totalPackages + ", progressInstalling=" + progressInstalling);

                if( count != countPackage ){
                    updateMassage = true;
                }
                count= countPackage;
                max = totalPackages;
                progress = progressInstalling;

                updateBarHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if( updateMassage ){
                            barProgressDialog.setMessage("install("+count+"/"+max+") in progress...");
                        }
                        barProgressDialog.setProgress(progress);
                    }
                });
            }

            @Override
            public void onCompleted(int event) {
                Log.d(TAG, "Asset install complete:" +event);
                if(event == -1){
                    Toast.makeText(getApplicationContext(), "Asset install fail", Toast.LENGTH_SHORT).show();
                }
                updateList();
                barProgressDialog.dismiss();
            }
        });

    }



    class ListViewItem{
        private static final String TAG = "ListViewItem";
        private Bitmap bitmapImage;
        private String assetName;
        private String assetId;
        private float ratio;
        //private float ratio;
        private int assetIdx;

        ListViewItem(Bitmap bitmap, String assetName, float ratio, String assetId){
            this.bitmapImage = bitmap;
            this.assetName = assetName;
            this.ratio = ratio;
            this.assetId = assetId;
            this.assetIdx = 0;
        }

        ListViewItem(Bitmap bitmap, String assetName, float ratio, String assetId, int assetIdx){
            this.bitmapImage = bitmap;
            this.assetName = assetName;
            this.ratio = ratio;
            this.assetId = assetId;
            this.assetIdx = assetIdx;
        }
        void setIcon(Bitmap image){
            bitmapImage = image;
        }

        Bitmap getIcon(){
            return bitmapImage;
        }

        void setAssetName(String id){
            assetName = id;
        }

        String getAssetName(){
            return assetName;
        }


        float getAspectRatio(){
            return ratio;
        }
        void setAssetID(String id){
            assetId = id;
        }

        String getAssetID(){
            return assetId;
        }
        int getAssetIdx(){return assetIdx;}
    }

    public static nexEngine getEngine(Context context){
        if( sEngine == null ){
            sEngine = new nexEngine(context);
        }
        return sEngine;
    }


    @Override
    public void onBackPressed() {
        //super.onBackPressed();
        if (System.currentTimeMillis() > backKeyPressedTime + 2000) {
            backKeyPressedTime = System.currentTimeMillis();
            Toast.makeText(getApplicationContext(),"finish, push on back button.", Toast.LENGTH_SHORT).show();

            return;
        }

        if (System.currentTimeMillis() <= backKeyPressedTime + 2000) {

            if( mTman != null ){
                mTman.cancel();
            }

            if( sEngine != null ) {
                sEngine.stop();
                mPlaying = false;

                sEngine = null;
            }
            nexApplicationConfig.releaseApp();
            finish();
        }
    }
}
