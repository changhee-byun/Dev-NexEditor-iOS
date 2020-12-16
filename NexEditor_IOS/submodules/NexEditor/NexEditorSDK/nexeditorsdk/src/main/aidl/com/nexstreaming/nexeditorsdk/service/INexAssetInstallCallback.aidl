// INexAssetCallback.aidl
package com.nexstreaming.nexeditorsdk.service;

// Declare any non-default types here with import statements

interface INexAssetInstallCallback {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
     void onProgressInstall(int assetIndex, int percent, int max);
     void onInstallCompleted(int assetIndex);
     void onInstallFailed(int assetIndex, String reason);

}
