package com.nexstreaming.app.common.nexasset.assetpackage;

import java.io.File;
import java.util.Map;

public interface AssetInfo {

    int getAssetIdx();
    String getAssetId();
    String getAssetUrl();
    String getThumbUrl();
    String getThumbPath();
    File getLocalPath();
    String getPackageURI();

    /**
     * For assets installed from the store, this gives the price type of the
     * asset as reported by the store.
     * @return  The price type, or `null` if the asset was not installed from the store (in other
     *          words, `null` if the asset is included inside the APK).
     */
    String getPriceType();

    /**
     * For assets installed from the store, this gives the asset category.  For assets not installed
     * from the store, there is no asset category.
     * @return      The asset category, or `null` if this asset was not installed from the store.
     */
    AssetCategory getAssetCategory();


    AssetSubCategory getAssetSubCategory();

    /**
     * Returns the translation map for the name of this asset.
     *
     * It is recommended to use KMTLocalization.getLocalizedString() to look up the appropriate
     * string for the current locale.
     *
     * @return  A map of locales to translated strings, or null if the asset was not installed from
     *          the asset store.
     */
    Map<String,String> getAssetName();

    /**
     * Returns the translation map for the description of this asset.
     *
     * It is recommended to use KMTLocalization.getLocalizedString() to look up the appropriate
     * string for the current locale.
     *
     * @return  A map of locales to translated strings, or null if the asset was not installed from
     *          the asset store.
     */
    Map<String,String> getAssetDesc();

    InstallSourceType getInstallSourceType();

    long getInstalledTime();

    long getExpireTime();

    int getMinVersion();

    int getPackageVersion();
}
