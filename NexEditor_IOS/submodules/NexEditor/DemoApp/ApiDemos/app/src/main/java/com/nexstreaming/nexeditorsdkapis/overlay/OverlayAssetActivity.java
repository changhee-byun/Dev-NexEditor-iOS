package com.nexstreaming.nexeditorsdkapis.overlay;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.nexstreaming.nexeditorsdk.exception.NoSuchObjectException;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEffectOptions;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayFilter;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.dialog.ColorPickerDialog;


import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class OverlayAssetActivity extends Activity {
    static private String TAG = "OverlayEdit";
    static private final int demoTime = 6000;
    private EditText mEditTextLine1;
    private EditText mEditTextLine2;
    private EditText mEditTextLine3;
    private EditText mEditTextLine4;

    private Spinner mSpinnerFont1;
    private Spinner mSpinnerFont2;
    private Spinner mSpinnerFont3;
    private Spinner mSpinnerFont4;

    private Spinner mSpinnerSelectOption1;
    private Spinner mSpinnerSelectOption2;

    private SeekBar mSeekRange1;
    private SeekBar mSeekRange2;
    private Button mBtnColor1;
    private Button mBtnColor2;
    private Button mBtnColor3;
    private Button mBtnColor4;
    private TextView mTextInfo;
    private Button mBtnPlayStop;
    private ToggleButton mToggle3D;
    private String mSelectedEffectId;
    private ArrayList<String> mEffectItems;
    private int mColor1;
    private int mColor2;
    private int mColor3;
    private int mColor4;
    private int mSelectedColor = 1;
    private ColorPickerDialog.OnColorChangedListener mColorChangedListener;

    private nexEngine mEngine;
    private nexEngineView m_editorView;

    private int mOverlayItemId;

    private double mLastDistance;
    private ArrayList<String> m_listfilepath;
    private boolean mEngineViewAvailable;
    private boolean mIsPlaying;
    private enum TouchMode {
        HIT,
        ROTATE,
        SCALE,
        MOVE,
        DELETE,
        NONE
    }
    private static final ExecutorService sPreviewAnimationThreadExcutor = Executors.newSingleThreadExecutor();

    private nexEngineListener mEditorListener = new nexEngineListener() {
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
            mEngine.seek(0);
            mIsPlaying = false;
            mBtnPlayStop.setText("Play");

        }

        @Override
        public void onPlayFail(int err, int iClipID) {
            mEngine.seek(0);
            mIsPlaying = false;
            mBtnPlayStop.setText("Play");
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_overlay_asset);
        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        nexProject project = new nexProject();

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setEventHandler(mEditorListener);
        mEngine.setProject(project);
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if(clip == null){
            Toast.makeText(getApplicationContext(), "getSupportedClip fail", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if(clip.getTotalTime() < demoTime ){
            Toast.makeText(getApplicationContext(), "Clip total time is "+demoTime+"(ms)", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        mEngine.getProject().add(clip);

        if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            clip.getVideoClipEdit().setTrim(0,demoTime);
        }else{
            clip.setImageClipDuration(demoTime);
        }


        mEngine.updateProject();

        m_editorView = (nexEngineView) findViewById(R.id.engineview_overlayasset_view);
        mEngine.setView(m_editorView);
        m_editorView.setOnTouchListener(new View.OnTouchListener() {
            nexOverlayItem.HitPoint mPosition = new nexOverlayItem.HitPoint();
            TouchMode mode = TouchMode.NONE;
            double beforeScale = -1;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (mEngine == null) {
                    return false;
                }
                int act = event.getAction();
                if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_DOWN) {

                    mPosition.mTime = mEngine.getCurrentPlayTimeTime();
                    mPosition.mViewX = event.getX();
                    mPosition.mViewY = event.getY();
                    mPosition.mViewWidth = v.getWidth();
                    mPosition.mViewHeight = v.getHeight();

                    if (mode == TouchMode.NONE) {
                        if (mEngine.getOverlayHitPoint(mPosition)) {
                            stopPreviewAnimation();
                            mode = TouchMode.HIT;
                            int id  = mPosition.getID();
                            if( mOverlayItemId != id ) {
                                mOverlayItemId = id;
                                updateData();
                            }
                            mLastDistance = mEngine.getProject().getOverlay(mOverlayItemId).getScaledX() -1;
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).display();
                        }
                        return true;
                    }

                    if (mEngine.getOverlayHitPoint(mPosition)) {
                        //mOverlayItemId = mPosition.getID();
                        Log.d(TAG, "ACTION_DOWN=(" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was ID = " + mPosition.getID() + ",Pos= " + mPosition.getHitInPosition());
                        if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_RightTop) { //rotate
                            mode = TouchMode.ROTATE;
                        } else if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_RightBottom) { //scale
                            mode = TouchMode.SCALE;
                        } else if (mPosition.getHitInPosition() == nexOverlayItem.kOutLine_Pos_LeftTop) { //delete
                            mode = TouchMode.DELETE;
                        } else { //move
                            mode = TouchMode.MOVE;
                        }
                    } else {
                        Log.d(TAG, "ACTION_DOWN = (" + event.getX() + ", " + event.getY() + ", " + v.getWidth() + ", " + v.getHeight() + ") was not hit.");
                        mode = TouchMode.NONE;
                        return true;
                    }
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP) {
                    if (mode != TouchMode.HIT) {
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(false).display();

                        if( mode == TouchMode.DELETE){
                            Log.d(TAG, "ACTION_DOWN = (" +mOverlayItemId+ ") delete.");
                            mEngine.getProject().removeOverlay(mOverlayItemId);
                            //mOverlayFilters.remove(mOverlayItemId);
                            if( mEngine.getProject().getOverlayItems().size() > 0 ){
                                mOverlayItemId = mEngine.getProject().getOverlayItems().get(0).getId();
                                updateData();
                                printInfo();
                            }else{
                                stopPreviewAnimation();
                                mode = TouchMode.NONE;
                                return true;
                            }
                        }
                        startPreviewAnimation();
                        mode = TouchMode.NONE;
                    }
                } else if ((act & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_MOVE) {
                    Log.d(TAG, "ACTION_MOVE = ["+mOverlayItemId+"](" + event.getX() + "," + event.getY() + "), mode = " + mode + ", view width: " + v.getWidth() + ", height: " + v.getHeight());
                    if (mode == TouchMode.MOVE) { // move
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true)
                                .setPositionX((int) (event.getX() * 1280 / v.getWidth()))
                                .setPositionY((int) (event.getY() * 720 / v.getHeight()))
                                .display();
                        mEngine.getProject().getOverlay(mOverlayItemId).setPosition((int) event.getX() * 1280 / v.getWidth(), (int) event.getY() * 720 / v.getHeight());
                        printInfo();
                    } else if (mode == TouchMode.ROTATE) { // rotate
                        int angle = (int) getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY());

                        Log.d(TAG, "View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getAngle(" + getAngle((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) + ")");
                        mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setRotateZ(angle).display();
                        mEngine.getProject().getOverlay(mOverlayItemId).setRotate(angle);
                        printInfo();
                    } else if (mode == TouchMode.SCALE) { // scale
                        double distance = getDistance((int) mPosition.mViewX, (int) mPosition.mViewY, (int) event.getX(), (int) event.getY()) * 0.003f; /* 0.003f is standard distance */
                        distance += mLastDistance;
                        if(distance==0){
                            return true;
                        }
                        float scale = Float.parseFloat(String.format("%.2f", (float) (1 + distance)));
                        if (scale <= 0.25f)
                            scale = 0.25f;

                        if (beforeScale != scale) {
                            Log.d(TAG, "TouchMode.SCALE -> View XY(" + mPosition.mViewX + ", " + mPosition.mViewY + "), Event XY (" + event.getX() + ", " + event.getY() + ") -> getDistance(" + distance + "), scale(" + scale + ")");
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setScaleX(scale).display();
                            mEngine.buildOverlayPreview(mOverlayItemId).setOutline(true).setScaleY(scale).display();

                            mEngine.getProject().getOverlay(mOverlayItemId).setScale(scale, scale);
                        }
                        beforeScale = scale;
                        printInfo();
                    }
                }
                return true;
            }
        });


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

        mTextInfo = (TextView)findViewById(R.id.textView_overlayasset_info);

        mEditTextLine1 = (EditText)findViewById(R.id.edittext_overlayasset_line1);
        mEditTextLine2 = (EditText)findViewById(R.id.edittext_overlayasset_line2);
        mEditTextLine3 = (EditText)findViewById(R.id.edittext_overlayasset_line3);
        mEditTextLine4 = (EditText)findViewById(R.id.edittext_overlayasset_line4);
        mEditTextLine1.setText("Text Line 1");
        mEditTextLine2.setText("Text Line 2");
        mEditTextLine3.setText("Text Line 3");
        mEditTextLine4.setText("Text Line 4");

        mEditTextLine1.setImeOptions(EditorInfo.IME_ACTION_DONE);

        mEditTextLine1.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    hideSoftKeyboard(mEditTextLine1);
                    try {
                        nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                        filter.getEffectOption().getTextOptions().get(0).setText(mEditTextLine1.getText().toString());
                        mEngine.buildOverlayPreview(mOverlayItemId).display();
                    } catch (NoSuchObjectException e) {
                        e.printStackTrace();
                    }

                    return true;
                }
                return false;
            }
        });

        mEditTextLine2.setImeOptions(EditorInfo.IME_ACTION_DONE);

        mEditTextLine2.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    hideSoftKeyboard(mEditTextLine2);
                    try {
                        nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                        filter.getEffectOption().getTextOptions().get(1).setText(v.getText().toString());
                        mEngine.buildOverlayPreview(mOverlayItemId).display();
                    } catch (NoSuchObjectException e) {
                        e.printStackTrace();
                    }

                    mEngine.buildOverlayPreview(mOverlayItemId).display();
                    return true;
                }
                return false;
            }
        });

        mEditTextLine3.setImeOptions(EditorInfo.IME_ACTION_DONE);

        mEditTextLine3.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    hideSoftKeyboard(mEditTextLine3);
                    try {
                        nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                        filter.getEffectOption().getTextOptions().get(2).setText(v.getText().toString());
                        mEngine.buildOverlayPreview(mOverlayItemId).display();
                    } catch (NoSuchObjectException e) {
                        e.printStackTrace();
                    }

                    mEngine.buildOverlayPreview(mOverlayItemId).display();
                    return true;
                }
                return false;
            }
        });

        mEditTextLine4.setImeOptions(EditorInfo.IME_ACTION_DONE);

        mEditTextLine4.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    hideSoftKeyboard(mEditTextLine4);
                    try {
                        nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                        filter.getEffectOption().getTextOptions().get(3).setText(v.getText().toString());
                        mEngine.buildOverlayPreview(mOverlayItemId).display();
                    } catch (NoSuchObjectException e) {
                        e.printStackTrace();
                    }

                    mEngine.buildOverlayPreview(mOverlayItemId).display();
                    return true;
                }
                return false;
            }
        });


        mEffectItems = new ArrayList();

        for( nexOverlayFilter filter :  nexEffectLibrary.getEffectLibrary(getApplicationContext()).getOverlayEffect() ){
            if( filter.getEffectOption().getTextOptions().size() > 0 ) {
                //Bitmap icon = filter.getIcon();
                mEffectItems.add(filter.getId());
            }
        }

        mSpinnerFont1 = (Spinner) findViewById(R.id.spinner_overlayasset_font1);
        mSpinnerFont1.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String fontid = null;
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    if( position == 0 ){
                        filter.getEffectOption().getTypefaceOptions().get(0).reset();
                    }else{
                        fontid = nexFont.getFontIds()[position-1];
                        filter.getEffectOption().getTypefaceOptions().get(0).setTypeface(nexEffectOptions.TAG_TYPEFACE_FONTID+fontid);
                    }
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        mSpinnerFont2 = (Spinner) findViewById(R.id.spinner_overlayasset_font2);
        mSpinnerFont2.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String fontid = null;
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    if( position == 0 ){
                        filter.getEffectOption().getTypefaceOptions().get(1).reset();
                    }else{
                        fontid = nexFont.getFontIds()[position-1];
                        filter.getEffectOption().getTypefaceOptions().get(1).setTypeface(nexEffectOptions.TAG_TYPEFACE_FONTID+fontid);
                    }
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        mSpinnerFont3 = (Spinner) findViewById(R.id.spinner_overlayasset_font3);
        mSpinnerFont3.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String fontid = null;
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    if( position == 0 ){
                        filter.getEffectOption().getTypefaceOptions().get(2).reset();
                    }else{
                        fontid = nexFont.getFontIds()[position-1];
                        filter.getEffectOption().getTypefaceOptions().get(2).setTypeface(nexEffectOptions.TAG_TYPEFACE_FONTID+fontid);
                    }
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        mSpinnerFont4 = (Spinner) findViewById(R.id.spinner_overlayasset_font4);
        mSpinnerFont4.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String fontid = null;
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    if( position == 0 ){
                        filter.getEffectOption().getTypefaceOptions().get(3).reset();
                    }else{
                        fontid = nexFont.getFontIds()[position-1];
                        filter.getEffectOption().getTypefaceOptions().get(3).setTypeface(nexEffectOptions.TAG_TYPEFACE_FONTID+fontid);
                    }
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSpinnerSelectOption1 = (Spinner) findViewById(R.id.spinner_overlayasset_selectoption1);
        mSpinnerSelectOption1.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    filter.getEffectOption().getSelectOptions().get(0).setSelectIndex(position);
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSpinnerSelectOption2 = (Spinner) findViewById(R.id.spinner_overlayasset_selectoption2);
        mSpinnerSelectOption2.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    filter.getEffectOption().getSelectOptions().get(1).setSelectIndex(position);
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }

            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSeekRange1 = (SeekBar) findViewById(R.id.seekBar_overlayasset_range1);
        mSeekRange1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lastProgress = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    filter.getEffectOption().getRangeOptions().get(0).setValue(lastProgress );
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }
            }
        });

        mSeekRange2 = (SeekBar) findViewById(R.id.seekBar_overlayasset_range2);
        mSeekRange2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int lastProgress = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lastProgress = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    filter.getEffectOption().getRangeOptions().get(1).setValue(lastProgress );
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }
            }
        });

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
                R.layout.simple_spinner_dropdown_item_1, mEffectItems);

        Spinner spinnerSelect = (Spinner) findViewById(R.id.spinner_overlayasset_select);
        spinnerSelect.setAdapter(adapter);
        spinnerSelect.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mSelectedEffectId = mEffectItems.get(position).toString();
                nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(getApplicationContext());
                for( nexOverlayFilter text : fxlib.getOverlayEffect() ){
                    if( text.getId().compareTo(mSelectedEffectId) == 0){

                        text.setWidth(text.getIntrinsicWidth()/2);
                        text.setHeight(text.getIntrinsicHeight()/2);
                        nexOverlayItem overlayItem = new nexOverlayItem(text,nexApplicationConfig.getAspectProfile().getWidth() / 2, nexApplicationConfig.getAspectProfile().getHeight() / 2,0,demoTime);
                        mOverlayItemId = overlayItem.getId();
                        mEngine.getProject().addOverlay(overlayItem);

                        updateData();
                        setText();
                        printInfo();
                        startPreviewAnimation();
                    }
                }
                //fxlib.info(mSelectedEffectId);

                parent.invalidate();
            }

            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        mBtnColor1 = (Button)findViewById(R.id.button_overlayasset_color1);
        mBtnColor1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 1;
                new ColorPickerDialog(OverlayAssetActivity.this, mColorChangedListener, mColor1).show();
            }
        });
        mBtnColor2 = (Button)findViewById(R.id.button_overlayasset_color2);
        mBtnColor2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 2;
                new ColorPickerDialog(OverlayAssetActivity.this, mColorChangedListener, mColor2).show();
            }
        });
        mBtnColor3 = (Button)findViewById(R.id.button_overlayasset_color3);
        mBtnColor3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 3;
                new ColorPickerDialog(OverlayAssetActivity.this, mColorChangedListener, mColor3).show();
            }
        });
        mBtnColor4 = (Button)findViewById(R.id.button_overlayasset_color4);
        mBtnColor4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedColor = 4;
                new ColorPickerDialog(OverlayAssetActivity.this, mColorChangedListener, mColor4).show();
            }
        });


        mColorChangedListener = new ColorPickerDialog.OnColorChangedListener(){
            @Override
            public void colorChanged(int color) {
                boolean pickColor = true;
                if( mSelectedColor == 1 ) {
                    mColor1 = color;
                }else if( mSelectedColor == 2 ){
                    mColor2 = color;
                }else if( mSelectedColor == 3 ){
                    mColor3 = color;
                }else if( mSelectedColor == 4 ){
                    mColor4 = color;
                }else{
                    pickColor = false;
                }
                if( pickColor ) {
                    try {
                        nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                        filter.getEffectOption().getColorOptions().get(mSelectedColor-1).setARGBColor(color);
                    } catch (NoSuchObjectException e) {
                        e.printStackTrace();
                    }
                }
            }
        };

        mBtnPlayStop = (Button)findViewById(R.id.button_overlayasset_play_stop);
        mBtnPlayStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if( mIsPlaying ){
                    mEngine.stop();
                    mEngine.seek(0);
                    mIsPlaying = false;
                    mBtnPlayStop.setText("Play");
                }else {
                    stopPreviewAnimation();
                    mEngine.play();
                    mIsPlaying = true;
                    mBtnPlayStop.setText("Stop");
                }
            }
        });

        mToggle3D = (ToggleButton)findViewById(R.id.toggleButton_overlayasset_3d);
        mToggle3D.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
                    filter.set3DMode(isChecked);
                } catch (NoSuchObjectException e) {
                    e.printStackTrace();
                }
            }
        });

        nexOverlayItem.setOutLine();
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_RightTop, R.drawable.layer_rotate);
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_RightBottom, R.drawable.layer_scale);
        nexOverlayItem.setOutLineIcon(this, nexOverlayItem.kOutLine_Pos_LeftTop, R.drawable.layer_cross);

    }
    public double getAngle(int startX, int startY, int endX, int endY) {
        double dy = endY - startY;
        double dx = endX - startX;
        double angle = Math.atan(dy/dx) * (180.0/Math.PI);

        if(dx < 0.0) {
            angle += 180.0;
        } else {
            if(dy < 0.0) angle += 360.0;
        }

        return angle;
    }

    public double getDistance(int startX, int startY, int endX, int endY) {
        double distance = Math.sqrt(Math.pow(Math.abs(endX - startX), 2) + Math.pow(Math.abs(endY - startY), 2));

        // 1 quadrant
        if(startX < endX && startY > endY) {
            return 1;
        }

        // 3 quadrant
        if(startX > endX && startY < endY) {
            return 1;
        }

        // 2 quadrant
        if(startX > endX && startY > endY) {
            distance *= -1;
        }

        return distance;
    }
    private double spacing(MotionEvent event) {
        double x = event.getX(0) - event.getX(1);
        double y = event.getY(0) - event.getY(1);
        return Math.sqrt(x * x + y * y);
    }

    private void updateData( ){

        nexEffectOptions option = null;
        String text = null;
        mEditTextLine1.setVisibility(View.GONE);
        mEditTextLine2.setVisibility(View.GONE);
        mEditTextLine3.setVisibility(View.GONE);
        mEditTextLine4.setVisibility(View.GONE);
        mSpinnerFont1.setVisibility(View.GONE);
        mSpinnerFont2.setVisibility(View.GONE);
        mSpinnerFont3.setVisibility(View.GONE);
        mSpinnerFont4.setVisibility(View.GONE);
        mSpinnerSelectOption1.setVisibility(View.GONE);
        mSpinnerSelectOption2.setVisibility(View.GONE);
        mBtnColor1.setVisibility(View.GONE);
        mBtnColor2.setVisibility(View.GONE);
        mBtnColor3.setVisibility(View.GONE);
        mBtnColor4.setVisibility(View.GONE);
        mSeekRange1.setVisibility(View.GONE);
        mSeekRange2.setVisibility(View.GONE);

        try {
            nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
            option = filter.getEffectOption();
            mToggle3D.setChecked(filter.is3DMode());
        } catch (NoSuchObjectException e) {
            e.printStackTrace();
            return;
        }

        if( option == null){
            return;
        }

        for( int line = 0; line < option.getTextFieldCount() ; line++ ){
            switch (line){
                case 0:
                    mEditTextLine1.setVisibility(View.VISIBLE);
                    text = option.getTextOptions().get(line).getText();
                    if( text != null) {
                        if (text.length() > 0) {
                            mEditTextLine1.setText(text);
                        }
                    }
                    break;

                case 1:
                    mEditTextLine2.setVisibility(View.VISIBLE);
                    text = option.getTextOptions().get(line).getText();
                    if( text != null) {
                        if (text.length() > 0) {
                            mEditTextLine2.setText(text);
                        }
                    }
                    break;

                case 2:
                    mEditTextLine3.setVisibility(View.VISIBLE);
                    text = option.getTextOptions().get(line).getText();
                    if( text != null) {
                        if (text.length() > 0) {
                            mEditTextLine3.setText(text);
                        }
                    }

                    break;

                case 3:
                    mEditTextLine4.setVisibility(View.VISIBLE);
                    text = option.getTextOptions().get(line).getText();
                    if( text != null) {
                        if (text.length() > 0) {
                            mEditTextLine4.setText(text);
                        }
                    }

                    break;

                default:
                    break;
            }
        }

        List<nexEffectOptions.ColorOpt> color = option.getColorOptions();
        int colorCount = 0;
        if( color != null ){
            for( nexEffectOptions.ColorOpt opt : color ){
                if( colorCount == 0 ){
                    mBtnColor1.setVisibility(View.VISIBLE);
                    mBtnColor1.setText(opt.getLabel());
                    mColor1 = opt.getARGBformat();
                }else if( colorCount == 1 ){
                    mBtnColor2.setVisibility(View.VISIBLE);
                    mBtnColor2.setText(opt.getLabel());
                    mColor2 = opt.getARGBformat();
                }else if( colorCount == 2 ){
                    mBtnColor3.setVisibility(View.VISIBLE);
                    mBtnColor3.setText(opt.getLabel());
                    mColor3 = opt.getARGBformat();
                }else if( colorCount == 3 ){
                    mBtnColor4.setVisibility(View.VISIBLE);
                    mBtnColor4.setText(opt.getLabel());
                    mColor4 = opt.getARGBformat();
                }
                colorCount++;
            }
        }

        List <nexEffectOptions.SelectOpt> select =  option.getSelectOptions();
        int selectCount = 0;
        if( select != null ) {
            for (nexEffectOptions.SelectOpt opt : select) {

                if (selectCount == 0) {
                    mSpinnerSelectOption1.setVisibility(View.VISIBLE);
                    ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                            R.layout.simple_spinner_dropdown_item_1, opt.getItems());
                    mSpinnerSelectOption1.setAdapter(adapter);
                } else if (selectCount == 1) {
                    mSpinnerSelectOption2.setVisibility(View.VISIBLE);
                    ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(this,
                            R.layout.simple_spinner_dropdown_item_1, opt.getItems());
                    mSpinnerSelectOption2.setAdapter(adapter2);
                }
                selectCount++;
            }
        }

        List<nexEffectOptions.RangeOpt> rangeOpts = option.getRangeOptions();
        if( rangeOpts != null){
            int count = 0;
            for( nexEffectOptions.RangeOpt opt : rangeOpts ){
                if( count == 0 ){
                    int range = opt.max() - opt.min() ;
                    int pos = opt.getValue()  - opt.min();
                    mSeekRange1.setVisibility(View.VISIBLE);
                    mSeekRange1.setMax(range);
                    mSeekRange1.setProgress(pos);
                }else if( count == 1 ){
                    int range = opt.max() - opt.min();
                    int pos = opt.getValue() - opt.min();
                    mSeekRange2.setVisibility(View.VISIBLE);
                    mSeekRange2.setMax(range);
                    mSeekRange2.setProgress(pos);
                }
                count++;
            }
        }

        List<nexEffectOptions.TypefaceOpt> typefaceOpts = option.getTypefaceOptions();
        if( typefaceOpts != null){
            int count = 0;
            nexFont.reload();
            ArrayList<String> fontList = new ArrayList<>();
            fontList.add("Default");
            for( String id :  nexFont.getFontIds() ){
                fontList.add(id);
            }
            ArrayAdapter<String> fontadapter = new ArrayAdapter<String>(this,
                    R.layout.simple_spinner_dropdown_item_1, fontList);


            for( nexEffectOptions.TypefaceOpt opt : typefaceOpts){
                if( count == 0 ){
                    mSpinnerFont1.setAdapter(fontadapter);
                    mSpinnerFont1.setVisibility(View.VISIBLE);
                }else if( count == 1 ){
                    mSpinnerFont2.setAdapter(fontadapter);
                    mSpinnerFont2.setVisibility(View.VISIBLE);
                }else if( count == 2 ){
                    mSpinnerFont3.setAdapter(fontadapter);
                    mSpinnerFont3.setVisibility(View.VISIBLE);
                }else if( count == 3 ){
                    mSpinnerFont4.setAdapter(fontadapter);
                    mSpinnerFont4.setVisibility(View.VISIBLE);
                }
                count++;
            }
        }
    }

    private void setText(){
        nexEffectOptions option = null;
        try {
            nexOverlayFilter filter = mEngine.getProject().getOverlay(mOverlayItemId).getOverLayFilter();
            option = filter.getEffectOption();
        } catch (NoSuchObjectException e) {
            e.printStackTrace();
            return;
        }

        if( option == null){
            return;
        }

        if (mEditTextLine1.getVisibility() == View.VISIBLE) {
            option.getTextOptions().get(0).setText(mEditTextLine1.getText().toString());
        }

        if (mEditTextLine2.getVisibility() == View.VISIBLE) {
            option.getTextOptions().get(1).setText(mEditTextLine2.getText().toString());
        }

        if (mEditTextLine3.getVisibility() == View.VISIBLE) {
            option.getTextOptions().get(2).setText(mEditTextLine3.getText().toString());
        }

        if (mEditTextLine4.getVisibility() == View.VISIBLE) {
            option.getTextOptions().get(3).setText(mEditTextLine4.getText().toString());
        }
    }

    private void printInfo(){
        nexOverlayItem oi = mEngine.getProject().getOverlay(mOverlayItemId);
        if( oi != null ){
            String info ="x="+oi.getPositionX()+",y="+oi.getPositionY()+",rotate="+oi.getRotate()+",scale="+oi.getScaledX();
            mTextInfo.setText(info);
        }
    }



    private AsyncTask previewAnimationTask;
    private void startPreviewAnimation(){
        stopPreviewAnimation();

        previewAnimationTask = new AsyncTask<Void,Void,Void>(){
            int time;
            @Override
            protected Void doInBackground(Void... params) {
                while(true) {

                    if( previewAnimationTask.isCancelled() )
                        break;
                    mEngine.fastPreview(nexEngine.FastPreviewOption.cts, time);
                    time += 33;
                    try {
                        Thread.sleep(33,0);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    if( time > demoTime){
                        time = 0;
                    }
                }
                mEngine.fastPreview(nexEngine.FastPreviewOption.cts, 0);
                return null;
            }
        }.executeOnExecutor(sPreviewAnimationThreadExcutor);

    }

    private void stopPreviewAnimation(){
        if( previewAnimationTask != null ) {
            previewAnimationTask.cancel(true);
            previewAnimationTask = null;
            mEngine.fastPreview(nexEngine.FastPreviewOption.cts, 0);
        }
    }

    void hideSoftKeyboard(EditText input) {
        input.setInputType(0);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(input.getWindowToken(), 0);
    }


    @Override
    protected void onStop() {
        stopPreviewAnimation();
        mEngine.stop();
        super.onStop();
    }
}
