package com.nexstreaming.app.vasset.global.ui.adapter;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.nexstreaming.app.assetlibrary.adapter.BaseAdapterImpl;
import com.nexstreaming.app.assetlibrary.view.NetworkImageView;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.assetlibrary.model.NexInstalledAssetItem;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.utils.CommonUtils;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class SettingAssetListAdapter extends RecyclerView.Adapter<SettingAssetListAdapter.SettingAssetHolder>  implements BaseAdapterImpl<NexInstalledAssetItem> {

    private static final String TAG = "SettingAssetListAdapter";

    public interface OnClickUninstallButtonListener{
        void onClick(int pos, NexInstalledAssetItem item);
    }

    private List<NexInstalledAssetItem> mAssetInfos;
    private OnClickUninstallButtonListener mOnClickUninstallButtonListener;

    public SettingAssetListAdapter(List<NexInstalledAssetItem> categoryInfos, OnClickUninstallButtonListener ll){
        mAssetInfos = categoryInfos;
        mOnClickUninstallButtonListener = ll;
    }

    @Override
    public SettingAssetHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view  = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_list_item, null);
        return new SettingAssetHolder(view);
    }

    @Override
    public void onBindViewHolder(SettingAssetHolder holder, final int position) {
        final NexInstalledAssetItem item = getItem(position);
        if(holder != null && item != null){
            final String lang = CommonUtils.getLanguage(holder.imageView.getContext());
            holder.imageView.setImageUrl(item.thumbUrl, KMVolley.getInstance(holder.imageView.getContext()).getImageLoader());
            String title = item.assetName;
            String description = item.categoryName;
            holder.title.setText(title);
            holder.description.setText(description);

            holder.button.setVisibility(View.VISIBLE);
            holder.button.setText(R.string.delete);
            holder.downloadingLayout.setVisibility(View.GONE);
            holder.button.setBackgroundResource(R.drawable.selector_common_negative_button);
            holder.button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    mOnClickUninstallButtonListener.onClick(position, item);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return mAssetInfos.size();
    }

    @Override
    public NexInstalledAssetItem getItem(int pos) {
        return mAssetInfos.get(pos);
    }

    @Override
    public List<NexInstalledAssetItem> getData() {
        return mAssetInfos;
    }

    public static final class SettingAssetHolder extends RecyclerView.ViewHolder{

        public NetworkImageView imageView;
        public View layout;
        public TextView title;
        public TextView description;
        public Button button;
        public ViewGroup downloadingLayout;
        public TextView downloadPercent;
        public ProgressBar downloadProgress;

        public SettingAssetHolder(View itemView) {
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
    }
}
