package com.nexstreaming.app.common.nexasset.assetpackage.db;


import com.nexstreaming.app.common.nexasset.assetpackage.AssetSubCategory;
import com.nexstreaming.app.common.norm.NormTable;

import java.util.Map;

public class SubCategoryRecord extends NormTable implements AssetSubCategory {

    public long _id;
    @Unique @NotNull
    public long subCategoryId;
    public Map<String,String> subCategoryName;
    public String subCategoryAlias;

    @Override
    public long getSubCategoryId() {
        return subCategoryId;
    }

    @Override
    public Map<String, String> getSubCategoryName() {
        return subCategoryName;
    }

    @Override
    public String getSubCategoryAlias() {
        return subCategoryAlias;
    }
}
