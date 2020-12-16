package com.nexstreaming.app.vasset.global.asset.category;

import com.nexstreaming.app.vasset.global.ui.adapter.BannerPagerAdapter;

/**
 * Created by ojin.kwon on 2016-11-23.
 */
public class CategoryBannerSet implements CategoryItem {

    private BannerPagerAdapter adapter;

    public CategoryBannerSet(BannerPagerAdapter adapter) {
        this.adapter = adapter;
    }

    @Override
    public int getIndex() {
        return 0;
    }

    @Override
    public String getTitle() {
        return null;
    }

    @Override
    public String getSubTitle() {
        return null;
    }

    @Override
    public String getImageUrl() {
        return null;
    }

    @Override
    public String getTargetUrl() {
        return null;
    }

    public BannerPagerAdapter getAdapter(){
        return adapter;
    }
}
