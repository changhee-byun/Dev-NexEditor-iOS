/******************************************************************************
 * File Name        : nexOverlayFilter.java
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


import android.graphics.Bitmap;
import android.graphics.Rect;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAssetFactory;
import com.nexstreaming.app.common.nexasset.overlay.impl.NexEDLOverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.impl.RenderItemOverlayAsset;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;


import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;


/**
 * A filter used on an overlay. Applied to an overlay, the filter applies to video, Overlay images, and others on the Primary Track.
 * <p>Example code :</p>
 * {@code
 * nexOverlayFilter[] filters = nexEffectLibrary.getEffectLibrary(getApplicationContext()).getOverlayFilters();
 * for( nexOverlayFilter filter : filters ){
 *    if( filter.getId().compareTo("com.nexstreaming.editor.blurall") == 0 ){
 *        List<nexEffectOptions.RangeOpt> opts= filter.getEffectOption().getRangeOptions();
 *        for( nexEffectOptions.RangeOpt ro : opts ){
 *            ro.setValue(5);
 *        }
 *        nexOverlayItem overlayItem8 = new nexOverlayItem(filter,x,y,0,6000);
 *        project.addOverlay(overlayItem8);
 *    }
 * }
 *
 * @since 1.7.0
 */
public class nexOverlayFilter {
    private String id;
    private nexEffectOptions option;
    private String cacheEncodeOption ="";
    private boolean updateOption;
    private int width=700;
    private int height=700;
    private float relativeWidth = 1;
    private float relativeHeight = 1;
    private boolean isRelativeWidth = true;
    private boolean isRelativeHeight = true;
    private String[] mTitles= null;
    private static final int kMaxStringTrackCount = 8;
    private boolean updateBound;
    private boolean is3dmode;

    nexOverlayFilter(String id){
        this.id = id;
    }

    /**
     * This gets the ID.
     * @return The ID.
     * @since 1.7.0
     */
    public String getId() {
        return id;
    }

    /**
     * This gets the name.
     * @param locale
     * @return \c NULL, when there is no title in an Asset package.
     * @since 1.7.0
     */
    public String getName(String locale) {

            ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(id);
            if (info != null) {
                return info.getLabel().get("en");
            }

        return null;
    }

    /**
     * This gets the icon. Returns \c NULL, when there is no icon.
     * @return \c NULL, when there is no icon in an Asset package. 
     * @since 1.7.0
     */
    public Bitmap getIcon(){

            if( id != null) {
                ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(id);
                if( info != null ) {
                    try {
                        return IconHelper.getIconBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info,0,0);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }

        return null;
    }

    /**
     * If a Filter has a Text, this prints the Text that was input using this API. 
     * @param TrackNum 0 ~ 8
     * @param str The Text to set.
     * @since 1.7.0
     */
    public void setTitle(int TrackNum, String str) {
        if( mTitles == null ){
            mTitles = new String[kMaxStringTrackCount];
        }

        if( TrackNum < kMaxStringTrackCount ){
            mTitles[TrackNum] = str;
        }
    }

    /**
     * This gets the Title Text to set.
     * @param TrackNum 0 ~ 8
     * @return \c NULL - if there is not set Title or TrackNum is off the count. 
     * @since 1.7.0
     */
    public String getTitle(int TrackNum) {
        if( mTitles == null ){
            return null;
        }
        if( TrackNum < kMaxStringTrackCount ){
            return mTitles[TrackNum];
        }
        return null;
    }

    String[] getTitles(){
        return mTitles;
    }

    /**
     * This sets the width to which overlay filter will be applied. 
     * @param width 16:9 - 0 ~ 1280, 9:16 - 0 ~ 720
     * @since 1.7.0
     */
    public void setWidth(int width) {
        isRelativeWidth = false;
        if( this.width != width )
            updateBound = true;
        this.width = width;
    }

    /**
     * This sets the height to which overlay filter will be applied.
     * @param height 16:9 - 0 ~ 720, 9:16 - 0 ~ 1280
     * @since 1.7.0
     */
    public void setHeight(int height) {
        isRelativeHeight = false;
        if( this.height != height )
            updateBound = true;
        this.height = height;
    }

    /**
     * This gets the width to which overlay filter will be applied.
     * @return
     * @since 1.7.0
     */
    public int getWidth() {
        if( !isRelativeWidth)
            return width;
        float layer = nexApplicationConfig.getAspectProfile().getWidth();
        return (int)(layer * relativeWidth);
    }

    /**
     * This gets the height to which overlay filter will be applied.
     * @return
     * @since 1.7.0
     */
    public int getHeight() {
        if( !isRelativeWidth)
            return height;
        float layer = nexApplicationConfig.getAspectProfile().getHeight();
        return (int)(layer * relativeHeight);
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public float getRelativeWidth() {
        return relativeWidth;
    }

    /**
     *
     * @param relativeWidth
     * @since 2.0.0
     */
    public void setRelativeWidth(float relativeWidth) {
        isRelativeWidth = true;

        if( this.relativeWidth != relativeWidth )
            updateBound = true;

        this.relativeWidth = relativeWidth;
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public float getRelativeHeight() {
        return relativeHeight;
    }

    /**
     *
     * @param relativeHeight
     * @since 2.0.0
     */
    public void setRelativeHeight(float relativeHeight) {
        isRelativeHeight = true;

        if( this.relativeHeight != relativeHeight )
            updateBound = true;

        this.relativeHeight = relativeHeight;
    }

    /**
     * This gets the options that can be additionally set to a filter. 
     * @return
     * @since 1.7.0
     */
    public nexEffectOptions getEffectOption(){
        if( option == null ){
            option = nexEffectLibrary.getEffectLibrary(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getEffectOptions(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),id);
            option.setEffectType(nexEffect.kEFFECT_OVERLAY_FILTER);
        }
        updateOption = true;
        return option;
    }

    public int getIntrinsicWidth(){
        try {
            getOverlayAssetFilter();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        }

        if( cachedOverlayAsset == null ){
            return 0;
        }

        return cachedOverlayAsset.getIntrinsicWidth();
    }

    public int getIntrinsicHeight(){
        try {
            getOverlayAssetFilter();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        }

        if( cachedOverlayAsset == null ){
            return 0;
        }

        return cachedOverlayAsset.getIntrinsicHeight();
    }

    public boolean is3DMode(){
        return is3dmode;
    }

    public void set3DMode(boolean on){
        if( is3dmode != on){
            try {
                getOverlayAssetFilter();
                if( cachedOverlayAsset instanceof NexEDLOverlayAsset){
                    int mode = 0;
                    if( on ){
                        mode = 1;
                    }
                    ((NexEDLOverlayAsset)cachedOverlayAsset).setMode(mode);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } catch (XmlPullParserException e) {
                e.printStackTrace();
            }
        }
        is3dmode = on;
    }

    private OverlayAsset cachedOverlayAsset;

    OverlayAsset getOverlayAssetFilter() throws IOException, XmlPullParserException {
        if (cachedOverlayAsset == null) {
            cachedOverlayAsset = OverlayAssetFactory.forItem(id);
        }
        return cachedOverlayAsset;
    }

    String getEncodedEffectOptions(){

        if( option == null){
            return "";
        }else {
            if( updateOption ){
                updateOption = false;
                option.clearUpadted();
                cacheEncodeOption = nexEffect.getTitleOptions(option);
            }
            return cacheEncodeOption;
        }
    }

    void getBound(Rect bound){

        bound.left = 0 - getWidth()/2;
        bound.top = 0 - getHeight()/2;
        bound.right = 0 + getWidth()/2;
        bound.bottom = 0 + getHeight()/2;
    }

    boolean isUpdated(){
        if( updateBound )
            return true;

        if( option == null){
            return false;
        }else{
            return option.isUpdated();
        }

    }
}
