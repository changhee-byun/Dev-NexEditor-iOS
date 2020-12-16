package com.nexstreaming.nexwebview;

import android.os.Environment;

import java.io.File;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class ConfigGlobal {
    public static final String kAssetStoreMimeType = "AssetStore.intent.extra.MimeType"; //int
    public static final String kAssetStoreMimeTypeExtra = "AssetStore.intent.extra.MimeTypeExtra"; //String
    public static final String kAssetStoreAssetID = "AssetStore.intent.extra.AssetID"; //String
    public static final String kAssetStoreMultiSelect = "AssetStore.intent.extra.MultiSelect"; //boolean
    public static final String kAssetStoreVendor = "AssetStore.intent.extra.Vendor"; //String

    public static final int AssetStoreMimeType_Template = 0x1;
    public static final int AssetStoreMimeType_Effect = 0x2;
    public static final int AssetStoreMimeType_Transition = 0x4;
    public static final int AssetStoreMimeType_Audio = 0x8;
    public static final int AssetStoreMimeType_Filter = 0x10;
    public static final int AssetStoreMimeType_Background = 0x20;
    public static final int AssetStoreMimeType_Overlay = 0x40;
    public static final int AssetStoreMimeType_RenderItem = 0x80;
    public static final int AssetStoreMimeType_Font = 0x100;
    public static final int AssetStoreMimeType_TitleTemplate = 0x200;

    public static final int AssetStoreMimeType_Extra = 0x80000000;


    public static File getAssetStoreRootDirectory() {
        return new File( Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nexassets" );
    }
}
