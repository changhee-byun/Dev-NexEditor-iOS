/******************************************************************************
 * File Name        : nexCollageManager.java
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
import android.graphics.Rect;
import android.os.AsyncTask;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.Stopwatch;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @brief This class provides an easy way to use items of the collage Category in nexAssetPackageManager in nexCollageManager.
 * @since 2.0.0
 */
public class nexCollageManager {
    private static final String TAG="nexCollageManager";

    private static nexCollageManager sSingleton = null;
    private Context mAppContext;
    private Context mResContext;
    private List<Collage> collageEntries = new ArrayList<Collage>();

    /**
     * This enumeration defines the type of collage.
     *
     * @since 2.0.0
     */
    public enum CollageType{
        StaticCollage, DynamicCollage, ALL
    }

    /**
     * Item {@link nexAssetPackageManager.Item} interface of nexAssetPackageManager was inherited to make this class.
     *
     * This processes Collage Items in an AssetPackage as a group.
     * @since 2.0.0
     */
    public static class Collage extends nexAssetPackageManager.ItemEnt {
        private nexCollage collage;

        Collage(){
        }

        private Collage(nexAssetPackageManager.Item item){
            super(item);
        }

        /**
         * If the category of {@link nexAssetPackageManager.Item} is Collage, this API can create a Collage.
         * @param item - Item received using {@link nexAssetPackageManager#getInstalledAssetItemById(String)}
         * @return - \c NULL if the Item category is not collage.
         * @since 2.0.0
         */
        protected static Collage promote(nexAssetPackageManager.Item item){
            if( item.category() == nexAssetPackageManager.Category.collage ||
                    item.category() == nexAssetPackageManager.Category.staticcollage ||
                    item.category() == nexAssetPackageManager.Category.dynamiccollage ){
                Collage t = new Collage(item);
                return t;
            }
            return null;
        }

        protected void setCollage(nexCollage collage) {
            this.collage = collage;
        }

        protected nexCollage getCollage() {
            return this.collage;
        }

        @Override
        public String id(){
            return super.id();
        }

        @Override
        public String name(String locale){
            String name = packageInfo().assetName(locale);
            if( name != null)
                return name;
            return super.name(locale);
        }

        @Override
        public String[] getSupportedLocales(){
            if( packageInfo() == null){
                return new String[0];
            }
            return packageInfo().getSupportedLocales();
        }

        /**
         * This method gets the sub informations of collage .
         * @return The collage sub information of the collage.
         * @since 2.0.0
         */
        public List<nexCollageInfo> getCollageInfos() {
            if( collage == null || parsingCollage() == false ) {
                return null;
            }

            List<nexCollageInfo> infos = new ArrayList<>();

            for(nexCollageDrawInfo info : collage.getDrawInfos() ) {
                if( info.isSystemSource() == false )
                    infos.add(info);
            }

            for(nexCollageTitleInfo info : collage.getTitleInfos() ) {
                if( info.isUserTitle() ) {
                    infos.add(info);
                }
            }

            return infos;
        }

        /**
         * This method gets the sub information of x, y position from collage.
         * @return The collage information of the collage.
         * @since 2.0.0
         */
        public nexCollageInfo getCollageInfos(float x, float y) {
            if( collage == null || parsingCollage() == false ) {
                return null;
            }

            for(nexCollageTitleInfo info : collage.getTitleInfos() ) {

                if( info.isUserTitle() && info.isContains(x, y) ) {
                    return info;
                }
            }

            for(nexCollageDrawInfo info : collage.getDrawInfos() ) {
                if( info.isSystemSource() == false && info.isContains(x, y) ) {
                    return info;
                }
            }
            return null;
        }

        /**
         * This method gets collage type of collage.
         * @return Type of collage. StaticCollage orDynamicCollage
         * @since 2.0.0
         */
        public CollageType getType() {
            nexAssetPackageManager.ItemMethodType type = this.type();
            if( type == nexAssetPackageManager.ItemMethodType.ItemDynamicCollage ) {
                return CollageType.DynamicCollage;
            }
            else if( type == nexAssetPackageManager.ItemMethodType.ItemStaticCollage ) {
                return CollageType.StaticCollage;
            }
            else {
            }

            if( collage == null || parsingCollage() == false ) {
                return null;
            }

            return collage.getType();
        }

        /**
         * This method gets default duration of collage.
         * @return Default duration.
         * @since 2.0.0
         */
        public int getDuration() {

            if( collage == null || parsingCollage() == false ) {
                return 0;
            }

            return collage.getDuration();
        }

        /**
         * This method gets edit time of collage. All sub information must be draw at this time.
         * @return Edit time.
         * @since 2.0.0
         */
        public int getEditTime() {

            if( collage == null || parsingCollage() == false ) {
                return 0;
            }

            return (int)(collage.getDuration() * collage.getEditTime());
        }

        /**
         * This method gets ratio(width/height) of collage.
         * @return Width height ratio.
         * @since 2.0.0
         */
        public float getRatio() {

            if( collage == null || parsingCollage() == false ) {
                return 0;
            }

            float ratio = collage.getCollageRatio();
            return ratio;
        }

        /**
         * This method gets <tt>kAspecRatio</tt> of collage.
         * @return Ration mode.
         * @since 2.0.0
         */
        public int getRatioMode() {

            if( collage == null || parsingCollage() == false ) {
                return nexApplicationConfig.kAspectRatio_Mode_16v9;
            }

            float ratio = collage.getCollageRatio();
            if( ratio == 1f/1f )
                return nexApplicationConfig.kAspectRatio_Mode_1v1;

            if( ratio == 16f/9f )
                return nexApplicationConfig.kAspectRatio_Mode_16v9;

            if( ratio == 9f/16f )
                return nexApplicationConfig.kAspectRatio_Mode_9v16;

            if( ratio == 2f/1f )
                return nexApplicationConfig.kAspectRatio_Mode_2v1;

            if( ratio == 1f/2f )
                return nexApplicationConfig.kAspectRatio_Mode_1v2;

            if( ratio == 4f/3f )
                return nexApplicationConfig.kAspectRatio_Mode_4v3;

            if( ratio == 3f/4f )
                return nexApplicationConfig.kAspectRatio_Mode_3v4;

            return 0;
        }

        /**
         * This method apply collage on project.
         * @param project Destination project.
         * @param engine Engine instance.
         * @param recommendDuration  Default duration for applying collage.
         * @param appContext  App context.
         * @return Collage apply success or not.
         * @since 2.0.0
         */
        public boolean applyCollage2Project(nexProject project, nexEngine engine, int recommendDuration, Context appContext) throws ExpiredTimeException {
            if( nexAssetPackageManager.checkExpireAsset(packageInfo()) ){
                throw new ExpiredTimeException(id());
            }

            if( collage == null || parsingCollage() == false ) {
                return false;
            }

            String ret = collage.apply2Project(project, engine, recommendDuration, appContext, false);
            if( ret != null ) {
                Log.d(TAG, "applyCollage2Project failed with " + ret);
                return false;
            }
            return true;
        }

        protected boolean loadCollage2Project(nexProject project, nexEngine engine, int recommendDuration, Context appContext) {

            if( collage == null || parsingCollage() == false ) {
                return false;
            }

            String ret = collage.apply2Project(project, engine, recommendDuration, appContext, true);
            if( ret != null ) {
                Log.d(TAG, "applyCollage2Project failed with " + ret);
                return false;
            }
            return true;
        }
        /**
         * This method restore default collage bgm.
         * @return Restore success or not.
         * @since 2.0.0
         */
        public boolean restoreBGM() {
            if( collage == null ) return false;
            return collage.setBGM(null);
        }

        /**
         * This method get state of frame or general collage.
         * @return True is frame collage otherwise general collage.
         * @since 2.0.0
         */
        public boolean isFrameCollage() {
            if( collage == null ) return false;
            return collage.isFrameCollage();
        }

        /**
         * This method change clip of drawinfo.
         * @param info1 Instance of collage drawinfo
         * @param info2 Instance of collage drawinfo
         * @return True is that clips swapped or not.
         * @since 2.0.0
         */
        public boolean swapDrawInfoClip(nexCollageInfoDraw info1, nexCollageInfoDraw info2) {
            if( collage == null ) return false;

            return collage.swapDrawInfoClip((nexCollageDrawInfo)info1, (nexCollageDrawInfo)info2);
        }


        /**
         * This method get string from state of collage.
         * @return save data is format of string.
         * @since 2.0.0
         * @see #loadFromSaveData(Context, nexEngine, nexProject, String)
         */
        public String saveToString(){
            if( collage == null ) return null;

            Gson gson = new Gson();
            nexSaveDataFormat save = collage.getSaveData();

            return gson.toJson(save);

        }

        /**
         * load collage from {@link #saveToString()}.
         * <p>Example code :</p>
         * <pre>
         *     {@code
         *     //save
         *     String saveId = mCurCollage.id();
         *     String saveData = mCurCollage.saveToString();
         *
         *     //load
         *     nexEngine mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
         *
         *     nexProject mProject = nexProject.createFromSaveString(saveData);
         *
         *     List<nexCollageManager.Collage> mCollages = mKmCollageManager.getCollages(mProject.getTotalClipCount(true), collageShowType);
         *     nexCollageManager.Collage mCurCollage = mCollages.get(0);
         *     for( nexCollageManager.Collage collage : mCollages ){
                    if( collage.id().compareTo(saveId) == 0 ){
                        mCurCollage = collage;
                        break;
                    }
                }
                mCurCollage.loadFromSaveData(getApplicationContext(),mEngine,mProject,saveData);

         *
         *
            }</pre>
         * @param appContext App context.
         * @param engine Engine instance.
         * @param project project was made by createFromSaveString().
         * @param getSaveDataString {@link #saveToString()} return value.
         * @return true - load successful.
         * @see #saveToString()
         * @see nexProject#createFromSaveString(String)
         */
        public boolean loadFromSaveData(Context appContext, nexEngine engine,nexProject project, String getSaveDataString){

            if( collage == null ) return false;

            Gson gson = new Gson();
            nexSaveDataFormat saved = gson.fromJson(getSaveDataString,nexSaveDataFormat.class);
            if( saved.collage == null ){
                return false;
            }

            if( saved.project == null){
                return false;
            }
            collage.loadSaveData(saved.collage);
            loadCollage2Project(project,engine,getDuration(),appContext);
            engine.setProject(project);
            return true;
        }

        public int getSourceCount() {

            if( id().contains(".sc.") ) {
                String tmpID = id();
                int idx = tmpID.indexOf(".sc.");
                String sc = tmpID.substring(idx+4, tmpID.length());

                if( sc != null && sc.length() > 0 )
                    return Integer.parseInt(sc);
            }

            if( collage == null || parsingCollage() == false ) {
                return 0;
            }
            return collage.getSourceCount();
        }

        private boolean parsingCollage() {
            if( collage.isParsed() == false ) {
                try {
                    if(AssetPackageJsonToString(id())!=null) {
                        JSONObject jsonObject = new JSONObject(AssetPackageJsonToString(id()));
                        String ret = collage.parseCollage(jsonObject);
                        if( ret != null ) {
                            Log.d(TAG, "collage parsing error" + ret);
                            return false;
                        }
                    }
                } catch (JSONException e) {
                    e.printStackTrace();
                    Log.d(TAG, "Collage parsing error" + e.getMessage());
                    return false;
                }
            }
            return true;
        }
    }

    /**
     * This gets the collages information being managed in the form of CollageEntry.
     * @param sourceCount user image count.
     * @param type Collage type{@link nexCollageManager.CollageType}
     * @return The CollageEntry of the collages.
     * @since 2.0.0
     */
    public List<Collage> getCollages(int sourceCount, CollageType type) {

        if(EditorGlobal.SUPPORT_COLLAGE ) {

            List<Collage> newCollages = new ArrayList<>();
            for (Collage coll : collageEntries) {
                if (coll.getSourceCount() != sourceCount) continue;

//            float sdkRatio = nexApplicationConfig.getAspectRatio();
//            float collRatio = coll.collage.getCollageRatio();
//            Log.d(TAG, String.format("Ratio(%f %f)", sdkRatio, collRatio));
//            if( collRatio != sdkRatio ) {
//                continue;
//            }
                if (type == CollageType.ALL) {
                    newCollages.add(coll);
                }

                if (coll.getType() == type ) {
                    newCollages.add(coll);
                }
            }
            return newCollages;
        }
        return null;
    }

    public List<Collage> getCollages(CollageType type) {

        if(EditorGlobal.SUPPORT_COLLAGE ) {

            List<Collage> newCollages = new ArrayList<>();
            for (Collage coll : collageEntries) {
                if (type == CollageType.ALL) {
                    newCollages.add(coll);
                }

                if (coll.getType() == type ) {
                    newCollages.add(coll);
                }
            }
            return newCollages;
        }
        return null;
    }

    public List<Collage> getCollages() {

        if(EditorGlobal.SUPPORT_COLLAGE ) {

            List<Collage> newCollages = new ArrayList<>();
            for (Collage coll : collageEntries) {
                newCollages.add(coll);
            }
            return newCollages;
        }
        return null;
    }

    /**
     * This gets the collage information being managed in the form of CollageEntry.
     * @return The Collage from collage id.
     * @since 2.0.0
     */
    public Collage getCollage(String collageId) {
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            synchronized (m_collageEntryLock) {

                for (Collage coll : collageEntries) {
                    if (coll.id() != null && coll.id().compareTo(collageId) == 0) {
                        return coll;
                    }
                }
            }
        }
        return null;
    }

    private nexCollageManager(Context appContext, Context resContext ){
        mAppContext = appContext;
        mResContext = resContext;
    }

    /**
     * Creates and gets the instance of nexCollageManager. Call this API first to create an instance of nexCollageManager.
     *
     * Instances created by this API are saved as a \c static and shared with all.
     * @param appContext The application context.
     * @param resContext The context with collage resources.
     * @return The singleton instance of nexCollageManager.
     * @see #getCollageManager()
     * @since 2.0.0
     */
    public static nexCollageManager getCollageManager(Context appContext, Context resContext ) {

        if(EditorGlobal.SUPPORT_COLLAGE ) {
            if (sSingleton != null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName())) {
                sSingleton = null;
            }
            if (sSingleton == null) {
                sSingleton = new nexCollageManager(appContext, resContext);
            }
            return sSingleton;
        }
        return null;
    }

    /**
     * Gets the instances of nexCollageManager that have been created.
     * @note Do not call this API first. For it to work properly, {@link #getCollageManager(Context, Context)} should have been executed.
     * @return The singleton instance of nexCollageManager. If {@link #getCollageManager(Context, Context)} was never called before, the return value will be \c NULL.
     * @see #getCollageManager(Context, Context)
     * @since 2.0.0
     */
    public static nexCollageManager getCollageManager() {
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            return sSingleton;
        }
        return null;
    }

    private Object m_collageEntryLock = new Object();
    private boolean resolveCollage(boolean ignoreExpiredAsset){
        synchronized (m_collageEntryLock) {
            collageEntries.clear();

            List<nexAssetPackageManager.Item> collages = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(nexAssetPackageManager.Category.collage);
            for (nexAssetPackageManager.Item info : collages) {
                if (info.hidden() || (ignoreExpiredAsset && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo())) ) {
                    continue;
                }
                Collage t = Collage.promote(info);;
                if (t != null) {
                    nexCollage collage = new nexCollage();
                    t.setCollage(collage);
                    collageEntries.add(t);
//                    try {
//                        if(AssetPackageJsonToString(t.id())!=null) {
//                            JSONObject jsonObject = new JSONObject(AssetPackageJsonToString(t.id()));
//                            nexCollage collage = new nexCollage();
//                            String ret = collage.parseCollage(jsonObject);
//                            if( ret != null ) {
//                                Log.d(TAG, "collage parse error" + ret);
//                            }
//                            else {
//                                t.setCollage(collage);
//                                collageEntries.add(t);
//                            }
//                        }
//                    } catch (JSONException e) {
//                        e.printStackTrace();
//                        Log.d(TAG, "json create failed" + e.getMessage());
//                        return false;
//                    }
                }
            }

            collages = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(nexAssetPackageManager.Category.staticcollage);
            for (nexAssetPackageManager.Item info : collages) {
                if (info.hidden() || (ignoreExpiredAsset && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo())) ) {
                    continue;
                }
                Collage t = Collage.promote(info);;
                if (t != null) {
                    nexCollage collage = new nexCollage();
                    t.setCollage(collage);
                    collageEntries.add(t);
//                    try {
//                        if(AssetPackageJsonToString(t.id())!=null) {
//                            JSONObject jsonObject = new JSONObject(AssetPackageJsonToString(t.id()));
//                            nexCollage collage = new nexCollage();
//                            String ret = collage.parseCollage(jsonObject);
//                            if( ret != null ) {
//                                Log.d(TAG, "collage parse error" + ret);
//                            }
//                            else {
//                                t.setCollage(collage);
//                                collageEntries.add(t);
//                            }
//                        }
//                    } catch (JSONException e) {
//                        e.printStackTrace();
//                        Log.d(TAG, "json create failed" + e.getMessage());
//                        return false;
//                    }
                }
            }

            collages = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(nexAssetPackageManager.Category.dynamiccollage);
            for (nexAssetPackageManager.Item info : collages) {
                if (info.hidden() || (ignoreExpiredAsset && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo())) ) {
                    continue;
                }
                Collage t = Collage.promote(info);;
                if (t != null) {
                    nexCollage collage = new nexCollage();
                    t.setCollage(collage);
                    collageEntries.add(t);
//                    try {
//                        if(AssetPackageJsonToString(t.id())!=null) {
//                            JSONObject jsonObject = new JSONObject(AssetPackageJsonToString(t.id()));
//                            nexCollage collage = new nexCollage();
//                            String ret = collage.parseCollage(jsonObject);
//                            if( ret != null ) {
//                                Log.d(TAG, "collage parse error" + ret);
//                            }
//                            else {
//                                t.setCollage(collage);
//                                collageEntries.add(t);
//                            }
//                        }
//                    } catch (JSONException e) {
//                        e.printStackTrace();
//                        Log.d(TAG, "json create failed" + e.getMessage());
//                        return false;
//                    }
                }
            }
        }
        return true;
    }

    /**
     * Loads the collage resources from the assets and external drive.
     *
     * This API works through blocking.
     * For faster loading, it does not check collage effects.
     * @return true is success or not
     * @since 2.0.0
     */
    public boolean loadCollage(){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            return resolveCollage(false);
        }
        return false;
    }

    /**
     * Loads the collage resources from the assets and external drive.
     *
     * This API works through blocking.
     * For faster loading, it does not check collage effects.
     * @param ignoreExpired false - load expired asset, true - don't load expired asset
     * @return true is success or not
     * @since 2.0.0
     */
    public boolean loadCollage(boolean ignoreExpired){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            return resolveCollage(ignoreExpired);
        }
        return false;
    }

    /**
     * This method to uninstalls a Collage.
     *
     * This API is same as nexAssetPackageManager.uninstallPackageById(String).
     * @param collageId nexAssetPackageManager.ItemEnt.id() or nexAssetPackageManager.Item.id() of Collage Category.
     * @since 2.0.0
     */
    public void uninstallPackageById(String collageId){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).uninstallPackageById(collageId);
        }
    }

    /**
     * This checks if there is any new Collage package.
     *
     * This API is same as {@link nexAssetPackageManager#uninstallPackageById(String)}.
     * @return The number of new packages to install. 
     * @since 2.0.0
     */
    public int findNewPackages(){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            return nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).findNewPackages();
        }
        return 0;
    }

    /**
     * This API checks if installPackagesAsync is running or not.
     *
     * This API is same as {@link nexAssetPackageManager#isInstallingPackages()}.
     * @return True is running or not.
     * @since 2.0.0
     */
    public boolean isInstallingPackages(){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            return nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).isInstallingPackages();
        }
        return false;
    }

    /**
     * This API installs Collages received from the Asset Store.
     *
     * As a pre-requisite, run findNewPackages() first. 
     * If findNewPackages() finds a new package, then the developer should use it. 
     * installPackagesAsync is a single task, so check if it is running using isInstallingPackages() before using it. 
     * This API is same as {@link nexAssetPackageManager#installPackagesAsync(nexAssetPackageManager.OnInstallPackageListener)}.
     * @param listener
     * @since 2.0.0
     */
    public void installPackagesAsync(final nexAssetPackageManager.OnInstallPackageListener listener){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackagesAsync(listener);
        }
    }

    /**
     *
     * @param downloadedAssetIdx
     * @param listener
     * @since 2.0.0
     */
    public void installPackagesAsync(int downloadedAssetIdx, final nexAssetPackageManager.OnInstallPackageListener listener){
        if(EditorGlobal.SUPPORT_COLLAGE ) {
            nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).installPackagesAsync(downloadedAssetIdx, listener);
        }
    }

    private static String AssetPackageJsonToString(String ItemId){
        ItemInfo itemInfo = AssetPackageManager.getInstance().getInstalledItemById(ItemId);
        String content = null;
        if( itemInfo == null){
            Log.e(TAG,"AssetPackageJsonToString info fail="+ItemId);
            return null;
        }

        if( AssetPackageManager.getInstance().checkExpireAsset(itemInfo.getAssetPackage()) ){
            Log.e(TAG,"AssetPackageJsonToString expire id="+ItemId);
            return null;
        }

        AssetPackageReader reader = null;
        InputStream inputStream = null;

        try {
            reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),itemInfo.getPackageURI(),itemInfo.getAssetPackage().getAssetId());
        }catch (IOException e) {
            e.printStackTrace();
            return null;
        }

        try {
            inputStream = reader.openFile(itemInfo.getFilePath());
            if(inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferReader = new BufferedReader(inputStreamReader);
                StringBuilder stringBuilder = new StringBuilder();
                String receiveString;

                while ((receiveString = bufferReader.readLine()) != null) {
                    stringBuilder.append(receiveString);
                    stringBuilder.append("\n");
                }

                inputStream.close();
                content = stringBuilder.toString();
                return content;
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            CloseUtil.closeSilently(reader);
        }
        return null;
    }

    /**
     * get Collage ID from asset index and source count.
     * @param reload true - reload collage list. false - it was not reload.
     * @param assetIdx - asset index
     * @param sourceCount - able to count in college.
     * @return null - it was not mach for asset idx and source count, or preload collage package.
     * @since 2.0.0
     */
    public String getCollageItemId(boolean reload , int assetIdx, int sourceCount){
        if( reload ){
            loadCollage();
        }

        if( assetIdx < 2 ){
            return null;
        }
        //Log.d(TAG,"getCollageItemId assetIdx="+assetIdx+", sourceCount="+sourceCount);
        String id = null;
        for (nexCollageManager.Collage c : collageEntries) {
            if (c.packageInfo().assetIdx() == assetIdx) {
                //Log.d(TAG,"getCollageItemId id="+c.id()+", sourceCount="+c.getSourceCount());
                if( c.getSourceCount() == sourceCount ) {
                    id = c.id();
                    break;
                }
            }
        }
        //Log.d(TAG,"getCollageItemId id="+id+", sourceCount="+sourceCount);
        return id;
    }

    boolean updateCollage(boolean installed , nexAssetPackageManager.Item item) {
        synchronized (m_collageEntryLock) {
            boolean update = false;
            if (installed) {
                if (item.hidden()) {
                    return update;
                }
                Collage t = Collage.promote(item);
                if (t != null) {
                    try {
                        if (AssetPackageJsonToString(t.id()) != null) {
                            JSONObject jsonObject = new JSONObject(AssetPackageJsonToString(t.id()));
                            nexCollage collage = new nexCollage();
                            String ret = collage.parseCollage(jsonObject);
                            if (ret != null) {
                                Log.d(TAG, "collage parse error" + ret);
                            } else {
                                t.setCollage(collage);
                                collageEntries.add(t);
                                update = true;
                            }
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();
                        Log.d(TAG, "json create failed" + e.getMessage());
                    }
                }
            } else {
                List<Collage> removeEntries = new ArrayList<Collage>();
                for( Collage c : collageEntries ){
                    if (c.packageInfo().assetIdx() == item.packageInfo().assetIdx()) {
                        removeEntries.add(c);
                        //collageEntries.remove(c);
                        update = true;
                        //break;
                    }
                }
                collageEntries.removeAll(removeEntries);
            }
            return update;
        }
    }
}
