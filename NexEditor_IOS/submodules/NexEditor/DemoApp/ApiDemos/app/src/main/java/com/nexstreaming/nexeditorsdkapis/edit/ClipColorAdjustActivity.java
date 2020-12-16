/******************************************************************************
 * File Name        : ClipColorAdjustActivity.java
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
import android.graphics.Bitmap;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;


public class ClipColorAdjustActivity extends Activity {
    private static final String TAG = "ClipColorAdjust";
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

    private SeekBar seekBar_CustomLUTPower;
    private TextView textView_CustomLUTPower;
    private Spinner mSelectCustomLUT_A;
    private Spinner mSelectCustomLUT_B;

    private Button button_Play;
    private Button button_Export;
    private Button button_Intent;

    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;

    private boolean mEngineViewAvailable = false;
    private Spinner mSelectColorEffect;
    private nexEngineListener mEditorListener;
    private enum State {
        IDLE,
        STOP,
        PLAY
    }

    State state = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_clip_color_adjust);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));

        if( clip != null ){
            clip.setRotateDegree(clip.getRotateInMeta());
            project.add(clip);
            project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        }

        m_editorView = (nexEngineView)findViewById(R.id.engineview_clip_color_adjust);
        m_editorView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                mEngineViewAvailable = true;
                if( mEngine != null)
                    mEngine.seek(2000);
            }

            @Override
            public void onEngineViewSizeChanged(int width, int height) {

            }

            @Override
            public void onEngineViewDestroyed() {
                mEngineViewAvailable = false;
            }
        });
        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int oldState, int newState) {

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
                button_Play.setText("Play");
                state = State.STOP;
            }

            @Override
            public void onPlayFail(int err, int iClipID) {

            }

            @Override
            public void onPlayStart() {

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

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.updateProject();
        mEngine.setEventHandler(mEditorListener);

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
                mEngine.setBrightness(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_brightness,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Bri.setText(""+ value);
                mEngine.setBrightness(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_brightness, value);
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
                mEngine.setSaturation(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_saturation,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Sat.setText(""+ value);
                mEngine.setSaturation(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_saturation, value);
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
                mEngine.setContrast(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_contrast,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Con.setText(""+ value);
                mEngine.setContrast(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_contrast, value);
            }
        });
        
        seekBar_Vignette = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_vignette);
        textView_Vignette = (TextView) findViewById(R.id.textview_clip_color_adjust_vignette);
        seekBar_Vignette.setMax(100);
        seekBar_Vignette.setProgress(0);
        seekBar_Vignette.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_Vignette.setText(String.format("%1.2f", value * 0.01f));
                mEngine.setVignette(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_vignette, value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Vignette.setText(String.format("%1.2f", value * 0.01f));
                mEngine.setVignette(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_vignette, value);
            }
        });
        
        seekBar_VignetteRange = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_vignette_range);
        textView_InnerVignette = (TextView) findViewById(R.id.textview_clip_color_adjust_vignette_range);
        
        seekBar_VignetteRange.setMax(100);
        seekBar_VignetteRange.setProgress(0);
        seekBar_VignetteRange.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_InnerVignette.setText(String.format("%1.2f", value * 0.01f));
                mEngine.setVignetteRange(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_vignetteRange, value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_InnerVignette.setText(String.format("%1.2f", value * 0.01f));
                mEngine.setVignetteRange(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_vignetteRange, value);
            }
        });

        seekBar_Sharpness = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_sharpness);
        textView_Sharpness = (TextView) findViewById(R.id.textview_clip_color_adjust_sharpness);
        seekBar_Sharpness.setMax(500);
        seekBar_Sharpness.setProgress(0);
        seekBar_Sharpness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_Sharpness.setText(String.format("%1.2f",value * 0.01f));
                mEngine.setSharpness(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_sharpness, value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Sharpness.setText(String.format("%1.2f", value * 0.01f));
                mEngine.setSharpness(value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.adj_sharpness, value);
            }
        });
        
        seekBar_CustomLUTPower = (SeekBar) findViewById(R.id.seekbar_clip_color_adjust_customlut);
        textView_CustomLUTPower = (TextView) findViewById(R.id.textview_clip_color_adjust_customlut);
        textView_CustomLUTPower.setText(""+ 100000);
        seekBar_CustomLUTPower.setMax(100000);
        seekBar_CustomLUTPower.setProgress(100000);
        seekBar_CustomLUTPower.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = -1;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                textView_CustomLUTPower.setText(""+ value);
                mEngine.fastPreviewCustomlut( value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

                if(value > -1) {
                    textView_CustomLUTPower.setText("" + value);
                    mEngine.getProject().getClip(0, true).setCustomLUTPower(value);
                    mEngine.updateProject();
                    mEngine.seek(2000);
                }
            }
        });

        List<String> items = new ArrayList<String>();

        List<nexColorEffect> list = nexColorEffect.getPresetList();
        for( nexColorEffect ce: list ){
            items.add(ce.getPresetName());
        }

        mSelectColorEffect = (Spinner)findViewById(R.id.spinner_clip_color_adjust_coloreffect);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, items);
        mSelectColorEffect.setAdapter(adapter);

        mSelectColorEffect.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEngine.getProject().getClip(0, true).setColorEffect(nexColorEffect.getPresetList().get(position));
                mEngine.updateProject();
                mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        final List<String> custom_lut_items = new ArrayList<>();

        makeAssetLUT("custom_lut/sample.bin", "com.user.lut.sample", 1);
        makeAssetLUT("custom_lut/cine-max.bin", "com.user.lut.cine_max", 1);
        makeAssetLUT("custom_lut/cine-min.bin", "com.user.lut.cine_min", 1);
        makeAssetLUT("custom_lut/test.cube", "com.user.lut.test_cube", 2);

        custom_lut_items.add("NONE");
        custom_lut_items.add("com.user.lut.sample");
        custom_lut_items.add("com.user.lut.cine_max");
        custom_lut_items.add("com.user.lut.cine_min");
        custom_lut_items.add("com.user.lut.test_cube");

        mSelectCustomLUT_A = (Spinner)findViewById(R.id.spinner_clip_color_adjust_customlut_a);
        ArrayAdapter<String> custom_lut_adapter_a = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, custom_lut_items);
        mSelectCustomLUT_A.setAdapter(custom_lut_adapter_a);

        mSelectCustomLUT_A.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                mEngine.getProject().getClip(0, true).setCustomLUTA(nexColorEffect.getLUTUID(custom_lut_items.get(position)));
                mEngine.updateProject();
                mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSelectCustomLUT_B = (Spinner)findViewById(R.id.spinner_clip_color_adjust_customlut_b);
        ArrayAdapter<String> custom_lut_adapter_b = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, custom_lut_items);
        mSelectCustomLUT_B.setAdapter(custom_lut_adapter_b);

        mSelectCustomLUT_B.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                mEngine.getProject().getClip(0, true).setCustomLUTB(nexColorEffect.getLUTUID(custom_lut_items.get(position)));
                mEngine.updateProject();
                mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        button_Export = (Button) findViewById(R.id.button_clip_color_adjust_export);
        button_Export.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(state == State.PLAY) {
                    button_Play.setText("Play");
                    state = State.STOP;
                }
                mEngine.stop();
                startActivity(new Intent(getBaseContext(), ExportActivity.class));

            }
        });

        button_Play = (Button) findViewById(R.id.button_clip_color_adjust_play);
        button_Play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(state == State.IDLE || state == State.STOP) {
                    mEngine.stop();
                    mEngine.play();
                    button_Play.setText("Stop");
                    state = State.PLAY;
                }
                else {
                    mEngine.stop();
                    button_Play.setText("Play");
                    state = State.STOP;
                }
            }
        });

        button_Intent = (Button) findViewById(R.id.button_clip_color_adjust_intent);
        button_Intent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                state = State.STOP;
                //startActivity(mEngine.getProject().makeKineMasterIntent());
                UtilityCode.launchKineMaster(ClipColorAdjustActivity.this, m_listfilepath);
            }
        });

        final Button capture = (Button)findViewById(R.id.button_clip_color_adjust_capture);
        capture.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
                button_Play.setText("Play");
                state = State.STOP;
                mEngine.captureCurrentFrame(new nexEngine.OnCaptureListener() {
                    @Override
                    public void onCapture(Bitmap bm) {
                        saveBitmap(bm);
                    }

                    @Override
                    public void onCaptureFail(nexEngine.nexErrorCode error) {

                    }
                });
            }
        });
   }

   private boolean makeAssetLUT(String path, String id, int type){

       //type 0 - legacy lut
       //type 1 - 1024x1024 binary yuv lut
       //type 2 - cube lut

       try {

           InputStream is = getAssets().open(path);
           ByteArrayOutputStream buf = new ByteArrayOutputStream();
           try {
               copy(is,buf);
               nexColorEffect.addCustomLUT(id, buf.toByteArray(), 0, buf.size(), type);
           } finally {
               is.close();
           }
       }
       catch(IOException e){

           e.printStackTrace();
           return false;
       }
       return true;
   }

    private static void copy(InputStream input, OutputStream output) throws IOException {
        byte[] copybuf = new byte[1024*4];
        long count = 0;
        int n = 0;
        while (-1 != (n = input.read(copybuf))) {
            output.write(copybuf, 0, n);
            count += n;
        }
    }

    private void saveBitmap(Bitmap bmp){
        FileOutputStream fos = null;

        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File captureDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Capture");
        captureDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();

        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File out = new File(captureDir, "ColorAdj_" + export_time+".png");

        try {
            out.createNewFile();
            fos = new FileOutputStream(out);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);
        }catch (final FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    /**
                     * can ignore exception.
                     */
                }
            }
        }
        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{out.getAbsolutePath()}, null, null);
    }

    @Override
    protected void onStart() {
        if( mEngineViewAvailable ){
            mEngine.seek(2000);
        }
        Log.d(TAG, "onStart");
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        nexColorEffect.removeAllCustomLUT();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
    @Override
    protected void onResume() {
        mEngine.setEventHandler(mEditorListener);
        super.onResume();
    }
}
