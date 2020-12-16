package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.ColorMatrix;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

/**
 * Created by damian.lee on 2016-04-20.
 */
public class NexLayerRenderer {

    public native void createRenderer();
    public native void removeRenderer();

    public native float[] getTexMatrix();
    public native float getOutputWidth();
    public native float getOutputHeight();
    public native int getCurrentTime();
    public native void setColorMatrix(float[] colormatrix_array);
    public native ColorMatrix getColorMatrix();
    public native void save();
    public native void restore();
    public native void translate(float x, float y, float z);
    public native void translateXY(float x, float y);
    public native void scale(float x, float y, float cx, float cy);
    public native void scaleXY(float x, float y);
    public native void scaleXYZ(float x, float y, float z);
    public native void rotateAroundAxis(float a, float x, float y, float z);
    public native float[] getMatrix();
    public native void rotate(float a, float cx, float cy);
    public native void setAlpha(float alpha);
    public native void setCurrentTime(int currentTime);
    public native void setShaderAndParam(boolean is_export);
    public native void setFrameDimensions(int width, int height);
    public native void setScreenDimenisions(int width, int height);
    public native void preRender();
    public native void postRender();
    public native void setChromakeyViewMaskEnabled(boolean maskOnOff);
    public native int getChromakeyViewMaskEnabled();
    public native void setChromakeyEnabled(boolean enable);
    public native void setChromakeyColor(int color, float clipFg, float clipBg, float bx0, float by0, float bx1, float by1);
    public native void setHBlurEnabled(boolean enable);
    public native void setVBlurEnabled(boolean enable);
    public native void setMosaicEnabled(boolean enable);
    public native void setEffectStrength(float strength);
    public native void setEffectTextureSize(int width, int height);
    public native float getScreenDimensionWidth();
    public native float getScreenDimensionHeight();
    public native void setMaskEnabled(boolean enable);
    public native boolean getMaskEnabled();
    public native void setMaskTexID(int texid);
    public native float getAlpha();
    public native void setAlphaTestValue(float alpha);
    public native void resetMatrix();
    public native void setRenderTarget(int rendertarget);
    public native void setZTestMode();
    public native void setZTest();
    public native void releaseZTest();
    public native void releaseZTestMode();
    public native void clearMask();
    public native void clearMasktoColor(int color);
    public native void setRenderMode(boolean is_export);
    public native int  getRenderMode();
    public native void setLUT(int tex_id);
    public native void setLUTEnable(boolean enable);
    public native void drawBitmap(int img_id, float left, float top, float right, float bottom, int flipmode);
    public native void drawBitmapRepeat(int img_id, float left, float top, float right, float bottom, float repeat_x, float repeat_y);
    public native void drawRenderItem(int img_id, String uesString, int current_time, int start_time, int end_time, float x, float y, float w, float h, float alpha, boolean mask_enabled);
    public native void drawRenderItemBlend(int img_id, int tex_id_for_second, int type, String uesString, int current_time, int start_time, int end_time, float x, float y, float w, float h, float alpha, boolean mask_enabled);
    public native void drawRenderItemBlendOverlay(int effect_id_for_blend, int effect_id, String usersettings_for_blend, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, boolean mask_enabled);
    public native void drawRenderItemBitmap(int img_id, int tex_id_for_second, String uesString, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, boolean mask_enabled);
    public native void fillRect(int fill_rect_tex_id, int color, float left, float top, float right, float bottom);
    public native void clearMaskRegion(int fill_rect_tex_id, int color, float left, float top, float right, float bottom);
    public native void presetForDrawing(float left, float top, float right, float bottom, int type);
    public native float getEffectStrength();
    public native void drawVideo(int clipid, float x, float y, float w, float h);
    public native void drawDirect(int tex_name, int type, float x, float y, float w, float h, int flipMode);
    public native void setEffectTextureWidth(float width);
    public native void setEffectTextureHeight(float height);
    public native float getEffectTextureWidth();
    public native float getEffectTextureHeight();
    public native int getRenderTarget();
    public native void setColorLookupTable(int img_id, float x, float y);
    public native void setHue(float hue);
    public native float getHue();
    public native void drawNexEDL(int effect_id, String usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, int mode);
    public native void setBlendFuncNormal();
    public native void setBlendFuncMultiply();

    // static {
    //     try {
    //         System.loadLibrary("stlport_shared");
    //         Log.d("NexLayerRenderer", "loadLibrary nexeditorsdk");
    //         System.loadLibrary("nexeditorsdk");
    //     } catch ( UnsatisfiedLinkError exc ) {
    //         if(LL.E) Log.e("LayerRenderer", "[LayerRenderer.java] nexeditor load failed : " + exc);
    //         ;;
    //     }
    // }

}
