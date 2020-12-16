/******************************************************************************
 * File Name        : nexOverlayKineMasterText.java
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
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;

import java.io.File;
import java.text.Bidi;

import static java.lang.Math.max;

/**
 * This is a layer text class which is compatible with the KineMaster&trade;&nbsp;. 
 * Use this class to display an overlay text easily. 
 * For compatibility issues with KineMaster&trade;&nbsp;, there won't be any more expansions.
 *
 * <p>Example code :</p>
 {@code
  nexOverlayKineMasterText builtinText = new nexOverlayKineMasterText(getApplicationContext(),"nexOverlayStandardText", 50);
  builtinText.setTextColor(0xffff0000);
  String[] fontIds = nexFont.getFontIds();
  if( fontIds.length > 0 ) {
      builtinText.setFontId(fontIds[0]);
  }
  nexOverlayItem overlayItem = new nexOverlayItem(builtinText,640,180,0,6000);
  project.addOverlay(overlayItem);
 }

 * @since version 1.5.23
 */
@Deprecated
public final class nexOverlayKineMasterText extends nexOverlayImage {
    private static final String LOG_TAG = "nexOverlayKineMasterText";
    //    private static final int SHADOW_SIZE = 8;
    private static final int TEXT_WRAP_WIDTH = 1280;
    private static final int MAX_TEX_SIZE = 2000;
    private static final int DEFAULT_BACKGROUND_COLOR = 0x88000000;
    private String layerText = "";
    private float textSize = 50;
    private String fontId;
    private int textColor = 0xFFFFFFFF;

    private boolean bUserFontMode;
    private boolean enableBackground = false;
    private boolean extendBackground = false;
    private int backgroundColor = DEFAULT_BACKGROUND_COLOR;

    private boolean enableShadow = true;
    private int shadowColor = 0xFF000000;
    private float shadowRadius = 5;
    private float shadowDx = 3;
    private float shadowDy = 3;

    private boolean enableGlow = false;
    private int glowColor = 0xAAFFFFAA;
    private float glowRadius = 8;
    private int glowType = 0;

    private boolean enableOutline = false;
    private int outlineColor = 0xFFCCCCCC;
    private float outlineWidth = 1;

    private boolean enableGradient = false;
    private int[] gradientColors;

    private transient int width;
    private transient int height;
    private transient boolean validDimensions;
    private transient StaticLayout textLayout;
//    private transient ScaleRange scaleRange;
    private float actualScale = 1;

//    private transient Bitmap renderBitmap;
    private Layout.Alignment textAlign = Layout.Alignment.ALIGN_CENTER;
    private int TextId = 0;
    private static int number = 1;

    private Context mContext;

    final boolean serialKMText = true;

    /**
     * This is a constructor of <tt>nexOverlayKineMasterText</tt>. 
     *
     * @param context The application context with the built-in SDK font. 
     * 
     * @since version 1.5.23
     */
    @Deprecated
    public nexOverlayKineMasterText(Context context){
        super("nexOverlayStandardText");
        mContext = context;
        TextId = number;
        number++;
    }

    /**
     * This is a constructor of <tt>nexOverlayKineMasterText</tt>. 
     *
     * @param context  The application context with the built-in SDK font. 
     * @param Text     The text to display on the overlay item.
     * @param textSize The size of the text, which follows the size of class <tt>Paint</tt>.
     *
     * @since version 1.5.23
     */
    @Deprecated
    public nexOverlayKineMasterText(Context context, String Text, int textSize) {
        super("nexOverlayStandardText");
        mContext = context;
        layerText = Text;
        this.textSize = textSize;
        TextId = number;
        number++;
    }

    /**
     * This method gets the width value when a text is made into a Bitmap. 
     *
     * @return The overlay width of the text.
     * @since version 1.5.23
     */
    @Override
    public int getWidth() {
        calcDimensions();
        return width;
    }

    /**
     * This method gets the height value when a text is made into a Bitmap. 
     * 
     * @return The overlay height of the text.
     * @since version 1.5.23
     */
    @Override
    public int getHeight() {
        calcDimensions();
        return height;
    }

    @Override
    protected Bitmap getUserBitmap() {
        return makeTextBitmap();
    }

    @Override
    protected String getUserBitmapID() {
        return "KineMasterText-"+TextId;
    }

    /**
     * This method gets the text as a <tt>string</tt> to print on the overlay item.
     *
     * @return The input text. 
     * @since version 1.5.23
     */
    @Deprecated
    public String getText() {
        return layerText==null?"":layerText;
    }


    /**
     * This method sets the text to print on the overlay item. 
     *
     * @param text The text to set. 
     * @since version 1.5.23
     */
    @Deprecated
    public void setText(String text) {
        layerText = text;
        validDimensions = false;
        mUpdate = true;
    }

    /**
     * This method sets the size of the text to print on the overlay item.
     *  
     * @param size The size of text, which follows the size of class <tt>Paint</tt>.
     * @since version 1.5.23
     */
    @Deprecated
    public void setTextSize(float size) {
        textSize = size;
        validDimensions = false;
        mUpdate = true;
    }

    /**
     * This method sets the color of the text to print on the overlay item. 
     *
     * @param argb The color of the text as in <tt>integer</tt>. (e.g. 0xffff0000 ->Red)
     *
     * @since version 1.5.23
     */
    @Deprecated
    public void setTextColor(int argb) {
        if( textColor !=  argb ) {
            textColor = argb;
            mUpdate = true;
        }
    }

    /**
     * This method gets the size of the text printed on the current overlay item. 
     * 
     * @return The size of text. 
     * @since version 1.5.23
     */
    @Deprecated
    public float getTextSize(){
        return textSize;
    }

    /**
     * This method gets the color of the text printed on the current overlay item. 
     * @return The color of the text in argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public int getTextColor() {
        return textColor;
    }

    /**
     * This method gets the color of the glow set to the current overlay item. 
     * @return The color of the glow in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public int getGlowColor(){
        return getGlowColor(true);
    }

    int getGlowColor(boolean org){
        if( !org ){
            if (enableGlow) {
                return glowColor;
            } else {
                return 0;
            }
        }
        return glowColor;
    }

    /**
     * This method sets the color of the glow set to the current overlay item. 
     * @param argbColor The color of the glow in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public void setGlowColor(int argbColor){
        if( glowColor != argbColor) {
            glowColor = argbColor;
            if( enableGlow ){
                mUpdate = true;
            }
        }
    }

    /**
     * This method gets the color of the outline set to the current overlay item. 
     * @return The color of the outline in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public int getOutlineColor(){
        return getOutlineColor(true);
    }

    int getOutlineColor(boolean org){
        if( !org ){
            if (enableOutline) {
                return outlineColor;
            } else {
                return 0;
            }
        }
        return outlineColor;
    }
    /**
     * This method sets the color of the outline of the overlay item. 
     * @param argbColor The color of the outline in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public void setOutlineColor(int argbColor){
        if( outlineColor != argbColor) {
            outlineColor = argbColor;
            if( enableOutline ){
                mUpdate = true;
            }
        }
    }

    /**
     * This method gets the color of the shadow set to the current overlay item. 
     * @return The color of the shadow in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public int getShadowColor(){
        return getShadowColor(true);
    }

    int getShadowColor(boolean org){
        if( !org ){
            if (enableShadow) {
                return shadowColor;
            } else {
                return 0;
            }
        }
        return shadowColor;
    }

    /**
     * This method sets the color of the shadow of the overlay item. 
     * @param argbColor The color of the shadow in the argb form. (e.g. 0xffff0000 ->Red)
     * @since version 1.5.23
     */
    @Deprecated
    public void setShadowColor(int argbColor){
        if( shadowColor != argbColor) {
            shadowColor = argbColor;
            if( enableShadow ){
                mUpdate = true;
            }
        }
    }

    /**
     * This method indicates whether or not the shadow effect is enabled on the current overlay item. 
     * @return <tt>True</tt> if the effect is on; otherwise <tt>false</tt>.
     * @since version 1.5.23
     */
    @Deprecated
    public boolean isEnableShadow(){
        return enableShadow;
    }

    /**
     * This method enables the shadow effect on the overlay item. 
     * @param enable <tt>True</tt> if the effect is enabled; otherwise <tt>false</tt>. 
     * @since version 1.5.23
     */
    @Deprecated
    public void EnableShadow(boolean enable){
        if( enableShadow != enable ) {
            enableShadow = enable;
            mUpdate = true;
        }
    }

    /**
     * This method indicates whether or not the glow effect is enabled on the current overlay item. 
     * @return <tt>True</tt> if the effect is on; otherwise <tt>false</tt>.
     * @since version 1.5.23
     */
    @Deprecated
    public boolean isEnableGlow(){
        return enableGlow;
    }

    /**
     * This method enables the glow effect on the overlay item. 
     * @param enable <tt>True</tt> if the effect is enabled; otherwise <tt>false</tt>. 
     * @since version 1.5.23
     */
    @Deprecated
    public void EnableGlow(boolean enable){
        if( enableGlow != enable ) {
            enableGlow = enable;
            mUpdate = true;
        }
    }

    /**
     * This method indicates whether or not the outline effect is enabled on the current overlay item. 
     * @return <tt>True</tt> if the effect is on; otherwise <tt>false</tt>.
     * @since version 1.5.23
     */
    @Deprecated
    public boolean isEnableOutline(){
        return enableOutline;
    }

    /**
     * This method enables the outline effect on the overlay item. 
     * @param enable <tt>True</tt> if the effect is enabled; otherwise <tt>false</tt>. 
     * @since version 1.5.23
     */
    @Deprecated
    public void EnableOutline(boolean enable){
        if( enableOutline != enable ) {
            enableOutline = enable;
            mUpdate = true;
        }
    }

    /**
     * This method sets the built-in SDK font. 
     * If this method is not set, the device's Android default font will be used. 
     * @param fontId The ID of built-in SDK font {@link nexFont} or .ttf file path.
     * <p>Example code : 1</p>
     * {@code
     * File ffile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+"gallery"+File.separator+"font.ttf");
        if( ffile.isFile() ) {
            mDefaultFont = ffile.getAbsolutePath();
        }

        mOverlayKMText = new nexOverlayKineMasterText(getApplicationContext(),"", 50);
        mOverlayKMText.setFontId(mDefaultFont);
        }
     * <p>Example code : 2</p>
     * {@code
     * String[] ids = nexFont.getFontIds();
        mOverlayKMText = new nexOverlayKineMasterText(getApplicationContext(),"", 50);
        if(ids.length > 0){
            mOverlayKMText.setFontId(ids[0]);
        }
        }
     * @since version 1.5.23
     */
    @Deprecated
    public void setFontId(String fontId){
        if( fontId == null ){
            bUserFontMode = false;
            this.fontId = null;
            validDimensions = false;
            mUpdate = true;
            calcDimensions();
            return;
        }

        if( this.fontId != fontId ) {
            if (fontId.charAt(0) == '/' ){
                int index = fontId.lastIndexOf('.');
                if( index != -1 && fontId.substring(index).compareToIgnoreCase(".ttf") == 0 ) {
                    File file = new File(fontId);
                    if (file.isFile()) {
                        bUserFontMode = true;
                        this.fontId = fontId;
                        validDimensions = false;
                        mUpdate = true;
                        calcDimensions();
                    }
                }
            } else {
                if (nexFont.isLoadedFont(fontId)) {
                    bUserFontMode = false;
                    this.fontId = fontId;
                    validDimensions = false;
                    mUpdate = true;
                    calcDimensions();
                }
            }
        }
    }

    /**
     * This method gets the ID of the built-in SDK font set to the current overlay item. 
     * @return The ID of the set built-in SDK font {@link nexFont} or .ttf file path.
     * @since version 1.5.23
     */
    @Deprecated
    public String getFontId(){
        return getFontIdInternal(true);
    }

    String getFontIdInternal(boolean all){
        if( all ) {
            return fontId;
        }else {
            if (!bUserFontMode) {
                return fontId;
            }
        }
        return null;
    }

    private synchronized Bitmap makeTextBitmap(){
        calcDimensions();

        int shadowPadding = getShadowPadding();
/*
        if( scaleRange==null )
            scaleRange = new ScaleRange();
        getScaleRange(scaleRange);
        float maxTexScale = MAX_TEX_SIZE/Math.max(width,height);
        float actualScale = Math.min(Math.min(scaleRange.weightedAverageScale * 2, scaleRange.maxScale), maxTexScale);
        Log.d(LOG_TAG, "TextLayer resolution upscale factor: " + actualScale);
*/
        int scaled_w = (int)(width*actualScale);
        int scaled_h = (int)(height*actualScale);

        //if you pass these values without check, it can cause crash with IllegalArgumentException
        if(scaled_w <= 0 || scaled_h <= 0)
            return null;

        Bitmap bitmap = Bitmap.createBitmap(scaled_w, scaled_h, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        canvas.scale(actualScale,actualScale);
        canvas.translate(shadowPadding, shadowPadding);

//        Paint layerPaint = null;

//        if( layerPaint==null ) {
//            layerPaint = new Paint();
//        }
//
//        layerPaint.reset();
//        layerPaint.setAntiAlias(true);

        TextPaint textPaint = textLayout.getPaint();

//        Paint p = getPaint();
//        String s = getText();
////        Rect bounds = new Rect();
////        p.getTextBounds(s,0,s.length(),bounds);
//        p.setColor(textColor);
//        if( enableShadow ) {
//            p.setShadowLayer(shadowRadius, shadowDx, shadowDy, shadowColor);
//        }
        textPaint.setTextAlign(Paint.Align.LEFT);
        textPaint.setStyle(Paint.Style.FILL);
        if(enableShadow) {
            textPaint.setMaskFilter(new BlurMaskFilter(shadowRadius/actualScale, BlurMaskFilter.Blur.NORMAL));
            textPaint.setColor(shadowColor);
//            canvas.saveLayer(0, 0, bitmap.getWidth(), bitmap.getHeight(), layerPaint, Canvas.ALL_SAVE_FLAG);
            canvas.save();
            canvas.translate(shadowDx,shadowDy);
            textLayout.draw(canvas);
            canvas.restore();
        }

        if(enableGlow) {
            textPaint.setMaskFilter(new BlurMaskFilter(glowRadius/actualScale, BlurMaskFilter.Blur.OUTER));
            textPaint.setColor(glowColor);
//            layerPaint.setColorFilter(new LightingColorFilter(glowColor,0));
//            canvas.saveLayer(0, 0, bitmap.getWidth(), bitmap.getHeight(), layerPaint, Canvas.ALL_SAVE_FLAG);
            textLayout.draw(canvas);
//            canvas.restore();
        }

        textPaint.setMaskFilter(null);
        textPaint.setColor(textColor);
//        layerPaint.setColorFilter(new LightingColorFilter(textColor,0));
//        canvas.saveLayer(0, 0, bitmap.getWidth(), bitmap.getHeight(), layerPaint, Canvas.ALL_SAVE_FLAG);
        textLayout.draw(canvas);
//        canvas.restore();

        if(enableOutline) {
            textLayout.getPaint().setStyle(Paint.Style.STROKE);
            textPaint.setColor(outlineColor);
            textPaint.setStrokeWidth(outlineWidth/actualScale);
//            layerPaint.setMaskFilter(null);
//            layerPaint.setColorFilter(new LightingColorFilter(outlineColor,0));
//            canvas.saveLayer(0, 0, bitmap.getWidth(), bitmap.getHeight(), layerPaint, Canvas.ALL_SAVE_FLAG);
            textLayout.draw(canvas);
//            canvas.restore();
        }

//        if(enableGlow){
//            Paint glowPaint = getPaint();
//            glowPaint.setTextSize(actualTextSize);
//            glowPaint.setColor(textColor);
//            glowPaint.setShadowLayer(glowRadius, 0, 0, glowColor);
//            if( userFont!=null ) {
//                glowPaint.setTypeface(userFont);
//            }
//            canvas.drawText(s,bounds.left+SHADOW_SIZE,-bounds.top+SHADOW_SIZE, glowPaint);
//        }
//
//        canvas.drawText(s,bounds.left+SHADOW_SIZE,-bounds.top+SHADOW_SIZE,p);
//        if(enableOutline){
//            Paint strokePaint = getPaint();
//            strokePaint.setTextSize(actualTextSize);
//            strokePaint.setColor(outlineColor);
//            strokePaint.setStyle(Paint.Style.STROKE);
//            strokePaint.setStrokeWidth(outlineWidth);
//            if( userFont!=null ) {
//                strokePaint.setTypeface(userFont);
//            }
//            canvas.drawText(s,bounds.left+SHADOW_SIZE,-bounds.top+SHADOW_SIZE, strokePaint);
//        }

        return bitmap;
    }


    private TextPaint cachedTextPaint;

    private TextPaint getPaint() {
        if( cachedTextPaint==null ) {
            cachedTextPaint = new TextPaint();
        } else {
            cachedTextPaint.reset();
        }
        TextPaint p = cachedTextPaint;
        p.setTextSize(textSize);
        p.setAntiAlias(true);
        p.setColor(0xFFFFFFFF);
        p.setStrokeWidth(outlineWidth);
        p.setStyle(Paint.Style.FILL_AND_STROKE);
        Typeface userFont = null;

        if (fontId != null) {
            if( bUserFontMode ){
                userFont = Typeface.createFromFile(fontId);
            }else {
                userFont = nexFont.getTypeface(mContext, fontId);
            }
        }
        if( userFont!=null ) {
            p.setTypeface(userFont);
        }

        return p;
    }

    private int getShadowPadding() {
        return (int)Math.ceil(   Math.max(outlineWidth, Math.max(glowRadius, (shadowRadius + Math.max(Math.abs(shadowDx), Math.abs(shadowDy))))) );
    }

    private synchronized void calcDimensions() {
        if( validDimensions && textLayout!=null )
            return;

        TextPaint p = getPaint();

        String s = getText();

        float spaceingmult = 1;
        float spacingadd = 0;
        boolean includepad = true;
        int overlay_width = nexApplicationConfig.getAspectProfile().getWidth();
/*
        if( nexApplicationConfig.getAspectRatioMode() != nexApplicationConfig.kAspectRatio_Mode_16v9 ){
            overlay_width = 720;
        }
*/
        int outerWidth = (int)Math.min(overlay_width, StaticLayout.getDesiredWidth(s,p)+1);
        Bidi bidi = new Bidi(s, Bidi.DIRECTION_DEFAULT_LEFT_TO_RIGHT);
        if(textAlign.equals(Layout.Alignment.ALIGN_CENTER) || bidi.baseIsLeftToRight()) {
            textLayout = new StaticLayout(s, 0, layerText.length(), p, outerWidth, textAlign, spaceingmult, spacingadd, includepad);
        }else if (!bidi.baseIsLeftToRight() && textAlign.equals(Layout.Alignment.ALIGN_NORMAL)){
            textLayout = new StaticLayout(s,0,layerText.length(),p,outerWidth, Layout.Alignment.ALIGN_OPPOSITE,spaceingmult,spacingadd,includepad);
        }else if (!bidi.baseIsLeftToRight() && textAlign.equals(Layout.Alignment.ALIGN_OPPOSITE)){
            textLayout = new StaticLayout(s,0,layerText.length(),p,outerWidth, Layout.Alignment.ALIGN_NORMAL,spaceingmult,spacingadd,includepad);
        }

//        Rect bounds = new Rect();
//        p.getTextBounds(s,0,s.length(),bounds);
        int shadowPadding = getShadowPadding();
        width = max(1,textLayout.getWidth()) + shadowPadding*2;
        height = max(1,textLayout.getHeight()) + shadowPadding*2;
//        Log.d(LOG_TAG,"calcDimensions : " + bounds);
        validDimensions = true;
    }
}
