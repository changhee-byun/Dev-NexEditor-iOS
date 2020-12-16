package com.nexstreaming.app.common.nexasset.assetpackage.db;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetCategory;
import com.nexstreaming.app.common.norm.NormTable;

public class CategoryRecord extends NormTable implements AssetCategory {

    public long _id;

    @NormTable.Unique
    @NotNull
    public long categoryId;
    public String categoryName;
    public String categoryIconURL;
    public String categoryURL;

    public CategoryRecord(){

    }

    @Override
    public long getCategoryId() {
        return categoryId;
    }

    @Override
    public String getCategoryIconURL() {
        return categoryIconURL;
    }

    @Override
    public String getCategoryURL() {
        return categoryURL;
    }

    @Override
    public String getCategoryAlias() {
        return categoryName;
    }
}
