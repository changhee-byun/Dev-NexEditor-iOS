package com.nexstreaming.editordemo.activity;

import android.app.AlertDialog;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Paint;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.BounceInterpolator;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.utility.utilityColorEffect2Thumbnail;
import com.nexstreaming.nexeditorsdk.nexAnimate;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexOverlayKineMasterExpression;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTAdjustmentFragment;
import com.nexstreaming.editordemo.fragment.kmSTColorEffectFragment;
import com.nexstreaming.editordemo.fragment.kmSTPreviewFragment;
import com.nexstreaming.editordemo.fragment.kmSTSpeedFragment;
import com.nexstreaming.editordemo.fragment.kmSTStickerFragment;
import com.nexstreaming.editordemo.fragment.kmSTTextAnimationFragment;
import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityCode;
import com.nexstreaming.editordemo.utility.utilityThumbnail;
import com.nexstreaming.editordemo.utility.utilityTimelineTransitionIcon;
import com.nexstreaming.editordemo.utility.utilityTextOverlay;
import com.nexstreaming.nexeditorsdk.nexUtils;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;

public class kmEditPreviewActivity extends ActionBarActivity implements View.OnClickListener,
        kmSTSpeedFragment.OnSTSpeedSelectedListener,
        kmSTTextAnimationFragment.OnSTTextSelectedListener,
        kmSTColorEffectFragment.OnSTColorSelectedListener,
        kmSTAdjustmentFragment.OnSTAdjustmentSelectedListener,
        kmSTStickerFragment.OnSTStickerSelectedListener,
        kmSTPreviewFragment.OnSTPreviewListener {

    private static final String TAG = "KM_EditPreviewActivity";

    private ImageView mAlbumArt;
    private TextView mAlbumTitle;
    private TextView mAlbumArtist;
    private TextView mAlbumDuration;

    private ImageButton mIBSpeed;
    private ImageButton mIBText;
    private ImageButton mIBSticker;
    private ImageButton mIBColor;
    private ImageButton mIBAdjustment;

    private FrameLayout mFLSpeed;
    private FrameLayout mFLText;
    private FrameLayout mFLSticker;
    private FrameLayout mFLColor;
    private FrameLayout mFLAdjustment;

    private ImageButton mIButton;

    private ImageView mIVBack;
    private ImageView mIVPlay;
    private ImageView mIVETC;
    private TextView mTVPlayingTime;
    private TextView mTVTotlaTime;
    private SeekBar mSeekBar;

    private ProgressBar mProgressBar;

    private AlertDialog mAlertDialog;

    private dataParcel mDataParcel;

    private nexEngine mEngine;

    private nexClip mSelectedClip;
    private Fragment mSelectedFragment = null;

    private int mCurrentMenuTap = R.id.texteffect;

    private int mSeekTS;

    private File mFile;

    private final static int PLAYING = 1;
    private final static int PAUSE = 2;
//    private final static int COMPLETE = 3;
    private final static int ENCODING = 4;
    private final static int ENCODING_COMPLETE = 5;
    private final static int IDLE = 6;
    private final static int ERROR = 7;

    private static int mActivityStatus = PLAYING;

    private RecyclerView recyclerView;
    private EditListAdapter editListAdapter;

    private static nexProject mProject;
    private static nexProject mEditProject;

    private static utilityThumbnail mThumbnail;
    private static utilityTimelineTransitionIcon mTransitionIcon;

    private static int selectedidx = -1;
    private int selectedTransitionIdx;

    // forced codes.
    // frankly, sticker isn't related with clip.
    private ArrayList<String> stickerEffectList = new ArrayList<>();
    private ArrayList<Integer> textAnimationIdxList = new ArrayList<>();

    private utilityColorEffect2Thumbnail colorEffect2Thumbnail;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        WindowManager.LayoutParams winParams = getWindow().getAttributes();
        winParams.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_IN_OVERSCAN |
                WindowManager.LayoutParams.FLAG_FULLSCREEN |
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
        getWindow().setAttributes(winParams);

        MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();

        mDataParcel = getIntent().getParcelableExtra("parcelData");

        boolean is_exist_errclip = false;
        /*  Process
        **  1. Project Create.
        **  2. Insert Clip to Project, created.
        **  3. Engine Create, Progress Registration.
         */
        mEditProject = new nexProject();

        int index = 0;
        for(int i=0; i<mDataParcel.getPath().size(); i++) {
            String path = mDataParcel.getPath().get(i);
            if(path == null) {
                is_exist_errclip = true;
                break;
            }
            nexClip clip = nexClip.getSupportedClip(path);
            if(clip != null) {
                mEditProject.add(clip);
                if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                    metadataRetriever.setDataSource(mDataParcel.getPath().get(i));
                    if(metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)!=null) {
                        int videoRotation = Integer.parseInt(metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                        if(videoRotation == 90) {
                            mEditProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_270);
                        } else if(videoRotation == 180) {
                            mEditProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_180);
                        } else if(videoRotation == 270) {
                            mEditProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_90);
                        }
                    }
                    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                }
                else {
                    clip.getCrop().randomizeStartEndPosition(true, nexCrop.CropMode.PAN_FACE);
                }
                textAnimationList.add("#Hint:Input Text");
                textAnimationList2.add("#Hint:Input Text");
                mStickerOverlayList.add(-1);
                mTextOverlayList.add(-1);
                textAnimationIdxList.add(1);
                stickerEffectList.add("none");
                index++;
            } else {
                is_exist_errclip = true;
                break;
            }
        }

        // Set SoundTrack
        try {
            utilityCode.raw2file(this, R.raw.whistling_and_fun, "whistling_and_fun.mp3");
            String soundtrackPath = getFilesDir().getAbsolutePath()+"/whistling_and_fun.mp3";
            mEditProject.setBackgroundMusicPath(soundtrackPath);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // MJ/2016.2.2
        colorEffect2Thumbnail = new utilityColorEffect2Thumbnail(mEditProject, getApplicationContext());

        // ETC
        mThumbnail = new utilityThumbnail(getBaseContext(), mEditProject);
        mTransitionIcon = new utilityTimelineTransitionIcon(getBaseContext(), mEditProject);

        setContentView(R.layout.activity_km_edit_preview);

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

        int deviceHeight = displayMetrics.heightPixels;
        int deviceWidth = displayMetrics.widthPixels;

        if(kmEngine.getAspectMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
            FrameLayout layout = (FrameLayout) findViewById(R.id.fragment_preview);
            ViewGroup.LayoutParams params = layout.getLayoutParams();

            params.height = deviceHeight-(int)utilityCode.convertDpToPixel(366, this);

            FrameLayout layout2 = (FrameLayout) findViewById(R.id.surface);
            ViewGroup.LayoutParams params2 = layout2.getLayoutParams();

            params2.height = (int)((deviceWidth*9)/16);
        } else if(kmEngine.getAspectMode() == nexApplicationConfig.kAspectRatio_Mode_1v1) {
            // TODO
            try {
                throw new Exception("No implement preview as aspectMode_1v1 in kmEditPreviewActivity.");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        getFragmentManager().beginTransaction()
                .replace(R.id.surface, new kmSTPreviewFragment(), "previewFragment")
                .commit();

        nexEngine.setLoadListAsync(true);
        mEngine = kmEngine.getEngine();

        // Timeline
        //
        editListAdapter = new EditListAdapter(getBaseContext(), mEditProject);
        editListAdapter.setOnItemClickListener(new EditListAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(int idx) {
                Log.d(TAG, "onItemClick idx=" + idx);
                if (idx == -1) {
                    // Hide Fragment Screen..
                    setInitButton();
                    return;
                }
                if(getActivityStatus() == ENCODING_COMPLETE) {
                    return;
                }
                if (idx % 2 == 0) {
                    // MJ/2016.2.2
                    colorEffect2Thumbnail.evictCache();
                    colorEffect2Thumbnail.setThumbnail(idx / 2);
                    //
                    mSelectedClip = mEditProject.getClip(idx / 2, true);
                    // Video or Image
                    if (mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        setInitButton();

                        setEnabledButton(R.id.speedeffect);
                        setEnabledButton(R.id.texteffect);
                        setEnabledButton(R.id.coloreffect);
                        setEnabledButton(R.id.adjustmentffect);
                        setEnabledButton(R.id.stickerffect);
                        setSelectedButton(R.id.texteffect);

                        mSelectedFragment = new kmSTTextAnimationFragment();
                        FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                        Bundle bundle = new Bundle();
                        bundle.putInt("textAnimation", textAnimationIdxList.get(selectedidx/2));
                        mSelectedFragment.setArguments(bundle);

                        fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment, "textAnimationFragment");
                        fragmentTransaction.commit();
                    } else if (mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                        setInitButton();

                        setEnabledButton(R.id.texteffect);
                        setEnabledButton(R.id.coloreffect);
                        setEnabledButton(R.id.adjustmentffect);
                        setSelectedButton(R.id.texteffect);
                        setEnabledButton(R.id.stickerffect);

                        mSelectedFragment = new kmSTTextAnimationFragment();
                        FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                        Bundle bundle = new Bundle();
                        bundle.putInt("textAnimation", textAnimationIdxList.get(selectedidx/2));
                        mSelectedFragment.setArguments(bundle);

                        fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment, "textAnimationFragment");
                        fragmentTransaction.commit();
                    }
                } else {
                    selectedTransitionIdx = idx;
                    mEngine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            // kmTransitionListActivity
                            Intent intent = new Intent(getBaseContext(), kmTransitionListActivity.class);
                            intent.putExtra("EdittoolData", 100/*OK except for zero.*/);
                            startActivityForResult(intent, 2000);
                        }
                    });
                }
            }
        });

        LinearLayoutManager layoutManager = new LinearLayoutManager(getBaseContext());
        layoutManager.setOrientation(LinearLayoutManager.HORIZONTAL);
        recyclerView = (RecyclerView) findViewById(R.id.recyclerview);
        recyclerView.setLayoutManager(layoutManager);
        recyclerView.setAdapter(editListAdapter);

        mAlbumArt = (ImageView) findViewById(R.id.editor_albumart);
        mAlbumTitle = (TextView) findViewById(R.id.editor_albumtitle);
        mAlbumArtist = (TextView) findViewById(R.id.editor_albumartist);
        mAlbumDuration = (TextView) findViewById(R.id.editor_albumduration);

        mAlbumTitle.setText("whistling and fun");
        mAlbumArtist.setText("KineMaster Theme");
        mAlbumDuration.setText("01:51");
        mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));

        mIButton = (ImageButton) findViewById(R.id.goto_music);
        mIButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (getActivityStatus() < ENCODING
                        || getActivityStatus() == IDLE) {
//                    setPause();
//                    setActivityStatus(IDLE);
                    mEngine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            Intent intent = new Intent(getBaseContext(), kmMusicListActivity.class);
                            intent.putExtra("EdittoolData", 1/*Temp*/);
                            startActivityForResult(intent, 100);
                        }
                    });
                }
            }
        });

        mIBSpeed = (ImageButton) findViewById(R.id.speedeffect);
        mFLSpeed = (FrameLayout) findViewById(R.id.speedeffect_select);
        mIBSpeed.setOnClickListener(this);

        mIBText = (ImageButton) findViewById(R.id.texteffect);
        mFLText = (FrameLayout) findViewById(R.id.texteffect_select);
        mIBText.setOnClickListener(this);

        mIBSticker = (ImageButton) findViewById(R.id.stickerffect);
        mFLSticker = (FrameLayout) findViewById(R.id.stickerffect_select);
        mIBSticker.setOnClickListener(this);

        mIBColor = (ImageButton) findViewById(R.id.coloreffect);
        mFLColor = (FrameLayout) findViewById(R.id.coloreffect_select);
        mIBColor.setOnClickListener(this);

        mIBAdjustment = (ImageButton) findViewById(R.id.adjustmentffect);
        mFLAdjustment = (FrameLayout) findViewById(R.id.adjustmentffect_select);
        mIBAdjustment.setOnClickListener(this);

        setInitButton();

        if(is_exist_errclip) {
            if(mEditProject.getTotalClipCount(true) == 0) {
                Log.d(TAG, "There is a certain unsupported clip in consisting the project.");
                setActivityStatus(ERROR);
            } else {
                setActivityStatus(IDLE);
            }
        }

        if(getActivityStatus() == ERROR) {
            if(mAlertDialog!=null) mAlertDialog.dismiss();
            mAlertDialog = displayErrorDialog("There is a certain unsupported clip in consisting the project.");
            mAlertDialog.show();
            isFinish = true;
        } else {
            mEditProject.updateProject();
            mProject = nexProject.clone(mEditProject);
            mEngine.setProject(mProject);
        }
    }

    void setInitButton() {
        mIBSpeed.setEnabled(false);
        mIBSpeed.setSelected(false);
        mFLSpeed.setBackgroundColor(getResources().getColor(R.color.timelineBackground));

        mIBText.setEnabled(false);
        mIBText.setSelected(false);
        mFLText.setBackgroundColor(getResources().getColor(R.color.timelineBackground));

        mIBSticker.setEnabled(false);
        mIBSticker.setSelected(false);
        mFLSticker.setBackgroundColor(getResources().getColor(R.color.timelineBackground));

        mIBColor.setEnabled(false);
        mIBColor.setSelected(false);
        mFLColor.setBackgroundColor(getResources().getColor(R.color.timelineBackground));

        mIBAdjustment.setEnabled(false);
        mIBAdjustment.setSelected(false);
        mFLAdjustment.setBackgroundColor(getResources().getColor(R.color.timelineBackground));

        mCurrentMenuTap = -1;

        if(mSelectedFragment != null) {
            FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

            fragmentTransaction.remove(mSelectedFragment);
            fragmentTransaction.commit();
        }
    }

    void setEnabledButton(int idx) {
        switch(idx) {
            case R.id.speedeffect:
                mIBSpeed.setEnabled(true);
                break;

            case R.id.texteffect:
                mIBText.setEnabled(true);
                break;

            case R.id.stickerffect:
                mIBSticker.setEnabled(true);
                break;

            case R.id.coloreffect:
                mIBColor.setEnabled(true);
                break;

            case R.id.adjustmentffect:
                mIBAdjustment.setEnabled(true);
                break;
        }
    }

    void setSelectedButton(int idx) {
        switch(idx) {
            case R.id.speedeffect:
                mIBSpeed.setSelected(true);
                mFLSpeed.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                break;

            case R.id.texteffect:
                mIBText.setSelected(true);
                mFLText.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                break;

            case R.id.stickerffect:
                mIBSticker.setSelected(true);
                mFLSticker.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                break;

            case R.id.coloreffect:
                mIBColor.setSelected(true);
                mFLColor.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                break;

            case R.id.adjustmentffect:
                mIBAdjustment.setSelected(true);
                mFLAdjustment.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                break;
        }
    }

    @Override
    public void onClick(View v) {
        setInitButton();

        if(mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
            setEnabledButton(R.id.texteffect);
            setEnabledButton(R.id.speedeffect);
            setEnabledButton(R.id.coloreffect);
            setEnabledButton(R.id.adjustmentffect);
            setEnabledButton(R.id.stickerffect);
        } else if(mSelectedClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
            setEnabledButton(R.id.texteffect);
            setEnabledButton(R.id.coloreffect);
            setEnabledButton(R.id.adjustmentffect);
            setEnabledButton(R.id.stickerffect);
        }

        switch(v.getId()) {
            case R.id.speedeffect:
                if(mCurrentMenuTap != v.getId()) {
                    mCurrentMenuTap = v.getId();

                    mSelectedFragment = new kmSTSpeedFragment();
                    FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                    Bundle bundle = new Bundle();
                    bundle.putInt("speed", mSelectedClip.getVideoClipEdit().getSpeedControl());
                    mSelectedFragment.setArguments(bundle);

                    fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment);
                    fragmentTransaction.commit();
                }
                setSelectedButton(R.id.speedeffect);
                break;

            case R.id.texteffect:
                if(mCurrentMenuTap != v.getId()) {
                    mCurrentMenuTap = v.getId();

                    mSelectedFragment = new kmSTTextAnimationFragment();
                    FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                    Bundle bundle = new Bundle();
                    bundle.putInt("textAnimation", textAnimationIdxList.get(selectedidx/2));
                    mSelectedFragment.setArguments(bundle);

                    fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment, "textAnimationFragment");
                    fragmentTransaction.commit();
                }
                setSelectedButton(R.id.texteffect);
                break;

            case R.id.coloreffect:
                if(mCurrentMenuTap != v.getId()) {
                    mCurrentMenuTap = v.getId();

                    mSelectedFragment = new kmSTColorEffectFragment();
                    FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                    if(mSelectedClip.getColorEffect().getPresetName() != null) {
                        Bundle bundle = new Bundle();
                        bundle.putString("colorEffect", mSelectedClip.getColorEffect().getPresetName());
                        mSelectedFragment.setArguments(bundle);
                    } else {
                        Bundle bundle = new Bundle();
                        bundle.putString("colorEffect", "NONE");
                        mSelectedFragment.setArguments(bundle);
                    }

                    fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment);
                    fragmentTransaction.commit();
                }
                setSelectedButton(R.id.coloreffect);
                break;

            case R.id.adjustmentffect:
                if(mCurrentMenuTap != v.getId()) {
                    mCurrentMenuTap = v.getId();

                    mSelectedFragment = new kmSTAdjustmentFragment();
                    FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                    Bundle bundle = new Bundle();
                    bundle.putInt("bright", mSelectedClip.getBrightness());
                    bundle.putInt("contrast", mSelectedClip.getContrast());
                    bundle.putInt("saturation", mSelectedClip.getSaturation());
                    mSelectedFragment.setArguments(bundle);

                    fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment);
                    fragmentTransaction.commit();
                }
                setSelectedButton(R.id.adjustmentffect);
                break;

            case R.id.stickerffect:
                if(mCurrentMenuTap != v.getId()) {
                    mCurrentMenuTap = v.getId();

                    mSelectedFragment = new kmSTStickerFragment();
                    FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();

                    Bundle bundle = new Bundle();
                    bundle.putString("stickereffect", stickerEffectList.get(selectedidx/2));
                    mSelectedFragment.setArguments(bundle);

                    fragmentTransaction.replace(R.id.fragment_container, mSelectedFragment);
                    fragmentTransaction.commit();
                }
                setSelectedButton(R.id.stickerffect);
                break;
        }
    }

    @Override
    public void onStSpeedSelected(int speed) {
        if(getActivityStatus() == ENCODING_COMPLETE) {
            return;
        }

        int previous_seek = mSelectedClip.getVideoClipEdit().getSpeedControl();
        int original_seekTS = (int)(mEngine.getCurrentPlayTimeTime() * previous_seek / 100);
        int original_duration = (int)(mEngine.getDuration() * previous_seek / 100);

        final int seekTs = (int)(original_seekTS * 100 / speed);

        if(mStickerOverlayList.get(selectedidx / 2) != -1
                && mEditProject.getOverlay(mStickerOverlayList.get(selectedidx / 2)) != null) {
            mEditProject.getOverlay(mStickerOverlayList.get(selectedidx / 2)).setTimePosition(0, original_duration * 100 / speed);
        }

        mSelectedClip.getVideoClipEdit().setSpeedControl(speed);

        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if (mProject != null) {
                    mProject = null;
                }
                mProject = nexProject.clone(mEditProject);
                mEngine.setProject(mProject);
                mEngine.updateProject();
                setSeek(seekTs);
//                setActivityStatus(IDLE);
            }
        });
    }

    private int textAnimationIdx = 1;
    private ArrayList<String> textAnimationList = new ArrayList<>();
    private ArrayList<String> textAnimationList2 = new ArrayList<>();
    private String textFont;
    private Paint.Align textAlign = Paint.Align.CENTER;
    private int textColor = 0xFFFFFFFF;
    private int shadowColor = 0xFF000000;
    private int glowColor = 0xFF848484;
    private int outlineColor =0xFFC2C2C2;
    private float textSize = 100f;
    private boolean shadowVisible = true;
    private boolean glowVisible = true;
    private boolean outlineVisible = true;

    @Override
    public void onStTextAnimationSelected(int ta_idx) {
        if(getActivityStatus() == ENCODING_COMPLETE) {
            return;
        }
        if(ta_idx-1 > 10) {
            // Exception Code.
            return;
        } else if(ta_idx-1 == 0) {
            // In case of "None"
            mEditProject.removeOverlay(mTextOverlayList.get(selectedidx / 2));
            textAnimationIdxList.set(selectedidx / 2, -1);
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(0);
                }
            });
            return;
        }

        textAnimationIdx = ta_idx-1;
        textAnimationIdxList.set(selectedidx / 2, ta_idx);

        Intent intent = new Intent(getBaseContext(), kmTexteffectEdit5Activity.class);
        intent.putExtra("titleText_1st", textAnimationList.get(selectedidx/2));
        intent.putExtra("titleText_2nd", textAnimationList2.get(selectedidx/2));
        if(textAnimationIdx == 3/*POP*/ || textAnimationIdx == 5/*SPIN*/) {
            intent.putExtra("visible_2nd", true);
        }
        startActivityForResult(intent, 1000);
    }

    String presetName = "NONE";

    @Override
    public void onStColorSelected(int ce_idx) {
        if(getActivityStatus() == ENCODING_COMPLETE) {
            return;
        }

        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
        nexColorEffect colorEffect = colorEffects.get(ce_idx);
        mSelectedClip.setColorEffect(colorEffect);

        if (mProject != null) {
            mProject = null;
        }
        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if (mProject != null) {
                    mProject = null;
                }
                int seekTs = mEngine.getCurrentPlayTimeTime();
                mProject = nexProject.clone(mEditProject);
                mEngine.setProject(mProject);
                mEngine.updateProject();
                setSeek(seekTs);
            }
        });
    }

    @Override
    public void onStAdjustmentSelected(String value, int control_value) {
        if(getActivityStatus() == ENCODING_COMPLETE) {
            return;
        }
        if(value.equals("bright")) {
            mSelectedClip.setBrightness(control_value);
        } else if(value.equals("contrast")) {
            mSelectedClip.setContrast(control_value);
        } else if(value.equals("saturation")) {
            mSelectedClip.setSaturation(control_value);
        }
        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if (mProject != null) {
                    mProject = null;
                }
                int seekTs = mEngine.getCurrentPlayTimeTime();
                mProject = nexProject.clone(mEditProject);
                mEngine.setProject(mProject);
                mEngine.updateProject();
                setSeek(seekTs);
            }
        });
    }

    private final int OVERLAYVIEW_WIDTH = 1280; // Fixed Value
    private final int OVERLAYVIEW_HEIGHT = 720; // Fixed Value

    private ArrayList<Integer> mStickerOverlayList = new ArrayList<>();

    @Override
    public void onStStickerSelected(String stickereffect) {
        if(getActivityStatus() == ENCODING_COMPLETE) {
            return;
        }
        stickerEffectList.set(selectedidx / 2, stickereffect);
        Log.d(TAG, "onStStickerSelected value= " + stickereffect + " get=" + stickerEffectList.get(selectedidx / 2));

        if(mStickerOverlayList.get(selectedidx / 2) != -1
                && mEditProject.getOverlay(mStickerOverlayList.get(selectedidx / 2)) != null) {
            mEditProject.removeOverlay(mStickerOverlayList.get(selectedidx / 2));
        }

        if(stickereffect.equals("ilove")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT/2;
            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("ilove", this, R.drawable.ilove0001), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            nexAnimate animate = nexAnimate.getAnimateImages(0, 6000,
                    R.drawable.ilove0001, R.drawable.ilove0002, R.drawable.ilove0003, R.drawable.ilove0004, R.drawable.ilove0005,
                    R.drawable.ilove0006, R.drawable.ilove0007, R.drawable.ilove0008, R.drawable.ilove0009, R.drawable.ilove0010,
                    R.drawable.ilove0011, R.drawable.ilove0012, R.drawable.ilove0013, R.drawable.ilove0014, R.drawable.ilove0015,
                    R.drawable.ilove0016, R.drawable.ilove0017, R.drawable.ilove0018, R.drawable.ilove0019, R.drawable.ilove0020,
                    R.drawable.ilove0021, R.drawable.ilove0022, R.drawable.ilove0023, R.drawable.ilove0024, R.drawable.ilove0025,
                    R.drawable.ilove0026, R.drawable.ilove0027, R.drawable.ilove0028, R.drawable.ilove0029, R.drawable.ilove0030,
                    R.drawable.ilove0031, R.drawable.ilove0032, R.drawable.ilove0033, R.drawable.ilove0034, R.drawable.ilove0035,
                    R.drawable.ilove0036, R.drawable.ilove0037, R.drawable.ilove0038, R.drawable.ilove0039, R.drawable.ilove0040,
                    R.drawable.ilove0041, R.drawable.ilove0042, R.drawable.ilove0043, R.drawable.ilove0044, R.drawable.ilove0045,
                    R.drawable.ilove0046, R.drawable.ilove0047, R.drawable.ilove0048, R.drawable.ilove0049, R.drawable.ilove0050);
            stickerOverlayItem.addAnimate(animate);
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("zoo")) {

            /*zoo's sticker postion: bottom|left*/
            int x_position = 253;
            int y_position = OVERLAYVIEW_HEIGHT-216;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("zoo", this, R.drawable.zoo0001), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            nexAnimate animate = nexAnimate.getAnimateImages(0, 6000,
                    R.drawable.zoo0001, R.drawable.zoo0002, R.drawable.zoo0003, R.drawable.zoo0004, R.drawable.zoo0005,
                    R.drawable.zoo0006, R.drawable.zoo0007, R.drawable.zoo0008, R.drawable.zoo0009, R.drawable.zoo0010,
                    R.drawable.zoo0011, R.drawable.zoo0012, R.drawable.zoo0013, R.drawable.zoo0014, R.drawable.zoo0015,
                    R.drawable.zoo0016, R.drawable.zoo0017, R.drawable.zoo0018, R.drawable.zoo0019, R.drawable.zoo0020,
                    R.drawable.zoo0021, R.drawable.zoo0022, R.drawable.zoo0023, R.drawable.zoo0024, R.drawable.zoo0025,
                    R.drawable.zoo0026, R.drawable.zoo0027, R.drawable.zoo0028, R.drawable.zoo0029, R.drawable.zoo0030,
                    R.drawable.zoo0031, R.drawable.zoo0032, R.drawable.zoo0033, R.drawable.zoo0034, R.drawable.zoo0035,
                    R.drawable.zoo0036, R.drawable.zoo0037, R.drawable.zoo0038, R.drawable.zoo0039, R.drawable.zoo0040,
                    R.drawable.zoo0041, R.drawable.zoo0042, R.drawable.zoo0043, R.drawable.zoo0044, R.drawable.zoo0045,
                    R.drawable.zoo0046, R.drawable.zoo0047, R.drawable.zoo0048, R.drawable.zoo0049, R.drawable.zoo0050,
                    R.drawable.zoo0051, R.drawable.zoo0052, R.drawable.zoo0053, R.drawable.zoo0054, R.drawable.zoo0055,
                    R.drawable.zoo0056, R.drawable.zoo0057, R.drawable.zoo0058, R.drawable.zoo0059, R.drawable.zoo0060);
            stickerOverlayItem.addAnimate(animate);
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("sun")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT/2;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("sun", this, R.drawable.sun0001), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            nexAnimate animate = nexAnimate.getAnimateImages(0, 6000,
                    R.drawable.sun0001, R.drawable.sun0002, R.drawable.sun0003, R.drawable.sun0004, R.drawable.sun0005,
                    R.drawable.sun0006, R.drawable.sun0007, R.drawable.sun0008, R.drawable.sun0009, R.drawable.sun0010,
                    R.drawable.sun0011, R.drawable.sun0012, R.drawable.sun0013, R.drawable.sun0014, R.drawable.sun0015,
                    R.drawable.sun0016, R.drawable.sun0017, R.drawable.sun0018, R.drawable.sun0019, R.drawable.sun0020,
                    R.drawable.sun0021, R.drawable.sun0022, R.drawable.sun0023, R.drawable.sun0024, R.drawable.sun0025,
                    R.drawable.sun0026, R.drawable.sun0027, R.drawable.sun0028, R.drawable.sun0029, R.drawable.sun0030,
                    R.drawable.sun0031, R.drawable.sun0032, R.drawable.sun0033, R.drawable.sun0034, R.drawable.sun0035,
                    R.drawable.sun0036, R.drawable.sun0037, R.drawable.sun0038, R.drawable.sun0039, R.drawable.sun0040,
                    R.drawable.sun0041, R.drawable.sun0042, R.drawable.sun0043, R.drawable.sun0044, R.drawable.sun0045,
                    R.drawable.sun0046, R.drawable.sun0047, R.drawable.sun0048, R.drawable.sun0049, R.drawable.sun0050,
                    R.drawable.sun0051, R.drawable.sun0052, R.drawable.sun0053, R.drawable.sun0054, R.drawable.sun0055,
                    R.drawable.sun0056, R.drawable.sun0057, R.drawable.sun0058, R.drawable.sun0059, R.drawable.sun0060);
            stickerOverlayItem.addAnimate(animate);
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("fish")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT-169;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("fish", this, R.drawable.fish0001), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            nexAnimate animate = nexAnimate.getAnimateImages(0, 6000,
                    R.drawable.fish0001, R.drawable.fish0002, R.drawable.fish0003, R.drawable.fish0004, R.drawable.fish0005,
                    R.drawable.fish0006, R.drawable.fish0007, R.drawable.fish0008, R.drawable.fish0009, R.drawable.fish0010,
                    R.drawable.fish0011, R.drawable.fish0012, R.drawable.fish0013, R.drawable.fish0014, R.drawable.fish0015,
                    R.drawable.fish0016, R.drawable.fish0017, R.drawable.fish0018, R.drawable.fish0019, R.drawable.fish0020,
                    R.drawable.fish0021, R.drawable.fish0022, R.drawable.fish0023, R.drawable.fish0024, R.drawable.fish0025,
                    R.drawable.fish0026, R.drawable.fish0027, R.drawable.fish0028, R.drawable.fish0029, R.drawable.fish0030,
                    R.drawable.fish0031, R.drawable.fish0032, R.drawable.fish0033, R.drawable.fish0034, R.drawable.fish0035,
                    R.drawable.fish0036, R.drawable.fish0037, R.drawable.fish0038, R.drawable.fish0039, R.drawable.fish0040,
                    R.drawable.fish0041, R.drawable.fish0042, R.drawable.fish0043, R.drawable.fish0044, R.drawable.fish0045,
                    R.drawable.fish0046, R.drawable.fish0047, R.drawable.fish0048, R.drawable.fish0049, R.drawable.fish0050,
                    R.drawable.fish0051, R.drawable.fish0052, R.drawable.fish0053, R.drawable.fish0054, R.drawable.fish0055,
                    R.drawable.fish0056, R.drawable.fish0057, R.drawable.fish0058, R.drawable.fish0059, R.drawable.fish0060);
            stickerOverlayItem.addAnimate(animate);
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("christmas_newyear")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT/2;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("christmas_newyear", this, R.drawable.christmas_newyear), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("hello")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT/2;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("hello", this, R.drawable.hello), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("birthday")) {

            int x_position = OVERLAYVIEW_WIDTH/2;
            int y_position = OVERLAYVIEW_HEIGHT/2;

            nexOverlayItem stickerOverlayItem = new nexOverlayItem(new nexOverlayImage("birthday", this, R.drawable.birthday), x_position, y_position, mSelectedClip.getProjectStartTime(), mSelectedClip.getProjectEndTime());
            mEditProject.addOverlay(stickerOverlayItem);
            mEditProject.getOverlay(stickerOverlayItem.getId()).setLayerExpression(nexOverlayKineMasterExpression.FADE);
            mStickerOverlayList.set(selectedidx / 2, stickerOverlayItem.getId());
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        } else if(stickereffect.equals("none")) {
            mStickerOverlayList.set(selectedidx / 2, -1);
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        }
        Log.d(TAG, "onStStickerSelected addOverlay stickerEffect="+stickereffect+" overlayId="+ stickerEffectList.get(selectedidx / 2)
            + " startTime="+mSelectedClip.getProjectStartTime()+" endTime="+mSelectedClip.getProjectEndTime());
    }

    private static class EditListAdapter extends RecyclerView.Adapter<EditListAdapter.ViewHolder> {

        private Context context;
        private nexProject project;
        private int clipCount;
        private ArrayList<Boolean> loadinfolist = new ArrayList<>();
        private ArrayList<nexClip> cliplist = new ArrayList<>();

        public EditListAdapter(Context context, nexProject project) {
            this.project = project;
            this.context = context;
            loadinfolist.clear();
            clipCount = project.getTotalVisualClipCount();
        }

        private final int REFRESH_THUMBNAIL = 1;
        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                if(msg.what == REFRESH_THUMBNAIL) {
                    notifyDataSetChanged();
                }
            }
        } ;

        // Create new views (invoked by the layout manager)
        @Override
        public EditListAdapter.ViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
            View itemLayoutView = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.listitem3_view, null);

            for(int id=0; id<project.getTotalClipCount(true); id++) {
                loadinfolist.add(false);
                cliplist.add(project.getClip(id, true));
            }

            ViewHolder viewHolder = new ViewHolder(itemLayoutView);
            return viewHolder;
        }

        public void releaseResources() {
            loadinfolist.clear();
            cliplist.clear();
        }

        private void setThumbnail(ViewHolder vh, nexClip clip) {
            final ViewHolder viewHolder = vh;
            clip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
                @Override
                public void onLoadThumbnailResult(int event) {
                    //
                    if(cliplist.size()==0) {
                        Log.d(TAG, "cliplist is empty.");
                        return;
                    }
                    //Codes to add for issue.
                    int index = 0;
                    for(Boolean isload: loadinfolist) {
                        if(isload == false) {
                            if(cliplist.get(index).getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                                Log.d(TAG, index+"th clip: image, skip");
                                index++;
                                continue;
                            }
                            Log.d(TAG, index+"th clip: video, get thumbnail");
                            break;
                        }
                        else {
                            Log.d(TAG, index+"th clip: already load, skip");
                            index++;
                        }
                    }
                    //
                    if(event == nexClip.OnLoadVideoClipThumbnailListener.kEvent_Ok ||
                            event == nexClip.OnLoadVideoClipThumbnailListener.kEvent_loadCompleted) {
                        Bitmap bitmap = cliplist.get(index).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density);
                        viewHolder.iv_thumbnail.setImageBitmap(bitmap);
                        loadinfolist.set(index, true);
                        Log.d(TAG, index + "th clip: video, path=" + cliplist.get(index).getPath());
                        if(event == nexClip.OnLoadVideoClipThumbnailListener.kEvent_Ok) {
                            mHandler.sendEmptyMessage(REFRESH_THUMBNAIL);
                        }
                    } else if(event == nexClip.OnLoadVideoClipThumbnailListener.kEvent_mustRetry) {
                        viewHolder.iv_thumbnail.setBackgroundColor(context.getResources().getColor(R.color.timelineBackground));
                        setThumbnail(viewHolder, cliplist.get(index));
                        Log.d(TAG, index + "th clip path=" + cliplist.get(index).getPath() + " ReTry" + " event=" + event);
                    } else if(event < nexClip.OnLoadVideoClipThumbnailListener.kEvent_Ok) {
                        viewHolder.iv_thumbnail.setImageResource(R.mipmap.ic_clip_fail_avi);
                        Log.d(TAG, index + "th clip path=" + cliplist.get(index).getPath() + " Fail" + " event=" + event);
                    }
                }
            });
        }

        // Replace the contents of a view (invoked by the layout manager)
        @Override
        public void onBindViewHolder(ViewHolder vh, int i) {
            nexClip clip = cliplist.get(i / 2);
            int clipType = clip.getClipType();

            /*
             * One Project Composition
             * -> Clip+Transition
            **/
            if((i%2) == 0) {
                vh.fl_clipLayout.setVisibility(View.VISIBLE);
                vh.fl_transitionLayout.setVisibility(View.GONE);

                Log.d(TAG, "onBindViewHolder> i="+i+" cliptype="+clipType);

                if(selectedidx == i) {
                    vh.fl_transparentLayout.setVisibility(View.VISIBLE);
                } else {
                    vh.fl_transparentLayout.setVisibility(View.GONE);
                }

                if (clipType == nexClip.kCLIP_TYPE_VIDEO) {
                    loadinfolist.set(i / 2, false);
                    vh.iv_thumbnail.setImageResource(R.mipmap.ic_clip_background);
                    setThumbnail(vh, cliplist.get(i / 2));
                    vh.iv_typeicon.setImageResource(R.mipmap.ic_clip_avatar_avi_18dp);
                } else if(clipType == nexClip.kCLIP_TYPE_IMAGE) {
                    if(clip.getWidth() < clip.getHeight()) {
                        vh.iv_thumbnail.setScaleType(ImageView.ScaleType.FIT_CENTER);
                    } else {
                        vh.iv_thumbnail.setScaleType(ImageView.ScaleType.FIT_XY);
                    }
                    mThumbnail.loadBitmap(i/2, null, vh.iv_thumbnail);
                    vh.iv_typeicon.setImageResource(R.mipmap.ic_clip_avatar_pic_18dp);
                }
            } else {
                if((i/2+1)!= mEditProject.getTotalVisualClipCount()) {
                    vh.fl_clipLayout.setVisibility(View.GONE);
                    vh.fl_transitionLayout.setVisibility(View.VISIBLE);
                    vh.fl_transparentLayout.setVisibility(View.GONE);
                    mTransitionIcon.loadBitmap(i/2, null, vh.iv_transition);
                } else {
                    vh.fl_clipLayout.setVisibility(View.GONE);
                    vh.fl_transitionLayout.setVisibility(View.GONE);
                    vh.fl_transparentLayout.setVisibility(View.GONE);
                }
            }
        }

        // inner class to hold a reference to each item of RecyclerView
        public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {

            public FrameLayout fl_clipLayout;
            public FrameLayout fl_transitionLayout;
            public FrameLayout fl_transparentLayout;
            public ImageView iv_thumbnail;
            public ImageView iv_typeicon;
            public ImageView iv_transition;

            public ViewHolder(View v) {
                super(v);

                fl_transparentLayout = (FrameLayout) v.findViewById(R.id.layout_transparence);

                fl_clipLayout = (FrameLayout) v.findViewById(R.id.layout_clip);
                iv_thumbnail = (ImageView) v.findViewById(R.id.edit_thumbnail);
                iv_typeicon = (ImageView) v.findViewById(R.id.edit_typeicon);

                fl_transitionLayout = (FrameLayout) v.findViewById(R.id.layout_transition);
                iv_transition = (ImageView) v.findViewById(R.id.edit_transition);

                iv_thumbnail.setOnClickListener(this);
                iv_transition.setOnClickListener(this);
            }

            @Override
            public void onClick(View v) {
                // If selected clip is video clip, refresh
                if((getPosition()%2) == 0) {
                    if(selectedidx == getPosition()) {
                        selectedidx = -1;
                    } else {
                        selectedidx = getPosition();
                    }
                    notifyDataSetChanged();
                    if(onItemClickListener!=null) {
                        onItemClickListener.onItemClick(selectedidx);
                    }
                }
                // Else, return selected value.
                else {
                    if(onItemClickListener!=null) {
                        onItemClickListener.onItemClick(getPosition());
                    }
                }
            }
        }

        // Return the size of your itemsData (invoked by the layout manager)
        @Override
        public int getItemCount() {
            return clipCount*2;
        }

        private OnItemClickListener onItemClickListener;

        public void setOnItemClickListener(OnItemClickListener listener) {
            onItemClickListener = listener;
        }

        public interface OnItemClickListener {
            public void onItemClick(int idx);
        }
    }

    private ArrayList<Integer> mTextOverlayList = new ArrayList<>();

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (data != null) {
            if(resultCode == 1018) {
                kmSTTextAnimationFragment fragment = (kmSTTextAnimationFragment)getFragmentManager().findFragmentByTag("textAnimationFragment");
                fragment.rollBack();

                mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        if (mProject != null) {
                            mProject = null;
                        }
                        mProject = nexProject.clone(mEditProject);
                        mEngine.setProject(mProject);
                        mEngine.updateProject();
                        setSeek(mEngine.getCurrentPlayTimeTime());
                    }
                });
            }else if(resultCode == 1000) {
                kmSTTextAnimationFragment fragment = (kmSTTextAnimationFragment)getFragmentManager().findFragmentByTag("textAnimationFragment");
                fragment.updatePrevInfo();

                // Text Animation
                String text_1st = data.getStringExtra("titleText_1st");
                String text_2nd = data.getStringExtra("titleText_2nd");

                String text = null;
                if((text_1st != null/* KMSA-704 && text_1st.length() != 0*/) &&
                        (text_2nd != null/* && KMSA-704 text_2nd.length() != 0*/)) {
                    if(text_1st.length() == 0) text_1st = "";
                    if(text_2nd.length() == 0) text_2nd = "";
                    if(text_1st.length() != 0 && text_2nd.length() != 0) {
                        text = text_1st + "\n" + text_2nd;
                    } else if(text_1st.length() != 0) {
                        text = text_1st;
                    } else {
                        text = text_2nd;
                    }
                    textAnimationList.set(selectedidx / 2, text_1st);
                    textAnimationList2.set(selectedidx / 2, text_2nd);
                } else if((text_1st != null/* && KMSA-704 text_1st.length() != 0*/)) {
                    if(text_1st.length() == 0) text_1st = "";
                    text = text_1st;
                    textAnimationList.set(selectedidx / 2, text_1st);
                } else if((text_2nd != null/* && KMSA-704 text_2nd.length() != 0*/)) {
                    if(text_2nd.length() == 0) text_2nd = "";
                    text = text_2nd;
                    textAnimationList2.set(selectedidx / 2, text_2nd);
                } else {
                    if (mProject != null) {
                        mProject = null;
                    }
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(0);
                    return;
                }

                int overlay_xpos = 1280;//Fixed Value
                int overlay_ypos = 720;//Fixed Value

                switch(textAnimationIdx) {
                    case 1:
                    {//basic
                        textFont = "fonts/droidsans_bold.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        break;
                    }
                    case 2:
                    {//fade
                        textFont = "fonts/berkshireswash_regular.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        break;
                    }
                    case 3:
                    {//pop
                        textFont = "fonts/lato_black.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowVisible = false;
                        glowVisible = false;
                        outlineVisible = false;
                        text=text.toUpperCase(Locale.US);
                        break;
                    }
                    case 4:
                    {//slide
                        textFont = "fonts/knewave.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        break;
                    }
                    case 5:
                    {//spin
                        textFont = "fonts/fascinateinline_regular.otf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        text=text.toUpperCase(Locale.US);
                        break;
                    }
                    case 6:
                    {//drop
                        textFont = "fonts/codystar_regular.ttf";
                        textSize = 168;//56dp
                        textColor = 0xFFFFFFFF;
                        shadowVisible = false;
                        glowVisible = false;
                        outlineVisible = false;
                        break;
                    }
                    case 7:
                    {//scale
                        textFont = "fonts/blackopsone_regular.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowVisible = false;
                        glowVisible = false;
                        outlineVisible = false;
                        text=text.toUpperCase(Locale.US);
                        break;
                    }
                    case 8:
                    {//floating
                        textFont = "fonts/exo_bold.otf";
                        textSize = 150;//50dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        text=text.toUpperCase(Locale.US);
                        break;
                    }
                    case 9:
                    {//drifting
                        textFont = "fonts/berkshireswash_regular.ttf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowColor = 0xFF000000;
                        shadowVisible = true;
                        glowVisible = false;
                        outlineVisible = false;
                        break;
                    }
                    case 10:
                    {//squishing
                        textFont = "fonts/exo_bold.otf";
                        textSize = 156;//52dp
                        textColor = 0xFFFFFFFF;
                        shadowVisible = false;
                        glowVisible = false;
                        outlineVisible = false;
                        text=text.toUpperCase(Locale.US);
                        break;
                    }
                    case 0:
                    default:
                    {
                        Log.d(TAG, "Wrong Parameter:"+textAnimationIdx);
                        break;
                    }
                }

                int startTime = (mSelectedClip.getProjectEndTime() - mEngine.getCurrentPlayTimeTime()) < 5000/*text displayTime*/ ? mSelectedClip.getProjectEndTime() - 5000/*text displayTime*/ : mEngine.getCurrentPlayTimeTime();
                int endTime = (mSelectedClip.getProjectEndTime() - startTime) < 5000/*text displayTime*/ ? mSelectedClip.getProjectEndTime() - 1000/**/ : startTime + 5000/*text displayTime*/;

                utilityTextOverlay textOverlay = new utilityTextOverlay(getBaseContext(), text, 1280/*Fixed Value*/, 720/*Fixed Value*/, 1);
                nexOverlayItem textOverlayItem = new nexOverlayItem(new nexOverlayImage("demoText", textOverlay), overlay_xpos/2, overlay_ypos/2, startTime, endTime);

                textOverlay.setText(textSize, textColor, textAlign, textFont);
                if(shadowVisible) {
                    float shadowRadius = 5f;
                    float shadowDx = 3f;
                    float shadowDy = 3f;
                    textOverlay.setTextShadow(true, shadowColor, shadowRadius, shadowDx, shadowDy);
                }
                if(glowVisible) {
                    float glowRadius = 8f;
                    textOverlay.setTextGlow(true, glowColor, glowRadius);
                }
                if(outlineVisible) {
                    float outlineWidth = 1f;
                    textOverlay.setTextOutline(true, outlineColor, outlineWidth);
                }
                textOverlay.setTextOverlayId(textOverlayItem.getId());

                if(mEditProject.getOverlay(mTextOverlayList.get(selectedidx / 2)) != null) {
                    mEditProject.removeOverlay(mTextOverlayList.get(selectedidx / 2));
                }

                mEditProject.addOverlay(textOverlayItem);

                if(textAnimationIdx == 2) { // fade
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - startTime - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 3) { // pop
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getScale(0, 1000/*expressDuration*/, 0, 0, 1, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getScale(endTime - startTime  - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 1, 2, 2).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 4) { // slide
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getMove(0, 1000/*expressDuration*/, new nexAnimate.MoveTrackingPath() {
                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if (coordinate == nexAnimate.kCoordinateX) {
                                return 100 - (100 * timeRatio);
                            }
                            return 0;
                        }
                    }).setInterpolator(new AccelerateDecelerateInterpolator()));

                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getMove(endTime - startTime  - 1000/*expressDuration*/, 1000/*expressDuration*/, new nexAnimate.MoveTrackingPath() {
                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if (coordinate == nexAnimate.kCoordinateX) {
                                return -100 * timeRatio;
                            }
                            return 0;
                        }
                    }).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 5) { // spin
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getRotate(0, 1000, true, 360 * 2, null).setInterpolator(new AccelerateDecelerateInterpolator()));

                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getRotate(endTime - startTime  - 1000/*expressDuration*/, 1000, false, 360 * 2, null).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 6) { // drop
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new BounceInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getMove(0, 1000/*expressDuration*/, new nexAnimate.MoveTrackingPath() {
                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if (coordinate == nexAnimate.kCoordinateY) {
                                return 1200 - (1200 * timeRatio);
                            }
                            return 0;
                        }
                    }).setInterpolator(new BounceInterpolator()));

                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - startTime  - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 7) { // scale
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getScale(0, 1000/*expressDuration*/, 0, 0, 1, 1).setInterpolator(new AccelerateDecelerateInterpolator()));

                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getScale(endTime - startTime  - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 1, 0, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 8) { // floating
                    final int duration = endTime;
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getMove(0, duration, new nexAnimate.MoveTrackingPath() {
                        private float _cx = 0;
                        private float _cy = 0;
                        private float _dx = 0;
                        private float _dy = 0;
                        private int time = 0;
                        private boolean _cal = true;

                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if (_cal) {
                                float cx = _cx / 500 + duration * 10;
                                float cy = _cy / 500 + duration * 10;
                                double dx = 40 * nexUtils.noise(cx, cy, (float) time / 500);
                                double dy = 40 * nexUtils.noise(cx + 7534.5431, cy + 123.432, (float) time / 500);
                                double dx2 = 10 * nexUtils.noise(cx + 543, cy + 823.25, (float) time / 300 + 234);
                                double dy2 = 10 * nexUtils.noise(cx + 734.5431, cy + 13.432, (float) time / 300 + 567);

                                _cx = cx;
                                _cy = cy;
                                _dx = (float) (dx + dx2);
                                _dy = (float) (dy + dy2);
                                _cal = false;

                                time += 33;
                            }
                            if (coordinate == nexAnimate.kCoordinateX) {
                                return _dx;
                            } else if (coordinate == nexAnimate.kCoordinateY) {
                                return _dy;
                            } else {
                                _cal = true;
                            }
                            return 0;
                        }
                    }).setInterpolator(new AccelerateDecelerateInterpolator()));

                    textOverlayItem.addAnimate(nexAnimate.getAlpha(duration - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 9) { // drifting
                    final int duration = endTime;
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getMove(0, duration, new nexAnimate.MoveTrackingPath() {
                        private float _cx = 0;
                        private float _cy = 0;
                        private float _dx = 0;
                        private float _dy = 0;
                        private int time = 0;
                        private boolean _cal = true;
                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if( _cal ) {
                                float cx = _cx / 500 + duration*10;
                                float cy = _cy / 500 + duration*10;
                                double dx = 40 * nexUtils.noise(cx, cy, (float) time / 1000);
                                double dy = 40 * nexUtils.noise(cx + 7534.5431, cy + 123.432, (float) time / 1000);
                                double dx2 = 10 * nexUtils.noise(cx + 543, cy + 823.25, (float) time / 500 + 234);
                                double dy2 = 10 * nexUtils.noise(cx + 734.5431, cy + 13.432, (float) time / 500 + 567);

                                _cx = cx;
                                _cy = cy;
                                _dx = (float) (dx + dx2);
                                _dy = (float) (dy + dy2);
                                _cal = false;

                                time += 33;
                            }
                            if (coordinate == nexAnimate.kCoordinateX) {
                                return _dx;
                            }else if( coordinate == nexAnimate.kCoordinateY ) {
                                return _dy;
                            }else{
                                _cal = true;
                            }
                            return 0;
                        }
                    }).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(duration - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                } else if(textAnimationIdx == 10) { // squishing
                    textOverlayItem.clearAnimate();
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(0, 1000/*expressDuration*/, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getScale(0, endTime, new nexAnimate.MoveTrackingPath() {
                        int time = 0;
                        @Override
                        public float getTranslatePosition(int coordinate, float timeRatio) {
                            if (coordinate == nexAnimate.kCoordinateX){
                                return (float) (Math.sin((float)time/400f)*0.2f );
                            }else if( coordinate == nexAnimate.kCoordinateY ){
                                return (float) (Math.cos((float)time/400f)*0.2f );
                            }else{
                                time += 33;
                            }
                            return 1;
                        }
                    }).setInterpolator(new AccelerateDecelerateInterpolator()));
                    textOverlayItem.addAnimate(nexAnimate.getAlpha(endTime - startTime  - 1000/*expressDuration*/, 1000/*expressDuration*/, 1, 0).setInterpolator(new AccelerateDecelerateInterpolator()));
                }

                mTextOverlayList.set(selectedidx / 2, textOverlayItem.getId());

                mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        if (mProject != null) {
                            mProject = null;
                        }
                        mProject = nexProject.clone(mEditProject);
                        mEngine.setProject(mProject);
                        mEngine.updateProject();
                        setSeek(mEngine.getCurrentPlayTimeTime());
                    }
                });
            } else if(resultCode == 2000) {
                // Transition Effect
                String transition_effect = data.getStringExtra("parcelDataString");
                Log.d(TAG, "transition_effect:"+transition_effect+" selectedTransitionIdx:"+selectedTransitionIdx/2);
                mEditProject.getClip(selectedTransitionIdx/2, true).getTransitionEffect().setTransitionEffect(transition_effect);
                int[] transitionTimes = mEditProject.getTransitionDurationTimeGuideLine(selectedTransitionIdx/2, 2000);
                Log.d(TAG, "transitionTime[0]=" + transitionTimes[0] + " transitionTime[1]=" + transitionTimes[1]);
                if(transitionTimes[1] >= 2000) {
                    mEditProject.getClip(selectedTransitionIdx/2,true).getTransitionEffect().setDuration(2000);
                } else {
                    mEditProject.getClip(selectedTransitionIdx/2,true).getTransitionEffect().setDuration(transitionTimes[0]);
                }
                int seekTs = mEngine.getCurrentPlayTimeTime();
                mProject = nexProject.clone(mEditProject);
                mEngine.setProject(mProject);
                mEngine.updateProject();
                setSeek(seekTs);
                editListAdapter.notifyDataSetChanged();
            } else if(resultCode == 1) {
                // Music <Theme>
                String soundtrackPath = data.getStringExtra("soundtrack_path");
                if(mEditProject.setBackgroundMusicPath(soundtrackPath)) {
                    mEngine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            if (mProject != null) {
                                mProject = null;
                            }
                            int seekTs = mEngine.getCurrentPlayTimeTime();
                            mProject = nexProject.clone(mEditProject);
                            mEngine.setProject(mProject);
                            mEngine.updateProject();
                            setSeek(seekTs);
                        }
                    });
                    if(soundtrackPath != null) {
                        if(soundtrackPath.contains("/baby_music.aac")) {
                            mAlbumTitle.setText("baby");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("00:51");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baby));
                        }
                        else if(soundtrackPath.contains("/baseball_music.aac")) {
                            mAlbumTitle.setText("baseball");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("01:00");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_baseball));
                        }
                        else if(soundtrackPath.contains("/iloveyou_music.aac")) {
                            mAlbumTitle.setText("iloveyou");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("01:00");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_iloveyou));
                        }
                        else if(soundtrackPath.contains("/spring_music.aac")) {
                            mAlbumTitle.setText("spring");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("01:15");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_spring));
                        }
                        else if(soundtrackPath.contains("/themepark_music.aac")) {
                            mAlbumTitle.setText("themepark");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("00:52");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_themepark));
                        }
                        else {
                            mAlbumTitle.setText("whistling and fun");
                            mAlbumArtist.setText("KineMaster Theme");
                            mAlbumDuration.setText("01:51");
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
                        }
                    }
                } else {
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    setSeek(seekTs);
                }
            } else {
                // Music
                String soundtrackPath = data.getStringExtra("soundtrack_path");
                if(mEditProject.setBackgroundMusicPath(soundtrackPath)) {
                    mEngine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            if (mProject != null) {
                                mProject = null;
                            }
                            int seekTs = mEngine.getCurrentPlayTimeTime();
                            mProject = nexProject.clone(mEditProject);
                            mEngine.setProject(mProject);
                            mEngine.updateProject();
                            setSeek(seekTs);
                        }
                    });
                    if(soundtrackPath != null) {
                        String album_title;
                        String album_artist;
                        String album_duration;
                        byte[] album_art;

                        MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
                        metadataRetriever.setDataSource(soundtrackPath);

                        album_title = data.getStringExtra("album_title");
                        album_artist = data.getStringExtra("alubm_artist");
                        album_duration = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
                        album_art = metadataRetriever.getEmbeddedPicture();

                        if(album_title != null)
                            mAlbumTitle.setText(album_title);
                        if(album_artist != null)
                            mAlbumArtist.setText(album_artist);
                        if(album_duration != null) {
                            int duration = Integer.parseInt(album_duration);
                            mAlbumDuration.setText(utilityCode.stringForTime(duration));
                        }
                        if(album_art!=null
                                && album_art.length != 0) {
                            InputStream is = new ByteArrayInputStream(album_art);
                            Bitmap bm = BitmapFactory.decodeStream(is);
                            mAlbumArt.setImageBitmap(bm);
                        } else {
                            mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));
                        }
                    }
                } else {
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    setSeek(seekTs);
                }
            }
        } else {
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    if (mProject != null) {
                        mProject = null;
                    }
                    int seekTs = mEngine.getCurrentPlayTimeTime();
                    mProject = nexProject.clone(mEditProject);
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(seekTs);
                }
            });
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        Log.d(TAG, "onDestroy");

        loaded_index = 0;
        selectedidx = -1;

        if(getActivityStatus() != ERROR) {
            mEditProject.clearOverlay();
            mEngine.stop();
            mEngine.clearTrackCache();
        }

        mSelectedClip = null;
        mSelectedFragment = null;

        setActivityStatus(PLAYING);

        mTextOverlayList.clear();
        mStickerOverlayList.clear();
        stickerEffectList.clear();
        textAnimationList.clear();
        textAnimationList2.clear();
        textAnimationIdxList.clear();

        editListAdapter.releaseResources();
        colorEffect2Thumbnail.releaseResources();
    }

    private boolean mIsLCDoff;
    private boolean mIsSecured;

    @Override
    protected void onPause() {
        super.onPause();

        KeyguardManager keyguardManager = (KeyguardManager) getSystemService(KEYGUARD_SERVICE);
        if(keyguardManager.inKeyguardRestrictedInputMode()) mIsSecured = true;

        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        boolean isScreenOn = powerManager.isScreenOn();
        if(!isScreenOn) {
            mIsLCDoff = true;
        } else {
            mIsLCDoff = false;
        }

        if(getActivityStatus()!= ENCODING
                && getActivityStatus()!=ENCODING_COMPLETE
                && getActivityStatus()!=ERROR) {
            setPause();
        }  else if(getActivityStatus()==ENCODING) {
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    setActivityStatus(IDLE);
                    if(mAlertDialog!=null) mAlertDialog.dismiss();
                    mAlertDialog = displayErrorDialog("Please do not lock your device or switch to another application during the export duration.");
                    mAlertDialog.show();
                    isFinish = true;
                }
            });
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION /* hide nav bar*/ | View.SYSTEM_UI_FLAG_FULLSCREEN /* hide status bar*/ | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        getWindow().getDecorView().invalidate();

        if(!mIsLCDoff && !mIsSecured) {
            if (getActivityStatus() != ENCODING
                    && getActivityStatus() != ENCODING_COMPLETE
                    && getActivityStatus() != ERROR
                    && getActivityStatus() != PLAYING
                    && mAlertDialog == null) {
                setPlay();
            }
        }
    }

    private void setSeek(int seekTS) {
        setActivityStatus(PLAYING);
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");
        fragment.setSeek(seekTS);
    }

    private void setPlay() {
        setActivityStatus(PLAYING);
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");
        fragment.setPlay(true);
    }

    private void setPause() {
        setActivityStatus(PAUSE);
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");
        fragment.setPause();
    }

    private AlertDialog displayProgressDialog(String context) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View progressdialogView = factory.inflate(R.layout.dialog_progress, null);

        TextView tv = (TextView) progressdialogView.findViewById(R.id.tv_filename);
        ImageView iv = (ImageView) progressdialogView.findViewById(R.id.iv_encoding_progress);
        mProgressBar = (ProgressBar) progressdialogView.findViewById(R.id.pbar_encoding);
        mProgressBar.setProgress(0);
        mProgressBar.setMax(100);

        tv.setText(context);
        iv.setImageResource(R.mipmap.ic_movie_24dp);

        return new AlertDialog.Builder(this)
                .setView(progressdialogView)
                .setCancelable(false)
                .create();
    }

    private boolean isFinish = false;

    private AlertDialog displayErrorDialog(String message) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View dialogView = factory.inflate(R.layout.dialog_error, null);

        TextView tv = (TextView) dialogView.findViewById(R.id.tv_err_message);
        Button btn = (Button) dialogView.findViewById(R.id.btn_err_confirm);

        tv.setText(message);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mAlertDialog.dismiss();
                mAlertDialog = null;
                if(isFinish) {
                    finish();
                }
            }
        });

        return new AlertDialog.Builder(this)
                .setView(dialogView)
                .setCancelable(false)
                .create();
    }

    private static int getActivityStatus() {
        return mActivityStatus;
    }

    private void setActivityStatus(int status) {
        mActivityStatus = status;
    }

    private File getExportFile() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "NexDemo" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NEditor_" + export_time+".mp4");
        return exportFile;
    }

    @Override
    public void onSTEncodingStart() {
        if(getActivityStatus() == ERROR) {
            return;
        }
        mFile = getExportFile();
        mAlertDialog = displayProgressDialog(mFile.getName());
        mAlertDialog.show();
        setActivityStatus(ENCODING);
        mEngine.export(mFile.getAbsolutePath(), 1280, 720, 5 * 1024 * 1024, Long.MAX_VALUE, 0);
    }

    @Override
    public void onSTEncodingSuccess() {
        mAlertDialog.dismiss();
        mAlertDialog = null;
        setActivityStatus(ENCODING_COMPLETE);
        if(mFile != null) {
            // To update files at Gallery
            //
            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
        }
    }

    @Override
    public void onSTEncodingFail(int error) {
        mAlertDialog.dismiss();
        if (mFile != null) {
            mFile.delete();
            mFile = null;
        }
        setActivityStatus(IDLE);
        if(mAlertDialog!=null) mAlertDialog.dismiss();

        mAlertDialog = displayErrorDialog("Error<"+error+"> in encoding.");
        mAlertDialog.show();
        isFinish = true;
    }

    @Override
    public void onSTEncodingProgress(int percent) {
        Log.d(TAG, "Encoding="+percent);
        mProgressBar.setProgress(percent);
    }

    @Override
    public void onSTActivityBack() {
        finish();
    }

    @Override
    public void onSTHome() {
        finish();
    }

    @Override
    public void onSTEncodedFilePlay() {
        Intent intent = new Intent(getBaseContext(), kmVideoActivity.class);
        intent.putExtra("path", mFile.getAbsolutePath());
        startActivity(intent);
    }

    static int loaded_index = 0;

    @Override
    public void onSTSetupEngine() {
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");

        mIVBack = (ImageView) findViewById(R.id.preview_back);
        mIVETC = (ImageView) findViewById(R.id.preview_etc);
        mIVPlay = (ImageView) findViewById(R.id.preview_play);
        mTVPlayingTime = (TextView) findViewById(R.id.preview_playingTime);
        mTVTotlaTime = (TextView) findViewById(R.id.preview_TotalTime);
        mSeekBar = (SeekBar) findViewById(R.id.preview_seek);

        fragment.setEngine(mEngine, false);
        fragment.setViews(mIVBack, mIVETC, mIVPlay, mTVPlayingTime, mTVTotlaTime, mSeekBar);

        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if (mEditProject.getTotalClipCount(true) != 0) {
                    mAlertDialog = displayProgressDialog("Collecting Thumbnails of Clips");
                    mAlertDialog.show();

                    for (int i = 0; i < mEditProject.getTotalClipCount(true); i++) {
                        nexClip clip = mEditProject.getClip(i, true);
                        if (clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                            Log.d(TAG, i + "th is a video clip and request thumbnail");
                            clip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {

                                @Override
                                public void onLoadThumbnailResult(int event) {
                                    Log.d(TAG, loaded_index + "th thumbnail resonse");
                                    loaded_index++;
                                    mProgressBar.setProgress(loaded_index * 100 / mEditProject.getTotalClipCount(true));
                                    if (loaded_index == mEditProject.getTotalClipCount(true)) {
                                        loaded_index = 0;
                                        mAlertDialog.dismiss();
                                        editListAdapter.notifyDataSetChanged();
                                        setPlay();
                                    }
                                }
                            });
                        } else {
                            Log.d(TAG, i + "th is an image clip");
                            loaded_index++;
                            if (loaded_index == mEditProject.getTotalClipCount(true)) {
                                loaded_index = 0;
                                mAlertDialog.dismiss();
                                editListAdapter.notifyDataSetChanged();
                                setPlay();
                            }
                        }
                    }
                }
            }
        });
    }

    @Override
    public void onSTPlayFail(int error) {

    }

    @Override
    public void onSTSeekFail(int result) {
        final int error = result;
        if(mAlertDialog!=null) mAlertDialog.dismiss();
        mAlertDialog = displayErrorDialog("Error<"+error+"> in seeking.");
        mAlertDialog.show();
        isFinish = true;
    }
}
