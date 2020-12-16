package com.nexstreaming.app.vasset.global.ui.adapter;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.android.volley.toolbox.NetworkImageView;
import com.nexstreaming.app.assetlibrary.adapter.BaseAdapterImpl;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.assetlibrary.network.KMVolley;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-21.
 */

public class AssetDetailImageListAdapter extends RecyclerView.Adapter<AssetDetailImageListAdapter.ImageViewHolder>  implements BaseAdapterImpl<String> {

    private static final String TAG = "DetailImageListAdapter";

    private List<String> mData;

    public AssetDetailImageListAdapter(List<String> imageUrlList){
        mData = imageUrlList;
    }

    @Override
    public ImageViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_asset_image_list, null);
        return new ImageViewHolder(view);
    }

    @Override
    public void onBindViewHolder(ImageViewHolder holder, int position) {
        String imageUrl = getItem(position);
        if(imageUrl != null){
            int width = holder.layout.getWidth();
            float height = (float) (width / 1.77);
            holder.imageView.setImageUrl(imageUrl, KMVolley.getInstance(holder.imageView.getContext()).getImageLoader());
            holder.layout.getLayoutParams().height = (int) height;
        }
    }

    @Override
    public String getItem(int pos) {
        return mData.get(pos);
    }

    @Override
    public int getItemCount() {
        return mData.size();
    }

    @Override
    public List<String> getData() {
        return mData;
    }

    public static final class ImageViewHolder extends RecyclerView.ViewHolder{

        public ViewGroup layout;
        public NetworkImageView imageView;

        public ImageViewHolder(View itemView) {
            super(itemView);
            layout = (ViewGroup) itemView.findViewById(R.id.layout_item_asset_image_list);
            imageView = (NetworkImageView) itemView.findViewById(R.id.iv_item_asset_image_list);
        }
    }
}
