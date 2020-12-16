package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.KeyguardManager;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Color;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.utility.utilityCode;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTheme;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTPreviewFragment;
import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityThemeIcon;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

public class kmThemePreviewActivity extends Activity implements kmSTPreviewFragment.OnSTPreviewListener {

    private static final String TAG = "KM_ThemePreviewActivity";

    private ImageView mIViewThemeIcon;
    private TextView mTViewTheme;

    private ProgressBar mProgressBar;

    private EditText mETextOpening;
    private EditText mETextEnding;

    private AlertDialog mAlertDialog;

    private dataParcel mDataParcel;
    private utilityThemeIcon mThemeIcon;
    private nexTheme mTheme;
    private nexProject mProject;
    private nexEngine mEngine;

    private boolean mIsLCDoff;
    private boolean mIsSecured;

    private File mFile;

    public static Activity previewAct;

    private final int PLAYING = 1;
    private final int PAUSE = 2;
//    private final int COMPLETE = 3;
    private final int ENCODING = 4;
    private final int ENCODING_COMPLETE = 5;
    private final int IDLE = 6;
    private final int ERROR = 7;

    private int mActivityStatus = PLAYING;

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

        kmTitleEditActivity.titleditAct.finish();
        previewAct = this;

        mDataParcel = getIntent().getParcelableExtra("parcelData");
        mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById(mDataParcel.getNameId());
        mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);

        int index = 0;
        boolean is_exist_errclip = false;

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        /*  Process
        **  1. Project Create.
        **  2. Insert Clip to Project, created.
        **  3. Engine Create, Progress Registration.
         */
        mProject = new nexProject(mTheme.getId(),
                mDataParcel.getOpeningTitle().length()==0?null:mDataParcel.getOpeningTitle(),
                mDataParcel.getEndingTitle().length()==0?null:mDataParcel.getEndingTitle());

        for(int i=0; i<mDataParcel.getPath().size(); i++) {
            nexClip clip = nexClip.getSupportedClip(mDataParcel.getPath().get(i));
            if(clip != null) {
                mProject.add(clip);
                if(mProject.getTotalClipCount(true) != 0
                        && mProject.getClip(index, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                    retriever.setDataSource(mDataParcel.getPath().get(i));
                    int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
                    if(videoRotation == 90) {
                        mProject.getClip(index, true).setRotateDegree(nexClip.kClip_Rotate_270);
                    }
                    mProject.getClip(index, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                } else {
                    mProject.getClip(index, true).getCrop().randomizeStartEndPosition(true, nexCrop.CropMode.PAN_FACE);
                }
                index++;
            } else {
                is_exist_errclip = true;
            }
        }

        if(mProject.getTotalClipCount(true) == 1) {
            nexClip clip = mProject.getClip(0, true);
            int interval = clip.getTotalTime()/3;
            if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                retriever.setDataSource(mDataParcel.getPath().get(0));
                int videoRotation = Integer.parseInt(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));

                if(clip.getTotalTime() > 12000) {
                    for(int i=0; i<2; i++) {
                        mProject.add(nexClip.dup(clip));
                    }
                    for(int j=0 ; j < 3 ; j++){
                        if(videoRotation == 90) {
                            mProject.getClip(j, true).setRotateDegree(nexClip.kClip_Rotate_270);
                        }
                        mProject.getClip(j,true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        mProject.getClip(j,true).getVideoClipEdit().setTrim(interval * j, interval * (j+1));
                    }
                }
            } else if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                nexClip c = mProject.getClip(0, true);
                for(int i=0; i<2; i++) {
                    mProject.add(nexClip.dup(c));
                    mProject.getClip(i,true).getCrop().randomizeStartEndPosition(true, nexCrop.CropMode.PAN_FACE);
                }
            }
        }

        if(is_exist_errclip) {
            if(mProject.getTotalClipCount(true) == 0) {
                if(mAlertDialog!=null) mAlertDialog.dismiss();
                mAlertDialog = displayErrorDialog("No item to support in the Project.");
                mAlertDialog.show();
                isFinish = true;
            }
        }

        setContentView(R.layout.activity_km_themepreview);

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
                throw new Exception("No implement preview as aspectMode_1v1 in kmThemePreviewActivity.");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        getFragmentManager().beginTransaction()
                .replace(R.id.fragment_preview, new kmSTPreviewFragment(), "previewFragment")
                .commit();

        nexEngine.setLoadListAsync(true);
        mEngine = kmEngine.getEngine();

        mIViewThemeIcon = (ImageView) findViewById(R.id.preview_theme_icon);
        mThemeIcon.loadBitmap(0, null, mIViewThemeIcon);

        mTViewTheme = (TextView) findViewById(R.id.preview_theme_id);
        mTViewTheme.setText(mTheme.getName(getBaseContext()));

        mETextOpening = (EditText) findViewById(R.id.preview_opening);
        mETextOpening.setHintTextColor(Color.parseColor("#474747"));
        mETextOpening.setHint(mDataParcel.getOpeningTitle());
        mETextOpening.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setActivityStatus(IDLE);
                mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        ArrayList<String> titlelist = new ArrayList<>();
                        titlelist.add(mETextOpening.getHint().toString());
                        titlelist.add(mETextEnding.getHint().toString());
                        dataParcel Theme = dataParcel.getDataParcel(mDataParcel.getNameId(), mDataParcel.getPath(), titlelist);

                        Intent intent = new Intent(getBaseContext(), kmTitleEdit2Activity.class);
                        intent.putExtra("from_edit", 0);
                        intent.putExtra("parcelData", Theme);
                        startActivityForResult(intent, 100);
                    }
                });
            }
        });

        mETextEnding = (EditText) findViewById(R.id.preview_ending);
        mETextEnding.setHintTextColor(Color.parseColor("#474747"));
        mETextEnding.setHint(mDataParcel.getEndingTitle());
        mETextEnding.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setActivityStatus(IDLE);
                mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        ArrayList<String> titlelist = new ArrayList<>();
                        titlelist.add(mETextOpening.getHint().toString());
                        titlelist.add(mETextEnding.getHint().toString());
                        dataParcel parcel = dataParcel.getDataParcel(mDataParcel.getNameId(), mDataParcel.getPath(), titlelist);

                        Intent intent = new Intent(getBaseContext(), kmTitleEdit2Activity.class);
                        intent.putExtra("from_edit", 1);
                        intent.putExtra("parcelData", parcel);
                        startActivityForResult(intent, 100);
                    }
                });
            }
        });

        if(getActivityStatus() != ERROR) {
            mEngine.setProject(mProject);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(data != null) {
            String openingTitle = data.getStringExtra("openingTitle");
            String endingTitle = data.getStringExtra("endingTitle");

            mETextOpening.setHint(openingTitle);
            mETextEnding.setHint(endingTitle);

            mProject.setTitle(openingTitle.length() == 0 ? null : openingTitle,
                    endingTitle.length() == 0 ? null : endingTitle);

            setPlay();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                setActivityStatus(PLAYING);
            }
        });
    }

    @Override
    protected void onStop() {
        super.onStop();
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
                    if (mAlertDialog != null) {
                        mAlertDialog.dismiss();
                        mAlertDialog = null;
                    }
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
                mAlertDialog.dismiss();
                mAlertDialog = null;
                if(isFinish) {
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

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "KineMaster_" + export_time+".mp4");
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
    public void onSTEncodingFail(int result) {
        if (mFile != null) {
            mFile.delete();
            mFile = null;
        }
        setActivityStatus(IDLE);
        if(mAlertDialog!=null) mAlertDialog.dismiss();
        mAlertDialog = displayErrorDialog("Error<"+result+"> in encoding.");
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

        Intent intent = new Intent(getBaseContext(), kmThemeListActivity.class);
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
