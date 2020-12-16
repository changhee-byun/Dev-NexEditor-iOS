package com.nexstreaming.nexwebview;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class AssetInfo {
    int idx;
    String packageURL;
    String thumbnailURL;

    public int idx(){
        return idx;
    }

    public String packageURL(){
        return packageURL;
    }

    public String thumbnailURL(){
        return thumbnailURL;
    }

    @Override
    public String toString() {
        return "AssetInfo{" +
                "idx=" + idx +
                ", packageURL='" + packageURL + '\'' +
                ", thumbnailURL='" + thumbnailURL + '\'' +
                '}';
    }
}
