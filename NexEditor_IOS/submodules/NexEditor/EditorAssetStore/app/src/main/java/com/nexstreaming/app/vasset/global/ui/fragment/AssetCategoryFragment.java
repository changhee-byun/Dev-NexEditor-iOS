package com.nexstreaming.app.vasset.global.ui.fragment;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.StateListDrawable;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.util.StateSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.android.volley.VolleyError;
import com.android.volley.toolbox.ImageLoader;
import com.nexstreaming.app.assetlibrary.view.PagerTabContent;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.vasset.global.asset.AssetManager;
import com.nexstreaming.app.vasset.global.download.DownloadInfo;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreCategoryInfo;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreFeaturedAssetInfo;
import com.nexstreaming.app.vasset.global.asset.category.CategoryAssetItem;
import com.nexstreaming.app.vasset.global.asset.category.CategoryBannerSet;
import com.nexstreaming.app.vasset.global.asset.category.CategoryItem;
import com.nexstreaming.app.vasset.global.asset.category.CategoryTitleItem;
import com.nexstreaming.app.vasset.global.ui.MainActivity;
import com.nexstreaming.app.vasset.global.ui.adapter.AssetListAdapter;
import com.nexstreaming.app.vasset.global.ui.adapter.BannerPagerAdapter;
import com.nexstreaming.app.assetlibrary.utils.CommonUtils;

import java.util.ArrayList;
import java.util.List;

import jp.wasabeef.recyclerview.animators.FadeInAnimator;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class AssetCategoryFragment extends BaseFragment implements PagerTabContent {

    private static final String TAG = "AssetCategoryFragment";

    private static final int FEATURED_HOT_INDEX = 1;
    private static final int FEATURED_NEW_INDEX = 2;
    private static final int FEATURED_BANNER_INDEX = 3;

    private static final String KEY_INDEX = "index";
    private static final String KEY_TITLE = "title";
    private static final String KEY_IMAGE_URL = "imageUrl";
    private static final String KEY_IMAGE_URL_ON = "imageUrlOn";

    private SwipeRefreshLayout mSwipeRefreshLayout;
    private RecyclerView mRecyclerView;

    private AssetListAdapter mAdapter;

    private ProgressBar mLoadingView;

    private int mCategoryIndex;
    private String mCategoryTitle;
    private String mCategoryImageUrl;

    private String mCategoryImageUrlOn;

    private Drawable mIconDrawable;

    private BannerPagerAdapter mBannerPagerAdapter;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initData();
    }

    @Override
    public void onStart() {
        super.onStart();
        if(mAdapter != null){
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_category, container , false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        initView(view);
        startLoadingCategoryDatas();
        loadCategoryIcon();
    }

    private void initData(){
        mCategoryIndex = getArguments().getInt(KEY_INDEX);
        mCategoryTitle = getArguments().getString(KEY_TITLE);
        mCategoryImageUrl = getArguments().getString(KEY_IMAGE_URL);
        mCategoryImageUrlOn = getArguments().getString(KEY_IMAGE_URL_ON);
        mAdapter = new AssetListAdapter(getActivity(), new ArrayList<CategoryItem>(), new AssetListAdapter.OnDownloadCompletedListener() {
            @Override
            public void onCompleted(final AssetListAdapter.CategoryAssetItemHolder holder, int position, CategoryAssetItem item, DownloadInfo info) {
                holder.showDownloadButton();
                holder.button.setText(R.string.installing);
                holder.button.setEnabled(false);

                AssetManager.getInstance().installAsset(item.getAssetInfo(), info).onComplete(new Task.OnTaskEventListener() {
                    @Override
                    public void onTaskEvent(Task t, Task.Event e) {
                        if(LL.D) Log.d(TAG, "onTaskEvent() called with: t = [" + t + "], e = [" + e + "]");
                        if(getActivity() != null){
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    holder.showDownloadButton();
                                    holder.button.setText(R.string.installed);
                                    holder.button.setEnabled(false);
                                }
                            });
                        }
                    }
                }).onFailure(new Task.OnFailListener() {
                    @Override
                    public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                        if(getActivity() != null){
                            getActivity().runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    holder.showDownloadButton();
                                    holder.button.setText(R.string.download);
                                    holder.button.setEnabled(true);
                                }
                            });
                        }
                    }
                }).onProgress(new Task.OnProgressListener() {
                    @Override
                    public void onProgress(Task t, Task.Event e, final int progress, final int maxProgress) {
                        if(LL.D) Log.d(TAG, "onProgress() called with: t = [" + t + "], e = [" + e + "], progress = [" + progress + "], maxProgress = [" + maxProgress + "]");
                        if(getActivity() != null){
                            if(getActivity() != null){
                                getActivity().runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        holder.showDownloadButton();
                                        holder.button.setText(R.string.installing);
                                        holder.button.setEnabled(false);
//                                        holder.showDownloadProgressLayout();
//                                        holder.downloadProgress.setProgress(progress);
//                                        holder.downloadProgress.setMax(maxProgress);
                                    }
                                });
                            }
                        }
                    }
                });
            }
        });

        mIconDrawable = null;
    }

    public void initView(View view){
        mSwipeRefreshLayout = (SwipeRefreshLayout) view.findViewById(R.id.refresh_fragment_category);
        mRecyclerView = (RecyclerView) view.findViewById(R.id.rv_fragment_category);

        mLoadingView = (ProgressBar) view.findViewById(R.id.pb_fragment_category);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(getActivity(), LinearLayoutManager.VERTICAL, false));
        mRecyclerView.setItemAnimator(new FadeInAnimator());
        mRecyclerView.setAdapter(mAdapter);

        mSwipeRefreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                AssetStoreSession.getInstance(getActivity()).getFeaturedAssets(FEATURED_BANNER_INDEX, mCategoryIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
                    @Override
                    public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                        setBannerAsset(result.getFeaturedAssetList());
                        loadCategoryFeaturedHotAssets();
                    }
                }).onFailure(new Task.OnFailListener() {
                    @Override
                    public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                        setBannerAsset(null);
                        loadCategoryFeaturedHotAssets();
                    }
                });
            }
        });
    }

    private void startLoadingCategoryDatas(){
        if(getActivity() != null){
            showLoading();
            AssetStoreSession.getInstance(getActivity()).getFeaturedAssets(FEATURED_BANNER_INDEX, mCategoryIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
                @Override
                public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                    setBannerAsset(result.getFeaturedAssetList());
                    loadCategoryFeaturedHotAssets();
                }
            }).onFailure(new Task.OnFailListener() {
                @Override
                public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                    setBannerAsset(null);
                    loadCategoryFeaturedHotAssets();
                }
            });
        }
    }

    private void setBannerAsset(List<StoreAssetInfo> bannerAsset){
        if(mAdapter != null && getActivity() != null){
            mAdapter.getData().clear();
            mAdapter.notifyDataSetChanged();
            if(bannerAsset == null){
                bannerAsset = new ArrayList<>();
                bannerAsset.add(null);
            }
            if(mBannerPagerAdapter == null){
                mBannerPagerAdapter = new BannerPagerAdapter(bannerAsset);
            }else{
                mBannerPagerAdapter.getData().clear();
                mBannerPagerAdapter.getData().addAll(bannerAsset);
                mBannerPagerAdapter.notifyDataSetChanged();
            }
            addItemToList(new CategoryBannerSet(mBannerPagerAdapter));
        }
    }

    private void loadCategoryFeaturedHotAssets(){
        if(getActivity() != null){
            AssetStoreSession.getInstance(getActivity()).getFeaturedAssets(FEATURED_HOT_INDEX, mCategoryIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
                @Override
                public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                    if(mAdapter != null && result != null && getActivity() != null){
                        String lang = CommonUtils.getLanguage(getActivity());
                        addItemToList(new CategoryTitleItem(getString(R.string.Hot)));
                        for(StoreAssetInfo info : result.getFeaturedAssetList()){
                            addItemToList(new CategoryAssetItem(info, lang));
                        }
                    }
                    loadCategoryFeaturedNewAssets();
                }
            }).onFailure(new Task.OnFailListener() {
                @Override
                public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                    loadCategoryFeaturedNewAssets();
                }
            });
        }
    }

    private void loadCategoryFeaturedNewAssets(){
        if(getActivity() != null){
            AssetStoreSession.getInstance(getActivity()).getFeaturedAssets(FEATURED_NEW_INDEX, mCategoryIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
                @Override
                public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                    if(mAdapter != null && result != null && getActivity() != null){
                        String lang = CommonUtils.getLanguage(getActivity());
                        addItemToList(new CategoryTitleItem(getString(R.string.New)));
                        for(StoreAssetInfo info : result.getFeaturedAssetList()){
                            addItemToList(new CategoryAssetItem(info, lang));
                        }
                    }
                    loadCategoryAssets();
                }
            }).onFailure(new Task.OnFailListener() {
                @Override
                public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                    loadCategoryAssets();
                }
            });
        }
    }

    private void addItemToList(CategoryItem item){
        if(item != null && mAdapter != null){
            mAdapter.getData().add(item);
            mAdapter.notifyItemInserted(mAdapter.getItemCount() - 1);
        }
    }

    private void loadCategoryAssets(){
        if(getActivity() != null){
            AssetStoreSession.getInstance(getActivity()).getAssetsInCategory(mCategoryIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<List<StoreAssetInfo>>() {
                @Override
                public void onResultAvailable(ResultTask<List<StoreAssetInfo>> task, Task.Event event, List<StoreAssetInfo> result) {
                    hideLoading();
                    if(result != null && getActivity() != null){
                        String lang = CommonUtils.getLanguage(getActivity());
                        addItemToList(new CategoryTitleItem(mCategoryTitle));
                        for(StoreAssetInfo info : result){
                            addItemToList(new CategoryAssetItem(info, lang));
                        }
                    }
                }
            }).onFailure(new Task.OnFailListener() {
                @Override
                public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                    hideLoading();
                }
            });
        }
    }

    private void loadCategoryIcon(){
        if(getActivity() != null){
            KMVolley.getInstance(getActivity()).getImageLoader().get(mCategoryImageUrl, new ImageLoader.ImageListener() {
                @Override
                public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {
                    if(response != null && response.getBitmap() != null){
                        final Bitmap normal = response.getBitmap();
                        if(getActivity() != null){
                            KMVolley.getInstance(getActivity()).getImageLoader().get(mCategoryImageUrlOn, new ImageLoader.ImageListener() {
                                @Override
                                public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {

                                    StateListDrawable drawable = new StateListDrawable();

                                    if(response != null && response.getBitmap() != null && !response.getBitmap().isRecycled()){
                                        drawable.addState(new int[]{android.R.attr.state_pressed}, new BitmapDrawable(response.getBitmap()));
                                        drawable.addState(new int[]{android.R.attr.state_selected}, new BitmapDrawable(response.getBitmap()));
                                    }
                                    drawable.addState(StateSet.NOTHING, new BitmapDrawable(normal));
                                    mIconDrawable = drawable;
                                    if(getActivity() instanceof MainActivity){
                                        ((MainActivity) getActivity()).updateTab();
                                    }
                                }

                                @Override
                                public void onErrorResponse(VolleyError error) {

                                }
                            });
                        }
                    }
                }

                @Override
                public void onErrorResponse(VolleyError error) {

                }
            });
        }
    }

    public void showLoading(){
        if(mLoadingView != null){
            mLoadingView.setVisibility(View.VISIBLE);
        }
    }

    public void hideLoading(){
        if(mLoadingView != null) {
            mLoadingView.setVisibility(View.GONE);
        }
        if(mSwipeRefreshLayout != null){
            mSwipeRefreshLayout.setRefreshing(false);
        }
    }

    public void updateAssetList(){
        if(mAdapter != null){
            mAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public String getTitle(Context context) {
        return getArguments().getString(KEY_TITLE);
    }

    @Override
    public int getImageResource() {
        return R.drawable.ic_launcher;
    }

    public static Bundle makeArguments(Context context, StoreCategoryInfo info){
        Bundle b = new Bundle();
        b.putInt(KEY_INDEX, info.getCategoryIdx());
        b.putString(KEY_IMAGE_URL, info.getIconURL());
        b.putString(KEY_IMAGE_URL_ON, info.getSelectedIconURL());
        String title = info.getCategoryAliasName();
        String lang = CommonUtils.getLanguage(context);
        if(info.getCategoryName() != null && info.getCategoryName().get(lang) != null){
            title = info.getCategoryName().get(lang);
        }
        b.putString(KEY_TITLE, title);
        return b;
    }

    @Override
    public String getTabTitle(Context context) {
        return context.getString(R.string.download);
    }

    @Override
    public Drawable getTabDrawable(Context context) {
        return mIconDrawable;
    }

    @Override
    public boolean isShowingIcon() {
        return true;
    }
}
