package com.nexstreaming.app.assetlibrary.network.assetstore;


import java.util.List;
import java.util.Map;

public class FeaturedCategoryInfo implements StoreCategoryInfo {


    @Override
    public String getIconURL() {
        return null;
    }

    @Override
    public String getSelectedIconURL() {
        return null;
    }

    @Override
    public int getCategoryIdx() {
        return -1;
    }

    @Override
    public Map<String, String> getCategoryName() {
        return null;
    }

    @Override
    public String getCategoryAliasName() {
        return "Featured";
    }

    @Override
    public List<StoreSubcategoryInfo> getSubCategories() {
        return null;
    }
}
