// IAssetManageService.aidl
package com.nexstreaming.sdk.assetstore;

import android.os.Bundle;
import com.nexstreaming.sdk.assetstore.IRemoteServiceCallback;

interface IRemoteService {

    void getFeaturedList(String marketId, int sdkLevel, int assetDeployType, int featuredType, IRemoteServiceCallback callback);

    void getImage(String imageUrl, IRemoteServiceCallback callback);
}
