package com.nexstreaming.app.assetlibrary.model;


/**
 * Created by ojin.kwon on 2016-11-30.
 */

public class NexInstalledAssetItem {

    public int index;
    public String id;
    public String assetName;
    public String categoryName;
    public String thumbUrl;

    public NexInstalledAssetItem(int index, String id, String assetName, String categoryName, String thumbUrl) {
        this.index = index;
        this.id = id;
        this.assetName = assetName;
        this.categoryName = categoryName;
        this.thumbUrl = thumbUrl;
    }

    @Override
    public String toString() {
        return "nexAssetTransferData{" +
                "index=" + index +
                ", id='" + id + '\'' +
                ", assetName='" + assetName + '\'' +
                ", categoryName='" + categoryName + '\'' +
                ", thumbUrl='" + thumbUrl + '\'' +
                '}';
    }
}
