package com.nexstreaming.app.assetlibrary.network.assetstore;

import java.util.List;
import java.util.Map;

/**
 * Created by matthewfeinberg on 11/10/16.
 */

public class WrappedStoreAssetInfo implements StoreAssetInfo {

    private final StoreAssetInfo wrapped;

    public WrappedStoreAssetInfo(StoreAssetInfo wrapped) {
        this.wrapped = wrapped;
    }

    @Override
    public int getAssetIndex() {
        return wrapped.getAssetIndex();
    }

    @Override
    public String getAssetId() {
        return wrapped.getAssetId();
    }

    @Override
    public String getAssetTitle() {
        return wrapped.getAssetTitle();
    }

    @Override
    public String getAssetDescription() {
        return wrapped.getAssetDescription();
    }

    @Override
    public int getCategoryIndex() {
        return wrapped.getCategoryIndex();
    }

    @Override
    public String getCategoryAliasName() {
        return wrapped.getCategoryAliasName();
    }

    @Override
    public String getCategoryIconURL() {
        return wrapped.getCategoryIconURL();
    }

    @Override
    public int getSubCategoryIndex() {
        return wrapped.getSubCategoryIndex();
    }

    @Override
    public Map<String, String> getSubCategoryNameMap() {
        return wrapped.getSubCategoryNameMap();
    }

    @Override
    public Map<String, String> getAssetNameMap() {
        return wrapped.getAssetNameMap();
    }

    @Override
    public Map<String, String> getAssetDescriptionMap() {
        return wrapped.getAssetDescriptionMap();
    }

    @Override
    public String getAssetPackageDownloadURL() {
        return wrapped.getAssetPackageDownloadURL();
    }

    @Override
    public String getPriceType() {
        return wrapped.getPriceType();
    }

    @Override
    public String getAssetThumbnailURL() {
        return wrapped.getAssetThumbnailURL();
    }

    @Override
    public String getAssetThumbnailURL_L() {
        return wrapped.getAssetThumbnailURL_L();
    }

    @Override
    public String getAssetThumbnailURL_S() {
        return wrapped.getAssetThumbnailURL_S();
    }

    @Override
    public List<String> getThumbnailPaths() {
        return wrapped.getThumbnailPaths();
    }

    @Override
    public int getAssetVersion() {
        return wrapped.getAssetVersion();
    }

    @Override
    public int getAssetScopeVersion() {
        return wrapped.getAssetScopeVersion();
    }

    @Override
    public int getAssetFilesize() {
        return wrapped.getAssetFilesize();
    }

    @Override
    public String getAssetVideoURL() {
        return wrapped.getAssetVideoURL();
    }

    @Override
    public int getUpdateTime() {
        return wrapped.getUpdateTime();
    }

    @Override
    public String getSubCategoryAliasName() {
        return wrapped.getSubCategoryAliasName();
    }

    @Override
    public boolean equals(Object obj) {
        if(obj != null && obj instanceof StoreAssetInfo){
            return getAssetIndex() == ((StoreAssetInfo) obj).getAssetIndex();
        }
        return super.equals(obj);
    }
}
