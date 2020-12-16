package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.Bitmap;
import android.graphics.ColorMatrix;
import android.opengl.GLU;
import android.opengl.GLUtils;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.WeakHashMap;

import static android.opengl.GLES20.GL_CLAMP_TO_EDGE;
import static android.opengl.GLES20.GL_LINEAR;
import static android.opengl.GLES20.GL_NO_ERROR;
import static android.opengl.GLES20.GL_RGBA;
import static android.opengl.GLES20.GL_TEXTURE0;
import static android.opengl.GLES20.GL_TEXTURE_2D;
import static android.opengl.GLES20.GL_TEXTURE_MAG_FILTER;
import static android.opengl.GLES20.GL_TEXTURE_MIN_FILTER;
import static android.opengl.GLES20.GL_TEXTURE_WRAP_S;
import static android.opengl.GLES20.GL_TEXTURE_WRAP_T;
import static android.opengl.GLES20.GL_UNSIGNED_BYTE;
import static android.opengl.GLES20.glActiveTexture;
import static android.opengl.GLES20.glBindTexture;
import static android.opengl.GLES20.glCopyTexImage2D;
import static android.opengl.GLES20.glCopyTexSubImage2D;
import static android.opengl.GLES20.glDeleteTextures;
import static android.opengl.GLES20.glGenTextures;
import static android.opengl.GLES20.glGetError;
import static android.opengl.GLES20.glTexImage2D;
import static android.opengl.GLES20.glTexParameteri;

/**
 * Created by matthew on 2/12/15.
 */
public class LayerRenderer implements LayerRendererInterface {

    private static final String LOG_TAG = "LayerRenderer";
    private static int sTexNameScratch[] = new int[1];

    private NexLayerRenderer mRealRenderer = new NexLayerRenderer();
    private Map<Bitmap,LayerBitmap> mLayerBitmapMap = new WeakHashMap<>();
    private Set<LayerBitmap> mLayerBitmapList = new HashSet<>();
    private Map<EffectTexture, Integer> mEffectLayerMap = new WeakHashMap<>();
    private Set<Integer> mEffectLayerList = new HashSet<>();

    private Bitmap mFillRectBitmap = null;

    public enum RenderTarget {
        Normal, Mask
    }

    public enum RenderMode {
        Preview(0), Export(1);

        public final int id;
        RenderMode(int id) {
            this.id = id;
        }
    }

    public LayerRenderer(){
        mRealRenderer.createRenderer();
    }

    public void destroy() {
        mRealRenderer.removeRenderer();
    }

    public float[] getTexMatrix() {
        return mRealRenderer.getTexMatrix();
    }

    public float getOutputWidth() {
        return mRealRenderer.getOutputWidth();
    }

    public float getOutputHeight() {
        return mRealRenderer.getOutputHeight();
    }

    public float getScreenWidth(){
        return mRealRenderer.getScreenDimensionWidth();
    }

    public float getScreenHeight(){
        return mRealRenderer.getScreenDimensionHeight();
    }

    public void setTextureSizeW(float w){
        mRealRenderer.setEffectTextureWidth(w);
    }

    public void setTextureSizeH(float h){
        mRealRenderer.setEffectTextureHeight(h);
    }

    public float getTextureSizeW(){
        return mRealRenderer.getEffectTextureWidth();
    }

    public float getTextureSizeH(){
        return mRealRenderer.getEffectTextureHeight();
    }

    public void setHue(float hue) { mRealRenderer.setHue(hue); }

    public float getHue() { return mRealRenderer.getHue(); }

    private Object expressionState;

    public Object getExpressionState() {
        return expressionState;
    }

    public void setExpressionState(Object expressionState) {
        this.expressionState = expressionState;
    }

    public void setRenderTarget( RenderTarget renderTarget ) {
        mRealRenderer.setRenderTarget(renderTarget == RenderTarget.Normal ? 0 : 1);
    }

    public boolean getMaskEnabled(){
        return mRealRenderer.getMaskEnabled();
    }

    public void setMaskEnabled( boolean maskEnabled ) {
        mRealRenderer.setMaskEnabled(maskEnabled);
    }

    public void setLUT(Bitmap bitmap){

        if(bitmap == null) {

            mRealRenderer.setLUTEnable(false);
            return;
        }

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.setLUT(lb.texName);
    }

    public void setChromakeyEnabled(boolean flag){
        mRealRenderer.setChromakeyEnabled(flag);
    }

    public void setChromakeyMaskEnabled(boolean maskOnOff){
        mRealRenderer.setChromakeyViewMaskEnabled(maskOnOff);
    }

    public int getChromakeyMaskEnabled(){
        return mRealRenderer.getChromakeyViewMaskEnabled();
    }

    public void setChromakeyColor(int color, float clipFg, float clipBg, float bx0, float by0, float bx1, float by1){
        mRealRenderer.setChromakeyColor(color, clipFg, clipBg, bx0, by0, bx1, by1);
    }

    private static class LayerBitmap {

        int texName;
        int width;
        int height;
        int generationId;
        boolean destroyed = false;

        public LayerBitmap(Bitmap bitmap) {
            CHECK_GL_ERROR();
            width = bitmap.getWidth();
            height = bitmap.getHeight();
            generationId = bitmap.getGenerationId();

            glGenTextures(1, sTexNameScratch, 0);                                                     CHECK_GL_ERROR();
            texName = sTexNameScratch[0];
            if (LL.D) Log.d(LOG_TAG, "Made layer texture: " + texName + " (generation=" + generationId + ")");
            glActiveTexture(GL_TEXTURE0);                                                           CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, texName);                                                  CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            GLUtils.texImage2D(GL_TEXTURE_2D, 0, bitmap, 0); //                                       CHECK_GL_ERROR();
            if( glGetError()!=GL_NO_ERROR ) {
                texImage2D(GL_TEXTURE_2D, 0, bitmap, 0);    CHECK_GL_ERROR();
            }
        }

        private static void texImage2D(int target, int level, Bitmap bitmap, int border) {
            int w = bitmap.getWidth();
            int h = bitmap.getHeight();
            IntBuffer pixels = IntBuffer.allocate(w * h);
            bitmap.getPixels(pixels.array(), 0, w, 0, 0, w, h);
            int[] a = pixels.array();
            int len = a.length;
            for( int i=0; i<len; i++ ) {
                int n = a[i];
                a[i] = ((n&0x00FF0000) >> 16) | ((n&0x00000FF) << 16) | (n&0xFF00FF00);
            }
            glTexImage2D(target, level, GL_RGBA, w, h, border, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        }

        public void updateIfNeeded(Bitmap bitmap) {
            if( bitmap.getGenerationId()==generationId )
                return;
            if (LL.D) Log.d(LOG_TAG, "Update layer texture: " + texName + " (generation=" + generationId + "->" + bitmap.getGenerationId() + ")");
            generationId = bitmap.getGenerationId();
            if( bitmap.getWidth()== width && bitmap.getHeight()== height) {
                glActiveTexture(GL_TEXTURE0);                                                       CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName);                                              CHECK_GL_ERROR();
                GLUtils.texSubImage2D(GL_TEXTURE_2D,0,0,0,bitmap);                                  CHECK_GL_ERROR();
            } else {
                width = bitmap.getWidth();
                height = bitmap.getHeight();
                glActiveTexture(GL_TEXTURE0);                                                       CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName);                                              CHECK_GL_ERROR();
                GLUtils.texImage2D(GL_TEXTURE_2D, 0, bitmap, 0);                                    CHECK_GL_ERROR();
            }
        }

        public void destroy() {
            if( !destroyed ) {
                if (LL.D) Log.d(LOG_TAG, "Destroy texture: " + texName );
                sTexNameScratch[0] = texName;
                glDeleteTextures(1,sTexNameScratch,0);                                              CHECK_GL_ERROR();
                texName = 0;
                destroyed = true;
            }
        }
    }

    public static class EffectTexture{

        int tex_id;
        float width;
        float height;
        float xPos;
        float yPos;
        boolean destroyed = false;

        public EffectTexture(float w, float h){
            width = w;
            height = h;
            glGenTextures(1, sTexNameScratch, 0);                                                   CHECK_GL_ERROR();
            tex_id = sTexNameScratch[0];
            if (LL.D) Log.d(LOG_TAG, "Create EffectTexture tex_id : " + tex_id);
            glActiveTexture(GL_TEXTURE0);                                                           CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, tex_id);                                                   CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) width, (int) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, null); CHECK_GL_ERROR();
        }

        public EffectTexture(float x, float y, float w, float h){
            xPos = x;
            yPos = y;
            width = w;
            height = h;
            glGenTextures(1, sTexNameScratch, 0);                                                   CHECK_GL_ERROR();
            tex_id = sTexNameScratch[0];
            glActiveTexture(GL_TEXTURE0);                                                           CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, tex_id);                                                   CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);        CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);                    CHECK_GL_ERROR();
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) xPos, (int) yPos, (int) width, (int) height, 0);
        }

        public void update(float x, float y, float w, float h) {
            if (w == width && h == height) {
                glActiveTexture(GL_TEXTURE0);                                                       CHECK_GL_ERROR();
                if (LL.D) Log.d(LOG_TAG, "Not change width and height.  copyScrToTexture tex_id : " + tex_id);
                glBindTexture(GL_TEXTURE_2D, tex_id);                                               CHECK_GL_ERROR();
                glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int) x, (int) y, (int) w, (int) h);
            } else {
                width = w;
                height = h;
                glActiveTexture(GL_TEXTURE0);                                                       CHECK_GL_ERROR();
                if (LL.D) Log.d(LOG_TAG, "Change width and height. copyScrToTexture tex_id : " + tex_id);
                glBindTexture(GL_TEXTURE_2D, tex_id);                                               CHECK_GL_ERROR();
                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) x, (int) y, (int) width, (int) height, 0);
            }
        }

        public void destroy() {
            if( !destroyed ) {
                sTexNameScratch[0] = tex_id;
                if (LL.D) Log.d(LOG_TAG, "Destroy effect texture  tex_id : " + tex_id );
                glDeleteTextures(1, sTexNameScratch, 0);                                              CHECK_GL_ERROR();
                tex_id = 0;
                destroyed = true;
            }
        }

        public int getTex_id() {
            return tex_id;
        }
        public float getWidth(){
            return width;
        }
        public float getHeight(){
            return height;
        }
    }

    public void setEffectStrength(float strength){
        mRealRenderer.setEffectStrength(strength);
    }

    public float getEffectStrength(){
        return mRealRenderer.getEffectStrength();
    }

    public void setBlurEnabled(boolean enabled){
        mRealRenderer.setHBlurEnabled(enabled);
    }

    public void setVBlurEnabled(boolean enabled){
        mRealRenderer.setVBlurEnabled(enabled);
    }

    public void setMosaicEnabled(boolean enabled){
        mRealRenderer.setMosaicEnabled(enabled);
    }

    private static void CHECK_GL_ERROR() {

        int err;
        boolean first = true;
        while( (err = glGetError())!=GL_NO_ERROR ) {
            if(first) {
                first = false;
                StackTraceElement[] st = Thread.currentThread().getStackTrace();
                Log.e(LOG_TAG, "GLError(s) detected at:");
                for( int i=0; i<st.length && i<5; i++ ) {
                    Log.e(LOG_TAG, "    " + i + ": " + st[i].getFileName() + ":" + st[i].getLineNumber() + " (" + st[i].getClassName() + "." + st[i].getMethodName() + ")");
                }
            }
            Log.e(LOG_TAG, "GLError: 0x" + Integer.toHexString(err) + " (" + GLU.gluErrorString(err) + ")");
        }
    }

    public int getCurrentTime() {
        return mRealRenderer.getCurrentTime();
    }

    private void freeUnusedLayerBitmaps() {
        List<LayerBitmap> toRemove = null;
        for( LayerBitmap b: mLayerBitmapList) {
            if( !mLayerBitmapMap.containsValue(b) ) {
                if( toRemove==null ) {
                    toRemove = new ArrayList<>();
                }
                toRemove.add(b);
            }
        }
        if( toRemove!=null ) {
            for( LayerBitmap b: toRemove ) {
                b.destroy();
            }
            mLayerBitmapList.removeAll(toRemove);
        }
    }

    private void freeUnusedEffectTextures(){
        List<Integer> toRemove = null;
        for( Integer i: mEffectLayerList) {
            if( !mEffectLayerMap.containsValue(i) ) {
                if( toRemove==null ) {
                    toRemove = new ArrayList<>();
                }
                toRemove.add(i);
            }
        }
        if( toRemove!=null ) {
            for( Integer i: toRemove ) {
                sTexNameScratch[0] = i;
                if (LL.D) Log.d(LOG_TAG, "freeUnusedEffectTextures  Destroy effect texture: " + i );
                glDeleteTextures(1, sTexNameScratch, 0);                                              CHECK_GL_ERROR();
            }
            mEffectLayerList.removeAll(toRemove);
        }
    }

    public void removeBitmapFromCache(Bitmap bitmap) {
        if(mLayerBitmapMap.remove(bitmap)!=null) {
            freeUnusedLayerBitmaps();
        }
    }

    private LayerBitmap getLayerBitmap( Bitmap bitmap ) {

        LayerBitmap lb = mLayerBitmapMap.get(bitmap);
        if( lb != null ) {
            lb.updateIfNeeded(bitmap);        CHECK_GL_ERROR();
        } else {
            lb = new LayerBitmap(bitmap);        CHECK_GL_ERROR();
            mLayerBitmapMap.put(bitmap,lb);
            mLayerBitmapList.add(lb);
        }
        return lb;
    }

    public EffectTexture getEffectTexture(EffectTexture effectTex, float x, float y, float w, float h){
        EffectTexture et = effectTex;
        if(et != null){
            et.update(x, y, w, h);
        }else {
            et = new EffectTexture(w, h);
            et.update(x, y, w, h);
            int tex_id = et.getTex_id();
            mEffectLayerMap.put(et, tex_id);
            mEffectLayerList.add(tex_id);
        }
        return et;
    }

    public int getTexNameForBitmap(Bitmap bitmap, int export_flag){

        if(null == bitmap)
            return 0;

        LayerBitmap lb = mLayerBitmapMap.get(bitmap);
        if( lb != null ) {
            lb.updateIfNeeded(bitmap);        CHECK_GL_ERROR();
        } else {
            lb = new LayerBitmap(bitmap);        CHECK_GL_ERROR();
            mLayerBitmapMap.put(bitmap,lb);
            mLayerBitmapList.add(lb);
        }

        return lb.texName;
    }

    private void presetForDrawing(float left, float top, float right, float bottom, int type){
        mRealRenderer.presetForDrawing(left, top, right, bottom, type);
    }

    public void drawDirect(int texname, int type, float x, float y, float w, float h, int flipMode){

        if(texname < 0)
            return;

        mRealRenderer.drawDirect(texname, type, x, y, w, h, flipMode);
    }
    
    public void drawBitmap(Bitmap bitmap, float left, float top, float right, float bottom) {

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);          CHECK_GL_ERROR();
        mRealRenderer.drawBitmap(lb.texName, left, top, right, bottom, 0);
    }

    public void drawBitmap(Bitmap bitmap, float left, float top, float right, float bottom, int flipMode) {

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);          CHECK_GL_ERROR();
        mRealRenderer.drawBitmap(lb.texName, left, top, right, bottom, flipMode);
    }

    public void drawBitmap(Bitmap bitmap, float left, float top, float right, float bottom, float repeat_x, float repeat_y) {

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);          CHECK_GL_ERROR();
        mRealRenderer.drawBitmapRepeat(lb.texName,left, top, right, bottom, repeat_x, repeat_y);
    }

    public void drawBitmap(Bitmap bitmap, float x, float y) {
        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawBitmap(lb.texName,x - lb.width / 2, y - lb.height / 2, x + lb.width / 2, y + lb.height / 2, 0);
    }

    public void drawBitmap(Bitmap bitmap, float x, float y, int flipMode) {

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawBitmap(lb.texName,x - lb.width / 2, y - lb.height / 2, x + lb.width / 2, y + lb.height / 2, flipMode);
    }
    
    public void drawBitmap(Bitmap bitmap) {
        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawBitmap(lb.texName,-lb.width / 2, -lb.height / 2, lb.width / 2, lb.height / 2, 0);
    }
    
    public void drawBitmap(Bitmap bitmap, int flipMode) {

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawBitmap(lb.texName,-lb.width / 2, -lb.height / 2, lb.width / 2, lb.height / 2, flipMode);
    }

    public void drawRenderItem(int effect_id, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, boolean mask_enabled){
        mRealRenderer.drawRenderItem(effect_id, usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled);
    }

    public void drawRenderItem(int effect_id, int tex_id_for_second, int type, String usersettings, int current_time, int start_time, int end_time, float x, float y, float w, float h, float alpha, boolean mask_enabled){
        mRealRenderer.drawRenderItemBlend(effect_id, tex_id_for_second, type, usersettings, current_time, start_time, end_time, x, y, w, h, alpha, mask_enabled);
    }

    public void drawRenderItem(int effect_id_for_blend, int effect_id, String usersettings_for_blend, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, boolean mask_enabled){
        mRealRenderer.drawRenderItemBlendOverlay(effect_id_for_blend, effect_id, usersettings_for_blend, usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled);
    }


    public void drawRenderItem(int effect_id, Bitmap bitmap, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, boolean mask_enabled){

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawRenderItemBitmap(effect_id, lb.texName, usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled);
    }

    public void drawRenderItem(int effect_id, Bitmap bitmap, String usersettings, int current_time, int start_time, int end_time, float x, float y, float alpha, boolean mask_enabled){

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.drawRenderItemBitmap(effect_id, lb.texName, usersettings, current_time, start_time, end_time, x - lb.width / 2, y - lb.height / 2, x + lb.width / 2, y + lb.height / 2, alpha, mask_enabled);
    }

    public void setColorLookupTable(Bitmap bitmap, float x, float y){

        if(bitmap == null)
            return;

        LayerBitmap lb = getLayerBitmap(bitmap);
        mRealRenderer.setColorLookupTable(lb.texName, x, y);
    }

    public void clearMask() {
        mRealRenderer.clearMask();
    }

    public void clearMask(int color) {
        mRealRenderer.clearMasktoColor(color);
    }

    public void preCacheBitmap(Bitmap bitmap) {

        if(bitmap == null)
            return;

        getLayerBitmap(bitmap);
    }

    public void setColorMatrix(ColorMatrix colorMatrix) {
        mRealRenderer.setColorMatrix(colorMatrix.getArray());
    }

    public void hollowRect(int color, float left, float top, float right, float bottom, float borderWidth) {

        fillRect(color, left, top, left + borderWidth, bottom);
        fillRect(color, right - borderWidth, top, right, bottom);
        fillRect(color, left + borderWidth, top, right - borderWidth, top + borderWidth);
        fillRect(color, left + borderWidth, bottom - borderWidth, right - borderWidth, bottom);
    }



    public void clearMaskRegion(int color, float left, float top, float right, float bottom) {

        if( mFillRectBitmap==null ) {
            mFillRectBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
            mFillRectBitmap.eraseColor(0xFFFFFFFF);
        }

        LayerBitmap lb = getLayerBitmap(mFillRectBitmap);
        mRealRenderer.clearMaskRegion(lb.texName, color, left, top, right, bottom);
    }

    public void fillRect(int color, float left, float top, float right, float bottom) {

        if( mFillRectBitmap==null ) {
            mFillRectBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
            mFillRectBitmap.eraseColor(0xFFFFFFFF);
        }

        LayerBitmap lb = getLayerBitmap(mFillRectBitmap);
        mRealRenderer.fillRect(lb.texName, color, left, top, right, bottom);
    }

    public ColorMatrix getColorMatrix() {
        return mRealRenderer.getColorMatrix();
    }

    public void save() {
        mRealRenderer.save();
    }

    public void restore() {
        mRealRenderer.restore();
    }

    public void setZTestMode(){
        mRealRenderer.setZTestMode();
    }

    public void releaseZTestMode(){

        mRealRenderer.releaseZTest();
    }

    public void setZTest(){
        mRealRenderer.setZTest();
    }

    public void releaseZTest(){
        mRealRenderer.releaseZTest();
    }

    public void translate(float x, float y, float z) {
        mRealRenderer.translate(x, y, z);
    }

    public void translate(float x, float y) {
        mRealRenderer.translateXY(x, y);
    }

    public void scale(float x, float y, float z) {
        mRealRenderer.scaleXYZ(x, y, z);
    }

    public void scale(float x, float y) {
        mRealRenderer.scaleXY(x,y);
    }

    public void scale(float x, float y, float cx, float cy) {
        mRealRenderer.scale(x, y, cx, cy);
    }

    public void rotateAroundAxis(float a, float x, float y, float z) {
        mRealRenderer.rotateAroundAxis(a,x,y,z);
    }

    public float[] getMatrix() {
        return mRealRenderer.getMatrix();
    }

    public void rotate(float a, float cx, float cy) {
        mRealRenderer.rotate(a,cx,cy);
    }

    public void setAlpha(float alpha) {
        mRealRenderer.setAlpha(alpha);
    }

    public float getAlpha() {
        return mRealRenderer.getAlpha();
    }

    public void setCurrentTime(int currentTime) {
        mRealRenderer.setCurrentTime(currentTime);
    }

    void setFrameDimensions( int frameWidth, int frameHeight ) {
        mRealRenderer.setFrameDimensions(frameWidth,frameHeight);
    }

    void setScreenDimensions(int screenWidth, int screenHeight){
        mRealRenderer.setScreenDimenisions(screenWidth,screenHeight);
    }

    void setShaderAndParams(boolean isExport)    {
        mRealRenderer.setShaderAndParam(isExport);
    }

    public void resetMatrix(){
        mRealRenderer.resetMatrix();
    }

    void preRender() {

        freeUnusedLayerBitmaps();
        freeUnusedEffectTextures();
        mRealRenderer.preRender();
    }

    void postRender() {

        freeUnusedLayerBitmaps();
        freeUnusedEffectTextures();
        mRealRenderer.postRender();
    }
//    void destroy() {
//        clearImageCache();
//    }

    public RenderMode getRenderMode(){
        return mRealRenderer.getRenderMode() == 0 ? RenderMode.Preview: RenderMode.Export;
    }

    public void drawNexEDL(int effect_id, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, int mode){
        mRealRenderer.drawNexEDL(effect_id, usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mode);
    }

    public void setBlendFuncNormal(){

        mRealRenderer.setBlendFuncNormal();
    }

    public void setBlendFuncMultiply(){

        mRealRenderer.setBlendFuncMultiply();
    }
}