/******************************************************************************
 * File Name        : TransCoderTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.export;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTranscode;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class TransCoderTestActivity extends Activity {
    private nexEngine mEngine;
    private nexTranscode mTransCoder;
    private ArrayList<String> m_listfilepath;
    private nexProject project;
    private ProgressBar mProgressTransCode;
    private TextView mSource;

    private TextView mOutput;

    private TextView mResult;

    private EditText mOutputW;
    private EditText mOutputH;
    private EditText mBitrate;

    private Spinner mOutputRM;
    private Spinner mOutputR;
    private CheckBox mOutputFit;

    private TextView mTransCodeTime;

    private ImageView mResultThumb;
    private Bitmap mResultBitmap;
    private int outputWidth = 640;
    private int outputHeight = 360;
    private int outputBitrate = 2*1024*1024;
    private int outputSamplingRate = 44100;

    private nexTranscode.Rotate outputRotateMeta = nexTranscode.Rotate.BYPASS;
    private nexTranscode.Rotate outputRotate = nexTranscode.Rotate.CW_0;

    private boolean outputFit = true;
    private long transStartTime = 0;
    private File outFile;
    private Handler mHandler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_trans_coder_test);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if( clip != null ){
            project = new nexProject();
            project.add(clip);
        }else{
            Toast.makeText(getApplicationContext(), "Not supported clip : " + m_listfilepath.get(0), Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        int rotate = 360 - clip.getRotateInMeta();
        if(rotate == 360 ){
            rotate = 0;
        }

        mSource = (TextView)findViewById(R.id.textView_vs_source);
        mSource.setText(", W:"+clip.getWidth()+", H:"+clip.getHeight()+", D:"+clip.getTotalTime()+"(ms)"+", RM:"+rotate);

        mOutput = (TextView)findViewById(R.id.textView_vs_output);
        mOutputFit = (CheckBox)findViewById(R.id.checkBox_output_fit);
        mOutputFit.setChecked(true);

        mProgressTransCode = (ProgressBar)findViewById(R.id.progressBar_vs_TransCode);
        mProgressTransCode.setProgress(0);
        mTransCodeTime = (TextView)findViewById(R.id.textView_vs_transcode_time);
        mTransCodeTime.setText("T:0(ms)");

        mResult = (TextView)findViewById(R.id.textView_vs_result);

        final Button buttonTransCode = (Button)findViewById(R.id.button_vs_transcode);
        buttonTransCode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mProgressTransCode.setProgress(0);
                transStartTime = System.currentTimeMillis();
                outFile = getOutputFile(1, outputWidth, outputHeight);
                nexTranscode.Option opt = new nexTranscode.Option(outFile, outputWidth, outputHeight, outputBitrate, outputSamplingRate, outputFit, outputRotateMeta, outputRotate);
                mTransCoder.run(opt);
            }
        });

        mOutputW = (EditText)findViewById(R.id.editText_output_width);
        mOutputW.setText("" + outputWidth);

        mOutputH = (EditText)findViewById(R.id.editText_output_height);
        mOutputH.setText("" + outputHeight);

        mBitrate = (EditText)findViewById(R.id.editText_bitrate);
        mBitrate.setText("" +outputBitrate);
        mOutputRM = (Spinner)findViewById(R.id.spinner_output_rotate_meta);
        ArrayAdapter<String> adapter_rm = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, new String[]{"BYPASS","CW_0","CW_90","CW_180","CW_270"});
        mOutputRM.setAdapter(adapter_rm);

        mOutputR = (Spinner)findViewById(R.id.spinner_output_rotate);
        ArrayAdapter<String> adapter_r = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, new String[]{"CW_0","CW_90","CW_180","CW_270"});
        mOutputR.setAdapter(adapter_r);

        final Button buttonSet = (Button)findViewById(R.id.button_output_set);
        buttonSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mOutputW.getText().length() != 0) {
                    outputWidth = Integer.parseInt(mOutputW.getText().toString());
                }

                if (mOutputH.getText().length() != 0) {
                    outputHeight = Integer.parseInt(mOutputH.getText().toString());
                }
                if (mBitrate.getText().length() != 0) {
                    outputBitrate = Integer.parseInt(mBitrate.getText().toString());
                }
                outputRotateMeta = nexTranscode.Rotate.valueOf((String)mOutputRM.getSelectedItem());
                outputRotate = nexTranscode.Rotate.valueOf((String)mOutputR.getSelectedItem());
                outputFit = mOutputFit.isChecked();
                printOutPut();
            }
        });

        mResultThumb = (ImageView)findViewById(R.id.imageView_vs_result);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        nexTranscode.init(mEngine);
        mTransCoder = new nexTranscode(clip.getPath(),false);

        mTransCoder.setTransCoderListener(new nexTranscode.OnTransCoderListener() {
            @Override
            public void onProgress(int progress, int maxProgress) {
                mProgressTransCode.setMax(maxProgress);
                mProgressTransCode.setProgress(progress);

            }

            @Override
            public void onTransCodeDone(nexTranscode.Error error, int suberror) {
                long time = System.currentTimeMillis() - transStartTime;
                mTransCodeTime.setText("T:"+time+"(ms)");
                if( error == nexTranscode.Error.NONE ) {
                    MediaScannerConnection.scanFile(getApplicationContext(), new String[]{outFile.getAbsolutePath()}, null, null);
                    resultThumbnail();
                }
            }
        });
        printOutPut();
    }

    private void printOutPut(){
        mOutput.setText(", W:" + outputWidth+", H:" + outputHeight+", B:"+outputBitrate+", RM:"+outputRotateMeta+", R:"+outputRotate );
    }

    private File getOutputFile(int mode ,int wid , int hei) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File outputDir  = new File(sdCardPath + File.separator + "KM" + File.separator + "Trans");
        outputDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        String mid = "export";
        if( mode == 1 ){
            mid = "trans";
        }
        File exportFile = new File(outputDir, "nexEditor_"+mid+"_"+wid+"X"+hei +"_"+ export_time+".mp4");
        return exportFile;
    }

    public Runnable runnable = new Runnable() {
        @Override
        public void run()
        {
            mResultThumb.setImageBitmap(mResultBitmap);
        }
    };

    private void resultThumbnail(){

         nexClip clip = new nexClip(outFile.getAbsolutePath());
         int rotate = 360 - clip.getRotateInMeta();
         if (rotate == 360) {
              rotate = 0;
         }
         mResult.setText("W:" + clip.getWidth() + ", H:" + clip.getHeight() + ", RM:" + rotate + ", D:" + clip.getTotalTime());

         clip.getVideoClipDetailThumbnails(clip.getWidth(), clip.getHeight(), 0, 2000, 1, clip.getRotateInMeta(), new nexClip.OnGetVideoClipDetailThumbnailsListener() {
              @Override
              public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                  if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                        mResultBitmap = Bitmap.createBitmap(bm);
                        mHandler.post(runnable);
                      //mResultThumb.setImageBitmap(Bitmap.createBitmap(bm));
                   }
               }
          });
    }

}
