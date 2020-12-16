package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.media.MediaMetadataRetriever;
import android.media.MediaScannerConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.nexstreaming.editordemo.utility.utilityCode;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTPreviewFragment;
import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexOverlayManager;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTemplateComposer;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdk.nexTemplateTitle;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

public class kmTemplateActivity extends Activity implements kmSTPreviewFragment.OnSTPreviewListener {

    private static final String TAG = "KM_ThemePreviewActivity";

    private ProgressBar mProgressBar;
    private ListView mListView;
    private AlertDialog mAlertDialog;

    private dataParcel mDataParcel;

    private nexProject mProject;
    private nexProject mCloneProject;
    private nexEngine mEngine;

    private ImageView mIVBack;
    private ImageView mIVPlay;
    private ImageView mIVETC;
    private TextView mTVPlayingTime;
    private TextView mTVTotlaTime;
    private SeekBar mSeekBar;

    private boolean mIsLCDoff;
    private boolean mIsSecured;
    private int mAspectMode;

    private File mFile;

    private final int PLAYING = 1;
    private final int PAUSE = 2;
//    private final int COMPLETE = 3;
    private final int ENCODING = 4;
    private final int ENCODING_COMPLETE = 5;
    private final int IDLE = 6;
    private final int ERROR = 7;

    private int mActivityStatus = PLAYING;

    public static Activity previewAct;

    private nexTemplateComposer mKmTemplateComposer;
    private nexTemplateManager mKmTemplateManager;

    private nexOverlayManager mKmOverlayManager;
    private int mOverApplyIndex = 0;
    private ArrayList<String> mTemplateList = new ArrayList<>();
    private ArrayList<Integer> mTemplateThumbList = new ArrayList<>();
    private SparseArray<String> mTemplateFileList = new SparseArray<>();

    private TemplateListAdapter mAdpater;

    void consistListItem(final int aspectMode) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected void onPreExecute() {
                mTemplateList.clear();
            }

            @Override
            protected Void doInBackground(Void... params) {
                ArrayList<String> paths = new ArrayList<>();

                /*
                **  i. read files in the specified directory
                **  Priority. files in "assets" > files in "sd card"
                **  ii. construct the list with key and path
                **  iii. get item to the list and consist list items.
                **/

                int index = 0;
//                try {
//
//                    if(aspectMode==0/*16:9*/) {
//                        if(getAssets().list(new String("template/16v9")).length != 0) {
//                            String[] list = getAssets().list(new String("template/16v9"));
//                            for(String file : list) {
//                                paths.add(new String("template/16v9/")+file);
//                            }
//                        }
//                        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
//                        if(new File(sdCardPath+"/NexDemo/Template/16v9/").listFiles() != null) {
//                            File folder = new File(sdCardPath+"/NexDemo/Template/16v9/");
//                            File[] list = folder.listFiles();
//                            for(File file : list) {
//                                paths.add(new String(sdCardPath+"/NexDemo/Template/16v9/")+file.getName());
//                            }
//                        }
//
//                        retreviewTemplate(paths, "/NexDemo/Template/", "_16v9_");
//                    } else if(aspectMode==1/*9:16*/) {
//                        if(getAssets().list(new String("template/9v16")).length != 0) {
//                            String[] list = getAssets().list(new String("template/9v16"));
//                            for(String file : list) {
//                                paths.add(new String("template/9v16/")+file);
//                            }
//                        }
//
//                        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
//                        if(new File(sdCardPath+"/NexDemo/Template/9v16/").listFiles() != null) {
//                            File folder = new File(sdCardPath+"/NexDemo/Template/9v16/");
//                            File[] list = folder.listFiles();
//                            for(File file : list) {
//                                paths.add(new String(sdCardPath+"/NexDemo/Template/9v16/")+file.getName());
//                            }
//                        }
//
//                        retreviewTemplate(paths, "/NexDemo/Template/", "_9v16_");
//                    } else/*1:1*/ {
//                        if(getAssets().list(new String("template/1v1")).length != 0) {
//                            String[] list = getAssets().list(new String("template/1v1"));
//                            for(String file : list) {
//                                paths.add(new String("template/1v1/")+file);
//                            }
//                        }
//                        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
//                        if(new File("/storage/emulated/0/NexDemo/Template/1v1/").listFiles() != null) {
//                            File folder = new File(sdCardPath+"/NexDemo/Template/1v1/");
//                            File[] list = folder.listFiles();
//                            for(File file : list) {
//                                paths.add(new String(sdCardPath+"/NexDemo/Template/1v1/")+file.getName());
//                            }
//                        }
//
//                        retreviewTemplate(paths, "/NexDemo/Template/", "_1v1_");
//                    }
//
//
//                    String jsonStr;
//                    String template_name;
//                    JSONObject jsonObject;
//                    for(String path : paths) {
//                        if(path.regionMatches(true, 0, "template", 0, 8)) {
//                            jsonStr = readFromFile(getAssets().open(path));
//                        } else {
//                            jsonStr = readFromFile(path);
//                        }
//                        jsonObject = new JSONObject(jsonStr);
//                        template_name = jsonObject.getString("template_name");
//                        mTemplateList.add(template_name);
//                        mTemplateThumbList.add(getThumbnail4Template(template_name));
//                        mTemplateFileList.put(index, path);
//                        index++;
//
//                    }
//                } catch (JSONException e) {
//                    e.printStackTrace();
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }

                for (nexTemplateManager.Template template : mKmTemplateManager.getTemplates()) {
                    if( template.aspect() == 16/9.0f && aspectMode==0 )
                    {
                        mTemplateList.add(template.name("en"));
                        mTemplateFileList.put(index, template.id());
                        mTemplateThumbList.add(getThumbnail4Template(template.name("en")));
                        index++;
                    }
                    else if( template.aspect() == 9/16.0f && aspectMode==1 )
                    {
                        mTemplateList.add(template.name("en"));
                        mTemplateFileList.put(index, template.id());
                        mTemplateThumbList.add(getThumbnail4Template(template.name("en")));
                        index++;
                    }
                    else if( template.aspect() == 1.0f && aspectMode==2 )
                    {
                        mTemplateList.add(template.name("en"));
                        mTemplateFileList.put(index, template.id());
                        mTemplateThumbList.add(getThumbnail4Template(template.name("en")));
                        index++;
                    }

                    Log.d(TAG, String.format("Template Activity load templates(%s) (%f)", template.name("en"), template.aspect()));
                }

                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                mAdpater = new TemplateListAdapter();
                mListView = (ListView) findViewById(R.id.template_list);
                mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        selectedListIdx = position;
                        mAdpater.notifyDataSetChanged();

                        setActivityStatus(IDLE);
                        mEngine.stop(new nexEngine.OnCompletionListener() {
                            @Override
                            public void onComplete(int resultCode) {
                                mEngine.clearProject();
                                mCloneProject = nexProject.clone(mProject);
                                String path = mTemplateFileList.get(selectedListIdx);

                                ArrayList<nexTemplateTitle> titleInfo = mKmTemplateComposer.parseTitleOfTemplate(getApplicationContext(), getApplicationContext(), path);
                                
                                String errorMsg = mKmTemplateComposer.setTemplateEffects2Project(mCloneProject, getApplicationContext(), getApplicationContext(), path, false, titleInfo);
                                if (errorMsg != null) {
                                    if (mAlertDialog != null) mAlertDialog.dismiss();
                                    mAlertDialog = displayErrorDialog(errorMsg);
                                    mAlertDialog.show();
                                    // isFinish = true;
                                    return;
                                }

                                List<nexOverlayManager.Overlay> lists = mKmOverlayManager.getOverlays();
                                if( lists.size() > 0 )
                                {
                                    List<nexOverlayManager.nexTitleInfo> titleArray = new ArrayList<nexOverlayManager.nexTitleInfo>();
                                    mKmOverlayManager.parseOverlay(lists.get(mOverApplyIndex).getId(), titleArray);
                                    for( nexOverlayManager.nexTitleInfo title : titleArray )
                                    {
                                        // title.setText(lists.get(mOverApplyIndex).getName("en"));
                                    }

                                    mKmOverlayManager.applyOverlayToProjectById(mCloneProject, lists.get(mOverApplyIndex).getId(), titleArray);

                                    mOverApplyIndex++;
                                    mOverApplyIndex = mOverApplyIndex >= lists.size() ? 0 : mOverApplyIndex;
                                }

                                mEngine.setProject(mCloneProject);
                                setPlay();
                            }
                        });
                    }
                });

                mListView.setAdapter(mAdpater);
                if(mTemplateList.size() == 0) {
                    TextView emptyview = (TextView) findViewById(R.id.emptyview);
                    emptyview.setVisibility(View.VISIBLE);
                } else {
                    mEngine.clearProject();
                    mCloneProject = nexProject.clone(mProject);
                    String path = mTemplateFileList.get(selectedListIdx);

                    ArrayList<nexTemplateTitle> titleInfo = mKmTemplateComposer.parseTitleOfTemplate(getApplicationContext(), getApplicationContext(), path);

                    String errorMsg = mKmTemplateComposer.setTemplateEffects2Project(mCloneProject, getApplicationContext(), getApplicationContext(), path, false, titleInfo);
                    if (errorMsg != null) {
                        if (mAlertDialog != null) mAlertDialog.dismiss();
                        mAlertDialog = displayErrorDialog(errorMsg);
                        mAlertDialog.show();
                        // isFinish = true;
                        return;
                    }

                    List<nexOverlayManager.Overlay> lists = mKmOverlayManager.getOverlays();
                    if( lists.size() > 0 )
                    {
                        List<nexOverlayManager.nexTitleInfo> titleArray = new ArrayList<nexOverlayManager.nexTitleInfo>();
                        mKmOverlayManager.parseOverlay(lists.get(mOverApplyIndex).getId(), titleArray);
                        mKmOverlayManager.applyOverlayToProjectById(mCloneProject, lists.get(mOverApplyIndex).getId(), titleArray);

                        mOverApplyIndex++;
                        mOverApplyIndex = mOverApplyIndex >= lists.size() ? 0 : mOverApplyIndex;
                    }

                    mEngine.setProject(mCloneProject);
                    setPlay();
                }
            }
        }.execute();
    }

    void retreviewTemplate(ArrayList<String> paths, String subDir, String token)
    {

        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        if(new File(sdCardPath+subDir).listFiles() != null) {
            File folder = new File(sdCardPath+subDir);
            File[] list = folder.listFiles();
            for(File file : list) {
                if( file.isDirectory() )
                {
                    File[] subDirs = file.listFiles();
                    for( File f : subDirs )
                    {
                        if( f.isFile() )
                        {
                            if( f.getAbsolutePath().contains(token) )
                            {
                                paths.add(f.getAbsolutePath());
                            }
                        }
                    }
                }
            }
        }
    }

    String readFromFile(String path) {
        String content = null;
        File file = new File(path);
        try {
            InputStream inputStream = new BufferedInputStream(new FileInputStream(file));
            if(inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferReader = new BufferedReader(inputStreamReader);
                StringBuilder stringBuilder = new StringBuilder();
                String receiveString;

                while((receiveString = bufferReader.readLine()) != null) {
                    if(receiveString.contains("template_name")) {
                        stringBuilder.append(receiveString, 0, receiveString.length()-1);
                        stringBuilder.append("\n");
                        stringBuilder.append("}");
                        stringBuilder.append("\n");
                        break;
                    } else {
                        stringBuilder.append(receiveString);
                        stringBuilder.append("\n");
                    }
                }

                inputStream.close();
                content = stringBuilder.toString();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return content;
    }

    String readFromFile(InputStream inputStream) {
        String content = null;
        try {
            InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
            BufferedReader bufferReader = new BufferedReader(inputStreamReader);
            StringBuilder stringBuilder = new StringBuilder();
            String receiveString;

            while((receiveString = bufferReader.readLine()) != null) {
                if(receiveString.contains("template_name")) {
                    stringBuilder.append(receiveString, 0, receiveString.length()-1);
                    stringBuilder.append("\n");
                    stringBuilder.append("}");
                    stringBuilder.append("\n");
                    break;
                } else {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }
            }

            inputStream.close();
            content = stringBuilder.toString();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return content;
    }

    int getThumbnail4Template(String template_name) {
        int thumbnail_resource = -1;

        switch(template_name) {
            case "love":
            case "love 9v16":
                thumbnail_resource = R.mipmap.theme_thumb_love;
                break;
            case "family":
            case "family 9v16":
                thumbnail_resource = R.mipmap.theme_thumb_family;
                break;
            case "oldfilm":
            case "oldfilm 9v16":
                thumbnail_resource = R.mipmap.theme_thumb_remember;
                break;
            case "sports":
            case "sports 9v16":
                thumbnail_resource = R.mipmap.theme_thumb_sports;
                break;
            case "Grid":
                thumbnail_resource = R.mipmap.theme_thumb_grid;
                break;
            case "Simple":
                thumbnail_resource = R.mipmap.theme_thumb_simple;
                break;
            case "Symmetry":
                thumbnail_resource = R.mipmap.theme_thumb_symmetry;
                break;
            case "PhotoFrame":
                thumbnail_resource = R.mipmap.theme_thumb_photo_frame;
                break;
            default:
                thumbnail_resource = R.mipmap.default_thumbid;
                break;
        }

        return thumbnail_resource;
    }

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

        kmAspectModeListActivity.templatelistAct.finish();
        previewAct = this;

        mDataParcel = getIntent().getParcelableExtra("parcelData");
        mAspectMode = getIntent().getIntExtra("aspectMode", -1);

        if(mAspectMode==1/*9:16*/) {
            setContentView(R.layout.activity_km_themepreview4);
        }
        else /*16:9*//*1:1*/ {
            setContentView(R.layout.activity_km_themepreview2);

            DisplayMetrics displayMetrics = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

            int deviceHeight = displayMetrics.heightPixels;
            int deviceWidth = displayMetrics.widthPixels;

            if(mAspectMode == 0 /* 16:9 */) {
                FrameLayout layout = (FrameLayout) findViewById(R.id.fragment_preview);
                ViewGroup.LayoutParams params = layout.getLayoutParams();

                params.height = deviceHeight-(int)utilityCode.convertDpToPixel(366, this);

                FrameLayout layout2 = (FrameLayout) findViewById(R.id.surface);
                ViewGroup.LayoutParams params2 = layout2.getLayoutParams();

                params2.height = (int)((deviceWidth*9)/16);
            } else /* 1:1 */ {
                FrameLayout layout = (FrameLayout) findViewById(R.id.fragment_preview);
                ViewGroup.LayoutParams params = layout.getLayoutParams();

                params.height = deviceWidth;

                FrameLayout layout2 = (FrameLayout) findViewById(R.id.surface);
                ViewGroup.LayoutParams params2 = layout2.getLayoutParams();

                params2.height = deviceWidth;
            }
        }
        getFragmentManager().beginTransaction()
                .replace(R.id.surface, new kmSTPreviewFragment(), "previewFragment")
                .commit();

        int index = 0;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();

        nexEngine.setLoadListAsync(true);
        mEngine = kmEngine.getEngine();

        mKmTemplateComposer = new nexTemplateComposer();
        mProject = mKmTemplateComposer.createProject();

        mKmTemplateManager = nexTemplateManager.getTemplateManager(getApplicationContext(), getApplicationContext());
        mKmTemplateManager.loadTemplate();


        mKmOverlayManager = nexOverlayManager.getOverlayManager(getApplicationContext(), getApplicationContext());
        mKmOverlayManager.loadOverlay();

        // nexAssetPackageManager am = nexAssetPackageManager.getAssetPackageManager(getApplicationContext());
        // am.getInstalledAssetItems(Cate)

        boolean is_exist_errclip = false;

        for( int i=0; i<mDataParcel.getPath().size(); i++ ) {
            String path = mDataParcel.getPath().get(i);
            if(path == null) {
                setActivityStatus(ERROR);
                break;
            }
            nexClip clip = nexClip.getSupportedClip(path);
            if(clip != null) {
                mProject.add(clip);
                if(mProject.getTotalClipCount(true) != 0
                        && mProject.getClip(index, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
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
                    }
                }
                index++;
            } else {
                is_exist_errclip = true;
            }
        }

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
            consistListItem(mAspectMode);
            mEngine.setProject(mCloneProject);
        }
    }

    private int selectedListIdx = 0;

    private class TemplateListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public TemplateListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mTemplateList.size();
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi = convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem4_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);

            imageView.setImageResource(mTemplateThumbList.get(position));

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mTemplateList.get(position));

            ImageView checkerView = (ImageView) vi.findViewById(R.id.list_checker);
            if(selectedListIdx != position) {
                checkerView.setVisibility(View.INVISIBLE);
            } else {
                checkerView.setVisibility(View.VISIBLE);
            }

            return vi;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(mTemplateList != null) {
            mTemplateList.clear();
        }
        if(mTemplateFileList != null) {
            mTemplateFileList.clear();
        }
        if(mTemplateThumbList != null) {
            mTemplateThumbList.clear();
        }
        if(mKmTemplateComposer != null) {
            mKmTemplateComposer.release();
        }
        if(getActivityStatus() != ERROR) {
            mEngine.stop(new nexEngine.OnCompletionListener() {
                @Override
                public void onComplete(int resultCode) {
                    setActivityStatus(PLAYING);
                    kmEngine.releaseEngine(); //
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
                    kmEngine.createEngine(); //
                }
            });
        }
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

    private void setPlay() {
        if(getActivityStatus() == ERROR) {
            return;
        }
        setActivityStatus(PLAYING);
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");
        fragment.setPlay(true);
//        fragment.setPause();
//        fragment.setSeek(0);
    }

    private void setPause() {
        if(getActivityStatus() == ERROR) {
            return;
        }
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
        if(mAspectMode==0/*16:9*/) {
            mEngine.export(mFile.getAbsolutePath(), 1280, 720, 6 * 1024 * 1024, Long.MAX_VALUE, 0);
        } else if(mAspectMode==1/*9:16*/) {
            mEngine.export(mFile.getAbsolutePath(), 720, 1280, 6 * 1024 * 1024, Long.MAX_VALUE, 0);
        } else {
            mEngine.export(mFile.getAbsolutePath(), 480, 480, 3 * 1024 * 1024, Long.MAX_VALUE, 0);
        }
    }

    @Override
    public void onSTEncodingSuccess() {
        mListView.setEnabled(false);
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
        finish();
//        dataParcel parcel = dataParcel.getDataParcel(null, mDataParcel.getPath(), null);
//
//        Intent intent = new Intent(getBaseContext(), kmAspectModeListActivity.class);
//        intent.putExtra("from_preview", 1);
//        intent.putExtra("parcelData", parcel);
//        startActivity(intent);
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

    @Override
    public void onSTSetupEngine() {
        kmSTPreviewFragment fragment = (kmSTPreviewFragment)getFragmentManager().findFragmentByTag("previewFragment");

        mIVBack = (ImageView) findViewById(R.id.preview_back);
        mIVETC = (ImageView) findViewById(R.id.preview_etc);
        mIVPlay = (ImageView) findViewById(R.id.preview_play);
        mTVPlayingTime = (TextView) findViewById(R.id.preview_playingTime);
        mTVTotlaTime = (TextView) findViewById(R.id.preview_TotalTime);
        mSeekBar = (SeekBar) findViewById(R.id.preview_seek);

        fragment.setEngine(mEngine, true);
        fragment.setViews(mIVBack, mIVETC, mIVPlay, mTVPlayingTime, mTVTotlaTime, mSeekBar);
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
