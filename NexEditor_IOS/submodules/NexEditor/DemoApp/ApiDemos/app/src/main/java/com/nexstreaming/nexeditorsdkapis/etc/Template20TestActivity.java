/******************************************************************************
 * File Name        : Template20TestActivity.java
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
import android.graphics.Bitmap;
import android.graphics.Color;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.SystemClock;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.Log;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SeekBar;

//import com.nexstreaming.gfwfacedetection.GFWFaceDetector;
import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetMediaManager;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexExportFormat;
import com.nexstreaming.nexeditorsdk.nexExportFormatBuilder;
import com.nexstreaming.nexeditorsdk.nexExportListener;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayInfoTitle;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayManager;
import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdk.service.nexAssetService;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.BuildConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;
//import com.nexstreaming.nexfacedetection.nexFaceDetector;

import java.io.File;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import static com.nexstreaming.nexeditorsdk.nexApplicationConfig.letterbox_effect_black;
import static com.nexstreaming.nexeditorsdk.nexApplicationConfig.letterbox_effect_blur10;

public class Template20TestActivity extends Activity {

    private static String TAG = "Template20TestActivity";

    private ListView mList;
    private ArrayList<String> mListFilePath;

    private nexEngineView mView;
    private nexEngine mEngin;
    private int mEngineState=nexEngine.nexPlayState.NONE.getValue();
    private nexProject mProject;
    private nexProject mCloneProject;

	
    private SeekBar seekBar_Bri;
    private TextView textView_Bri;

    private SeekBar seekBar_Sat;
    private TextView textView_Sat;

    private SeekBar seekBar_Con;
    private TextView textView_Con;
    
    private SeekBar seekBar_Vignette;
    private TextView textView_Vignette;
    
    private SeekBar seekBar_VignetteRange;
    private TextView textView_InnerVignette;
    
    private SeekBar seekBar_Sharpness;
    private TextView textView_Sharpness;

    private SeekBar seekBar_Projectseek;
    private TextView textView_Projectseek;

    private nexTemplateManager mKmTemplateManager;
    private List<nexTemplateManager.Template> mTemplates = new ArrayList<>();
    private List<Bitmap> mTemplateIcons = new ArrayList<>();
    private nexTemplateManager.Template mCurTemplate = null;
    private AdaptorTemplateListItem mTemplateAdaptor;

    private int AssetStoreRequestCode = 0;

    private nexOverlayManager mKmOverlayManager;
    private List<nexOverlayManager.Overlay> mOverlays = new ArrayList<>();
    private nexOverlayManager.Overlay mCurOverlay;
    private List<nexOverlayManager.nexTitleInfo> mCurOverlayTitleInfos = new ArrayList<>();

    private nexAssetMediaManager mKmBGMManager;
    private List<nexAssetMediaManager.AssetMedia> mBGMs = null;
    private nexAssetMediaManager.AssetMedia mCurBGM = null;

    private List<nexFont> mFonts = null;
    private nexFont mCurFont = null;

    private List<String> mStickers = null;
    private String mCurSticker = null;
    private int stickerXPos=0;
    private int stickerYPos=0;
    private int stickerStartTime=0;
    private int stickerEndTime=6000;

    private Stopwatch watch = new Stopwatch();
    private Stopwatch asset_install_watch = new Stopwatch();

    private long startActivityTime = 0;
    private nexEngine.nexUndetectedFaceCrop faceMode = nexEngine.nexUndetectedFaceCrop.ZOOM;
    private boolean mBackgroundMode = false;
    private boolean mEngineViewAvailable = false;
    private int mPendingCommand = 0;
    private boolean mForceRefreshAssets = false;

    private String storeVendor = "Store";
    private boolean isOverlapedTransition = !UtilityCode.checkLowPerformanceCPU();
    private boolean isExporting = false;

    private boolean mTemplate30Mode = false;
    private AlertDialog mAlertDlg = null;
    private enum DialogType {
        NONE, BGM , FONT, TITLE
    };
    private DialogType dialogType=DialogType.NONE;

//    private nexFaceDetector nexFD = null;
//    private GFWFaceDetector GWFFD = null;
    public class InputFilterMinMax implements InputFilter {

        private int min, max;

        public InputFilterMinMax(int min, int max) {
            this.min = min;
            this.max = max;
        }

        public InputFilterMinMax(String min, String max) {
            this.min = Integer.parseInt(min);
            this.max = Integer.parseInt(max);
        }

        @Override
        public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
            try {
                int input = Integer.parseInt(dest.toString() + source.toString());
                if (isInRange(min, max, input))
                    return null;
            } catch (NumberFormatException nfe) { }
            return "";
        }

        private boolean isInRange(int a, int b, int c) {
            return b > a ? c >= a && c <= b : c >= b && c <= a;
        }
    }

    private nexEngineListener sEngineListener = new nexEngineListener() {
        @Override
        public void onStateChange(int i, int i1) {
            mEngineState = i1;
            Log.d(TAG, "onStateChange() state=" + mEngineState);
        }

        @Override
        public void onTimeChange(int i) {
            seekBar_Projectseek.setProgress(i);
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
//            mPlaying = false;
//            mEngine.seek(0);
        }

        @Override
        public void onPlayFail(int i, int i1) {
//            mPlaying = false;
//            mEngine.seek(0);
        }

        @Override
        public void onPlayStart() {
//            mPlaying =true;

            if(BuildConfig.directTemplate) {
            }
            else {
                if (watch.isRunning()) {
                    watch.stop();
                    Toast.makeText(Template20TestActivity.this, "Template change and play start elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();
                    Log.d(TAG, "Template change and play start elapsed = " + watch.toString());
                } else {
                    long elapsedTime = System.nanoTime() - startActivityTime;
                    Log.d(TAG, "Template20Activity Start and play elapsed = " + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0));
                    Toast.makeText(Template20TestActivity.this, "Template20 Start and play elapsed=" + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0), Toast.LENGTH_LONG).show();
                }
            }
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

    int loopid=0;
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
                    updateAssets(true, idx);
                    if ( mAlertDlg!=null && dialogType != DialogType.NONE ) {
                        if ( dialogType == DialogType.BGM && categoryName.equalsIgnoreCase("Audio") ) {
                            mAlertDlg.cancel();
                            mAlertDlg = null;
                            selBGM();
                        } else if ( dialogType == DialogType.TITLE && categoryName.equalsIgnoreCase("TextEffect") ) {
                            mAlertDlg.cancel();
                            mAlertDlg = null;
                            selTitle();
                        }
                    }
                }else if( nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED.equals(action) ){
                    int idx = intent.getIntExtra("index",-1);
                    Log.d(TAG,"uninstalled Asset, index="+idx);
                    mForceRefreshAssets = true;
                }
            }
        }
    };

    private boolean check_for_wait_for_first_template = false;

    private void waitForFirstTemplate(Object sync_obj) throws InterruptedException{

        synchronized (sync_obj){

            while(!check_for_wait_for_first_template){

                sync_obj.wait();
            }

            check_for_wait_for_first_template = false;
        }
    }

    private void NotifyOfFirstTemplate(Object sync_obj){

        synchronized (sync_obj){

            check_for_wait_for_first_template = true;
            sync_obj.notify();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if( getIntent().hasExtra(Constants.TEMPLATE_VERSION) ) {
            if( getIntent().getIntExtra(Constants.TEMPLATE_VERSION, 2) == 3 )
                TAG = "Template30TestActivity";
        }

        //2. BroadcastReceiver was set intent filter.
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(nexAssetService.ACTION_ASSET_INSTALL_COMPLETED);
        intentFilter.addAction(nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED);
        registerReceiver(broadcastReceiver, intentFilter);

        updateBarHandler = new Handler();


        ApiDemosConfig.getApplicationInstance().getEngine();
    	nexApplicationConfig.setDefaultLetterboxEffect(letterbox_effect_blur10);
        nexApplicationConfig.waitForLoading(this, new Runnable() {
            @Override
            public void run() {
                setContentView(R.layout.activity_template20_test);

                nexAssetStoreAppUtils.setAvailableCategorys(
                        nexAssetStoreAppUtils.AssetStoreMimeType_Template|
                        nexAssetStoreAppUtils.AssetStoreMimeType_Audio|
                        nexAssetStoreAppUtils.AssetStoreMimeType_TitleTemplate|
                        nexAssetStoreAppUtils.AssetStoreMimeType_BeatTemplate
        );

                // set screen timeout to never
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

                Button title = (Button) findViewById(R.id.btnTitle);
                title.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        selTitle();
                    }
                });

                Button font = (Button) findViewById(R.id.btnFont);
                font.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        selFont();
                    }
                });

                Button bgm = (Button) findViewById(R.id.btnBGM);
                bgm.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        selBGM();
                    }
                });

                mStickers = new ArrayList<String>();
                Button sticker = (Button) findViewById(R.id.btnSticker);
                sticker.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        selSticker();
                    }
                });

                Button export = (Button) findViewById(R.id.btnExport);
                export.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( mCurTemplate != null )
                        {
                            if( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).validateAssetPackage(mCurTemplate.packageInfo().assetIdx()) == false )
                            {
                                Toast.makeText(getApplicationContext(), "Invalid template!", Toast.LENGTH_SHORT).show();
                                return;
                            }
                        }
                        if (mEngin.getProject() == null) {
                            Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                            return;
                        }
                        mEngin.stop(new nexEngine.OnCompletionListener() {
                            @Override
                            public void onComplete(int resultCode) {
                                mEngineState = nexEngine.nexPlayState.IDLE.getValue();
                                Log.d(TAG, "nexEngine.stop() done! state=" + mEngineState);
                            }
                        });
                        Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                        intent.putExtra("FaceMode", faceMode.getValue());
                        startActivity(intent);
                    }
                });


                Button setting = (Button)findViewById(R.id.btnSet);
                setting.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        showSettingDlg();
                    }
                });

                Button export2 = (Button) findViewById(R.id.btnExport2);
                export2.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        if( mCurTemplate != null )
                        {
                            if( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).validateAssetPackage(mCurTemplate.packageInfo().assetIdx()) == false )
                            {
                                Toast.makeText(getApplicationContext(), "Invalid template!", Toast.LENGTH_SHORT).show();
                                return;
                            }
                        }
                        if (mEngin.getProject() == null) {
                            Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                            return;
                        }

                        mEngin.stop(new nexEngine.OnCompletionListener() {
                            @Override
                            public void onComplete(int resultCode) {
                            }
                        });

                        if( isExporting ) return;
                        isExporting = true;

                        final ProgressDialog progressExport = new ProgressDialog(Template20TestActivity.this);
                        progressExport.setTitle("Export ...");
                        progressExport.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
                        progressExport.setProgress(0);
                        progressExport.setMax(100);
                        progressExport.setCanceledOnTouchOutside(false);
                        progressExport.setCancelable(false);
                        progressExport.show();

                        int width = 1280;
                        int height = 720;

                        final File f = getExportFile(width , height, "mp4");

                        nexExportFormat format = nexExportFormatBuilder.Builder()
                                .setType("mp4")
                                .setVideoCodec(nexEngine.ExportCodec_AVC)
                                .setVideoBitrate(6*1024*1024)
                                .setVideoProfile(nexEngine.ExportProfile_AVCBaseline)
                                .setVideoLevel(nexEngine.ExportAVCLevel31)
                                .setVideoRotate(0)
                                .setVideoFPS(30*100)
                                .setWidth(width)
                                .setHeight(height)
                                .setAudioSampleRate(44100)
                                .setMaxFileSize(Long.MAX_VALUE)
                                .setPath(f.getAbsolutePath())
                                .build();

                        mEngin.export(format, new nexExportListener() {
                            @Override
                            public void onExportFail(nexEngine.nexErrorCode err) {
                                Log.d(TAG, "onExportFail: " + err.toString());

                                Toast.makeText(Template20TestActivity.this, "onExportFail: " + err.toString(), Toast.LENGTH_LONG).show();
                                progressExport.dismiss();
                                isExporting = false;
                            }

                            @Override
                            public void onExportProgress(int percent) {
                                Log.d(TAG, "onExportProgress: " + percent);
                                progressExport.setProgress(percent);
                            }

                            @Override
                            public void onExportDone(Bitmap bitmap) {
                                Log.d(TAG, "onExportDone");
                                progressExport.dismiss();
                                Toast.makeText(Template20TestActivity.this, "onExportDone (" + f.getAbsolutePath() + ")", Toast.LENGTH_LONG).show();
                                MediaScannerConnection.scanFile(getApplicationContext(), new String[]{f.getAbsolutePath()}, null, null);
                                isExporting = false;
                            }
                        });
                    }
                });

                seekBar_Bri = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_bri);
                textView_Bri = (TextView) findViewById(R.id.textview_clip_color_adjust_bri);
                seekBar_Bri.setMax(510);
                seekBar_Bri.setProgress(255);

                seekBar_Bri.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress-255;
                        textView_Bri.setText(""+ value);
                        mEngin.setBrightness(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_brightness, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {

                        textView_Bri.setText(""+ value);
                        mEngin.setBrightness(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_brightness, value);
                    }
                });

                seekBar_Sat = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_sat);
                textView_Sat = (TextView) findViewById(R.id.textview_clip_color_adjust_sat);
                seekBar_Sat.setMax(510);
                seekBar_Sat.setProgress(255);
                seekBar_Sat.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress-255;
                        textView_Sat.setText(""+ value);
                        mEngin.setSaturation(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_saturation, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {

                        textView_Sat.setText(""+ value);
                        mEngin.setSaturation(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_saturation, value);
                    }
                });


                seekBar_Con = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_con);
                textView_Con = (TextView) findViewById(R.id.textview_clip_color_adjust_con);
                seekBar_Con.setMax(510);
                seekBar_Con.setProgress(255);
                seekBar_Con.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress-255;
                        textView_Con.setText(""+ value);
                        mEngin.setContrast(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_contrast, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        textView_Con.setText(""+ value);
                        mEngin.setContrast(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_contrast, value);
                    }
                });

                seekBar_Vignette = (SeekBar) findViewById(R.id.seekbar_clip_vignette);
                textView_Vignette = (TextView) findViewById(R.id.textview_clip_vignette);
                seekBar_Vignette.setMax(100);
                seekBar_Vignette.setProgress(0);
                seekBar_Vignette.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress;
                        textView_Vignette.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setVignette(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_vignette, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        textView_Vignette.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setVignette(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_vignette, value);
                    }
                });
                
                seekBar_VignetteRange = (SeekBar) findViewById(R.id.seekbar_clip_vignette_range);
                textView_InnerVignette = (TextView) findViewById(R.id.textview_clip_vignette_range);
                seekBar_VignetteRange.setMax(100);
                seekBar_VignetteRange.setProgress(0);
                seekBar_VignetteRange.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress;
                        textView_InnerVignette.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setVignetteRange(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_vignetteRange, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        textView_InnerVignette.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setVignetteRange(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_vignetteRange, value);
                    }
                });

                seekBar_Sharpness = (SeekBar) findViewById(R.id.seekbar_clip_sharpness);
                textView_Sharpness = (TextView) findViewById(R.id.textview_clip_sharpness);
                seekBar_Sharpness.setMax(500);
                seekBar_Sharpness.setProgress(0);
                seekBar_Sharpness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress;
                        textView_Sharpness.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setSharpness(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_sharpness, value);
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {

                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        textView_Sharpness.setText(String.format("%1.2f", value * 0.01f));
                        mEngin.setSharpness(value);
                        mEngin.fastPreview(nexEngine.FastPreviewOption.adj_sharpness, value);
                    }
                });
                seekBar_Projectseek = (SeekBar) findViewById(R.id.seekbar_project_seek);
                textView_Projectseek = (TextView) findViewById(R.id.textview_project_seek);
                seekBar_Projectseek.setProgress(0);
                seekBar_Projectseek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    int value = 0;

                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        value = progress;
                        textView_Projectseek.setText(String.format("%d", value));
                        if(fromUser){
                            mEngin.seek(value);
                        }
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                        mEngin.stop();
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });
                Intent intent = getIntent();
                if( intent.hasExtra(Constants.TEMPLATE_VERSION) ) {
                    if( intent.getIntExtra(Constants.TEMPLATE_VERSION, 2) == 3 )
                        mTemplate30Mode = true;
                }

                mKmOverlayManager = nexOverlayManager.getOverlayManager(getApplicationContext(), getApplicationContext());

                if(BuildConfig.directTemplate) {
                    LinearLayout panel1 = (LinearLayout) findViewById(R.id.panel1);
                    panel1.setVisibility(View.GONE);

                    LinearLayout panel2 = (LinearLayout) findViewById(R.id.panel2);
                    panel2.setVisibility(View.VISIBLE);
                }

                mKmOverlayManager = nexOverlayManager.getOverlayManager(getApplicationContext(), getApplicationContext());

                mKmTemplateManager = nexTemplateManager.getTemplateManager(getApplicationContext(), getApplicationContext());

                mKmBGMManager = nexAssetMediaManager.getAudioManager(getApplicationContext());

                mList = (ListView) findViewById(R.id.listview_template_test);
                mTemplateAdaptor = new AdaptorTemplateListItem();
                mList.setAdapter(mTemplateAdaptor);

                Object sync = new Object();
                refreshAssets(sync, 5); //refreshAssets(true);

                mListFilePath = intent.getStringArrayListExtra("filelist");
                mProject = new nexProject();

                startActivityTime = intent.getLongExtra("startActivityTime", 0);
                long elapsedTime = System.nanoTime()-startActivityTime;

                Log.d(TAG, "Template20/30Activity onCreate elapsed = " + String.format("%1$,.3f", (double) elapsedTime / 1000000.0));

                for (int i = 0; i < mListFilePath.size(); i++) {
                    Log.d(TAG, "Add content to project = " + mListFilePath.get(i));

                    nexClip clip = nexClip.getSupportedClip(mListFilePath.get(i));
                    if (clip != null) {
                        mProject.add(clip);
        //                        if( i == 1)
        //                            clip.getVideoClipEdit().setSpeedControl(25);
                        int rotate = clip.getRotateInMeta();

                        Log.i(TAG, "SDK ROTATION=" + rotate);

                        if( clip.getClipType()== nexClip.kCLIP_TYPE_VIDEO ) {
                            try {
                                MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                                retriever.setDataSource(mListFilePath.get(i));
                                String rotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);

                                if (rotation != null) {
                                    rotate = Integer.parseInt(rotation);
                                    Log.i(TAG, "MediaMetadataRetriever ROTATION=" + rotate);
                                }

                                Log.i(TAG, "rotate=" + rotate);
                                clip.setRotateDegree(360 - rotate);

                            } catch (RuntimeException e) {
                                Log.e(TAG, e.toString());
                            }
                        }
                        clip.getCrop().resetStartEndPosition();

                    } else {
                        String skip = "Add content to project failed = " + mListFilePath.get(i);

                        Toast.makeText(getApplicationContext(), skip, Toast.LENGTH_LONG).show();
                        Log.d(TAG, "Add content to project failed = " + mListFilePath.get(i));
                    }
                }

                mView = (nexEngineView) findViewById(R.id.engineview_template_test);
                mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
                mEngin.setView(mView);
                mEngin.set360VideoForceNormalView();
                if(faceMode != nexEngine.nexUndetectedFaceCrop.NONE) {
                    setFaceModule();
                }
                mEngin.setThumbnailRoutine(2);
                mView.setListener(new nexEngineView.NexViewListener() {
                    @Override
                    public void onEngineViewAvailable(int i, int i1) {
                        mEngineViewAvailable = true;
                        Log.d(TAG, "onEngineViewAvailable  mPendingCommand= " + mPendingCommand);
                        if( mPendingCommand == 1 ){
                            int ret = mEngin.faceDetect(true, 1, faceMode);
                            if(BuildConfig.directTemplate) {
                            }
                            else {
                                if (ret == 0) {
                                    Toast.makeText(Template20TestActivity.this, "Disable FaceDetector", Toast.LENGTH_SHORT).show();
                                } else {
                                    Toast.makeText(Template20TestActivity.this, "Enable FaceDetector", Toast.LENGTH_SHORT).show();
                                }
                            }
                            if( mEngin.play(true) == false )
                                Toast.makeText(Template20TestActivity.this, "Play error with crashed template", Toast.LENGTH_SHORT).show();
                        }else if( mPendingCommand == 2 ){
                            if(mEngin.getDuration()<=mEngin.getCurrentPlayTimeTime()){
                                mEngin.seek(mEngin.getCurrentPlayTimeTime() - (mEngin.getCurrentPlayTimeTime() - mEngin.getDuration()));
                            }else mEngin.seek(mEngin.getCurrentPlayTimeTime() + 1);
                        }
                        mPendingCommand = 0;
                    }

                    @Override
                    public void onEngineViewSizeChanged(int i, int i1) {

                    }

                    @Override
                    public void onEngineViewDestroyed() {
                        mEngineViewAvailable = false;
                    }
                });

                mView.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        switch (event.getAction()) {
                            case MotionEvent.ACTION_DOWN:
                                nexOverlayInfoTitle title_info = null;
                                if( mCurOverlay != null && mKmOverlayManager != null )
                                {
                                    title_info = mKmOverlayManager.getOverlayInfoTitle(mEngin.getCurrentPlayTimeTime(), event.getX()/mView.getMeasuredWidth(), event.getY()/mView.getMeasuredHeight());
                                    if( title_info != null ) {
                                        inputTitle(title_info);
                                    }
                                }
                                Log.i(TAG, "+ title info :" + title_info + " event: " + event );
                                break;
                        }
                        return false;
                    }
                });

                String asset_install_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + File.separator + ".kmsdk_asset_install";
                File dir = new File(asset_install_path);
                if( dir.isDirectory() == false )
                    dir.mkdir();

                nexApplicationConfig.setAssetInstallRootPath(dir.getAbsolutePath());
                nexApplicationConfig.setAssetStoreRootPath(dir.getAbsolutePath());

                for(int i = 0; i < mProject.getTotalClipCount(true); i++ ) {
                    nexClip clip = mProject.getClip(i, true);
                    if( clip != null && clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO )
                    {
                        final String path = clip.getPath();
                        new Thread(new Runnable() {
                            public void run() {
                                Log.i(TAG, "+ checkPFrameDirectExportSync :" + path );
                                if( mEngin.checkPFrameDirectExportSync(path) )
                                {
                                    Log.i(TAG, "- checkPFrameDirectExportSync true");
                                }
                                else
                                {
                                    Log.i(TAG, "- checkPFrameDirectExportSync false");
                                }
                            }
                        }).start();


                    }
                }

                try {
                    waitForFirstTemplate(sync);
                }catch(Exception e){

                    e.printStackTrace();
                }

                if( mTemplates.size() > 0 )
                {
                    watch.reset();
                    mCurTemplate = mTemplates.get(0);

                    String errorMsg = setEffects2Project();
                    if (errorMsg != null) {
                        Log.d(TAG, errorMsg);

                        Toast.makeText(Template20TestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();

                        // finish();
                        return;
                    }
                    mEngin.setProject(mCloneProject);

                    // mEngin.updateProject();
                    // mEngin.resume();
                }
                else {
                    mCloneProject = nexProject.clone(mProject);
                    mEngin.setProject(mCloneProject);

                    int count = mCloneProject.getTotalClipCount(true);
                    for(int i=0; i<count; i++){
                        nexClip clip = mCloneProject.getClip(i, true);
                            clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                    }

                    mEngin.updateProject();
                }
                seekBar_Projectseek.setMax(mCloneProject.getTotalTime());
            }
        });
    }
    @Override
    protected void onPause() {
        if( mEngin != null)
            mEngin.stop();
        mBackgroundMode = true;
        super.onPause();
    }

    @Override
    protected void onResume() {
        if ( mForceRefreshAssets ) {
            refreshAssets(true);
            mForceRefreshAssets = false;
        }

        if( !mBackgroundMode ) {
            if (mEngineViewAvailable) {
                if(faceMode != nexEngine.nexUndetectedFaceCrop.NONE) {
                    setFaceModule();
                }
                boolean enableFaceDetect = true;
                if( mCurTemplate != null ){
                    if( mCurTemplate.category() == nexAssetPackageManager.Category.beattemplate
                            || mCurTemplate.type() ==  nexAssetPackageManager.ItemMethodType.ItemBeat){
                        enableFaceDetect = false;
                    }
                }

                if( !enableFaceDetect ){
                    mEngin.faceDetect(true, 1, nexEngine.nexUndetectedFaceCrop.NONE);
                }else {


                    int ret = mEngin.faceDetect(true, 1, faceMode);

                    mEngin.faceDetect(true, 1, nexEngine.nexUndetectedFaceCrop.ZOOM);

                    if (BuildConfig.directTemplate) {
                    } else {
                        if (ret == 0) {
                            Toast.makeText(Template20TestActivity.this, "Disable FaceDetector", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(Template20TestActivity.this, "Enable FaceDetector", Toast.LENGTH_SHORT).show();
                        }
                    }
                }
                if (mEngin.play(true) == false)
                    Toast.makeText(Template20TestActivity.this, "Play error with crashed template", Toast.LENGTH_SHORT).show();
            } else {
                mPendingCommand = 1;
            }
        }else{
            if (mEngineViewAvailable) {
                if(mEngin.getDuration()<=mEngin.getCurrentPlayTimeTime()){
                    mEngin.seek(mEngin.getCurrentPlayTimeTime() - (mEngin.getCurrentPlayTimeTime() - mEngin.getDuration()));
                }else mEngin.seek(mEngin.getCurrentPlayTimeTime() + 1);
            } else {
                mPendingCommand = 2;
            }
        }

        mBackgroundMode = false;
        // long elapsedTime = System.nanoTime()-startActivityTime;

        // Log.d(TAG, "Template20Activity onResume elapsed = " + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0));

        // Toast.makeText(Template20TestActivity.this, "Template apply and play elapsed=" + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0), Toast.LENGTH_LONG).show();
        super.onResume();
    }

    String setEffects2Project() {

        if( mProject.getTotalTime() <= 0 )
        {
            return "Project is empty";
        }

        if( mCurTemplate == null )
        {
            return "Template did not selected";
        }

        if ( checkAsset(mCurTemplate.id()) == false ) {
            return "This is expired asset!";
        }

        if(mCloneProject != null){
            mCloneProject.allClear(true);
        }

        mCloneProject = nexProject.clone(mProject);

        try {
            if( mKmTemplateManager.applyTemplateToProjectById(mCloneProject, mCurTemplate.id(), isOverlapedTransition, faceMode.getValue()) == false )
            {

    //            if( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).validateAssetPackage(mCurTemplate.packageInfo().assetIdx()) == false )
    //            {
    //                mKmTemplateManager.uninstallPackageById(mCurTemplate.id());
    //                mCurTemplate = null;
    //                refreshAssets(true);
    //            }
                return "Fail to apply template on project";
            }
        } catch (ExpiredTimeException e) {
            return "This is expired asset!";
        }

        if( mCurOverlay != null )
        {
            if( mCurFont != null )
            {
                for(nexOverlayManager.nexTitleInfo info : mCurOverlayTitleInfos )
                {
                    info.setFontID(mCurFont.getId());
                }
            }

            if( mCurOverlay.getVersion().startsWith("2")) {
                mKmOverlayManager.applyOverlayToProjectById(mCloneProject , mCurOverlay);
            }
            else {
                mKmOverlayManager.applyOverlayToProjectById(mCloneProject, mCurOverlay.getId(), mCurOverlayTitleInfos);
            }
        }

        boolean enableUserBgm = true;
        if( mCurTemplate != null ){
            if( mCurTemplate.category() == nexAssetPackageManager.Category.beattemplate
                    || mCurTemplate.type() ==  nexAssetPackageManager.ItemMethodType.ItemBeat){
                enableUserBgm = false;
            }
        }

        if ( mCurSticker != null ) {
            nexOverlayImage overlayImage = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getOverlayImage(mCurSticker);
            nexOverlayItem overlayItem = new nexOverlayItem(overlayImage, nexOverlayItem.AnchorPoint_MiddleMiddle, false, stickerXPos, stickerYPos, stickerStartTime, stickerEndTime);
            mCloneProject.addOverlay(overlayItem);
        }

        if( enableUserBgm ) {
            if( mCurBGM != null )
            {
                mCloneProject.setBackgroundMusicPath(mCurBGM.id());
            }
        }
        return null;
    }

    private void showWorryingDig( int interval ){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String message = "This File found I-frame interval average is "+interval+"(ms).\n"+"Did you want play on template?";
        builder.setMessage(message )
                .setNeutralButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                    }
                }).setNegativeButton("NOK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        builder.create().show();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");

        // mEngin.stop();
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        mEngin.setEventHandler(sEngineListener);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        mEngin.setFaceModule(null);
        mEngin.stop();
        mCloneProject = null;
        //3.  BroadcastReceiver was unregisterReceiver
        unregisterReceiver(broadcastReceiver);

        nexApplicationConfig.setDefaultLetterboxEffect(letterbox_effect_black);// Set default value.

        ApiDemosConfig.getApplicationInstance().releaseEngine();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == AssetStoreRequestCode && resultCode == Activity.RESULT_OK) {
            Log.d(TAG,"onActivityResult from Asset Store");
            int count = mKmTemplateManager.findNewPackages();
            if(  count > 0 ){
                showInstallDialog();
            }
        }
    }

    public void updateOverlays() {
        mOverlays.clear();
        mCurOverlay = null;
        mCurOverlayTitleInfos.clear();

        for(nexOverlayManager.Overlay o : mKmOverlayManager.getOverlays(false) ) {
            if( o.getVersion().startsWith("2") ) {
                mOverlays.add(o);
            }
            else {
                if( o.getRatio() == nexApplicationConfig.getAspectRatio() )
                    mOverlays.add(o);
            }
        }
    }

    public static InputFilter getEditTextFilterEmoji()
    {
        return new InputFilter()
        {
            @Override
            public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend)
            {

                for (int i = start; i < end; i++) {
                    int type = Character.getType(source.charAt(i));
                    if (type == Character.SURROGATE || type == Character.OTHER_SYMBOL) {
                        return source.subSequence(start, i);
                    }
                }
                return null;
            }
        };
    }


    public void inputTitle(nexOverlayInfoTitle info)
    {
        if( info == null ) return;
        if( mAlertDlg != null ) return;

        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_input_title, null);

        final EditText editText = (EditText)dialogView.findViewById(R.id.edittext_title);
        editText.setTag(info);

        InputFilter[] filterArray = new InputFilter[2];
        filterArray[0] = new InputFilter.LengthFilter(info.getTitleMaxLength());
        filterArray[1] = getEditTextFilterEmoji();
        editText.setFilters(filterArray);

        editText.setMaxLines(info.getTitleMaxLines());
        editText.setText(info.getTitle(Locale.getDefault().getLanguage()));


        editText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                Log.d(TAG, "beforeTextChanged");

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Log.d(TAG, "onTextChanged");

            }

            @Override
            public void afterTextChanged(Editable s) {
                Log.d(TAG, "afterTextChanged: " + s.toString());
                nexOverlayInfoTitle titleInfo = (nexOverlayInfoTitle)editText.getTag();
                if( titleInfo != null ) {
                    if( s.length() <= 0 )
                        titleInfo.setTitle(null);
                    else
                        titleInfo.setTitle(s.toString());
                    mEngin.fastPreview(nexEngine.FastPreviewOption.cts, mEngin.getCurrentPlayTimeTime());
                }
            }
        });

        dialogBuilder.setView(dialogView).
                setTitle("Input title").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mAlertDlg = null;
                        dialogType = DialogType.NONE;
                    }
                }).
                setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mAlertDlg = null;
                        dialogType = DialogType.NONE;
                    }
                }).
                setCancelable(false);
        mAlertDlg = dialogBuilder.create();
        dialogType = DialogType.TITLE;
        mAlertDlg.show();
    }

    public void selTitle()
    {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_sel_overlay_title, null);

        final LinearLayout ll = (LinearLayout)dialogView.findViewById(R.id.layout_title_info);

        ListView overlayList = (ListView)dialogView.findViewById(R.id.listview_title);
        final AdaptorOverlayListItem adt_overlay = new AdaptorOverlayListItem();
        overlayList.setAdapter(adt_overlay);
        overlayList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mOverlays.size() <= position) {
                    return;
                }

                if (mCurOverlay != mOverlays.get(position)) {

                    mCurOverlay = mOverlays.get(position);

                    if( mCurOverlay.getVersion().startsWith("1") ) {
                        mCurOverlayTitleInfos.clear();
                        mKmOverlayManager.parseOverlay(mCurOverlay.getId(), mCurOverlayTitleInfos);
                    }

                    updateInputField(ll);

                    adt_overlay.notifyDataSetChanged();
                }
            }
        });

        overlayList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position, long id) {
                if (adt_overlay.getCount() <= position)
                    return false;

                final nexOverlayManager.Overlay item = (nexOverlayManager.Overlay) adt_overlay.getItem(position);
                if(item.isDelete() == false  ) {
                    return false;
                }

                mEngin.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(Template20TestActivity.this);
                        builder.setMessage("Do you want to delete this item?")
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        String id = item.getId();

                                        mKmOverlayManager.uninstallPackageById(id);
                                        mKmOverlayManager.loadOverlay();

                                        updateOverlays();

                                        adt_overlay.notifyDataSetChanged();
                                    }
                                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                            }
                        });
                        builder.create().show();
                    }
                });

                return false;
            }
        });
        
        if( mCurOverlay != null )
        {
            updateInputField(ll);
        }

        dialogBuilder.setView(dialogView).
                setTitle("Select Title").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
						mAlertDlg = null;
						dialogType = DialogType.NONE;
                    }
                }).
                setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mCurOverlay = null;
						mAlertDlg = null;
						dialogType = DialogType.NONE;
                    }
                });
        mAlertDlg = dialogBuilder.create();
        dialogType = DialogType.TITLE;
        mAlertDlg.show();
    }

    public class AdaptorOverlayListItem extends BaseAdapter {

        @Override
        public int getCount() {
            return mOverlays == null ? 0 : mOverlays.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mOverlays == null ? null : mOverlays.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            nexOverlayManager.Overlay overlay = (nexOverlayManager.Overlay)getItem(arg0);

            if( overlay == null )
                return null;


            LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
            View rowView = inflater.inflate(R.layout.listitem_assets, null, true);

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);
            TextView status = (TextView)rowView.findViewById(R.id.status);

            image.setImageBitmap(overlay.getIcon());
            name.setText(overlay.getName("en"));

            if( overlay.equals(mCurOverlay) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0x00000000);

            if( overlay.isDelete() )
            {
                status.setText("Installed");
            }
            else {
                status.setText("Bundle");
            }

            return rowView;
        }
    }

    void updateInputField(LinearLayout ll)
    {
        if( ll == null ) return;

        ll.removeAllViews();

        LayoutInflater inflater = LayoutInflater.from(this);

        for(final nexOverlayManager.nexTitleInfo titleInfo : mCurOverlayTitleInfos ) {

            LinearLayout childLL = (LinearLayout) inflater.inflate(R.layout.linear_overlay_title_info, null, false);

            TextView title_desc = (TextView) childLL.findViewById(R.id.title_desc);

            final EditText title = (EditText) childLL.findViewById(R.id.title);

            title_desc.setText(titleInfo.getTextDesc());
            title.setText(titleInfo.getText());
            title.setTag(titleInfo.getId());

            InputFilter[] inputFilters = new InputFilter[] {
                    new InputFilter()
                    {
                        @Override
                        public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend)
                        {
                            int overlayWidth = titleInfo.getOverlayWidth();
                            int currentWidth = titleInfo.getTextWidth(dest.subSequence(0, dstart).toString() + source.toString());

                            if( overlayWidth > currentWidth ) {
                                return null;
                            }

                            int keep = dend - dstart;

                            return source.subSequence(0, keep);
                        }
                    }
            };

            // title.setFilters(new InputFilter[] { new InputFilter.LengthFilter(titleInfo.getTextMaxLen())});
            title.setFilters(inputFilters);
            title.setSingleLine();

            title.addTextChangedListener(new TextWatcher() {
                @Override
                public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                }

                @Override
                public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
                    for(nexOverlayManager.nexTitleInfo ti : mCurOverlayTitleInfos ) {
                        if( title.getTag().equals(ti.getId()) )
                        {
                            ti.setText(charSequence.toString());
                            break;
                        }
                    }
                }

                @Override
                public void afterTextChanged(Editable editable) {

                }
            });

            ll.addView(childLL);
        }
    }

    public void selFont()
    {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_sel_font, null);

        ListView fontList = (ListView)dialogView.findViewById(R.id.listview_font);
        final AdaptorFontListItem adt_font = new AdaptorFontListItem();
        fontList.setAdapter(adt_font);
        fontList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mFonts.size() <= position) {
                    return;
                }

                if (mCurFont != mFonts.get(position)) {

                    mCurFont = mFonts.get(position);
                    adt_font.notifyDataSetChanged();
                }
            }
        });

        fontList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position, long id) {
                if (adt_font.getCount() <= position)
                    return false;

                final nexFont item = (nexFont) adt_font.getItem(position);
                if(item == null || item.isSystemFont() || item.isBuiltinFont() ) {
                    return false;
                }

                mEngin.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(Template20TestActivity.this);
                        builder.setMessage("Do you want to delete this item?")
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        String id = item.getId();
                                        nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(id);
                                        mFonts = nexFont.getPresetList();
                                        mCurFont = null;
                                        adt_font.notifyDataSetChanged();
                                    }
                                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                            }
                        });
                        builder.create().show();
                    }
                });

                return false;
            }
        });

        dialogBuilder.setView(dialogView).
                setTitle("Select Font").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
						mAlertDlg = null;
						dialogType = DialogType.NONE;
                    }
                }).
                setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mCurFont = null;
						mAlertDlg = null;
						dialogType = DialogType.NONE;
                    }
                });
        mAlertDlg = dialogBuilder.create();
        dialogType = DialogType.FONT;
        mAlertDlg.show();
    }

    public class AdaptorFontListItem extends BaseAdapter {

        @Override
        public int getCount() {
            return mFonts == null ? 0 : mFonts.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mFonts == null ? null : mFonts.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            nexFont font = (nexFont)getItem(arg0);

            if( font == null )
                return null;

            LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
            View rowView = inflater.inflate(R.layout.listitem_font, null, true);

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView status = (TextView)rowView.findViewById(R.id.title_status);

            image.setImageBitmap(font.getSampleImage(getApplicationContext()));

            if( font.equals(mCurFont) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0xFF949494);

            if( font.isSystemFont() )
            {
                status.setText("System");
            }
            else if( font.isBuiltinFont() )
            {
                status.setText("Bundle");
            }
            else {
                status.setText("Installed");
            }

            return rowView;
        }
    }

    public void selBGM()
    {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_sel_bgm, null);

        ListView bgmList = (ListView)dialogView.findViewById(R.id.listview_bgm);
        final AdaptorBGMListItem adt_bgm = new AdaptorBGMListItem();
        bgmList.setAdapter(adt_bgm);
        bgmList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mBGMs.size() <= position) {
                    return;
                }

                if (mCurBGM != mBGMs.get(position)) {

                    mCurBGM = mBGMs.get(position);
                    adt_bgm.notifyDataSetChanged();
                }
            }
        });

        bgmList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position, long id) {
                if (adt_bgm.getCount() <= position)
                    return false;

                final nexAssetPackageManager.Item item = (nexAssetPackageManager.Item) adt_bgm.getItem(position);
                if (item != null && item.isDelete()) {

                    mEngin.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            AlertDialog.Builder builder = new AlertDialog.Builder(Template20TestActivity.this);
                            builder.setMessage("Do you want to delete this item?")
                                    .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog, int which) {
                                            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(item.id());
                                            mKmBGMManager.loadMedia(nexClip.kCLIP_TYPE_AUDIO);
                                            mBGMs = mKmBGMManager.getAssetMedias();
                                            //mBGMs = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
                                            mCurBGM = null;
                                            adt_bgm.notifyDataSetChanged();
                                        }
                                    }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                }
                            });
                            builder.create().show();
                        }
                    });
                }

                return false;
            }
        });

        dialogBuilder.setView(dialogView).
                setTitle("Select BGM").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
						mAlertDlg = null;
                        dialogType = DialogType.NONE;
                    }
                }).
                setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mCurBGM = null;
						mAlertDlg = null;
                        dialogType = DialogType.NONE;
                    }
                });
        mAlertDlg = dialogBuilder.create();
        dialogType = DialogType.BGM;
        mAlertDlg.show();
    }

    public class AdaptorBGMListItem extends BaseAdapter {

        @Override
        public int getCount() {
            return mBGMs == null ? 0 : mBGMs.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mBGMs == null ? null : mBGMs.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            nexAssetPackageManager.Item bgm = (nexAssetPackageManager.Item)getItem(arg0);

            if( bgm == null )
                return null;

            LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
            View rowView = inflater.inflate(R.layout.listitem_assets, null, true);

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);
            TextView status = (TextView)rowView.findViewById(R.id.status);
            Bitmap thumb =  bgm.thumbnail();
            if( thumb == null ){
                thumb = bgm.icon();
            }
            image.setImageBitmap(thumb);
            name.setText(bgm.name("en"));

            if( bgm.equals(mCurBGM) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0x00000000);

            if( bgm.isDelete() )
            {
                status.setText("Installed");
            }
            else {
                status.setText("Bundle");
            }

            return rowView;
        }
    }

    public void selSticker()
    {
        mEngin.stop();
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView= inflater.inflate(R.layout.dialog_sel_sticker, null);
        ListView stickerList = (ListView)dialogView.findViewById(R.id.listview_sticker);

        final EditText posX = (EditText) dialogView.findViewById(R.id.posX2);
        final EditText posY = (EditText) dialogView.findViewById(R.id.posY2);
        final EditText startTime = (EditText) dialogView.findViewById(R.id.startTime2);
        final EditText endTime = (EditText) dialogView.findViewById(R.id.endTime2);

        posX.setText(""+stickerXPos);
        posY.setText(""+stickerYPos);
        startTime.setText(""+stickerStartTime);
        endTime.setText(""+stickerEndTime);

        if ( mCurSticker == null ) {
            posX.setEnabled(false);
            posY.setEnabled(false);
            startTime.setEnabled(false);
            endTime.setEnabled(false);
        }

        final AdaptorStickerListItem adt_sticker = new AdaptorStickerListItem();
        stickerList.setAdapter(adt_sticker);
        stickerList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (mStickers.size() <= position) {
                    return;
                }

                if (mCurSticker != mStickers.get(position)) {
                    posX.setEnabled(true);
                    posY.setEnabled(true);
                    startTime.setEnabled(true);
                    endTime.setEnabled(true);
                    mCurSticker = mStickers.get(position);
                    adt_sticker.notifyDataSetChanged();
                }
            }
        });

        stickerList.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position, long id) {
                if (adt_sticker.getCount() <= position)
                    return false;

                final String item = (String) adt_sticker.getItem(position);
                if(item == null /* || item.isSystemFont() || item.isBuiltinFont() */) {
                    return false;
                }

                mEngin.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(Template20TestActivity.this);
                        builder.setMessage("Do you want to delete this item?")
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageById(item);
                                        String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                                        for (String id : stickers) {
                                            mStickers.add(id);
                                        }
                                        mCurSticker = null;
                                        adt_sticker.notifyDataSetChanged();
                                    }
                                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                            }
                        });
                        builder.create().show();
                    }
                });

                return false;
            }
        });

        dialogBuilder.setView(dialogView).
                setTitle("Select Sticker").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mAlertDlg = null;
                        dialogType = DialogType.NONE;

                        if ( mCurSticker != null ) {
                            stickerXPos = Integer.parseInt(posX.getText().toString());
                            stickerYPos = Integer.parseInt(posY.getText().toString());
                            stickerStartTime = Integer.parseInt(startTime.getText().toString());
                            stickerEndTime = Integer.parseInt(endTime.getText().toString());
                            Log.d(TAG, String.format("Sticker Info : X=%d, Y=%d, Start=%d, End=%d", stickerXPos, stickerYPos, stickerStartTime, stickerEndTime));

                            nexOverlayImage overlayImage = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getOverlayImage(mCurSticker);
                            nexOverlayItem overlayItem = new nexOverlayItem(overlayImage, nexOverlayItem.AnchorPoint_MiddleMiddle, false, stickerXPos, stickerYPos, stickerStartTime, stickerEndTime);
                            mEngin.getProject().clearOverlay();
                            mEngin.getProject().addOverlay(overlayItem);
                            mEngin.seek(0);
                            mEngin.play();
                        }
                    }
                }).
                setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        mCurSticker = null;
                        mAlertDlg = null;
                        dialogType = DialogType.NONE;
                    }
                });
        mAlertDlg = dialogBuilder.create();
        dialogType = DialogType.FONT;
        mAlertDlg.show();
    }

    public class AdaptorStickerListItem extends BaseAdapter {
        private LruCache bitmapCache= new LruCache<String, Bitmap>(1024*1024);

        @Override
        public int getCount() {
            return mStickers == null ? 0 : mStickers.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mStickers == null ? null : mStickers.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            String sticker = (String)getItem(arg0);

            if( sticker == null )
                return null;

            LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
            View rowView = inflater.inflate(R.layout.listitem_sticker, null, true);

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);

            Bitmap icon = (Bitmap) bitmapCache.get(sticker);
            if ( icon == null ) {
                icon = (Bitmap) nexOverlayPreset.getOverlayPreset().getIcon(mStickers.get(arg0),250,250);
                bitmapCache.put(sticker, icon);
            }
            image.setImageBitmap(icon);
            String s = mStickers.get(arg0);
            s = s.substring(s.lastIndexOf(".")+1);
            name.setText(s);

            if( sticker.equals(mCurSticker) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0xFF949494);

            return rowView;
        }
    }

    public void clearTemplate()
    {
        mCurTemplate = null;
        mTemplateAdaptor.setSize(mTemplates.size());
        mTemplateAdaptor.notifyDataSetChanged();

        mCloneProject = nexProject.clone(mProject);

        mEngin.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {

                AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

                LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

                View dialogView = inflater.inflate(R.layout.dialog_template_clear, null);

                final Spinner spinner_image_dur = (Spinner)dialogView.findViewById(R.id.spinner_image_dur);

                Integer[] items = new Integer[]{1000, 1100, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000};
                ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);

                spinner_image_dur.setAdapter(adapter);
                spinner_image_dur.setSelection(3);

                dialogBuilder.setView(dialogView).
                        setTitle("Select image duration").
                        setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                int iDur = (int)spinner_image_dur.getSelectedItem();
                                for(int idx = 0; idx < mCloneProject.getTotalClipCount(true); idx++)
                                {
                                    nexClip clip = mCloneProject.getClip(idx, true);

                                    clip.setImageClipDuration(iDur);
                                    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);

                                }
                                mCloneProject.updateProject();
                                seekBar_Projectseek.setMax(mCloneProject.getTotalTime());
                                mEngin.setProject(mCloneProject);

                                if( mEngin.updateProject(true) )
                                {
                                    if(faceMode != nexEngine.nexUndetectedFaceCrop.NONE) {
                                        setFaceModule();
                                    }
                                    int ret = mEngin.faceDetect(true , 1, faceMode);
                                    if(ret ==0){
                                        Toast.makeText(Template20TestActivity.this, "Disable FaceDetector", Toast.LENGTH_SHORT).show();
                                    }else{
                                        Toast.makeText(Template20TestActivity.this, "Enable FaceDetector", Toast.LENGTH_SHORT).show();
                                    }
                                    if( mEngin.play(true) == false )
                                    {
                                        Toast.makeText(Template20TestActivity.this, "Play error with crashed template", Toast.LENGTH_SHORT).show();
                                    }
                                }
                                else
                                {
                                    Toast.makeText(Template20TestActivity.this, "Update project error with crashed template", Toast.LENGTH_SHORT).show();
                                }
                            }
                        });
                dialogBuilder.create().show();
            }
        });
//
//
//
//        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);
//
//        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
//
//        View dialogView = inflater.inflate(R.layout.dialog_template_clear, null);
//
//        final Spinner spinner_image_dur = (Spinner)dialogView.findViewById(R.id.spinner_image_dur);
//
//        Integer[] items = new Integer[]{1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
//        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);
//
//        spinner_image_dur.setAdapter(adapter);
//        spinner_image_dur.setSelection(3);
//
//        mCurTemplate = null;
//        mTemplateAdaptor.notifyDataSetChanged();
//
//        mCloneProject = nexProject.clone(mProject);
//
//        dialogBuilder.setView(dialogView).
//                setTitle("Select image duration").
//                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
//                    @Override
//                    public void onClick(DialogInterface dialogInterface, int i) {
//                        int iDur = (int)spinner_image_dur.getSelectedItem();
//                        for(int idx = 0; idx < mCloneProject.getTotalClipCount(true); idx++)
//                        {
//                            nexClip clip = mCloneProject.getClip(idx, true);
//                            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE )
//                            {
//                                clip.setImageClipDuration(iDur);
//                            }
//                        }
//                        mCloneProject.updateProject();
//
//                        mEngin.stop(new nexEngine.OnCompletionListener() {
//                            @Override
//                            public void onComplete(int resultCode) {
//
//                                mEngin.setProject(mCloneProject);
//
//                                mEngin.updateProject();
//                                mEngin.play();
//                            }
//                        });
//                    }
//                });
//        dialogBuilder.create().show();
    }

    public class AdaptorTemplateListItem extends BaseAdapter {
        boolean setNewTemplate=false;

        private int size_for_template = 0;

        public void setSize(int size){

            size_for_template = size;
        }

        @Override
        public int getCount() {

            return size_for_template;
        }

        @Override
        public Object getItem(int arg0)
        {
            if(mTemplates.size() > arg0 && mTemplates != null)
                return mTemplates.get(arg0);
            else
                return null;
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {
            final int position = arg0;
            nexTemplateManager.Template template = (nexTemplateManager.Template)getItem(arg0);

            if( template == null )
                return null;

            View rowView = currentView;
            if( rowView == null ) {
                LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
                rowView = inflater.inflate(R.layout.listitem_template, null, true);
            }

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);
            Button play = (Button) rowView.findViewById(R.id.btn_play);
            Button del = (Button)rowView.findViewById(R.id.btn_del);
            play.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    watch.reset();
                    watch.start();

                    if (mTemplateAdaptor.getCount() <= position)
                        return;

                    mCurTemplate = (nexTemplateManager.Template) mTemplateAdaptor.getItem(position);
                    mTemplateAdaptor.setSize(mTemplates.size());
                    mTemplateAdaptor.notifyDataSetChanged();

                    Log.d(TAG, "Template with " + mCurTemplate.id());

                    mEngin.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            if(faceMode != nexEngine.nexUndetectedFaceCrop.NONE) {
                                //clearFaceModule();
                                setFaceModule();
                            }
                            //for Xiaomi test
                            /*for(int i =0; i<mProject.getTotalClipCount(true); i++){
                                mProject.getClip(i, true).getCrop().setEndPosition(new Rect(0, 0, 50, 50));
                                mProject.getClip(i,true).getCrop().setEndPositionLock(true);
                            }*/
                            String errorMsg = setEffects2Project();
                            if (errorMsg != null) {
                                Log.d(TAG, errorMsg);

                                Toast.makeText(Template20TestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();

                                // finish();
                                return;
                            }
                            mEngin.setProject(mCloneProject);
                            seekBar_Projectseek.setMax(mCloneProject.getTotalTime());
                            // mEngin.updateProject();
                            // mEngin.resume();
                            boolean enableFaceDetect = true;
                            if( mCurTemplate != null ){
                                if( mCurTemplate.category() == nexAssetPackageManager.Category.beattemplate
                                        || mCurTemplate.type() ==  nexAssetPackageManager.ItemMethodType.ItemBeat){
                                    enableFaceDetect = false;
                                }
                            }

                            if( enableFaceDetect ) {
                                int ret = mEngin.faceDetect(true, 1, faceMode);
                                if (BuildConfig.directTemplate) {
                                } else {
                                    if (ret == 0) {
                                        Toast.makeText(Template20TestActivity.this, "Disable FaceDetector", Toast.LENGTH_SHORT).show();
                                    } else {
                                        Toast.makeText(Template20TestActivity.this, "Enable FaceDetector", Toast.LENGTH_SHORT).show();
                                    }
                                }
                            }else{
                                mEngin.faceDetect(true, 1, nexEngine.nexUndetectedFaceCrop.NONE);
                            }
                            if (mEngin.play(true) == false) {
                                Toast.makeText(Template20TestActivity.this, "Play error with crashed template", Toast.LENGTH_SHORT).show();
                            }
                        }
                    });
                }
            });

            del.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mTemplateAdaptor.getCount() <= position)
                        return;

                    final nexTemplateManager.Template template = (nexTemplateManager.Template) mTemplateAdaptor.getItem(position);
                    if (template != null && template.isDelete()) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(Template20TestActivity.this);
                        String message = "Do you want to delete this item?";
                        builder.setMessage(message)
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        if ((nexTemplateManager.Template)mTemplateAdaptor.getItem(position) == mCurTemplate) {
                                            Log.d(TAG, "Delete current template!");
                                            setNewTemplate = true;
                                        }

                                        mKmTemplateManager.uninstallPackageById(template.id());

                                        // Don't reload template list. Just remove the template from template/icon list.
                                        //refreshAssets(true);
                                        if (mCurTemplate==template) {
                                            mCurTemplate = null;
                                            mCurBGM = null;
                                            mCurFont = null;
                                        }

                                        int idxOfArray = mTemplates.indexOf(template);
                                        if (idxOfArray != -1) {
                                            Log.d(TAG, "Delete asset : name="+template.name(null)+", id="+template.id()+", idxOfArray="+idxOfArray);
                                            mTemplates.remove(idxOfArray);
                                            mTemplateIcons.remove(idxOfArray);
                                            mTemplateAdaptor.setSize(mTemplates.size());
                                            mTemplateAdaptor.notifyDataSetChanged();
                                        }

                                        if ( setNewTemplate && mTemplateAdaptor.getCount() > 0 ) {
                                            mList.setSelection(position>0 ? position-1:0);
                                        } else {
                                            int pos = -1;
                                            for(nexTemplateManager.Template template : mTemplates ) {
                                                if(mCurTemplate!=null && template.id().equals(mCurTemplate.id())){
                                                    pos++;
                                                    break;
                                                }
                                                pos++;
                                            }
                                            mList.setSelection(pos);
                                            mEngin.resume();
                                            setNewTemplate = false;
                                        }
                                    }
                                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                mEngin.resume();
                            }
                        });
                        builder.create().show();
                    }
                    mEngin.pause();
                    return;
                }
            });

            while (mTemplateIcons.size() <= arg0)
            {
                SystemClock.sleep(10);
            }
            Bitmap icon = mTemplateIcons.get(arg0);
            if(icon == null){

                icon = template.icon();
                mTemplateIcons.remove(arg0);
                mTemplateIcons.add(arg0, icon);
            }
            image.setImageBitmap(icon);  //image.setImageBitmap(mTemplateIcons.get(arg0));
            name.setText(template.name(null));  // for template localized name.

            /* Sample code for getSupportedLocales() by Robin.
            String[] locales=template.getSupportedLocales();
            if ( locales != null ) {
                String lang=getResources().getConfiguration().locale.getLanguage().toLowerCase();
                String langCountry=getResources().getConfiguration().locale.getLanguage().toLowerCase() + "-" + getResources().getConfiguration().locale.getCountry().toLowerCase();

                for (String str : locales) {
                    if (str.equals(lang) == true) {
                        name.setText(template.name(lang));
                        Log.d(TAG, "Language="+lang+", Template name="+template.name(lang));
                        break;
                    } else if (str.equals(langCountry) == true) {
                        name.setText(template.name(langCountry));
                        Log.d(TAG, "Language="+langCountry+", Template name="+template.name(langCountry));
                        break;
                    }
                }
            }
            */

            if (setNewTemplate) {
                play.performClick();
                setNewTemplate=false;
            }

            if ( template.packageInfo().expireRemain()!=0 ) Log.d(TAG, "Asset idx=" + template.packageInfo().assetIdx() + " : Remain expiretime is = " + template.packageInfo().expireRemain() + ", InstalledTime = "+template.packageInfo().installedTime() + ". Expired="+nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).checkExpireAsset(template.packageInfo()));

            if ( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).checkExpireAsset(template.packageInfo()) ) {
                background.setBackgroundColor(Color.RED);
            }
            else if ( template.packageInfo().expireRemain()!=0 && nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).expireRemainTime(template.packageInfo()) < (1000*60*60*24) ) {
                background.setBackgroundColor(Color.MAGENTA);// You should check that the asset will be expired in 24 hours. (1000*60*60*24) = 1day.
            }
            else if(mCurTemplate!= null && template.id().equals(mCurTemplate.id()) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0x00000000);

            if( template.isDelete() )
            {
                del.setEnabled(true);
            }
            else {
                del.setEnabled(false);
            }

            return rowView;
        }
    }

    void updateAssets(boolean installed , int assetIdx) {
        nexAssetPackageManager.PreAssetCategoryAlias[] cats = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).updateAssetInManager(installed, assetIdx);

        for (nexAssetPackageManager.PreAssetCategoryAlias c : cats) {
            Log.d(TAG,"updateAssets("+installed+","+assetIdx+", update module="+c.toString());
            if (c == nexAssetPackageManager.PreAssetCategoryAlias.Template) {
                if(installed ) {
					nexTemplateManager.Template template=mKmTemplateManager.getTemplateById(mKmTemplateManager.getTemplateItemId(false, assetIdx));
					if (template.aspect() == 16 / 9.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
						mTemplates.add(template);
					} else if (template.aspect() == 9 / 16.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
						mTemplates.add(template);
					} else if (template.aspect() == 1.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1) {
						mTemplates.add(template);
					} else if (template.aspect() == 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1) {
						mTemplates.add(template);
					} else if (template.aspect() == 1 / 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2) {
						mTemplates.add(template);
					}
					mTemplateIcons.add(template.icon());
                    mTemplateAdaptor.setSize(mTemplates.size());
                    mTemplateAdaptor.notifyDataSetChanged();
                } else {
					refreshAssets(false);
                }
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.TextEffect) {

                updateOverlays();
                if( BuildConfig.directTemplate && mOverlays.size() > 0) {
                    mCurOverlay = mOverlays.get(0);

                    if( mCurOverlay.getVersion().startsWith("1") ) {

                        mKmOverlayManager.parseOverlay(mCurOverlay.getId(), mCurOverlayTitleInfos);

                        Date date = new Date();
                        DateFormat df = DateFormat.getDateInstance(DateFormat.MEDIUM);

                        mCurOverlayTitleInfos.get(0).setText("Story 1");
                        mCurOverlayTitleInfos.get(1).setText("");
                        mCurOverlayTitleInfos.get(2).setText("Story 1");
                        mCurOverlayTitleInfos.get(3).setText(df.format(date));
                    }
                }
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Audio) {
                mBGMs = mKmBGMManager.getAssetMedias();
                mCurBGM = null;
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Font) {
                mFonts = nexFont.getPresetList();
                mCurFont = null;
            } else if (c == nexAssetPackageManager.PreAssetCategoryAlias.Overlay) {
                mStickers.clear();
                String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                for (String id : stickers) {
                    mStickers.add(id);
                }
                mCurSticker = null;
            }
        }
    }

    void refreshAssets(final boolean bReloadAssets)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if( mTemplates != null ) mTemplates.clear();
                //if( mBGMs != null ) mBGMs.clear();
                if( mFonts != null ) mFonts.clear();
                if ( mStickers != null ) mStickers.clear();

                if( mKmTemplateManager != null ) {
					if ( bReloadAssets ) {
                    	mKmTemplateManager.loadTemplate();
					}
                    //mCurTemplate = null;
                    for (nexTemplateManager.Template template : mKmTemplateManager.getTemplates()) {
                        if (template.aspect() == 16 / 9.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 9 / 16.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 1.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 1 / 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2) {
                            mTemplates.add(template);
                        }

                        Log.d(TAG, String.format("Template Activity load templates(%s) (%f)", template.name("en"), template.aspect()));
                    }


                    for(Bitmap bitmap : mTemplateIcons)
                        if ( bitmap != null ) bitmap.recycle();
                    mTemplateIcons.clear();

                    boolean findTemplate = false;
                    for(nexTemplateManager.Template template : mTemplates ) {
                        mTemplateIcons.add(template.icon());
                        if(mCurTemplate!=null && template.id().equals(mCurTemplate.id())){
                            findTemplate = true;
                        }
                    }
                    if(!findTemplate){
                        mCurTemplate = null;
                    }
                }

                if( mKmOverlayManager != null ) {
					if ( bReloadAssets ) {
                    	mKmOverlayManager.loadOverlay();
					}

                    updateOverlays();

                    if( BuildConfig.directTemplate && mOverlays.size() > 0) {
                        mCurOverlay = mOverlays.get(0);

                        if( mCurOverlay.getVersion().startsWith("1") ) {
                            mKmOverlayManager.parseOverlay(mCurOverlay.getId(), mCurOverlayTitleInfos);

                            Date date = new Date();
                            DateFormat df = DateFormat.getDateInstance(DateFormat.MEDIUM);

                            mCurOverlayTitleInfos.get(0).setText("Story 1");
                            mCurOverlayTitleInfos.get(1).setText("");
                            mCurOverlayTitleInfos.get(2).setText("Story 1");
                            mCurOverlayTitleInfos.get(3).setText(df.format(date));
                        }
                    }
                }

                if( mKmBGMManager != null ){
                    if ( bReloadAssets ) {
                        mKmBGMManager.loadMedia(nexClip.kCLIP_TYPE_AUDIO);
                    }
                    mBGMs = mKmBGMManager.getAssetMedias();
                }

                //mBGMs = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
                mCurBGM = null;

                mFonts = nexFont.getPresetList();
                mCurFont = null;
                String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                for (String id : stickers) {
                    mStickers.add(id);
                }
                mCurSticker = null;
                mTemplateAdaptor.setSize(mTemplates.size());
                mTemplateAdaptor.notifyDataSetChanged();

                nexColorEffect.updatePluginLut();
            }
        });
    }

    void refreshAssets(final Object sync, final int preload_icon_count)
    {
        if( mTemplates != null ) mTemplates.clear();
        if( mBGMs != null ) mBGMs.clear();
        if( mFonts != null ) mFonts.clear();
        if( mStickers != null) mStickers.clear();

        if( mKmTemplateManager != null ) {

            mKmTemplateManager.loadTemplate(new Runnable(){

                @Override
                public void run(){

                    mCurTemplate = null;
                    boolean first = false;
                    for (nexTemplateManager.Template template : mKmTemplateManager.getTemplates()) {

                        String ver = template.getVersion();
                        if( mTemplate30Mode ) {
                            if( ver.startsWith("3") == false )
                                continue;
                        }
                        else {
                            if( ver.startsWith("2") == false )
                                continue;
                        }

                        if (template.aspect() == 16 / 9.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 9 / 16.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 1.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1) {
                            mTemplates.add(template);
                        } else if (template.aspect() == 1 / 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2) {
                            mTemplates.add(template);
                        }

                        if(mTemplates.size() > 0 && first == false){

                            first = true;
                            NotifyOfFirstTemplate(sync);
                            runOnUiThread(new Runnable() {

                                public void run() {

                                    mTemplateAdaptor.setSize(mTemplates.size());
                                    mTemplateAdaptor.notifyDataSetChanged();
                                }
                            });
                        }

                        Log.d(TAG, String.format("Template Activity load templates(%s) (%f)", template.name("en"), template.aspect()));
                    }
                    if(first == false){

                        NotifyOfFirstTemplate(sync);
                        runOnUiThread(new Runnable() {

                            public void run() {

                                mTemplateAdaptor.setSize(mTemplates.size());
                                mTemplateAdaptor.notifyDataSetChanged();
                            }
                        });
                    }

                    for(Bitmap bitmap : mTemplateIcons){

                        if(bitmap != null)
                            bitmap.recycle();
                    }

                    mTemplateIcons.clear();

                    int preload_count = preload_icon_count;

                    for(nexTemplateManager.Template template : mTemplates ){

                        if(preload_count > 0) {

                            preload_count--;
                            mTemplateIcons.add(template.icon());
                        }
                        else{
                            mTemplateIcons.add(null);
                        }
                    }

                    if( mKmOverlayManager != null ) {
                        mKmOverlayManager.loadOverlay();

                        updateOverlays();
                    }

                    if( mKmBGMManager != null ){
                        mKmBGMManager.loadMedia(nexClip.kCLIP_TYPE_AUDIO);
                        mBGMs = mKmBGMManager.getAssetMedias();
                    }
                    //mBGMs = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
                    mCurBGM = null;

                    mFonts = nexFont.getPresetList();
                    mCurFont = null;
                    nexColorEffect.updatePluginLut();

                    String[] stickers = nexOverlayPreset.getOverlayPreset(getApplicationContext()).getIDs();
                    for (String id : stickers) {
                        mStickers.add(id);
                    }
                    mCurSticker = null;

                    runOnUiThread(new Runnable(){

                        public void run(){

                            mTemplateAdaptor.setSize(mTemplates.size());
                            mTemplateAdaptor.notifyDataSetChanged();
                        }
                    });
                }
            });
        }
    }

    ProgressDialog barProgressDialog;
    Handler updateBarHandler;

    public void showInstallDialog(){
        barProgressDialog = new ProgressDialog(Template20TestActivity.this);
        barProgressDialog.setTitle("Installing Asset Package ...");
        barProgressDialog.setMessage("ready to install");
        barProgressDialog.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
        barProgressDialog.setProgress(0);
        barProgressDialog.setMax(100);
        barProgressDialog.show();

        mKmTemplateManager.installPackagesAsync(new nexAssetPackageManager.OnInstallPackageListener() {
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
            public void onCompleted(int event, int idx) {

                refreshAssets(true);

                barProgressDialog.dismiss();

                Toast.makeText(Template20TestActivity.this, "Template install elapsed =" + asset_install_watch.toString(), Toast.LENGTH_LONG).show();
                Log.d(TAG, "Template install elapsed = " + asset_install_watch.toString());
            }
        });

    }

    public boolean checkAsset(String itemId){
        final nexAssetPackageManager.Item item = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItemById(itemId);
        if( item != null ){
            if( item.packageInfo() != null ){
                if( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).checkExpireAsset(item.packageInfo()) ){
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
                            refreshAssets(true);
                        }
                    });
                    builder.setNegativeButton("re-install", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            //move Vasset for re-install asset.
                            nexAssetStoreAppUtils.runAssetStoreApp(Template20TestActivity.this,""+item.packageInfo().assetIdx());//Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK,  Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP
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

    public class AdaptorVendorListItem extends BaseAdapter {

        String[] vendorList = getResources().getStringArray(R.array.VendorList);

        @Override
        public int getCount() {
            return vendorList == null ? 0 : vendorList.length;
        }

        @Override
        public Object getItem(int arg0) {
            return vendorList == null ? null : vendorList[arg0];
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            String vendor = (String)getItem(arg0);

            if( vendor == null )
                return null;

            if( currentView == null ) {
                LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();
                currentView = inflater.inflate(R.layout.listitem_vendor, null, true);
            }

            TextView v = (TextView)currentView.findViewById(R.id.vendor);
            v.setText(vendor);

            if( storeVendor != null && storeVendor.compareTo(vendor) == 0 ) {
                v.setTextColor(Color.argb(255, 255, 0, 0));
            }
            else {
                v.setTextColor(Color.argb(255, 255, 255, 255));
            }

            return currentView;
        }
    }

    public void showSettingDlg() {
        final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(Template20TestActivity.this);

        LayoutInflater inflater = Template20TestActivity.this.getLayoutInflater();

        View dialogView = inflater.inflate(R.layout.dialog_template_set, null);

        dialogBuilder.setView(dialogView).
                setTitle("Template setting").
                setPositiveButton("Done", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                    }
                });

        final AlertDialog dialog = dialogBuilder.create();

        final Switch letterBox = (Switch)dialogView.findViewById(R.id.switchLetter);
        letterBox.setChecked(mEngin.getLetterBox());
        letterBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if( isChecked ) {
                    mEngin.setLetterBox(true);
                }
                else {
                    mEngin.setLetterBox(false);
                }
            }
        });

        RadioButton.OnClickListener showOnClickListener = new RadioButton.OnClickListener() {
            public void onClick(View v) {
                faceMode = (nexEngine.nexUndetectedFaceCrop)v.getTag();
                if( faceMode == nexEngine.nexUndetectedFaceCrop.ZOOM )
                {
                    letterBox.setEnabled(true);
                }
                else {
                    letterBox.setChecked(false);
                    letterBox.setEnabled(false);
                }
            }
        };

        final RadioButton rFace = (RadioButton)dialogView.findViewById(R.id.rFace);
        final RadioButton rZoom = (RadioButton)dialogView.findViewById(R.id.rZoom);

        rFace.setTag(nexEngine.nexUndetectedFaceCrop.NONE);
        rZoom.setTag(nexEngine.nexUndetectedFaceCrop.ZOOM);

        rFace.setOnClickListener(showOnClickListener);
        rZoom.setOnClickListener(showOnClickListener);

        if( faceMode == nexEngine.nexUndetectedFaceCrop.NONE ) {
            rFace.setChecked(true);
            letterBox.setEnabled(false);
        } else if( faceMode == nexEngine.nexUndetectedFaceCrop.ZOOM ) {
            rZoom.setChecked(true);
            letterBox.setEnabled(true);
        }

        ListView vendorList = (ListView)dialogView.findViewById(R.id.listview_vendor);
        final AdaptorVendorListItem vendorAdaptor = new AdaptorVendorListItem();
        vendorList.setAdapter(vendorAdaptor);
        vendorList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                storeVendor = (String)vendorAdaptor.getItem(position);
                vendorAdaptor.notifyDataSetChanged();
            }
        });

        Switch overlap = (Switch)dialogView.findViewById(R.id.switchOverlap);
        overlap.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isOverlapedTransition = isChecked;
            }
        });
        overlap.setChecked(isOverlapedTransition);

        Button reset = (Button)dialogView.findViewById(R.id.resetTemplate);
        reset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clearTemplate();
                dialog.dismiss();
            }
        });

        Button store = (Button)dialogView.findViewById(R.id.callVasset);
        store.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexAssetStoreAppUtils.setVendor(storeVendor);
                if ( nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext()) ) {
                    Log.d(TAG, "Asset store installed : " + storeVendor);
                    nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
                    nexAssetStoreAppUtils.setMimeType(nexAssetStoreAppUtils.AssetStoreMimeType_Template);
                    AssetStoreRequestCode = nexAssetStoreAppUtils.runAssetStoreApp(Template20TestActivity.this, null);//Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK, Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP
                } else {
                    Log.d(TAG, "Asset store isn't installed : " + storeVendor);
                    nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                }
                dialog.dismiss();
            }
        });

        Button installAsset = (Button)dialogView.findViewById(R.id.installTemplate);
        installAsset.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                asset_install_watch.reset();
                asset_install_watch.start();
                int count = mKmTemplateManager.findNewPackages();
                if (count > 0) {
                    // Toast.makeText(getApplicationContext(), "new package found. start install", Toast.LENGTH_LONG).show();
                    showInstallDialog();
                } else {
                    asset_install_watch.reset();
                    Toast.makeText(getApplicationContext(), "new package did not exist", Toast.LENGTH_SHORT).show();
                }
                dialog.dismiss();
            }
        });

        Button callIntent = (Button)dialogView.findViewById(R.id.intent);
        callIntent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngin.getProject() == null) {
                    Toast.makeText(getApplicationContext(), "select template! retry again", Toast.LENGTH_SHORT).show();
                    return;
                }
                mEngin.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        mEngineState = nexEngine.nexPlayState.IDLE.getValue();
                        Log.d(TAG, "nexEngine.stop() done! state=" + mEngineState);
                    }
                });
                //startActivity(mProject.makeKineMasterIntent());
                if ( nexAssetStoreAppUtils.isInstalledKineMaster(getApplicationContext())==true ) {
                    UtilityCode.launchKineMaster(Template20TestActivity.this, mListFilePath);
                } else {
                    Toast.makeText(getApplicationContext(), "Install Kinemaster first!", Toast.LENGTH_SHORT).show();
                }
                dialog.dismiss();
            }
        });


        dialog.setCancelable(false);
        dialog.show();
    }

    public void setFaceModule(){
        setNexFaceModule();
		//setSZFaceModule();
    }


    public void setNexFaceModule(){

        mEngin.setFaceModule("6a460d22-cd87-11e7-abc4-cec278b6b50a");
    }

    public void setGFWFaceModule(){

        mEngin.setFaceModule("31717431-8d11-41e5-9dc1-aaaaafda0dca");
    }

    public void setSZFaceModule(){

        mEngin.setFaceModule("b3fa3b66-419c-4053-95c2-5087a013495e");
    }

    public void clearFaceModule(){
        mEngin.setFaceModule(null);
    }

    private File getExportFile(int wid , int hei, String ext) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NexEditor_"+wid+"X"+hei + "_" + export_time+"." + ext);
        return exportFile;
    }
}
