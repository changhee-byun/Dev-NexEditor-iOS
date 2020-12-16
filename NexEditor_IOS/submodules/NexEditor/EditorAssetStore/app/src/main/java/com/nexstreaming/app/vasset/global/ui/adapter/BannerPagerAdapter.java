package com.nexstreaming.app.vasset.global.ui.adapter;

import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.nexstreaming.app.assetlibrary.adapter.BasePagerAdapter;
import com.nexstreaming.app.assetlibrary.view.NetworkImageView;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.vasset.global.ui.AssetDetailActivity;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-23.
 */

public class BannerPagerAdapter extends BasePagerAdapter {

    private static final String TAG = "BannerPagerAdapter";

    private List<StoreAssetInfo> mBannerList;

    public BannerPagerAdapter(List<StoreAssetInfo> assetInfos){
        mBannerList = assetInfos;
    }

    @Override
    public Object getItem(int position) {
        return mBannerList.get(position);
    }

    @Override
    public int getRealCount() {
        return mBannerList.size();
    }

    @Override
    public int getVirtualPosition(int position) {
        return mBannerList.size();
    }

    @Override
    public int getCount() {
        return mBannerList.size();
    }

    @Override
    public boolean isViewFromObject(View view, Object object) {
        return view == object;
    }

    @Override
    public Object instantiateItem(ViewGroup container, int position) {
        NetworkImageView image = new NetworkImageView(container.getContext());
        ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        image.setLayoutParams(params);
        image.setScaleType(ImageView.ScaleType.FIT_XY);

        final StoreAssetInfo assetInfo = mBannerList.get(position);
        if(assetInfo != null){
            String imageUrl = assetInfo.getAssetThumbnailURL_L();
            image.setImageUrl(imageUrl, KMVolley.getInstance(container.getContext()).getImageLoader());
            image.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    String description = assetInfo.getAssetDescription();
                    if(LL.D) Log.d(TAG, "banner asset info  description [" + description + "], assetIndex [" + assetInfo.getAssetIndex() + "]");
                    if(description != null && (description.startsWith("http://") || description.startsWith("https://") )){
                        Intent intent = new Intent(Intent.ACTION_VIEW);
                        intent.setData(Uri.parse(description));
                    }else{
                        if(assetInfo.getAssetIndex() > 0){
                            Intent intent = AssetDetailActivity.makeIntent(view.getContext(), assetInfo);
                            view.getContext().startActivity(intent);
                        }
                    }
                }
            });
        }else{
            image.setDefaultImageResId(R.drawable.default_banner_image);
        }
        container.addView(image);
        return image;
    }

    @Override
    public void destroyItem(ViewGroup container, int position, Object object) {
        container.removeView((ImageView) object);
    }

    public List<StoreAssetInfo> getData(){
        return mBannerList;
    }
}
