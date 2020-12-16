package com.nexstreaming.app.vasset.global.asset.category;

/**
 * Created by ojin.kwon on 2016-11-23.
 */

public class CategoryTitleItem implements CategoryItem {

    private String title;

    public CategoryTitleItem(String title) {
        this.title = title;
    }

    @Override
    public int getIndex() {
        return 0;
    }

    @Override
    public String getTitle() {
        return title;
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
}
