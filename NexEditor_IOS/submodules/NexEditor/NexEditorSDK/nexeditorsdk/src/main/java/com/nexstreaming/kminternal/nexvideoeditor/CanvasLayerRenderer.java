package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.RectF;

/**
 * Usage:
 *
 *      public void onDraw( Canvas canvas ) {
 *          canvasLayerRenderer.setCurrentTime(...);
 *          canvasLayerRenderer.beginRender(canvas, true);
 *          // ... normal rendering code here
 *          canvasLayerRenderer.endRender()
 *      }
 *
 */
public class CanvasLayerRenderer implements LayerRendererInterface {

    private Canvas canvas = null;
    private int saveLevel = 0;
    private int currentTime = 0;
    private boolean useStandardizedResolution;
    private RectF dstRect = new RectF();
    private Paint paint = new Paint();
    ColorMatrix colorMatrix;
    private float hue = 0.0f;
    private RendererState[] stateStack = new RendererState[16];

    public CanvasLayerRenderer(){}

    private static class RendererState {
        int alpha;
        ColorMatrix colorMatrix;
    }

    /**
     *
     * @param canvas
     * @param useStandardizedResolution  If true, set scaling as if using 720p; if false use canvas dimensions
     */
    public void beginRender(Canvas canvas, boolean useStandardizedResolution) {
        this.canvas = canvas;
        this.useStandardizedResolution = useStandardizedResolution;
        onBeginRender();
    }

    private void onBeginRender() {
        if( saveLevel!=0 ) {
            throw new IllegalStateException("beginRender() called again without endRender()");
        }
        saveLevel = canvas.save();
        if( useStandardizedResolution ) {
            canvas.scale(canvas.getWidth()/1280f, canvas.getHeight()/720f);
        }
        colorMatrix = null;
        paint.reset();
        paint.setAntiAlias(true);
        paint.setFilterBitmap(true);
    }

    public void endRender() {
        onEndRender();
        this.canvas = null;
    }

    private void onEndRender() {
        canvas.restoreToCount(saveLevel);
        saveLevel = 0;
    }

    @Override
    public float getOutputWidth() {
        return useStandardizedResolution?1280:canvas.getWidth();
    }

    @Override
    public float getOutputHeight() {
        return useStandardizedResolution?720:canvas.getHeight();
    }

    @Override
    public int getCurrentTime() {
        return currentTime;
    }

    @Override
    public void drawBitmap(Bitmap bitmap, float left, float top, float right, float bottom) {
        dstRect.set(left,top,right,bottom);
        canvas.drawBitmap(bitmap,null,dstRect,paint);
    }

    @Override
    public void drawBitmap(Bitmap bitmap, float x, float y) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        float left = x - width/2f;
        float top = x - width/2f;
        drawBitmap(bitmap,left,top,left+width,top+height);
    }

    @Override
    public void drawBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        float left = -width/2f;
        float top = -width/2f;
        drawBitmap(bitmap,left,top,left+width,top+height);
    }

    @Override
    public void preCacheBitmap(Bitmap bitmap) {
        // Nothing to do
    }

    @Override
    public void setColorMatrix(ColorMatrix colorMatrix) {
        if( this.colorMatrix==colorMatrix )
            return;
        this.colorMatrix = colorMatrix;
        if( colorMatrix==null ) {
            paint.setColorFilter(null);
        } else {
            paint.setColorFilter(new ColorMatrixColorFilter(colorMatrix));
        }
    }

    @Override
    public void setHue(float hue){

        this.hue = hue;
    }

    @Override
    public float getHue(){

        return hue;
    }

    @Override
    public ColorMatrix getColorMatrix() {
        return colorMatrix;
    }

    @Override
    public void save() {
        canvas.save();
        int saveCount = canvas.getSaveCount();
        if( saveCount+1 > stateStack.length ) {
            RendererState[] newStateStack = new RendererState[saveCount+16];
            System.arraycopy(stateStack,0,newStateStack,0,stateStack.length);
            newStateStack = stateStack;
        }
        if( stateStack[saveCount]==null )
            stateStack[saveCount] = new RendererState();
        stateStack[saveCount].alpha = paint.getAlpha();
        stateStack[saveCount].colorMatrix = colorMatrix;
    }

    @Override
    public void restore() {
        int saveCount = canvas.getSaveCount();
        canvas.restore();
        if( stateStack[saveCount]!=null ) {
            setAlpha(stateStack[saveCount].alpha);
            setColorMatrix(stateStack[saveCount].colorMatrix);
        }
    }

    @Override
    public void translate(float x, float y, float z) {
        if( z!=0 )
            throw new UnsupportedOperationException();
        canvas.translate(x,y);
    }

    @Override
    public void translate(float x, float y) {
        canvas.translate(x,y);
    }

    @Override
    public void scale(float x, float y, float z) {
        if( z!=0 )
            throw new UnsupportedOperationException();
        canvas.scale(x,y);
    }

    @Override
    public void scale(float x, float y) {
        canvas.scale(x,y);
    }

    @Override
    public void scale(float x, float y, float cx, float cy) {
        canvas.scale(x,y,cx,cy);
    }

    @Override
    public void rotateAroundAxis(float a, float x, float y, float z) {
        throw new UnsupportedOperationException();
    }

    @Override
    public float[] getMatrix() {
        throw new UnsupportedOperationException();
    }

    @Override
    public void rotate(float a, float cx, float cy) {
        canvas.rotate(a,cx,cy);
    }

    @Override
    public void setAlpha(float alpha) {
        paint.setAlpha((int) (alpha*255));
    }

    @Override
    public void setCurrentTime(int currentTime) {
        this.currentTime = currentTime;
    }
}
