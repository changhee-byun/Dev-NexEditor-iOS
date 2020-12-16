package com.nexstreaming.app.common.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;

import java.text.Bidi;

import static java.lang.Math.max;

public class utilityTextOverlay implements nexOverlayImage.runTimeMakeBitMap {

    private Context mContext;

    private String mText;

    private int mId;
    private int mOverlayWidth;
    private int mOverlayHeight;

    private transient int width;
    private transient int height;
    private transient boolean validDimensions;
    private transient StaticLayout textLayout;
    private float actualScale;

    public utilityTextOverlay(String text, int id){
        mText = text;
        mId = id;
    }

    public utilityTextOverlay(Context context, String text, int overlayWidth, int overlayHeight, float actualScale){
        mContext = context;
        mText = text;
        mOverlayWidth = overlayWidth;
        mOverlayHeight = overlayHeight;
        this.actualScale = actualScale;
    }

    public void setTextOverlayId(int id) {
        mId = id;
    }

    private class Text {
        private float textSize;
        private Paint.Align textAlign;
        private int textColor;
        private String textFont;
        private int drawAlign;

        public Text(float textSize, int textColor, Paint.Align textAlign, String textFont, int drawAlign) {
            this.textSize = textSize;
            this.textColor = textColor;
            this.textAlign = textAlign;
            this.textFont = textFont;
            this.drawAlign = drawAlign;
        }
    }

    private Text text = null;

    public void setText(float textSize, int textColor, Paint.Align textAlign, String textFont, int drawAlign) {
        text = new Text(textSize, textColor, textAlign, textFont, drawAlign);
    }

    private class TextOutline {
        private boolean enableOutline;
        private int outlineColor;
        private float outlineWidth;

        public TextOutline(boolean enableOutline, int outlineColor, float outlineWidth) {
            this.enableOutline = enableOutline;
            this.outlineColor = outlineColor;
            this.outlineWidth = outlineWidth;
        }
    }

    private TextOutline textOutline = null;

    public void setTextOutline(boolean enableOutline, int outlineColor, float outlineWidth) {
        textOutline = new TextOutline(enableOutline, outlineColor, outlineWidth);
    }

    private class TextGlow {
        private boolean enableGlow;
        private int glowColor;
        private float glowRadius;

        public TextGlow(boolean enableGlow, int glowColor, float glowRadius) {
            this.enableGlow = enableGlow;
            this.glowColor = glowColor;
            this.glowRadius = glowRadius;
        }
    }

    private TextGlow textGlow = null;

    public void setTextGlow(boolean enableGlow, int glowColor, float glowRadius) {
        textGlow = new TextGlow(enableGlow, glowColor, glowRadius);
    }

    private class TextShadow {
        private boolean enableShadow;
        private int shadowColor;
        private float shadowRadius;
        private float shadowDx;
        private float shadowDy;

        public TextShadow(boolean enableShadow, int shadowColor, float shadowRadius, float shadowDx, float shadowDy) {
            this.enableShadow = enableShadow;
            this.shadowColor = shadowColor;
            this.shadowRadius = shadowRadius;
            this.shadowDx = shadowDx;
            this.shadowDy = shadowDy;
        }
    }

    private TextShadow textShadow;

    public void setTextShadow(boolean enableShadow, int shadowColor, float shadowRadius, float shadowDx, float shadowDy) {
        textShadow = new TextShadow(enableShadow, shadowColor, shadowRadius, shadowDx, shadowDy);
    }

    private Bitmap makeTextBitmap(){
        calcDimensions();

        // Bitmap bitmap = Bitmap.createBitmap((int) (width * actualScale), (int) (height * actualScale), Bitmap.Config.ARGB_8888);
        Bitmap bitmap = Bitmap.createBitmap((int) (width * actualScale), (int) (mOverlayHeight * actualScale), Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);
        canvas.scale(actualScale, actualScale);
        canvas.translate(getShadowPadding(), getShadowPadding());

//        Paint pt = new Paint();
//        pt.setAntiAlias(true);
//        pt.setColor(Color.BLUE);
//
//        canvas.drawRect(new Rect(0, 0, mOverlayWidth, mOverlayHeight), pt);

        Typeface tf = nexFont.getTypeface(mContext, text.textFont);
        if( tf == null )
            tf = Typeface.createFromAsset(mContext.getAssets(), text.textFont);
        TextPaint textPaint = textLayout.getPaint();
        textPaint.setStyle(Paint.Style.FILL);
        textPaint.setTypeface(tf);

        if( text.textAlign == Paint.Align.RIGHT ) {
            textPaint.setTextAlign(Paint.Align.LEFT);
        }

        if( text.drawAlign == 1 ) {
        }
        else if( text.drawAlign == 2 ) {
            canvas.translate(0, (mOverlayHeight*actualScale - height*actualScale)/2);
        }
        else if( text.drawAlign == 3 ) {
            canvas.translate(0, mOverlayHeight*actualScale - height*actualScale);
        }

        if (textShadow != null) {
            textPaint.setMaskFilter(new BlurMaskFilter(textShadow.shadowRadius/actualScale, BlurMaskFilter.Blur.NORMAL));
            textPaint.setColor(textShadow.shadowColor);
            canvas.save();
            canvas.translate(textShadow.shadowDx, textShadow.shadowDy);
            textLayout.draw(canvas);
            canvas.restore();
        }
        if(textGlow != null) {
            textPaint.setMaskFilter(new BlurMaskFilter(textGlow.glowRadius / actualScale, BlurMaskFilter.Blur.OUTER));
            textPaint.setColor(textGlow.glowColor);
            textLayout.draw(canvas);
        }
        if(text != null) {
            textPaint.setMaskFilter(null);
            textPaint.setTextSize(text.textSize);
            textPaint.setColor(text.textColor);
            textLayout.draw(canvas);
        }
        if(textOutline != null) {
            textPaint.setMaskFilter(null);
            textLayout.getPaint().setStyle(Paint.Style.STROKE);
            textPaint.setColor(textOutline.outlineColor);
            textPaint.setStrokeWidth(textOutline.outlineWidth / actualScale);
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

        if(text != null) {
            cachedTextPaint.setTextSize(text.textSize);
            cachedTextPaint.setColor(text.textColor);

            Typeface tf = nexFont.getTypeface(mContext, text.textFont);
            if( tf == null )
                tf = Typeface.createFromAsset(mContext.getAssets(), text.textFont);

            cachedTextPaint.setTypeface(tf);
        }
        if(textOutline != null) {
            cachedTextPaint.setStrokeWidth(textOutline.outlineWidth / actualScale);
        }
        cachedTextPaint.setAntiAlias(true);
        cachedTextPaint.setStyle(Paint.Style.FILL_AND_STROKE);
        cachedTextPaint.setTextAlign(Paint.Align.LEFT);
        return cachedTextPaint;
    }

    private int getShadowPadding() {
        float shadowRadius = 0.0f;
        float glowRadius = 0.0f;
        float outlineRadius = 0.0f;

        if(textShadow != null) {
            shadowRadius = max(Math.abs(textShadow.shadowDx), Math.abs(textShadow.shadowDy));
        }
        if(textGlow != null) {
            glowRadius = textGlow.glowRadius;
        }
        if(textOutline != null) {
            outlineRadius = textOutline.outlineWidth;
        }

        return (int)Math.ceil(max(outlineRadius, max(glowRadius, shadowRadius)));
    }

    private void calcDimensions() {
        if( validDimensions && textLayout!=null )
            return;

        float spaceingmult = 1f;
        float spacingadd = 0f;
        boolean includepad = true;

        int shadowPadding = getShadowPadding();

        TextPaint paint = getPaint();

        width = mOverlayWidth;
        height = mOverlayHeight + 1;

        Layout.Alignment loa = Layout.Alignment.ALIGN_CENTER;

        if( text.textAlign == Paint.Align.CENTER ) {
            loa = Layout.Alignment.ALIGN_CENTER;
        }
        else if( text.textAlign == Paint.Align.LEFT ) {
            loa = Layout.Alignment.ALIGN_NORMAL;
        }
        else if( text.textAlign == Paint.Align.RIGHT ) {
            loa = Layout.Alignment.ALIGN_OPPOSITE;
        }


        StaticLayout tl = null;
        Log.d("utilityTextOverlay", String.format("calcDimension(%s, %d %d)", loa.toString(), height, mOverlayHeight));
        while(height > mOverlayHeight )
        {
            tl = new StaticLayout(mText,/*source*/
                    0,/*bufstart*/
                    mText.length(),/*bufend*/
                    paint,/*paint*/
                    mOverlayWidth - shadowPadding*2,/*outerwidth*/
                    loa,/*align*/
                    spaceingmult,/*spacingmult*/
                    spacingadd,/*spacingadd*/
                    includepad/*includepad*/);

            height = max(1,tl.getHeight()) + shadowPadding*2;
            float textSize = paint.getTextSize();


            if( height <= mOverlayHeight ) {
                // Log.d("utilityTextOverlay", String.format("calcDimension2(%d %d, %f)", height, mOverlayHeight, textSize));
                break;
            }


            paint.setTextSize(textSize-1);
            // Log.d("utilityTextOverlay", String.format("calcDimension(%d %d, %f)", height, mOverlayHeight, textSize));
        }

        textLayout = tl;

        width = max(1,textLayout.getWidth()) + shadowPadding*2;
        height = max(1,textLayout.getHeight()) + shadowPadding*2;

        validDimensions = true;
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
}
