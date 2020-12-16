package com.nexstreaming.app.assetlibrary.network.assetstore;

import java.util.List;
import java.util.Map;

public interface StoreCategoryInfo {

    String getIconURL();
    String getSelectedIconURL();
    int getCategoryIdx();
    Map<String,String> getCategoryName();
    String getCategoryAliasName();
    List<StoreSubcategoryInfo> getSubCategories();

}
