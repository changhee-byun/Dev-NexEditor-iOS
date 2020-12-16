package com.nexstreaming.kminternal.nexvideoeditor;

import java.util.ArrayList;
import java.util.WeakHashMap;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.EmbossMaskFilter;
import android.graphics.MaskFilter;
import android.graphics.Paint;
import android.graphics.Paint.Join;
import android.graphics.Paint.Style;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;

import android.support.annotation.NonNull;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.util.Pair;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;


public class NexTextEffect extends TextView { //android.support.v7.widget.AppCompatTextView
    private ArrayList<Shadow> outerShadows;
    private ArrayList<Shadow> outerGlows;
    private ArrayList<Shadow> innerGlows;
    private ArrayList<Emboss> embosses;

    private WeakHashMap<String, Pair<Canvas, Bitmap>> canvasStore;

    private Canvas tempCanvas;
    private Bitmap tempBitmap;

    private Drawable foregroundDrawable;

    private float strokeWidth;
    private Integer strokeColor;
    private Join strokeJoin;
    private float strokeMiter;

    private float sizeFactor = 1.0f;

    private int[] lockedCompoundPadding;
    private boolean frozen = false;

    public NexTextEffect(Context context) {
        super(context);
        init(null);
    }
    public NexTextEffect(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(attrs);
    }
    public NexTextEffect(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(attrs);
    }

    public void init(AttributeSet attrs){
        outerShadows = new ArrayList<Shadow>();
        outerGlows = new ArrayList<Shadow>();
        innerGlows = new ArrayList<Shadow>();
        embosses = new ArrayList<Emboss>();

        if(canvasStore == null){
            canvasStore = new WeakHashMap<String, Pair<Canvas, Bitmap>>();
        }

        if(attrs != null){
/*            TypedArray a = getContext().obtainStyledAttributes(attrs, R.styleable.nexTextEffect);

            String typefaceName = a.getString( R.styleable.nexTextEffect_typeface);
            if(typefaceName != null) {
                Typeface tf = Typeface.createFromAsset(getContext().getAssets(), String.format("fonts/%s.ttf", typefaceName));
                setTypeface(tf);
            }

            if(a.hasValue(R.styleable.nexTextEffect_foreground)){
                Drawable foreground = a.getDrawable(R.styleable.nexTextEffect_foreground);
                if(foreground != null){
                    this.setForegroundDrawable(foreground);
                }else{
                    this.setTextColor(a.getColor(R.styleable.nexTextEffect_foreground, 0xff000000));
                }
            }

            if(a.hasValue(R.styleable.nexTextEffect_background2)){
                Drawable background = a.getDrawable(R.styleable.nexTextEffect_background2);
                if(background != null){
                    this.setBackgroundDrawable(background);
                }else{
                    this.setBackgroundColor(a.getColor(R.styleable.nexTextEffect_background2, 0xff000000));
                }
            }

            if(a.hasValue(R.styleable.nexTextEffect_innerGlowColor)){
                this.addInnerGlow(a.getDimensionPixelSize(R.styleable.nexTextEffect_innerGlowRadius, 0),
                        a.getColor(R.styleable.nexTextEffect_innerGlowColor, 0xff000000));
            }

            if(a.hasValue(R.styleable.nexTextEffect_outerShadowColor)){
                this.addOuterShadow(a.getDimensionPixelSize(R.styleable.nexTextEffect_outerShadowRadius, 0),
                        a.getDimensionPixelOffset(R.styleable.nexTextEffect_outerShadowDx, 0),
                        a.getDimensionPixelOffset(R.styleable.nexTextEffect_outerShadowDy, 0),
                        a.getColor(R.styleable.nexTextEffect_outerShadowColor, 0xff000000));
            }

            if(a.hasValue(R.styleable.nexTextEffect_strokeColor)){
                float strokeWidth = a.getDimensionPixelSize(R.styleable.nexTextEffect_strokeWidth, 1);
                int strokeColor = a.getColor(R.styleable.nexTextEffect_strokeColor, 0xff000000);
                float strokeMiter = a.getDimensionPixelSize(R.styleable.nexTextEffect_strokeMiter, 10);
                Join strokeJoin = null;
                switch(a.getInt(R.styleable.nexTextEffect_strokeJoinStyle, 0)){
                    case(0): strokeJoin = Join.MITER; break;
                    case(1): strokeJoin = Join.BEVEL; break;
                    case(2): strokeJoin = Join.ROUND; break;
                }
                this.setStroke(strokeWidth, strokeColor, strokeJoin, strokeMiter);
            }
*/
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB
                && (  embosses.size() > 0 || innerGlows.size() > 0
                || foregroundDrawable != null
        )
        ){
            setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }
    }

    public void setStroke(float width, int color, Join join, float miter){
        strokeWidth = width;
        strokeColor = color;
        strokeJoin = join;
        strokeMiter = miter;
    }

    public void setStroke(float width, int color){
        setStroke(width, color, Join.MITER, 10);
    }

    /**
     * Set the default text size to the given value, interpreted as "scaled
     * pixel" units.  This size is adjusted based on the current density and
     * user font size preference.
     *
     * <p>Note: if this TextView has the auto-size feature enabled than this function is no-op.
     *
     * @param size The scaled pixel size.
     *
     * @attr ref android.R.styleable#TextView_textSize
     */
    @Override
    public void setTextSize(float size) {
        setTextSize(TypedValue.COMPLEX_UNIT_SP, size * sizeFactor);
    }

    /**
     * Set the default text size to a given unit and value. See {@link
     * TypedValue} for the possible dimension units.
     *
     * <p>Note: if this TextView has the auto-size feature enabled than this function is no-op.
     *
     * @param unit The desired dimension unit.
     * @param size The desired size in the given units.
     *
     * @attr ref android.R.styleable#TextView_textSize
     */
    @Override
    public void setTextSize(int unit, float size) {
        Context c = getContext();
        Resources r;

        if (c == null) {
            r = Resources.getSystem();
        } else {
            r = c.getResources();
        }

        float convertedSize = TypedValue.applyDimension(unit, size, r.getDisplayMetrics());

        final float MinTextSize = 60;
        float minTextSize = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, MinTextSize, r.getDisplayMetrics());

        if ( convertedSize < minTextSize) { // threshold is 50sp(LG G6) because of EmbossMaskFilter issue.
            sizeFactor = minTextSize / convertedSize;
        } else {
            sizeFactor = 1.0f;
        }

        super.setTextSize(unit, size * sizeFactor);
    }

    public void addOuterShadow(float r, float dx, float dy, int color){
        if(r == 0){ r = 0.0001f; }
        outerShadows.add(new Shadow(r,dx,dy,color));
    }

    public void addOuterGlow(float r, float dx, float dy, int color){
        if(r == 0){ r = 0.0001f; }
        outerGlows.add(new Shadow(r,dx,dy,color));
    }

    public void addInnerGlow(float r, int color){
        if(r == 0){ r = 0.0001f; }

        innerGlows.add(new Shadow(r,0,0,color));
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB
                && (  embosses.size() > 0 || innerGlows.size() > 0
                || foregroundDrawable != null
        )
        ){
            setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }
    }

    /**
     *
     * @param direction  array of 3 scalars [x, y, z] specifying the direction of the light source
     * @param ambient    0...1 amount of ambient light
     * @param specular   coefficient for specular highlights (e.g. 8)
     * @param blurRadius amount to blur before applying lighting (e.g. 3)
     */
    public void addEmboss(float[] direction, float ambient, float specular, float blurRadius){
        if (direction.length < 3) {
            throw new ArrayIndexOutOfBoundsException();
        }
        embosses.add(new Emboss(direction,ambient,specular,blurRadius));

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB
                && (  embosses.size() > 0 || innerGlows.size() > 0
                || foregroundDrawable != null
        )
        ){
            setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }
    }

    public void clearStroke() {
        strokeColor = null;
    }

    public void clearOuterShadows() {
        outerShadows.clear();
    }

    public void clearOuterGlows() {
        outerGlows.clear();
    }

    public void clearInnerGlows() {
        innerGlows.clear();
    }

    public void clearEmbosses() {
        embosses.clear();
    }

    public void setForegroundDrawable(Drawable d){
        this.foregroundDrawable = d;
    }

    public Drawable getForeground(){
        return this.foregroundDrawable == null ? this.foregroundDrawable : new ColorDrawable(this.getCurrentTextColor());
    }


    @Override
    public void onDraw(Canvas canvas){
        super.onDraw(canvas);

        freeze();
        Drawable restoreBackground = this.getBackground();
        Drawable[] restoreDrawables = this.getCompoundDrawables();
        int restoreColor = this.getCurrentTextColor();

        this.setCompoundDrawables(null,  null, null, null);

        for(Shadow shadow : outerShadows){
            this.setShadowLayer(shadow.r * sizeFactor, shadow.dx * sizeFactor, shadow.dy * sizeFactor, shadow.color);
            super.onDraw(canvas);
        }
        for(Shadow shadow : outerGlows){
            this.setShadowLayer(shadow.r * sizeFactor, shadow.dx * sizeFactor, shadow.dy * sizeFactor, shadow.color);
            super.onDraw(canvas);
        }

        this.setShadowLayer(0,0,0,0);
        this.setTextColor(restoreColor);

        if(embosses.size() > 0){
            generateTempCanvas(canvas);
            TextPaint paint = this.getPaint();
            for(Emboss emboss : embosses){
                //paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
                MaskFilter filter = new EmbossMaskFilter(emboss.direction, emboss.ambient, emboss.specular, emboss.blurRadius * sizeFactor);
                paint.setMaskFilter(filter);
                super.onDraw(canvas);
                //paint.setXfermode(null);
                paint.setMaskFilter(null);
            }
        }

        if(this.foregroundDrawable != null && this.foregroundDrawable instanceof BitmapDrawable){
            generateTempCanvas(canvas);
            super.onDraw(tempCanvas);
            Paint paint = ((BitmapDrawable) this.foregroundDrawable).getPaint();
            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_ATOP));
            this.foregroundDrawable.setBounds(canvas.getClipBounds());
            this.foregroundDrawable.draw(tempCanvas);
            canvas.drawBitmap(tempBitmap, 0, 0, null);
            tempCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
        }

        if(strokeColor != null){
            TextPaint paint = this.getPaint();
            paint.setStyle(Style.STROKE);
            paint.setStrokeJoin(strokeJoin);
            paint.setStrokeMiter(strokeMiter);
            this.setTextColor(strokeColor);
            paint.setStrokeWidth(strokeWidth * sizeFactor);
            super.onDraw(canvas);
            paint.setStyle(Style.FILL);
            this.setTextColor(restoreColor);
        }

        if(innerGlows.size() > 0){
            generateTempCanvas(canvas);
            TextPaint paint = this.getPaint();
            for(Shadow shadow : innerGlows){
                this.setTextColor(shadow.color);
                super.onDraw(tempCanvas);
                this.setTextColor(0xFF000000);
                paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
                paint.setMaskFilter(new BlurMaskFilter(shadow.r * sizeFactor, BlurMaskFilter.Blur.NORMAL));

                tempCanvas.save();
                tempCanvas.translate(shadow.dx, shadow.dy);
                super.onDraw(tempCanvas);
                tempCanvas.restore();
                canvas.drawBitmap(tempBitmap, 0, 0, null);
                tempCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

                paint.setXfermode(null);
                paint.setMaskFilter(null);
                this.setTextColor(restoreColor);
                this.setShadowLayer(0,0,0,0);
            }
        }

        if(restoreDrawables != null){
            this.setCompoundDrawablesWithIntrinsicBounds(restoreDrawables[0], restoreDrawables[1], restoreDrawables[2], restoreDrawables[3]);
        }
        this.setBackgroundDrawable(restoreBackground);
        this.setTextColor(restoreColor);

        unfreeze();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        //int shadowpadding = getShadowPadding() * 2;
        int horiPadding = getShadowLeftPadding() + getShadowRightPadding();
        int verPadding = getShadowTopPadding() + getShadowBottomPadding();
        setMeasuredDimension(getMeasuredWidth() + horiPadding, getMeasuredHeight()+ verPadding);
    }

    private int getShadowPadding() {
        float glowRadius = 0f;
        float shadowDx = 0f;
        float shadowDy = 0f;

        for(Shadow shadow : outerShadows){
            glowRadius = Math.max(glowRadius, shadow.r);
            shadowDx = Math.max(shadowDx, Math.abs(shadow.dx  * sizeFactor));
            shadowDy = Math.max(shadowDy, Math.abs(shadow.dy  * sizeFactor));
        }

        for(Shadow shadow : outerGlows){
            glowRadius = Math.max(glowRadius, shadow.r);
            shadowDx = Math.max(shadowDx, Math.abs(shadow.dx * sizeFactor));
            shadowDy = Math.max(shadowDy, Math.abs(shadow.dy * sizeFactor));
        }

        for(Emboss emboss : embosses){
            glowRadius = Math.max(glowRadius, emboss.blurRadius * sizeFactor);
        }

        return (int)Math.ceil(   Math.max(strokeWidth * sizeFactor, Math.max(glowRadius, (glowRadius + Math.max(Math.abs(shadowDx), Math.abs(shadowDy))))) );
    }

    private int getShadowLeftPadding() {
        float glowRadius = 0f;
        float shadowDx = 0f;
        float shadowDy = 0f;

        for(Shadow shadow : outerShadows){
            shadowDx = Math.min(shadowDx, shadow.dx * sizeFactor - shadow.r * sizeFactor);
        }
        for(Shadow shadow : outerGlows){
            glowRadius = Math.max(glowRadius, shadow.r * sizeFactor);
            shadowDx = Math.min(shadowDx, shadow.dx * sizeFactor - shadow.r * sizeFactor);
        }

        shadowDx = Math.min(0, shadowDx);
        for(Emboss emboss : embosses){
            glowRadius = Math.max(glowRadius, emboss.blurRadius * sizeFactor);
        }

        return (int)Math.ceil( Math.max(glowRadius, Math.abs(shadowDx)) );
    }

    private int getShadowTopPadding() {
        float glowRadius = 0f;
        float shadowDy = 0f;

        for(Shadow shadow : outerShadows){
            shadowDy = Math.min(shadowDy, shadow.dy * sizeFactor - shadow.r * sizeFactor);
        }
        for(Shadow shadow : outerGlows){
            glowRadius = Math.max(glowRadius, shadow.r * sizeFactor);
            shadowDy = Math.min(shadowDy, shadow.dy * sizeFactor - shadow.r * sizeFactor);
        }

        shadowDy = Math.min(0, shadowDy);
        for(Emboss emboss : embosses){
            glowRadius = Math.max(glowRadius, emboss.blurRadius * sizeFactor);
        }

        return (int)Math.ceil( Math.max(glowRadius, Math.abs(shadowDy)) );
    }

    private int getShadowRightPadding() {
        float glowRadius = 0f;
        float shadowDx = 0f;
        float shadowDy = 0f;

        for(Shadow shadow : outerShadows){
            shadowDx = Math.max(shadowDx, shadow.dx * sizeFactor + shadow.r * sizeFactor);
        }
        for(Shadow shadow : outerGlows){
            glowRadius = Math.max(glowRadius, shadow.r * sizeFactor);
            shadowDx = Math.max(shadowDx, shadow.dx * sizeFactor + shadow.r * sizeFactor);
        }

        shadowDx = Math.max(0, shadowDx);
        for(Emboss emboss : embosses){
            glowRadius = Math.max(glowRadius, emboss.blurRadius * sizeFactor);
        }

        return (int)Math.ceil( Math.max(glowRadius, Math.abs(shadowDx)) );
    }

    private int getShadowBottomPadding() {
        float glowRadius = 0f;
        float shadowDy = 0f;

        for(Shadow shadow : outerShadows){
            glowRadius = Math.max(glowRadius, shadow.r * sizeFactor);
            shadowDy = Math.max(shadowDy, shadow.dy * sizeFactor + shadow.r * sizeFactor);
        }
        for(Shadow shadow : outerGlows){
            glowRadius = Math.max(glowRadius, shadow.r * sizeFactor);
            shadowDy = Math.max(shadowDy, shadow.dy * sizeFactor + shadow.r * sizeFactor);
        }

        shadowDy = Math.max(0, shadowDy);
        for(Emboss emboss : embosses){
            glowRadius = Math.max(glowRadius, emboss.blurRadius * sizeFactor);
        }

        return (int)Math.ceil( Math.max(glowRadius, Math.abs(shadowDy)) );
    }


    private void generateTempCanvas(@NonNull Canvas canvas){
        int width = canvas.getWidth();
        int height = canvas.getHeight();

        String key = String.format("%dx%d", width, height);
        Pair<Canvas, Bitmap> stored = canvasStore.get(key);
        if(stored != null){
            tempCanvas = stored.first;
            tempBitmap = stored.second;
        }else{
            tempCanvas = new Canvas();
            tempBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            tempCanvas.setBitmap(tempBitmap);
            canvasStore.put(key, new Pair<Canvas, Bitmap>(tempCanvas, tempBitmap));
        }
    }

    public Bitmap makeTextBitmap() {
        int scaled_w = 0;
        int scaled_h = 0;
        boolean isLaidOut = false;
        boolean hasParent = getParent() != null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            //isLaidOut = isLaidOut();
            isLaidOut = false;
        } else {
            //TODO
            // should be checked this view is laid out.
        }

        if (isLaidOut) {
            scaled_w = (int)this.getWidth();
            scaled_h = (int)this.getHeight();
        } else {
            ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            setLayoutParams(params);
            int leftPadding = getShadowLeftPadding();
            int topPadding = getShadowTopPadding();
            int rightPdding = getShadowRightPadding();
            int bottomPdding = getShadowBottomPadding();
            this.setPadding(leftPadding, topPadding, rightPdding, bottomPdding);

            this.requestLayout();

            this.measure(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
            scaled_h = this.getMeasuredHeight();
            scaled_w = this.getMeasuredWidth();
            this.layout(0, 0, scaled_w, scaled_h);
        }
        return makeTextBitmap(scaled_w, scaled_h);
    }

    private Bitmap makeTextBitmap(int width, int height) {

        if (width <= 0 || height <= 0)
            return null;

        Bitmap bitmap;
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);

        draw(canvas);
        //canvas.scale(1/sizeFactor, 1/sizeFactor);

        if(sizeFactor > 1.0f) {
            return Bitmap.createScaledBitmap(bitmap, (int)(width / sizeFactor), (int)(height / sizeFactor), true);
        } else
            return bitmap;
    }

    // Keep these things locked while onDraw in processing
    public void freeze(){
        lockedCompoundPadding = new int[]{
                getCompoundPaddingLeft(),
                getCompoundPaddingRight(),
                getCompoundPaddingTop(),
                getCompoundPaddingBottom()
        };
        frozen = true;
    }

    public void unfreeze(){
        frozen = false;
    }

    @Override
    public void requestLayout(){
        if(!frozen) super.requestLayout();
    }

    @Override
    public void postInvalidate(){
        if(!frozen) super.postInvalidate();
    }

    @Override
    public void postInvalidate(int left, int top, int right, int bottom){
        if(!frozen) super.postInvalidate(left, top, right, bottom);
    }

    @Override
    public void invalidate(){
        if(!frozen)	super.invalidate();
    }

    @Override
    public void invalidate(Rect rect){
        if(!frozen) super.invalidate(rect);
    }

    @Override
    public void invalidate(int l, int t, int r, int b){
        if(!frozen) super.invalidate(l,t,r,b);
    }

    @Override
    public int getCompoundPaddingLeft(){
        return !frozen ? super.getCompoundPaddingLeft() : lockedCompoundPadding[0];
    }

    @Override
    public int getCompoundPaddingRight(){
        return !frozen ? super.getCompoundPaddingRight() : lockedCompoundPadding[1];
    }

    @Override
    public int getCompoundPaddingTop(){
        return !frozen ? super.getCompoundPaddingTop() : lockedCompoundPadding[2];
    }

    @Override
    public int getCompoundPaddingBottom(){
        return !frozen ? super.getCompoundPaddingBottom() : lockedCompoundPadding[3];
    }

    public static class Shadow{
        float r;
        float dx;
        float dy;
        int color;
        public Shadow(float r, float dx, float dy, int color){
            this.r = r;
            this.dx = dx;
            this.dy = dy;
            this.color = color;
        }
    }

    public class Emboss{
        float[] direction = {1,5,1};
        float ambient = 0.5f;
        float specular = 10;
        float blurRadius = 7.5f; // this value must be greater than zero.

        public Emboss(float direction[], float ambient, float specular, float blurRadius) {
            this.direction = direction;
            this.ambient = ambient;
            this.specular = specular;
            if(blurRadius == 0)
                blurRadius = 0.1f;
            this.blurRadius = blurRadius;
        }
    }
}