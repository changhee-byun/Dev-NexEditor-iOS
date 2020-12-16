// INexAssetCallback.aidl
package com.nexstreaming.nexeditorsdk.service;

// Declare any non-default types here with import statements

interface INexAssetUninstallCallback {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
     void onUninstallCompleted(int assetIndex);
     void onUninstallFailed(int assetIndex, String reason);
}
