package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.Bitmap;
import android.graphics.ColorMatrix;
import android.opengl.Matrix;

public interface LayerRendererInterface {

    public float getOutputWidth();
    public float getOutputHeight();
    public int getCurrentTime();
    public void drawBitmap(Bitmap bitmap, float left, float top, float right, float bottom);
    public void drawBitmap(Bitmap bitmap, float x, float y);
    public void drawBitmap(Bitmap bitmap);
    public void preCacheBitmap(Bitmap bitmap);
    public void setHue(float hue);
    public float getHue();
    public void setColorMatrix(ColorMatrix colorMatrix);
    public ColorMatrix getColorMatrix();
    public void save();
    public void restore();
    public void translate(float x, float y, float z);
    public void translate(float x, float y);
    public void scale(float x, float y, float z);
    public void scale(float x, float y);
    public void scale(float x, float y, float cx, float cy);
    public void rotateAroundAxis(float a, float x, float y, float z);
    public float[] getMatrix();
    public void rotate(float a, float cx, float cy);
    public void setAlpha(float alpha);
    public void setCurrentTime(int currentTime);

}

