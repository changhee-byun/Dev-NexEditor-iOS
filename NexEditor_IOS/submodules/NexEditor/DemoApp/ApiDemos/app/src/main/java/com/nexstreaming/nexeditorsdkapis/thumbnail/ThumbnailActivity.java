/******************************************************************************
 * File Name        : ThumbnailActivity.java
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

package com.nexstreaming.nexeditorsdkapis.thumbnail;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.YuvImage;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.AutoTestUtils;
import com.nexstreaming.nexeditorsdkapis.common.ImageCompare;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;


public class ThumbnailActivity extends Activity {
    private static final String TAG = "ThumbnailActivity";

    private ArrayList<String> mListFilePath;
    private String mResolutionItems[] = {"640x360", "320x180", "160x90"};
    private String mAngleItems[] = {"0", "90", "180", "270"};

    private int mSelectedResolution =0;
    private int mSelectedAngle = 0;
    private int mWidth = 160;
    private int mHeight =90;
    private int mAngle = 0;
    private int mStartTime = 0;
    private int mEndTime = 0;

    private nexClip mClip;
    private Spinner mSpinnerResolution;
    private Spinner mSpinnerAngle;
    private SeekBar mSeekbarStartTime;
    private SeekBar mSeekbarEndTime;
    private EditText mEdittextCount;
    private EditText mEdittextStart;
    private EditText mEdittextEnd;
    private Button mGetThumbnail;
    private Button mCancelThumbnail;


    private Stopwatch watch = new Stopwatch();

    private AdaptorSeekTable mGridAdapterSeekTable;
    private AdaptorThumbnail mGridAdapterThumbnail;

    // for test Auto
    String at_test_root = null;
    String at_result_root = null;
    String at_test_path = null;
    String at_result_path = null;
    String at_testFile = null;

    ImageCompare at_imageCompare = new ImageCompare();
    ImageCompare.CompareValue at_compareValue = new ImageCompare.CompareValue();

    StringBuilder at_sb = new StringBuilder();
    BlockingQueue<String> at_blockingQueue = new ArrayBlockingQueue<String>(1);
    Stopwatch at_elapsed = new Stopwatch();

    boolean at_enable = false;
    Handler at_message_handler = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_thumbnail);



        final ScrollView sv = (ScrollView)findViewById(R.id.scrollView);

        mGridAdapterSeekTable = new AdaptorSeekTable(null);
        final GridView gvSeek = (GridView)findViewById(R.id.gridview_seektable);
        gvSeek.setAdapter(mGridAdapterSeekTable);
        gvSeek.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                AdaptorSeekTable.Item item = (AdaptorSeekTable.Item)mGridAdapterSeekTable.getItem(position);
                item.sel = !item.sel;
                mGridAdapterSeekTable.notifyDataSetChanged();
            }
        });
        gvSeek.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                 sv.requestDisallowInterceptTouchEvent(true);
                return false;
            }
        });

        mGridAdapterThumbnail = new AdaptorThumbnail();
        GridView gvThumb = (GridView) findViewById(R.id.gridview_thumbnail_info);
        gvThumb.setAdapter(mGridAdapterThumbnail);
        gvThumb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                sv.requestDisallowInterceptTouchEvent(true);
                return false;
            }
        });

        // mSwitchNoCache = (Switch) findViewById(R.id.switch_thumbnail_nocache);
        mEdittextCount = (EditText)findViewById(R.id.edittext_thumbnail_line1);

        mSpinnerResolution = (Spinner) findViewById(R.id.spinner_thumbnail_select);
        ArrayAdapter<String> arrayResolutionAdapter = new ArrayAdapter(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, mResolutionItems);
        mSpinnerResolution.setAdapter(arrayResolutionAdapter);
        mSpinnerResolution.setSelection(0);

        mSpinnerResolution.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedResolution = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSpinnerAngle = (Spinner) findViewById(R.id.spinner_thumbnail_angle);
        ArrayAdapter<String> arrayAngleAdapter = new ArrayAdapter(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, mAngleItems);
        mSpinnerAngle.setAdapter(arrayAngleAdapter);
        mSpinnerAngle.setSelection(0);

        mSpinnerAngle.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedAngle = position;
                //Log.d(LOG_TAG, "mSpinnerAngle onItemSelected $ postion: " + selectedAngle);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });


        mSeekbarStartTime = (SeekBar) findViewById(R.id.seekbar_thumbnail_starttime);
        mSeekbarStartTime.setProgress(0);
        mSeekbarStartTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                mEdittextStart.setText(""+value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mStartTime = value;
            }
        });

        mSeekbarEndTime = (SeekBar) findViewById(R.id.seekbar_thumbnail_endtime);
        mSeekbarEndTime.setProgress(0);
        mSeekbarEndTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                mEdittextEnd.setText(""+value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEndTime = value;
            }
        });

        mEdittextStart = (EditText) findViewById(R.id.edittext_thumbnail_starttime);
        mEdittextStart.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                try {
                    if (UtilityCode.isNumeric(s.toString())) {
                        if (Integer.parseInt(s.toString()) <= mClip.getTotalTime()) {
                            mSeekbarStartTime.setProgress(Integer.parseInt(s.toString()));
                            mStartTime = Integer.parseInt(s.toString());
                        }

                    } else if (s.length() == 0) {
                        mSeekbarStartTime.setProgress(0);
                        mStartTime = 0;
                    }
                } catch (NumberFormatException e) {
                    Log.e(TAG, e.toString());
                }

                Log.d(TAG, "onTextChanged: " + s);
                mEdittextStart.setSelection(mEdittextStart.length());
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });

        mEdittextEnd = (EditText) findViewById(R.id.edittext_thumbnail_endtime);
        mEdittextEnd.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                try {
                    if (UtilityCode.isNumeric(s.toString())) {
                        if (Integer.parseInt(s.toString()) <= mClip.getTotalTime()) {
                            mSeekbarEndTime.setProgress(Integer.parseInt(s.toString()));
                            mEndTime = Integer.parseInt(s.toString());
                        }
                    } else if (s.length() == 0) {
                        mSeekbarEndTime.setProgress(0);
                        mEndTime = 0;
                    }
                } catch (NumberFormatException e) {
                    Log.e(TAG, e.toString());
                }

                Log.d(TAG, "onTextChanged: " + s);
                mEdittextEnd.setSelection(mEdittextEnd.length());
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });

        mCancelThumbnail = (Button) findViewById(R.id.button_thumbnail_cancel);
        mCancelThumbnail.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ProgressBar pg = (ProgressBar)findViewById(R.id.progress_thumnail);

                Log.d(TAG, "Thumbnail progress value is="+pg.getProgress());
                if ( pg.getProgress() != 0 ) {
                    Log.d(TAG, "cancelThumbnails() start");
                    mClip.cancelThumbnails();
                    Log.d(TAG, "cancelThumbnails() end");
                    pg.setProgress(0);
                }
                return;
            }
        });
        mGetThumbnail = (Button) findViewById(R.id.button_thumbnail_get);
        mGetThumbnail.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                watch.reset();
                watch.start();

                mGridAdapterThumbnail.clearItem();
                mGridAdapterThumbnail.notifyDataSetChanged();

                mWidth = 160;
                mHeight = 90;
                if (mSelectedResolution == 0) {
                    mWidth = 640;
                    mHeight = 360;
                } else if (mSelectedResolution == 1) {
                    mWidth = 320;
                    mHeight = 180;
                }

                mAngle = 0;
                if (mSelectedAngle == 1) {
                    mAngle = 90;
                } else if (mSelectedAngle == 2) {
                    mAngle = 180;
                } else if (mSelectedAngle == 3) {
                    mAngle = 270;
                }

                if( mAngle == 90 || mAngle == 270 ) {
                    int tmp = mWidth;
                    mWidth = mHeight;
                    mHeight = tmp;
                }

                int maxCount = 0;
                if (mEdittextCount.getText().toString().length() != 0)
                    maxCount = Integer.parseInt(mEdittextCount.getText().toString());

                final ProgressBar pg = (ProgressBar)findViewById(R.id.progress_thumnail);
                pg.setMax(maxCount);
                pg.setProgress(0);

                int [] sTable = mGridAdapterSeekTable.getSeekTable();
                if( sTable != null ) {
                    pg.setMax(sTable.length);
                    mClip.getVideoClipDetailThumbnails(mWidth, mHeight, 0, 0, sTable.length, mAngle, false, sTable, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
                        @Override
                        public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                            if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                                mGridAdapterThumbnail.addItem(String.format("cur:%d total:%d, time:%d", index, totalCount, timestamp), Bitmap.createBitmap(bm));
                                pg.setProgress(index);

                                if( at_enable ) {
                                    at_elapsed.stop();
                                    String ret_path = at_result_path + at_testFile + "_" + timestamp + "_" + index + "_" + totalCount + ".png";
                                    String expect_path = at_test_path + "expect/" + at_testFile + "_" + timestamp + "_" + index + "_" + totalCount + ".png";

                                    String checked = "(not checked)";
                                    Bitmap expect = BitmapFactory.decodeFile(expect_path);

                                    if (expect != null) {
                                        at_compareValue.reset();
                                        at_imageCompare.compareBitmapWithPSNR(bm, expect, at_compareValue);
                                        if (at_compareValue.diffPercent < 5f) {
                                            checked = "(pass)";
                                        } else {
                                            checked = "(not pass)";
                                            AutoTestUtils.savebitmap(bm, ret_path);
                                        }
                                        expect = null;
                                    } else {
                                        AutoTestUtils.savebitmap(bm, ret_path);
                                    }

                                    at_sb.append("Thumbnail saved" + checked + " : " + at_result_path + " diffPercent(" + at_compareValue.diffPercent + ") aveDiff(" + at_compareValue.aveDiff + ")");
                                    at_sb.append(" mse(" + at_compareValue.mse + ") snr(" + at_compareValue.snr + ")");
                                    at_sb.append(" psnr(" + at_compareValue.psnr + "/" + at_compareValue.max_psnr + ")\n");

                                    at_elapsed.start();
                                }

                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                mGridAdapterThumbnail.notifyDataSetChanged();
                                Toast.makeText(ThumbnailActivity.this, "GetThumbnail elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();
                                Log.d(TAG,"GetThumbnail elapsed = "+watch.toString());
                                pg.setProgress(index);

                                if( at_enable ) {
                                    try {
                                        at_blockingQueue.put("Complated!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                Toast.makeText(getApplicationContext(), "getVideoClipDetailThumbnails fail!", Toast.LENGTH_SHORT).show();

                                if( at_enable ) {
                                    try {
                                        at_blockingQueue.put("fail!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                Toast.makeText(getApplicationContext(), "system error", Toast.LENGTH_LONG).show();

                                if( at_enable ) {
                                    try {
                                        at_blockingQueue.put("system error!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                        }
                    });
                    return;
                }

                CheckBox edPf = (CheckBox) findViewById(R.id.edittext_thumbnail_pframe);
                CheckBox edFirst = (CheckBox) findViewById(R.id.edittext_thumbnail_first);
                CheckBox edLast = (CheckBox) findViewById(R.id.edittext_thumbnail_last);

                if (mStartTime >= mEndTime) {
                    Toast.makeText(getApplicationContext(), "startTime >= endTime, retry again!!", Toast.LENGTH_SHORT).show();
                    return;
                }

                mClip.getVideoClipDetailThumbnails(mWidth, mHeight, mStartTime, mEndTime, maxCount, mAngle, true, edPf.isChecked(), edFirst.isChecked(), edLast.isChecked(), null, new nexClip.OnGetVideoClipDetailThumbnailsListener() {

                    @Override
                    public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                        if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                            mGridAdapterThumbnail.addItem(String.format("cur:%d total:%d, time:%d", index, totalCount, timestamp), Bitmap.createBitmap(bm));
                            pg.setProgress(index);
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                            mGridAdapterThumbnail.notifyDataSetChanged();
                            Toast.makeText(ThumbnailActivity.this, "GetThumbnail elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();
                            Log.d(TAG,"GetThumbnail elapsed = "+watch.toString());
                            pg.setProgress(index);

                            if( at_enable ) {
                                try {
                                    at_blockingQueue.put("Complated!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                            Toast.makeText(getApplicationContext(), "getVideoClipDetailThumbnails fail!", Toast.LENGTH_LONG).show();

                            if( at_enable ) {
                                try {
                                    at_blockingQueue.put("fail!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                            Toast.makeText(getApplicationContext(), "system error", Toast.LENGTH_LONG).show();

                            if( at_enable ) {
                                try {
                                    at_blockingQueue.put("system error!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    }
                });
            }
        });

        // set screen timeout to never
        mListFilePath = getIntent().getStringArrayListExtra("filelist");
        if( mListFilePath != null ) {

            if (mListFilePath.size() == 0) {
                Toast.makeText(getApplicationContext(), "select content. retry again!!", Toast.LENGTH_SHORT).show();
                finish();
            }

            mClip = nexClip.getSupportedClip(mListFilePath.get(0), true);
            if (mClip == null) {
                finish();
                Toast.makeText(getApplicationContext(), "Not Supported clip.", Toast.LENGTH_LONG).show();
                return;
            }

            mSeekbarStartTime.setMax(mClip.getTotalTime());
            mSeekbarEndTime.setMax(mClip.getTotalTime());

            mGridAdapterSeekTable.setSeekTable(mClip.getSeekPointsSync());
            mGridAdapterSeekTable.notifyDataSetChanged();
            return;
        }

        at_test_root = getIntent().getStringExtra("test_root");
        at_result_root = getIntent().getStringExtra("result_root");

        at_test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + at_test_root + "thumbnail/";
        at_result_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + at_result_root + "thumbnail/";

        at_message_handler = new Handler(){
            @Override
            public void handleMessage(Message msg){
                if(msg.what==1){
                    mSpinnerResolution.setSelection(0);
                    mSpinnerAngle.setSelection(0);
                    mSeekbarStartTime.setProgress(0);
                    mSeekbarEndTime.setProgress(mClip.getTotalTime());
                    mEdittextCount.setText("15");

                    mGridAdapterSeekTable.notifyDataSetChanged();

                    mGetThumbnail.performClick();
                }else if(msg.what==2){
                    finish();
                }
            }
        };

        ThumbnailAuto thumbnailAuto = new ThumbnailAuto();
        thumbnailAuto.start();

    }

    public Bitmap yOnly2bitmap(byte[] data, int width, int height) {
        byte[] yuv = new byte[data.length*3/2];
        for( int i = 0 ; i < data.length ; i++ ){
            yuv[i] = data[i];
        }

        for( int i = 0 ; i < data.length/2 ; i++ ){
            yuv[data.length+i] = (byte)0x80;
        }
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        YuvImage yuvImage = new YuvImage(yuv, ImageFormat.NV21, width, height,
                null);
        yuvImage.compressToJpeg(new android.graphics.Rect(0, 0, width, height),
                100, out);
        byte[] imageBytes = out.toByteArray();
        Bitmap bitmap = BitmapFactory.decodeByteArray(imageBytes, 0,
                imageBytes.length);

        return bitmap;
    }

    public class AdaptorSeekTable extends BaseAdapter {
        class Item{
            int time;
            boolean sel;
        }
        ArrayList<Item> tables = new ArrayList<>();

        public AdaptorSeekTable(int [] tables) {
            setSeekTable(tables);
        }

        public void setSeekTable(int [] tables) {
            this.tables.clear();
            if( tables == null ) return;
            for(int i : tables) {
                Item item = new Item();
                item.time = i;
                item.sel = false;
                this.tables.add(item);
            }
        }

        public int [] getSeekTable() {
            int i = 0;
            for(Item item : tables ) {
                if( item.sel ) i++;
            }

            if( i > 0 ) {
                int idx = 0;
                int [] t = new int[i];
                for(Item item : tables ) {
                    if( item.sel ) {
                        t[idx] = item.time;
                        idx++;
                    }
                }
                return t;
            }
            return null;
        }

        public void randomSel() {
            int i = 0;
            for(Item item : tables ) {
                if( i % 3 == 0 )
                    item.sel = true;
                else
                    item.sel = false;
                i++;
            }
        }

        @Override
        public int getCount() {
            return tables == null ? 0 : tables.size();
        }

        @Override
        public Object getItem(int arg0) {
            return tables == null ? null : tables.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            if( currentView == null ) {
                LayoutInflater inflater = ThumbnailActivity.this.getLayoutInflater();
                currentView = inflater.inflate(R.layout.griditem_seektable, null, true);
            }

            LinearLayout background = (LinearLayout)currentView.findViewById(R.id.ll_seekitem);
            TextView seek_pos = (TextView)currentView.findViewById(R.id.tv_time);

            Item item = (Item) getItem(arg0);
            seek_pos.setText(""+item.time);

            if( item.sel ) {
                background.setBackgroundColor(0x3303A9F4);
            }
            else {
                background.setBackgroundColor(0x00000000);
            }

            return currentView;
        }
    }

    public class AdaptorThumbnail extends BaseAdapter {
        class Item {
            String text;
            Bitmap thumb;
        }

        private ArrayList<Item> thumbs = new ArrayList<>();

        public AdaptorThumbnail() {
        }

        public void addItem(String text, Bitmap bitmap) {
            Item item = new Item();
            item.text = text;
            item.thumb = bitmap;

            thumbs.add(item);
        }

        public void clearItem() {
            thumbs.clear();
        }

        public int getCount() {
            return thumbs == null ? 0 : thumbs.size();
        }

        @Override
        public Object getItem(int arg0) {
            return thumbs == null ? null : thumbs.get(arg0);
        }

        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            if( currentView == null ) {
                LayoutInflater inflater = ThumbnailActivity.this.getLayoutInflater();
                currentView = inflater.inflate(R.layout.linear_clip_thumbnails, null, true);
            }

            ImageView iv = (ImageView)currentView.findViewById(R.id.imageview_clipthumb_image);
            TextView tv = (TextView) currentView.findViewById(R.id.textview_clipthumb_time);

            Item item = (Item)getItem(arg0);

            iv.setImageBitmap(item.thumb);
            tv.setText(item.text);

            return currentView;
        }
    }

    class ThumbnailAuto extends Thread {
        public void run() {

            at_enable = true;
            File dir = new File(at_test_path);
            if (dir.isDirectory() == false) {
                Log.e(TAG, "Thumbnail auto test failed with wrong test path : " + at_test_path);
                return;
            }

            File dir_result = new File(at_result_path);
            if (dir_result.exists() == false) {
                dir_result.mkdirs();
            }

            at_sb.append("Thumbnail auto test start\n");
            at_sb.append("\n-----------------------------------------------------------------\n");

            for (File f : dir.listFiles()) {
                if (f.isDirectory() && f.getName().equals("content")) {
                    Log.e(TAG, "Thumbnail auto test start");

                    at_sb.append(AutoTestUtils.checkMemoryStatus());
                    at_sb.append("\n-----------------------------------------------------------------\n");

                    for (File tf : f.listFiles()) {
                        if (tf.isDirectory()) continue;

                        at_testFile = tf.getName();
                        nexClip clip = nexClip.getSupportedClip(tf.getAbsolutePath());
                        if (clip == null) {
                            at_sb.append("Test clip: " + tf.getAbsolutePath() + " do not support\n");
                            at_sb.append("\n-----------------------------------------------------------------\n");
                            continue;
                        }
                        at_sb.append("Test clip: " + tf.getAbsolutePath() + " is checking\n");

                        mClip = clip;

                        mGridAdapterSeekTable.setSeekTable(mClip.getSeekPointsSync());
                        mGridAdapterSeekTable.randomSel();

                        at_elapsed.reset();
                        at_elapsed.start();

                        at_message_handler.sendEmptyMessage(1);

                        try {
                            String result = (String) at_blockingQueue.take();
                            at_elapsed.stop();
                            at_sb.append("Test clip checking End with " + result + " (Elapsed time : " + at_elapsed.toString() + ")\n");
                            at_sb.append("\n-----------------------------------------------------------------\n");
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }

                    System.gc();
                    at_sb.append(AutoTestUtils.checkMemoryStatus());
                    at_sb.append("\n-----------------------------------------------------------------\n");

                    String thumb_test_path = at_result_path + "/Thumbnail_test_log.txt";
                    AutoTestUtils.savelog(at_sb.toString(), thumb_test_path);
                }
            }
            at_enable = false;

            AutoTestUtils.deleteFlagFile(Environment.getExternalStorageDirectory().getAbsoluteFile() + at_test_root);

            at_message_handler.sendEmptyMessage(2);
        }
    }
}
