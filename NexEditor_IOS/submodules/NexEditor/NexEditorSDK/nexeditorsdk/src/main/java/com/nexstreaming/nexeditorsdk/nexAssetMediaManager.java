/******************************************************************************
 * File Name        : nexAssetMediaManager.java
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
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * This is a class that manages effects or BGM media from Asset Items.
 *
 * @since 1.7.1
 */

public class nexAssetMediaManager {
    private static nexAssetMediaManager sSingleton = null;
    private static Context mAppContext;
    private static final String TAG="nexAudioManager";
    private List<nexAssetMediaManager.AssetMedia> mediaEntries = new ArrayList<nexAssetMediaManager.AssetMedia>();
    private List<nexAssetMediaManager.AssetMedia> externalView_mediaEntries = null;

    /**
     * This is created by inheriting {@link com.nexstreaming.nexeditorsdk.nexAssetPackageManager.Item}, which is a media class of an Asset Item.
     * @since 1.7.1
     */
    public static class AssetMedia extends nexAssetPackageManager.ItemEnt{
        private int type;
        private boolean filter;
        private String mediaPath;
        private boolean getPath;
        private String preloadedAssetPath;
        private boolean getpreloadedAssetPath;

        AssetMedia(nexAssetPackageManager.Item item){
            super(item);
        }

        public static AssetMedia promote(nexAssetPackageManager.Item item){
            AssetMedia am = new AssetMedia(item);
            am.type = nexClip.kCLIP_TYPE_AUDIO;
            am.filter = false;
            //createAspect(t);
            return am;
        }

        /**
         * This gets the Media Path. For details, please see {@link nexAssetPackageManager#getAssetPackageMediaPath(Context, String)}.
         * @return  The Absolute Path of content.
         * @see nexAssetPackageManager#getAssetPackageMediaPath(Context, String)
         * @since 1.7.1
         */
        public String getPath(){
            if( !getPath ) {
                mediaPath = nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),id());
                getPath = true;
            }
            return mediaPath;
        }

        /**
         * This gets the AssetMedia Type value, which is the same as that of {@link nexClip#getClipType()}.
         * @return nexClip.kCLIP_TYPE_AUDIO - audio, nexClip.kCLIP_TYPE_VIDEO - video, nexClip.kCLIP_TYPE_IMAGE - image
         * @see nexClip#getClipType()
         * @since 1.7.1
         */
        public int getClipType(){
            return type;
        }

        /**
         * Assets may include video content that processes sound effects or background images from different media.
         * Media like this has a property of Filter.
         * @return \c TRUE - media for filters.
         */
        public boolean filter(){
            return filter;
        }

        /**
         *  If the type is {@link nexClip#kCLIP_TYPE_IMAGE}, then the developer can get the image thumbnail.
         *
         * @return \c NULL - No thumbnail or incorrect ID OR it is not nexClip.kCLIP_TYPE_IMAGE
         * @since 1.7.1
         */
        public Bitmap getImageThumbnail(){
            if( type != nexClip.kCLIP_TYPE_IMAGE ) {
                return null;
            }
            ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(id());
            Bitmap bmp = null;
            if (info != null) {
                try {
                    bmp = IconHelper.getThumbnailBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info, 0, 0);
                } catch (IOException e) {
                    //e.printStackTrace();
                }
            }
            return bmp;
        }

        /**
         * get path from preload nexAssetPackage.
         * @return app asset path at preload nexAssetPackage. null - It was not preload nexAssetPackage.
         * @since 2.0.0
         */
        public String getPreloadedAssetPath(){
            if( !getpreloadedAssetPath ) {
                preloadedAssetPath = nexAssetPackageManager.getPreloadedMediaAppAssetPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), id());
                getpreloadedAssetPath = true;
            }
            return preloadedAssetPath;
        }
    }

    private nexAssetMediaManager(Context appContext ){
        mAppContext = appContext;
    }

    /**
     * This gets an instance of nexAssetMediaManager.
     * @param appContext Input getApplicationContext(), but NOT \c NULL.
     * @return An instance of nexAssetMediaManager.
     * @since 1.7.1
     */
    public static nexAssetMediaManager getAudioManager(Context appContext ) {

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexAssetMediaManager(appContext);
        }
        return sSingleton;
    }
    private Object m_mediaentryLock = new Object();
    private void resolveMedia(int clipType, boolean ignoreExpired){
        synchronized (m_mediaentryLock) {
            mediaEntries.clear();
            if (clipType == nexClip.kCLIP_TYPE_AUDIO) {
                List<nexAssetPackageManager.Item> audios = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
                for (nexAssetPackageManager.Item info : audios) {
                    if ( info.hidden() || ( ignoreExpired && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo()) ) ) {
                        continue;
                    }
                    AssetMedia t = new AssetMedia(info);
                    t.type = nexClip.kCLIP_TYPE_AUDIO;
                    t.filter = false;
                    mediaEntries.add(t);
                }
            }
        }
    }

    /**
     * This loads media from Asset DB to nexAssetMediaManager.
     * This is only called once in the beginning; should also be called if the DB is updated.
     * @param clipType nexClip.kCLIP_TYPE_AUDIO - audio, nexClip.kCLIP_TYPE_VIDEO - video, nexClip.kCLIP_TYPE_IMAGE - image
     * @since 1.7.1
     */
    public void loadMedia(int clipType){
        resolveMedia(clipType, false);
    }

    /**
     * This loads media from Asset DB to nexAssetMediaManager.
     * This is only called once in the beginning; should also be called if the DB is updated.
     * @param clipType nexClip.kCLIP_TYPE_AUDIO - audio, nexClip.kCLIP_TYPE_VIDEO - video, nexClip.kCLIP_TYPE_IMAGE - image
     * @param ignoreExpired false - load expired asset, true - don't load expired asset
     * @since 1.7.52
     */
    public void loadMedia(int clipType, boolean ignoreExpired){
        resolveMedia(clipType, ignoreExpired);
    }

    /**
     * This gets the media item ID of a desired clip type.
     * Run {@link #loadMedia(int)} at least once
     * @param clipType nexClip.kCLIP_TYPE_AUDIO - audio, nexClip.kCLIP_TYPE_VIDEO - video, nexClip.kCLIP_TYPE_IMAGE - image
     * @return Array
     * @since 1.7.1
     */
    public String[] getAssetMediaIds(int clipType){
        synchronized (m_mediaentryLock) {
            List<String> ids = new ArrayList<>();
            for (AssetMedia item : mediaEntries) {
                if (item.getClipType() == nexClip.kCLIP_TYPE_AUDIO) {
                    ids.add(item.id());
                }
            }

            String[] ret = new String[ids.size()];
            for (int i = 0; i < ret.length; i++) {
                ret[i] = ids.get(i);
            }
            return ret;
        }
    }

    /**
     * This gets AssetMedia with the ID of a Media Asset item.
     * Run {@link #loadMedia(int)} at least once
     * @param Id  Asset Item ID
     * @return \c NULL - {@link #loadMedia(int)} did not run OR the ID is not one of the Media Asset Type IDs.
     * @since 1.7.1
     */
    public nexAssetMediaManager.AssetMedia getAssetMedia(String Id){
        synchronized (m_mediaentryLock) {
            for (nexAssetMediaManager.AssetMedia t : mediaEntries) {
                if (t.id().compareTo(Id) == 0) {
                    return t;
                }
            }
            return null;
        }
    }

    /**
     * This gets a list of all Media Asset items.
     * Run {@link #loadMedia(int)} at least once
     * @return An uneditable List.
     * @since 1.7.1
     */
    public List<nexAssetMediaManager.AssetMedia> getAssetMedias() {
        if( externalView_mediaEntries == null ){
            externalView_mediaEntries = Collections.unmodifiableList(mediaEntries);
        }
        return externalView_mediaEntries;
    }

    /**
     * This gets a list of Media Asset Items of a desire type as an \c array.
     * Run {@link #loadMedia(int)} at least once
     * @param type nexClip.kCLIP_TYPE_AUDIO - audio, nexClip.kCLIP_TYPE_VIDEO - video, nexClip.kCLIP_TYPE_IMAGE - image
     * @return Array
     * @since 1.7.1
     */
    public nexAssetMediaManager.AssetMedia[] getAssetMedias(int type) {
        synchronized (m_mediaentryLock) {
            List<AssetMedia> list = new ArrayList<>();
            for (AssetMedia m : mediaEntries) {
                if (m.getClipType() == type) {
                    list.add(m);
                }
            }

            AssetMedia[] ret = new AssetMedia[list.size()];
            for (int i = 0; i < ret.length; i++) {
                ret[i] = list.get(i);
            }
            return ret;
        }
    }

    /**
     * This creates a nexClip with the ID of a Media Asset item.
     * Run {@link #loadMedia(int)} at least once
     * @param Id Media Asset item ID
     * @return \c NULL - {@link #loadMedia(int)} did not run OR the ID is not one of the Media Asset Type IDs.
     * @since 1.7.1
     */
    public nexClip createAudioClip(String Id){
        AssetMedia info = getAssetMedia(Id);
        if( info == null )
            return null;
        return nexClip.getSupportedClip(info.getPath());
    }

    /**
     * This sets the BGM of {@link nexProject} with the ID of a Media Asset item.
     * If the ID is incorrect or it is not Audio, BGM is muted.
     * Run {@link #loadMedia(int)} at least once
     * @param project The project to which BGM will be set.
     * @param Id Media Asset item ID, which must be Audio type.
     * @since 1.7.1
     */
    public void applyProjectBGM(nexProject project, String Id){
        String path = null;
        AssetMedia info = getAssetMedia(Id);
        if( info != null ){
            if( nexAssetPackageManager.checkExpireAsset(info.packageInfo()) ){
                Log.d(TAG,"applyProjectBGM expire Id="+Id);
                return;
            }

            if( info.getClipType() == nexClip.kCLIP_TYPE_AUDIO ) {
                path = info.getPath();
            }
        }
        project.setBackgroundMusicPath(path);
    }

    /**
     * This uninstalls a BGM Media.
     *
     * This API is same as nexAssetPackageManager.uninstallPackageById(String).
     * @param Id nexAssetPackageManager.Item.id() of Template Category.
     * @since 1.7.22
     */
    public void uninstallPackageById(String Id){
        nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).uninstallPackageById(Id);
    }

    boolean updateMedia(boolean installed , int clipType, boolean filter ,nexAssetPackageManager.Item item){
        synchronized (m_mediaentryLock) {
            boolean update = false;
            Log.d(TAG,"updateMedia("+installed+","+clipType+","+item.packageInfo().assetIdx()+")");
            if(installed){
                if (!item.hidden()) {
                    AssetMedia t = new AssetMedia(item);
                    t.type = clipType;
                    t.filter = filter;
                    mediaEntries.add(t);
                }
            }else{
                for (AssetMedia m : mediaEntries) {
                    if( m.id().compareTo(item.id()) == 0 ){
                        mediaEntries.remove(m);
                        break;
                    }
                }
            }
            return update;
        }
    }
}
