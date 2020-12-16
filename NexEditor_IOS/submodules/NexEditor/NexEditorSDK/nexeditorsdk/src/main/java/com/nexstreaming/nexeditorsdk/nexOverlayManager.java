/******************************************************************************
 * File Name        : nexOverlayManager.java
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
import android.graphics.BitmapFactory;
import android.graphics.Paint;
import android.graphics.Rect;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.fonts.Font;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;


/**
 * This class is used to retrieve and set overlay title templates.
 *
 * @since version 1.7.0
 */
public class nexOverlayManager {
    private static nexOverlayManager sSingleton = null;
    private static Context mAppContext;
    private static final String TAG="nexOverlayManager";

    private List<Overlay> overlayEntries = new ArrayList<Overlay>();
    private List<Overlay> externalView_overlayEntries = null;


    private nexOverlayTitle overlayTitle = null;

    private static boolean sAutoAspectSelect = true;

    
    public static void setAutoSelectFromAspect(boolean on){
        sAutoAspectSelect = on;
    }


    /**
     * This class is used to retrieve the overlay title template.
     *
     * @since version 1.7.0
     */
    public static class Overlay extends nexAssetPackageManager.ItemEnt {
        private int tag;
        private float aspect[];
        private int selectAspect;
        private int maxAspect;
        private String [] ids;

        private nexOverlayTitle overlayTitle = null;

        private Overlay(nexAssetPackageManager.Item item){
            super(item);
        }

        @Override
        public String id(){
            if( sAutoAspectSelect ){
                selectAspect();
            }
            return ids[selectAspect];
        }

        @Override
        public String name(String locale){
            String name = packageInfo().assetName(locale);
            if( name != null)
                return name;
            return super.name(locale);
        }
        /**
         * This method is used to get the ID of an overlay title template.
         *
         * @return  ID of an overlay title template.
         * @since version 1.7.0
         */
        public String getId() {
            return id();
        }

        /**
         * This method is used to get the name of an overlay title template.
         *
         * @param locale Encoded locale of name.
         * @return  Name of an overlay title template.
         * @since version 1.7.0
         */
        public String getName(String locale) {
            return name(locale);
        }

        /**
         * This method is used to get the bitmap icon of an overlay title template.
         *
         * @return  Bitmap icon of an overlay title template.
         * @since version 1.7.0
         */
        public Bitmap getIcon() {
            return icon();
        }

        /**
         *
         * @return
         * @since 1.7.8
         */
        public Bitmap getThumbnail(){
            return thumbnail();
        }

        /**
         * @brief This method is used to get the ratio of an overlay title template.
         *
         * @return  Ratio of an overlay title template.
         * @since version 1.7.2
         */
        public float getRatio() {
            if( sAutoAspectSelect ){
                selectAspect();
            }

/*
            if( id.contains("9v16") ){
                return 9f/16f;
            }else if( id.contains("2v1") ){
                return 2f;
            }else if( id.contains("1v1") ){
                return 1;
            }else if( id.contains("1v2") ){
                return 1f/2f;
            }

            return 16f/9f;
*/
            return aspect[selectAspect];
        }

        /**
         * @brief This method is used to get the sub title included in an overlay title template.
         *
         * @return Sub title included in an overlay title template.
         * @since version 1.7.2
         */
        public boolean includeSubtitle() {
            if( ids != null ) {
                for (String id : ids) {
                    if( id.contains("sub") ){
                        return true;
                    }
                }
            }
            return false;
        }

        /**
         * @brief This method is used to get a tag value.
         *
         * @return Tag value.
         * @since version 1.7.2
         */
        public int getTag() {
            return tag;
        }

        /**
         * This method is used to set the tag value for saving custom information.
         *
         * @param tag new custom int value.
         * @since version 1.7.2
         */
        public void setTag(int tag) {
            this.tag = tag;
        }

        private static Overlay promote(nexAssetPackageManager.Item item){
            if( item.category() == nexAssetPackageManager.Category.overlay && item.type() == nexAssetPackageManager.ItemMethodType.ItemTemplate ){
                ;
            }else{
                return null;
            }

            Overlay t = new Overlay(item);
            return t;
        }

        /**
         * The supported Aspect ratio values.
         * @return 16/9 - 16:9 , 1 - 1:1 , 9/16 - 9:16 , 2 - 2:1
         * @since 1.7.0
         */
        public float[] getSupportedAspects(){
            float[] ret = new float[maxAspect];
            for(int i = 0 ; i < ret.length ; i++){
                ret[i] = aspect[i];
            }
            return ret;
        }

        /**
         * The Aspect ratio index to set.
         * @param index Value smaller than the length of getSupportedAspects().
         * @see #getSupportedAspects()
         * @since 1.7.0
         */
        public void selectAspect(int index){
            if( maxAspect <=  index )
                return;
            selectAspect = index;
        }

        private void selectAspect(){
            float ar = nexApplicationConfig.getAspectRatio();
            float aspectDim = 3f;
            for(int i = 0 ; i < maxAspect ; i++  ){
                float aspectDelta = ar - aspect[i];
                if( aspectDelta < 0 ) aspectDelta *= -1f;
                if( aspectDim > aspectDelta ){
                    aspectDim = aspectDelta;
                    selectAspect = i;
                }
            }
        }

        void setAspect(String id){
            if( maxAspect >= 5 ){
                return;
            }
            if( this.aspect == null ) {
                this.aspect = new float[5];
                this.ids = new String[5];
            }

            if( id.contains("9v16") ){
                this.aspect[maxAspect] = 9f/16f;
            }else if( id.contains("2v1") ){
                this.aspect[maxAspect] = 2;
            }else if( id.contains("1v2") ){
                this.aspect[maxAspect] = 1f/2f;
            }else if( id.contains("1v1") ){
                this.aspect[maxAspect] = 1;
            }else{
                this.aspect[maxAspect] = 16f/9f;
            }

            this.ids[maxAspect] = id;

            maxAspect++;

            selectAspect();
        }

        public String getVersion() {
            if( id.contains(".ver_") ) {
                String ver = id.substring(id.indexOf(".ver_")+5, id.indexOf(".ver_")+8);
                return ver;
            }
            return "100";
        }

        protected nexOverlayTitle getOverlayTitle() {
            return overlayTitle;
        }

        protected void setOverlayTitle(nexOverlayTitle overlayTitle) {
            this.overlayTitle = overlayTitle;
        }

    }

    /**
     * This class is used to retrieve the title information of an overlay title template.
     *
     * @since version 1.7.0
     */
    public static class nexTitleInfo {
        private int id;
        private String text;
        private String textDesc;
        private int textMaxLen;
        private String fontID;
        private int fontSize;
        private int overlayWidth;
        private int overlayHeight;
        private String group;
        private TextPaint cachePaint;
        private int realMaxFontSize;
        private boolean changeText;
        private nexTitleInfo(int id, int textLen, String textDesc)
        {
            this.id = id;
            this.textMaxLen = textLen;
            this.textDesc = textDesc;
        }

        /**
         * This method is used to get the ID of title information.
         *
         * @return  Id of title information.
         * @since version 1.7.0
         */
        public int getId() {
            return id;
        }

        /**
         * This method is used to get the title of title information.
         *
         * @return  Title of title information.
         * @since version 1.7.0
         */
        public String getText() {
            return text;
        }

        /**
         * This method is used to set a new title string.
         *
         * @param text New title string.
         * @since version 1.7.0
         */
        public void setText(String text) {
            this.text = text;
        }

        /**
         * This method is used to get the text description of title information.
         *
         * @return  Description of title information.
         * @since version 1.7.0
         */
        public String getTextDesc() {
            return textDesc;
        }

        /**
         * This method is used to get the max length of input title.
         *
         * @return  Max length of input title.
         * @since version 1.7.0
         */
        public int getTextMaxLen() {
            return textMaxLen;
        }

        /**
         * This method is used to get the font ID.
         *
         * @return  Font ID.
         * @since version 1.7.2
         */
        public String getFontID() {
            return fontID;
        }

        /**
         * @brief This method is used to set a new font ID.
         *
         * @param fontID  New font ID.
         * @since version 1.7.2
         */
        public void setFontID(String fontID) {
            this.fontID = fontID;
            changeText = true;
        }

        /**
         * @brief This method is used to get the font size.
         *
         * @return Font size.
         * @since version 1.7.4
         */
        public int getFontSize() {
            return fontSize;
        }

        /**
         * @brief This method is used to set the font size.
         *
         * @param fontSize New font size.
         * @since version 1.7.4
         */
        public void setFontSize(int fontSize) {
            this.fontSize = fontSize;
            changeText = true;
        }

        /**
         * @brief This method is used to get the overlay image width.
         *
         * @return Width of overlay image.
         * @since version 1.7.4
         */
        public int getOverlayWidth() {
            return overlayWidth;
        }

        /**
         * @brief This method is used to get the overlay image height.
         *
         * @return Height of overlay image.
         * @since version 1.7.4
         */
        public int getOverlayHeight() {
            return overlayHeight;
        }

        /**
         * @brief This method is used to get the maximum font size of overlay title.
         *
         * @return Maximum font size of overlay image.
         * @since version 1.7.11
         */
        public int getMaxFontSize(){
            calcFontSize();
            return realMaxFontSize;
        }

        void setOverlayResolution(int width, int height) {
            overlayWidth = width;
            overlayHeight = height;
        }

        /**
         * @brief This method is used to get the text width.
         *
         * @return The actual text width with font and size applied.
         * @since version 1.7.4
         */
        public int getTextWidth(String text) {

            TextPaint paint = new TextPaint();
            paint.setTypeface(nexFont.getTypeface(mAppContext, fontID));
            paint.setTextSize(fontSize);

            float width = paint.measureText(text);

            return (int)width;
        }

        private synchronized void calcFontSize(){

            if( !changeText && realMaxFontSize > 0 ){
                return ;
            }

            if( cachePaint == null){
                cachePaint = new TextPaint();
            }else{
                cachePaint.reset();
            }

            int maxFontSize = (overlayHeight-4);

            cachePaint.setTextSize(maxFontSize);
            cachePaint.setTypeface(nexFont.getTypeface(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),fontID));
            cachePaint.setAntiAlias(true);
            cachePaint.setStyle(Paint.Style.FILL_AND_STROKE);

            while(true){
                int real = (int)cachePaint.getFontSpacing();
                if( real < (overlayHeight-4)){
                    break;
                }else{
                    if( maxFontSize < 10 ){
                        break;
                    }

                    maxFontSize -= 10;
                    cachePaint.setTextSize(maxFontSize);
                }
            }

            realMaxFontSize = maxFontSize;

            changeText = false;
        }

        /**
         * @brief This method is used to get the text height of single line.
         *
         * @return The actual text height with font and size applied.
         * @since version 1.7.13
         */
        public synchronized int getTextHeight(String text) {

            if( cachePaint == null){
                cachePaint = new TextPaint();
            }else{
                cachePaint.reset();
            }
            cachePaint.setTextSize(fontSize);
            cachePaint.setTypeface(nexFont.getTypeface(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),fontID));
            cachePaint.setAntiAlias(true);
            cachePaint.setStyle(Paint.Style.FILL_AND_STROKE);

            return (int)cachePaint.getFontSpacing();
        }

        String getGroup() {
            return group;
        }

        void setGroup(String group) {
            this.group = group;
        }
    }


    private nexOverlayManager(Context appContext, Context resContext){
        mAppContext = appContext;
    }

    /**
     * This method is used to get a nexOverlayManager instance.
     *
     * @param appContext        The application context instance.
     * @param resContext        The context instance that can access the asset.
     * @return  The nexOverlayManager instance.
     * @since version 1.7.0
     */
    public static nexOverlayManager getOverlayManager( Context appContext, Context resContext ) {

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexOverlayManager(appContext,resContext);
        }
        return sSingleton;
    }
/*
    private void resolveOverlay(){
        overlayEntries.clear();

        List<? extends ItemInfo> infos = AssetPackageManager.getInstance().getInstalledItemsByCategory(ItemCategory.overlay);

        for( ItemInfo info : infos )
        {
            if( info.getType() == ItemType.template )
            {
                Overlay t = Overlay.promote(info);
                overlayEntries.add(t);
            }
        }
    }
*/

    private Overlay getAssetOverlay(String AssetId ){
        for( Overlay t : overlayEntries ){
            if( t.packageInfo().assetId().compareTo(AssetId) == 0 ){
                return t;
            }
        }
        return null;
    }
    private Object m_overlayEntryLock = new Object();
    private void resolveOverlay(){
        synchronized (m_overlayEntryLock) {
            overlayEntries.clear();

            List<nexAssetPackageManager.Item> infos = nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledAssetItems(nexAssetPackageManager.Category.overlay);
            for (nexAssetPackageManager.Item info : infos) {
                if (info.type() == nexAssetPackageManager.ItemMethodType.ItemTemplate) {
                    Overlay t = getAssetOverlay(info.packageInfo().assetId());
                    if (t == null) {
                        t = Overlay.promote(info);
                        overlayEntries.add(t);
                    }
                    t.setAspect(info.id());
                }
            }
        }
    }

    boolean updateOverlayTitle(boolean install, nexAssetPackageManager.Item item){
        synchronized (m_overlayEntryLock) {
            boolean update = false;
            if (install) {

                Overlay t = getAssetOverlay(item.packageInfo().assetId());
                if (t == null) {
                    t = Overlay.promote(item);
                    overlayEntries.add(t);
                }
                t.setAspect(item.id());
            } else {
                for (Overlay t : overlayEntries) {
                    if( t.packageInfo().assetId().compareTo(item.packageInfo().assetId()) == 0 ){
                        overlayEntries.remove(t);
                        break;
                    }
                }
            }
            return update;
        }
    }

    /**
     * This method is used to load all installed overlay title templates.
     *
     * @since version 1.7.0
     */
    public void loadOverlay(){
        resolveOverlay();
    }

    /**
     * This method is used to get all installed overlay title templates.
     *
     * @param matchRatio matchRatio flag, if \c TRUE, gets an overlay title templates of matching current ratio.
     *                   Otherwise get all overlay title templates.
     * @return  The list of installed overlay title templates.
     * @since version 1.7.0
     */
    public List<Overlay> getOverlays(boolean matchRatio) {
        Log.d(TAG, "getOverlays total : " + overlayEntries.size() + " match : " + matchRatio);
        synchronized (m_overlayEntryLock) {
            List<Overlay> overlays = new ArrayList<Overlay>();
/**
 float mode = 16f/9f;
 if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16 )
 {
 mode = 9f/16f;
 }
 else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1 )
 {
 mode = 1;
 }
 else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1 )
 {
 mode = 2;
 }
 else if( nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2 )
 {
 mode = 1f/2f;
 }
 */
            float mode = nexApplicationConfig.getAspectRatio();
            for (Overlay over : overlayEntries) {
                if (matchRatio) {
                    if (over.getRatio() == mode )
                        overlays.add(over);
                } else {
                    overlays.add(over);
                }
            }

            //return externalView_overlayEntries = Collections.unmodifiableList(overlays);
            return overlays;
        }
    }

    /**
     * This method returns a list of installed Title Effect. Exception will be thrown when Title Effect is added or deleted.
     * @return List of Title Effect
     * @since 1.7.11
     */
    public List<Overlay> getOverlays( ) {

        Log.d(TAG, "getOverlays total : " + overlayEntries.size());

        if( externalView_overlayEntries == null ){
            externalView_overlayEntries = Collections.unmodifiableList(overlayEntries);
        }
        return externalView_overlayEntries;
    }

    /**
     * This method is used to parse the title information list from an overlay title template.
     *
     * @param overlayId     ID of an overlay title template for parsing.
     * @param titleInfos    Instance of the nexOverlayTitleInfo list for receiving title information.
     * @return  \c TRUE or \c FALSE.
     * @since version 1.7.0
     */
    public boolean parseOverlay(String overlayId, final List<nexTitleInfo> titleInfos ){
        overlayTitle = new nexOverlayTitle();

        if( overlayId != null )
            Log.d(TAG, "parseOverlay with : " + overlayId);
                
        JSONObject jsonObject = null;

        try {
            if(AssetPackageJsonToString(overlayId)!=null) {
                jsonObject = new JSONObject(AssetPackageJsonToString(overlayId));
            }else{
                Log.d(TAG, "json create failed" + " overlayid is null");
                return false;
            }
        } catch (JSONException e) {
            e.printStackTrace();

            Log.d(TAG, "json create failed" + e.getMessage());
            return false;
        }

        String ret = overlayTitle.parseOverlayAsset(jsonObject, new nexOverlayTitle.TitleInfoListener() {
            @Override
            public void OnTitleInfoListener(int id, String font, int font_size, String text, int maxLen, String desc, String group, int overlay_width, int overlay_height) {

                if( titleInfos != null ) {
                    nexTitleInfo info = new nexTitleInfo(id, maxLen, desc);
                    info.setText(text);
                    info.setFontID(font);
                    info.setFontSize(font_size);
                    info.setGroup(group);
                    info.setOverlayResolution(overlay_width, overlay_height);
                    titleInfos.add(info);
                }
            }
        });

        if( ret != null )
        {
            Log.d(TAG, "json parse failed" + ret);
            titleInfos.clear();
            return false;
        }
        return true;
    }

    /**
     * This method is used to apply an overlay title template to a project with title information.
     *
     * @param project       Instance of project.
     * @param overlayId     ID of overlay title template for applying.
     * @param titleInfos    Instance of nexOverlayTitleInfo list for applying title information.
     * @return  \c TRUE or \c FALSE.
     * @since version 1.7.0
     */
    public boolean applyOverlayToProjectById(nexProject project , String overlayId, List<nexTitleInfo> titleInfos){

        Log.d(TAG, "applyOverlayToProjectById with : " + overlayId);
        parseOverlay(overlayId, null);

        if( overlayTitle == null )
        {
            Log.d(TAG, "Overlay data did not exist");
            return false;
        }

        String ret = overlayTitle.applyOverlayAsset(mAppContext, project, titleInfos);
        if( ret != null ) {
            Log.d(TAG, "Overlay apply failed");
            return false;
        }

        return true;
    }

    /**
     * This method is used to apply an overlay title template to a project with title information.
     *
     * @param project       Instance of project.
     * @param overlay       ID of overlay title template for applying.
     * @return  \c TRUE or \c FALSE.
     * @since version 1.7.0
     */
    public boolean applyOverlayToProjectById(nexProject project, Overlay overlay){

        Log.d(TAG, "applyOverlayToProjectById with : " + overlay.getId());

        overlayTitle = overlay.getOverlayTitle();

        if( overlayTitle == null ) {
            parseOverlay(overlay.getId(), null);

            if( overlayTitle == null ) {
                Log.d(TAG, "Overlay data did not exist");
                return false;
            }

            overlay.setOverlayTitle(overlayTitle);
        }

        String ret = overlayTitle.applyOverlayAsset(mAppContext, project, null);
        if( ret != null ) {
            Log.d(TAG, "Overlay apply failed");
            return false;
        }

        return true;
    }

    /**
     * This method is used to get nexOverlayInfoTitle list of applied overlay title.
     *
     * @return              Applied nexOverlayInfoTitle list.
     * @since version 2.0.15
     */
    public List<nexOverlayInfoTitle> getOverlayInfoTitle(){
        if( overlayTitle == null )
        {
            Log.d(TAG, "Overlay data did not exist");
            return null;
        }

        List<nexOverlayInfoTitle> titles = overlayTitle.getOverlayInfoTitle();
        return titles;
    }


    /**
     * This method is used to get nexOverlayInfoTitle instance of current position and time.
     *
     * @param time          time.
     * @param x             x position.
     * @param y             y position.
     * @return              nexOverlayInfoTitle instance.
     * @since version 2.0.15
     */
    public nexOverlayInfoTitle getOverlayInfoTitle(int time, float x, float y){
        if( overlayTitle == null )
        {
            Log.d(TAG, "Overlay data did not exist");
            return null;
        }

        nexOverlayInfoTitle title = overlayTitle.getOverlayInfoTitle(time, x, y);
        return title;
    }

    /**
     * @brief This method is used to clear overlay title templates on a project.
     *
     * @param project       Instance of project.
     * @return \c TRUE or \c FALSE.
     * @since version 1.7.0
     */
    public boolean clearOverlayToProject(nexProject project){
        if( overlayTitle == null )
        {
            Log.d(TAG, "Overlay data did not exist");
            return false;
        }

        String ret = overlayTitle.clearOverlayAsset(project);
        if( ret != null ) {
            Log.d(TAG, "Overlay apply failed");
            return false;
        }

        return true;
    }

    /**
     * @brief This uninstalls an Overlay.
     *
     * This API is same as nexAssetPackageManager.uninstallPackageById(String).
     * @param overlayId nexOverlayManager.nexOverlay.getID() or nexAssetPackageManager.Item.id() of Overlay Category.
     * @since 1.7.3
     */
    public void uninstallPackageById(String overlayId){
        nexAssetPackageManager.getAssetPackageManager(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).uninstallPackageById(overlayId);
    }


    private String AssetPackageJsonToString(String ItemId){
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
}
