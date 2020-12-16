package com.nexstreaming.app.common.nexasset.assetpackage;

import java.io.File;
import java.util.Map;


public class NoneAsset implements AssetInfo {

    public static NoneAsset INSTANCE = new NoneAsset();

    private NoneAsset(){}

    @Override
    public int getAssetIdx() {
        return 0;
    }

    @Override
    public String getAssetId() {
        return null;
    }

    @Override
    public String getAssetUrl() {
        return null;
    }

    @Override
    public String getThumbUrl() {
        return null;
    }

    @Override
    public String getThumbPath() {
        return null;
    }

    @Override
    public File getLocalPath() {
        return null;
    }

    @Override
    public String getPackageURI() {
        return null;
    }

    @Override
    public String getPriceType() {
        return null;
    }

    @Override
    public AssetCategory getAssetCategory() {
        return null;
    }

    @Override
    public AssetSubCategory getAssetSubCategory() {
        return null;
    }

    @Override
    public Map<String, String> getAssetName() {
        return null;
    }

    @Override
    public Map<String, String> getAssetDesc() {
        return null;
    }

    @Override
    public InstallSourceType getInstallSourceType() {
        return null;
    }

    @Override
    public long getInstalledTime() {
        return 0;
    }

    @Override
    public long getExpireTime() {
        return 0;
    }

    @Override
    public int getMinVersion() {
        return 0;
    }

    @Override
    public int getPackageVersion() {
        return 0;
    }
}
