package com.nexstreaming.app.common.nexasset.store;

import android.content.Context;

import java.util.List;
import java.util.Map;

public interface StoreCategoryInfo {

    String getIconURL();
    String getSelectedIconURL();
    int getCategoryIdx();
    Map<String,String> getCategoryName();
    String getCategoryAliasName ();
    List<StoreSubcategoryInfo> getSubCategories();

}
