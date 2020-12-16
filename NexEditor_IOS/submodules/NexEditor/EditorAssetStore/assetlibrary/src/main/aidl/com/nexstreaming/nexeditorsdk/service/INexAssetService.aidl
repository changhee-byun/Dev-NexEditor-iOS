// INexAssetService.aidl
package com.nexstreaming.nexeditorsdk.service;
// Declare any non-default types here with import statements
import com.nexstreaming.nexeditorsdk.service.INexAssetInstallCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetUninstallCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetConnectionCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetDataCallback;

interface INexAssetService {

    void connectInstaller(int assetIndex, String thumbnailByteString, String json, INexAssetConnectionCallback callback);
    void sendAssetData(int assetIndex, String byteString, int size, long max, INexAssetInstallCallback callback);
    void uninstallAsset(int assetIndex, INexAssetUninstallCallback callback);
    void loadInstalledAssetList(INexAssetDataCallback callback);

    void saveAssetInfoData(int type, int index, String data);
}
