/******************************************************************************
 * File Name        : NexInstalledAssetItem.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk.service;

/**
 * Created by ojin.kwon on 2016-11-30.
 */

public class NexInstalledAssetItem {

    public int index;
    public String id;
    public String assetName;
    public String categoryName;
    public String thumbUrl;
    public long installedTime;
    public long expireTime;
    public long sdkLevel;
    public long assetVersion;

    public NexInstalledAssetItem(int index, String id, String assetName, String categoryName, String thumbUrl) {
        this.index = index;
        this.id = id;
        this.assetName = assetName;
        this.categoryName = categoryName;
        this.thumbUrl = thumbUrl;
        this.installedTime = 0;
        this.expireTime = 0;
        sdkLevel = 2;
        assetVersion = 1;
    }

    public NexInstalledAssetItem(int index, String id, String assetName, String categoryName, String thumbUrl, long installedTime, long expireTime, int minVersion, int packageVersion) {
        this.index = index;
        this.id = id;
        this.assetName = assetName;
        this.categoryName = categoryName;
        this.thumbUrl = thumbUrl;
        this.installedTime = installedTime;
        this.expireTime = expireTime;
        this.sdkLevel = minVersion;
        this.assetVersion = packageVersion;
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
