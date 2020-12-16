package com.nexstreaming.app.assetlibrary.network.assetstore;

import java.util.Map;

public interface StoreSubcategoryInfo {

    String getSubcategoryAliasName();
    Map<String,String> getSubcategoryName();
    int getSubcategoryIdx();
    int getCategoryIdx();

}
