package com.nexstreaming.app.vasset.global.ui.adapter;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.app.assetlibrary.adapter.BaseAdapterImpl;
import com.nexstreaming.app.assetlibrary.view.NetworkImageView;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.asset.AssetManager;
import com.nexstreaming.app.vasset.global.download.DownloadHelper;
import com.nexstreaming.app.vasset.global.download.DownloadInfo;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.vasset.global.asset.category.CategoryAssetItem;
import com.nexstreaming.app.vasset.global.asset.category.CategoryBannerSet;
import com.nexstreaming.app.vasset.global.asset.category.CategoryItem;
import com.nexstreaming.app.vasset.global.asset.category.CategoryTitleItem;
import com.nexstreaming.app.vasset.global.ui.AssetDetailActivity;
import com.nexstreaming.app.assetlibrary.utils.CommonUtils;
import com.nexstreaming.app.assetlibrary.utils.DialogUtil;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class AssetListAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder>  implements BaseAdapterImpl<CategoryItem> {

    private static final String TAG = "AssetListAdapter";

    private static final int TYPE_TEXT = 0;
    private static final int TYPE_ITEM = 1;
    private static final int TYPE_BANNER = 2;

    public interface OnDownloadCompletedListener{
        void onCompleted(CategoryAssetItemHolder holder, int position, CategoryAssetItem item, DownloadInfo info);
    }

    private List<CategoryItem> mCateogyItems;

    private DownloadHelper mDownloadHelper;
    private AssetManager mAssetManager;

    private OnDownloadCompletedListener mOnDownloadCompletedListener;

    public AssetListAdapter(Activity context, List<CategoryItem> categoryInfos, OnDownloadCompletedListener ll){
        mCateogyItems = categoryInfos;
        mDownloadHelper = DownloadHelper.getInstance(context);
        mAssetManager = AssetManager.getInstance();
        mOnDownloadCompletedListener = ll;
    }

    @Override
    public int getItemViewType(int position) {
        CategoryItem info = getItem(position);
        int type = TYPE_ITEM;
        if(info instanceof CategoryTitleItem){
            type = TYPE_TEXT;
        }else if(info instanceof CategoryAssetItem){
            type = TYPE_ITEM;
        }else if(info instanceof CategoryBannerSet){
            type = TYPE_BANNER;
        }
        return type;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        if(TYPE_ITEM == viewType){
            View view  = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_list_item, null);
            return new CategoryAssetItemHolder(view);
        }else if(TYPE_TEXT == viewType){
            View view  = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_list_title, null);
            return new CategoryTitleHolder(view);
        }else{
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_list_banner, null);
            return new CategoryBannerHolder(view);
        }
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {

        if(holder != null){
            CategoryItem item = getItem(position);
            if(holder.getItemViewType() == TYPE_TEXT){
                onBindTitleItemHolder((CategoryTitleHolder) holder, position, (CategoryTitleItem) item);
            }else if(holder.getItemViewType() == TYPE_ITEM){
                onBindListItemHolder((CategoryAssetItemHolder) holder, position, (CategoryAssetItem) item);
            }else if(holder.getItemViewType() == TYPE_BANNER){
                onBindBannerItemHolder((CategoryBannerHolder)holder, position, (CategoryBannerSet) item);
            }
        }
    }

    private void onBindListItemHolder(final CategoryAssetItemHolder holder, final int position, final CategoryAssetItem item){
        if(holder != null && item != null){
            holder.imageView.setImageUrl(item.getImageUrl(), KMVolley.getInstance(holder.imageView.getContext()).getImageLoader());
            final String lang = CommonUtils.getLanguage(holder.button.getContext());
            holder.title.setText(item.getTitle());
            holder.description.setText(item.getSubTitle());
            holder.layout.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent intent = AssetDetailActivity.makeIntent(view.getContext(), item.getAssetInfo());
                    view.getContext().startActivity(intent);
                }
            });

            String assetIndex = String.valueOf(item.getIndex());
            if(mAssetManager.isInstalledAsset(item.getAssetInfo())){
                holder.button.setEnabled(false);
                holder.button.setText(R.string.installed);
                holder.button.setVisibility(View.VISIBLE);
                holder.downloadingLayout.setVisibility(View.GONE);
            }else{
                holder.button.setEnabled(true);
                holder.button.setText(R.string.download);
                holder.button.setVisibility(View.VISIBLE);
                holder.downloadingLayout.setVisibility(View.GONE);
                if(mDownloadHelper.isDownloading(mAssetManager.getAssetDownloadPath(holder.button.getContext(), item.getIndex()))) {
                    mDownloadHelper.getDownloadingTask(assetIndex).onResultAvailable(new ResultTask.OnResultAvailableListener<DownloadInfo>() {
                        @Override
                        public void onResultAvailable(ResultTask<DownloadInfo> task, Task.Event event, DownloadInfo result) {
                            mOnDownloadCompletedListener.onCompleted(holder, position, item, result);
                        }
                    }).onProgress(new Task.OnProgressListener() {
                        @Override
                        public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                            updateProgress(holder, progress, maxProgress);
                        }
                    }).onFailure(new Task.OnFailListener() {
                        @Override
                        public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                            updateFailure(holder);
                        }
                    }).onCancel(new Task.OnTaskEventListener() {
                        @Override
                        public void onTaskEvent(Task t, Task.Event e) {
                            cancelDownload(holder, item.getAssetInfo());
                        }
                    });
                }else{
                    if(mAssetManager.getInstallAssetTask(item.getAssetInfo()) != null){
                        mOnDownloadCompletedListener.onCompleted(holder, position, item, null);
                    }
                    holder.button.setEnabled(true);
                    holder.button.setText(R.string.download);
                    holder.button.setVisibility(View.VISIBLE);
                    holder.downloadingLayout.setVisibility(View.GONE);
                }

                holder.button.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        //Try download.
                        if(CommonUtils.isOnline(view.getContext())){
                            if(!mAssetManager.isInstalledAsset(item.getAssetInfo())){
                                final DownloadInfo downloadInfo = new DownloadInfo(item.getIndex() + "", item.getTitle()
                                        , item.getImageUrl(), item.getTargetUrl()
                                        , mAssetManager.getAssetDownloadPath(view.getContext(), item.getIndex()), item.getAssetInfo().getAssetFilesize());

                                mDownloadHelper.download(downloadInfo).onProgress(new Task.OnProgressListener() {

                                    boolean notification = false;

                                    @Override
                                    public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                                        if(!notification){
                                            if(LL.D)
                                                Log.d(TAG, "try notifyItemChanged for downloading");

                                            for(int i = 0 ; i < getItemCount() ; i ++){
                                                if(getItem(i).equals(item.getAssetInfo())){
                                                    if(LL.D)
                                                        Log.d(TAG, "notifyItemChanged : index " + i);
                                                    notifyItemChanged(i);
                                                }
                                            }
                                            notification = true;
                                        }
                                    }
                                });
                            }else{
                                notifyItemChanged(position);
                            }
                        }else{
                            Toast.makeText(holder.button.getContext(), R.string.theme_download_server_connection_error, Toast.LENGTH_SHORT).show();
                        }

                    }
                });

                holder.downloadingLayout.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        // Cancel downloading
                        if(mDownloadHelper.isDownloading(mAssetManager.getAssetDownloadPath(view.getContext(), item.getIndex()))) {
                            DownloadInfo downloadInfo = new DownloadInfo(item.getIndex() + "", item.getTitle()
                                    , item.getImageUrl(), item.getTargetUrl()
                                    , mAssetManager.getAssetDownloadPath(view.getContext(), item.getIndex()), item.getAssetInfo().getAssetFilesize());
                            mDownloadHelper.cancelDownload(downloadInfo);
                        }
                    }
                });
            }
        }
    }

    private void onBindTitleItemHolder(CategoryTitleHolder holder, int position, CategoryTitleItem item){
        if(holder != null && item != null){
            holder.title.setText(item.getTitle());
        }
    }

    private void onBindBannerItemHolder(CategoryBannerHolder holder, int position, CategoryBannerSet item){
        if(holder != null && item != null){
            if(item.getAdapter() != null){
                holder.viewPager.setAdapter(item.getAdapter());
                if(item.getAdapter().getData().size() > 0){
                    holder.viewPager.setVisibility(View.VISIBLE);
                    if(item.getAdapter().getData().get(0) == null){
                        holder.tabLayout.setVisibility(View.GONE);
                    }else{
                        holder.tabLayout.setVisibility(View.VISIBLE);
                    }
                }else{
                    holder.viewPager.setVisibility(View.GONE);
                }
            }
        }
    }

    private void updateProgress(CategoryAssetItemHolder holder, int current, int max){
        holder.downloadingLayout.setVisibility(View.VISIBLE);
        holder.button.setVisibility(View.GONE);
        holder.downloadProgress.setMax(max);
        holder.downloadProgress.setProgress(current);
        holder.downloadPercent.setText(current + "%");
    }

    private void updateFailure(CategoryAssetItemHolder holder){
        holder.button.setEnabled(true);
        holder.button.setVisibility(View.VISIBLE);
        holder.downloadingLayout.setVisibility(View.GONE);
        DialogUtil.createSimpleAlertDialog(holder.button.getContext(), R.string.theme_download_server_connection_error, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                dialogInterface.dismiss();
            }
        });
    }

    private void cancelDownload(CategoryAssetItemHolder holder, final StoreAssetInfo assetInfo){
        for(int i = 0 ; i < getItemCount() ; i ++){
            if(getItem(i).equals(assetInfo)){
                notifyItemChanged(i);
            }
        }
        holder.button.setEnabled(true);
        holder.button.setVisibility(View.VISIBLE);
        holder.downloadingLayout.setVisibility(View.GONE);
    }

    @Override
    public int getItemCount() {
        return mCateogyItems.size();
    }

    @Override
    public CategoryItem getItem(int pos) {
        return mCateogyItems.get(pos);
    }

    @Override
    public List<CategoryItem> getData() {
        return mCateogyItems;
    }

    public static final class CategoryAssetItemHolder extends RecyclerView.ViewHolder{

        public NetworkImageView imageView;
        public View layout;
        public TextView title;
        public TextView description;
        public Button button;
        public ViewGroup downloadingLayout;
        public TextView downloadPercent;
        public ProgressBar downloadProgress;

        public CategoryAssetItemHolder(View itemView) {
            super(itemView);
            layout = itemView.findViewById(R.id.layout_item_asset_list);
            imageView = (NetworkImageView) itemView.findViewById(R.id.iv_item_asset_list_item);
            title = (TextView) itemView.findViewById(R.id.tv_item_asset_list_item_title);
            description = (TextView) itemView.findViewById(R.id.tv_item_asset_list_item_desc);
            button = (Button) itemView.findViewById(R.id.btn_item_asset_list_item);

            downloadingLayout = (ViewGroup) itemView.findViewById(R.id.layout_item_asset_list_downloading);
            downloadPercent = (TextView) itemView.findViewById(R.id.tv_item_asset_list_item_download_percent);
            downloadProgress = (ProgressBar) itemView.findViewById(R.id.pb_item_asset_list_item_download);
        }

        public void showDownloadProgressLayout(){
            button.setVisibility(View.GONE);
            downloadingLayout.setVisibility(View.VISIBLE);
        }

        public void showDownloadButton(){
            button.setVisibility(View.VISIBLE);
            downloadingLayout.setVisibility(View.GONE);
        }
    }

    public static final class CategoryTitleHolder extends RecyclerView.ViewHolder{
        public TextView title;

        public CategoryTitleHolder(View itemView) {
            super(itemView);
            title = (TextView) itemView.findViewById(R.id.tv_item_asset_list_item_title);
        }
    }

    public static final class CategoryBannerHolder extends RecyclerView.ViewHolder{

        public ViewPager viewPager;
        public TabLayout tabLayout;

        public CategoryBannerHolder(View itemView) {
            super(itemView);
            viewPager = (ViewPager) itemView.findViewById(R.id.vp_item_asset_list_banner);
            tabLayout = (TabLayout) itemView.findViewById(R.id.tab_item_asset_list_banner);
            tabLayout.setupWithViewPager(viewPager, true);
            viewPager.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                    int width = viewPager.getResources().getDisplayMetrics().widthPixels;
                    float height = (float) (width / 1.77);
                    viewPager.getLayoutParams().height = (int) height;
                    viewPager.requestLayout();
                }
            });
        }
    }
}
