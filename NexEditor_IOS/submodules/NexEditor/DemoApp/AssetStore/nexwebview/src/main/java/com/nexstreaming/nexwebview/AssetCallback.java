package com.nexstreaming.nexwebview;

/**
 * Created by alex.kang on 2016-10-17.
 */

public interface AssetCallback {
    public void onTaskDone(String assetID);
    public void onFail(String message);
}