package com.nexstreaming.app.common.nexasset.assetpackage;

import java.util.Map;

public interface AssetSubCategory {
    long getSubCategoryId();
    Map<String,String> getSubCategoryName();
    String getSubCategoryAlias();
}
