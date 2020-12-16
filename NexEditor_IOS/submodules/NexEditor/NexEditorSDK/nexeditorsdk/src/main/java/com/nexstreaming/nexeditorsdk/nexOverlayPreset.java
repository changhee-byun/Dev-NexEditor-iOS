/******************************************************************************
 * File Name        : nexOverlayPreset.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.graphics.Bitmap;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


/**
 * This class allows the application to access image resources in the <tt>assets/overlay</tt> folder.
 * 
 * @return A bitmap image.
 * @since version 1.5.19
 */
public final class nexOverlayPreset {
    private static String TAG ="OverlayPresetLoad";
    private static nexOverlayPreset sSingleton = null;

    /**
     * This method returns a <tt>nexOverlayPreset</tt> instance. 
     *
     * @param context The context to get asset. 
     * @return An instance of <tt> nexOverlayPreset</tt>.
     * @since version 1.5.15
     */
    static public nexOverlayPreset getOverlayPreset(Context context){
        if( sSingleton == null ){
            sSingleton = new nexOverlayPreset(context);
        }
        return sSingleton;
    }
    /**
     * This method returns a <tt>nexOverlayPreset</tt> instance. 
     * 
     * @return An instance of <tt>nexOverlayPreset</tt>.
     * @since version 1.5.15
     */
    static public nexOverlayPreset getOverlayPreset(){
        return sSingleton;
    }

    nexOverlayPreset(Context context){

    }

    /**
     * This method returns a <tt>nexOverlayImage</tt> instance, which corresponds to the value set by the parameter. 
     *
     * <p>Example code :</p>
     * {@code String[] overlayIds = overlayPreset.getIDs();
        for(String overlayId: overlayIds) {
            if(overlayId.equals("search_id"))
                nexOverayImage overlayImage = ovelayPreset.getOverlayImage(overlayId);
            ...   
        }}
     *
     * @param overlayImageId An ID of <tt>nexOverlayImage</tt>. 
     *
     * @return An instance of <tt>nexOverlayImage</tt>.
     * @since version 1.5.15
     */
    public nexOverlayImage getOverlayImage(String overlayImageId ){
        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(overlayImageId);
        if( info == null ){
            return null;
        }

        if( info.getCategory() == ItemCategory.overlay && info.getType() == ItemType.overlay ) {
            return new nexOverlayImage(overlayImageId, true);
        }
        return null;
    }

    /**
     * This method gets the resource IDs in the <tt>assets/overlay</tt> folder.
     * <p>Example code :</p>
     * {@code String[] overlayIds = overlayPreset.getIDs();}
     *
     * @return The resource IDs, as a <tt>String</tt>.
     * @since version 1.5.15
     */
    public String[] getIDs(){
        ArrayList<String> list = new ArrayList<>();

        List<? extends ItemInfo> overlays = AssetPackageManager.getInstance().getInstalledItemsByCategory(ItemCategory.overlay);
        for( ItemInfo info : overlays ){
            if( info.isHidden()){
                continue;
            }

            if(info.getType() == ItemType.overlay ) {
                list.add(info.getId());
            }
        }

        String[] ret = new String[list.size()];
        for( int i = 0 ; i < ret.length ; i++ ){
            ret[i] = list.get(i);
        }
        return ret;
    }

    /**
     *
     * @param assetIdx
     * @return
     * @since 2.0.0
     */
    public String[] getIDs(int assetIdx){
        ArrayList<String> list = new ArrayList<>();

        List<? extends ItemInfo> overlays = AssetPackageManager.getInstance().getInstalledItemsByAssetPackageIdx(assetIdx);
        for( ItemInfo info : overlays ){
            if( info.isHidden()){
                continue;
            }

            if(info.getCategory() == ItemCategory.overlay && info.getType() == ItemType.overlay ) {
                list.add(info.getId());
            }
        }

        String[] ret = new String[list.size()];
        for( int i = 0 ; i < ret.length ; i++ ){
            ret[i] = list.get(i);
        }
        return ret;
    }


    /**
     * @brief This method gets the main thumbnail image of an Asset Sticker.
     * If the sticker format is SVG, this gets the thumbnail of a desired size.
     * @param overlayImageId Item ID
     * @param width The desired width of a thumbnail.
     * @param height The desired height of a thumbnail.
     * @return The bitmap of a thumbnail.
     * @since 1.7.0
     */
    public Bitmap getIcon(String overlayImageId, int width, int height){
        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(overlayImageId);
        Bitmap icon = null;

        if( info != null ) {
            try {
                icon = IconHelper.getIconBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info, width, height);
            } catch (IOException e) {
                //e.printStackTrace();
            }

            if (icon == null) {
                try {
                    icon = IconHelper.getThumbnailBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info, width, height);
                } catch (IOException e) {
                    //e.printStackTrace();
                }
            }

            if (icon != null) {
                return icon;
            }
        }
        return null;
    }
}
