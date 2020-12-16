package com.nexstreaming.app.common.nexasset.assetpackage.db;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetSubCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;
import com.nexstreaming.app.common.norm.NormTable;

import java.io.File;
import java.util.Map;

public class AssetPackageRecord extends NormTable implements AssetInfo {
    public long _id;

    @Unique @NormTable.NotNull
    public String assetId;

    public int assetIdx;

    public String assetUrl;
    public String thumbUrl;
    public String thumbPath;
    public String priceType;
    public String localPath;
    public CategoryRecord category;
    public SubCategoryRecord subCategory;
    public Map<String,String> assetName;
    public Map<String,String> assetDesc;
    public String packageURI;

    @AddColumn(12)
    public long installedTime;
    @AddColumn(12)
    public long expireTime;

    @AddColumn(13)
    public int minVersion;
    @AddColumn(13)
    public int packageVersion;

    @Foreign @Index
    public InstallSourceRecord installSource;

    @Override
    public int getAssetIdx() {
        return assetIdx;
    }

    @Override
    public String getAssetId() {
        return assetId;
    }

    @Override
    public String getAssetUrl() {
        return assetUrl;
    }

    @Override
    public String getThumbUrl() {
        return thumbUrl;
    }

    @Override
    public String getThumbPath() {
        return thumbPath;
    }

    @Override
    public File getLocalPath() {
        return localPath==null?null:new File(localPath);
    }

    @Override
    public String getPackageURI() {
        return packageURI;
    }

    @Override
    public String getPriceType() {
        return priceType;
    }

    @Override
    public AssetCategory getAssetCategory() {
        return category;
    }

    @Override
    public AssetSubCategory getAssetSubCategory() {
        return subCategory;
    }

    @Override
    public Map<String, String> getAssetName() {
        return assetName;
    }

    @Override
    public Map<String, String> getAssetDesc() {
        return assetDesc;
    }

    @Override
    public InstallSourceType getInstallSourceType() {
        return installSource==null?null:installSource.installSourceType;
    }

    @Override
    public long getInstalledTime() {
        return installedTime;
    }

    @Override
    public long getExpireTime() {
        return expireTime;
    }

    @Override
    public int getMinVersion() {
        return minVersion;
    }

    @Override
    public int getPackageVersion() {
        return packageVersion;
    }

    @Override
    public String toString() {
        return "AssetPackageRecord{" +
                "_id=" + _id +
                ", assetId='" + assetId + '\'' +
                ", assetIdx=" + assetIdx +
                ", assetUrl='" + assetUrl + '\'' +
                ", thumbUrl='" + thumbUrl + '\'' +
                ", thumbPath='" + thumbPath + '\'' +
                ", priceType='" + priceType + '\'' +
                ", localPath='" + localPath + '\'' +
                ", category=" + category +
                ", subCategory=" + subCategory +
                ", assetName=" + assetName +
                ", assetDesc=" + assetDesc +
                ", packageURI='" + packageURI + '\'' +
                ", installSource=" + installSource +
                '}';
    }
}
