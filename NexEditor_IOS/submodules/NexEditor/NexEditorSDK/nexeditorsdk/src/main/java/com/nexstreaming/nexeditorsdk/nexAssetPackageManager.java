/******************************************************************************
 * File Name        : nexAssetPackageManager.java
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
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;
import android.util.Log;
import android.util.SparseArray;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.util.CopyUtil;
import com.nexstreaming.app.common.util.FileType;
import com.nexstreaming.app.common.util.StringUtil;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.editorwrapper.LookUpTable;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

/**
 *
 * @brief nexAssetPackageManager is a class that installs and manages any downloaded Assets from the Asset Store Application from NexStreaming.
 *
 * Assets downloaded from the Asset Store APP may not immediately work in nexEditorSDK. To use them properly, use nexAssetPackageManager to install them. 
 * Assets downloaded from the  Asset Store APP are encrypted, so the downloads are not available for immediate use. 
 * Therefore, communication with the Asset Store APP is required.
 * 
 * For this step, run the Asset Store APP service using nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(Context).
 * For your information, deleting the Asset Store APP when Assets are downloaded will make them unusable, and will not be usable even if the application is reinstalled. 
 * Reinstalling the Asset Store APP will delete all the Assets that were not installed.
 * 
 *  The code below is for installation.
 * 
 *  <p>Example code : 1</p>
 * {@code
 * barProgressDialog = new ProgressDialog(TemplateManagerActivity.this);
 * barProgressDialog.setTitle("Installing Asset Package ...");
 * barProgressDialog.setMessage("ready to install");
 * barProgressDialog.setProgressStyle(barProgressDialog.STYLE_HORIZONTAL);
 * barProgressDialog.setProgress(0);
 * barProgressDialog.setMax(100);
 * barProgressDialog.show();
 *
 * nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).installPackagesAsync(new nexAssetPackageManager.OnInstallPackageListener() {
 *     private int progress;
 *     private int count = 0;
 *     private int max = 0;
 *     private boolean updateMassage = false;
 *
 *     public void onProgress(int countPackage, int totalPackages, int progressInstalling) {
 *
 *         if( count != countPackage ){
 *            updateMassage = true;
 *         }
 *         count= countPackage;
 *         max = totalPackages;
 *         progress = progressInstalling;
 *
 *         updateBarHandler.post(new Runnable() {
 *
 *             public void run() {
 *                if( updateMassage ){
 *                    barProgressDialog.setMessage("install("+count+"/"+max+") in progress...");
 *                }
 *                barProgressDialog.setProgress(progress);
 *            }
 *        });
 *    }
 *
 *    public void onCompleted(int event) {
 *        barProgressDialog.dismiss();
 *    }
 * });
 * }
 * Most Asset IDs share its concept with path.
 * For example, read the examples below if you wish to use an audio Asset.
 * 
 *  <p>Example code : 2</p>
 * 
 * {@code
 * List<nexAssetPackageManager.Item> audios = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.audio);
 * if( audios.size() > 0 ){
 *    nexClip clip = new nexClip(audios.get(0).id());
 * }
 * }
 * 
 * If you wish to get the file path of audio and video Assets, use {@link #getAssetPackageMediaPath(Context, String)}.
 * If you wish to get a Typeface with a font ID, use {@link nexFont#getTypeface(Context, String)}. Input the font Asset ID in the second ID parameter.
 *  For instructions on how to use a Template, please see {@link nexTemplateManager}.
 *  For Stickers or Overlays, see {@link nexOverlayPreset}.
 * @since 1.7.0
 */
public class nexAssetPackageManager {
    private static final String TAG="nexAssetPackageMan";
    private static nexAssetPackageManager sSingleton = null;
    private Context mAppContext;

    public static final int Mode_Hot = 1;
    public static final int Mode_New = 2;

    /**
     * The value of {@link Item#category()}.
     *
     * @see {@link Item#category()}
     * @since 1.7.0
     */
    public enum Category{
        audio,
        audiofilter,
        background,
        effect,
        filter,
        font,
        overlay,
        template,
        transition,
        extra,
        collage,
        staticcollage,
        dynamiccollage,
        beattemplate
    }

    /**
     * A category of an Asset Package.
     * This is different from the category of item in an Asset. This category is used in the VAsset Store Application,
     * and the values are used for when an Asset Package wants to find out the directory of the category.
     * The number of categories may increase as the SDK version number goes up.
     *
     * @see {@link Asset#getCategoryAlias()}
     * @see {@link RemoteAssetInfo#getCategoryAlias()}
     */
    public enum PreAssetCategoryAlias{
        Effect,
        Transition,
        Font,
        Overlay,
        Audio,
        Template,
        ClipGraphics,
        TextEffect,
        Extra,
        Collage,
        StaticCollage,
        DynamicCollage,
        BeatTemplate
    }

    /**
     * The value of {@link Asset#installedType()}.
     *
     * @see {@link Asset#installedType()}
     * @since 1.7.0
     */
    public enum AssetInstallType{
        STORE, SHARE, APP_ASSETS, EXTRA
    }

    /**
     * The value of {@link Item#type()}.
     *
     * @see {@link Item#type()}
     * @since 1.7.57
     */
    public enum ItemMethodType{
        ItemOverlay,
        ItemRenderitem,
        ItemKedl,
        ItemAudio,
        ItemFont,
        ItemTemplate,
        ItemLut,
        ItemMedia,
        ItemExtra,
        ItemCollage,
        ItemStaticCollage,
        ItemDynamicCollage,
        ItemBeat
    }

    /**
     * This is an interface which gets the information of an installed Asset.
     * @since 1.7.0
     */
    public interface Item{
        /**
         * This gets the item label. If there is no label set to the Asset, it is same as id().
         *
         * @param locale The language or region information. For example,  "en" - English , "ko_KR"- Korean, \c NULL is default value.
         * @return The label name.
         * @since 1.7.0
         */
        String name(String locale);

        /**
         * The Asset ID.
         * @return The Asset ID.
         * @since 1.7.0
         */
        String id();

        Asset packageInfo();

        /**
         * Preparing...
         * @return
         * @since 1.7.0
         */
        boolean validate();

        /**
         * Only downloaded Assets from the Asset Store can be deleted. This API notifies whether or not the asset can be uninstalled.
         * @return \c TRUE - Can be deleted, \c FALSE - Do not delete.
         * @see #uninstallPackageById(String)
         * @see #uninstallPackageByAssetIdx(int)
         * @since 1.7.0
         */
        boolean isDelete();

        /**
         * This returns the icon in a package as a Bitmap.
         *
         * If there is no icon, it returns a thumbnail; \c NULL if thumbnail does not exist.
         * @return \c NULL - No icon set to the item.
         * @since 1.7.0
         */
        Bitmap icon();

        /**
         * This is the item category.
         * @return The item category.
         * @since 1.7.0
         */
        nexAssetPackageManager.Category category();

        /**
         * This gets the hidden property of an item. Sometimes, the hidden property is set to \c TRUE
         * in order to prevent the end user from using items for other purposes such as editing,
         * combining with other items, and ultimately causing license issues.
         * In this case, the application must process it so that the end user cannot see this item.
         * Basically, if this property is set to \c TRUE, the list of {@link nexTemplateManager}, {@link nexFont}, {@link nexOverlayPreset}, {@link nexAssetMediaManager}, and {@link nexEffectLibrary} will not appear visible.
         *
         * @return \c TRUE - the end user will not see this item, FALSE - visible and can be used for other purposes.
         * @since 1.7.1
         */
        boolean hidden();

        /**
         *
         * @return null - reload or debug asset , not null - Asset Store small size thumbnail
         * @since 1.7.4
         */
        Bitmap thumbnail();

        /**
         *
         * @return
         * @since 1.7.57
         */
        String [] getSupportedLocales();

        /**
         *
         * @return
         * @since 1.7.57
         */
        ItemMethodType type();
    }

    static class ItemEnt implements Item{
        protected String name;
        protected String id;
        protected Asset packInfo;
//        protected String assetIdx;
//        protected String assetId;
        protected boolean isDelete;
        protected boolean isHidden;
        protected nexAssetPackageManager.Category category;
        protected ItemMethodType type;
        private Map<String,String> namesMap;
        private boolean getNamesMap;
        ItemEnt(){

        }

        private void loadLabels(){
            if( namesMap == null && getNamesMap == false ){
                getNamesMap = true;
                namesMap = new HashMap<>();
                ItemInfo info= AssetPackageManager.getInstance().getInstalledItemById(id());
                if (info== null) {
                    Log.d(TAG, "No ItemInfo! id="+id());
                    return;
                }
                Map<String, String> labelMap = info.getLabel();
                if( labelMap != null ){
                    Set<String> keys = labelMap.keySet();
                    for( String key : keys ){
                        if( name == null ){
                            name = labelMap.get(key);
                        }
                        namesMap.put(key.toLowerCase(Locale.ENGLISH), labelMap.get(key));
                    }
                }
            }
        }

        protected ItemEnt(Item item){
            //name = item.name(null);
            id = item.id();
            packInfo = item.packageInfo();
//            assetIdx = item.assetIdx();
            category = item.category();
            isDelete = item.isDelete();
            isHidden = item.hidden();
            category = item.category();
            type = item.type();
        }

        @Override
        public String name(String locale) {
            loadLabels();

            if( locale == null ) {
                name = StringUtil.getLocalizedString(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), namesMap ,id);
            }

            if( locale == null ){
                if( name == null )
                    return id;
                return name;
            }

            if( namesMap != null ){
                String label= namesMap.get(locale.toLowerCase(Locale.ENGLISH));
                if( label != null ){
                    return label;
                }
            }

            if( name == null )
                return id;

            return name;
        }

        @Override
        public String id() {
            return id;
        }

        @Override
        public Asset packageInfo() {
            return packInfo;
        }

        @Override
        public boolean validate() {
            if( AssetPackageManager.getInstance().getInstalledItemById(id) != null )
                return true;
            return false;
        }

        @Override
        public boolean isDelete() {
            return isDelete;
        }

        @Override
        public Bitmap icon() {
            ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(id);
            Bitmap icon = null;

            if( info != null ) {
                try {
                    icon = IconHelper.getIconBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info,0,0);
                } catch (IOException e) {
                    //e.printStackTrace();
                }

                if( icon == null ){
                    try {
                        icon = IconHelper.getThumbnailBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info,0,0);
                    } catch (IOException e) {
                        //e.printStackTrace();
                    }
                }

                if( icon != null ){
                    return icon;
                }
            }
            return null;
        }

        @Override
        public Category category() {
            return category;
        }

        @Override
        public boolean hidden() {
            return isHidden;
        }

        @Override
        public Bitmap thumbnail(){

            if( packInfo == null ) {

                ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(id);

                if (info == null) {
                    return null;
                }

                AssetInfo assetInfo = info.getAssetPackage();
                if (assetInfo != null) {
                    String iconPath = assetInfo.getThumbPath();
                    if (iconPath != null) {
                        File iconFile = new File(iconPath);
                        if (iconFile.isFile()) {
                            return BitmapFactory.decodeFile(iconPath);
                        }
                    }
                }
                return null;
            }
            String thumbnailPath = packInfo.getThumbnailPath();
            if( thumbnailPath != null ){
                File iconFile = new File(thumbnailPath);
                if (iconFile.isFile()) {
                    return BitmapFactory.decodeFile(thumbnailPath);
                }
            }
            return null;
        }

        @Override
        public String[] getSupportedLocales() {
            loadLabels();
            if( namesMap == null ){
                return new String[0];
            }

            if( namesMap.size() == 0 ){
                return new String[0];
            }

            String [] ret = new String[namesMap.size()];
            int i = 0;
            for( String locale : namesMap.keySet() ){
                ret[i] = locale;
                i++;
            }
            return ret;
        }

        @Override
        public ItemMethodType type() {
            return type;
        }
    }
    /**
     * The Asset Package information.
     * @since 1.7.0
     */
    public interface Asset{
        /**
         * The Asset Store index.
         *
         * If it is App_Asset or Share type, the value has no meaning.
         *
         * @return
         * @since 1.7.0
         */
        int assetIdx();

        /**
         * The Asset ID.
         *
         * If it is App_Asset or Share type, the rootDirectory name of the package.
         * @return
         * @since 1.7.0
         */
        String assetId();

        /**
         * The Asset name. The developer must check if it is \c NULL. (null in some cases)
         * @param locale
         * @return
         * @since 1.7.0
         */
        String assetName(String locale);

        /**
         * Preparing...
         * @return
         * @since 1.7.0
         */
        String priceType();

        /**
         * Preparing...
         * @return
         * @since 1.7.0
         */
        long installedTime();

        /**
         * Preparing...
         * @return
         * @since 1.7.0
         */
        long expireRemain();

        /**
         * This gets the installation type of the Asset.
         * @return STORE - Downloaded from the Asset Store, SHARE - Dev shared folder, APP_ASSETS - Assets folder of the App.
         * @since 1.7.0
         */
        AssetInstallType installedType();

        /**
         * Asset category alias name.
         * @return
         * @since 1.7.1
         */
        String getCategoryAlias();

        /**
         * Asset installed thumbnail path.
         * Only use downloaded asset from Vasset Store.
         * @return null - pre loaded asset or plugin asset.
         * @since 1.7.9
         */
        String getThumbnailPath();

        /**
         *
         * @return
         * @since 1.7.57
         */
        String [] getSupportedLocales();

        /**
         *
         * @return
         * @since 1.7.57
         */
        int getAssetVersionCode();
    }

    static class AssetEnt implements Asset{
        protected int idx;
        protected String id;
        protected String name;
        protected Map<String,String > names;
        protected String price;
        protected long installedTime;
        protected long expireRemain;
        protected AssetInstallType type;
        protected String categoryAlias;
        protected String thumbnailPath;
        private boolean getNames;
        protected int assetVersionCode;

        AssetEnt(){

        }

        AssetEnt(Asset src){
            idx = src.assetIdx();
            id = src.assetId();
            name = src.assetName("en");
            price = src.priceType();
            installedTime = src.installedTime();
            expireRemain = src.expireRemain();
            type = src.installedType();
            categoryAlias = src.getCategoryAlias();
            thumbnailPath = src.getThumbnailPath();
            assetVersionCode = src.getAssetVersionCode();
        }

        private void loadNames(){
            if( names == null && getNames == false ){
                getNames = true;
                names = new HashMap<>();
                AssetInfo info = AssetPackageManager.getInstance().getInstalledPackageInfo(id);
                if ( info == null ) {
                    Log.d(TAG,"No AssetInfo! id="+id);
                    return;
                }
                Map<String, String> labelMap = info.getAssetName();
                if (labelMap != null) {
                    Set<String> keys = labelMap.keySet();
                    for( String key : keys){
                        if( name == null ){
                            name = labelMap.get(key);
                        }
                        names.put(key.toLowerCase(Locale.ENGLISH), labelMap.get(key));
                    }
                }else{
                    List<? extends ItemInfo> items = AssetPackageManager.getInstance().getInstalledItemsByAssetPackage(id);
                    AssetPackageReader reader = null;
                    try {
                        try {
                            reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), items.get(0).getPackageURI(), id);
                            labelMap = reader.getAssetName();

                            if (labelMap != null) {
                                Set<String> keys = labelMap.keySet();
                                for( String key : keys){
                                    if( name == null ){
                                        name = labelMap.get(key);
                                    }
                                    names.put(key.toLowerCase(Locale.ENGLISH), labelMap.get(key));
                                }
                            }
                        } finally {
                            reader.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        @Override
        public int assetIdx() {
            return idx;
        }

        @Override
        public String assetId() {
            return id;
        }

        @Override
        public String assetName(String locale) {
            loadNames();

            if( locale == null ) {
                name = StringUtil.getLocalizedString(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), names ,id);
            }
            /*
            if(name == null){
                name = id;
            }
*/
            if( names != null){
                if( locale == null ){
                    return name;
                }
                String label = names.get(locale.toLowerCase(Locale.ENGLISH));
                if( label != null ){
                    return label;
                }
            }

            if( name == null )
                return id;

            return name;
        }

        @Override
        public String priceType() {
            return price;
        }

        @Override
        public long installedTime() {
            return installedTime;
        }

        @Override
        public long expireRemain() {
            return expireRemain;
        }

        @Override
        public AssetInstallType installedType() {
            return type;
        }

        @Override
        public String getCategoryAlias() {
            return categoryAlias;
        }

        @Override
        public String getThumbnailPath(){
            return thumbnailPath;
        }

        @Override
        public String[] getSupportedLocales() {
            loadNames();
            if( names == null ){
                return new String[0];
            }

            if( names.size() == 0 ){
                return new String[0];
            }

            String [] ret = new String[names.size()];
            int i = 0;
            for( String locale : names.keySet() ){
                Log.d(TAG,"AssetEnt locale="+locale);
                ret[i] = locale;
                i++;
            }
            return ret;
        }

        @Override
        public int getAssetVersionCode() {
            return assetVersionCode;
        }
    }

    private nexAssetPackageManager(Context appContext ){
        mAppContext = appContext;
    }

    /**
     * This works via nexAssetPackageManager SingleTon 
     *
     * In the beginning, it creates an instance, and after the second call, it gets the instance that has been made. 
     * @param appContext Input getApplicationContext().
     * @return SingleTon instance of nexAssetPackageManager
     * @since 1.7.0
     */
    public static nexAssetPackageManager getAssetPackageManager( Context appContext ) {

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexAssetPackageManager(appContext);
            AssetLocalInstallDB.getInstance(appContext);
        }
        return sSingleton;
    }

    private ItemCategory getItemCategory(Category category){
        if( category == Category.audio ){
            return ItemCategory.audio;
        }else if( category == Category.audiofilter ){
            return ItemCategory.audiofilter;
        }else if( category == Category.background ){
            return ItemCategory.background;
        }else if( category == Category.effect ){
            return ItemCategory.effect;
        }else if( category == Category.filter ){
            return ItemCategory.filter;
        }else if( category == Category.font ){
            return ItemCategory.font;
        }else if( category == Category.overlay ){
            return ItemCategory.overlay;
        }else if( category == Category.template ){
            return ItemCategory.template;
        }else if( category == Category.transition ){
            return ItemCategory.transition;
        }else if( category == Category.collage ){
            return ItemCategory.collage;
        }else if( category == Category.staticcollage ){
            return ItemCategory.staticcollage;
        }else if( category == Category.dynamiccollage ){
            return ItemCategory.dynamiccollage;
        }else if( category == Category.beattemplate ){
            return ItemCategory.beattemplate;
        }

        return null;
    }

    private Category getCategory(ItemCategory category){
        if( category == ItemCategory.audio ){
            return Category.audio;
        }else if( category == ItemCategory.audiofilter ){
            return Category.audiofilter;
        }else if( category == ItemCategory.background ){
            return Category.background;
        }else if( category == ItemCategory.effect ){
            return Category.effect;
        }else if( category == ItemCategory.filter ){
            return Category.filter;
        }else if( category == ItemCategory.font ){
            return Category.font;
        }else if( category == ItemCategory.overlay ){
            return Category.overlay;
        }else if( category == ItemCategory.template ){
            return Category.template;
        }else if( category == ItemCategory.transition ) {
            return Category.transition;
        }else if( category == ItemCategory.collage ) {
            return Category.collage;
        }else if( category == ItemCategory.staticcollage ) {
            return Category.staticcollage;
        }else if( category == ItemCategory.dynamiccollage ) {
            return Category.dynamiccollage;
        }else if( category == ItemCategory.beattemplate ) {
            return Category.beattemplate;
        }
        return Category.extra;
    }

    static ItemMethodType getMethodType(ItemType type){
        if( type == ItemType.overlay ){
            return ItemMethodType.ItemOverlay;
        }else if( type == ItemType.renderitem ){
            return ItemMethodType.ItemRenderitem;
        }else if( type == ItemType.kedl ){
            return ItemMethodType.ItemKedl;
        }else if( type == ItemType.audio ){
            return ItemMethodType.ItemAudio;
        }else if( type == ItemType.font ){
            return ItemMethodType.ItemFont;
        }else if( type == ItemType.template ){
            return ItemMethodType.ItemTemplate;
        }else if( type == ItemType.lut ){
            return ItemMethodType.ItemLut;
        }else if( type == ItemType.media ){
            return ItemMethodType.ItemMedia;
        }else if( type == ItemType.collage ){
            return ItemMethodType.ItemCollage;
        }else if( type == ItemType.staticcollage ){
            return ItemMethodType.ItemStaticCollage;
        }else if( type == ItemType.dynamiccollage ){
            return ItemMethodType.ItemDynamicCollage;
        }else if( type == ItemType.beat ){
            return ItemMethodType.ItemBeat;
        }
        return null;
    }

    private void resolveAssets(List<Item> items, Category category){
        ItemCategory itemCategory = null;
        if( category != null ){
            itemCategory = getItemCategory(category);
            if( itemCategory == null ){
                return;
            }
        }

        List<? extends ItemInfo> infos = null;
        if( itemCategory == null ){
            return;
        }else {
            infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemsByCategory(itemCategory);
        }

        if( infos != null) {
            for (ItemInfo info : infos) {
                items.add(getItemEnt(info));
            }
        }
    }

    /**
     * This gets the list of the installed Asset Items. 
     * @return
     * @since 1.7.0
     */
    public List<Item> getInstalledAssetItems(){
        List<Item> list = new ArrayList<>();
        List<? extends ItemInfo> infos  = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItems();
        if( infos != null) {
            for (ItemInfo info : infos) {
                list.add(getItemEnt(info));
            }
        }
        return list;
    }

    /**
     * This gets the list of the installed Asset Items. 
     * @param category The category to search.
     * @return The item list that exists in the category.
     * @since 1.7.0
     */
    public List<Item> getInstalledAssetItems(Category category){
        List<Item> list = new ArrayList<>();
        resolveAssets(list,category);
        return list;
    }

    /**
     * This gets the list of the installed Asset Items. 
     * @param input The list that will input existing items in a category.
     * @param category Category to search.
     * @since 1.7.0
     */
    public void getInstalledAssetItems(List<Item> input, Category category){
        resolveAssets(input,category);
    }

    /**
     * This gets the list of Items which have the same IDX of the Asset.
     *
     * Some Asset includes just one Asset, but some like Template is a package of many Assets.
     * In other words, AssetIdx is the package ID.
     * @param assetIdx AssetIdx {@link Item#packageInfo()}
     * @return The Item list packaged with AssetIdx.
     * @since 1.7.0
     */
    public List<Item> getInstalledAssetItemsByAssetIDx( int assetIdx){
        List<? extends ItemInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemsByAssetPackageIdx(assetIdx);
        List<Item> list = new ArrayList<>();
        for( ItemInfo info : infos){
            list.add(getItemEnt(info));
        }
        return list;
    }

    /**
     * This gets the list of items included in theAsset Package ID.
     * @param assetId
     * @return
     * @since 1.7.0
     */
    public List<Item> getInstalledAssetItemsByAssetID( String assetId){
        List<? extends ItemInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemsByAssetPackage(assetId);
        List<Item> list = new ArrayList<>();
        for( ItemInfo info : infos){
            list.add(getItemEnt(info));
        }
        return list;
    }

    /**
     * This gets all information of the installed Asset Package.
     * @return
     * @since 1.7.0
     */
    public List<Asset> getInstalledAsset(){
        List<? extends AssetInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetPackages();
        List<Asset> list = new ArrayList<>();
        for( AssetInfo info : infos){
            list.add(getAssetEnt(info));
        }
        return list;
    }

    /**
     *
     * @param assetIdx
     * @param itemCategory
     * @return
     * @since 1.7.57
     */
    public List<Item> getInstalledAssetItemsByAssetIDx( int assetIdx, Category itemCategory ){

        List<? extends ItemInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemsByAssetPackageIdxAndCategory(assetIdx,getItemCategory(itemCategory));
        List<Item> list = new ArrayList<>();
        for( ItemInfo info : infos){
            list.add(getItemEnt(info));
        }
        return list;
    }

    /**
     * This gets all information of the Assets in the application from installed Asset Packages.
     * @return
     * @since 1.7.0
     */
    public List<Asset> getInstalledAssetByAppAsset(){
        List<? extends AssetInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetPackages();
        List<Asset> list = new ArrayList<>();
        for( AssetInfo info : infos){
            if( info.getInstallSourceType() == InstallSourceType.APP_ASSETS )
                list.add(getAssetEnt(info));
        }
        return list;
    }

    /**
     * This gets information of the dev shared folder from installed Asset Packages.
     * @return
     * @since 1.7.0
     */
    public List<Asset> getInstalledAssetByAppShare(){
        List<? extends AssetInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetPackages();
        List<Asset> list = new ArrayList<>();
        for( AssetInfo info : infos){
            if( info.getInstallSourceType() == InstallSourceType.FOLDER )
                list.add(getAssetEnt(info));
        }
        return list;
    }

    /**
     * From installed Asset Packages, this gets the installation information of the Asset downloaded from the store. 
     * @return
     * @since 1.7.0
     */
    public List<Asset> getInstalledAssetByStore(){
        List<? extends AssetInfo> infos = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetPackages();
        List<Asset> list = new ArrayList<>();
        for( AssetInfo info : infos){
            if( info.getInstallSourceType() == InstallSourceType.STORE )
                list.add(getAssetEnt(info));
        }
        return list;
    }

    /**
     *
     * @param assetIdx
     * @return
     * @since 1.7.7
     */
    public Asset getInstalledAssetByIdx(int assetIdx){
        AssetInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledPackageInfoByAssetIdx(assetIdx);
        if( info == null ){
            return null;
        }
        return getAssetEnt(info);
    }

    private ItemEnt getItemEnt(ItemInfo info){
        ItemEnt ent = new ItemEnt();
        ent.id = info.getId();
        Map<String,String> labelMap = info.getLabel();
        if( labelMap != null ){
            ent.name = labelMap.get("en");
        }
        if( ent.name == null){
            ent.name = ent.id;
        }

        ent.packInfo = getAssetEnt(info.getAssetPackage());

        if (ent.packInfo.installedType() == AssetInstallType.STORE) {
            ent.isDelete = true;
        }else{
            ent.isDelete = false;
        }
        ent.isHidden = info.isHidden();
        ent.category = getCategory(info.getCategory());
        ent.type = getMethodType(info.getType());

        return ent;
    }

    private AssetEnt getAssetEnt(AssetInfo info){
        AssetEnt ent = new AssetEnt();
        if( info == null ){
            Log.d(TAG,"getAssetEnt AssetInfo is null!");
            ent.type = AssetInstallType.EXTRA;
            ent.categoryAlias = "Unknown";
            ent.price = "Free";
        }else{
            ent.idx = info.getAssetIdx();
            ent.id = info.getAssetId();
            Map<String,String> labelMap = info.getAssetName();
            if( labelMap != null ) {
                ent.name = labelMap.get("en");
            }
            ent.price = info.getPriceType();
            if( ent.price == null ){
                ent.price = "Free";
            }
            ent.expireRemain = info.getExpireTime();
            ent.installedTime = info.getInstalledTime();
            if( info.getInstallSourceType() == InstallSourceType.APP_ASSETS ){
                ent.type = AssetInstallType.APP_ASSETS;
            }else if( info.getInstallSourceType() == InstallSourceType.FOLDER ){
                ent.type = AssetInstallType.SHARE;
            }else if( info.getInstallSourceType() == InstallSourceType.STORE ){
                ent.type = AssetInstallType.STORE;
            }else {

            }

            if( info.getAssetCategory() != null )
                ent.categoryAlias = info.getAssetCategory().getCategoryAlias();
            else
                ent.categoryAlias = "Unknown";
            ent.thumbnailPath = info.getThumbPath();
            ent.assetVersionCode = info.getPackageVersion();
        }
        return ent;
    }

    /**
     * If the Asset ID is known, the developer can get item information with this API.
     * @param id
     * @return Item
     * @since 1.7.0
     */
    public Item getInstalledAssetItemById(String id){
        ItemInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemById(id);
        if( info == null ){
            return null;
        }

        return getItemEnt(info);
    }

    /**
     * This uninstalls the package that matches the Asset IDX.
     *
     * Check with {@link Item#isDelete()} first whether or not the Asset can be uninstalled, then use the API.
     * @param assetIdx
     * @since 1.7.0
     */
    public void uninstallPackageByAssetIdx(int assetIdx){
        //yoon test
        putUninstallItem(assetIdx);

        try {
            AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).uninstallPackage(assetIdx);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * This uninstalls the Asset package that matches the ID.
     * 
     * Check with {@link Item#isDelete()} first whether or not the Asset can be uninstalled, then use the API.
     * @warning This does not only uninstall the item, but also other items that match the Asset IDX automatically.
     * @param itemId
     * @since 1.7.0
     */
    public void uninstallPackageById(String itemId){

        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(itemId);

        if( info == null ){
            return;
        }

        AssetInfo assetInfo = info.getAssetPackage();
        if(assetInfo != null){
            uninstallPackageByAssetIdx(assetInfo.getAssetIdx());
        }else{
            Log.d(TAG,"AssetInfo is null");
        }
    }

    /**
     * This checks if there is any new Template package.
     *
     * Call this API first to install a package with installPackagesAsync().
     * @return The number of new packages to install.
     * @since 1.7.0
     * @see #installPackagesAsync(OnInstallPackageListener)
     */
    public int findNewPackages(){
        return AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).checkStoreInstall();
    }

    /**
     * installPackagesAsync is a single task. If it is running, it cannot be re-run until it is finished.
     *
     * This API checks if installPackagesAsync is running or not.
     * @return \c TRUE - installPackagesAsync is running. \c FALSE - installPackagesAsync is ready.
     * @see #installPackagesAsync(OnInstallPackageListener)
     * @since 1.7.0
     */
    public boolean isInstallingPackages(){
        return AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).isInstallingPackages();
    }

    /**
     * This is the listener used in installPackagesAsync.
     * @see #installPackagesAsync(OnInstallPackageListener)
     * @since 1.7.0
     *
     */
    public static abstract class OnInstallPackageListener {
        /**
         * @since 1.7.0
         */
        public static int kEvent_installOk = 0;

        /**
         * @since 1.7.0
         */
        public static int kEvent_linstallFail = -1;

        /**
         * This notifies the installation progress of installPackagesAsync.
         * @param countPackage Count of packages currently installing. ( 1 ~ totalPackages )
         * @param totalPackages Count of new packages found using {@link nexAssetPackageManager#findNewPackages()}.
         * @param progressInstalling The installation progress of packages currently installing. ( 0 ~ 100 )
         * @since 1.7.0
         */
        public abstract void onProgress( int countPackage, int totalPackages,int progressInstalling );

        /**
         * This notifies the end of all installations.
         * @param event kEvent_installOk - OK, kEvent_linstallFail - fail
         * @since 1.7.0
         */
        public abstract void onCompleted( int event , int assetIdx);
    }

    private int installPackagesCount = 0;
    private int installPackagesMaxCount = 0;

    /**
     * This API installs Assets received from the Asset Store.
     *
     * As a pre-requisite, run findNewPackages() first.
     * If findNewPackages() finds a new package, then the developer should use it.
     * installPackagesAsync is a single task, so check if it is running using isInstallingPackages() before using it.
     * @param listener OnInstallPackageListener
     * @see #findNewPackages()
     * @see #isInstallingPackages()
     * @since 1.7.0
     */
    public void installPackagesAsync(final OnInstallPackageListener listener){
        installPackagesAsync(0, listener);
    }

    /**
     *
     * @param downloadedAssetIdx
     * @param listener
     * @since 1.7.34
     */
    public void installPackagesAsync(final int downloadedAssetIdx ,final OnInstallPackageListener listener){
        installPackagesCount = 0;
        installPackagesMaxCount = 0;

        Task task = AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackageAsync(downloadedAssetIdx);
        task.onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                if(listener != null)
                    listener.onCompleted(0,downloadedAssetIdx);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                if(listener != null)
                    listener.onCompleted(-1,downloadedAssetIdx);
            }
        }).onProgress(new Task.OnProgressListener() {
            @Override
            public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                if(listener != null){
                    if( maxProgress == 100 ) {
                        listener.onProgress(installPackagesCount, installPackagesMaxCount, progress);
                    }else{
                        installPackagesCount = progress;
                        installPackagesMaxCount = maxProgress;
                        listener.onProgress(progress, maxProgress, 0);
                    }
                }
            }
        });
    }

    /**
     * @brief This method installs Asset packages in the Asset Store Root path on a local disk through synchronization.
     * If they are properly installed, the existing Asset packages in the Asset Store Root path will be deleted.
     * @param assetIdx The index number of an Asset package in the Asset Store Root path on a local disk
     * @return \c TRUE - install succeeded; \c FALSE - install failed.
     * @see nexApplicationConfig#setAssetStoreRootPath(String)
     * @since 1.7.4
     */
    public boolean installPackageFromStorePath(int assetIdx){
        return AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackageSync(assetIdx);
    }

    /**
     * This gets the absolute path of the video or audio content included in the Asset.  
     * If the content is packed like an Asset or Zip, this API automatically copies the file to the cache space and returns that location.  
     * If the content file size is big, there might be blocking for a short time. 
     * Packages downloaded from the Asset Store are installed, so they notify the file location and therefore, there is no blocking.  
     * So putting Assets in the plugin folder is recommended in case where there are video or audio content in preloaded Assets. 
     * @param appContext - Input getApplicationContext().
     * @param ItemId - Item ID of the Audio or video included in the Asset.
     * @return  The Absolute Path of content.
     * @since 1.7.0
     */
    public static String getAssetPackageMediaPath(Context appContext, String ItemId){
        boolean isEncryptedItem = false;
        AssetLocalInstallDB.getInstance(appContext);

        ItemInfo mediaInfo = AssetPackageManager.getInstance().getInstalledItemById(ItemId);
        if (mediaInfo != null) {

            if( AssetPackageManager.getInstance().checkExpireAsset(mediaInfo.getAssetPackage()) ){
                Log.d(TAG,"getAssetPackageMediaPath expire id="+ItemId);
                return null;
            }

            AssetPackageReader reader = null;

            try {
                reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), mediaInfo.getPackageURI(), mediaInfo.getAssetPackage().getAssetId());
                try {
                    File f = reader.getLocalPath(mediaInfo.getFilePath());
                } catch (AssetPackageReader.EncryptedException e) {
                    Log.d(TAG, "getAssetPackageMediaPath encrypted item id=" + ItemId);
                    isEncryptedItem = true;
                } catch (IOException e){
                    ;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            if( reader == null ){
                Log.d(TAG,"getAssetPackageMediaPath file not found!="+ItemId);
                return null;
            }

            if( !isEncryptedItem ) {
                String packageURI = mediaInfo.getPackageURI();
                if (packageURI != null) {

                    String subURI = packageURI.substring(packageURI.indexOf(':') + 1);
                    if (packageURI.startsWith("file:")) {
                        File file = new File(subURI, mediaInfo.getFilePath());
                        if (file.isFile()) {
                            try {
                                reader.close();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                            return file.getAbsolutePath();
                        }
                    } else if (packageURI.startsWith("assets:")) {
                        String path = subURI + "/" + mediaInfo.getFilePath();
                        FileType ft = FileType.fromExtension(path);
                        if (!ft.isImage()) {
                            try {
                                AssetFileDescriptor descriptor = appContext.getAssets().openFd(path);
                                long offset = descriptor.getStartOffset();
                                long length = descriptor.getLength();
                                descriptor.close();
                                //cacheOpendAssets.put(path,descriptor);
                                try {
                                    reader.close();
                                } catch (IOException e) {
                                    e.printStackTrace();
                                }
                                return "nexasset://" + ItemId.hashCode() + ":" + offset + ":" + length;
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                }
            }else{
                String packageURI = mediaInfo.getPackageURI();
                if( packageURI != null) {
                    String path = packageURI+ "/" + mediaInfo.getFilePath();
                    FileType t =FileType.fromExtension(path);
                    if( t.isImage() ) {
                        try {
                            reader.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        return "@assetItem:" + ItemId;
                    }
                }
            }

            String cachePath = appContext.getCacheDir().getAbsolutePath() + File.separator + "media" ;
            File file = new File(cachePath);
            if (!file.exists()) {
                file.mkdirs();
            }

            Log.d(TAG, "media path=" + mediaInfo.getFilePath());
            File bgmFile = new File(cachePath , mediaInfo.getId());

            if (!bgmFile.isFile() && reader != null ) {
                //AssetPackageReader reader = null;
                try {
                    //reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), mediaInfo.getPackageURI(), mediaInfo.getAssetPackage().getAssetId());
                    InputStream inputStream = reader.openFile(mediaInfo.getFilePath());

                    if (inputStream != null) {
                        CopyUtil.in2sdcard(inputStream, bgmFile);
                        inputStream.close();
                        //project.setBackgroundMusicPath(folderPath + bgmInfo.getId());
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    bgmFile.delete();
                } finally {
                    if( reader != null )
                        try {
                            reader.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                }
                return bgmFile.getAbsolutePath();
            }else{
                try {
                    reader.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return bgmFile.getAbsolutePath();
            }
        }
        return null;
    }



    static Rect getAssetPackageMediaOptions(String ItemId){
        ItemInfo mediaInfo = AssetPackageManager.getInstance().getInstalledItemById(ItemId);
        BitmapFactory.Options opt = new BitmapFactory.Options();
        Rect rect = new Rect();
        if( mediaInfo != null ) {
            AssetPackageReader reader = null;
            try {
                reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), mediaInfo.getPackageURI(), mediaInfo.getAssetPackage().getAssetId());
                try {
                    InputStream inputStream = reader.openFile(mediaInfo.getFilePath());
                    InputStream is = new BufferedInputStream(inputStream);
                    is.mark(is.available());
                    opt.inJustDecodeBounds = true;
                    BitmapFactory.decodeStream(is,null, opt);
                    rect.set(0,0,opt.outWidth,opt.outHeight);
                    is.close();
                    inputStream.close();
                }catch (IOException e) {
                    e.printStackTrace();
                }
                reader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            return rect;
        }
        return rect;
    }

    /**
     *
     * @param appContext
     * @param ItemId
     * @return
     * @since 2.0.0
     */
    public static String getPreloadedMediaAppAssetPath(Context appContext, String ItemId){
        AssetLocalInstallDB.getInstance(appContext);

        ItemInfo mediaInfo = AssetPackageManager.getInstance().getInstalledItemById(ItemId);
        if (mediaInfo != null) {

            if( AssetPackageManager.getInstance().checkExpireAsset(mediaInfo.getAssetPackage()) ){
                Log.d(TAG,"getPreloadedAssetPath expire id="+ItemId);
                return null;
            }

            String packageURI = mediaInfo.getPackageURI();
            if( packageURI != null){
                String subURI = packageURI.substring(packageURI.indexOf(':')+1);
                if( packageURI.startsWith("assets:") ){
                    String path = subURI+"/"+mediaInfo.getFilePath();
                    Log.d(TAG,"getPreloadedAssetPath assets path="+path);
                    return path;
                }
            }
        }
        return null;
    }



    /**
     * This gets the category alias names of the currently installed Assets.
     * For the category alias name of basic Assets, see {@link PreAssetCategoryAlias}.
     * @return  category alias names of the currently installed Assets.
     * @see PreAssetCategoryAlias
     * @since 1.7.1
     */
    public String[] getAssetCategoriesWithInstalledItems(){
        List<? extends AssetCategory> cats = AssetPackageManager.getInstance().getAssetCategoriesWithInstalledItems();
        String[] ret = new String[cats.size()];
        for( int i = 0 ; i < ret.length ; i++ ){
            ret[i] = new String(cats.get(i).getCategoryAlias());
        }
        return ret;
    }
    /**
     * This is a class that notifies the information of a featured Asset in the Asset Store.
     *
     * This is not an installed Asset, but a link of a featured Asset in the Asset Store.
     * @see #getRemoteAssetInfos(int)
     * @see #getRemoteAssetInfo(int)
     * @since 1.7.0
     */
    public interface RemoteAssetInfo{

        /**
         * The Asset idx. Page redirection is available with this value. 
         * @return
         * @since 1.7.0
         */
        int idx();

        /**
         * The Asset ID.
         * @return
         * @since 1.7.0
         */
        String id();

        /**
         * The Asset Name.
         * @return
         * @since 1.7.0
         */
        String name();

        /**
         * The Asset icon.
         * @return
         * @since 1.7.0
         */
        Bitmap icon();

        /**
         * The Asset Category alias name.
         * @return
         * @since 1.7.1
         */
        String getCategoryAlias();
    }

    static class RemoteAssetInfoEnt implements RemoteAssetInfo{
        private int idx;
        private String id;
        private String name;
        private String iconPath;
        private String category;
        RemoteAssetInfoEnt(String iconPath, String id, int idx, String name, String category) {
            this.iconPath = iconPath;
            this.id = id;
            this.idx = idx;
            this.name = name;
            this.category = category;
        }

        @Override
        public int idx() {
            return idx;
        }

        @Override
        public String id() {
            return id;
        }

        @Override
        public String name() {
            return name;
        }

        @Override
        public Bitmap icon() {
            if( iconPath == null )
                return null;
            return BitmapFactory.decodeFile(iconPath);
        }

        @Override
        public String getCategoryAlias() {
            return category;
        }

        @Override
        public String toString() {
            return "RemoteAssetInfoEnt{" +
                    "category='" + category + '\'' +
                    ", idx=" + idx +
                    ", id='" + id + '\'' +
                    ", name='" + name + '\'' +
                    ", iconPath='" + iconPath + '\'' +
                    '}';
        }
    }
    /**
     * This gets the information of a featured Asset in the Asset Store.  
     * To get information, the Asset Store App must be installed and service must be running.
     * If the API does not acquire any information, the array size of the return value becomes 0.
     *
     * @param mode  Input the value of {@link nexAssetPackageManager#Mode_Hot}.
     * @return
     * @since 1.7.0
     */
    public RemoteAssetInfo[] getRemoteAssetInfos(int mode){

        List<AssetLocalInstallDB.remoteAssetItem>list = AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getList(mode,nexAssetStoreAppUtils.vendorName());
        RemoteAssetInfoEnt[] ret = new RemoteAssetInfoEnt[list.size()];
        int i = 0;
        for( AssetLocalInstallDB.remoteAssetItem item :  list){

            ret[i] = new RemoteAssetInfoEnt(item.thumbnailPath,item.id,item.idx,item.name, item.category);
            i++;
        }
        return ret;
    }

    private class AssetFeaturedList{
        private int id;
        private int current;
        private List<RemoteAssetInfoEnt> list;
    }

    private List<AssetFeaturedList> featuredLists;

    private void updateList(int mode){
        if( featuredLists == null ){
            featuredLists = new ArrayList<>();
        }

        List<AssetLocalInstallDB.remoteAssetItem>list = AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getList(mode,nexAssetStoreAppUtils.vendorName());
        if( list.size() == 0 ){
            return;
        }

        AssetFeaturedList afl = null;
        for( AssetFeaturedList l : featuredLists ){
            if(l.id == mode){
                afl = l;
                break;
            }
        }

        if( afl == null ){
            afl = new AssetFeaturedList();
            afl.id = mode;
            afl.list = new ArrayList<RemoteAssetInfoEnt>();
            featuredLists.add(afl);
        }else{
            afl.list.clear();
            afl.current = 0;
        }
        for( AssetLocalInstallDB.remoteAssetItem item :  list){
            afl.list.add(new RemoteAssetInfoEnt(item.thumbnailPath,item.id,item.idx,item.name, item.category));
        }
    }

    /**
     * This gets a random piece of information from a featured Asset.
     * Gets one piece at a time from Assets that are currently not installed. 
     * Returns \c NULL if it does not acquire any information.
     * @param mode Input the value of {@link nexAssetPackageManager#Mode_Hot}.
     * @return
     * @since 1.7.0
     */
    public RemoteAssetInfo getRemoteAssetInfo(int mode){
        if( featuredLists == null ) {
            updateList(mode);
        }

        AssetFeaturedList afl = null;
        for( AssetFeaturedList l : featuredLists ){
            if(l.id == mode){
                afl = l;
                break;
            }
        }

        if( afl == null ){
            return null;
        }

        if( afl.list.size() == 0 ){
            return null;
        }

        if( afl.current >= afl.list.size() ){
            updateList(mode);
            afl.current = 0;
        }
        Log.d(TAG,"afl.current="+afl.current+", afl.list.size()="+afl.list.size());
        RemoteAssetInfo ret = afl.list.get(afl.current);
        afl.current += 1;
        return ret;
    }

    /**
     * @deprecated For internal use only. Please do not use.
     * @since 1.7.1
     */
    @Deprecated
    public void uninstallFromAssetStoreApp(){
        AssetLocalInstallDB.getInstance(mAppContext).uninstallFromAssetStoreApp();
    }

    /**
     * This method validates an AssetPackage, whether all items of an Asset exist inside the AssetPackage.
     * Integrity of an Asset should be checked when the Asset installation path is a public directory.
     * @param assetIdx Index of an Asset to be checked
     * @return true - OK, false - NOK
     * @since 1.7.7
     */
    public boolean validateAssetPackage(int assetIdx){
        if( assetIdx < 3 ){
            return true;
        }

        List<? extends ItemInfo> infos = AssetPackageManager.getInstance().getInstalledItemsByAssetPackageIdx(assetIdx);
        if( infos.size() == 0 ){
            return false;
        }
        String packageUrl = infos.get(0).getPackageURI();
        if( !packageUrl.startsWith("file:") ){
            return true;
        }
        String subURI = packageUrl.substring(packageUrl.indexOf(':')+1);

        if( !(new File(subURI)).isDirectory() ){
            return false;
        }

        for( ItemInfo info : infos ){
            //Log.d(TAG,"getPackageURI="+info.getPackageURI()+", validateAssetPackage()1 ="+info.getFilePath());
            if( !(new File(subURI,info.getFilePath())).isFile() ){
                Log.d(TAG,"file not found "+subURI+"/"+info.getFilePath());
                return false;
            }
        }
        return true;
    }

    /**
     * This method checks the expiration time of an Asset.
     * If return value is false, an exception will be thrown when trying to use the Asset.
     * @param asset an Asset to be checked
     * @return false - valid, true - invalid (expired).
     * @see Item#packageInfo()
     * @see #getInstalledAssetByIdx(int)
     * @see #expireRemainTime(Asset)
     * @since 1.7.22
     */
    public static boolean checkExpireAsset(Asset asset){
        if ( asset == null ) {
            Log.d(TAG, "checkExpireAsset() : asset is null! return true.");
            return true;
        } else {
            return AssetPackageManager.getInstance().checkExpireAsset(AssetPackageManager.getInstance().getInstalledPackageInfoByAssetIdx(asset.assetIdx()));
        }
    }

    /**
     * This method checks the remaining time before expiration.
     * It returns the remaing time in milliseconds of an Asset to be used.
     * @param asset an Asset to be checked
     * @return remaining time to use; Long.MAX_VALUE means no expiration (unlimited use).
     * @see Item#packageInfo()
     * @see #getInstalledAssetByIdx(int)
     * @see #checkExpireAsset(Asset)
     * @since 1.7.22
     */
    public static long expireRemainTime(Asset asset){
        long expireRemainTime = Long.MAX_VALUE;
        if( asset.expireRemain() > 0 ){
            expireRemainTime = asset.installedTime() + asset.expireRemain() - System.currentTimeMillis();
            if( expireRemainTime < 0 ){
                expireRemainTime = 0;
            }
        }
        return expireRemainTime;
    }

    /**
     *
     * @param itemId
     * @return
     * @since 1.7.52
     */
    public PreAssetCategoryAlias getPreAssetCategoryAliasFromItem(String itemId){
        Item item = nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetItemById(itemId);
        if( item == null ){
            return PreAssetCategoryAlias.Extra;
        }
        return getPreAssetCategoryAliasFromItem(item);
    }

    /**
     *
     * @param item
     * @return
     * @since 1.7.52
     */
    public static PreAssetCategoryAlias getPreAssetCategoryAliasFromItem(Item item){
        if( item.category() == Category.template ){
            return PreAssetCategoryAlias.Template;
        }else if( item.category() == Category.overlay && item.type() == ItemMethodType.ItemTemplate ){
            return PreAssetCategoryAlias.TextEffect;
        }else if( item.category() == Category.overlay && item.type() == ItemMethodType.ItemOverlay){
            return PreAssetCategoryAlias.Overlay;
        }else if( item.category() == Category.audio ){
            return PreAssetCategoryAlias.Audio;
        }else if( item.category() == Category.effect ){
            return PreAssetCategoryAlias.Effect;
        }else if( item.category() == Category.transition ){
            return  PreAssetCategoryAlias.Transition;
        }else if( item.category() == Category.font ){
            return PreAssetCategoryAlias.Font;
        }else if( item.category() == Category.collage ){
            return PreAssetCategoryAlias.Collage;
        }else if( item.category() == Category.staticcollage){
            return PreAssetCategoryAlias.StaticCollage;
        }else if( item.category() == Category.dynamiccollage ){
            return PreAssetCategoryAlias.DynamicCollage;
        }else if( item.category() == Category.beattemplate ){
            return PreAssetCategoryAlias.BeatTemplate;
        }
        return PreAssetCategoryAlias.Extra;
    }

    /**
     * This api update asset information cache on manager(nexTemplateMansger, nexMediaManager, nexOverlayManager) by installed/uninstalled asset.
     * @param installed true - installed, false - uninstalled
     * @param assetIdx
     * @return
     * @since 1.7.52
     */
    public PreAssetCategoryAlias[] updateAssetInManager(boolean installed, int assetIdx ){
        Set<PreAssetCategoryAlias> set = new HashSet<>();
        List<Item> infos = null;

        if( !installed ) {
            infos = popUninstallItems(assetIdx);
            if( infos == null ) {
                Log.d(TAG, "updateAssetInManager can not found uninstalled items assetIdx=" + assetIdx);
            }
        }

        if( infos == null ){
            infos = nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetItemsByAssetIDx(assetIdx);
        }

        if( infos == null ){
            Log.d(TAG,"updateAssetInManager asset not found assetIdx="+assetIdx);
            return new PreAssetCategoryAlias[0];
        }

        if( infos.size() ==  0){
            Log.d(TAG,"updateAssetInManager asset size 0 assetIdx="+assetIdx);
            return new PreAssetCategoryAlias[0];
        }

        boolean clearTemplate = true;
        boolean clearTitleEffect = true;
        boolean clearCollage = true;
        boolean clearMusicTemplate = true;
        for (Item info : infos) {

            if (info.category() == Category.filter && info.type() == ItemMethodType.ItemLut) {
                Log.d(TAG,"updateAssetInManager update color effect assetIdx="+assetIdx);
                LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
                if (lut != null) {
                    if (info.hidden()) {
                        lut.setNeedUpdate();
                    } else {
                        nexColorEffect.setNeedUpdate();
                    }
                }
            } else if (info.category() == Category.template || info.category() == Category.beattemplate ) {
                nexTemplateManager man = nexTemplateManager.getTemplateManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), KineMasterSingleTon.getApplicationInstance().getApplicationContext());
                if (clearTemplate) {
                    man.updateTemplate(false, info);
                    clearTemplate = false;
                }

                if( installed ) {
                    man.updateTemplate(true, info);
                }

                if( info.category() == Category.beattemplate ){
                    nexBeatTemplateManager btman = nexBeatTemplateManager.getBeatTemplateManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
                    if( clearMusicTemplate ){
                        btman.updateBeatTemplate(false,info);
                        clearMusicTemplate = false;
                    }

                    if( installed ) {
                        btman.updateBeatTemplate(true,info);
                    }
                }

                if (!info.hidden()) {
                    if( info.category() == Category.template )
                        set.add(PreAssetCategoryAlias.Template);
                    else
                        set.add(PreAssetCategoryAlias.BeatTemplate);
                }
            } else if (info.category() == Category.overlay && info.type() == ItemMethodType.ItemTemplate) {

                nexOverlayManager man = nexOverlayManager.getOverlayManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), KineMasterSingleTon.getApplicationInstance().getApplicationContext());

                if (clearTitleEffect) {
                    man.updateOverlayTitle(false, info);
                    clearTitleEffect = false;
                }

                if( installed ) {
                    man.updateOverlayTitle(true, info);
                }

                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.TextEffect);
                }
            } else if (info.category() == Category.audio) {
                nexAssetMediaManager man = nexAssetMediaManager.getAudioManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
                man.updateMedia(false, 0, false, info);

                if( installed ) {
                    man.updateMedia(true, nexClip.kCLIP_TYPE_AUDIO, false, info);
                }

                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.Audio);
                }
            } else if (info.category() == Category.font) {
                nexFont.needUpdate();
                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.Font);
                }
            } else if (info.category() == Category.effect) {
                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.Effect);
                }
            } else if (info.category() == Category.transition) {
                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.Transition);
                }
            } else if (info.category() == Category.overlay && info.type() == ItemMethodType.ItemOverlay) {
                if (!info.hidden()) {
                    set.add(PreAssetCategoryAlias.Overlay);
                }
            } else if( info.category() == Category.collage || info.category() == Category.staticcollage || info.category() == Category.dynamiccollage){
                nexCollageManager man = nexCollageManager.getCollageManager();

                if( clearCollage ){
                    man.updateCollage(false,info);
                    clearCollage = false;
                }

                if( installed ) {
                    man.updateCollage(true,info);
                }

                if (!info.hidden()) {
                    PreAssetCategoryAlias alias = PreAssetCategoryAlias.Collage;

                    if( info.category() == Category.staticcollage ) {
                        alias = PreAssetCategoryAlias.StaticCollage;
                    }else if( info.category() == Category.dynamiccollage ) {
                        alias = PreAssetCategoryAlias.DynamicCollage;
                    }
                    set.add(alias);
                }
            }
        }

        PreAssetCategoryAlias [] ret = new PreAssetCategoryAlias[set.size()];
        int i = 0;
        for( PreAssetCategoryAlias al : set) {
            ret[i] =  al;
            i++;
        }
        return ret;
    }

    private List<Integer> uninstalledAssetIdxList = new ArrayList<>();
    private SparseArray<List<nexAssetPackageManager.Item>> uninstalledItems = new SparseArray<>();
    private Object uninstalledAssetLock = new Object();

    private void removeUninstallItem(int assetIdx){
        for(Iterator<Integer> it = uninstalledAssetIdxList.iterator(); it.hasNext() ; )
        {
            Integer value = it.next();

            if( value.intValue() == assetIdx ){
                it.remove();
                break;
            }
        }
        uninstalledItems.remove(assetIdx);

        if( uninstalledAssetIdxList.size() > 5 ){
            Integer idx = uninstalledAssetIdxList.remove(0);
            uninstalledItems.remove(idx.intValue());
        }
    }

    /**
     *
     * @param assetIdx
     * @since 1.7.57
     */
    public void putUninstallItem( int assetIdx  ){
        synchronized (uninstalledAssetLock) {
            removeUninstallItem(assetIdx);

            List<nexAssetPackageManager.Item> items = getInstalledAssetItemsByAssetIDx(assetIdx);
            if (items == null) {
                return;
            }
            uninstalledItems.put(assetIdx, items);
            uninstalledAssetIdxList.add(assetIdx);
        }
    }

    /**
     *
     * @param assetIdx
     * @return
     * @since 1.7.57
     */
    public List<nexAssetPackageManager.Item> popUninstallItems(int assetIdx){
        synchronized (uninstalledAssetLock) {
            List<nexAssetPackageManager.Item> items = uninstalledItems.get(assetIdx);
            removeUninstallItem(assetIdx);
            return items;
        }
    }
}
