package com.nexstreaming.app.assetlibrary.network.assetstore;


import java.util.List;

public interface StoreFeaturedAssetInfo {

    long getServerTimestamp();
    String getFeatuedAssetLayoutType();
    List<StoreAssetInfo> getFeaturedAssetList();

}
