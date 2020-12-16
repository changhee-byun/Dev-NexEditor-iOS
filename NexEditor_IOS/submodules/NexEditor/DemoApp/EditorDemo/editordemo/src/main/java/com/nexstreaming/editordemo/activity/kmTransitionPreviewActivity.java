package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.KeyguardManager;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTPreviewFragment;
import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityCode;
import com.nexstreaming.editordemo.utility.utilityTransitionIcon;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;

public class kmTransitionPreviewActivity extends Activity implements kmSTPreviewFragment.OnSTPreviewListener {

    private static final String TAG = "KM_TransitionPreview";

    private ImageView mIViewTransEffectIcon;
    private TextView mTViewTransEffect;
    private ImageButton mIButton;
    private ProgressBar mProgressBar;

    private ImageView mAlbumArt;
    private TextView mAlbumTitle;
    private TextView mAlbumArtist;
    private TextView mAlbumDuration;

    private MediaMetadataRetriever metadataRetriever;

    private AlertDialog mAlertDialog;

    private dataParcel mDataParcel;
    private utilityTransitionIcon mTransEffectIcon;
    private nexTransitionEffect mTransEffect;
    private nexProject mProject;
    private nexProject mClone;
    private nexEngine mEngine;

    private boolean mIsLCDoff;
    private boolean mIsSecured;
    private boolean mIsRandomTransition;

    private File mFile;

    private final int PLAYING = 1;
    private final int PAUSE = 2;
//    private final int COMPLETE = 3;
    private final int ENCODING = 4;
    private final int ENCODING_COMPLETE = 5;
    private final int IDLE = 6;
    private final int ERROR = 7;

    private int mActivityStatus = PLAYING;
    private int mSeekTS;

    public static Activity previewAct;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        WindowManager.LayoutParams winParams = getWindow().getAttributes();
        winParams.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_IN_OVERSCAN |
                WindowManager.LayoutParams.FLAG_FULLSCREEN |
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
        getWindow().setAttributes(winParams);

        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION /* hide nav bar*/ | View.SYSTEM_UI_FLAG_FULLSCREEN /* hide status bar*/ | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

        kmTransitionListActivity.translistActivity.finish();
        previewAct = this;

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();

        mDataParcel = getIntent().getParcelableExtra("parcelData");

        if(mDataParcel.getNameId()==null) {
            mIsRandomTransition = true;
        }

        boolean is_exist_errclip = false;
        nexEffectLibrary effectLibrary = nexEffectLibrary.getEffectLibrary(this);
        if(mIsRandomTransition) {
            int index = 0;
            String transitionId;
            mProject = new nexProject();

            for(int i=0; i<mDataParcel.getPath().size(); i++) {
                String path = mDataParcel.getPath().get(i);
                if(path == null) {
                    is_exist_errclip = true;
                    break;
                }
                nexClip clip = nexClip.getSupportedClip(path);
                if(clip != null) {
                    mProject.add(clip);
                    transitionId = mDataParcel.getRandomTransitionId(index);
                    if(effectLibrary.findTransitionEffectById(transitionId) != null) {
                        mProject.getClip(index,true).getTransitionEffect().setTransitionEffect(effectLibrary.findTransitionEffectById(transitionId).getId());
                    }
                    if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        retriever.setDataSource(mDataParcel.getPath().get(i));
                        if(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)!=null) {
                            int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                            if(videoRotation == 90) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_270);
                            } else if(videoRotation == 180) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_180);
                            } else if(videoRotation == 270) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_90);
                            }
                            clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        }
                    } else {
                        clip.getCrop().randomizeStartEndPosition(true, nexCrop.CropMode.PAN_FACE);
                    }
                    index++;
                } else {
                    is_exist_errclip = true;
                }
            }

            if(mProject.getTotalVisualClipCount() == 1) {
                nexClip clip = mProject.getClip(0, true);
                int interval = clip.getTotalTime()/3;
                if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                    retriever.setDataSource(mDataParcel.getPath().get(0));
                    if(clip.getTotalTime() > 12000) {
                        for(int i=0; i<2; i++) {
                            mProject.add(nexClip.dup(clip));
                        }
                        for(int j=0 ; j < 3 ; j++){
                            if(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)!=null) {
                                int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                                if(videoRotation == 90) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_270);
                                } else if(videoRotation == 180) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_180);
                                } else if(videoRotation == 270) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_90);
                                }
                            }
                            mProject.getClip(j,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                            mProject.getClip(j,true).getVideoClipEdit().setTrim(interval * j, interval * (j + 1));
                            transitionId = mDataParcel.getRandomTransitionId(j);
                            if(effectLibrary.findTransitionEffectById(transitionId) != null) {
                                mProject.getClip(j,true).getTransitionEffect().setTransitionEffect(effectLibrary.findTransitionEffectById(transitionId).getId());
                            }
                        }
                    }
                } else if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    nexClip clip2 = mProject.getClip(0, true);
                    for(int i=0; i<2; i++) {
                        mProject.add(nexClip.dup(clip2));
                    }
                    for(int j=0; j<3; j++) {
                        mProject.getClip(j,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_FACE);
                        //
                        transitionId = mDataParcel.getRandomTransitionId(j);
                        if(effectLibrary.findTransitionEffectById(transitionId) != null) {
                            mProject.getClip(j,true).getTransitionEffect().setTransitionEffect(effectLibrary.findTransitionEffectById(transitionId).getId());
                        }
                    }
                }
            }
        } else {
            int index = 0;

            mTransEffect = effectLibrary.findTransitionEffectById(mDataParcel.getNameId());
            mTransEffectIcon = new utilityTransitionIcon(getBaseContext(), mTransEffect);
            mProject = new nexProject();

            for(int i=0; i<mDataParcel.getPath().size(); i++) {
                String path = mDataParcel.getPath().get(i);
                if(path == null) {
                    is_exist_errclip = true;
                    break;
                }
                nexClip clip = nexClip.getSupportedClip(path);
                if(clip != null) {
                    mProject.add(clip);
                    mProject.getClip(index, true).getTransitionEffect().setTransitionEffect(mTransEffect.getId());
                    int[] transitionTimes = mProject.getTransitionDurationTimeGuideLine(index, 2000);
                    mProject.getClip(index,true).getTransitionEffect().setDuration(transitionTimes[1]);
                    if(mProject.getClip(index, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                        retriever.setDataSource(mDataParcel.getPath().get(i));
                        if(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)!=null) {
                            int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                            if(videoRotation == 90) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_270);
                            } else if(videoRotation == 180) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_180);
                            } else if(videoRotation == 270) {
                                mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_90);
                            }
                            mProject.getClip(index, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        }
                    } else {
                        mProject.getClip(index, true).getCrop().randomizeStartEndPosition(true, nexCrop.CropMode.PAN_FACE);
                    }
                    index++;
                } else {
                    is_exist_errclip = true;
                }
            }

            if(mProject.getTotalVisualClipCount() == 1) {
                nexClip clip = mProject.getClip(0, true);
                int interval = clip.getTotalTime()/3;
                if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                    retriever.setDataSource(mDataParcel.getPath().get(0));

                    clip.getTransitionEffect().setTransitionEffect(mTransEffect.getId());
                    if(clip.getTotalTime() > 12000) {
                        for(int i=0; i<2; i++) {
                            mProject.add(nexClip.dup(clip));
                        }
                        for(int j=0 ; j<3 ; j++){
                            if(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)!=null) {
                                int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                                if(videoRotation == 90) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_270);
                                } else if(videoRotation == 180) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_180);
                                } else if(videoRotation == 270) {
                                    mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_90);
                                }
                            }
                            mProject.getClip(j,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                            mProject.getClip(j,true).getVideoClipEdit().setTrim(interval * j, interval * (j + 1));
                            mProject.getClip(j,true).getTransitionEffect().setTransitionEffect(mTransEffect.getId());
                        }
                    }
                } else if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    nexClip clip2 = mProject.getClip(0, true);
                    for(int i=0; i<2; i++) {
                        mProject.add(nexClip.dup(clip2));
                    }
                    for(int j=0; j<3; j++) {
                        mProject.getClip(j,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_FACE);
                        //
                        mProject.getClip(j,true).getTransitionEffect().setTransitionEffect(mTransEffect.getId());
                    }
                }
            }
        }

        for(int k=0; k<mProject.getTotalClipCount(true); k++) {
            int[] transitionTimes = mProject.getTransitionDurationTimeGuideLine(k, 2000);
            Log.d(TAG, "transitionTime[0]=" + transitionTimes[0] + " transitionTime[1]=" + transitionTimes[1]);
            if(transitionTimes[1] >= 2000) {
                mProject.getClip(k,true).getTransitionEffect().setDuration(2000);
            } else {
                mProject.getClip(k,true).getTransitionEffect().setDuration(transitionTimes[0]);
            }
        }

        // Set SoundTrack
        try {
            utilityCode.raw2file(this, R.raw.whistling_and_fun, "whistling_and_fun.mp3");
            String soundtrackPath = getFilesDir().getAbsolutePath()+"/whistling_and_fun.mp3";
            mProject.setBackgroundMusicPath(soundtrackPath);
        } catch (Exception e) {
            e.printStackTrace();
        }

        setContentView(R.layout.activity_km_transpreview);

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
                throw new Exception("No implement preview as aspectMode_1v1 in kmTransitionPreviewActivity.");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        getFragmentManager().beginTransaction()
                .replace(R.id.surface, new kmSTPreviewFragment(), "previewFragment")
                .commit();

        nexEngine.setLoadListAsync(true);
        mEngine = kmEngine.getEngine();

        mIButton = (ImageButton) findViewById(R.id.goto_music);
        mIButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(getActivityStatus()<ENCODING) {
                    if(mClone != null) {
                        mClone = null;
                    }
                    mClone = nexProject.clone(mProject);
                    mSeekTS = mEngine.getCurrentPlayTimeTime();
                    mEngine.stop(new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
//                            setActivityStatus(IDLE);
                            Intent intent = new Intent(getBaseContext(), kmMusicListActivity.class);
                            startActivityForResult(intent, 100);
                        }
                    });
                }
            }
        });

        mIViewTransEffectIcon = (ImageView) findViewById(R.id.preview_trans_icon);
        mTViewTransEffect = (TextView) findViewById(R.id.preview_trans_id);

        if(mIsRandomTransition) {
            mIViewTransEffectIcon.setImageResource(R.mipmap.ic_list_avatar_random_40dp);
            mTViewTransEffect.setText("Random");
        } else {
            mTransEffectIcon.loadBitmap(0, null, mIViewTransEffectIcon);
            mTViewTransEffect.setText(mTransEffect.getName(getBaseContext()));
        }

        mAlbumArt = (ImageView) findViewById(R.id.trans_albumart);
        mAlbumTitle = (TextView) findViewById(R.id.trans_albumtitle);
        mAlbumArtist = (TextView) findViewById(R.id.trans_albumartist);
        mAlbumDuration = (TextView) findViewById(R.id.trans_albumduration);

        mAlbumTitle.setText("whistling and fun");
        mAlbumArtist.setText("KineMaster Theme");
        mAlbumDuration.setText("01:51");
        mAlbumArt.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp));

        if(is_exist_errclip) {
            if(mProject.getTotalClipCount(true) == 0) {
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
            mEngine.setProject(mProject);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(data != null) {
            if(resultCode == 1) {
                String soundtrackPath = data.getStringExtra("soundtrack_path");
                if(mClone.setBackgroundMusicPath(soundtrackPath)) {
                    mProject = mClone;
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(mSeekTS);
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
                    setSeek(mSeekTS);
                }
            } else {
                String soundtrackPath = data.getStringExtra("soundtrack_path");
                if(mClone.setBackgroundMusicPath(soundtrackPath)) {
                    mProject = mClone;
                    mEngine.setProject(mProject);
                    mEngine.updateProject();
                    setSeek(mSeekTS);
                    if(soundtrackPath != null) {
                        String album_title;
                        String album_artist;
                        String album_duration;
                        byte[] album_art;

                        metadataRetriever = new MediaMetadataRetriever();
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
                    setSeek(mSeekTS);
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(getActivityStatus() != ERROR) {
            mEngine.stop();
        }
        setActivityStatus(PLAYING);
    }

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
        if(getActivityStatus()!=ENCODING
                && getActivityStatus()!=ENCODING_COMPLETE
                && getActivityStatus()!=ERROR) {
            setPause();
        } else if(getActivityStatus()==ENCODING) {
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    setActivityStatus(IDLE);
                    if (mAlertDialog != null) mAlertDialog.dismiss();
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

        if(!mIsLCDoff && !mIsSecured) {
            if(getActivityStatus()!=ENCODING
                    && getActivityStatus()!=ENCODING_COMPLETE
                    && getActivityStatus()!=ERROR
                    && getActivityStatus()!=PLAYING
                    && mAlertDialog==null) {
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

    private AlertDialog displayProgressDialog(String path) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View progressdialogView = factory.inflate(R.layout.dialog_progress, null);

        TextView tv = (TextView) progressdialogView.findViewById(R.id.tv_filename);
        ImageView iv = (ImageView) progressdialogView.findViewById(R.id.iv_encoding_progress);
        mProgressBar = (ProgressBar) progressdialogView.findViewById(R.id.pbar_encoding);
        mProgressBar.setProgress(0);
        mProgressBar.setMax(100);

        tv.setText(path);
        iv.setImageResource(R.mipmap.ic_movie_24dp);

        return new AlertDialog.Builder(this)
                .setView(progressdialogView)
                .setCancelable(false)
                .create();
    }

    private boolean isFinish = false;

    private AlertDialog displayErrorDialog(String message) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View progressdialogView = factory.inflate(R.layout.dialog_error, null);

        TextView tv = (TextView) progressdialogView.findViewById(R.id.tv_err_message);
        Button btn = (Button) progressdialogView.findViewById(R.id.btn_err_confirm);

        tv.setText(message);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mAlertDialog != null) {
                    mAlertDialog.dismiss();
                    mAlertDialog = null;
                }
                if (isFinish) {
                    finish();
                }
            }
        });

        return new AlertDialog.Builder(this)
                .setView(progressdialogView)
                .setCancelable(false)
                .create();
    }

    private int getActivityStatus() {
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
        mEngine.export(mFile.getAbsolutePath(), 1280, 720, 6 * 1024 * 1024, Long.MAX_VALUE, 0);
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
        mProgressBar.setProgress(percent);
    }

    @Override
    public void onSTActivityBack() {
        dataParcel parcel = dataParcel.getDataParcel(null, mDataParcel.getPath(), null);

        Intent intent = new Intent(getBaseContext(), kmTransitionListActivity.class);
        intent.putExtra("from_preview", 1);
        intent.putExtra("parcelData", parcel);
        startActivity(intent);
    }

    @Override
    public void onSTHome() {
        finish();
    }

    @Override
    public void onSTEncodedFilePlay() {
//        Intent intent = new Intent(Intent.ACTION_VIEW);
//        Uri uri = Uri.parse(mFile.getAbsolutePath());
//        intent.setDataAndType(uri, "video/*");
//        startActivity(intent);
        Intent intent = new Intent(getBaseContext(), kmVideoActivity.class);
        intent.putExtra("path", mFile.getAbsolutePath());
        startActivity(intent);
    }

    private ImageView mIVBack;
    private ImageView mIVPlay;
    private ImageView mIVETC;
    private TextView mTVPlayingTime;
    private TextView mTVTotlaTime;
    private SeekBar mSeekBar;

    @Override
    public void onSTSetupEngine() {
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");

        mIVBack = (ImageView) findViewById(R.id.preview_back);
        mIVETC = (ImageView) findViewById(R.id.preview_etc);
        mIVPlay = (ImageView) findViewById(R.id.preview_play);
        mTVPlayingTime = (TextView) findViewById(R.id.preview_playingTime);
        mTVTotlaTime = (TextView) findViewById(R.id.preview_TotalTime);
        mSeekBar = (SeekBar) findViewById(R.id.preview_seek);

        if(getActivityStatus() != ERROR) {
            fragment.setEngine(mEngine, true);
            fragment.setViews(mIVBack, mIVETC, mIVPlay, mTVPlayingTime, mTVTotlaTime, mSeekBar);
        } else {
            fragment.setEngine(mEngine, false);
            fragment.setViews(mIVBack, mIVETC, mIVPlay, mTVPlayingTime, mTVTotlaTime, mSeekBar);
        }
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
