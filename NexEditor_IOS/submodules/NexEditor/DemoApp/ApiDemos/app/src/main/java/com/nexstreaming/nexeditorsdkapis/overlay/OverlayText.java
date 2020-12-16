/******************************************************************************
 * File Name        : OverlayText.java
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

package com.nexstreaming.nexeditorsdkapis.overlay;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;

import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import static java.lang.Math.max;

/**
 * Created by jeongwook.yoon on 2015-06-04.
 */
public class OverlayText implements nexOverlayImage.runTimeMakeBitMap {
    private String mText;
    private int mId;
    private int mOverlayWidth=1280;
    private int mOverlayHeight=720;

 //   private static final int TEXT_WRAP_WIDTH = 1280;
 //   private static final int MAX_TEX_SIZE = 2000;
 //   private String layerText = "";
    private float textSize = 50;
    private int textColor = 0xFFFF00FF;

    private transient int width;
    private transient int height;
    private transient boolean validDimensions;
    private transient StaticLayout textLayout;
    private float actualScale = 1;

    private TextGlow glow = new TextGlow();
    private TextShadow shadow = new TextShadow();
    private TextOutline outline = new TextOutline();

//    private transient Bitmap renderBitmap;

    public OverlayText(Context context, String text, int id){
        mText = text;
        mId = id;
    }


    public OverlayText(Context context, String text, int id, int overlayWidth, int overlayHegit, float actualScale){
        mText = text;
        mId = id;
        mOverlayWidth = overlayWidth;
        mOverlayHeight = overlayHegit;
        this.actualScale = actualScale;
    }

    @Override
    public boolean isAniMate() {
        return false;
    }

    @Override
    public int getBitmapID() {
        return mId;
    }

    @Override
    public Bitmap makeBitmap() {
        return makeTextBitmap();
    }

    private Bitmap makeTextBitmapSimple(){
        Bitmap bitmap = Bitmap.createBitmap(mOverlayWidth, mOverlayHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setTextAlign(Paint.Align.CENTER);
        paint.setTextSize(textSize);
        paint.setStyle(Paint.Style.FILL_AND_STROKE);
        paint.setColor(Color.WHITE);
        canvas.drawColor(Color.TRANSPARENT);
        canvas.drawText(mText, mOverlayWidth/2, mOverlayHeight/2, paint);

        return bitmap;
    }

    private Bitmap makeTextBitmap(){
        calcDimensions();

        int shadowPadding = getShadowPadding();

        Bitmap bitmap = Bitmap.createBitmap((int)(width*actualScale), (int)(height*actualScale), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        canvas.scale(actualScale,actualScale);
        canvas.translate(shadowPadding, shadowPadding);

        TextPaint textPaint = textLayout.getPaint();

        textPaint.setStyle(Paint.Style.FILL);
        if(glow.isEnableGlow()) {
            textPaint.setMaskFilter(new BlurMaskFilter(shadow.getShadowRadius()/actualScale, BlurMaskFilter.Blur.NORMAL));
            textPaint.setColor(shadow.getShadowColor());
            canvas.save();
            canvas.translate(shadow.getShadowDx(), shadow.getShadowDy());
            textLayout.draw(canvas);
            canvas.restore();
        }

        if(glow.isEnableGlow()) {
            textPaint.setMaskFilter(new BlurMaskFilter(glow.getGlowRadius()/actualScale, BlurMaskFilter.Blur.OUTER));
            textPaint.setColor(glow.getGlowColor());
            textLayout.draw(canvas);
        }

        textPaint.setMaskFilter(null);
        textPaint.setColor(textColor);
        textLayout.draw(canvas);

        if(outline.isEnableOutline()) {
            textLayout.getPaint().setStyle(Paint.Style.STROKE);
            textPaint.setColor(outline.getOutlineColor());
            textPaint.setStrokeWidth(outline.getOutlineWidth()/actualScale);
            textLayout.draw(canvas);
        }
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
        p.setStrokeWidth(outline.getOutlineWidth());
        p.setStyle(Paint.Style.FILL_AND_STROKE);
        return p;
    }

    private int getShadowPadding() {
        return (int)Math.ceil(   Math.max(outline.getOutlineWidth(), Math.max(glow.getGlowRadius(), (shadow.getShadowRadius() + Math.max(Math.abs(shadow.getShadowDx()), Math.abs(shadow.getShadowDy()))))) );
    }

    private void calcDimensions() {
        if( validDimensions && textLayout!=null )
            return;

        TextPaint p = getPaint();

        String s = mText;

        float spaceingmult = 1;
        float spacingadd = 0;
        boolean includepad = true;
        int outerWidth = (int)Math.min(mOverlayWidth, StaticLayout.getDesiredWidth(s, p)+1);
        textLayout = new StaticLayout(s,0,s.length(),p,outerWidth, Layout.Alignment.ALIGN_CENTER,spaceingmult,spacingadd,includepad);

        int shadowPadding = getShadowPadding();
        width = max(1,textLayout.getWidth()) + shadowPadding*2;
        height = max(1,textLayout.getHeight()) + shadowPadding*2;

        validDimensions = true;
    }


    public class TextShadow {
        private boolean enableShadow = true;
        private int shadowColor = 0xFF000000;
        private float shadowRadius = 5;
        private float shadowDx = 3;
        private float shadowDy = 3;

        public void setEnableShadow(boolean enableShadow) {
            this.enableShadow = enableShadow;
        }

        public void setShadowColor(int shadowColor) {
            this.shadowColor = shadowColor;
        }

        public void setShadowRadius(float shadowRadius) {
            this.shadowRadius = shadowRadius;
        }

        public void setShadowDx(float shadowDx) {
            this.shadowDx = shadowDx;
        }

        public void setShadowDy(float shadowDy) {
            this.shadowDy = shadowDy;
        }

        public boolean isEnableShadow() {
            return enableShadow;
        }

        public int getShadowColor() {
            return shadowColor;
        }

        public float getShadowRadius() {
            return shadowRadius;
        }

        public float getShadowDx() {
            return shadowDx;
        }

        public float getShadowDy() {
            return shadowDy;
        }
    }

    public class TextGlow {
        private boolean enableGlow = false;
        private int glowColor = 0xAAFFFFAA;
        private float glowRadius = 8;

        public void setEnableGlow(boolean enableGlow) {
            this.enableGlow = enableGlow;
        }

        public void setGlowColor(int glowColor) {
            this.glowColor = glowColor;
        }

        public void setGlowRadius(float glowRadius) {
            this.glowRadius = glowRadius;
        }

        public boolean isEnableGlow() {
            return enableGlow;
        }

        public int getGlowColor() {
            return glowColor;
        }

        public float getGlowRadius() {
            return glowRadius;
        }
    }


    public class TextOutline {
        private boolean enableOutline = false;
        private int outlineColor = 0xFFCCCCCC;
        private float outlineWidth = 1;

        public void setEnableOutline(boolean enableOutline) {
            this.enableOutline = enableOutline;
        }

        public void setOutlineColor(int outlineColor) {
            this.outlineColor = outlineColor;
        }

        public void setOutlineWidth(float outlineWidth) {
            this.outlineWidth = outlineWidth;
        }

        public boolean isEnableOutline() {
            return enableOutline;
        }

        public int getOutlineColor() {
            return outlineColor;
        }

        public float getOutlineWidth() {
            return outlineWidth;
        }
    }
}


