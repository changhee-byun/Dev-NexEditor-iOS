package com.nexstreaming.app.assetlibrary.network.assetstore;


import java.util.List;
import java.util.Map;

public interface StoreAssetInfo{

    int getAssetIndex();
    String getAssetId();
    String getAssetTitle();
    String getAssetDescription();
    int getCategoryIndex();
    String getCategoryAliasName();
    String getCategoryIconURL();
    int getSubCategoryIndex();
    Map<String,String> getSubCategoryNameMap();
    Map<String,String> getAssetNameMap();
    Map<String,String> getAssetDescriptionMap();
    // TODO: Matthew Descritpion
    String getAssetPackageDownloadURL();
    String getPriceType();
    String getAssetThumbnailURL();
    String getAssetThumbnailURL_L();
    String getAssetThumbnailURL_S();
    List<String> getThumbnailPaths();
    int getAssetVersion();
    int getAssetScopeVersion();
    int getAssetFilesize();
    String getAssetVideoURL();
    int getUpdateTime();
    String getSubCategoryAliasName();

}
