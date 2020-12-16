package com.nexstreaming.app.common.nexasset.store;

import com.nexstreaming.app.common.nexasset.store.json.AssetStoreAPIData;

import java.util.List;
import java.util.Map;

public interface StoreSubcategoryInfo {

    String getSubcategoryAliasName();
    Map<String,String> getSubcategoryName();
    int getSubcategoryIdx();
    int getCategoryIdx();

}
