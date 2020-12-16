package com.nexstreaming.app.common.nexasset.assetpackage.db;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.norm.NormTable;

import java.util.Collections;
import java.util.Map;

public class ItemRecord extends NormTable implements ItemInfo {
    public long _id;
    @Unique @NormTable.NotNull
    public String itemId;
    public String packageURI;
    public String filePath;
    public String iconPath;
    public String thumbPath;
    public Map<String,String> label;
    public ItemType itemType;
    public ItemCategory itemCategory;
    public String sampleText;
    public boolean hidden;
    @Foreign @Index
    public AssetPackageRecord assetPackageRecord;

    @Override
    public String getId() {
        return itemId;
    }

    @Override
    public String getPackageURI() {
        return packageURI;
    }

    @Override
    public String getFilePath() {
        return filePath;
    }

    @Override
    public String getIconPath() {
        return iconPath;
    }

    @Override
    public String getThumbPath() {
        return thumbPath;
    }

    @Override
    public Map<String, String> getLabel() {
        if( label==null || label.isEmpty() ) {
            return Collections.singletonMap("en", itemId);
        }
        return label;
    }

    @Override
    public String getSampleText() {
        return sampleText;
    }

    @Override
    public ItemType getType() {
        return itemType;
    }

    @Override
    public ItemCategory getCategory() {
        return itemCategory;
    }

    @Override
    public AssetInfo getAssetPackage() {
        return assetPackageRecord;
    }

    @Override
    public boolean isHidden() {
        return hidden;
    }

    @Override
    public String toString() {
        return "ItemRecord{" +
                "_id=" + _id +
                ", itemId='" + itemId + '\'' +
                ", packageURI='" + packageURI + '\'' +
                ", filePath='" + filePath + '\'' +
                ", iconPath='" + iconPath + '\'' +
                ", thumbPath='" + thumbPath + '\'' +
                ", label=" + label +
                ", itemType=" + itemType +
                ", itemCategory=" + itemCategory +
                ", assetPackageRecord=" + assetPackageRecord +
                '}';
    }
}
