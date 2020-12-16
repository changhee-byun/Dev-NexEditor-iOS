package com.nexstreaming.app.vasset.global.asset.category;

import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;


/**
 * Created by ojin.kwon on 2016-11-23.
 */

public class CategoryAssetItem implements CategoryItem {

    private StoreAssetInfo assetInfo;
    private String lang;

    public CategoryAssetItem(StoreAssetInfo assetInfo, String lang) {
        this.assetInfo = assetInfo;
        this.lang = lang;
    }

    @Override
    public int getIndex() {
        return assetInfo.getAssetIndex();
    }

    @Override
    public String getTitle() {
        String title = assetInfo.getAssetTitle();
        if(assetInfo.getAssetNameMap() != null && assetInfo.getAssetNameMap().get(lang) != null){
            title = assetInfo.getAssetNameMap().get(lang);
        }
        return title;
    }

    @Override
    public String getSubTitle() {
        String category = assetInfo.getSubCategoryAliasName();
        if(assetInfo.getSubCategoryNameMap() != null && assetInfo.getSubCategoryNameMap().get(lang) != null){
            category = assetInfo.getSubCategoryNameMap().get(lang);
        }
        return category;
    }

    @Override
    public String getImageUrl() {
        return assetInfo.getAssetThumbnailURL_S();
    }

    @Override
    public String getTargetUrl() {
        return assetInfo.getAssetPackageDownloadURL();
    }

    public StoreAssetInfo getAssetInfo(){
        return assetInfo;
    }

    @Override
    public boolean equals(Object obj) {
        if(obj != null){
            if(obj instanceof StoreAssetInfo){
                return assetInfo.equals(obj);
            }else if(obj instanceof CategoryAssetItem){
                return assetInfo.equals(((CategoryAssetItem) obj).getAssetInfo());
            }
        }
        return super.equals(obj);
    }
}
