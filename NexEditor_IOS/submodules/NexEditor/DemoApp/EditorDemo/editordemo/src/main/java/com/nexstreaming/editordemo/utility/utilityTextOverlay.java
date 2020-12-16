package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.view.View;

import com.nexstreaming.nexeditorsdk.nexOverlayImage;

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

    public utilityTextOverlay(Context context, String text, int overlayWidth, int overlayHegit, float actualScale){
        mContext = context;
        mText = text;
        mOverlayWidth = overlayWidth;
        mOverlayHeight = overlayHegit;
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

        public Text(float textSize, int textColor, Paint.Align textAlign, String textFont) {
            this.textSize = textSize;
            this.textColor = textColor;
            this.textAlign = textAlign;
            this.textFont = textFont;
        }
    }

    private Text text = null;

    public void setText(float textSize, int textColor, Paint.Align textAlign, String textFont) {
        text = new Text(textSize, textColor, textAlign, textFont);
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

    private Bitmap makeTextBitmapSimple(){
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setTextAlign(text.textAlign);
        paint.setStyle(Paint.Style.FILL_AND_STROKE);
        if(text != null) {
            Typeface tf = Typeface.createFromAsset(mContext.getAssets(), text.textFont);
            paint.setTextSize(text.textSize);
            paint.setColor(text.textColor);
            paint.setTypeface(tf);
        }

        Bitmap bitmap = Bitmap.createBitmap(mOverlayWidth, mOverlayHeight, Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);
        canvas.drawColor(Color.TRANSPARENT);
        canvas.drawText(mText, mOverlayWidth/2, mOverlayHeight/2, paint);

        return bitmap;
    }

    private Bitmap makeTextBitmap(){
        calcDimensions();

        Bitmap bitmap = Bitmap.createBitmap((int) (width * actualScale), (int) (height * actualScale), Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);
        canvas.scale(actualScale,actualScale);
        canvas.translate(getShadowPadding(), getShadowPadding());

        Typeface tf = Typeface.createFromAsset(mContext.getAssets(), text.textFont);
        TextPaint textPaint = textLayout.getPaint();
        textPaint.setStyle(Paint.Style.FILL);
        textPaint.setTypeface(tf);

        if(textShadow != null) {
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
        }
        if(textOutline != null) {
            cachedTextPaint.setStrokeWidth(textOutline.outlineWidth / actualScale);
        }
        cachedTextPaint.setAntiAlias(true);
        cachedTextPaint.setStyle(Paint.Style.FILL_AND_STROKE);
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
        textLayout = new StaticLayout(mText,/*source*/
                0,/*bufstart*/
                mText.length(),/*bufend*/
                getPaint(),/*paint*/
                mOverlayWidth,/*outerwidth*/
                Layout.Alignment.ALIGN_CENTER,/*align*/
                spaceingmult,/*spacingmult*/
                spacingadd,/*spacingadd*/
                includepad/*includepad*/);

        int shadowPadding = getShadowPadding();
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
