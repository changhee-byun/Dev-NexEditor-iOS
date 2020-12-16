package com.nexstreaming.nexeditorsdkapis.edit;

import android.app.Activity;

import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.common.RangeSeekBar;
import com.nexstreaming.nexeditorsdkapis.effect.EffectPreviewActivity;
import com.nexstreaming.nexeditorsdkapis.export.ExportActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class ProjectEdit2Activity extends Activity implements nexEngineListener, nexEngineView.NexViewListener{
    private static final String TAG = "ProjectEditor2";

    private SeekBar mSeekBarPlayTime;
    private Button mPlayStop;
    private Button mExport;
    private Button mBGM;
    private TextView mPlayTime;
    private TextView mEndTime;
    private LinearLayout mGallery;
    private LayoutInflater mInflater;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private boolean trackingSeekbarTouch = false;
    private int trackingSeektime = -1;
    private boolean seekInProgress = false;
    private boolean lastUIisPlaying = false;
    private int playTime = 0;
    private int mSeekMode = 1; /*1: Seek, 2: Seek IDR*/
    private int mSeekCurrentTS;
    private int mSeekms = 0;
    private nexProject project;
    private LinearLayout mVideoLayout;
    private LinearLayout mImageLayout;
    private LinearLayout mClipEditLayout;
    private LinearLayout mBGMEditLayout;
    private TextView mClipPath;
    private RangeSeekBar mVideoTrimSeekBar;
    private RangeSeekBar mBGMTrimSeekBar;
    private RangeSeekBar mBGMFadeInOutSeekBar;
    private SeekBar mBGMStartTimeSeekBar;
    private SeekBar mBGMVolumeSeekBar;
    private Switch mBGMLoop;
    private TextView mBGMStartTimeTextView;
    private TextView mBGMVolumeTextView;

    private SeekBar mVideoSpeedSeekBar;
    private TextView mVideoSpeedText;
    private SeekBar mImageDurationSeekBar;
    private TextView mImageDurationText;
    private Spinner mRotateSpinner;
    private Spinner mColorEffectSpinner;
    private static final int[] s_speedTab ={3,6,13,25,50,75,100,125,150,175,200,400};
    private static final Integer[] s_rotateTab = new Integer[]{0,90,180,270};
    private nexClip mSelectedClip ;
    private SeekBar seekBar_Bri;
    private TextView textView_Bri;

    private SeekBar seekBar_Sat;
    private TextView textView_Sat;

    private SeekBar seekBar_Con;
    private TextView textView_Con;
    private String lastSavedData;
    private int bgmStartTime = 0;
    private int bgmTrimStartTime = 0;
    private int bgmTrimEndTime = 0;

    private void createBGMEdit(){
        mBGMEditLayout = (LinearLayout) findViewById(R.id.layout_project2_bgmedit);
        mBGMTrimSeekBar = (RangeSeekBar) findViewById(R.id.rangeseekbar_project_edit2_bgm_trim);
        mBGMFadeInOutSeekBar = (RangeSeekBar) findViewById(R.id.rangeseekbar_project_edit2_bgm_fadeinout);
        mBGMStartTimeSeekBar = (SeekBar)findViewById(R.id.seekbar_project_edit2_bgm_starttime);
        mBGMVolumeSeekBar = (SeekBar)findViewById(R.id.seekbar_project_edit2_bgm_volume);
        mBGMLoop = (Switch)findViewById(R.id.switch_project_edit2_bgm_loop);
        mBGMStartTimeTextView = (TextView)findViewById(R.id.textview_project_edit2_bgm_starttime);
        mBGMVolumeTextView = (TextView)findViewById(R.id.textview_project_edit2_bgm_volume);
        final Button btSet = (Button)findViewById(R.id.button_project_edit2_bgm_set);
        btSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent( );
                intent.setType("audio/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 400);

            }
        });

        mBGMStartTimeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mBGMStartTimeTextView.setText(""+progress*33);
                value = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                bgmStartTime = value * 33;
                project.setBackgroundConfig(bgmStartTime,false,mBGMLoop.isChecked());
            }
        });

        mBGMLoop.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                project.setBackgroundConfig(bgmStartTime,false,isChecked);
            }
        });

        mBGMVolumeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 100;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mBGMVolumeTextView.setText(""+progress);
                value = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                project.setProjectVolume(value);
            }
        });
        mBGMVolumeSeekBar.setMax(200);
        mBGMVolumeSeekBar.setProgress(100);

        mBGMTrimSeekBar.setOnRangeSeekBarChangeListener(new RangeSeekBar.OnRangeSeekBarChangeListener() {
            @Override
            public void onRangeSeekBarValuesChanged(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {

            }

            @Override
            public void onRangeSeekBarStop(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {
                project.setBackgroundTrim((int)minValue,(int)maxValue);
            }
        });
        mBGMFadeInOutSeekBar.setRangeValues(0, 10000);
        mBGMFadeInOutSeekBar.setOnRangeSeekBarChangeListener(new RangeSeekBar.OnRangeSeekBarChangeListener() {
            @Override
            public void onRangeSeekBarValuesChanged(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {

            }

            @Override
            public void onRangeSeekBarStop(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {
                if( isMin )
                    project.setProjectAudioFadeInTime((int)minValue);
                else
                    project.setProjectAudioFadeOutTime(10000-(int)maxValue);
            }
        });
        mBGMEditLayout.setVisibility(View.GONE);
    }

    private void enableBGMSetting(){
        mBGMEditLayout.setVisibility(View.VISIBLE);
        mClipEditLayout.setVisibility(View.GONE);
        nexClip clip = project.getBackgroundMusic();
        if( clip == null ){
            mBGMStartTimeSeekBar.setEnabled(false);
            mBGMFadeInOutSeekBar.setEnabled(false);
            mBGMTrimSeekBar.setEnabled(false);
            mBGMVolumeSeekBar.setEnabled(false);
            mBGMLoop.setEnabled(false);
            return;
        }else{
            mBGMStartTimeSeekBar.setEnabled(true);
            mBGMFadeInOutSeekBar.setEnabled(true);
            mBGMTrimSeekBar.setEnabled(true);
            mBGMVolumeSeekBar.setEnabled(true);
            mBGMLoop.setEnabled(true);
        }
        mBGMStartTimeSeekBar.setMax(project.getTotalTime()/33);
        mBGMStartTimeSeekBar.setProgress(bgmStartTime/33);
        mBGMTrimSeekBar.setRangeValues(0 , clip.getTotalTime() );
        mBGMTrimSeekBar.setSelectedMinValue(bgmTrimStartTime);
        mBGMTrimSeekBar.setSelectedMinValue(bgmTrimEndTime);
        mBGMVolumeSeekBar.setProgress(project.getProjectVolume());
        mBGMFadeInOutSeekBar.setSelectedMinValue(project.getProjectAudioFadeInTime());
        mBGMFadeInOutSeekBar.setSelectedMaxValue(10000-project.getProjectAudioFadeOutTime());
    }

    private void createClipEdit(){
        mClipEditLayout = (LinearLayout) findViewById(R.id.layout_project2_clipedit);
        mClipPath = (TextView) findViewById(R.id.textView_project2_clipname);

        mVideoLayout = (LinearLayout) findViewById(R.id.layout_project2_video);

        mVideoTrimSeekBar = (RangeSeekBar)findViewById(R.id.rangeseekbar_project_edit2);
        mVideoTrimSeekBar.setOnRangeSeekBarChangeListener(new RangeSeekBar.OnRangeSeekBarChangeListener() {
            @Override
            public void onRangeSeekBarValuesChanged(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {

            }

            @Override
            public void onRangeSeekBarStop(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {
                mSelectedClip.getVideoClipEdit().setTrim((int)minValue,(int)maxValue);
                project.updateProject();
                mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
                mEndTime.setText(getTimeString(project.getTotalTime()));
            }
        });

        mVideoSpeedText = (TextView)  findViewById(R.id.textview_project_edit2_speed);

        mVideoSpeedSeekBar = (SeekBar) findViewById(R.id.seekbar_project_edit2_speed);
        mVideoSpeedSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int speed;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                speed = s_speedTab[progress];
                mVideoSpeedText.setText(""+speed);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if( mSelectedClip != null )
                    mSelectedClip.getVideoClipEdit().setSpeedControl(speed);
                project.updateProject();
                mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
                mEndTime.setText(getTimeString(project.getTotalTime()));
            }
        });
        mVideoSpeedSeekBar.setMax(s_speedTab.length);
        mVideoSpeedSeekBar.setProgress(getIndexSpeed(100));

        mImageLayout = (LinearLayout) findViewById(R.id.layout_project2_image);

        mImageDurationText = (TextView)  findViewById(R.id.textview_project2_image_duration);
        mImageDurationSeekBar = (SeekBar) findViewById(R.id.seekbar_project2_image_duration);
        mImageDurationSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int duration;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mImageDurationText.setText(""+progress);
                duration = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int numFace = 0;
                if( mSelectedClip != null ){
                    mSelectedClip.setImageClipDuration(duration);
                    numFace = mSelectedClip.getCrop().faceStartEndPosition(duration);
                }
                project.updateProject();
                mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
                mEndTime.setText(numFace + "," + getTimeString(project.getTotalTime()));
            }
        });

        mClipEditLayout.setVisibility(View.INVISIBLE);
        mVideoLayout.setVisibility(View.GONE);
        mImageLayout.setVisibility(View.GONE);

        mRotateSpinner = (Spinner)  findViewById(R.id.spinner_project_edit2_rotate);
        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, s_rotateTab);
        mRotateSpinner.setAdapter(adapter);
        mRotateSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if( mSelectedClip != null ) {
                    mSelectedClip.setRotateDegree(s_rotateTab[position]);
                    mEngine.updateProject();
                    moveClipTime();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        List<String> colorEffectItems = new ArrayList<String>();

        List<nexColorEffect> list = nexColorEffect.getPresetList();
        for( nexColorEffect ce: list ){
            colorEffectItems.add(ce.getPresetName());
        }

        mColorEffectSpinner = (Spinner)  findViewById(R.id.spinner_project_edit2_coloreffect);
        ArrayAdapter<String> adapterColor = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, colorEffectItems);
        mColorEffectSpinner.setAdapter(adapterColor);
        mColorEffectSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if( mSelectedClip != null ) {
                    mSelectedClip.setColorEffect(nexColorEffect.getPresetList().get(position));
                    mEngine.updateProject();
                    nexColorEffect.cleanLutCache();
                    moveClipTime();
                }
                //mEngine.seek(2000);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        seekBar_Bri = (SeekBar) findViewById(R.id.seekbar_project_edit2_adjust_bri);
        textView_Bri = (TextView) findViewById(R.id.textview_project_edit2_adjust_bri);
        seekBar_Bri.setMax(510);
        seekBar_Bri.setProgress(255);

        seekBar_Bri.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress-255;
                textView_Bri.setText(""+ value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.brightness,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Bri.setText(""+ value);
                if( mSelectedClip != null ) {
                    mSelectedClip.setBrightness(value);
                    mEngine.updateProject();
                    moveClipTime();
                }
            }
        });

        seekBar_Sat = (SeekBar) findViewById(R.id.seekbar_project_edit2_adjust_sat);
        textView_Sat = (TextView) findViewById(R.id.textview_project_edit2_adjust_sat);
        seekBar_Sat.setMax(510);
        seekBar_Sat.setProgress(255);
        seekBar_Sat.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress-255;
                textView_Sat.setText(""+ value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.saturation,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Sat.setText(""+ value);
                if( mSelectedClip != null ) {
                    mSelectedClip.setSaturation(value);
                    mEngine.updateProject();
                    moveClipTime();
                }
            }
        });


        seekBar_Con = (SeekBar) findViewById(R.id.seekbar_project_edit2_adjust_con);
        textView_Con = (TextView) findViewById(R.id.textview_project_edit2_adjust_con);
        seekBar_Con.setMax(510);
        seekBar_Con.setProgress(255);
        seekBar_Con.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress-255;
                textView_Con.setText(""+ value);
                mEngine.fastPreview(nexEngine.FastPreviewOption.contrast,value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textView_Con.setText(""+ value);
                if( mSelectedClip != null ) {
                    mSelectedClip.setContrast(value);
                    mEngine.updateProject();
                    moveClipTime();
                }
            }
        });

        final Button btEffect = (Button)findViewById(R.id.button_project_edit2_effect);
        btEffect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mSelectedClip != null ){
                    Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
                    intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_CLIP);
                    startActivityForResult(intent, 200);
                }
            }
        });

        final Button btTransition = (Button)findViewById(R.id.button_project_edit2_trasition);
        btTransition.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mSelectedClip != null ){
                    Intent intent = new Intent(getBaseContext(), EffectPreviewActivity.class);
                    intent.putExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_SELECT_TRANSITION);
                    startActivityForResult(intent, 300);
                }
            }
        });

        final  Button btDelete = (Button)findViewById(R.id.button_project_edit2_delete);
        btDelete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                deleteClip(mSelectedClip);
                mSelectedClip = null;
                setEditMode(null);
                validateButtons();
            }
        });
    }

    private static int getIndexSpeed(int value){
        int reval = 0;
        for( int i = 0 ; i < s_speedTab.length ; i++ ){
            if( s_speedTab[i] >= value ){
                reval = i;
                break;
            }
        }
        return reval;
    }

    private void setEditMode(nexClip clip){
        mSelectedClip = clip;
        if( mSelectedClip == null ){
            mBGMEditLayout.setVisibility(View.INVISIBLE);
            mClipEditLayout.setVisibility(View.INVISIBLE);
            return;
        }
        mBGMEditLayout.setVisibility(View.GONE);
        mClipEditLayout.setVisibility(View.VISIBLE);
        mClipPath.setText(clip.getPath());
        if( mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            mVideoLayout.setVisibility(View.VISIBLE);
            mImageLayout.setVisibility(View.GONE);

            mVideoTrimSeekBar.setRangeValues(0,mSelectedClip.getTotalTime());
            mVideoTrimSeekBar.setSelectedMinValue(mSelectedClip.getVideoClipEdit().getStartTrimTime());
            mVideoTrimSeekBar.setSelectedMaxValue(mSelectedClip.getVideoClipEdit().getEndTrimTime() == 0 ? mSelectedClip.getTotalTime() : mSelectedClip.getVideoClipEdit().getEndTrimTime() );
            mVideoSpeedSeekBar.setProgress(getIndexSpeed(mSelectedClip.getVideoClipEdit().getSpeedControl()));
        }else if( mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ){
            mVideoLayout.setVisibility(View.GONE);
            mImageLayout.setVisibility(View.VISIBLE);
            mImageDurationSeekBar.setMax(10000);
            mImageDurationSeekBar.setProgress(mSelectedClip.getImageClipDuration());
        }
        int rotateIndex = 0;
        for(int i = 0 ; i < s_rotateTab.length ; i++ ){
            if( s_rotateTab[i] == mSelectedClip.getRotateDegree() ){
                rotateIndex = i;
                break;
            }
        }
        mRotateSpinner.setSelection(rotateIndex);
        int colorIndex = 0;
        boolean find = false;
        for( nexColorEffect c : nexColorEffect.getPresetList() ){
            if( c.getPresetName().compareTo(mSelectedClip.getColorEffect().getPresetName()) == 0 ){
                find = true;
                break;
            }
            colorIndex++;
        }
        if( !find )
            colorIndex = 0;
        mColorEffectSpinner.setSelection(colorIndex);
        seekBar_Bri.setProgress(255-mSelectedClip.getBrightness());
        seekBar_Sat.setProgress(255-mSelectedClip.getSaturation());
        seekBar_Con.setProgress(255-mSelectedClip.getContrast());
        mEngine.updateProject();
        moveClipTime();
    }

    private void moveClipTime(){
        if( mSelectedClip != null ){
            int duration = (mSelectedClip.getProjectEndTime() - mSelectedClip.getProjectStartTime()) /2;
            mEngine.seek(mSelectedClip.getProjectStartTime()+duration);
        }
    }

    private void validateButtons(){
        int i = project.getTotalClipCount(true);
        if( i > 0 ) {
            mPlayStop.setEnabled(true);
            mExport.setEnabled(true);
            mBGM.setEnabled(true);
        }else{
            mPlayStop.setEnabled(false);
            mExport.setEnabled(false);
            mBGM.setEnabled(false);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_project_edit2);

        mPlayTime = (TextView) findViewById(R.id.textview_project_edit2_time);
        mEndTime = (TextView) findViewById(R.id.textview_project_edit2_total);

        m_editorView = (nexEngineView) findViewById(R.id.engineview_project_edit2);
        m_editorView.setListener(this);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();

        mEngine.setFaceModule("6a460d22-cd87-11e7-abc4-cec278b6b50a");

        mEngine.setView(m_editorView);
        project = new nexProject();
        mEngine.setProject(project);
        mEngine.setEventHandler(this);

        mInflater = LayoutInflater.from(this);
        mGallery = (LinearLayout) findViewById(R.id.layout_project2_thumb_seekbar);


        mSeekBarPlayTime = (SeekBar) findViewById(R.id.seekbar_project_edit2);
        //mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
        mSeekBarPlayTime.setProgress(0);
        mSeekBarPlayTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                if (trackingSeekbarTouch) {

                    playTime = i * 33;
                    Log.d(TAG, "seekTs:" + mSeekCurrentTS + " progress:" + i + "/" + Math.abs(mSeekCurrentTS - playTime));
                    if (Math.abs(mSeekCurrentTS - playTime) > 100/*Tmp Value*/) {
                        mSeekMode = 2;
                    } else {
                        mSeekMode = 1;
                    }
                    trackingSeektime = playTime;

                    if (!lastUIisPlaying) {
                        seek(trackingSeektime);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                trackingSeekbarTouch = true;
                stop();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                trackingSeekbarTouch = false;
            }
        });

        mPlayStop = (Button) findViewById(R.id.button_project_edit2_play_stop);
        mPlayStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //changeEngineView();
                setEditMode(null);
                if (mState == State.PLAYING  ) {

                    stop();
                    mPlayStop.setText("Play");
                    lastUIisPlaying = false;
                } else {
                    mEngine.updateProject();
                    /*
                    if( mEngine.updateProjectAdvance(false) ){
                        seek(curr);
                    }
                    */
                    mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
                    resume();
                    mPlayStop.setText("Stop");
                    lastUIisPlaying = true;
                }
            }
        });

        View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
        ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
        Bitmap bmpAdd = BitmapFactory.decodeResource(getApplicationContext().getResources(), R.drawable.add);
        img.setImageBitmap(bmpAdd);
        img.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType("*/*");
                String[] mimetypes = {"image/*", "video/*"};
                intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });
        mGallery.addView(view);

        mExport = (Button) findViewById(R.id.button_project_edit2_export);
        mExport.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setEditMode(null);
                if (mEngine.getProject() == null ) {
                    Toast.makeText(getApplicationContext(), "you must add clip!", Toast.LENGTH_SHORT).show();
                    return;
                }

                if( mEngine.getProject().getTotalClipCount(true) == 0 ){
                    Toast.makeText(getApplicationContext(), "you must add clip!", Toast.LENGTH_SHORT).show();
                    return;
                }

                mEngine.stop();
                Intent intent = new Intent(getBaseContext(), ExportActivity.class);
                intent.putExtra("AutoPauseResume",true);
                startActivity(intent);
            }
        });


        mBGM = (Button)findViewById(R.id.button_project_edit2_bgm);
        mBGM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                enableBGMSetting();
            }
        });
        createClipEdit();
        createBGMEdit();
        validateButtons();
        loadDialog();
    }

    @Override
    public void onEngineViewAvailable(int width, int height) {
        isAvailableView = true;
        for (Runnable runnable : postOnAvailableSurfaceRunnables) {
            runnable.run();
        }
        Log.d(TAG, "onEngineViewAvailable (" + width + "X" + height + ")");
        postOnAvailableSurfaceRunnables.clear();
    }

    @Override
    public void onEngineViewSizeChanged(int width, int height) {
        Log.d(TAG, "onEngineViewSizeChanged (" + width + "X" + height + ")");
    }

    @Override
    public void onEngineViewDestroyed() {
        isAvailableView = false;
        Log.d(TAG, "onEngineViewDestroyed ()");
    }

    @Override
    public void onStateChange(int oldState, int newState) {
        Log.i(TAG, "onStateChange(): oldState=" + oldState + ", newState=" + newState);
        if (newState == 1) {

            mState = State.STOPED;
            if (trackingSeektime != -1) {
                seek(trackingSeektime);
                trackingSeektime = -1;
            }

            if (mPlayStop != null) {
                mPlayStop.setText("Play");
            }
        } else if (newState == 2) {

            mState = State.PLAYING;
            if (mPlayStop != null) {
                mPlayStop.setText("Stop");
            }
        }
    }

    @Override
    public void onTimeChange(int currentTime) {
        if (!trackingSeekbarTouch) {
            playTime = currentTime;
            mSeekBarPlayTime.setProgress(currentTime / 33);
            mPlayTime.setText(getTimeString(currentTime));
        }
    }

    @Override
    public void onSetTimeDone(int currentTime) {
        mSeekBarPlayTime.setProgress(currentTime / 33);
        mPlayTime.setText(getTimeString(currentTime));

        Log.i(TAG, String.format("onSetTimeDone(): currentTime=%d, seekInProgress=%b lastUIisPlaying=%b", currentTime, seekInProgress, lastUIisPlaying));

        mState = State.STOPED;

        if (trackingSeektime != -1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if (lastUIisPlaying) {
            resume();
        }
    }

    @Override
    public void onSetTimeFail(int err) {
        Log.i(TAG, "onSetTimeFail(): err=" + err + ", seekInProgress=" + seekInProgress);

        mState = State.STOPED;

        if (trackingSeektime != -1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if (lastUIisPlaying) {
            resume();
        }
    }

    @Override
    public void onSetTimeIgnored() {
        Log.i(TAG, "onSetTimeIgnored(): seekInProgress=" + seekInProgress);

        mState = State.STOPED;

        if (trackingSeektime != -1) {
            seek(trackingSeektime);
            trackingSeektime = -1;
            return;
        }

        if (lastUIisPlaying) {
            resume();
        }
        /*
        if( !seekInProgress && !trackingSeekbarTouch ){
            mState = State.STOPED;

            if( trackingSeektime != - 1) {
                seek(trackingSeektime);
                trackingSeektime = -1;
            }

            if( lastUIisPlaying ){
                resume();
            }
        }
        */

    }

    @Override
    public void onEncodingDone(boolean iserror, int result) {

    }

    @Override
    public void onPlayEnd() {
        Log.d(TAG, "onPlayEnd");
        mPlayStop.setText("Play");
        lastUIisPlaying = false;
        mEngine.seek(0);
    }

    @Override
    public void onPlayFail(int err, int iClipID) {

    }

    @Override
    public void onPlayStart() {
        Log.d(TAG, "onPlayStart");
    }

    @Override
    public void onClipInfoDone() {

    }

    @Override
    public void onSeekStateChanged(boolean isSeeking) {
        Log.i(TAG, "onSeekStateChanged() isSeeking=" + isSeeking);
        seekInProgress = isSeeking;

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

    private String getTimeString(int time) {

        return
                String.format("%02d:%02d:%02d.%02d",
                        time / 3600000,
                        (time % 3600000) / 60000,
                        (time % 60000) / 1000,
                        (time % 1000) / 10);
    }

    View.OnClickListener m_thumbailClick = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            nexClip clip = (nexClip)v.getTag();
            if( clip == null ){

            }else{
                //editor view
                setEditMode(clip);
            }
        }
    };

    private enum State {
        IDLE,
        STOP_READY,
        STOPED,
        SEEKING,
        PLAY_READY,
        PLAYING,
    }

    private ProjectEdit2Activity.State mState = ProjectEdit2Activity.State.IDLE;
    private List<Runnable> postOnAvailableSurfaceRunnables = new ArrayList<>();
    private boolean isAvailableView = false;
    private boolean backgroundMode;

    private void resume() {
        if (!isAvailableView && !backgroundMode) {
            postOnAvailableSurfaceRunnables.add(new Runnable() {
                @Override
                public void run() {
                    resume();
                }
            });
        } else {
            if (mState == State.STOPED || mState == State.IDLE) {
                mState = State.PLAY_READY;
                mEngine.resume();
            } else {
                Log.w(TAG, "resume() invaild state=" + mState);
            }
        }
    }

    private void stop() {
        if (mState == State.PLAYING || mState == State.PLAY_READY) {
            mState = State.STOP_READY;
            mEngine.stop();
        } else {
            Log.w(TAG, "stop() invaild state=" + mState);
        }
    }

    private void seek(int ms) {
        Log.d(TAG, "seek ms=" + ms + ", trackingSeektime=" + trackingSeektime);
        if (!isAvailableView && !backgroundMode) {
            mSeekms = ms;
            postOnAvailableSurfaceRunnables.add(new Runnable() {
                @Override
                public void run() {
                    seek(mSeekms);
                }
            });
        } else {
            mState = State.SEEKING;
            mSeekCurrentTS = ms;
            if (mSeekMode == 1) {
                mEngine.seek(ms);
            } else {
                mEngine.seekIDRorI(ms);
            }
        }
    }


    public static Bitmap getThumbnail(ContentResolver cr, String path) throws Exception {

        Cursor ca = cr.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[] { MediaStore.MediaColumns._ID }, MediaStore.MediaColumns.DATA + "=?", new String[] {path}, null);
        if (ca != null && ca.moveToFirst()) {
            int id = ca.getInt(ca.getColumnIndex(MediaStore.MediaColumns._ID));
            ca.close();
            return MediaStore.Images.Thumbnails.getThumbnail(cr, id, MediaStore.Images.Thumbnails.MINI_KIND, null );
        }

        ca.close();
        return null;
    }

    private void addClip(nexClip clip){
        if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
            View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
            ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
            Bitmap thumb = null;

            try {
                thumb = getThumbnail(getApplicationContext().getContentResolver(), clip.getPath());
            } catch (Exception e) {
                e.printStackTrace();
            }

            if (thumb == null) {
                Log.d(TAG, "getThumbnail is null=" + clip.getPath());
            }

            img.setImageBitmap(thumb);
            img.setTag(clip);
            img.setOnClickListener(m_thumbailClick);
            mGallery.addView(view);
            project.add(clip);
            int numFace=0;
            numFace = clip.getCrop().faceStartEndPosition(6000);
            project.updateProject();
            mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
            mEndTime.setText(numFace + "," + getTimeString(project.getTotalTime()));
        }else if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            project.add(clip);
            clip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
                @Override
                public void onLoadThumbnailResult(int event) {
                    View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
                    ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
                    Bitmap thumb = project.getLastPrimaryClip().getMainThumbnail(0,0);
                    img.setImageBitmap(thumb);
                    img.setTag(project.getLastPrimaryClip());
                    img.setOnClickListener(m_thumbailClick);
                    mGallery.addView(view);
                    project.updateProject();
                    mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
                    mEndTime.setText(getTimeString(project.getTotalTime()));
                }
            });
        }
        validateButtons();
    }

    private void deleteClip(nexClip clip){
        int index = 1;
        for(nexClip c : project.getPrimaryItems()){
            if( c == clip ){
                break;
            }
            index++;
        }

        for( int i = 0 ; i < mGallery.getChildCount() ; i++ ){
            View view = mGallery.getChildAt(i);
            if( view != null ){
                ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
                if( img != null ) {
                    if( img.getTag() != null ) {
                        Log.d(TAG, "deleteClip [" + i + "] tag find!");
                        if( img.getTag() == clip ){
                            Log.d(TAG, "deleteClip [" + i + "] tag same clip!");
                        }
                    }else {
                        Log.d(TAG, "deleteClip [" + i + "] tag is null");
                    }
                }
            }
        }

        mGallery.removeViewAt(index);
        project.remove(clip);
        mEngine.updateProject();
        mSeekBarPlayTime.setMax(project.getTotalTime() / 33);
        mEndTime.setText(getTimeString(project.getTotalTime()));
        mEngine.seek(0);
    }

    private void moveClip(nexClip clip){

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String filepath = null;

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                nexClip clip = nexClip.getSupportedClip(filepath);
                if(clip != null) {
                    if (clip.getTotalTime() < 2000 && clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        Toast.makeText(getApplicationContext(), "select content`s duration 2000 over", Toast.LENGTH_SHORT).show();
                        return;
                    }

                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }
/*
                    if( !clip.ableToDecoding() ){
                        Toast.makeText(getApplicationContext(), "ableToDecoding fail", Toast.LENGTH_LONG).show();
                        return;
                    }
*/
                    //mProject.add(clip);
                    addClip(clip);
                    //mItemAdp.notifyDataSetChanged();
                }else{
                    Toast.makeText(ProjectEdit2Activity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                }
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);

                    nexClip clip = nexClip.getSupportedClip(filepath);

                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }

                    if( clip != null ) {
                        //mProject.add(clip);
                        addClip(clip);
                        //mItemAdp.notifyDataSetChanged();
                    }else{
                        Toast.makeText(ProjectEdit2Activity.this, "Not supported clip : "+filepath , Toast.LENGTH_SHORT).show();
                    }
                }
            }
            //mProject.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
            //seekBar.setMax(mProject.getTotalTime()/33);
        }else if(requestCode == 200 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.EFFECT_OPTION);
            String effect = data.getStringExtra(Constants.EFFECT);
            mSelectedClip.getClipEffect().setEffect(effect);
            mSelectedClip.getClipEffect().updateEffectOptions(opt, true);
            mEngine.updateProject();
        }else if(requestCode == 300 && resultCode == Activity.RESULT_OK) {
            nexEffectOptions opt = (nexEffectOptions)data.getSerializableExtra(Constants.TRANSITION_EFFECT_OPTION);
            String effect = data.getStringExtra(Constants.TRANSITION_EFFECT);
            mSelectedClip.getTransitionEffect().setTransitionEffect(effect);
            int[] range = project.getTransitionDurationTimeGuideLine(mSelectedClip,500);
            int min = range[0];
            int max = range[1];
            Log.d(TAG,"Transition Effect max duration="+max);
            if( max == 0){
                mSelectedClip.getTransitionEffect().setEffectNone();
            }else{
                mSelectedClip.getTransitionEffect().updateEffectOptions(opt, true);
            }

            int transitionDuration = 2000;
            if( max < transitionDuration ){
                transitionDuration = max;
            }
            mSelectedClip.getTransitionEffect().setDuration(transitionDuration);
            mEngine.updateProject();
            //mSelectedClip.getTransitionEffect().setEffectNone(); //no transition effect. (= delete transition effect)
        }else if(requestCode == 400 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String audioFile = null;
            if (clipData == null) {
                audioFile = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                item = clipData.getItemAt(0);
                uri = item.getUri();
                audioFile = ConvertFilepathUtil.getPath(this, uri);
            }
            nexClip clip = nexClip.getSupportedClip(audioFile);
            if( clip == null ){
                Toast.makeText(getApplicationContext(),"Audio not supported!", Toast.LENGTH_SHORT).show();
            }else{
                project.setBackgroundMusicPath(audioFile);
                enableBGMSetting();
            }
        }
    }

    private String getSavedData(){
        File saveFile = new File( getApplicationContext().getCacheDir(),".projectedit2.sav");
        if( !saveFile.isFile() ){
            return null;
        }
        FileInputStream in = null;
        StringBuilder out = new StringBuilder();
        try {
            in = new FileInputStream(saveFile);
            BufferedReader reader = new BufferedReader(new InputStreamReader(in));
            String line;
            while ((line = reader.readLine()) != null) {
                out.append(line);
            }
            reader.close();
            in.close();
            saveFile.delete();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
            saveFile.delete();
        }
        return out.toString();
    }

    private void loadProject(String savedData){
        project = nexProject.createFromSaveString(savedData);

        new AsyncTask<Void,Void,Void>(){
            int thumbnailCount = 0;
            @Override
            protected Void doInBackground(Void... params) {
                for( nexClip clip : project.getPrimaryItems() ) {
                    if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                        thumbnailCount++;
                        Log.d(TAG,"loadProject clip thumbnail input="+thumbnailCount);
                        clip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
                            @Override
                            public void onLoadThumbnailResult(int event) {
                                thumbnailCount--;
                                Log.d(TAG,"loadProject clip thumbnail result="+thumbnailCount);
                            }
                        });
                    }
                }

                while(thumbnailCount != 0){
                    try {
                        Thread.sleep(100,0);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                for( nexClip clip : project.getPrimaryItems() ) {
                    View view = mInflater.inflate(R.layout.seek_thumb_item, mGallery, false);
                    ImageView img = (ImageView) view.findViewById(R.id.id_index_gallery_item_image);
                    Bitmap thumb = null;
                    if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                        try {
                            thumb = getThumbnail(getApplicationContext().getContentResolver(), clip.getPath());
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }else if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                        thumb = clip.getMainThumbnail(0,0);
                    }
                    if( thumb == null ){
                        Log.d(TAG,"getThumbnail is null="+clip.getPath());
                    }
                    img.setImageBitmap(thumb);
                    img.setTag(clip);
                    img.setOnClickListener(m_thumbailClick);
                    mGallery.addView(view);
                }
                project.updateProject();
                mEngine.setProject(project);
                mSeekBarPlayTime.setMax(project.getTotalTime()/33);
                mEndTime.setText(getTimeString(project.getTotalTime()));
                validateButtons();
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, null, null, null);
    }

    public void loadDialog() {
        lastSavedData = getSavedData();
        if( lastSavedData !=null ) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(ProjectEdit2Activity.this);
            builder.setMessage("save data found! do you want last state")
                    .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            loadProject(lastSavedData);
                            lastSavedData = null;
                        }
                    })
                    .setNegativeButton("NO", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            // User cancelled the dialog
                        }
                    }).create().show();
        }
    }

    public static void saveProject(Context appCon, nexProject project){
        if( project.getTotalClipCount(true) > 0 ) {

            File saveFile = new File(appCon.getCacheDir(), ".projectedit2.sav");
            FileOutputStream out = null;
            try {
                out = new FileOutputStream(saveFile);
                out.write(project.saveToString().getBytes());
                out.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    protected void onStart() {
        if (mEngine != null) {
            mEngine.setEventHandler(this);
        }
        super.onStart();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        saveProject(getApplication(),project);
        super.onDestroy();
    }
}
