package com.nexstreaming.app.vasset.global.ui;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.widget.NestedScrollView;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.animation.AlphaAnimation;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import com.android.volley.VolleyError;
import com.android.volley.toolbox.ImageLoader;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.ConfigGlobal;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.vasset.global.asset.AssetManager;
import com.nexstreaming.app.vasset.global.download.DownloadHelper;
import com.nexstreaming.app.vasset.global.download.DownloadInfo;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.vasset.global.ui.adapter.AssetDetailImageListAdapter;
import com.nexstreaming.app.assetlibrary.utils.CommonUtils;
import com.nexstreaming.app.assetlibrary.utils.DialogUtil;

import java.util.ArrayList;

import jp.wasabeef.recyclerview.animators.FadeInAnimator;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class AssetDetailActivity extends BaseActivity implements MediaPlayer.OnPreparedListener
        , MediaPlayer.OnCompletionListener{

    private static final String TAG = "AssetDetailActivity";

    private static final String KEY_ASSET_INDEX = "asset_index";
    private static final String KEY_ASSET_IMAGE_URL = "asset_image";

    private static final int MESSAGE_SHOW_PLAY_BUTTON = 0x00;
    private static final int MESSAGE_HIDE_PLAY_BUTTON = 0x01;


    private NestedScrollView mScrollView;

    private ViewGroup mLayoutImage;
    private ImageView mMainImage;
    private TextView mTitle;

    private TextView mAssetTitle;
    private TextView mAssetCategoryName;
    private TextView mAssetFileSize;

    private ImageView mAssetPlay;

    private Button mActionButton;
    private View mLayoutDownload;
    private TextView mDownloadPercent;
    private ProgressBar mDownloadProgress;

    private TextView mDescription;

    private RecyclerView mImageList;
    private AssetDetailImageListAdapter mImageListAdapter;

    private View mNetworkErrorLayout;
    private ProgressBar mLoadingView;
    private ProgressBar mVideoLoading;

    private int mAssetIndex;
    private String mAssetImageUrl;
    private boolean mMultiSelectFlag;

    private VideoView mVideoView;
    private MediaPlayerWrapper mMediaPlayerWrapper;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_detail);
        initData();
        initView();
        loadData();
    }

    private void initData(){
        mAssetIndex = getIntent().getIntExtra(KEY_ASSET_INDEX, -1);
        mAssetImageUrl = getIntent().getStringExtra(KEY_ASSET_IMAGE_URL);
        mMultiSelectFlag = getIntent().getBooleanExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, true);
        mImageListAdapter = new AssetDetailImageListAdapter(new ArrayList<String>());

        if(mAssetIndex < 0){
            if(getIntent() != null){
                String index = getIntent().getData().getQueryParameter("index");
                try{
                    mAssetIndex = Integer.parseInt(index);
                }catch (Exception e){
                    Log.w(TAG, "Invalid asset index: ", e);
                }
            }
        }

        if(mAssetIndex < 0){
            finish();
        }
    }

    private void initView(){
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle("");

        mScrollView = (NestedScrollView) findViewById(R.id.sv_detail);
        mLayoutImage = (ViewGroup) findViewById(R.id.layout_detail_main_image);
        mVideoView = (VideoView) findViewById(R.id.video_detail_main);
        mMainImage = (ImageView) findViewById(R.id.iv_detail_main_image);
        mAssetPlay = (ImageView) findViewById(R.id.iv_detail_play);

        mTitle = (TextView) findViewById(R.id.toolbar_title);

        mAssetTitle = (TextView) findViewById(R.id.tv_detail_title);
        mAssetCategoryName = (TextView) findViewById(R.id.tv_detail_category);
        mAssetFileSize = (TextView) findViewById(R.id.tv_detail_file_size);

        mActionButton = (Button) findViewById(R.id.btn_detail_action);
        mLayoutDownload = findViewById(R.id.layout_detail_download);
        mDownloadPercent = (TextView) findViewById(R.id.tv_detail_download_percent);
        mDownloadProgress = (ProgressBar) findViewById(R.id.pb_detail_download);

        mDescription = (TextView) findViewById(R.id.tv_detail_description);

        mImageList = (RecyclerView) findViewById(R.id.rv_detail_images);

        mLoadingView = (ProgressBar) findViewById(R.id.pb_detail);
        mVideoLoading = (ProgressBar) findViewById(R.id.pb_detail_video);

        mNetworkErrorLayout = findViewById(R.id.layout_detail_network_error);

        mImageList.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false));
        mImageList.setItemAnimator(new FadeInAnimator());
        mImageList.setAdapter(mImageListAdapter);

        mTitle.setText(R.string.details);
        if(mAssetImageUrl != null){
            KMVolley.getInstance(this).getImageLoader().get(mAssetImageUrl, new ImageLoader.ImageListener() {
                @Override
                public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {
                    if(response != null && response.getBitmap() != null && !response.getBitmap().isRecycled()){
                        mMainImage.setImageBitmap(response.getBitmap());
                    }
                }

                @Override
                public void onErrorResponse(VolleyError error) {

                }
            });
        }

        mScrollView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                int width = getResources().getDisplayMetrics().widthPixels;
                float height = (float) (width / 1.77);
                mLayoutImage.getLayoutParams().height =  (int) height;
                mLayoutImage.requestLayout();
            }
        });

        mLayoutImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(mMediaPlayerWrapper != null){
                    if(mAssetPlay.isShown()){
                        sendMessage(MESSAGE_HIDE_PLAY_BUTTON, 500L);
                    }else{
                        mAssetPlay.setVisibility(View.VISIBLE);
                        sendMessage(MESSAGE_HIDE_PLAY_BUTTON, 500L);
                    }
                }
            }
        });
    }

    private void loadData(){
        showLoadingView();
        AssetStoreSession.getInstance(this).getAssetInfo(mAssetIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreAssetInfo>() {
            @Override
            public void onResultAvailable(ResultTask<StoreAssetInfo> task, Task.Event event, StoreAssetInfo result) {
                hideLoadingView();
                setData(result);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                hideLoadingView();
                mNetworkErrorLayout.setVisibility(View.VISIBLE);
                mScrollView.setVisibility(View.GONE);
            }
        });
    }

    private void setData(final StoreAssetInfo assetInfo){
        if(assetInfo != null){

            KMVolley.getInstance(this).getImageLoader().get(assetInfo.getAssetThumbnailURL_L(), new ImageLoader.ImageListener() {
                @Override
                public void onResponse(final ImageLoader.ImageContainer response, boolean isImmediate) {
                    if(response != null
                            && response.getBitmap() != null && !response.getBitmap().isRecycled()){
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mMainImage.setImageBitmap(response.getBitmap());
                            }
                        });
                    }
                }

                @Override
                public void onErrorResponse(VolleyError error) {

                }
            });

            String lang = CommonUtils.getLanguage(this);

            String title = assetInfo.getAssetTitle();
            String category = assetInfo.getCategoryAliasName();
            String fileSIze = CommonUtils.formatFileSize(this, assetInfo.getAssetFilesize());
            String description = assetInfo.getAssetDescription();


            if(assetInfo.getAssetNameMap() != null && assetInfo.getAssetNameMap().get(lang) != null){
                title = assetInfo.getAssetNameMap().get(lang);
            }

            if(assetInfo.getSubCategoryNameMap() != null && assetInfo.getSubCategoryNameMap().get(lang) != null){
                category = assetInfo.getSubCategoryNameMap().get(lang);
            }
            if(assetInfo.getAssetDescriptionMap() != null && assetInfo.getAssetDescriptionMap().get(lang) != null){
                description = assetInfo.getAssetDescriptionMap().get(lang);
            }

            final DownloadInfo downloadInfo = new DownloadInfo(assetInfo.getAssetIndex() + "", title
                    , assetInfo.getAssetThumbnailURL_S(), assetInfo.getAssetPackageDownloadURL()
                    , AssetManager.getInstance().getAssetDownloadPath(AssetDetailActivity.this, assetInfo.getAssetIndex()), assetInfo.getAssetFilesize());

            mAssetTitle.setText(title);
            mAssetCategoryName.setText(category);
            mAssetFileSize.setText(fileSIze);
            mDescription.setText(description);
            //Check asset status

            if(DownloadHelper.getInstance(this).isDownloading(downloadInfo.getDestinationPath())){
                DownloadHelper.getInstance(this).getDownloadingTask(downloadInfo.getId()).onProgress(new Task.OnProgressListener() {
                    @Override
                    public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                        updateProgress(progress, maxProgress);
                    }
                }).onFailure(new Task.OnFailListener() {
                    @Override
                    public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                        updateFailure();
                    }
                }).onCancel(new Task.OnTaskEventListener() {
                    @Override
                    public void onTaskEvent(Task t, Task.Event e) {
                        updateFailure();
                    }
                }).onResultAvailable(new ResultTask.OnResultAvailableListener<DownloadInfo>() {
                    @Override
                    public void onResultAvailable(ResultTask<DownloadInfo> task, Task.Event event, DownloadInfo result) {
                        installAsset(assetInfo, result);
                    }
                });
            }else{
                Task task = AssetManager.getInstance().getInstallAssetTask(assetInfo);
                if(task != null){
                    task.onComplete(onInstallCompletedEventListener)
                            .onProgress(onInstallProgressEventListener)
                            .onFailure(onInstallFailListener);
                }
                mActionButton.setVisibility(View.VISIBLE);
                if(AssetManager.getInstance().isInstalledAsset(assetInfo)){
                    mActionButton.setEnabled(false);
                    mActionButton.setText(R.string.installed);
                }
            }

            mActionButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    //Try download.
                    if(CommonUtils.isOnline(AssetDetailActivity.this)){
                        DownloadHelper.getInstance(AssetDetailActivity.this).download(downloadInfo).onProgress(new Task.OnProgressListener() {
                            @Override
                            public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                                updateProgress(progress, maxProgress);
                            }
                        }).onFailure(new Task.OnFailListener() {
                            @Override
                            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                                DialogUtil.createSimpleAlertDialog(AssetDetailActivity.this, R.string.theme_download_server_connection_error, new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialogInterface, int i) {
                                        dialogInterface.dismiss();
                                    }
                                });
                                updateFailure();
                            }
                        }).onCancel(new Task.OnTaskEventListener() {
                            @Override
                            public void onTaskEvent(Task t, Task.Event e) {
                                updateFailure();
                            }
                        }).onResultAvailable(new ResultTask.OnResultAvailableListener<DownloadInfo>() {
                            @Override
                            public void onResultAvailable(ResultTask<DownloadInfo> task, Task.Event event, DownloadInfo result) {
                                installAsset(assetInfo, result);
                            }
                        });
                    }else{
                        Toast.makeText(AssetDetailActivity.this, R.string.theme_download_server_connection_error, Toast.LENGTH_SHORT).show();

                    }
                }
            });

            mLayoutDownload.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if(DownloadHelper.getInstance(AssetDetailActivity.this).isDownloading(downloadInfo.getDestinationPath())){
                        DownloadHelper.getInstance(AssetDetailActivity.this).cancelDownload(downloadInfo);
                    }
                }
            });

            for(String image : assetInfo.getThumbnailPaths()){
                mImageListAdapter.getData().add(image);
                mImageListAdapter.notifyItemInserted(mImageListAdapter.getItemCount() - 1);
            }

            if(!TextUtils.isEmpty(assetInfo.getAssetVideoURL())){
                mAssetPlay.setVisibility(View.VISIBLE);
                mAssetPlay.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        if(mMediaPlayerWrapper != null){
                            if(mMediaPlayerWrapper.isPlaying()){
                                mMediaPlayerWrapper.pause();
                                mAssetPlay.setVisibility(View.VISIBLE);
                            }else{
                                mMediaPlayerWrapper.start();
                                sendMessage(MESSAGE_HIDE_PLAY_BUTTON, 500L);
                            }
                        }else{
                            mVideoLoading.setVisibility(View.VISIBLE);
                            mVideoView.setOnPreparedListener(AssetDetailActivity.this);
                            mVideoView.setOnCompletionListener(AssetDetailActivity.this);
                            mVideoView.setVideoURI(Uri.parse(assetInfo.getAssetVideoURL()));
                            mAssetPlay.setVisibility(View.GONE);
                        }
                        updatePlayButton();

                    }
                });
            }
        }
    }

    private void showLoadingView(){
        if(mLoadingView != null){
            mLoadingView.setVisibility(View.VISIBLE);
        }
    }

    private void hideLoadingView(){
        if(mLoadingView != null){
            mLoadingView.setVisibility(View.GONE);
        }
    }

    private void updateProgress(int current, int max){
        if(mLayoutDownload != null && mActionButton != null
                && mDownloadPercent != null && mDownloadProgress != null){
            mLayoutDownload.setVisibility(View.VISIBLE);
            mActionButton.setVisibility(View.GONE);
            mDownloadProgress.setMax(max);
            mDownloadProgress.setProgress(current);
            mDownloadPercent.setText(current + "%");
        }
    }

    private void updateFailure(){
        if(mLayoutDownload != null && mActionButton != null){
            mLayoutDownload.setVisibility(View.GONE);
            mActionButton.setEnabled(true);
            mActionButton.setText(R.string.download);
            mActionButton.setVisibility(View.VISIBLE);
        }
    }

    private void installAsset(final StoreAssetInfo assetInfo, DownloadInfo downloadInfo){
        if(assetInfo != null && downloadInfo != null && mActionButton != null && mLayoutDownload != null){
            mLayoutDownload.setVisibility(View.GONE);
            mActionButton.setVisibility(View.VISIBLE);
            mActionButton.setEnabled(false);
            mActionButton.setText(R.string.installing);
            AssetManager.getInstance().installAsset(assetInfo, downloadInfo)
                    .onComplete(onInstallCompletedEventListener)
                    .onProgress(onInstallProgressEventListener)
                    .onFailure(onInstallFailListener);
        }
    }

    public static Intent makeIntent(Context context, StoreAssetInfo assetInfo){
        Intent intent = new Intent(context, AssetDetailActivity.class);
        intent.putExtra(KEY_ASSET_INDEX, assetInfo.getAssetIndex());
        intent.putExtra(KEY_ASSET_IMAGE_URL, assetInfo.getAssetThumbnailURL_S());
        return intent;
    }

    public static Intent makeIntent(Context context, int index){
        Intent intent = new Intent(context, AssetDetailActivity.class);
        intent.putExtra(KEY_ASSET_INDEX, index);
        return intent;
    }


    @Override
    protected void onStart() {
        super.onStart();
        if(mMediaPlayerWrapper != null && mMediaPlayerWrapper.isPause()){
            mVideoView.start();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mVideoView != null){
            mVideoView.stopPlayback();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if(mVideoView != null){
            mVideoView.pause();
        }
    }

    @Override
    public void finish() {
        super.finish();
    }

    private void updatePlayButton(){
        if(mVideoView.isPlaying()){
            mAssetPlay.setImageResource(R.drawable.selector_pause_button);
        }else{
            mAssetPlay.setImageResource(R.drawable.selector_play_button);
        }
    }

    private void sendMessage(int what, long delay){
        handler.removeMessages(what);
        Message msg = Message.obtain();
        msg.what = what;
        handler.sendMessageDelayed(msg, delay);
    }

    @Override
    public void onPrepared(MediaPlayer mediaPlayer) {
        mMediaPlayerWrapper = new MediaPlayerWrapper(mediaPlayer, mVideoLoading);
        mediaPlayer.start();
        mVideoView.setVisibility(View.VISIBLE);
        mVideoLoading.setVisibility(View.GONE);
        mMainImage.setVisibility(View.GONE);
        updatePlayButton();
    }

    @Override
    public void onCompletion(MediaPlayer mediaPlayer) {
        updatePlayButton();
    }

    private class MediaPlayerWrapper implements MediaPlayer.OnBufferingUpdateListener{

        private static final String TAG = "MediaPlayerWrapper";

        MediaPlayer mediaPlayer;
        View loadingView;
        int bufferPercent;
        int playPercent;
        int currentPos;
        int duration;

        private boolean isPause;

        MediaPlayerWrapper(MediaPlayer mediaPlayer, View loadingView) {
            this.mediaPlayer = mediaPlayer;
            this.loadingView = loadingView;
            this.mediaPlayer.setOnBufferingUpdateListener(this);
        }

        void start() {
            try {
                mediaPlayer.start();
                isPause = false;
            } catch (Exception e) {
                if (LL.E) Log.e(TAG, e.getMessage(), e);
            }
        }

        void pause() {
            try {
                mediaPlayer.pause();
                isPause = true;
            } catch (Exception e) {
                if (LL.E) Log.e(TAG, e.getMessage(), e);
            }
        }

        boolean isPlaying() {
            try {
                return mediaPlayer.isPlaying();
            } catch (Exception e) {
                if (LL.E) Log.e(TAG, e.getMessage(), e);
            }
            return false;
        }

        void seekTo(int msec) {
            try {
                mediaPlayer.seekTo(msec);
            } catch (Exception e) {
                if (LL.E) Log.e(TAG, e.getMessage(), e);
            }
        }

        boolean isPause(){
            return isPause;
        }

        @Override
        public void onBufferingUpdate(MediaPlayer mediaPlayer, int i) {
            bufferPercent = i;
            currentPos = mediaPlayer.getCurrentPosition();
            duration = mediaPlayer.getDuration();

            try{
                playPercent = (int) (((double)currentPos / (double)duration) * 100);
            }catch (Exception e){
                if(LL.D) Log.d(TAG, "onBufferingUpdate: divide error", e);
            }
            if(LL.D) Log.d(TAG, "onBufferingUpdate: currentPos = " + currentPos);
            if(LL.D) Log.d(TAG, "onBufferingUpdate: duration = " + duration);
            if(LL.D) Log.d(TAG, "onBufferingUpdate: bufferPercent  [" + bufferPercent + "], playPercent [" + playPercent + "]");
            if(loadingView != null){
                loadingView.setVisibility(bufferPercent > playPercent ? View.GONE : View.VISIBLE);
            }
        }
    }

    private Handler handler = new Handler(){

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if(msg != null){
                switch (msg.what){
                    case MESSAGE_HIDE_PLAY_BUTTON : {
                        if(mAssetPlay != null && mAssetPlay.isShown()){
                            if(mMediaPlayerWrapper != null){
                                if(mMediaPlayerWrapper != null && mMediaPlayerWrapper.isPlaying()){
                                    AlphaAnimation anim = new AlphaAnimation(1f, 0f);
                                    anim.setDuration(500L);
                                    mAssetPlay.setAnimation(anim);
                                    mAssetPlay.setVisibility(View.GONE);
                                }
                            }
                        }
                    }
                }
            }

        }
    };

    private Task.OnTaskEventListener onInstallCompletedEventListener = new Task.OnTaskEventListener() {
        @Override
        public void onTaskEvent(Task t, Task.Event e) {
            if(LL.D) Log.d(TAG, "onTaskEvent: onInstallCompletedEvent");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(mActionButton != null){
                        mActionButton.setEnabled(false);
                        mActionButton.setText(R.string.installed);
                        if(!mMultiSelectFlag){
                            finish();
                        }
                    }
                }
            });
        }
    };

    private Task.OnProgressListener onInstallProgressEventListener = new Task.OnProgressListener() {
        @Override
        public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
            if(LL.D) Log.d(TAG, "onProgress: onInstallProgress");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(mActionButton != null){
                        mActionButton.setEnabled(false);
                        mActionButton.setText(R.string.installing);
                    }
//                            updateProgress(progress, maxProgress);
                }
            });
        }
    };

    private Task.OnFailListener onInstallFailListener = new Task.OnFailListener() {
        @Override
        public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
            if(LL.D) Log.d(TAG, "onFail: onInstallFail");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    updateFailure();
                }
            });
        }
    };
}
