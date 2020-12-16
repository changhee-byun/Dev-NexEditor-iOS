package com.nexstreaming.app.common.nexasset.assetpackage;

import java.util.Map;

public interface ItemInfo {
    String getId();
    String getPackageURI();
    String getFilePath();
    String getIconPath();
    String getThumbPath();

    /**
     * Returns the translation map for the label for this item.
     *
     * It is recommended to use KMTLocalization.getLocalizedString() to look up the appropriate
     * string for the current locale.
     * @return
     */
    Map<String,String> getLabel();
    String getSampleText();
    ItemType getType();
    ItemCategory getCategory();
    AssetInfo getAssetPackage();
    boolean isHidden();
}
