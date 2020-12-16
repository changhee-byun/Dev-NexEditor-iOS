/******************************************************************************
 * File Name        : EffectCaptureActivity.java
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
import android.graphics.Bitmap;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexClipEffect;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexExportFormat;
import com.nexstreaming.nexeditorsdk.nexExportFormatBuilder;
import com.nexstreaming.nexeditorsdk.nexExportListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

public class EffectCaptureActivity extends Activity {
    private static final String TAG = "EffectCapture";

    private Button button_Play;

    private Spinner mSelectColorEffect;
    private Spinner mSelectLUTEffect;

    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private List<String> m_EffectIds;
    private List<String> m_LUTIds;
    nexEffectOptions selectOption;
    private boolean mEngineViewAvailable;
    private int aspectMode;
    private nexAspectProfile aspectProfile;
    private String exportFilePath;
    private LinearLayout topLL;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        m_listfilepath = new ArrayList<String>();
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));

        if( clip != null ){
            project.add(clip);
        }else{
            finish();
            return;
        }

        aspectMode = nexApplicationConfig.getAspectMode();

        aspectProfile = new nexAspectProfile(clip.getWidth(),clip.getHeight());
        nexApplicationConfig.setAspectProfile(aspectProfile);
        project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);

        setContentView(R.layout.activity_effect_capture);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_editorView = (nexEngineView)findViewById(R.id.engineview_effect_capture);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(m_editorView);
        mEngine.setProject(project);
        mEngine.updateProject();

        m_editorView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                mEngineViewAvailable = true;
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



        m_EffectIds = new ArrayList<String>();
        m_LUTIds = new ArrayList<String>();
        m_EffectIds.add("None");
        for( nexClipEffect effect : nexEffectLibrary.getEffectLibrary(getApplicationContext()).getClipEffects() ){
            if( effect.getMethodType() == nexAssetPackageManager.ItemMethodType.ItemRenderitem ){
                m_EffectIds.add(effect.getId());
            }
        }

        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
        m_LUTIds.add("None");
        for( nexColorEffect ce : colorEffects ){
            if( ce.getLUTId() != 0 ){
                m_LUTIds.add(ce.getAssetItemID());
            }
        }

        mSelectColorEffect = (Spinner)findViewById(R.id.spinner_effect_capture);
        ArrayAdapter<String> effectAdapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, m_EffectIds);
        mSelectColorEffect.setAdapter(effectAdapter);

        mSelectColorEffect.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //mEngine.getProject().getClip(0, true).setColorEffect(nexColorEffect.getPresetList().get(position));
                updateUI(m_EffectIds.get(position));
                mEngine.getProject().getClip(0, true).getClipEffect().setEffect(m_EffectIds.get(position));
                mEngine.updateProject();
                mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSelectLUTEffect = (Spinner)findViewById(R.id.spinner_lut_capture);
        ArrayAdapter<String> lutAdapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, m_LUTIds);
        mSelectLUTEffect.setAdapter(lutAdapter);

        mSelectLUTEffect.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                //mEngine.getProject().getClip(0, true).setColorEffect(nexColorEffect.getPresetList().get(position));
                updateUI(null);
                mEngine.getProject().getClip(0, true).setColorEffect(nexColorEffect.getLutColorEffect(m_LUTIds.get(position)));
                mEngine.updateProject();
                mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        topLL = (LinearLayout)findViewById(R.id.dynamicview);


        button_Play = (Button) findViewById(R.id.button_effect_capture_play);
        button_Play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                exportFilePath = getSaveFilePath().getAbsolutePath();
                nexExportFormat format = nexExportFormatBuilder.Builder()
                        .setType("jpeg")
                        .setWidth(aspectProfile.getWidth())
                        .setHeight(aspectProfile.getHeight())
                        .setPath(exportFilePath)
                        .setQuality(90)
                        .build();

                nexEngine.nexErrorCode code = mEngine.export(format, new nexExportListener() {
                    @Override
                    public void onExportFail(nexEngine.nexErrorCode err) {
                        Log.d(TAG, "onExportFail: " + err.toString());
                    }

                    @Override
                    public void onExportProgress(int percent) {

                    }

                    @Override
                    public void onExportDone(Bitmap bitmap) {
                        Log.d(TAG, "onExportDone: ");
                        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{exportFilePath}, null, null);
                    }
                });

                if( code.getValue() != 0 ){
                    Log.d(TAG, "export fail!: "+code);
                }
            }
        });


        final Button capture = (Button)findViewById(R.id.button_effect_capture);
        capture.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mEngine.stop();
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

    private void updateUI(String id){
        if( id != null ) {
            topLL.removeAllViews();
            selectOption = nexEffectLibrary.getEffectLibrary(getApplicationContext()).getEffectOptions(getApplicationContext(), id);
            if (selectOption != null) {
                int i = 0;
                for (nexEffectOptions.RangeOpt opt : selectOption.getRangeOptions()) {
                    final int inner_i = i;
                    LinearLayout subLL = new LinearLayout(getApplicationContext());
                    subLL.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 1f));
                    subLL.setOrientation(LinearLayout.HORIZONTAL);

                    TextView renderParam = new TextView(getApplicationContext());
                    renderParam.setText(opt.getLabel());
                    renderParam.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 2f));

                    final TextView count = new TextView(getApplicationContext());
                    count.setText("" + opt.getValue());
                    count.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 2f));

                    SeekBar range = new SeekBar(getApplicationContext());
                    range.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 1f));
                    range.setId(i);
                    range.setMax(opt.max() - opt.min());
                    range.setProgress(opt.getValue() + opt.min());
                    range.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                        private int lastValue;
                        @Override
                        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                            if( selectOption != null ) {
                                lastValue = progress + selectOption.getRangeOptions().get(inner_i).min();
                                count.setText("" + lastValue);
                            }
                            //mEngine.fas
                        }

                        @Override
                        public void onStartTrackingTouch(SeekBar seekBar) {

                        }

                        @Override
                        public void onStopTrackingTouch(SeekBar seekBar) {
                            if( selectOption != null ){
                                selectOption.getRangeOptions().get(inner_i).setValue(lastValue);
                                mEngine.getProject().getClip(0,true).getClipEffect().updateEffectOptions(selectOption,true);
                                mEngine.updateProject();
                                mEngine.seek(2000);
                            }
                        }
                    });

                    subLL.addView(renderParam);
                    subLL.addView(range);
                    subLL.addView(count);

                    topLL.addView(subLL);

                    i++;
                }
            }
        }
    }

    private File getSaveFilePath(){
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File captureDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        captureDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();

        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        return new File(captureDir, "EffectCap_" + export_time+".jpeg");
    }

    private void saveBitmap(Bitmap bmp){
        FileOutputStream fos = null;

        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File captureDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Capture");
        captureDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();

        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File out = new File(captureDir, "EffectCap_" + export_time+".png");

        try {
            out.createNewFile();
            fos = new FileOutputStream(out);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, fos);

            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{out.getAbsolutePath()}, null, null);
            Toast.makeText(getApplicationContext(),"saved="+out.getAbsolutePath(),Toast.LENGTH_SHORT).show();
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
    }

    @Override
    protected void onResume() {
        if( mEngineViewAvailable ){
            mEngine.seek(2000);
        }
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        nexApplicationConfig.setAspectMode(aspectMode);
        super.onDestroy();
    }
}
