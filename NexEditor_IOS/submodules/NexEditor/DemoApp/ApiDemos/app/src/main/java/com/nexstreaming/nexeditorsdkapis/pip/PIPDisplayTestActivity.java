/******************************************************************************
 * File Name        : PIPDisplayTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.pip;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Spinner;
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
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class PIPDisplayTestActivity extends Activity {
    public static final String TAG = "PIPDisplayTest";
    public static final int MAX_SELECTED_PIP_COUNT = 2;

    private ArrayList<PIPListItem> mSelectedPIP;
    private Button mButtonAdd;
    private Button mButtonPlay;
    private Button mButtonExport;


    private ProgressBar mProgressBar;

    private LinearLayout mLinearLayoutGetPIPInfo;
    private TextView mSelectedListItem;
    private EditText mEditTextX;
    private EditText mEditTextY;
    private Spinner mSpinnerAlpha;
    private Spinner mSpinnerRotate;
    private Spinner mSpinnerMask;
    private Button mOkButton;
    private SeekBar mSeekBarStartTrimTime;
    private SeekBar mSeekBarEndTrimTime;


    int mSelectedItemPosition = 0;
    int mSelectedItemX = 0;
    int mSelectedItemY = 0;
    float mSelectedItemAlpha = 0.25f;
    int mSelectedItemRotate = 0;
    int mSelectedItemMask = 0;
    int mStartTrimTime = 0;
    int mEndTrimTime = 0;

    private ListView mListViewSelctedPIP;
    private ArrayAdapter<String> mListAdapterSelectedPIP;
    private ArrayList<String> mListFilePath;

    private nexEngine mEngine;
    private nexProject mProject;
    private nexEngineView mEngineView;

    private enum State {
        IDLE,
        STOP,
        PLAY
    }

    private State mState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_pipdisplay_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = new ArrayList<String>();

        Intent intent = getIntent();
        mListFilePath = intent.getStringArrayListExtra("filelist");

        nexClip clip = nexClip.getSupportedClip(mListFilePath.get(0));

        if( clip == null ){
            finish();
            return;
        }

        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
            clip.setRotateDegree(clip.getRotateInMeta());
        }
        mProgressBar = (ProgressBar) findViewById(R.id.progress_pip_display_test);
        mProgressBar.setProgress(0);

        mSelectedPIP = new ArrayList<PIPListItem>();

        mListViewSelctedPIP = (ListView) findViewById(R.id.listview_pip_display_test);
        mListAdapterSelectedPIP = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_list_item_1);
        mListViewSelctedPIP.setAdapter(mListAdapterSelectedPIP);
        mListViewSelctedPIP.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Log.d(TAG, "mListViewSelctedPIP click -> :" + position);

                mSelectedItemPosition = position;

                mSelectedListItem.setText("SelectedListItem: " + ((PIPListItem) mSelectedPIP.get(position)).getFilePath());
                nexOverlayItem item = mProject.getOverlay(mSelectedPIP.get(position).getId());

                if( item == null ){
                    return;
                }


                mEditTextX.setText("" + item.getPositionX());
                mEditTextY.setText("" + item.getPositionY());

                mSeekBarStartTrimTime.setMax(mSelectedPIP.get(position).getTotalTime());
                mSeekBarEndTrimTime.setMax(mSelectedPIP.get(position).getTotalTime());
                mSeekBarStartTrimTime.setProgress(item.getStartTrimTime());
                mSeekBarEndTrimTime.setProgress(mSelectedPIP.get(position).getTotalTime() - item.getEndTrimTime());
                mStartTrimTime = item.getStartTrimTime();
                mEndTrimTime = mSelectedPIP.get(position).getTotalTime()- item.getEndTrimTime();

                float alpha = item.getAlpha();
                if( alpha < 0.25 ){
                    mSpinnerAlpha.setSelection(0);
                }else if( alpha < 0.75){
                    mSpinnerAlpha.setSelection(1);
                }else{
                    mSpinnerAlpha.setSelection(2);
                }

                int rotate = item.getRotate();
                if( rotate == 0 ){
                    mSpinnerRotate.setSelection(0);
                }else if( rotate == 90) {
                    mSpinnerRotate.setSelection(1);
                }else if( rotate == 180){
                    mSpinnerRotate.setSelection(2);
                }else{
                    mSpinnerRotate.setSelection(3);
                }

                int mode = item.getMask().getSplitMode();
                mSpinnerMask.setSelection(mode);

                mLinearLayoutGetPIPInfo.setVisibility(View.VISIBLE);

            }
        });

        mButtonAdd = (Button) findViewById(R.id.button_pip_display_test_add);
        mButtonAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
                    Toast.makeText(getApplicationContext(),"API Level is not supported!",Toast.LENGTH_SHORT).show();
                    return;
                }

                if (mState == State.PLAY) {
                    mProgressBar.setProgress(0);
                    mState = State.STOP;
                    mEngine.stop();
                }

                Intent intent = new Intent();
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);

                startActivityForResult(intent, 100);
            }
        });

        mButtonPlay = (Button) findViewById(R.id.button_pip_display_test_play);
        mButtonPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonPlay click ->");
                if(mState == State.PLAY) {
                    mEngine.stop();
                    mButtonPlay.setText("Play");
                    mState = State.STOP;
                } else {
                    mState = State.PLAY;
                    mProgressBar.setProgress(0);
                    mProgressBar.setMax(mProject.getTotalTime() / 33);
                    mEngine.play();
                    mButtonPlay.setText("Stop");

                }
            }
        });

        mButtonExport = (Button) findViewById(R.id.button_pip_display_test_export);
        mButtonExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonExport -> ");
                mEngine.stop();
                mState = State.STOP;
                mButtonPlay.setText("Play");
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                startActivity(intent);
            }
        });

        mLinearLayoutGetPIPInfo = (LinearLayout) findViewById(R.id.linearlayout_pip_display_test_get_pip_info);
        mLinearLayoutGetPIPInfo.setVisibility(View.INVISIBLE);

        mSelectedListItem = (TextView) findViewById(R.id.textview_pip_display_test_selected_listitem);

        mEditTextX = (EditText) findViewById(R.id.edittext_pip_display_test_x);
        mEditTextY = (EditText) findViewById(R.id.edittext_pip_display_test_y);


        mSeekBarStartTrimTime = (SeekBar) findViewById(R.id.seekbar_pip_display_test_start_time);
        // if list item is clicked, this property will be update to totalPlayTime of item
        mSeekBarStartTrimTime.setMax(1);
        mSeekBarStartTrimTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                ((TextView) findViewById(R.id.textview_pip_display_test_start_time)).setText(value + "");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mStartTrimTime = value;
                ((TextView) findViewById(R.id.textview_pip_display_test_start_time)).setText(value + "");
            }
        });

        mSeekBarEndTrimTime = (SeekBar) findViewById(R.id.seekbar_pip_display_test_end_time);
        // if list item is clicked, this property will be update to totalPlayTime of item
        mSeekBarEndTrimTime.setMax(1);
        mSeekBarEndTrimTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                ((TextView) findViewById(R.id.textview_pip_display_test_end_time)).setText(value + "");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mEndTrimTime = value;
                ((TextView) findViewById(R.id.textview_pip_display_test_end_time)).setText(value + "");
            }
        });

        mOkButton = (Button) findViewById(R.id.button_pip_display_test_ok);
        mOkButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                try {

                    mSelectedItemX = Integer.parseInt(mEditTextX.getText() + "");
                    mSelectedItemY = Integer.parseInt(mEditTextY.getText() + "");

                } catch(NumberFormatException e) {
                    Log.e(TAG, e.toString());
                    return;
                }

                if( mEndTrimTime <=  mStartTrimTime ) {
                    Toast.makeText(getApplicationContext(), "startTime >= endTime, retry again!!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                Log.d(TAG, "========= Selected PIP INFO ============");
                Log.d(TAG, "mEngineView Width: " + mEngineView.getWidth() + ", Height: " + mEngineView.getHeight());
                Log.d(TAG, "X : " + mSelectedItemX);
                Log.d(TAG, "Y : " + mSelectedItemY);
                Log.d(TAG, "Alpha : " + mSelectedItemAlpha);
                Log.d(TAG, "Rotate : " + mSelectedItemRotate);
                Log.d(TAG, "StartTimeTime: " + mStartTrimTime);
                Log.d(TAG, "EndTimeTime: " + mEndTrimTime);
                Log.d(TAG, "Mask type: " + mSelectedItemMask);
                Log.d(TAG, "=====================================");

                int selectedPIPId = mSelectedPIP.get(mSelectedItemPosition).getId();
                if(mProject.getOverlay(selectedPIPId) == null)
                    return ;
                int w = mSelectedPIP.get(mSelectedItemPosition).getWidth();
                int h = mSelectedPIP.get(mSelectedItemPosition).getHeight();
                mProject.getOverlay(selectedPIPId).setPosition(mSelectedItemX, mSelectedItemY);
                mProject.getOverlay(selectedPIPId).setAlpha(mSelectedItemAlpha);
                mProject.getOverlay(selectedPIPId).setRotate(mSelectedItemRotate);
                mProject.getOverlay(selectedPIPId).setTrim(mStartTrimTime, mEndTrimTime);

                if( mSelectedItemMask == 0 ) {
                    mProject.getOverlay(selectedPIPId).getMask().setState(false);
                    mProject.getOverlay(selectedPIPId).setScale(1280f / (float) w * 0.5f, 720f / (float) h * 0.5f);
                }else{
                    mProject.getOverlay(selectedPIPId).getMask().setState(true);
                    mProject.getOverlay(selectedPIPId).getMask().setSplitMode(mSelectedItemMask, false);
                    mProject.getOverlay(selectedPIPId).setScale(1280f / (float)w  , 720f / (float)h );
                }
                //mEngine.updateProject();
            }
        });


        String alphaItems[] = {"25", "75", "100"};
        ArrayAdapter<String> arrayAlphaAdapter = new ArrayAdapter(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, alphaItems);
        mSpinnerAlpha = (Spinner) findViewById(R.id.spinner_pip_display_test_alpha);
        mSpinnerAlpha.setAdapter(arrayAlphaAdapter);
        mSpinnerAlpha.setSelection(0);
        mSpinnerAlpha.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if(position == 0) {
                    mSelectedItemAlpha = 0.25f;
                } else if(position == 1) {
                    mSelectedItemAlpha = 0.75f;
                } else if(position == 2) {
                    mSelectedItemAlpha = 1f;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        String rotateItems[] = {"0", "90", "180", "270"};
        ArrayAdapter<String> arrayRotateAdapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, rotateItems);
        mSpinnerRotate = (Spinner) findViewById(R.id.spinner_pip_display_test_rotate);
        mSpinnerRotate.setAdapter(arrayRotateAdapter);
        mSpinnerRotate.setSelection(0);
        mSpinnerRotate.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if(position == 0) {
                    mSelectedItemRotate = 0;
                } else if(position == 1) {
                    mSelectedItemRotate = 90;
                } else if(position == 2) {
                    mSelectedItemRotate = 180;
                } else if(position == 3) {
                    mSelectedItemRotate = 270;
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        String maskItems[] = {"none", "left", "right", "top","bottom"};
        ArrayAdapter<String> arrayMaskAdapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, maskItems);
        mSpinnerMask = (Spinner) findViewById(R.id.spinner_pip_display_test_mask);
        mSpinnerMask.setAdapter(arrayMaskAdapter);
        mSpinnerMask.setSelection(0);
        mSpinnerMask.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedItemMask = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mProject = new nexProject();
        mProject.add(clip);
        mEngineView = (nexEngineView) findViewById(R.id.engineview_pip_display_test);
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setView(mEngineView);
        mEngine.setProject(mProject);
        mEngine.updateProject();
        mEngine.setEventHandler(mEngineListener);
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            if(mSelectedPIP.size() >= MAX_SELECTED_PIP_COUNT) {
                Toast.makeText(getApplicationContext(), "selected list size > 2, so fail add video to list", Toast.LENGTH_SHORT).show();
                return ;
            }

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                nexClip clip = nexClip.getSupportedClip(filepath);

                int clipTotalTime = 0;
                if(clip != null) {
                    if(clip.getTotalTime() < mProject.getClip(0, true).getTotalTime())
                        clipTotalTime = clip.getTotalTime();
                    else
                        clipTotalTime = mProject.getClip(0, true).getTotalTime();
                }

                if( clip != null ) {
                    int id = 0;

                    mListAdapterSelectedPIP.add(filepath + "(" + clipTotalTime + ")");
                    id = mSelectedPIP.size();
                    nexOverlayImage ov = new nexOverlayImage("video"+id,clip);
                    int x = 320 + 640 * id;
                    int y = 180;
                    nexOverlayItem ovitem = new nexOverlayItem(ov,x,y,0, clipTotalTime);
                    ovitem.setScale(1280f / (float) clip.getWidth() * 0.5f, 720f / (float) clip.getHeight() * 0.5f);
                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        ovitem.setRotate(90);
                    }

                    if(clip.getRotateInMeta() == 90) {
                        ovitem.setRotate(270);
                    }

                    mSelectedPIP.add(new PIPListItem(filepath, ovitem.getId(), clip.getWidth(), clip.getHeight(), clipTotalTime));
                    mProject.addOverlay(ovitem);
                }
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    nexClip clip = nexClip.getSupportedClip(filepath);
                    if( clip != null && clip.getTotalTime() < mProject.getClip(0, true).getTotalTime() ) {
                        int id = 0;

                        mListAdapterSelectedPIP.add(filepath);
                        id = mSelectedPIP.size();
                        nexOverlayImage ov = new nexOverlayImage("video"+id,clip);
                        int x = 320 + 640 * id;
                        int y = 180;
                        nexOverlayItem ovitem = new nexOverlayItem(ov,x,y,0, clip.getTotalTime());
                        ovitem.setScale(1280f / (float) clip.getWidth() * 0.5f, 720f / (float) clip.getHeight() * 0.5f);
                        if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                            ovitem.setRotate(90);
                        }

                        if(clip.getRotateInMeta() == 90) {
                            ovitem.setRotate(270);
                        }
                        mSelectedPIP.add(new PIPListItem(filepath, ovitem.getId(), clip.getWidth(), clip.getHeight(), clip.getTotalTime()));
                        mProject.addOverlay(ovitem);
                    }
                    else {
                        Toast.makeText(getApplicationContext(), "add pip error", Toast.LENGTH_SHORT).show();
                    }
                }
            }

            mEngine.updateProject();
        }
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart");
        if(mState == State.STOP)
            mEngine.setEventHandler(mEngineListener);
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }


    /**-----------------------------*
     *         Event Listener       *
     * ----------------------------**/
    nexEngineListener mEngineListener = new nexEngineListener() {
        @Override
        public void onStateChange(int oldState, int newState) {
            Log.i(TAG, "onStateChange -> oldState=" + oldState + ", newState=" + newState);
            if(newState == 1) {
                mState = State.STOP;
                mProgressBar.setProgress(0);
                mButtonPlay.setText("Play");
            } else if(newState == 2) {
                mState = State.PLAY;
            }
        }

        @Override
        public void onTimeChange(int currentTime) {
            //Log.i(TAG, "onTimeChange: currentTime: " + currentTime + "mState: " + mState);
            if(mState == State.PLAY)
                mProgressBar.setProgress(currentTime / 33);
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
            Log.d(TAG, "onEncodingDone -> iserror: " + iserror + " result: " + result);
        }

        @Override
        public void onPlayEnd() {
            mProgressBar.setProgress(0);
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
            mProgressBar.setProgress(percent);
            Log.d(TAG, "progress value: " + mProgressBar.getProgress() + " max: " + mProgressBar.getMax());
            if(percent == 100)
                Toast.makeText(getApplicationContext(), "export done!!", Toast.LENGTH_SHORT).show();
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

    private File getExportFile(int wid , int hei) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "PIP_"+wid+"X"+hei +"_"+ export_time+".mp4");
        return exportFile;
    }

    /**-----------------------------*
     *         Inner Clasee         *
     * ----------------------------**/
    private class PIPListItem {
        String filepath;
        int width;
        int height;
        int id;
        int totalTime;

        public PIPListItem(String f, int i, int w, int h, int t) {
            filepath = f;
            id = i;
            width = w;
            height = h;
            totalTime = t;
        }

        public String getFilePath() {
            return filepath;
        }

        public int getId() {
            return id;
        }

        public int getWidth() {
            return width;
        }

        public int getHeight() {
            return height;
        }

        public int getTotalTime() {
            return totalTime;
        }
    }
}
