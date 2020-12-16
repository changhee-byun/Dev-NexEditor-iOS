package com.nexstreaming.app.assetlibrary.network.assetstore;

import java.util.Collections;
import java.util.List;
import java.util.Map;

public abstract class AbstractStoreAssetInfo implements StoreAssetInfo {
    @Override
    public int getAssetIndex() {
        return 0;
    }

    @Override
    public String getAssetId() {
        return null;
    }

    @Override
    public int getCategoryIndex() {
        return 0;
    }

    @Override
    public String getCategoryAliasName() {
        return null;
    }

    @Override
    public String getCategoryIconURL() {
        return null;
    }

    @Override
    public int getSubCategoryIndex() {
        return 0;
    }

    @Override
    public Map<String, String> getSubCategoryNameMap() {
        return Collections.emptyMap();
    }

    @Override
    public Map<String, String> getAssetNameMap() {
        return Collections.emptyMap();
    }

    @Override
    public String getAssetPackageDownloadURL() {
        return null;
    }

    @Override
    public String getPriceType() {
        return null;
    }

    @Override
    public String getAssetThumbnailURL() {
        return null;
    }

    @Override
    public List<String> getThumbnailPaths() {
        return null;
    }

    @Override
    public int getAssetVersion() {
        return 0;
    }

    @Override
    public int getAssetScopeVersion() {
        return 0;
    }

    @Override
    public int getAssetFilesize() {
        return 0;
    }

    @Override
    public String getAssetVideoURL() {
        return null;
    }

    @Override
    public String getAssetTitle() {
        return null;
    }

    @Override
    public String getAssetDescription() {
        return null;
    }

    @Override
    public int getUpdateTime() {
        return 0;
    }

    @Override
    public String getAssetThumbnailURL_L() {
        return null;
    }

    @Override
    public String getAssetThumbnailURL_S() {
        return null;
    }

    @Override
    public Map<String, String> getAssetDescriptionMap() {
        return Collections.emptyMap();
    }
}
