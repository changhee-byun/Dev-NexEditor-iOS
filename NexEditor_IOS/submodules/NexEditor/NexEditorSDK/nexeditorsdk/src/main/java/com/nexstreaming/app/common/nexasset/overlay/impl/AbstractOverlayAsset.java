package com.nexstreaming.app.common.nexasset.overlay.impl;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAsset;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.IOException;
import java.io.InputStream;

public abstract class AbstractOverlayAsset implements OverlayAsset {

    private final ItemInfo itemInfo;

    public AbstractOverlayAsset(ItemInfo itemInfo) {
        this.itemInfo = itemInfo;
    }

    protected ItemInfo getItemInfo() {
        return itemInfo;
    }

    protected AssetPackageReader getAssetPackageReader() throws IOException {
        return AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),itemInfo.getPackageURI(),itemInfo.getAssetPackage().getAssetId());
    }

}
