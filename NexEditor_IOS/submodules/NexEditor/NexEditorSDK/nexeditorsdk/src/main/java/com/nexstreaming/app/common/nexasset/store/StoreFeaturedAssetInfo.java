package com.nexstreaming.app.common.nexasset.store;


import java.util.List;

public interface StoreFeaturedAssetInfo {

    long getServerTimestamp();
    String getFeatuedAssetLayoutType();
    List<StoreAssetInfo> getFeaturedAssetList();

}
