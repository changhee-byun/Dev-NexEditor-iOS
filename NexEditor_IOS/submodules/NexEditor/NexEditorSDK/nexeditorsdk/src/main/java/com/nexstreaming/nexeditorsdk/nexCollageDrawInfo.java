/******************************************************************************
 * File Name        : nexCollage.java
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
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.provider.MediaStore;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.utils.facedetect.FaceInfo;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

class nexCollageDrawInfo implements nexCollageInfo, nexCollageInfoDraw {
    private static String TAG="nexCollageDrawInfo";

    private static final String TAG_START = "start";
    private static final String TAG_END = "end";
    private static final String TAG_POSITION = "position";
    private static final String TAG_DRAW_ID = "draw_id";
    private static final String TAG_SOURCE_TYPE = "source_type";
    private static final String TAG_SOURCE_DEFAULT = "source_default";
    private static final String TAG_LUT = "lut";
    private static final String TAG_AUDIO_RES = "audio_res";
    private static final String TAG_AUDIO_RES_POS = "audio_res_pos";
    private static final String TAG_DRAW_WIDTH = "draw_width";
    private static final String TAG_DRAW_HEIGHT = "draw_height";

    private float start;
    private float end;
    private RectF position;
    private List<PointF> polygon = new ArrayList<>();
    private String id;
    private String sourceType;
    private String sourceDefault;
    private String lut;
    private String audio_res;
    private String audio_res_pos;
    private int draw_width = 0;
    private int draw_height = 0;

    protected nexClip baseClip;

    protected RectF scaledRect;
    private float displayRatio;

    private Rect original_rect_;
    private RectF display_rect_;
    private RectF scale_reference_rect_;

    public RectF getScaleReferenceRect(){

        return scale_reference_rect_;
    }

    public void setScaleReferenceRect(RectF rect){

        scale_reference_rect_ = null;
        if(rect != null){

            scale_reference_rect_ = new RectF(rect);
        }
    }

    protected nexCollage.CollageInfoChangedListener infoChanged;

    protected void setInfoChangedListener(nexCollage.CollageInfoChangedListener listener) {
        infoChanged = listener;
    }

    public boolean isSystemSource() {
        if( sourceType.compareTo("system") == 0 || sourceType.compareTo("system_mt") == 0)
            return true;
        return false;
    }

    public boolean isMotionTrackedSource() {
        if( sourceType.compareTo("system_mt") == 0)
            return true;
        return false;
    }

    public String getSystemResource(Context context) {
        return  nexAssetPackageManager.getAssetPackageMediaPath(context, sourceDefault);
    }

    public String getId() { return id; }

    public RectF getRectangle() {
        return position;
    }

    public List<PointF> getPositions() {
        return polygon;
    }

    public int getTagID() {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return 0;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        return info.getID();
    }

//    public boolean setTranslate(float xMov, float yMov) {
//        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
//            return false;
//        }
//
//        int width = baseClip.getWidth();
//        int height = baseClip.getHeight();
//
//        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
//            width = baseClip.getHeight();
//            height = baseClip.getWidth();
//        }
//
//        nexDrawInfo info = baseClip.getDrawInfos().get(0);
//        Rect rect = info.getStartRect();
//        Rect r = new Rect(rect);
//
//        nexCollage.restoreDimension(r, width, height);
//        int newX = (int)(r.width()*xMov);
//        int newY = (int)(r.height()*yMov);
//
//
//        Log.d(TAG, String.format("setTranslate 1: %f %f : %d %d", xMov, yMov, newX, newY));
//
//
//        int oldX = (info.getTranslateX() * width) / nexCrop.ABSTRACT_DIMENSION;
//        int oldY = (info.getTranslateY() * height) / nexCrop.ABSTRACT_DIMENSION;
//
//
//        oldX = newX;
//        oldY = newY;
//
//        oldX = oldX*nexCrop.ABSTRACT_DIMENSION/width;
//        oldY = oldY*nexCrop.ABSTRACT_DIMENSION/height;
//
//        oldX = oldX + info.getTranslateX();
//        oldY = oldY + info.getTranslateY();
//
//
//
//        Log.d(TAG, String.format("setTranslate 2: %d %d %d %d", info.getTranslateX(), info.getTranslateY(), oldX, oldY));
//
//        // info.setTranslate(info.getTranslateX() + xMov, info.getTranslateY() + yMov);
//        info.setTranslate(oldX, oldY);
//
//        if( infoChanged != null ) infoChanged.DrawInfoChanged(info);
//        return true;
//    }


    private Rect getAABB(nexDrawInfo info, Rect r, int width, int height){
        int user_translate_x = info.getUserTranslateX() * width / nexCrop.ABSTRACT_DIMENSION;
        int user_translate_y = info.getUserTranslateY() * height / nexCrop.ABSTRACT_DIMENSION;

        float cx = (float)r.width() * 0.5f;
        float cy = (float)r.height() * 0.5f;
        float left = -cx;
        float top = -cy;
        float right = cx;
        float bottom = cy;

        float user_rotate = -(float)Math.toRadians((double)info.getUserRotateState());
        float cos_val = (float)Math.cos(user_rotate);
        float sin_val = (float)Math.sin(user_rotate);

        float pos0_x = cos_val * left - sin_val * top;
        float pos0_y = sin_val * left + cos_val * top;
        float pos1_x = cos_val * right - sin_val * top;
        float pos1_y = sin_val * right + cos_val * top;
        float pos2_x = cos_val * left - sin_val * bottom;
        float pos2_y = sin_val * left + cos_val * bottom;
        float pos3_x = cos_val * right - sin_val * bottom;
        float pos3_y = sin_val * right + cos_val * bottom;
        
        float new_lt_x = Math.min(Math.min(Math.min(pos0_x, pos1_x), pos2_x), pos3_x);
        float new_lt_y = Math.min(Math.min(Math.min(pos0_y, pos1_y), pos2_y), pos3_y);
        float new_rb_x = Math.max(Math.max(Math.max(pos0_x, pos1_x), pos2_x), pos3_x);
        float new_rb_y = Math.max(Math.max(Math.max(pos0_y, pos1_y), pos2_y), pos3_y);

        Rect aabb = new Rect();

        aabb.left = (int)(new_lt_x + cx) + user_translate_x + r.left;
        aabb.top = (int)(new_lt_y + cy) + user_translate_y + r.top;
        aabb.right = (int)(new_rb_x + cx) + user_translate_x + r.left;
        aabb.bottom = (int)(new_rb_y + cy) + user_translate_y + r.top;

        return aabb;
    }

    private void fitScreen(){
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        if(original_rect_ == null){

            original_rect_ = new Rect(info.getStartRect());
            nexCollage.restoreDimension(original_rect_, width, height);
        }

        Rect r = new Rect(original_rect_);

        Rect aabb = getAABB(info, r, width, height);

        float center_x = aabb.exactCenterX();
        float center_y = aabb.exactCenterY();

        float min_scale = 1.0f;
        float half_width = (float)aabb.width() * 0.5f;
        float half_height = (float)aabb.height() * 0.5f;

        if(aabb.left < 0){
            min_scale = Math.min(center_x / half_width, min_scale);
        }

        if(aabb.right > width){
            min_scale = Math.min(((float)width - center_x) / half_width, min_scale);
        }

        if(aabb.top < 0){
            min_scale = Math.min(center_y / half_height, min_scale);
        }

        if(aabb.bottom > height){
            min_scale = Math.min(((float)height - center_y) / half_height, min_scale);
        }

        float r_half_height = (float)r.height() * 0.5f * min_scale;
        float r_half_width = r_half_height * displayRatio;

        float cx = r.exactCenterX();
        float cy = r.exactCenterY();

        display_rect_.left = cx - r_half_width;
        display_rect_.right = cx + r_half_width;
        display_rect_.top = cy - r_half_height;
        display_rect_.bottom = cy + r_half_height;

        display_rect_.round(r);

        nexCollage.updateDimension(r, width, height);

        info.setStartRect(r);
        info.setEndRect(r);
    }

    private void fitStandardScreenPos(){
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        Rect r = new Rect();
        r.set((int)display_rect_.left, (int)display_rect_.top, (int)display_rect_.right, (int)display_rect_.bottom);

        Rect aabb = getAABB(info, r, width, height);

        int trans_x = 0;
        int trans_y = 0;

        if(aabb.left < 0){
            trans_x -= aabb.left;
        }

        if(aabb.right > width){
            trans_x -= (aabb.right - width);
        }

        if(aabb.top < 0){
            trans_y -= aabb.top;
        }

        if(aabb.bottom > height){
            trans_y -= (aabb.bottom - height);
        }

        if(trans_x == 0 && trans_y == 0)
            return;

        display_rect_.left += trans_x;
        display_rect_.right += trans_x;
        display_rect_.top += trans_y;
        display_rect_.bottom += trans_y;

        display_rect_.round(r);

        nexCollage.updateDimension(r, width, height);

        info.setStartRect(r);
        info.setEndRect(r);
    }    

    private boolean checkTranformOk(){
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        Rect r = new Rect(info.getStartRect());
        nexCollage.restoreDimension(r, width, height);

        Rect aabb = getAABB(info, r, width, height);

        Log.d(TAG, "aabb:" + aabb.toString() + " width:" + width + " height:" + height +" r:" + r.toString());

        if(aabb.width() > width)
            return false;

        if(aabb.height() > height){
            return false;
        }

        Log.d(TAG, "aabb checkTranformOk");

        return true;
    }

    private void fitTranslate(){
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        Rect r = new Rect(info.getStartRect());
        nexCollage.restoreDimension(r, width, height);

        Rect aabb = getAABB(info, r, width, height);

        int trans_x = 0;
        int trans_y = 0;

        if(aabb.left < 0){
            trans_x -= aabb.left;
        }

        if(aabb.right > width){
            trans_x -= (aabb.right - width);
        }

        if(aabb.top < 0){
            trans_y -= aabb.top;
        }

        if(aabb.bottom > height){
            trans_y -= (aabb.bottom - height);
        }

        if(trans_x == 0 && trans_y == 0)
            return;

        int user_translate_x = info.getUserTranslateX() * width / nexCrop.ABSTRACT_DIMENSION;
        int user_translate_y = info.getUserTranslateY() * height / nexCrop.ABSTRACT_DIMENSION;

        int newX = (user_translate_x + trans_x)*nexCrop.ABSTRACT_DIMENSION/width;
        int newY = (user_translate_y + trans_y)*nexCrop.ABSTRACT_DIMENSION/height;

        info.setUserTranslate(newX, newY);        
    }

    private void calcUserScale(){

        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        if( scale_reference_rect_ == null )
            scale_reference_rect_ = new RectF(display_rect_);

        float scale_chk = (float)Math.sqrt(
             (double)(scale_reference_rect_.width() * scale_reference_rect_.height()) / (double)(display_rect_.width() * display_rect_.height())
             );

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        info.setRealScale(scale_chk);
    }

    private void calcUserScale(Rect sr){

        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        if( scale_reference_rect_ == null )
            scale_reference_rect_ = new RectF(display_rect_);

        float scale_chk = (float)Math.sqrt(
             (double)(scale_reference_rect_.width() * scale_reference_rect_.height()) / (double)(sr.width() * sr.height())
             );

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        info.setRealScale(scale_chk);
    }

    public boolean setTranslate(int xMov, int yMov, int screenWidth, int screenHeight) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        float user_rotate = -(float)Math.toRadians((double)info.getUserRotateState());
        float cos_val = (float)Math.cos(user_rotate);
        float sin_val = (float)Math.sin(user_rotate);

        float rxMov = (float)xMov * cos_val - (float)yMov * sin_val;
        float ryMov = (float)xMov * sin_val + (float)yMov * cos_val;

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        if(display_rect_ == null){
            Rect sr = new Rect(info.getStartRect());
            nexCollage.restoreDimension(sr, width, height);
            display_rect_ = new RectF(sr);
        }

        float draw_scale_chk = (float)Math.sqrt( (double) (display_rect_.width() * display_rect_.height()) / (double) (draw_width * draw_height) );

        xMov = (int) (rxMov * draw_scale_chk);
        yMov = (int) (ryMov * draw_scale_chk);

        int newX = xMov*nexCrop.ABSTRACT_DIMENSION/width + info.getUserTranslateX();
        int newY = yMov*nexCrop.ABSTRACT_DIMENSION/height + info.getUserTranslateY();

        info.setUserTranslate(newX, newY);

        fitTranslate();

        if( infoChanged != null ) infoChanged.DrawInfoChanged(info);
        return true;
    }

    public int getRotate()
    {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return 0;
        }
        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        return info.getUserRotateState();
    }

    public boolean setRotate(int state, int angle) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }
        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        Log.d(TAG, String.format("setRotate: %d %d", state, angle));

        if(state == 1 || state == 2 ) {
            //original_rect_ = null;
            return true;
        }

        if(display_rect_ == null){

            int width = baseClip.getWidth();
            int height = baseClip.getHeight();

            if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
                width = baseClip.getHeight();
                height = baseClip.getWidth();
            }

            Rect sr = new Rect(info.getStartRect());
            nexCollage.restoreDimension(sr, width, height);
            display_rect_ = new RectF(sr);
        }

        info.setUserRotateState(angle);

        fitScreen();

        calcUserScale();

        if( infoChanged != null ) infoChanged.DrawInfoChanged(info);

        return true;
    }

    public boolean setFlip(int direction) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int rotate = info.getRotateState();
        if( direction == 0 ) {
            rotate = rotate & ~(0x00030000);
        } else {
            if ( (direction & 1) == 1 ) {
                if( (rotate & 0x00010000) == 0x00010000 )
                    rotate = rotate & (~0x00010000);
                else
                    rotate = rotate | 0x00010000;
            }
            if( (direction & 2) == 2 ) {
                if( (rotate & 0x00020000) == 0x00020000 )
                    rotate = rotate & (~0x00020000);
                else
                    rotate = rotate | (0x00020000);
            }
        }

        info.setRotateState(rotate);

        if( infoChanged != null ) infoChanged.DrawInfoChanged(info);

        return true;
    }

    public void updatePosition(boolean calcscale) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();
        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        if(scale_reference_rect_ == null){
            Rect sr = new Rect(0, 0, width, height);
            nexCollage.shrinkToAspect(sr, displayRatio);
            setScaleReferenceRect(new RectF(sr));
        }

        float cx = scale_reference_rect_.centerX();
        float cy = scale_reference_rect_.centerY();

        Rect sr = new Rect(info.getStartRect());
        nexCollage.restoreDimension(sr, width, height);

        if(calcscale){
            calcUserScale(sr);
        }

        float user_scale = info.getRealScale();

        float w = scale_reference_rect_.width() / user_scale;
        float h = scale_reference_rect_.height() / user_scale;

        float cur_cx = sr.exactCenterX();
        float cur_cy = sr.exactCenterY();

        sr.left = (int) (cx - w / 2.0);
        sr.top = (int)(cy - h / 2.0);
        sr.right = (int)(cx + w / 2.0);
        sr.bottom = (int)(cy + h / 2.0);

        nexCollage.shrinkToAspect(sr, displayRatio);

        int trans_x = (int)(cur_cx - sr.exactCenterX());
        int trans_y = (int)(cur_cy - sr.exactCenterY());

        sr.left += trans_x;
        sr.right += trans_x;
        sr.top += trans_y;
        sr.bottom += trans_y;

        display_rect_ = new RectF(sr);

        nexCollage.updateDimension(sr, width, height);

        info.setStartRect(sr);
        info.setEndRect(sr);
        original_rect_ = null;
        
        fitStandardScreenPos();
        fitScreen();
        
        if(calcscale){
            calcUserScale();
            original_rect_ = null;
        }
    }

    public void scaler(float scale) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        int width = baseClip.getWidth();
        int height = baseClip.getHeight();

        if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
            width = baseClip.getHeight();
            height = baseClip.getWidth();
        }

        if( scale_reference_rect_ == null )
            scale_reference_rect_ = new RectF(display_rect_);

        float scale_chk = (float)Math.sqrt(
             (double)(scale_reference_rect_.width() * scale_reference_rect_.height()) / (double)(display_rect_.width() * display_rect_.height())
             );

        Log.d(TAG, "scale_chk:" + scale_chk + " display_rect:" + display_rect_.toString());

        float cx = display_rect_.centerX();
        float cy = display_rect_.centerY();

        float half_w = display_rect_.width() * 0.5f;
        float half_h = display_rect_.height() * 0.5f;

        if(scale > 0){

            if(scale_chk > 2.0f)
                return;

            half_h *= 0.97f;
            half_w = half_h * displayRatio;
        }
        else{

            if(scale_chk < 0.2f)
                return;

            half_h *= 1.03f;
            half_w = half_h * displayRatio;
        }

        display_rect_.left = (cx - half_w);
        display_rect_.right = (cx + half_w);
        display_rect_.top = (cy - half_h);
        display_rect_.bottom = (cy + half_h);

        Rect r = new Rect();
        display_rect_.round(r);

        nexCollage.updateDimension(r, width, height);

        info.setStartRect(r);
        info.setEndRect(r);
        original_rect_ = null;
    }

    public boolean setScale(float scale) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        if( scale == 0 ) {
            return true;
        }

        Log.d(TAG, String.format("setScale %f", scale));

        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        if(display_rect_ == null){

            int width = baseClip.getWidth();
            int height = baseClip.getHeight();

            if( baseClip.getRotateInMeta() == 90 || baseClip.getRotateInMeta() == 270 ) {
                width = baseClip.getHeight();
                height = baseClip.getWidth();
            }

            Rect sr = new Rect(info.getStartRect());
            nexCollage.restoreDimension(sr, width, height);
            display_rect_ = new RectF(sr);
            scale_reference_rect_ = new RectF(display_rect_);
        }

        RectF backup_display_rect = new RectF(display_rect_);
        Rect backup_sr = new Rect(info.getStartRect());
        int user_translate_x = info.getUserTranslateX();
        int user_translate_y = info.getUserTranslateY();

        scaler(scale);
        fitTranslate();

        if(!checkTranformOk()){
            display_rect_ = backup_display_rect;
            info.setStartRect(backup_sr);
            info.setEndRect(backup_sr);
            info.setUserTranslate(user_translate_x, user_translate_y);
        }

        calcUserScale();

        if( infoChanged != null ) infoChanged.DrawInfoChanged(info);

        return true;
    }

    public String getLut() {

        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return null;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        if( info != null )
            return info.getUserLUT();
        return null;
    }

    public boolean setLut(String lut) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }
        nexDrawInfo info = baseClip.getDrawInfos().get(0);

        if( lut == null || lut.compareTo("none") == 0 ) {

            info.setUserLUT(null);
            info.setLUT(0);

            if( infoChanged != null ) infoChanged.DrawInfoChanged(info);
            return true;
        }

        if( lut.compareTo("default") == 0 ) {

            info.setUserLUT(null);
            if( getDefaultLut() != null && getDefaultLut().compareTo("none") != 0 ) {
                nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(getDefaultLut());
                if( colorEffect != null)
                    info.setLUT( colorEffect.getLUTId() );
            }
            else {
                info.setLUT(0);
            }

            if( infoChanged != null ) infoChanged.DrawInfoChanged(info);
            return true;
        }

        if( lut.compareTo("none") != 0 || lut.compareTo("null") != 0 ) {
            nexColorEffect colorEffect = nexColorEffect.getLutColorEffect(lut);
            if( colorEffect != null) {
                int lutID = colorEffect.getLUTId();
                info.setLUT(lutID);
                info.setUserLUT(lut);
                // Log.d(TAG, String.format("Collage setLut %d", lutID));
                if( infoChanged != null ) infoChanged.DrawInfoChanged(info);
                return true;
            }
        }
        return false;
    }

    public boolean getVisible() {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        if( info != null ) {
            if( info.getBrightness() == -255 ) {
                return false;
            }
            return true;
        }
        return false;
    }


    public boolean setVisible(boolean visible) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }
        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        if( info != null ) {

            if (visible) {
                info.setBrightness(0);
            } else {
                info.setBrightness(-255);
            }
            if (infoChanged != null) infoChanged.DrawInfoChanged(info);
            return true;
        }
        return false;
    }

    public boolean changeSource(nexClip clip) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        nexDrawInfo info = baseClip.getDrawInfos().get(0);
        if( info == null ) return false;

        clip.clearDrawInfos();
        clip.addDrawInfo(info);

        clip.mStartTime = baseClip.mStartTime;
        clip.mEndTime = baseClip.mEndTime;

        if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            clip.setAudioOnOff(false);
        }
        else if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
            clip.setImageClipDuration(baseClip.getProjectDuration());
        }
        else {
            return false;
        }

        int iWidth = clip.getWidth();
        int iHeight = clip.getHeight();

        if( clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270 ) {
            iWidth = clip.getHeight();
            iHeight = clip.getWidth();
        }

        Rect sr = new Rect(0, 0, iWidth, iHeight);
        Rect fr = new Rect(0, 0, 1, 1);

        if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE)
        {
            RectF _sbounds = new RectF();
            if( infoChanged != null ) {
                _sbounds = infoChanged.FaceRect(clip.getPath());
            }

            sr.set((int)(_sbounds.left*iWidth), (int)(_sbounds.top*iHeight), (int)(_sbounds.right*iWidth), (int)(_sbounds.bottom*iHeight));

            if(sr.isEmpty()){
                sr = new Rect(0, 0, iWidth, iHeight);
            }else {
                fr.set((int)(_sbounds.left*iWidth), (int)(_sbounds.top*iHeight), (int)(_sbounds.right*iWidth), (int)(_sbounds.bottom*iHeight));

                int addspace = (sr.right - sr.left)/2;
                sr.left -= addspace;
                sr.right += addspace;
                if (sr.left < 0) {
                    sr.left = 0;
                }
                if (sr.right > iWidth) {
                    sr.right = iWidth;
                }
                addspace = (sr.bottom - sr.top)/2;
                sr.top -= addspace;
                sr.bottom += addspace;
                if (sr.top < 0) {
                    sr.top = 0;
                }
                if (sr.bottom > iHeight) {
                    sr.bottom = iHeight;
                }
            }
        }

        nexCollage.shrinkToAspect(sr, displayRatio);

        nexCollage.updateDimension(sr, iWidth, iHeight);
        nexCollage.updateDimension(fr, iWidth, iHeight);

        info.setStartRect(sr);
        info.setEndRect(sr);

        info.setFaceRect(fr);

        if(clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO){
            info.setRotateState(clip.getRotateInMeta());
        }
        else {
            info.setRotateState(0);
        }
        info.setUserRotateState(0);
        info.setUserTranslate(0, 0);
        info.setRealScale(1.0f);
        info.setBrightness(0);

        // info.setLUT(0);
        // info.setCustomLUTA(0);
        // info.setCustomLUTB(0);
        // info.setCustomLUTPower(0);

        original_rect_ = null;
        display_rect_ = null;
        scaledRect = new RectF(0, 0, iWidth, iHeight);

        nexClip tmpClip = baseClip;
        baseClip = clip;
        scale_reference_rect_ = null;
        updatePosition(true);

        if( infoChanged != null ) {
            infoChanged.SourceChanged(tmpClip, baseClip);
            return true;
        }
        return false;
    }



    public boolean updateSource(nexClip clip, nexDrawInfo info) {
        if (clip == null || info == null || baseClip == null || baseClip.getDrawInfos() == null
                || baseClip.getDrawInfos().size() <= 0) {
            return false;
        }

        clip.clearDrawInfos();
        clip.addDrawInfo(info);

        clip.mStartTime = baseClip.mStartTime;
        clip.mEndTime = baseClip.mEndTime;

        if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
            clip.setImageClipDuration(baseClip.getProjectDuration());
        }

        int iWidth = clip.getWidth();
        int iHeight = clip.getHeight();

        if (clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270) {
            iWidth = clip.getHeight();
            iHeight = clip.getWidth();
        }

        scaledRect = new RectF(0, 0, iWidth, iHeight);
        baseClip = clip;

        Rect sr = new Rect(0, 0, iWidth, iHeight);
        Rect fr = new Rect(0, 0, 1, 1);

        if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
            RectF _sbounds = new RectF();
            if (infoChanged != null) {
                _sbounds = infoChanged.FaceRect(clip.getPath());
            }

            sr.set((int) (_sbounds.left * iWidth), (int) (_sbounds.top * iHeight), (int) (_sbounds.right * iWidth),
                    (int) (_sbounds.bottom * iHeight));

            if (sr.isEmpty()) {
                sr = new Rect(0, 0, iWidth, iHeight);
            } else {

                fr.set((int) (_sbounds.left * iWidth), (int) (_sbounds.top * iHeight), (int) (_sbounds.right * iWidth),
                        (int) (_sbounds.bottom * iHeight));

                int addspace = (sr.right - sr.left) / 2;
                sr.left -= addspace;
                sr.right += addspace;
                if (sr.left < 0) {
                    sr.left = 0;
                }
                if (sr.right > iWidth) {
                    sr.right = iWidth;
                }
                addspace = (sr.bottom - sr.top) / 2;
                sr.top -= addspace;
                sr.bottom += addspace;
                if (sr.top < 0) {
                    sr.top = 0;
                }
                if (sr.bottom > iHeight) {
                    sr.bottom = iHeight;
                }
            }
        }

        nexCollage.shrinkToAspect(sr, displayRatio);

        nexCollage.updateDimension(sr, iWidth, iHeight);
        nexCollage.updateDimension(fr, iWidth, iHeight);

        info.setStartRect(sr);
        info.setEndRect(sr);
        info.setFaceRect(fr);

        original_rect_ = null;
        display_rect_ = null;
        scaledRect = new RectF(0, 0, iWidth, iHeight);
        scale_reference_rect_ = null;
        updatePosition(false);
        return true;
    }    

    public nexClip getBindSource() {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return null;
        }
        return baseClip;
    }

    public boolean setAudioVolume(int volume) {
        if( baseClip == null || baseClip.getDrawInfos() == null || baseClip.getDrawInfos().size() <= 0 ) {
            return false;
        }

        if( baseClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE )
            return false;
            
        if( volume == 0 ) {
            baseClip.setAudioOnOff(false);
            return true;
        }

        baseClip.setAudioOnOff(true);
        baseClip.setClipVolume(volume);
        return true;
    }

    public float getStart() {
        return start;
    }

    public float getEnd() {
        return end;
    }

    public String getDefaultLut() {
        return lut;
    }

    float isLeft( PointF P0, PointF P1, PointF P2 ) {
        return ( (P1.x - P0.x) * (P2.y - P0.y) - (P2.x -  P0.x) * (P1.y - P0.y) );
    }

    int wn_PnPoly( PointF P, List<PointF> poligons)
    {
        int    wn = 0;    // the  winding number counter
        for( int i = 0; i < poligons.size() - 1; i++) {
            if (poligons.get(i).y <= P.y) {          // start y <= P.y
                if (poligons.get(i+1).y  > P.y)      // an upward crossing
                    if (isLeft( poligons.get(i), poligons.get(i+1), P) > 0)  // P left of  edge
                        ++wn;            // have  a valid up intersect
            }
            else {                        // start y > P.y (no test needed)
                if (poligons.get(i+1).y  <= P.y)     // a downward crossing
                    if (isLeft( poligons.get(i), poligons.get(i+1), P) < 0)  // P right of  edge
                        --wn;            // have  a valid down intersect
            }
        }
        return wn;
    }

    public boolean isContains(float x, float y) {

        if( polygon != null && polygon.size() > 0 ) {

            PointF p = new PointF(x, y);
            int wn = wn_PnPoly(p, polygon);
            if( wn > 0 )
                return true;

            return false;
        }

        if( position != null )
            return position.contains(x, y);

        return false;
    }

    protected void setBaseClip(nexClip clip) {
        baseClip = clip;
        if( baseClip != null ) {
            int iWidth = clip.getWidth();
            int iHeight = clip.getHeight();

            if( clip.getRotateInMeta() == 90 || clip.getRotateInMeta() == 270 ) {
                iWidth = clip.getHeight();
                iHeight = clip.getWidth();
            }

            scaledRect = new RectF(0, 0, iWidth, iHeight);
        }
    }

    protected void setDisplayRatio(float ratio) {
        displayRatio = ratio;
    }

    protected String getContentTime(String format)
    {
        if( baseClip != null ) {

            File file = new File(baseClip.getPath());
            Date lastModDate = new Date(file.lastModified());

            SimpleDateFormat df = new SimpleDateFormat(format);
            return df.format(lastModDate);
        }
        return "";
    }

    protected String applySoundEffect2Clip(nexProject project, int duration) {
        int startInfo = (int)(start * duration);
        int endInfo = (int)(end * duration);
        int durationInfo = endInfo - startInfo;

        if( audio_res != null && audio_res.length() > 0 ) {
            if( !audio_res.equals("none") ) {
                project.updateProject();

                String file = nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), audio_res);
                nexClip tmpClip = nexClip.getSupportedClip(file);
                if (tmpClip != null) {
                    int audio_res_dur = tmpClip.getTotalTime();
                    float fPos = Float.parseFloat(audio_res_pos);
                    int start_pos = (int)(duration * fPos);
                    tmpClip.setCollageDrawInfoID(id);
                    tmpClip.setAssetResource(true);
                    project.addAudio(tmpClip, startInfo + start_pos, startInfo + start_pos + audio_res_dur);
                }
            }
        }
        return null;
    }

    int getDrawWidth(){
        return draw_width;
    }

    int getDrawHeight(){
        return draw_height;
    }

    String parseDrawInfo(JSONObject object) {
        try {
            start = Float.parseFloat(object.getString(TAG_START));
            end = Float.parseFloat(object.getString(TAG_END));

            if( object.has(TAG_POSITION) ) {
                String pos = object.getString(TAG_POSITION).replace(" ", "");
                String[] position = pos.split(",");

                if (position == null) {
                    return "Wrong position data of drawinfo";
                }

                polygon.clear();
                if( position.length == 4 ) {
                    float left = Float.parseFloat(position[0]);
                    float top = Float.parseFloat(position[1]);

                    float right = Float.parseFloat(position[2]);
                    float bottom = Float.parseFloat(position[3]);

                    PointF lt = new PointF(left, top);
                    PointF rt = new PointF(right, top);
                    PointF rb = new PointF(right, bottom);
                    PointF lb = new PointF(left, bottom);

                    polygon.add(lt);
                    polygon.add(rt);
                    polygon.add(rb);
                    polygon.add(lb);
                    polygon.add(lt);

                    this.position = new RectF(left, top, right, bottom);
                }
                else {
                    float left = Float.MAX_VALUE;
                    float top = Float.MAX_VALUE;
                    float right = Float.MIN_VALUE;
                    float bottom = Float.MIN_VALUE;
                    for(int i = 0; i < position.length; i+=2 ) {
                        float x = Float.parseFloat(position[i]);
                        float y = Float.parseFloat(position[i+1]);

                        left = left > x ? x : left;
                        right = right < x ? x : right;
                        top = top > y ? y : top;
                        bottom = bottom < y ? y : bottom;

                        PointF pointF = new PointF(x, y);
                        // Log.d(TAG, "Position : " + pointF.toString());
                        polygon.add(pointF);
                    }

                    this.position = new RectF(left, top, right, bottom);
                }
            }

            id = object.getString(TAG_DRAW_ID);

            sourceType = object.getString(TAG_SOURCE_TYPE);
            sourceDefault = object.getString(TAG_SOURCE_DEFAULT);

            lut = object.getString(TAG_LUT);

            audio_res = object.getString(TAG_AUDIO_RES);
            audio_res_pos = object.getString(TAG_AUDIO_RES_POS);

            if( object.has(TAG_DRAW_WIDTH) ) {
                draw_width = Integer.parseInt(object.getString(TAG_DRAW_WIDTH));
            }

            if( object.has(TAG_DRAW_HEIGHT) ) {
                draw_height = Integer.parseInt(object.getString(TAG_DRAW_HEIGHT));
            }

        } catch (JSONException e) {
            e.printStackTrace();
            if(LL.D) Log.d(TAG, "parse Collage failed : " + e.getMessage());
            return e.getMessage();
        }

        return null;
    }

    nexSaveDataFormat.nexCollageDrawInfoOf getSaveData(){
        nexSaveDataFormat.nexCollageDrawInfoOf data = new nexSaveDataFormat.nexCollageDrawInfoOf();
        data.scaledRect = scaledRect;
        return data;
    }

    void loadSaveData(nexSaveDataFormat.nexCollageDrawInfoOf data){
        scaledRect = data.scaledRect;
    }
}
