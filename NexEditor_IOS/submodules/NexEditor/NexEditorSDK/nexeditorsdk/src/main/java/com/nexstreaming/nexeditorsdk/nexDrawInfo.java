/******************************************************************************
 * File Name        : nexEffect.java
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

import android.graphics.Rect;
import android.util.Log;

import java.util.List;


public class nexDrawInfo {
    private static String TAG="nexDrawInfo";

    private int			mID;
    private int			mClipID;
    private int         mTopEffectID;
    private int         mSubEffectID;

    private String      mEffectID;
    private String      mTitle;
    private int         mIsTransition;

    private int			mStartTime;
    private int			mEndTime;

    private int			mRotateState;
    private int         mUserRotateState;
    private int         mUserTranslateX;
    private int         mUserTranslateY;
    private String      mUserLUT;
    private int			mBrightness;
    private int			mContrast;
    private int			mSaturation;
    private int			mTintcolor;
    private int			mLUT;
    private int         mCustomLUT_A;
    private int         mCustomLUT_B;
    private int         mCustomLUT_Power;

    private Rect        mStartRect = new Rect();
    private Rect        mEndRect = new Rect();
    private Rect        mFaceRect = new Rect();

    private float       mRealScale = 1.0f;

    private float       mRatio = 16.0f/9.0f;

    List<nexTemplateDrawInfo> subTemplateDrawInfos;

    public int getID() {
        return mID;
    }

    public void setID(int id) {
        this.mID = id;
    }

    public int getClipID() {
        return mClipID;
    }

    public void setClipID(int clipID) {
        this.mClipID = clipID;
    }

    public String getEffectID() {
        return mEffectID;
    }

    public int getSubEffectID() {
        return mSubEffectID;
    }

    public void setSubEffectID(int subEffectID) {
        this.mSubEffectID = subEffectID;
    }

    public int getTopEffectID() {
        return mTopEffectID;
    }

    public void setTopEffectID(int topEffectID) {
        this.mTopEffectID = topEffectID;
    }

    public void setEffectID(String effectID) {
        this.mEffectID = effectID;
    }

    public String getTitle() {
        return mTitle;
    }

    public void setTitle(String title) {
        this.mTitle = title;
    }

    public int getIsTransition() {
        return mIsTransition;
    }

    public void setIsTransition(int isTransition) {
        this.mIsTransition = isTransition;
    }

    public int getStartTime() {
        return mStartTime;
    }

    public void setStartTime(int startTime) {
        this.mStartTime = startTime;
    }

    public int getEndTime() {
        return mEndTime;
    }

    public void setEndTime(int endTime) {
        this.mEndTime = endTime;
    }

    public int getRotateState() {
        return mRotateState;
    }

    public void setRotateState(int rotate) {
        this.mRotateState = rotate;
    }

    public int getUserRotateState() {
        return mUserRotateState;
    }

    public void setUserRotateState(int rotate) {
        this.mUserRotateState = rotate;
    }

    public int getUserTranslateX(){

        return mUserTranslateX;
    }

    public int getUserTranslateY(){

        return mUserTranslateY;
    }

    public void setUserTranslate(int x, int y){

        mUserTranslateX = x;
        mUserTranslateY = y;
    }
    
    public float getRealScale(){

        return mRealScale;
    }

    public void setRealScale(float scale){

        mRealScale = scale;
    }

    public int getBrightness() {
        return mBrightness;
    }

    public void setBrightness(int brightness) {
        this.mBrightness = brightness;
    }

    public int getContrast() {
        return mContrast;
    }

    public void setContrast(int contrast) {
        this.mContrast = contrast;
    }

    public int getSaturation() {
        return mSaturation;
    }

    public void setSaturation(int saturation) {
        this.mSaturation = saturation;
    }

    public int getTintcolor() {
        return mTintcolor;
    }

    public void setTintcolor(int tintcolor) {
        this.mTintcolor = tintcolor;
    }

    public int getLUT() {
        return mLUT;
    }

    public int getCustomLUTA(){

        return mCustomLUT_A;
    }

    public int getCustomLUTB(){

        return mCustomLUT_B;
    }

    public int getCustomLUTPower(){

        return mCustomLUT_Power;
    }

    public void setLUT(int lut) {
        this.mLUT = lut;
    }

    public String getUserLUT() {
        return mUserLUT;
    }

    public void setUserLUT(String lut) {
        mUserLUT = lut;
    }

    public void setCustomLUTA(int lut) {
        this.mCustomLUT_A = lut;
    }

    public void setCustomLUTB(int lut) {
        this.mCustomLUT_B = lut;
    }

    public void setCustomLUTPower(int power) {
        this.mCustomLUT_Power = power;
    }

    public Rect getStartRect() {
        return mStartRect;
    }

    public void setStartRect(Rect startRect) {
        this.mStartRect.set(startRect.left, startRect.top, startRect.right, startRect.bottom);
    }

    public Rect getEndRect() {
        return mEndRect;
    }

    public void setEndRect(Rect endRect) {
        this.mEndRect.set(endRect.left, endRect.top, endRect.right, endRect.bottom);
    }

    public Rect getFaceRect() {
        return mFaceRect;
    }

    public void setFaceRect(Rect face) {
        this.mFaceRect.set(face.left, face.top, face.right, face.bottom);
    }

    public float getRatio() {
        return mRatio;
    }

    public void setRatio(float ratio) {
        this.mRatio = ratio;
    }

    public List<nexTemplateDrawInfo> getSubTemplateDrawInfos() {
        return subTemplateDrawInfos;
    }

    public void setSubTemplateDrawInfos(List<nexTemplateDrawInfo> subTemplateDrawInfos) {
        this.subTemplateDrawInfos = subTemplateDrawInfos;
    }

    public void print(){
        Log.d(TAG, String.format("nexDrawInfo id : %d", mID));
        Log.d(TAG, String.format("nexDrawInfo clip : %d", mClipID));
        Log.d(TAG, String.format("nexDrawInfo subID : %d", mSubEffectID));
        Log.d(TAG, String.format("nexDrawInfo start : %d", mStartTime));
        Log.d(TAG, String.format("nexDrawInfo end : %d", mEndTime));
        Log.d(TAG, String.format("nexDrawInfo effect : %s", mEffectID == null ? "" : mEffectID));
        Log.d(TAG, String.format("nexDrawInfo istransition : %d", mIsTransition));
        Log.d(TAG, String.format("nexDrawInfo color : %d %d %d", mBrightness, mContrast, mSaturation));
        Log.d(TAG, String.format("nexDrawInfo lut : %d", mLUT));
        Log.d(TAG, String.format("nexDrawInfo start rect : %d %d %d %d", mStartRect.left, mStartRect.top, mStartRect.right, mStartRect.bottom));
        Log.d(TAG, String.format("nexDrawInfo end rect : %d %d %d %d", mEndRect.left, mEndRect.top, mEndRect.right, mEndRect.bottom));
        Log.d(TAG, String.format("nexDrawInfo face rect : %d %d %d %d", mEndRect.left, mEndRect.top, mEndRect.right, mEndRect.bottom));
        Log.d(TAG, String.format("nexDrawInfo ratio : %f", mRatio));
        Log.d(TAG, "---------------------------------------------------");

    }
}
