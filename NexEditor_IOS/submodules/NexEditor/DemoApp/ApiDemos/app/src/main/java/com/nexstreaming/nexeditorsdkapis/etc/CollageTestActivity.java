package com.nexstreaming.nexeditorsdkapis.etc;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.app.common.util.RotationGestureDetector;
import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;
import com.nexstreaming.nexeditorsdk.module.nexExternalExportProvider;
import com.nexstreaming.nexeditorsdk.module.nexModuleProvider;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCollageInfo;
import com.nexstreaming.nexeditorsdk.nexCollageInfoDraw;
import com.nexstreaming.nexeditorsdk.nexCollageInfoTitle;
import com.nexstreaming.nexeditorsdk.nexCollageManager;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexExportFormat;
import com.nexstreaming.nexeditorsdk.nexExportFormatBuilder;
import com.nexstreaming.nexeditorsdk.nexExportListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.service.nexAssetService;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ActivitySave;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import static com.nexstreaming.nexeditorsdk.nexApplicationConfig.letterbox_effect_black;

//import android.support.v7.app.ActionBarActivity;

public class CollageTestActivity extends Activity {

    private static final String TAG = "CollageTestActivity";

    private int AssetStoreRequestCode = 0;

    private ListView mCollageList;
    private ListView mColorFilterList;
    private ArrayList<String> mListFilePath;

    private FrameLayout mHolderOverlay;
    private CollageOverlayView mOverlayView;

    private LinearLayout mLayoutDraw;
    private EditText mEditText;

    private Button btnChange;
    private Button btnMove;
    private Button btnScale;
    private Button btnRotate;
    private Button btnFilter;
    private Button btnFlip;
    private Button btnHide;
    private Button btnSwap;
    private nexEngineView mView;
    private int renderViewWidth = 720;
    private int renderViewHeight = 720;
    private nexEngine mEngine;
    private int mEngineState=nexEngine.nexPlayState.NONE.getValue();
    private nexProject mProject;

    private nexCollageManager mKmCollageManager;
    private List<nexCollageManager.Collage> mCollages = new ArrayList<>();
    private List<Bitmap> mCollageIcons = new ArrayList<>();
    private nexCollageManager.Collage mCurCollage = null;
    private nexCollageInfoDraw mCurCollageDrawInfo = null;
    private nexCollageInfoTitle mCurCollageTitleInfo = null;
    private AdaptorCollageListItem mCollageAdaptor;
    private AdaptorFilterListItem mFilterAdaptor;

    private Stopwatch watch = new Stopwatch();
    private Stopwatch export_watch = new Stopwatch();
    private Stopwatch asset_install_watch = new Stopwatch();

    private long startActivityTime = 0;
    private boolean mBackgroundMode = false;

    private RotationGestureDetector mRotateDetector;
    private ScaleGestureDetector mScaleDetector;

    boolean isRotating = false;
    boolean isScaling = false;
    boolean isMoving = false;
    boolean isSwapping = false;
    boolean existSaveData = false;

    private nexCollageInfoDraw selectedSwapSourceInfoDraw;

    nexCollageManager.CollageType collageShowType = nexCollageManager.CollageType.ALL;
    int collageStoreType = nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage;
    int collageStoreSourceCount = 1;
    int initUserSourceCount = 1;
    String storeVendor = "NexStreaming";

    int flip = 0;
    boolean needRefresh = false;
    boolean assetInstalled = false;
    int     lastUpdatedAssetIdx = -1;

    ProgressDialog exportDlg = null;
    boolean isAvailableEngineView = false;
    int     prevAspectMode=nexApplicationConfig.getAspectMode();

    public static final int ThumbnailToOriginal = 0;
    public static final int ThumbnailLoad = 1;
    public static final int OriginalLoad = 2;

    private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            float pre = detector.getPreviousSpan();
            float cur = detector.getCurrentSpan();

            float diff = cur - pre;

            if( Math.abs(diff) > 1)
                mCurCollageDrawInfo.setScale(diff);
            Log.d(TAG, String.format("onScale(%f %f %f)", pre, cur, diff));
            return true;
        }

        public boolean onScaleBegin(ScaleGestureDetector detector) {
            Log.d(TAG, String.format("onScaleBegin"));

            if( mCurCollageDrawInfo == null )
                return false;
            return true;
        }

        public void onScaleEnd(ScaleGestureDetector detector) {
            Log.d(TAG, String.format("onScaleEnd"));
        }

    }

    private class RotateListener implements RotationGestureDetector.OnRotationGestureListener {
        float angleStart = 0;
        @Override
        public boolean onBeginRotate(float focusX, float focusY) {
            Log.d(TAG, "onBeginRotate");
            if( mCurCollageDrawInfo == null )
                return false;

            mCurCollageDrawInfo.setRotate(nexCollageInfoDraw.kRotate_start, (int) mAngle);
            // isRotating = true;
            return true;
        }

        @Override
        public void onRotate(float focusX, float focusY, float angle) {

            Log.d(TAG, String.format("onRotate( %f %f %f %f)", focusX, focusX, angle, oldAngle));
            mAngle = oldAngle + angle;
            if(mAngle > 360){
                mAngle -= 360;
            }
            if(mAngle < -360){
                mAngle += 360;
            }
            if(mCurCollageDrawInfo != null) {
                mCurCollageDrawInfo.setRotate(nexCollageInfoDraw.kRotate_doing,(int) mAngle);
            }
        }

        @Override
        public void onEndRotate(boolean cancel) {
            oldAngle = mAngle;
            angleStart = 0;
            Log.d(TAG, "onEndRotate" + cancel);
            if(mCurCollageDrawInfo != null) {
                mCurCollageDrawInfo.setRotate(nexCollageInfoDraw.kRotate_end, (int) mAngle);
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(nexAssetService.ACTION_ASSET_INSTALL_COMPLETED);
        intentFilter.addAction(nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED);
        registerReceiver(broadcastReceiver, intentFilter);

        mScaleDetector = new ScaleGestureDetector(getApplicationContext(), new ScaleListener());
        mRotateDetector = new RotationGestureDetector(new RotateListener());

        nexApplicationConfig.setDefaultLetterboxEffect("transparent");
        watch.reset();
        watch.start();
        nexApplicationConfig.waitForLoading(this, new Runnable() {
            @Override
            public void run() {
            	ApiDemosConfig.getApplicationInstance().getEngine();

                setContentView(R.layout.activity_collage_test);

                // set screen timeout to never
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

                mKmCollageManager = nexCollageManager.getCollageManager(getApplicationContext(), getApplicationContext());

                mCollageList = (ListView) findViewById(R.id.listview_collage);
                mCollageAdaptor = new AdaptorCollageListItem();
                mCollageList.setAdapter(mCollageAdaptor);

                mColorFilterList = (ListView)findViewById(R.id.listview_filter);
                mFilterAdaptor = new AdaptorFilterListItem();
                mColorFilterList.setAdapter(mFilterAdaptor);
                mColorFilterList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        String colorId = (String)mFilterAdaptor.getItem(position);
                        if( mCurCollageDrawInfo != null ) {
                            mCurCollageDrawInfo.setLut(colorId);
                        }
                    }
                });

                Intent intent = getIntent();
                existSaveData = intent.getBooleanExtra("existsave",false);

                if( existSaveData){
                    mProject = nexProject.createFromSaveString(ActivitySave.getPram2());
                    startActivityTime = 0;
                }else {

                    mListFilePath = intent.getStringArrayListExtra("filelist");
                    mProject = new nexProject();

                    startActivityTime = intent.getLongExtra("startActivityTime", 0);
                    long elapsedTime = System.nanoTime() - startActivityTime;

                    Log.d(TAG, "CollageActivity onCreate elapsed = " + String.format("%1$,.3f", (double) elapsedTime / 1000000.0));

                    for (int i = 0; i < mListFilePath.size(); i++) {
                        Log.d(TAG, "Add content to project = " + mListFilePath.get(i));

                        nexClip clip = nexClip.getSupportedClip(mListFilePath.get(i));
                        if (clip != null) {
                            mProject.add(clip);

                            int rotate = clip.getRotateInMeta();

                            Log.i(TAG, "SDK ROTATION=" + rotate);

                            if (clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                                try {
                                    MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                                    retriever.setDataSource(mListFilePath.get(i));
                                    String rotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);

                                    if (rotation != null) {
                                        rotate = Integer.parseInt(rotation);
                                        Log.i(TAG, "MediaMetadataRetriever ROTATION=" + rotate);
                                    }

                                    Log.i(TAG, "rotate=" + rotate);
                                    clip.setRotateDegree(360 - rotate);

                                } catch (RuntimeException e) {
                                    Log.e(TAG, e.toString());
                                }
                            }
                        } else {
                            String skip = "Add content to project failed = " + mListFilePath.get(i);

                            Toast.makeText(getApplicationContext(), skip, Toast.LENGTH_LONG).show();
                            Log.d(TAG, "Add content to project failed = " + mListFilePath.get(i));
                        }
                    }
                }

                initUserSourceCount = collageStoreSourceCount = mProject.getTotalClipCount(true);

                refreshAssets();

                mLayoutDraw = (LinearLayout)findViewById(R.id.layout_draw);
                mEditText = (EditText)findViewById(R.id.title_edit);
                mEditText.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                        Log.d(TAG, "beforeTextChanged");

                    }

                    @Override
                    public void onTextChanged(CharSequence s, int start, int before, int count) {
                        Log.d(TAG, "onTextChanged");

                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        Log.d(TAG, "afterTextChanged: " + s.toString());
                        nexCollageInfoTitle titleInfo = (nexCollageInfoTitle)mEditText.getTag();
                        if( titleInfo != null ) {
                            if( s.length() <= 0 )
                                titleInfo.setTitle(null);
                            else
                                titleInfo.setTitle(s.toString());
                        }
                    }
                });
                // mOverlayLayout = (AbsoluteLayout)findViewById(R.id.alayout);

                btnChange = (Button)findViewById(R.id.btnDrawChange);
                btnChange.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Intent intent = makeGalleryIntent();
                        startActivityForResult(intent, 100);
                    }
                });

                btnMove = (Button)findViewById(R.id.btnDrawMove);
                btnMove.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( isMoving ) {
                            btnChange.setEnabled(true);
                            btnFilter.setEnabled(true);
                            btnMove.setEnabled(true);
                            btnScale.setEnabled(true);
                            btnRotate.setEnabled(true);
                            btnFlip.setEnabled(true);
                            btnHide.setEnabled(true);
                            btnSwap.setEnabled(true);
                            isMoving = false;
                        }
                        else {
                            btnChange.setEnabled(false);
                            btnFilter.setEnabled(false);
                            btnMove.setEnabled(true);
                            btnScale.setEnabled(false);
                            btnRotate.setEnabled(false);
                            btnFlip.setEnabled(false);
                            btnHide.setEnabled(false);
                            btnSwap.setEnabled(false);
                            isMoving = true;
                        }
                    }
                });

                btnScale = (Button)findViewById(R.id.btnDrawScale);
                btnScale.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( isScaling ) {
                            btnChange.setEnabled(true);
                            btnFilter.setEnabled(true);
                            btnMove.setEnabled(true);
                            btnScale.setEnabled(true);
                            btnRotate.setEnabled(true);
                            btnFlip.setEnabled(true);
                            btnHide.setEnabled(true);
                            btnSwap.setEnabled(true);
                            isScaling = false;
                        }
                        else {
                            btnChange.setEnabled(false);
                            btnFilter.setEnabled(false);
                            btnMove.setEnabled(false);
                            btnScale.setEnabled(true);
                            btnRotate.setEnabled(false);
                            btnFlip.setEnabled(false);
                            btnHide.setEnabled(false);
                            btnSwap.setEnabled(false);
                            isScaling = true;
                        }
                    }
                });

                btnRotate = (Button)findViewById(R.id.btnDrawRotate);
                btnRotate.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( isRotating ) {
                            btnChange.setEnabled(true);
                            btnFilter.setEnabled(true);
                            btnMove.setEnabled(true);
                            btnScale.setEnabled(true);
                            btnRotate.setEnabled(true);
                            btnFlip.setEnabled(true);
                            btnHide.setEnabled(true);
                            btnSwap.setEnabled(true);
                            isRotating = false;
                        }
                        else {
                            btnChange.setEnabled(false);
                            btnFilter.setEnabled(false);
                            btnMove.setEnabled(false);
                            btnScale.setEnabled(false);
                            btnRotate.setEnabled(true);
                            btnFlip.setEnabled(false);
                            btnHide.setEnabled(false);
                            btnSwap.setEnabled(false);
                            isRotating = true;
                            oldAngle = mCurCollageDrawInfo.getRotate();
                            mAngle = 0;
                        }
                    }
                });

                btnFilter = (Button)findViewById(R.id.btnDrawFilter);
                btnFilter.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( mColorFilterList != null )
                        {
                            if( mColorFilterList.getVisibility() == View.INVISIBLE ) {
                                mColorFilterList.setVisibility(View.VISIBLE);
                                mCollageList.setVisibility(View.INVISIBLE);
                                mFilterAdaptor.notifyDataSetChanged();
                            }
                            else {
                                mColorFilterList.setVisibility(View.INVISIBLE);
                                mCollageList.setVisibility(View.VISIBLE);
                            }
                        }
                    }
                });

                btnFilter.setLongClickable(true);
                btnFilter.setOnLongClickListener(new View.OnLongClickListener() {
                    @Override
                    public boolean onLongClick(View v) {
                        if( mCurCollageDrawInfo != null ) {
                            mCurCollageDrawInfo.setLut(null);
                        }
                        return false;
                    }
                });

                btnFlip = (Button)findViewById(R.id.btnDrawFlip);
                btnFlip.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        nexCollageInfoDraw drawInfo = (nexCollageInfoDraw)btnFlip.getTag();
                        if( drawInfo != null ) {
                            flip = (flip + 1) >= 3 ? 0 : flip + 1;
                            drawInfo.setFlip(flip);
                        }
                    }
                });

                btnHide = (Button)findViewById(R.id.btnDrawHide);
                btnHide.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        nexCollageInfoDraw drawInfo = (nexCollageInfoDraw)btnHide.getTag();
                        if( drawInfo != null ) {
                            drawInfo.setVisible(!drawInfo.getVisible());
                        }
                    }
                });

                btnSwap = (Button)findViewById(R.id.btnDrawSwap);
                btnSwap.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if( mCurCollage == null ) return;
                        if( !isSwapping ) {
                            isSwapping = true;
                            btnMove.setEnabled(false);
                            btnScale.setEnabled(false);
                            btnRotate.setEnabled(false);
                            btnChange.setEnabled(false);
                        }else{
                            isSwapping = false;
                            btnMove.setEnabled(true);
                            btnScale.setEnabled(true);
                            btnRotate.setEnabled(true);
                            btnChange.setEnabled(true);
                        }
                        selectedSwapSourceInfoDraw = (nexCollageInfoDraw)btnSwap.getTag();
                    }
                });

                Button btnJpeg = (Button)findViewById(R.id.btnExportJpeg);
                btnJpeg.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        if( isExportCollage ) {
                            Log.d(TAG, "Collage already is exporting now");
                            return;
                        }

                        isExportCollage = true;

                        if( mCollages.size() <= 0 ) {
                            Toast.makeText(CollageTestActivity.this, "No available collage", Toast.LENGTH_SHORT).show();
                            isExportCollage = false;
                            return;
                        }

                        if( mCurCollage == null ) {
                            Toast.makeText(CollageTestActivity.this, "Current collage is null", Toast.LENGTH_SHORT).show();
                            isExportCollage = false;
                            return;
                        }

                        synchronized (collageApplyLock) {

                            Float ratio = mCurCollage.getRatio();

                            if( mCurCollage.getType() == nexCollageManager.CollageType.DynamicCollage || isImageOnlyProject(mProject) == false ) {

                                List<nexModuleProvider> modules = nexApplicationConfig.getExternalModuleManager().getModules(nexExternalExportProvider.class);
                                if (modules.size() > 0) {

                                    if( exportDlg != null && exportDlg.isShowing() ) {
                                        Log.d(TAG, "Collage already is exporting now");
                                        isExportCollage = false;
                                        return;
                                    }

                                    String uuid = modules.get(0).uuid();
                                    String type = "external-" + modules.get(0).name();
                                    String ext = modules.get(0).format();
                                    if (uuid == null) {
                                        Log.d(TAG, "get External module failed");
                                        isExportCollage = false;
                                        return;
                                    }

                                    int width = 480;
                                    int height = 480;
                                    if (ratio >= 1)
                                        height = (int) (width / ratio);
                                    else
                                        width = (int) (height * ratio);

                                    final File f = getExportFile(width, height, ext);

                                    final nexExportFormat format = nexExportFormatBuilder.Builder()
                                            .setType(type)
                                            .setWidth(width)
                                            .setHeight(height)
                                            .setPath(f.getAbsolutePath())
                                            .setIntervalTime(200)
                                            .setStartTime(0)
                                            .setEndTime(8000)
                                            .setUUID(uuid)
                                            .setQuality(aniGifQuality)
                                            .build();

                                    exportDlg = new ProgressDialog(CollageTestActivity.this);

                                    exportDlg.setTitle("Export " + ext);
                                    exportDlg.setMessage("Prepare to export");
                                    exportDlg.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
                                    exportDlg.setProgress(0);
                                    exportDlg.setMax(100);
                                    exportDlg.setCancelable(false);
                                    exportDlg.setOnCancelListener(new DialogInterface.OnCancelListener() {
                                        @Override
                                        public void onCancel(DialogInterface dialog) {
                                            mEngine.stop();
                                            if( exportDlg != null ) {
                                                exportDlg.dismiss();
                                                exportDlg = null;
                                            }
                                            isExportCollage = false;
                                        }
                                    });
                                    exportDlg.show();

                                    Log.d(TAG, "Export GIF Dialog show");

                                    mEngine.stop();

                                    Log.d(TAG, "Export GIF Start");
                                    export_watch.reset();
                                    export_watch.start();
                                    nexEngine.nexErrorCode err = mEngine.export(format, new nexExportListener() {
                                        @Override
                                        public void onExportFail(nexEngine.nexErrorCode err) {
                                            Log.d(TAG, "onExportFail: GIF" + err.toString());
                                            if( exportDlg != null ) {
                                                exportDlg.dismiss();
                                                exportDlg = null;
                                            }
                                            isExportCollage = false;
                                            mEngine.seek(mCurCollage.getEditTime());
                                        }

                                        @Override
                                        public void onExportProgress(int persent) {
                                            if( exportDlg != null ) {
                                                exportDlg.setProgress(persent);
                                            }
                                        }

                                        @Override
                                        public void onExportDone(Bitmap bitmap) {
                                            Log.d(TAG, "onExportDone: GIF, elapsed="+export_watch.toString());
                                            if ( export_watch.isRunning() ) export_watch.stop();
                                            if( exportDlg != null ) {
                                                exportDlg.dismiss();
                                                exportDlg = null;
                                            }
                                            isExportCollage = false;
                                            mEngine.seek(mCurCollage.getEditTime());
                                            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{f.getAbsolutePath()}, null, null);
                                        }
                                    });

                                    if( err != nexEngine.nexErrorCode.NONE ) {
                                        if( exportDlg != null ) {
                                            exportDlg.dismiss();
                                            exportDlg = null;
                                        }

                                        mEngine.seek(mCurCollage.getEditTime());

                                        Log.d(TAG, "mEngine.export error : " + err.toString());
                                        if ( export_watch.isRunning() ) export_watch.stop();
                                    }

                                    return;
                                }
                            }

                            int width = 3840;
                            int height = 3840;
                            if (ratio >= 1)
                                height = (int) (width / ratio);
                            else
                                width = (int) (height * ratio);

                            final File f = getExportFile(width, height, "jpeg");

                            final nexExportFormat format = nexExportFormatBuilder.Builder()
                                    .setType("jpeg")
                                    .setWidth(width)
                                    .setHeight(height)
                                    .setPath(f.getAbsolutePath())
                                    .setQuality(90)
                                    .build();

                            mEngine.stop();
                            Log.d(TAG, "Export JPEG Start");
                            export_watch.reset();
                            export_watch.start();
                            mEngine.export(format, new nexExportListener() {
                                @Override
                                public void onExportFail(nexEngine.nexErrorCode err) {
                                    Log.d(TAG, "onExportFail: JPEG" + err.toString());
                                    if ( export_watch.isRunning() ) export_watch.stop();
                                    isExportCollage = false;
                                    mEngine.seek(mCurCollage.getEditTime());
                                }

                                @Override
                                public void onExportProgress(int persent) {
                                }

                                @Override
                                public void onExportDone(Bitmap bitmap) {
                                    Log.d(TAG, "onExportDone: JPEG, elapsed="+export_watch.toString());
                                    if ( export_watch.isRunning() ) export_watch.stop();
                                    isExportCollage = false;
                                    mEngine.seek(mCurCollage.getEditTime());
                                    MediaScannerConnection.scanFile(getApplicationContext(), new String[]{f.getAbsolutePath()}, null, null);
                                }
                            });
                        }
                    }
                });

                Button btnMp4 = (Button)findViewById(R.id.btnExportMp4);
                btnMp4.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        if( isExportCollage ) {
                            Log.d(TAG, "Collage already is exporting now");
                            return;
                        }

                        isExportCollage = true;

                        if( mCollages.size() <= 0 ) {
                            Toast.makeText(CollageTestActivity.this, "No available collage", Toast.LENGTH_SHORT).show();
                            isExportCollage = false;
                            return;
                        }

                        if( mCurCollage == null ) {
                            Toast.makeText(CollageTestActivity.this, "Current collage is null", Toast.LENGTH_SHORT).show();
                            isExportCollage = false;
                            return;
                        }

                        if( mCurCollage.getType() != nexCollageManager.CollageType.DynamicCollage ) {
                            Toast.makeText(CollageTestActivity.this, "Non dynamic collage did not support export mp4", Toast.LENGTH_SHORT).show();
                            isExportCollage = false;
                            return;
                        }

                        synchronized (collageApplyLock) {
                            Float ratio = mCurCollage.getRatio();

                            int width = 1280;
                            int height = 1280;
                            if (ratio >= 1)
                                height = (int) (width / ratio);
                            else
                                width = (int) (height * ratio);

                            final File f = getExportFile(width, height, "mp4");

                            final nexExportFormat format = nexExportFormatBuilder.Builder()
                                    .setType("mp4")
                                    .setVideoCodec(nexEngine.ExportCodec_AVC)
                                    .setVideoBitrate(12 * 1024 * 1024)
                                    .setVideoProfile(nexEngine.ExportProfile_AVCBaseline)
                                    .setVideoLevel(nexEngine.ExportAVCLevel31)
                                    .setVideoRotate(0)
                                    .setVideoFPS(3000) // 30*100 if 30 fps, 2997 if 29.97 fps.
                                    .setWidth(width)
                                    .setHeight(height)
                                    .setAudioSampleRate(44100)
                                    .setMaxFileSize(Long.MAX_VALUE)
                                    .setPath(f.getAbsolutePath())
                                    .build();

                            if( exportDlg != null && exportDlg.isShowing() ) {
                                Log.d(TAG, "Collage already is exporting now");
                                isExportCollage = false;
                                return;
                            }

                            exportDlg = new ProgressDialog(CollageTestActivity.this);

                            exportDlg.setTitle("Export Mp4...");
                            exportDlg.setMessage("Prepare to export");
                            exportDlg.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
                            exportDlg.setProgress(0);
                            exportDlg.setMax(100);
                            exportDlg.setCancelable(false);
                            exportDlg.setOnCancelListener(new DialogInterface.OnCancelListener() {
                                @Override
                                public void onCancel(DialogInterface dialog) {
                                    mEngine.stop();
                                    if( exportDlg != null ) {
                                        exportDlg.dismiss();
                                        exportDlg = null;
                                    }
                                    isExportCollage = false;
                                }
                            });
                            exportDlg.show();


                            mEngine.stop(new nexEngine.OnCompletionListener() {
                                @Override
                                public void onComplete(int resultCode) {
                                    Log.d(TAG, "Export Mp4 Start");
                                    export_watch.reset();
                                    export_watch.start();
                                    mEngine.export(format, new nexExportListener() {

                                        @Override
                                        public void onExportFail(nexEngine.nexErrorCode err) {
                                            Log.d(TAG, "onExportFail: MP4" + err.toString());
                                            if ( export_watch.isRunning() ) export_watch.stop();
                                            if( exportDlg != null ) {
                                                exportDlg.dismiss();
                                                exportDlg = null;
                                            }
                                            isExportCollage = false;
                                            mEngine.seek(mCurCollage.getEditTime());
                                        }

                                        @Override
                                        public void onExportProgress(int percent) {
                                            Log.d(TAG, "onExportProgress: " + percent);
                                            if( exportDlg != null )
                                                exportDlg.setProgress(percent);

                                        }

                                        @Override
                                        public void onExportDone(Bitmap bitmap) {
                                            Log.d(TAG, "onExportDone: MP4, elapsed="+export_watch.toString());
                                            if ( export_watch.isRunning() ) export_watch.stop();
                                            if( exportDlg != null ) {
                                                exportDlg.dismiss();
                                                exportDlg = null;
                                            }
                                            isExportCollage = false;
                                            mEngine.seek(mCurCollage.getEditTime());
                                            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{f.getAbsolutePath()}, null, null);
                                        }
                                    });
                                }
                            });
                        }
                    }
                });

                Button btnStore = (Button)findViewById(R.id.btnStore);
                btnStore.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        nexAssetStoreAppUtils.setVendor(storeVendor);
                        // nexAssetStoreAppUtils.setServer(nexAssetStoreAppUtils.AssetStoreServerDRAFT);
                        if ( nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext()) ) {
                            String type = "DynamicCollage";
                            if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage ) {
                                type = "IntegratedCollage";
                            } else if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage ) {
                                type = "StaticCollage";
                            } else if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_DynamicCollage ) {
                                type = "DynamicCollage";
                            }

                            Log.d(TAG, "Asset store installed: " + storeVendor + " Type: " + type+" , sourceCount:"+collageStoreSourceCount);
                            nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
                            nexAssetStoreAppUtils.setMimeType(collageStoreType);
                            nexAssetStoreAppUtils.setAvailableCategorys(nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage|nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage|nexAssetStoreAppUtils.AssetStoreMimeType_DynamicCollage );
                            if( collageStoreSourceCount > 0 ) {

                                String extra = "{\"UserSources\": "+collageStoreSourceCount+" }";

                                nexAssetStoreAppUtils.setMimeTypeExtra(extra);
                            }
                            AssetStoreRequestCode = nexAssetStoreAppUtils.runAssetStoreApp(CollageTestActivity.this, null);//Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK, Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP
                        } else {
                            Log.d(TAG, "Asset store isn't installed: " + storeVendor);
                            nexAssetStoreAppUtils.moveGooglePlayAssetStoreLink(getApplicationContext());
                        }
                    }
                });

                Button btnSet = (Button)findViewById(R.id.btnSet);
                btnSet.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        setOption();
                        updateControlUI(null);

                        mOverlayView.setCollageInfo(null);
                        mOverlayView.invalidate();

                        if( mColorFilterList.getVisibility() != View.INVISIBLE ) {
                            mColorFilterList.setVisibility(View.INVISIBLE);
                            mCollageList.setVisibility(View.VISIBLE);
                        }
                    }
                });

                Button btSave = (Button)findViewById(R.id.btnSave);
                btSave.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {

                        if( mCurCollage != null ) {
                            ActivitySave.setSaveData(mCurCollage.id(), mCurCollage.saveToString());
                        }
                        else {
                            Toast.makeText(CollageTestActivity.this, "Current collage was cleared", Toast.LENGTH_SHORT).show();
                        }
                    }
                });

                mOverlayView = new CollageOverlayView(getApplicationContext());
                mHolderOverlay = (FrameLayout) findViewById(R.id.holder_overlay);
                mHolderOverlay.addView(mOverlayView, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

                mOverlayView.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {

                        View view = getCurrentFocus();
                        if( view != null ) {
                            InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                            imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
                        }

                        if( mCurCollageDrawInfo != null ) {
                            if( isRotating ) {
                                mRotateDetector.onTouchEvent(event);
                                return true;
                            }

                            if( isScaling ) {
                                mScaleDetector.onTouchEvent(event);
                                return true;
                            }

                            if( isMoving ) {
                                move(event);
                                return true;
                            }
                        }

                        float width = mOverlayView.getMeasuredWidth();
                        float height = mOverlayView.getMeasuredHeight();
                        switch (event.getAction()) {
                            case MotionEvent.ACTION_DOWN:
                                nexCollageInfo info = null;
                                if( mCurCollage != null ) {
                                    info = mCurCollage.getCollageInfos(event.getX() / width, event.getY() / height);

                                    if( info instanceof nexCollageInfoTitle ) {
                                        if( mCurCollageDrawInfo != null )
                                            info = null;

                                    }
                                }

                                if( isSwapping ){
                                    if( mCurCollage != null ) {
                                        if( info != null) {

                                            if(selectedSwapSourceInfoDraw != null) {
                                                if (((nexCollageInfoDraw) info).getTagID() != selectedSwapSourceInfoDraw.getTagID()) {
                                                    mCurCollage.swapDrawInfoClip(selectedSwapSourceInfoDraw, (nexCollageInfoDraw) info);
                                                }
                                            }
                                            isSwapping = false;
                                            updateControlUI((nexCollageInfo) selectedSwapSourceInfoDraw);
                                        }
                                    }
                                }else {
                                    updateControlUI(info);
                                }
                                break;
                        }
                        return false;
                    }
                });

                // mOverlayView.setBackgroundColor(Color.argb(40, 255, 0, 0));


                mView = (nexEngineView) findViewById(R.id.engineview);
                mView.setBlackOut(true);

                mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
                mEngine.setView(mView);
                mEngine.set360VideoForceNormalView();
                mEngine.setThumbnailRoutine(ThumbnailToOriginal);
                mView.setListener(new nexEngineView.NexViewListener() {
                    @Override
                    public void onEngineViewAvailable(int width, int height) {
                        renderViewWidth = width;
                        renderViewHeight = height;

                        Log.d(TAG, String.format("onEngineViewAvailable(%d %d)", width, height));

                        if( mIsActivityResult ) {
                            mIsActivityResult = false;
                            isAvailableEngineView = true;
                            Log.d(TAG, String.format("onEngineViewAvailable after activityResult(%d %d)", width, height));

                            if( mCurCollage != null ) {
                                setNexFaceModule();
                                mEngine.seek(mCurCollage.getEditTime());
                            }

                            return;
                        }
                        if( !mBackgroundMode ) {
                            playCollage();
                        }
                        isAvailableEngineView = true;
                    }

                    @Override
                    public void onEngineViewSizeChanged(int width, int height) {
                        Log.d(TAG, String.format("onEngineViewSizeChanged(%d %d)", width, height));
                        renderViewWidth = width;
                        renderViewHeight = height;

                    }

                    @Override
                    public void onEngineViewDestroyed() {
                        isAvailableEngineView = false;

                    }
                });

                // Add this code for update preview surface after activity resume.
//                mEngine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
//                    @Override
//                    public void onSurfaceChanged() {
//                        Log.d(TAG, String.format("setOnSurfaceChangeListener"));
//                    }
//                });

                if( mCollages.size() > 0 )
                {
                    mCurCollage = mCollages.get(0);


                    if( existSaveData ){
                        for( nexCollageManager.Collage collage : mCollages ){
                            if( collage.id().compareTo(ActivitySave.getPram1()) == 0 ){
                                mCurCollage = collage;
                                break;
                            }
                        }
                        nexApplicationConfig.setAspectMode(mCurCollage.getRatioMode());
                        mCurCollage.loadFromSaveData(getApplicationContext(),mEngine,mProject,ActivitySave.getPram2());
                        ActivitySave.setSaveData(null,null);
                        Log.d(TAG, String.format("Make project with save data"));
                    }else {

                        Log.d(TAG, String.format("Make project with default collage"));
                        String errorMsg = setEffects2Project();
                        if (errorMsg != null) {
                            Log.d(TAG, errorMsg);

                            Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();

                            // finish();
                            return;
                        }
                    }
//                    setNexFaceModule();
//                    mEngine.faceDetect(true, mProject.getTotalClipCount(true), nexEngine.nexUndetectedFaceCrop.ZOOM);
//                    mEngine.seek(mCurCollage.getEditTime());
                    showOverlayView(false);
                }
                else {
                    showEmptyCollageDialog();
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        mEngine.stop();

        unregisterReceiver(broadcastReceiver);

        nexApplicationConfig.setDefaultLetterboxEffect(letterbox_effect_black);// Set default value.
        nexApplicationConfig.setAspectMode(prevAspectMode);

        ApiDemosConfig.getApplicationInstance().releaseEngine();
        // nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
    }

    @Override
    protected void onPause() {
        super.onPause();

        mBackgroundMode = true;

        mEngine.stop(new nexEngine.OnCompletionListener() {
            @Override
            public void onComplete(int resultCode) {
                if( exportDlg != null ) {
                    exportDlg.dismiss();
                    exportDlg = null;
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();

        Log.d(TAG, "onResume");

        mBackgroundMode = false;
        if( needRefresh ) {

            refreshAssets();
            needRefresh = false;

            if( mCollages != null && lastUpdatedAssetIdx != -1 ) {
                if( assetInstalled ) {
                    for (nexCollageManager.Collage c : mCollages) {
                        if (c.packageInfo().assetIdx() == lastUpdatedAssetIdx) {

                            mCurCollage = c;

                            watch.reset();
                            watch.start();

                            updateControlUI(null);

                            Log.d(TAG, "Collage with: " + mCurCollage.id());

                            String errorMsg = setEffects2Project();
                            if (errorMsg != null) {
                                Log.d(TAG, errorMsg);

                                Log.d(TAG, "Collage did not apply : " + mCurCollage.id());
                                Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();
                                break;
                            }

                            mCollageAdaptor.notifyDataSetChanged();

                            if( isAvailableEngineView ) {
                                if (mCurCollage.getType() == nexCollageManager.CollageType.StaticCollage) {
                                    mEngine.seek(mCurCollage.getEditTime());
                                } else {
                                    mOverlayView.setCollageInfo(null);
                                    mOverlayView.invalidate();
                                    setNexFaceModule();
                                    mEngine.play();
                                }
                            }
                            break;
                        }
                    }
                }
                else {
                    mCurCollage = mCollages.get(0);

                    watch.reset();
                    watch.start();

                    updateControlUI(null);

                    String errorMsg = setEffects2Project();
                    if (errorMsg != null) {
                        Log.d(TAG, errorMsg);

                        Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();
                    }
                    else {

                        mCollageAdaptor.notifyDataSetChanged();
                        if( isAvailableEngineView ) {

                            if (mCurCollage.getType() == nexCollageManager.CollageType.StaticCollage) {
                                mEngine.seek(mCurCollage.getEditTime());
                            } else {
                                mOverlayView.setCollageInfo(null);
                                mOverlayView.invalidate();
                                mEngine.play();
                            }
                        }
                    }
                }
            }
            lastUpdatedAssetIdx = -1;
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");

        // mEngine.stop();
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        mEngine.setEventHandler(sEngineListener);
    }

    boolean mIsActivityResult = false;

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        mIsActivityResult = true;
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            ClipData clipData = data.getClipData();
            String filepath = null;

            if( mListFilePath != null) {
                mListFilePath.clear();
            }

            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
            } else {
                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                }
            }
            if( filepath != null ) {
                nexClip clip = nexClip.getSupportedClip(filepath);
                nexCollageInfoDraw drawInfo = (nexCollageInfoDraw)btnChange.getTag();
                if( clip != null && drawInfo != null && filepath.equals(drawInfo.getBindSource().getPath())==false ) {
                    drawInfo.changeSource(clip);
                }
            }
            if( mEngine != null && mCurCollage != null && isAvailableEngineView ) {
                setNexFaceModule();
                mEngine.seek(mCurCollage.getEditTime());
            }

        }else if(requestCode == AssetStoreRequestCode && resultCode == Activity.RESULT_OK) {
            Log.d(TAG,"onActivityResult from Asset Store");
            int count = mKmCollageManager.findNewPackages();
            if(  count > 0 ){
                asset_install_watch.reset();
                asset_install_watch.start();
                showInstallDialog();
            }
        }
    }

    //1. create BroadcastReceiver
    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                if (nexAssetService.ACTION_ASSET_INSTALL_COMPLETED.equals(action)) {
                    //onUpdateAssetList();
                    lastUpdatedAssetIdx = intent.getIntExtra("index",-1);
                    String categoryName = intent.getStringExtra("category.alias");
                    Log.d(TAG,"installed Asset, categoryName="+categoryName+", index="+lastUpdatedAssetIdx);
                    //TODO : update UI
                    needRefresh = true;
                    assetInstalled = true;
                    // refreshAssets();
                }else if( nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED.equals(action) ){
                    lastUpdatedAssetIdx = intent.getIntExtra("index",-1);
                    Log.d(TAG,"uninstalled Asset, index="+lastUpdatedAssetIdx);
                    needRefresh = true;
                    assetInstalled = false;
                    // refreshAssets();
                }
            }
        }
    };

    public void playCollage() {
        if( mCurCollage != null ) {
            setNexFaceModule();
            mEngine.setThumbnailRoutine(ThumbnailToOriginal);
            if( mCurCollage.getType() == nexCollageManager.CollageType.StaticCollage && isImageOnlyProject(mProject) ) {
                mEngine.seek(mCurCollage.getEditTime());
            } else {
                mOverlayView.setCollageInfo(null);
                mOverlayView.invalidate();
                mEngine.play();
            }
        }
    }

    public boolean isImageOnlyProject(nexProject project) {
        if( project == null ) return false;
        for(int i = 0; i < project.getTotalClipCount(true); i++ ) {
            if (project.getClip(i, true).getClipType() != nexClip.kCLIP_TYPE_IMAGE)
                return false;
        }
        return true;
    }

    public void showEmptyCollageDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(CollageTestActivity.this);
        String message = "Not available collage with current source!!";
        builder.setCancelable(false)
                .setMessage(message)
                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                });
        builder.create().show();
    }

    public Bitmap getBitmapOfDrawInfo(nexCollageInfoDraw info, nexEngineView view) {
        if( info == null || view == null ) return null;

        Bitmap b = view.getBitmap();
        if( b == null ) return null;

        nexCollageInfo collageInfo = (nexCollageInfo)info;

        int viewWidth = view.getMeasuredWidth();
        int viewHeight = view.getMeasuredHeight();

        RectF rect = collageInfo.getRectangle();
        int x = (int)(rect.left*viewWidth);
        int y = (int)(rect.top*viewHeight);
        int width = (int)(rect.width()*viewWidth);
        int height = (int)(rect.height()*viewHeight);

        Bitmap resultBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Bitmap maskBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);

        Canvas canvasResult = new Canvas(resultBitmap);
        Canvas canvasMask = new Canvas(maskBitmap);
        canvasMask.drawARGB(0, 0, 0, 0);

        Path path=new Path();
        Iterator<PointF> positions = collageInfo.getPositions().iterator();
        if( positions.hasNext() ) {
            PointF pos = positions.next();
            path.moveTo((pos.x*viewWidth)-x, (pos.y*viewHeight)-y);
            while(positions.hasNext()) {
                pos = positions.next();
                path.lineTo((pos.x * viewWidth) - x, (pos.y * viewHeight) - y);
            }
            path.close();
        }

        Paint maskPaint = new Paint();
        maskPaint.setColor(Color.argb(255, 0, 255, 0));
        maskPaint.setStyle(Paint.Style.FILL);
        canvasMask.drawPath(path, maskPaint);

        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_IN));
        canvasResult.drawBitmap(b, new Rect(x, y, width, height), new Rect(0, 0, width, height), null);
        canvasResult.drawBitmap(maskBitmap, 0, 0, p);
        p.setXfermode(null);

        return resultBitmap;
    }

    ProgressDialog barProgressDialog;
    Handler updateBarHandler;

    public void showInstallDialog(){
        barProgressDialog = new ProgressDialog(CollageTestActivity.this);
        barProgressDialog.setTitle("Installing Asset Package ...");
        barProgressDialog.setMessage("ready to install");
        barProgressDialog.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
        barProgressDialog.setProgress(0);
        barProgressDialog.setMax(100);
        barProgressDialog.show();

        mKmCollageManager.installPackagesAsync(new nexAssetPackageManager.OnInstallPackageListener() {
            private int progress;
            private int count = 0;
            private int max = 0;
            private boolean updateMassage = false;
            @Override
            public void onProgress(int countPackage, int totalPackages, int progressInstalling) {
                Log.d(TAG, "countPackage=" + countPackage + ", totalPackages=" + totalPackages + ", progressInstalling=" + progressInstalling);

                if( count != countPackage ){
                    updateMassage = true;
                }
                count= countPackage;
                max = totalPackages;
                progress = progressInstalling;

                updateBarHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if( updateMassage ){
                            barProgressDialog.setMessage("install("+count+"/"+max+") in progress...");
                        }
                        barProgressDialog.setProgress(progress);
                    }
                });
            }

            @Override
            public void onCompleted(int event, int idx) {

                refreshAssets();

                barProgressDialog.dismiss();

                Toast.makeText(CollageTestActivity.this, "Collage install elapsed =" + asset_install_watch.toString(), Toast.LENGTH_LONG).show();
                Log.d(TAG, "Collage install elapsed = " + asset_install_watch.toString());
            }
        });
    }

    private Object collageApplyLock = new Object();
    String setEffects2Project() {
        synchronized (collageApplyLock) {
            if (mProject.getTotalTime() <= 0) {
                return "Project is empty";
            }

            if (mCollages == null || mCurCollage == null) {
                return "Collage did not selected";
            }

            if (mCurCollage.getRatioMode() == 0) {
                return "Collage with unkonwn screen ratio";
            }

            showOverlayView(false);
            mEngine.clearScreen();
            // mEngine.clearProject();
            setNexFaceModule();
            try {
                if (mCurCollage.applyCollage2Project(mProject, mEngine, mCurCollage.getDuration(), getApplicationContext()) == false) {
                    return "Fail to apply Collage on project";
                }
            } catch (ExpiredTimeException e) {
                //e.printStackTrace();
                nexAssetStoreAppUtils.runAssetStoreApp(CollageTestActivity.this,""+mCurCollage.packageInfo().assetIdx());
                return "Collage has expired";
            }

//            mView.setVisibility(View.GONE);
//            nexApplicationConfig.setAspectMode(mCurCollage.getRatioMode());
            nexApplicationConfig.setAspectMode(mCurCollage.getRatioMode());
            mView.requestLayout();
        }
        return null;
    }

    private void showOverlayView(boolean showLayout) {
        if( showLayout ) {
            if( mCurCollage != null ) {
                mOverlayView.setCollageInfo(mCurCollage.getCollageInfos());
            }
            else {
                mOverlayView.setCollageInfo(null);
            }
        }
        else {
            mOverlayView.setCollageInfo(null);
        }

        mOverlayView.invalidate();
    }

    public static InputFilter getEditTextFilterEmoji()
    {
        return new InputFilter()
        {
            @Override
            public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend)
            {

                for (int i = start; i < end; i++) {
                    int type = Character.getType(source.charAt(i));
                    if (type == Character.SURROGATE || type == Character.OTHER_SYMBOL) {
                        return source.subSequence(start, i);
                    }
                }
                return null;
            }
        };
    }


    private boolean updateControlUI(nexCollageInfo collageInfo) {
        if( mOverlayView.hasCollageInfo() == false ) return false;

        if( collageInfo == null ) {
            mLayoutDraw.setVisibility(View.GONE);
            mEditText.setVisibility(View.GONE);

            btnChange.setTag(null);
            btnFilter.setTag(null);
            btnMove.setTag(null);
            btnScale.setTag(null);
            btnRotate.setTag(null);
            btnFlip.setTag(null);
            btnHide.setTag(null);
            btnSwap.setTag(null);

            btnChange.setEnabled(true);
            btnFilter.setEnabled(true);
            btnMove.setEnabled(true);
            btnScale.setEnabled(true);
            btnRotate.setEnabled(true);
            btnFlip.setEnabled(true);
            btnHide.setEnabled(true);
            btnSwap.setEnabled(true);

            mCurCollageDrawInfo = null;
            selectedSwapSourceInfoDraw = null;
            mCurCollageTitleInfo = null;

            isRotating = false;
            isScaling = false;
            isMoving = false;
            isSwapping = false;
        }

        if( collageInfo instanceof nexCollageInfoDraw ) {
            if( mCurCollageDrawInfo != null && mCurCollageDrawInfo.equals(collageInfo) ) {
                mLayoutDraw.setVisibility(View.GONE);
                mEditText.setVisibility(View.GONE);

                btnChange.setTag(null);
                btnMove.setTag(null);
                btnScale.setTag(null);
                btnRotate.setTag(null);
                btnFlip.setTag(null);
                btnHide.setTag(null);
                btnSwap.setTag(null);

                btnChange.setEnabled(true);
                btnMove.setEnabled(true);
                btnScale.setEnabled(true);
                btnRotate.setEnabled(true);
                btnFlip.setEnabled(true);
                btnHide.setEnabled(true);
                btnSwap.setEnabled(true);

                mCurCollageDrawInfo = null;
                mCurCollageTitleInfo = null;
            }
            else {
                mLayoutDraw.setVisibility(View.VISIBLE);
                mEditText.setVisibility(View.GONE);

                btnChange.setTag(collageInfo);
                btnMove.setTag(collageInfo);
                btnScale.setTag(collageInfo);
                btnRotate.setTag(collageInfo);
                btnFlip.setTag(collageInfo);
                btnHide.setTag(collageInfo);
                btnSwap.setTag(collageInfo);

                btnChange.setEnabled(true);
                btnMove.setEnabled(true);
                btnScale.setEnabled(true);
                btnRotate.setEnabled(true);
                btnFlip.setEnabled(true);
                btnHide.setEnabled(true);
                btnSwap.setEnabled(true);

                mCurCollageDrawInfo = (nexCollageInfoDraw)collageInfo;
                mCurCollageTitleInfo = null;
            }
        }
        else if( collageInfo instanceof nexCollageInfoTitle ) {
            if( mCurCollageTitleInfo != null && mCurCollageTitleInfo.equals(collageInfo) ) {

                InputMethodManager imm = (InputMethodManager) getSystemService(Activity.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);

                mEditText.setVisibility(View.GONE);
                mEditText.setTag(null);

                mLayoutDraw.setVisibility(View.GONE);
                btnChange.setTag(null);
                btnMove.setTag(null);
                btnScale.setTag(null);
                btnRotate.setTag(null);
                btnFlip.setTag(null);
                btnHide.setTag(null);
                btnSwap.setTag(null);

                mCurCollageTitleInfo = null;
                mCurCollageDrawInfo = null;
            }
            else {
                mEditText.setVisibility(View.VISIBLE);
                mEditText.setTag(collageInfo);
                // mEditText.setText("");

                mLayoutDraw.setVisibility(View.GONE);
                btnChange.setTag(null);
                btnMove.setTag(null);
                btnScale.setTag(null);
                btnRotate.setTag(null);
                btnFlip.setTag(null);
                btnHide.setTag(null);
                btnSwap.setTag(null);

                mCurCollageTitleInfo = (nexCollageInfoTitle)collageInfo;
                mCurCollageDrawInfo = null;

                InputFilter[] filterArray = new InputFilter[2];
                filterArray[0] = new InputFilter.LengthFilter(mCurCollageTitleInfo.getTitleMaxLength());
                filterArray[1] = getEditTextFilterEmoji();
                mEditText.setFilters(filterArray);

                mEditText.setMaxLines(mCurCollageTitleInfo.getTitleMaxLines());
                mEditText.setText(mCurCollageTitleInfo.getTitle(Locale.getDefault().getLanguage()));
            }
        }

        mOverlayView.setSelCollageInfo((nexCollageInfo)mCurCollageDrawInfo, (nexCollageInfo)mCurCollageTitleInfo);
        mOverlayView.invalidate();

        return true;
    }

    private boolean isExportCollage = false;
    private boolean isChangeCollage = false;
    private int isCurentPosition = 0;

    public class AdaptorCollageListItem extends BaseAdapter {

        @Override
        public int getCount() {
            return mCollages == null ? 0 : mCollages.size();
        }

        @Override
        public Object getItem(int arg0) {
            return mCollages == null ? null : mCollages.get(arg0);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {
            final int position = arg0;
      //      isCurentPosition = arg0;
            nexCollageManager.Collage collage = (nexCollageManager.Collage)getItem(arg0);

            if( collage == null )
                return null;

            View rowView = currentView;
            if( rowView == null ) {
                LayoutInflater inflater = CollageTestActivity.this.getLayoutInflater();
                rowView = inflater.inflate(R.layout.listitem_template, null, true);
            }

            LinearLayout background = (LinearLayout)rowView.findViewById(R.id.background);

            ImageView image = (ImageView)rowView.findViewById(R.id.icon);
            TextView name = (TextView)rowView.findViewById(R.id.title);
            Button play = (Button) rowView.findViewById(R.id.btn_play);
            Button del = (Button)rowView.findViewById(R.id.btn_del);
            play.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if( isExportCollage ) {
                        Log.d(TAG, "Collage is exporting now");
                        return;
                    }
                    if( isChangeCollage ) {
                        Log.d(TAG, "Collage already is changing now");
                        return;
                    }
                    watch.reset();
                    watch.start();

                    if (mCollageAdaptor.getCount() <= position)
                        return;

                    updateControlUI(null);

                    final nexCollageManager.Collage tempCollage = (nexCollageManager.Collage) mCollageAdaptor.getItem(position);
                    isChangeCollage = true;
                    isCurentPosition = position;
                    Log.d(TAG, "Collage with: " + tempCollage.id());

                    mEngine.stop();

                    if( tempCollage.equals(mCurCollage) ) {
                    }
                    else {
                        mCurCollage = tempCollage;
                        String errorMsg = setEffects2Project();
                        if (errorMsg != null) {
                            Log.d(TAG, errorMsg);

                            Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();

                            // finish();
                            return;
                        }
                    }
                    mCollageAdaptor.notifyDataSetChanged();
                    isChangeCollage = false;
                    playCollage();



//                    mEngine.stop(new nexEngine.OnCompletionListener() {
//                        @Override
//                        public void onComplete(int resultCode) {
//
//                            if( tempCollage.equals(mCurCollage) ) {
//                            }
//                            else {
//                                mCurCollage = tempCollage;
//                                String errorMsg = setEffects2Project();
//                                if (errorMsg != null) {
//                                    Log.d(TAG, errorMsg);
//
//                                    Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();
//
//                                    // finish();
//                                    return;
//                                }
//                            }
//                            mCollageAdaptor.notifyDataSetChanged();
//                            playCollage();
//                            isChangeCollage = false;
//                        }
//                    });
                }
            });

            del.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mCollageAdaptor.getCount() <= position)
                        return;

                    final nexCollageManager.Collage collage = (nexCollageManager.Collage) mCollageAdaptor.getItem(position);
                    if (collage != null && collage.isDelete()) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(CollageTestActivity.this);
                        String message = "Do you want to delete this item?";
                        builder.setMessage(message)
                                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        mKmCollageManager.uninstallPackageById(collage.id());

                                        refreshAssets();

                                        mEngine.stop(new nexEngine.OnCompletionListener() {
                                            @Override
                                            public void onComplete(int resultCode) {

                                                if( mCollages == null ) return;
                                                if( mCollages.size() <= 0 ) {
                                                    showEmptyCollageDialog();
                                                    return;
                                                }

                                                mCurCollage = mCollages.get(0);

                                                String errorMsg = setEffects2Project();
                                                if (errorMsg != null) {
                                                    Log.d(TAG, errorMsg);

                                                    Toast.makeText(CollageTestActivity.this, errorMsg, Toast.LENGTH_SHORT).show();

                                                    // finish();
                                                    return;
                                                }
                                                mCollageAdaptor.notifyDataSetChanged();
                                                playCollage();
                                            }
                                        });
                                    }
                                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                            }
                        });
                        builder.create().show();
                    }
                    return;
                }
            });

            image.setImageBitmap(mCollageIcons.get(arg0));
            name.setText(collage.name(null));  // for template localized name.

            if ( collage.packageInfo().expireRemain()!=0 ) Log.d(TAG, "Asset idx=" + collage.packageInfo().assetIdx() + " : Remain expiretime is = " + collage.packageInfo().expireRemain() + ", InstalledTime = "+collage.packageInfo().installedTime() + ". Expired="+nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).checkExpireAsset(collage.packageInfo()));

            boolean expire_warning = false;
            if ( collage.packageInfo().expireRemain()!=0 &&
                    (collage.packageInfo().installedTime() + collage.packageInfo().expireRemain() - (1000*60*60*24)) < System.currentTimeMillis() )
                expire_warning = true;// You should check that the asset will be expired in 24 hours. (1000*60*60*24) = 1day.

            if ( nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).checkExpireAsset(collage.packageInfo()) ) {
                background.setBackgroundColor(Color.RED);
                play.setVisibility(View.INVISIBLE); // When asset is already expired, don't use it. So hide play button.
            }
            else if ( expire_warning==true ) {
                background.setBackgroundColor(Color.MAGENTA);
            }
            else if( collage.equals(mCurCollage) )
                background.setBackgroundColor(0x3303A9F4);
            else
                background.setBackgroundColor(0x00000000);

            if( collage.isDelete() )
            {
                del.setEnabled(true);
            }
            else {
                del.setEnabled(false);
            }

            return rowView;
        }
    }

    public class AdaptorFilterListItem extends BaseAdapter {

        String [] colorEffects = nexColorEffect.getLutIds();

        @Override
        public int getCount() {
            // return nexColorEffect.getPresetList().size();
            return colorEffects == null ? 0 : colorEffects.length;
        }

        @Override
        public Object getItem(int arg0) {
            // return nexColorEffect.getPresetList().get(arg0);
            return colorEffects == null ? null : colorEffects[arg0];
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            // nexColorEffect filter = (nexColorEffect)getItem(arg0);
            String filter = (String)getItem(arg0);

            if( filter == null )
                return null;

            View rowView = currentView;
            if( rowView == null ) {
                LayoutInflater inflater = CollageTestActivity.this.getLayoutInflater();
                rowView = inflater.inflate(R.layout.listitem_colorfilter, null, true);
            }

            TextView name = (TextView)rowView.findViewById(R.id.name);

            // name.setText(filter.getPresetName());
            name.setText(filter);
            return rowView;
        }
    }

    void refreshAssets()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mCollages != null) mCollages.clear();

                if (mKmCollageManager != null) {
                    mKmCollageManager.loadCollage();
                    mCurCollage = null;
                    isCurentPosition =0;

                    String type = "All";
                    if( collageShowType == nexCollageManager.CollageType.ALL ) {
                        type = "All";
                    }else if( collageShowType == nexCollageManager.CollageType.StaticCollage ) {
                        type = "StaticCollage";
                    }else if( collageShowType == nexCollageManager.CollageType.DynamicCollage ) {
                        type = "DynamicCollage";
                    }

                    Log.d(TAG, "get Collages with: " + type);

                    mCollages = mKmCollageManager.getCollages(initUserSourceCount, collageShowType);
                    if (mCollages == null || mCollages.size() <= 0) {
                        Toast.makeText(CollageTestActivity.this, "Not available collages on this project, Sources count: " + mProject.getTotalClipCount(true), Toast.LENGTH_LONG).show();
                    }

                    for (Bitmap bitmap : mCollageIcons) {
                        if( bitmap != null )
                            bitmap.recycle();
                    }
                    mCollageIcons.clear();

                    for (nexCollageManager.Collage collage : mCollages) {
                        Log.d(TAG, "Collage is frame =" + collage.isFrameCollage());

                        mCollageIcons.add(collage.icon());
                    }
                }

                mCollageAdaptor.notifyDataSetChanged();

                nexColorEffect.updatePluginLut();
            }
        });
    }

    private nexEngineListener sEngineListener = new nexEngineListener() {
        @Override
        public void onStateChange(int i, int i1) {
            mEngineState = i1;
            Log.d(TAG, "onStateChange() state=" + mEngineState);
        }

        @Override
        public void onTimeChange(int i) {

        }

        @Override
        public void onSetTimeDone(int i) {
            showOverlayView(true);

            Log.d(TAG, "onSetTimeDone");
            if( watch.isRunning() ) {
                watch.stop();
                Toast.makeText(CollageTestActivity.this, "Collage change and play start elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();

            }
        }

        @Override
        public void onSetTimeFail(int i) {

        }

        @Override
        public void onSetTimeIgnored() {

        }

        @Override
        public void onEncodingDone(boolean b, int i) {

        }

        @Override
        public void onPlayEnd() {
            Log.d(TAG, "onPlayEnd");
            if( mEngine != null && mCurCollage != null )
                mEngine.seek(mCurCollage.getEditTime());
        }

        @Override
        public void onPlayFail(int i, int i1) {
        }

        @Override
        public void onPlayStart() {
//            mPlaying =true;

            Log.d(TAG, "onPlayStart");

            showOverlayView(false);

            if( watch.isRunning() ) {
                watch.stop();
                Toast.makeText(CollageTestActivity.this, "Collage change and play start elapsed=" + watch.toString(), Toast.LENGTH_LONG).show();
                Log.d(TAG, "Collage change and play start elapsed = " + watch.toString());
            }
            else {
                long elapsedTime = System.nanoTime() - startActivityTime;
                Log.d(TAG, "CollageTestActivity Start and play elapsed = " + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0));
                Toast.makeText(CollageTestActivity.this, "CollageTestActivity Start and play elapsed=" + String.format("%1$,.3f ms", (double) elapsedTime / 1000000.0), Toast.LENGTH_LONG).show();
            }
        }

        @Override
        public void onClipInfoDone() {

        }

        @Override
        public void onSeekStateChanged(boolean b) {

        }

        @Override
        public void onEncodingProgress(int i) {

        }

        @Override
        public void onCheckDirectExport(int i) {

        }

        @Override
        public void onProgressThumbnailCaching(int i, int i1) {

        }

        @Override
        public void onFastPreviewStartDone(int i, int i1, int i2) {

        }

        @Override
        public void onFastPreviewStopDone(int i) {

        }

        @Override
        public void onFastPreviewTimeDone(int i) {

        }

        @Override
        public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

        }
    };
    static Intent makeGalleryIntent(){
            Intent intent = new Intent( );
            intent.setType("*/*");
            String[] mimetypes = new String[]{"image/*"};
            intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
            intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
            intent.setAction(Intent.ACTION_GET_CONTENT);
            return intent;
    }
    private static final int SPAN_SLOP = 7;
    private float preSpanDelta = 0;
    private boolean gestureTolerance(ScaleGestureDetector detector) {
        final float spanDelta = Math.abs(detector.getCurrentSpan() - detector.getPreviousSpan());
        Log.d(TAG, "spanDelta/preSpanDelta:"+spanDelta+"/"+preSpanDelta);
        if(preSpanDelta != 0 && spanDelta == preSpanDelta) {
            return false;
        }
        else {
            preSpanDelta = spanDelta;
            return spanDelta > SPAN_SLOP;
        }
    }

    public class MyScaleGestures implements View.OnTouchListener, ScaleGestureDetector.OnScaleGestureListener{
        private View view;
        private ScaleGestureDetector gestureScale;
        private float scaleFactor = 1;
        public MyScaleGestures(Context c){
            gestureScale = new ScaleGestureDetector(c, this);
        }

        @Override
        public boolean onTouch(View view, MotionEvent event) {
            this.view = view;
            gestureScale.onTouchEvent(event);
            Log.d(TAG, "motion event:"+event.getAction()+", point count :"+event.getPointerCount());

            return true;
        }

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            Log.d(TAG, "detector.getScaleFactor():"+detector.getScaleFactor());
            if (gestureTolerance(detector)) {
                scaleFactor *= detector.getScaleFactor();
                scaleFactor = (scaleFactor < 0.5 ? 0.5f : scaleFactor); // prevent our view from becoming too small //
                scaleFactor = ((float) ((int) (scaleFactor * 100))) / 100; // Change precision to help with jitter when user just rests their fingers //
            }
            return true;

        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            return true;
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {  }

    }

    private static final int INVALID_POINTER_INDEX = -1;
    private static final int INVALID_POINTER_ID = -1;
    private int mPtrIndex1 = INVALID_POINTER_INDEX;
    private int mPtrID1 = INVALID_POINTER_ID;
    private float mAngle;
    private float oldAngle=0;
    float firstX =0;
    float firstY =0;
    private float move(MotionEvent event) {

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                Log.v(TAG, "ACTION_DOWN");
                mPtrIndex1 =(event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                mPtrID1 = event.getPointerId(event.getActionIndex());
                firstX = event.getX();
                firstY = event.getY();
                Log.d(TAG, String.format("ACTION_DOWN %d %d (%f / %f)", mPtrID1, mPtrIndex1, firstX, firstY));
                break;
            case MotionEvent.ACTION_MOVE:
                Log.d(TAG, String.format("onMove(%d %d)", mPtrID1, event.getPointerId(event.getActionIndex())));
                if( mPtrID1 == event.getPointerId(event.getActionIndex()) ) {
                    Log.d(TAG, String.format("onMove(%f / %f / %f / %f)", firstX, event.getX(mPtrIndex1), firstY, event.getY(mPtrIndex1)));
                    mCurCollageDrawInfo.setTranslate((int)(firstX - event.getX(mPtrIndex1)), (int)(firstY - event.getY(mPtrIndex1)), mView.getMeasuredWidth(), mView.getMeasuredHeight());
                    firstX = event.getX(mPtrIndex1);
                    firstY = event.getY(mPtrIndex1);
                }
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                mPtrID1 = INVALID_POINTER_ID;
                mPtrIndex1 = INVALID_POINTER_INDEX;
                break;
            default:
                break;
        }
        return 0;
    }

    private File getExportFile(int wid , int hei, String ext) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();

        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NexEditor_"+wid+"X"+hei + "_" + export_time+"." + ext);
        return exportFile;
    }

    public class AdaptorVendorListItem extends BaseAdapter {

        String[] vendorList = getResources().getStringArray(R.array.VendorList);

        @Override
        public int getCount() {
            return vendorList == null ? 0 : vendorList.length;
        }

        @Override
        public Object getItem(int arg0) {
            return vendorList == null ? null : vendorList[arg0];
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int arg0, View currentView, ViewGroup parentView) {

            String vendor = (String)getItem(arg0);

            if( vendor == null )
                return null;

            if( currentView == null ) {
                LayoutInflater inflater = CollageTestActivity.this.getLayoutInflater();
                currentView = inflater.inflate(R.layout.listitem_vendor, null, true);
            }

            TextView v = (TextView)currentView.findViewById(R.id.vendor);
            v.setText(vendor);

            if( storeVendor != null && storeVendor.compareTo(vendor) == 0 ) {
                v.setTextColor(Color.argb(255, 255, 0, 0));
            }
            else {
                v.setTextColor(Color.argb(255, 255, 255, 255));
            }

            return currentView;
        }
    }
    private int aniGifQuality = 0;
    public void setOption()
    {
        final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(CollageTestActivity.this);

        LayoutInflater inflater = CollageTestActivity.this.getLayoutInflater();

        View dialogView = inflater.inflate(R.layout.dialog_collage_set, null);

        RadioButton.OnClickListener showOnClickListener = new RadioButton.OnClickListener() {
            public void onClick(View v) {
                collageShowType = (nexCollageManager.CollageType)v.getTag();
            }
        };

        final RadioButton rShowAll = (RadioButton)dialogView.findViewById(R.id.rCollageShowAll);
        final RadioButton rShowStatic = (RadioButton)dialogView.findViewById(R.id.rCollageShowStatic);
        final RadioButton rShowDynamic = (RadioButton)dialogView.findViewById(R.id.rCollageShowDynamic);

        rShowAll.setTag(nexCollageManager.CollageType.ALL);
        rShowStatic.setTag(nexCollageManager.CollageType.StaticCollage);
        rShowDynamic.setTag(nexCollageManager.CollageType.DynamicCollage);

        rShowAll.setOnClickListener(showOnClickListener);
        rShowStatic.setOnClickListener(showOnClickListener);
        rShowDynamic.setOnClickListener(showOnClickListener);

        if( collageShowType == nexCollageManager.CollageType.ALL ) {
            rShowAll.setChecked(true);
        } else if( collageShowType == nexCollageManager.CollageType.StaticCollage ) {
            rShowStatic.setChecked(true);
        } else if( collageShowType == nexCollageManager.CollageType.DynamicCollage ) {
            rShowDynamic.setChecked(true);
        }

        ListView bgmList = (ListView)dialogView.findViewById(R.id.listview_bgm);
        final AdaptorVendorListItem vendorAdaptor = new AdaptorVendorListItem();
        bgmList.setAdapter(vendorAdaptor);
        bgmList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                storeVendor = (String)vendorAdaptor.getItem(position);
                vendorAdaptor.notifyDataSetChanged();
            }
        });

        RadioButton.OnClickListener storeOnClickListener = new RadioButton.OnClickListener() {
            public void onClick(View v) {
                collageStoreType = (int)v.getTag();
            }
        };

        final RadioButton rStoreAll = (RadioButton)dialogView.findViewById(R.id.rCollageStoreAll);
        final RadioButton rStoreStatic = (RadioButton)dialogView.findViewById(R.id.rCollageStoreStatic);
        final RadioButton rStoreDynamic = (RadioButton)dialogView.findViewById(R.id.rCollageStoreDynamic);

        rStoreAll.setTag(nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage);
        rStoreStatic.setTag(nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage);
        rStoreDynamic.setTag(nexAssetStoreAppUtils.AssetStoreMimeType_DynamicCollage);

        rStoreAll.setOnClickListener(storeOnClickListener);
        rStoreStatic.setOnClickListener(storeOnClickListener);
        rStoreDynamic.setOnClickListener(storeOnClickListener);

        if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_IntegratedCollage ) {
            rStoreAll.setChecked(true);
        } else if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_StaticCollage ) {
            rStoreStatic.setChecked(true);
        } else if( collageStoreType == nexAssetStoreAppUtils.AssetStoreMimeType_DynamicCollage ) {
            rStoreDynamic.setChecked(true);
        }

        final EditText editSourceCount = (EditText)dialogView.findViewById(R.id.editText_source_count);
        editSourceCount.setText(""+collageStoreSourceCount);


        dialogBuilder.setView(dialogView).
                setTitle("Set option").
                setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        String sc = editSourceCount.getText().toString();
                        if( sc != null && sc.length() > 0 ){
                            try {
                                collageStoreSourceCount = Integer.parseInt(sc);
                            }
                            catch(NumberFormatException e){
                                collageStoreSourceCount = 1;
                            }
                        }
                        else {
                            collageStoreSourceCount = 1;
                        }
                        refreshAssets();
                    }
                });
        dialogBuilder.create().show();

        final SeekBar seekBarAniGifQuality = (SeekBar)dialogView.findViewById(R.id.seekBar_anigif_quality);
        seekBarAniGifQuality.setMax(100);
        seekBarAniGifQuality.setProgress(aniGifQuality);

        seekBarAniGifQuality.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                aniGifQuality = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }
    public void setNexFaceModule(){
        mEngine.setFaceModule("6a460d22-cd87-11e7-abc4-cec278b6b50a");
    }
}
