/******************************************************************************
 * File Name        : nexEffectLibrary.java
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
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemParameterDef;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemParameterType;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDef;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDefReader;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;


/**
 * This class sets clip effects on clips. 
 * To run this class properly, call {@link com.nexstreaming.nexeditorsdk.nexApplicationConfig#init init@endlink}(android.content.Context, String) first.
 * This operates using a singleton, therefore there is no need to create an instance.  Refer to the sample code below.
 * <p>Example code :</p>
 *     nexEffectLibrary fxLib = nexEffectLibrary.getEffectLibrary(this);
 * @since version 1.0.0
 * @see {@link com.nexstreaming.nexeditorsdk.nexApplicationConfig#waitForLoading waitForLoading@endlink}(android.content.Context, Runnable)
 */
public final class nexEffectLibrary {
    private static nexEffectLibrary sSingleton = null;
    private final Context mAppContext;

    private nexEffectLibrary(Context appContext){
        mAppContext = appContext;
    }

    /**
     *  This method gets the library handle for clip effects, transition effects, and theme packages needed for the editing.
     *
     * <p>Example code:</p>
     *     {@code     private nexEffectLibrary mfxLib;
                mfxLib = nexEffectLibrary.getEffectLibrary(context);
            }
     * @param context Context of the activity.
     * @return <tt>nexEffectLibrary</tt>
     * @since version 1.0.0
     */
    public static nexEffectLibrary getEffectLibrary( Context context ) {
        Context appContext = context.getApplicationContext();
        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexEffectLibrary(appContext);
        }
        return sSingleton;
    }

    /**
     *  This method gets the registered theme packages as an ArrayList.
     *
     * <p>Example code:</p>
     *     {@code
                mThemeTab = mfxLib.getThemesEx();
            }
     * @return The ArrayList of <tt>nexTheme</tt>.
     * @since version 1.0.0
     * @see #getTransitionEffects()
     * @see #getClipEffects()
     */
    @Deprecated
    public ArrayList<nexTheme> getThemesEx() {
        ArrayList<nexTheme> listThemes = new ArrayList<nexTheme>();
        return  listThemes;
    }

    /**
     *  This method gets registered theme packages in sequence.
     * <p>Example code :</p>
     *     {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
            nexTheme[] allThemes = fxlib.getThemes();
            for( nexTheme t: allThemes ) {
                 Log.i(LOG_TAG,"Name =" + t.getName(getBaseContext()));
            }
     *     }

     * @return The sequence of <tt>nexTheme</tt>.
     * @since version 1.0.0
     * @see #getTransitionEffects()
     * @see #getClipEffects()
     */
    @Deprecated
    public nexTheme[] getThemes() {
        nexTheme[] ret = null;
        ret = new nexTheme[0];
        return ret;
    }

    /**
     *  This method gets the basic transition effects that are not part of a theme package.
     *
     * <p>Example code:</p>
     *     {@code
                mTransitionTab = mfxLib.getTransitionEffectsEx();
            }
     * @return An array of <tt>nexTransitionEffect</tt> effects.
     * @since version 1.0.0
     */
    public ArrayList<nexTransitionEffect> getTransitionEffectsEx() {
        ArrayList<nexTransitionEffect> listTransEffects = new ArrayList<nexTransitionEffect>();

        for (ItemInfo tmpEffect : AssetPackageManager.getInstance(mAppContext).getInstalledItemsByCategory(ItemCategory.transition)) {
            if( !tmpEffect.isHidden() ) {
                listTransEffects.add(new nexTransitionEffect(tmpEffect.getId()));
            }
        }
            //assetInfos.get(0)
        return listTransEffects;
    }

    /**
     *  This method gets a basic transition effects that are not part of a theme package.
     * <p>Example code :</p>
     *     {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
            nexTransitionEffect[] transitionEffects = fxlib.getTransitionEffects();
            for (nexTransitionEffect e : transitionEffects) {
                Log.i(LOG_TAG,"Name =" + e.getName(getBaseContext()));
            }
     *     }
     * @return An array of <tt>nexTransitionEffect</tt>.
     * @since version 1.0.0
     * @see #getThemes()
     * @see #getClipEffects()

     */
    public nexTransitionEffect[] getTransitionEffects(){
        nexTransitionEffect[] ret = null;
        List<? extends ItemInfo> Infos =  AssetPackageManager.getInstance(mAppContext).getInstalledItemsByCategory(ItemCategory.transition);
        int count = 0;
        for(ItemInfo info :  Infos){
            if( !info.isHidden() ){
                count++;
            }
        }

        ret = new nexTransitionEffect[count];
        count = 0;
        for(ItemInfo info :  Infos){
            if( !info.isHidden() ){
                ret[count] = new nexTransitionEffect(info.getId());
                ret[count].itemMethodType = nexAssetPackageManager.getMethodType(info.getType());
                count++;
            }
        }
        return ret;
    }

    /**
     *  This method gets a basic clip effects that are not part of a theme package.
     * <p>Example code :</p>
     *     {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
            nexClipEffect[] clipEffects = fxlib.getClipEffects();
            for (nexClipEffect e : clipEffects) {
                Log.i(LOG_TAG,"Name =" + e.getName(getBaseContext()));
            }
     *     }
     * @return An array of <tt>nexClipEffect</tt>.
     * @since version 1.0.0
     * @see #getThemes()
     * @see #getTransitionEffects()
     */
    public nexClipEffect[] getClipEffects(){
        nexClipEffect[] ret = null;
        List<? extends ItemInfo> Infos =  AssetPackageManager.getInstance(mAppContext).getInstalledItemsByCategory(ItemCategory.effect);
        int count = 0;
        for(ItemInfo info :  Infos){
            if( !info.isHidden() ){
                count++;
            }
        }
        ret = new nexClipEffect[count];
        count = 0;
        for(ItemInfo info :  Infos){
            if( !info.isHidden() ){
                ret[count] = new nexClipEffect(info.getId());
                ret[count].itemMethodType = nexAssetPackageManager.getMethodType(info.getType());
                count++;
            }
        }
        return ret;
    }

    /**
     * @brief This method gets Overlay filters.
     * The Overlay filter is inserted in the Overlay track of a project.
     * In other words, it is an Overlay transition.
     * @return Overlay filter
     * @since 1.7.0
     */
    public nexOverlayFilter[] getOverlayFilters(){
        nexOverlayFilter[] ret = null;

            List<String> collect = new ArrayList<>();
            List<? extends ItemInfo> filters =  AssetPackageManager.getInstance(mAppContext).getInstalledItemsByCategory(ItemCategory.effect);
            for(ItemInfo info : filters ){
                if( !info.isHidden()) {
                    if (info.getType() == ItemType.renderitem) 
                    {
                        collect.add(info.getId());
                    }
                }
            }

            ret = new nexOverlayFilter[collect.size()];
            for(int i = 0 ; i < ret.length ; i++ ){
                ret[i] = new nexOverlayFilter(collect.get(i));
            }
        return ret;
    }

    public nexOverlayFilter[] getOverlayEffect(){
        nexOverlayFilter[] ret = null;

            List<String> collect = new ArrayList<>();
            List<? extends ItemInfo> filters =  AssetPackageManager.getInstance(mAppContext).getInstalledItemsByCategory(ItemCategory.effect);
            for(ItemInfo info : filters ){
                if( !info.isHidden()) {

                    collect.add(info.getId());
                }
            }

            ret = new nexOverlayFilter[collect.size()];
            for(int i = 0 ; i < ret.length ; i++ ){
                ret[i] = new nexOverlayFilter(collect.get(i));
            }
        return ret;
    }

    /**
     *  This method gets a theme package using a theme ID if the ID is known.
     *
     * <p>Example code:</p>
     *     {@code
                theme = fxLib.findThemeById(getThemeId());
            }
     * @param id  The theme ID as a <tt>String</tt>.  
     * @return <tt>nexTheme</tt>; <tt>null</tt> if no theme was found.
     * @since version 1.0.0
     */
    @Deprecated
    public nexTheme findThemeById(String id) {
        return null;
    }

    /**
     *  This method gets a clip effect using a clip effect ID if the ID is known.
     *
     * <p>Example code:</p>
     *     {@code
                Effect effect = EditorGlobal.getEditor().getEffectLibrary().findEffectById(options.getEffectID());
            }
     * @param id The clip effect ID as a <tt>String</tt>.
     * @return <tt>nexClipEffect</tt>; <tt>null</tt> if no clip effect was found.
     * @since version 1.0.0
     * @see #findTransitionEffectById(String)
     */
    public nexClipEffect findClipEffectById(String id){
        ItemInfo info = AssetPackageManager.getInstance(mAppContext).getInstalledItemById(id);
        if( info == null ){
            return null;
        }

        if( info.getCategory() == ItemCategory.effect ){
            nexClipEffect effect = new nexClipEffect(id);
            effect.itemMethodType = nexAssetPackageManager.getMethodType(info.getType());
            return effect;
        }
        return null;
    }

    /**
     *  This method get a transition effect with a transition effect ID if the ID is known.
     *
     * <p>Example code:</p>
     *     {@code
                Effect transitionEffect = findTransitionEffectById( id );
            }
     * @param id The transition effect ID as a <tt>String</tt>. 
     * @return <tt>nexTransitionEffect</tt>
     * @since version 1.0.0
     * @see #findClipEffectById(String)
     */
    public nexTransitionEffect findTransitionEffectById(String id){
        ItemInfo info = AssetPackageManager.getInstance(mAppContext).getInstalledItemById(id);
        if( info == null ){
            return null;
        }

        if( info.getCategory() == ItemCategory.transition ){
            nexTransitionEffect effect =  new nexTransitionEffect(id);
            effect.itemMethodType = nexAssetPackageManager.getMethodType(info.getType());
            return effect;
        }
        return null;
    }

    /**
     * @brief If the known ID is an overlay filter, this method creates and returns an instance of nexOverlayFiler.
     * @param id overlay filter ID.
     * @return null -- ID does not exist or it is not an Overlayfilter ID.
     * @since 1.7.0
     */
    public nexOverlayFilter findOverlayFilterById(String id){
        ItemInfo info = AssetPackageManager.getInstance(mAppContext).getInstalledItemById(id);
        if( info == null ){
            return null;
        }

        if( info.getType() == ItemType.renderitem ){
            return new nexOverlayFilter(id);
        }
        return null;
    }

    public nexOverlayFilter findOverlayEffectById(String id){
        ItemInfo info = AssetPackageManager.getInstance(mAppContext).getInstalledItemById(id);
        if( info == null ){
            return null;
        }

        return new nexOverlayFilter(id);
    }

    /**
     * This method gets the effect options to set to clip effects made by the developer. 
     * 
     * @param context  The Android context.
     * @param id    An ID of an effect.  
     * @return The instance of <tt>nexEffectOptions</tt>.
     * 
     * <p>Example code :</p>
     *     {@code  nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
    nexEffectOptions options = fxlib.getEffectOptions(this, transitionid);
    if( options != null ) {
        int selectCount = 0;
        int lastIndex = 0;
        if (options.getSelectOptions().size() > 0) {
            selectCount = options.getSelectOptions().get(0).getItems().length;
        }
        for (int i = 0; i < project.getTotalClipCount(true); i++) {
            project.getClip(i, true).getTransitionEffect().setTransitionEffect(transitionid);
            if (selectCount > 0) {
                options.getSelectOptions().get(0).setSelectIndex(lastIndex);
                lastIndex++;
                if (lastIndex >= selectCount) lastIndex = 0;

                project.getClip(i, true).getTransitionEffect().updateEffectOptions(options, true);
            }
        }
    }
    }
     * @see {@link com.nexstreaming.nexeditorsdk.nexEffect#updateEffectOptions updateEffectOptions}(nexEffectOptions, boolean)
     * @see {@link com.nexstreaming.nexeditorsdk.nexEffect#getTitleOptions getTitleOptions}(nexEffectOptions)
     * @since version 1.0.1
     */
    public nexEffectOptions getEffectOptions(Context context, String id){

        ItemInfo info = AssetPackageManager.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getInstalledItemById(id);
        if( info != null ) {
            int type = nexEffect.kEFFECT_NONE;

            XMLItemDef itemDef = null;
            if (info.getCategory() == ItemCategory.transition) {
                type = nexEffect.kEFFECT_TRANSITION_USER;
            } else if (info.getCategory() == ItemCategory.effect) {
                type = nexEffect.kEFFECT_CLIP_USER;
            } else if (info.getCategory() == ItemCategory.filter) {
                if (info.getType() == ItemType.renderitem) {
                    type = nexEffect.kEFFECT_OVERLAY_FILTER;
                }
            }

            nexEffectOptions eo = new nexEffectOptions(id, type);

            try {
                itemDef = XMLItemDefReader.getItemDef(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), id);
            } catch (XmlPullParserException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }

            if (itemDef == null) {

            } else {
                List<ItemParameterDef> itemDefs = itemDef.getParameterDefinitions();
                for (ItemParameterDef def : itemDefs) {
                    Map<String, Map<String, String>> strings = def.getStrings();
                    String label = null;
                    if (strings != null) {
                        Map<String, String> labels = strings.get("label");
                        if (labels != null) {
                            label = labels.get("en");
                        }
                    }

                    if (def.getType() == ItemParameterType.RGBA || def.getType() == ItemParameterType.RGB) {
                        if (label == null) {
                            label = "Color";
                        }
                        eo.addColorOpt(def.getId(), label, def.getDefaultValue());
                    } else if (def.getType() == ItemParameterType.TEXT) {
                        if( def.getId().endsWith("_font")){
                            if(label == null ){
                                label = "Typeface";
                            }
                            eo.addTypefaceOpt(def.getId(),label,def.getDefaultValue());
                        }else {
                            if (label == null) {
                                label = "Text";
                            }
                            eo.addTextOpt(def.getId(), label, def.isMultiline() ? 2 : 1);
                        }
                    } else if (def.getType() == ItemParameterType.CHOICE) {
                        if (label == null) {
                            label = "Choice";
                        }
                        List<ItemParameterDef.Option> options = def.getOptions();

                        int select = 0;
                        String[] items = new String[options.size()];
                        String[] values = new String[options.size()];


                        for (int i = 0; i < options.size(); i++) {
                            Map<String, Map<String, String>> optStrings = options.get(i).getStrings();
                            String name = null;
                            if (optStrings != null) {
                                Map<String, String> names = optStrings.get("label");
                                if (names != null) {
                                    name = names.get("en");
                                }
                            }

                            if (options.get(i).getValue().compareTo(def.getDefaultValue()) == 0) {
                                select = i;
                            }

                            if (name == null) {
                                name = new String("item" + i);
                            }
                            items[i] = name;
                            values[i] = options.get(i).getValue();
                        }
                        eo.addSelectOpt(def.getId(), label, items, values, select);
                    } else if (def.getType() == ItemParameterType.RANGE) {
                        if (label == null) {
                            label = "Range";
                        }
                        int val = Integer.parseInt(def.getDefaultValue());
                        eo.addRangeOpt(def.getId(), label, val, def.getMinimumValue(), def.getMaximumValue());
                    } else if (def.getType() == ItemParameterType.SWITCH) {
                        if (label == null) {
                            label = "Switch";
                        }
                        boolean on = false;

                        if (def.getOnValue().compareTo(def.getDefaultValue()) == 0) {
                            on = true;
                        }
                        eo.addSwitchOpt(def.getId(), label, on);
                    } else if( def.getType() == ItemParameterType.TYPEFACE){
                        if(label == null ){
                            label = "Typeface";
                        }
                        eo.addTypefaceOpt(def.getId(),label,def.getDefaultValue());
                    }
                }
            }
            eo.makeGroup();
            return eo;
        }
        return null;
    }

    boolean checkEffectID(String id){

            boolean b = AssetPackageManager.getInstance(mAppContext).doesAssetPackageContainItemsOfCategory(id,ItemCategory.transition );
            if( !b ){
                b = AssetPackageManager.getInstance(mAppContext).doesAssetPackageContainItemsOfCategory(id,ItemCategory.effect );
            }
            return b;
    }

    @Deprecated
    public static abstract class OnInstallPluginEffectPackageAsyncListener {
        public abstract void onPackageLoaded( int discount );
        public abstract void onComplete( int failedCount , String [] errorMessages);
    }

    @Deprecated
    public boolean installPluginEffectPackageAsync(String [] pluginKmtFileNames, final OnInstallPluginEffectPackageAsyncListener listener){

        return true;
    }

    @Deprecated
    static public String getPluginDirPath(){
        return EditorGlobal.getPluginDirectory().getAbsolutePath();
    }
}
