/******************************************************************************
 * File Name        : nexColorEffect.java
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
import android.graphics.ColorMatrix;
import android.util.Log;

import com.nexstreaming.app.common.thememath.NexTheme_Math;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.editorwrapper.LookUpTable;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * This class defines the color effect information possible for a clip such as brightness, saturation, contrast, and tint color. 
 *
 * By using preset values( NONE, ALIEN_INVASION, BLACK_AND_WHITE, COOL, DEEP_BLUE, PINK, RED_ALERT, SEPIA, SUNNY, PURPLE, 
 * ORANGE, STRONG_ORANGE, SPRING, SUMMER, FALL, ROUGE, PASTEL, NOIR), color effect settings can be used more efficiently.
 * 
 * 
 * <p>For Example :</p>
         {@code    mEngine.getProject().getClip(0,true).setColorEffect( nexColorEffect.getPresetList().get(position) );
             mEngine.getProject().getClip(1,true).setColorEffect( nexColorEffect.COOL);
         }
 * @since version 1.0.1
 */
public final class nexColorEffect implements Cloneable {

    public static final nexColorEffect NONE				= new nexColorEffect("NONE", 0, 0, 0, 0, 0);
    public static final nexColorEffect ALIEN_INVASION		= new nexColorEffect("ALIEN_INVASION", 0.12f, -0.06f, -0.3f, 0xFF147014, 0);
    public static final nexColorEffect BLACK_AND_WHITE		= new nexColorEffect("BLACK_AND_WHITE", 0f, 0f, -1f, 0, 0);
    public static final nexColorEffect COOL				= new nexColorEffect("COOL", 0.12f, -0.12f, -0.3f, 0xFF144270, 0);
    public static final nexColorEffect DEEP_BLUE			= new nexColorEffect("DEEP_BLUE", -0.2f, -0.3f, -0.6f, 0xFF0033FF, 0);
    public static final nexColorEffect PINK				= new nexColorEffect("PINK", 0.1f, -0.3f, -0.6f, 0xFF9C4F4F, 0);
    public static final nexColorEffect RED_ALERT			= new nexColorEffect("RED_ALERT", -0.30f, -0.19f, -1f, 0xFFFF0000, 0);
    public static final nexColorEffect SEPIA				= new nexColorEffect("SEPIA", 0.12f, -0.12f, -0.3f, 0xFF704214, 0);
    public static final nexColorEffect SUNNY				= new nexColorEffect("SUNNY", 0.08f, -0.06f, -0.3f, 0xFFCCAA55, 0);
    public static final nexColorEffect PURPLE				= new nexColorEffect("PURPLE", 0.08f, -0.06f, -0.3f, 0xFFAA55CC, 0);
    public static final nexColorEffect ORANGE				= new nexColorEffect("ORANGE", 0.08f, -0.06f, -0.35f, 0xFFFFBB00, 0);
    public static final nexColorEffect STRONG_ORANGE		= new nexColorEffect("STRONG_ORANGE", 0.08f, -0.06f, -0.5f, 0xFFFFBB00, 0);
    public static final nexColorEffect SPRING				= new nexColorEffect("SPRING", 0.08f, -0.06f, -0.3f, 0xFFAACC55, 0);
    public static final nexColorEffect SUMMER				= new nexColorEffect("SUMMER", 0.08f, -0.06f, -0.5f, 0xFFAAFF00, 0);
    public static final nexColorEffect FALL				= new nexColorEffect("FALL", 0.08f, -0.06f, -0.5f, 0xFF00FFAA, 0);
    public static final nexColorEffect ROUGE				= new nexColorEffect("ROUGE", 0.08f, -0.06f, -0.6f, 0xFFFF5555, 0);
    public static final nexColorEffect PASTEL				= new nexColorEffect("PASTEL", 0.08f, -0.06f, -0.5f, 0xFF555555, 0);
    public static final nexColorEffect NOIR				= new nexColorEffect("NOIR", -0.25f, 0.6f, -1.0f, 0xFF776655, 0);

    private static List<nexColorEffect> list = new ArrayList<nexColorEffect>();;
    private static List<nexColorEffect> listCollect = null;

    protected static nexColorEffect clone(nexColorEffect src) {
        //Log.d("clone", "clone work./nexColorEffect");
        nexColorEffect object = null;
        try {
            object = (nexColorEffect)src.clone();

            object.list = src.list;
            object.listCollect = src.listCollect;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    /**
     * This method gets a list of the color effect presets already created.
     *
     * <p>Example code:</p>
     *     {@code     List<nexColorEffect> list = nexColorEffect.getPresetList();
                for( nexColorEffect ce: list ){
                    items.add(ce.getPresetName());
                }
            }
     * @return  A list of the color presets, including:
     * <ul> 
     * <li>{@link #NONE}, {@link #ALIEN_INVASION}, {@link #BLACK_AND_WHITE}, {@link #COOL}, {@link #DEEP_BLUE}, {@link #PINK},
     * <li>{@link #RED_ALERT}, {@link #SEPIA}, {@link #SUNNY}, {@link #PURPLE}, {@link #ORANGE}, {@link #STRONG_ORANGE},
     * <li>{@link #SPRING}, {@link #SUMMER}, {@link #FALL}, {@link #ROUGE}, {@link #PASTEL}, {@link #NOIR}
     * </ul>
     * @see #getPresetName()
     * @since version 1.0.1
     */
    public static List<nexColorEffect> getPresetList() {
        //resolveList(false);
        checkUpdate();
        if( listCollect==null )
            listCollect = Collections.unmodifiableList(list);
        return listCollect;
    }

    /**
     * @brief This gets the LUT Color Effect.
     * When an Asset filter ID is entered, this gets the matching nexColorEffect.
     * @param assetId Asset filter ID
     * @return nexColorEffect
     * @since 1.7.0
     */
    public static nexColorEffect getLutColorEffect(String assetId){
        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (lut == null) {
            return null;
            //new nexColorEffect(ent.getName(), 0, 0, 0, 0, ent.getId(), ent.getKineMasterID()
        }
        LookUpTable.LUTEntry ent = lut.getLutByItemId(assetId);
        if( ent == null ){
            return null;
        }

        if( ent.isHidden() ){
            return new nexColorEffect(ent.getName(), 0, 0, 0, 0, 0, ent.getIdx(), ent.getKineMasterID(),ent.getAssetItemId());
        }

        //resolveList(false);
        checkUpdate();
        for( nexColorEffect effect : list ){
            if( effect.getLUTId() == ent.getIdx() ){
                return effect;
            }
        }
        return null;
    }

    /**
     *
     * @param lut_item_id
     * @return
     * @since 2.0.0
     */
    public static int removeCustomLUT(String lut_item_id){

        LookUpTable manager = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (manager == null) {
            return 0;
        }
        return manager.removeCustomLUT(lut_item_id);
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public static int removeAllCustomLUT(){

        LookUpTable manager = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (manager == null) {
            return 0;
        }
        manager.removeAllCustomLUT();
        return 1;
    }

    /**
     *
     * @param lut_item_id
     * @return
     * @since 2.0.9
     */
    public static boolean existCustomLUT(String lut_item_id){
        LookUpTable manager = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (manager == null) {
            return false;
        }
        return manager.existCustomLUT(lut_item_id);
    }


    /**
     *
     * @param lut_item_id
     * @param data
     * @param offset
     * @param length
     * @param option
     * @return
     * @since 2.0.0
     */
    public static int addCustomLUT(String lut_item_id, byte[] data, int offset, int length, int option){

        LookUpTable manager = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (manager == null) {
            return 0;
        }

        LookUpTable.LUT lut = manager.new LUT(data, offset, length, option);

        if(lut != null){

            try{
                return manager.addCustomLUT(lut_item_id, lut);
            }
            catch(Exception e){

                e.printStackTrace();
            }
        }

        return 0;
    }

    /**
     *
     * @param lut_item_id
     * @return
     * @since 2.0.0
     */
    public static int getLUTUID(String lut_item_id){

        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (lut == null) {
            return 0;
        }

        return lut.getUID(lut_item_id);
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public static String[] getLutIds(){
        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if (lut == null) {
            return new String[0];
            //new nexColorEffect(ent.getName(), 0, 0, 0, 0, ent.getId(), ent.getKineMasterID()
        }

        return lut.getLutItemIds(false);
    }

    /**
     * @brief An API for update after an LUT Asset is installed. 
     * @since 1.7.0
     */
    public static void updatePluginLut(){
        setNeedUpdate();
        /*
        LookUpTable lut = LookUpTable.getLookUpTable();
        lut.resolveTable();
        resolveList(true);
        */
    }

    /**
     * clear LUT cache.
     * One LUT color filter used about one Mbyte memory.
     * When engine has stopped, if you want release  Lut cache for save memory then use this API.
     * Do not call Api when engine running. (playing, exporting, seeking)
     * @since 2.0.0
     */
    public static void cleanLutCache(){
        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if( lut != null ){
            lut.cleanCache();
        }
    }

    static nexColorEffect[] getInternalPresetList() {
        return new nexColorEffect[]{
                ALIEN_INVASION, BLACK_AND_WHITE, COOL, PINK, SEPIA,
                SUNNY, PURPLE, ORANGE, STRONG_ORANGE, SPRING, SUMMER, FALL, ROUGE, PASTEL, NOIR};
    }

    private static void resolveList(boolean forceResolve){
        boolean reset = forceResolve;

        if( list.size() == 0 )
            reset = true;

        if( reset ) {
            list.clear();
            list.addAll(Arrays.asList(new nexColorEffect[]{
                NONE, ALIEN_INVASION, BLACK_AND_WHITE, COOL, DEEP_BLUE, PINK, RED_ALERT, SEPIA,
                    SUNNY, PURPLE, ORANGE, STRONG_ORANGE, SPRING, SUMMER, FALL, ROUGE, PASTEL, NOIR}));

            LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
            if (lut == null) {
                return;
            }
            for (LookUpTable.LUTEntry ent : lut.getLUTEntrys()) {
                if( !ent.isHidden() ) {
                    list.add(new nexColorEffect(ent.getName(), 0, 0, 0, 0, 0, ent.getIdx(), ent.getKineMasterID(),ent.getAssetItemId()));
                }
            }
        }
    }

    private static boolean needUpdate = true;

    static void setNeedUpdate(){
        needUpdate = true;
        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        if( lut != null ){
            lut.setNeedUpdate();
        }
    }

    static private void checkUpdate(){
        if( needUpdate ){
            needUpdate = false;
            resolveList(true);
        }else{
            resolveList(false);
        }
    }

    private ColorMatrix colorMatrix;
    private final float brightness;
    private final float contrast;
    private final float saturation;
    private final float hue;
    private final int tintColor;
    private final String presetName;
    private boolean lut_enabled_ = false;
    private int lut_resource_id_ = 0;
    private final String kineMasterID;
    private final String assetItemID;

    private nexColorEffect(String presetName, float brightness, float contrast, float saturation, int tintColor, float hue) {
        this.brightness = brightness;
        this.contrast = contrast;
        this.saturation = saturation;
        this.tintColor = tintColor;
        this.hue = hue;
        this.presetName = presetName;
        this.lut_enabled_ = false;
        kineMasterID = null;
        assetItemID = null;
    }

    /**
     * This method creates a color effect based on the values of brightness, contrast, saturation and tint color passed with this method.  
     * 
     * <p>Example code:</p>
     *     {@code
                public static final nexColorEffect ALIEN_INVASION = new nexColorEffect("ALIEN_INVASION", 0.12f, -0.06f, -0.3f, 0xFF147014);
            }
     * @param brightness The brightness value as a <tt>float(-1 ~ 1)</tt>.
     * @param contrast The contrast value as a <tt>float(-1 ~ 1)</tt>.
     * @param saturation The saturation value as a <tt>float(-1 ~ 1)</tt>.
     * @param tintColor The tint color value as an <tt>integer(ARGB color format)</tt>.
     * 
     * @since version 1.1.0
     */
    public nexColorEffect(float brightness, float contrast, float saturation, int tintColor, float hue) {
        this.brightness = brightness;
        this.contrast = contrast;
        this.saturation = saturation;
        this.tintColor = tintColor;
        this.hue = hue;
        this.presetName = null;
        this.lut_enabled_ = false;
        kineMasterID = null;
        assetItemID = null;
    }

    private nexColorEffect( String presetName, float brightness, float contrast, float saturation, int tintColor, float hue, int lut_resource_id, String kineMasterID , String assetItemID){

        this.brightness = brightness;
        this.contrast = contrast;
        this.saturation = saturation;
        this.tintColor = tintColor;
        this.hue = hue;
        this.presetName = presetName;
        this.lut_enabled_ = true;
        this.lut_resource_id_ = lut_resource_id;
        this.kineMasterID = kineMasterID;
        this.assetItemID = assetItemID;
    }

    /**
     * This method gets the ColorMatrix of the Color effect that is not LUT.
     * <p>Example code:</p>
     *     {@code ImageView iv = (ImageView)convertView.findViewById(R.id.imageView_color_effect_apply);
        iv.setColorFilter(new ColorMatrixColorFilter(nexColorEffect.COOL.getColorMatrix()));
        }
     * @return android ColorMatrix
     * @since version 1.5.23
     */
    public ColorMatrix getColorMatrix() {

        if( colorMatrix==null ) {
            colorMatrix = NexTheme_Math.colorAdjust(brightness, contrast, saturation, tintColor);
        }
        return colorMatrix;
    }

    /**
     * This method gets the name of a preset color effect. 
     * 
     * <p>Example code:</p>
     *     {@code     for( nexColorEffect ce: list ){
                    items.add(ce.getPresetName());
                }
            }
     * @return The name of the color effect preset, as a <tt>String</tt>. 
     * 
     * @see #getPresetList()
     * @since version 1.1.0
     */
    public String getPresetName() {
        return presetName;
    }
   
    
    /**
     * This method gets the contrast of a clip. 
     *
     * <p>Example code:</p>
     *     {@code if( mColorEffect==null )
                return m_Contrast + (int)(255*mColorEffect.getContrast());
            }
     * @return The contrast value, as a <tt>float</tt>. 
     * @since version 1.1.0
     */
    public float getContrast() {
        return contrast;
    }

    /**
     * This method gets the brightness of a clip.
     *
     * <p>Example code:</p>
     *     {@code if( mColorEffect==null )
                return m_Brightness + (int)(255*mColorEffect.getBrightness());
            }
     * @return The brightness value, as a <tt>float</tt>.
     * @since version 1.1.0
     */
    public float getBrightness() {
        return brightness;
    }

    /**
     * This method gets the saturation of a clip. 
     *
     * <p>Example code:</p>
     *     {@code if( mColorEffect==null )
                return m_Saturation + (int)(255*mColorEffect.getSaturation());
            }
     * @return The saturation value, as a <tt>float</tt>.
     * @since version  1.1.0
     */
    public float getSaturation() {
        return saturation;
    }

    public float getHue(){

        return hue;
    }

    /**
     * This method gets the tint color of a clip. 
     *
     * <p>Example code:</p>
     *     {@code if( mColorEffect==null )
                return  mColorEffect.getTintColor();
            }
     * @return The tint color value, as an <tt>integer</tt>.
     * @since version 1.1.0
     */
    public int getTintColor() {
        return tintColor;
    }

    /**
     * This method passes the LUT ID if the ColorEffect has LUT enabled.
     *
     * @return The ID number if LUT is enabled; otherwise, 0.
     * @since version 1.5.23
     */
    public int getLUTId(){
        if( !lut_enabled_ ) {
            return 0;
        }
        return lut_resource_id_;
    }

    /**
     * This method is a preview of a color effect that uses the LUT filter.
     * 
     * Other color effects that are non-LUT can be previewd using the {@link #getColorMatrix()}.
     * <p>Example code:</p>
     *     {@code Bitmap thumbnail = Bitmap.createBitmap(...);
        nexColorEffect effect = nexColorEffect.getPresetList().get(20);
        int lut_id = effect.getLUTId();
        if(  lut_id != 0 ) {
            Bitmap bm = nexColorEffect.applyColorEffectOnBitmap(thumbnail, effect);
        }
    }

     * @param bgBitmap The original image. 
     * @param colorEffect The Color effect with the LUT filter.
     * @return A LUT-filtered image; otherwise, the original image. 
     * @since version 1.5.23
     */
    static public Bitmap applyColorEffectOnBitmap(Bitmap bgBitmap, nexColorEffect colorEffect ){
        int id = colorEffect.getLUTId();
        if( id == 0 ){
            return bgBitmap;
        }
        LookUpTable lut = LookUpTable.getLookUpTable(KineMasterSingleTon.getApplicationInstance().getApplicationContext());
        return lut.applyColorLUTOnBitmap(bgBitmap, id);
    }

    String getKineMasterID(){
        if( lut_enabled_ ) {
            return kineMasterID;
        }
        return presetName;
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public String getAssetItemID(){
        return assetItemID;
    }

    /**
     * This method compares whether the color effect options set to two different methods are equal. 
     * 
     * @param o  <tt>nexColorEffect</tt>  
     *
     * @return <tt>TRUE</tt> if the setting options are equal, otherwise <tt>FALSE</tt>. 
     * @since version 1.1.0
     */
    @Override
    public boolean equals(Object o) {
        if( !(o instanceof nexColorEffect) ) {
            return false;
        }
        nexColorEffect cf = ((nexColorEffect)o);

        return (cf.brightness == brightness &&
                cf.contrast   == contrast   &&
                cf.saturation == saturation &&
                cf.hue        == hue &&
                cf.tintColor  == tintColor);
    }
 
    /** 
     * This method returns the unique value of a <tt>nexColorEffect</tt> object. 
     *
     * @return The unique value of a <tt>nexColorEffect</tt> object.
     * @since version 1.1.0
     */
    @Override
    public int hashCode() {
        int hash = (int)(brightness * 255f);
        hash = hash*71 + (int)(contrast * 255f);
        hash = hash*479 + (int)(saturation * 255f);
        hash = hash*977 + (int)(hue*180.0f);
        hash = hash*1877 + (int)(tintColor);
        return hash;
    }

    nexSaveDataFormat.nexColorEffectOf getSaveData(){
        nexSaveDataFormat.nexColorEffectOf data = new nexSaveDataFormat.nexColorEffectOf();
        data.brightness = brightness;
        data.contrast = contrast;
        data.saturation = saturation;
        data.hue = hue;
        data.tintColor = tintColor;
        data.presetName = presetName;
        data.lut_enabled_ = lut_enabled_;
        data.lut_resource_id_ = lut_resource_id_;
        data.kineMasterID = kineMasterID;
        data.assetItemID = assetItemID;
        return data;
    }

    nexColorEffect(nexSaveDataFormat.nexColorEffectOf data){
        brightness = data.brightness;
        contrast = data.contrast;
        saturation = data.saturation;
        hue = data.hue;
        tintColor = data.tintColor;
        presetName = data.presetName;
        lut_enabled_ = data.lut_enabled_;
        lut_resource_id_ = data.lut_resource_id_;
        kineMasterID = data.kineMasterID;
        assetItemID = data.assetItemID;
    }
}
