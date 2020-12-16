/******************************************************************************
 * File Name        : nexFont.java
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
import android.graphics.Typeface;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.util.StringUtil;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.fonts.Font;
import com.nexstreaming.kminternal.kinemaster.fonts.FontCollection;
import com.nexstreaming.kminternal.kinemaster.fonts.FontManager;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * This class allows to bring a built-in font of the NexEditor&tm;SDK as a <tt>Typeface</tt> instance. 
 *
 * <p>Example code :</p>
 {@code
   TextPaint p = cachedTextPaint;
    p.setTextSize(textSize);
    p.setAntiAlias(true);
    p.setColor(0xFFFFFFFF);
    Typeface userFont = null;

    String[] fontIds = nexFont.getFontIds();
    if( fontIds.length > 0 ) {
        userFont = nexFont.getTypeface(mContext,fontIds[0]);
    }

    if( userFont!=null ) {
        p.setTypeface(userFont);
    }
 }
 */
public final class nexFont {
    static final private String LOG_TAG="nexFont";

//    private static List<String> fontIDs = new ArrayList<>();
//    private static List<Font> assetFonts = new ArrayList<>();
    private static List<nexFont> list = new ArrayList<nexFont>();
    private Font font;

//    private String id;
    private String sample;
    private boolean builtin;
    private boolean system;

    private nexFont(boolean builtin, Font font, String sample, boolean system) {
        this.builtin = builtin;
        this.font = font;
        this.sample = sample;
        this.system = system;
    }

    /**
     * This gets the ID of a font.
     * @return The ID of a font.
     * @since 1.7.0
     */
    public String getId(){
        return font.getId();
    }

    /**
     * This is the text for the sample image of a font. 
     *
     * @return The text for the sample image of a font. 
     * @see #getSampleImage(Context)
     * @since 1.7.0
     */
    public String getSampleText(){
        return sample;
    }

    /**
     * This notifies whether or not the font is built-in the application. 
     * @return \c TRUE - built-in font , \c FALSE - Asset Store font
     * @since 1.7.0
     */
    public boolean isBuiltinFont(){
        return builtin;
    }

    /**
     * This notifies whether or not it is an Android system font.
     * @return true - system font , false - user font
     * @since 1.7.0
     */
    public boolean isSystemFont(){
        return system;
    }

    /**
     * This gets the thumbnail image of a font. 
     * A 1000 X 100 pixel size image.
     * @param appContext getApplicationContext()
     * @return A 1000 x 100 size font sample image. 
     * @since 1.7.0
     */
    public Bitmap getSampleImage(Context appContext){
        return font.getSampleImage(appContext);
    }

    /**
     * This gets the Typeface of a font.
     * @return The Typeface.
     * @since 1.7.0
     */
    public Typeface getTypeFace(){
        return FontManager.getInstance().getTypeface(getId());
    }

    /**
     * This gets the list of all built-in and downloaded fonts from the Asset Store to nexFont. 
     * @return The font list. 
     * @since 1.7.0
     */
    public static List<nexFont> getPresetList(){
        if(list.size() == 0 ) {
            List<FontCollection> fontCollections = FontManager.getInstance().getBuiltinFontCollections();
            for (FontCollection fc : fontCollections) {
                for (Font font : fc.getFonts()) {
                    //fontIDs.add(font.getId());
                    boolean system = font.getId().startsWith("system");
                    list.add(new nexFont(true, font, font.getName(null), system));
                }
            }

            List<? extends ItemInfo> fonts = AssetPackageManager.getInstance().getInstalledItemsByCategory(ItemCategory.font);
            for (ItemInfo font : fonts) {
                if( font.isHidden() ){
                    continue;
                }

                String collectionId = String.valueOf((int) font.getAssetPackage().getAssetSubCategory().getSubCategoryId());
                if (collectionId == null) {
                    collectionId = "asset";
                }
                String name = font.getSampleText();
                if (name == null || name.trim().length() < 1) {
                    name = StringUtil.getLocalizedString(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), font.getLabel());
                }

                if (name == null) {
                    name = font.getId();
                }

                Log.d(LOG_TAG, "asset font id=" + font.getId() + ", loacalPath=" + getLocalPath(font));
                Font f = new Font(font.getId(), collectionId, new File(getLocalPath(font)), name);

                boolean builtin = !font.getPackageURI().contains(AssetLocalInstallDB.getInstalledAssetPath());

                list.add(new nexFont(builtin, f, name, false));
            }
        }
        return list;
    }

    /**
     *
     * @param fontId
     * @return
     * @since 1.7.57
     */
    public static nexFont getFont(String fontId){
        //getPresetList();
        checkUpdate();
        for( nexFont ret : list ){
            if( ret.getId().compareTo(fontId) == 0 ){
                return ret;
            }
        }
        return null;
    }

    private static String getLocalPath(ItemInfo item) {
        AssetPackageReader reader = null;
        try {
            reader = AssetPackageReader.readerForPackageURI(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), item.getPackageURI(), item.getId());
            return reader.getLocalPath(item.getFilePath()).getAbsolutePath();
        } catch (Exception e) {
            Log.e(LOG_TAG, e.getMessage(), e);
        } finally {
            try {
                if (reader != null) {
                    reader.close();
                }
            } catch (IOException e) {
                if (LL.D) Log.e(LOG_TAG, e.getMessage(), e);
            }
        }
        return null;
    }

    /**
     * This method gets the IDs of built-in fonts of NexEditor&tm; SDK. 
     *
     * The ID is for internal use only, therefore it may differ with the Android font names. 
     *
     * @return The list of built-in font IDs.
     * @since version 1.5.23
     */
    public static String[] getFontIds(){
        //getPresetList();
        checkUpdate();
        String[] ret = new String[list.size()];
        for( int i = 0; i < ret.length ; i++ ){
            ret[i] = list.get(i).getId();
        }
        return ret;
    }

    /**
     * This method gets a built-in SDK font as a <tt>Typeface</tt> instance.
     * Also, this method can change all graphic fonts on Android to <tt>Typeface</tt>. 
     * @param appContext The context that has the built-in SDK font. 
     * @param fontId The ID of built-in SDK font which can be retrieved by <tt>getFontIds()</tt>. 
     * @return The font as in <tt>Typeface</tt>. 
     * @since version 1.5.23
     */
    public static Typeface getTypeface( Context appContext , String fontId){
        return FontManager.getInstance().getTypeface(fontId);
    }

    /**
     * This method updates the Fonts.
     * If new fonts are downloaded from the Asset Store or an existing font was deleted, the developer should use this method to update the internal list.
     * @see #getPresetList()
     * @since 1.7.0
     */
    public static void reload(){
        list.clear();
        getPresetList();
    }

    static boolean isLoadedFont(String fontId){
        if( FontManager.getInstance().isInstalled(fontId) )
            return true;

        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(fontId);
        if( info == null )
            return false;

        if( info.getCategory() == ItemCategory.font  )
            return true;
        return false;
    }

    /**
     * This method clears all cached Builtin Fonts.
     * @since 1.7.7
     */
    public static void clearBuiltinFontsCache(){
        FontManager.getInstance().clearBuiltinFonts();
    }

    private static boolean s_update;

    static void needUpdate(){
        s_update = true;
    }

    static void checkUpdate(){
        if( s_update){
            s_update = false;
            reload();
        }
    }
}
