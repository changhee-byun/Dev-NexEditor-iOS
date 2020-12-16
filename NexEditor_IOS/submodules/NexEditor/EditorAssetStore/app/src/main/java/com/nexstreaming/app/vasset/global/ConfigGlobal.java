package com.nexstreaming.app.vasset.global;

import android.os.Environment;

import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;

import java.io.File;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class ConfigGlobal {

    /**
     * these key is came from SDK.
     */
    public static final String ASSET_STORE_MIME_TYPE = "AssetStore.intent.extra.MimeType"; //int
    public static final String ASSET_STORE_MIME_TYPE_EXTRA = "AssetStore.intent.extra.MimeTypeExtra"; //String
    public static final String ASSET_STORE_ASSET_INDEX = "AssetStore.intent.extra.AssetID"; //String
    public static final String ASSET_STORE_MULTI_SELECT = "AssetStore.intent.extra.MultiSelect"; //boolean
    public static final String ASSET_STORE_VENDOR = "AssetStore.intent.extra.Vendor"; //String
    public static final String ASSET_STORE_SETTING = "AssetStore.intent.extra.Setting"; // boolean

    public static final int AssetStoreMimeType_Template = 0x1;
    public static final int AssetStoreMimeType_Effect = 0x2;
    public static final int AssetStoreMimeType_Transition = 0x4;
    public static final int AssetStoreMimeType_Audio = 0x8;
    public static final int AssetStoreMimeType_Filter = 0x10;
    public static final int AssetStoreMimeType_Background = 0x20;
    public static final int AssetStoreMimeType_Overlay = 0x40;
    public static final int AssetStoreMimeType_RenderItem = 0x80;
    public static final int AssetStoreMimeType_DetailPage = 0x160;
    public static final int AssetStoreMimeType_TitleTemplate = 0x200;

    public static final int AssetStoreMimeType_Extra = 0x80000000;

    /**
     * Asset Store
     */

    public static final AssetStoreSession.AssetEnv DEFAULT_ASSET_ENV = AssetStoreSession.AssetEnv.PRODUCTION;

    public static final String API_SERVER_TEST = "https://test-apis-assetstore.nexstreaming.com";
    public static final String API_SERVER_PRODUCTION = "";
    public static final String API_SERVER_URL = API_SERVER_TEST;

    public static final String API_EDITION = "TEST";
    public static final String API_MARKET_ID = "LG";
    public static final String API_APP_UCODE = "NTlmZjhlMTViZjJiY2VmOWUwOTkwZjhiZmQxNTliNTBjZDkwOWQ3MTpMRw==";

}
