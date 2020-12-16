/******************************************************************************
 * File Name        : nexClipEffect.java
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
import android.graphics.Bitmap;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.util.StringUtil;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;

import java.io.IOException;

/**
 * This class sets clip effects. 
 * A clip effect is an effect that can be set on a clip (to add dramatic visuals to the images or videos).
 * 
 * Different effects include different options that can be set and customized as desired.
 * 
 * <p>Example code 1:</p>
 *     {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
        nexClipEffect[] clipEffects = fxlib.getClipEffects();
        for (nexClipEffect e : clipEffects) {
            mEffectItem.add(e.getId());
        }
 *     }
 * <p>Example code 2:</p>
 *     <b>class sample code</b>
 *     {@code
 *      project.getClip(0,true).getClipEffect().setEffect(effectID);
 *     }
 * 
 * @see nexEffectLibrary#getClipEffects()
 * @see nexEffectOptions
 * @see com.nexstreaming.nexeditorsdk.nexEffectLibrary#findClipEffectById(String)
 * @see com.nexstreaming.nexeditorsdk.nexClip#getClipEffect()
 * @since version 1.0.0
 *
 */
public final class nexClipEffect extends nexEffect implements Cloneable {

    private int mShowStartTime = 0;
    private int mShowEndTime = 10000;

    protected static nexClipEffect clone(nexClipEffect src) {
        //Log.d("clone", "clone work./nexClipEffect");
        nexClipEffect object = null;
        try {
            object = (nexClipEffect)src.clone();
            object.mShowEndTime = src.mShowEndTime;
            object.mShowStartTime = src.mShowStartTime;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }

        return object;
    }

    nexClipEffect() {
        mType = kEFFECT_CLIP_AUTO;
    }

    nexClipEffect(String ID) {
        mID = ID;
        mType = kEFFECT_CLIP_USER;
    }

    /**
     * This method sets a clip effect, that is not part of any existing theme, on a clip. 
     * A clip effect that is not part of any theme can be brought from {@link nexEffectLibrary#getClipEffects()}.
     *
     * <p>Example code:</p>
     *     {@code
                mProject.getClip(i, true).getClipEffect().setEffect("com.nexstreaming.kmsdk.test.extendedpip");
            }
     * @param ID The clip effect ID as a <tt> String</tt>. 
     * @throws com.nexstreaming.nexeditorsdk.exception.InvalidEffectIDException Will occur if the ID is not a clip effect ID. 
     * 
     * @see #setAutoTheme()
     * @see #setEffectNone()
     * @since version 1.0.0
     * 
     */
    public void setEffect(String ID){
        super.setEffect(ID, kEFFECT_CLIP_USER);
    }

    /**
     * This methods adds text to clip effects. 
     * If this is set to <tt>null</tt>, no text will appear on the screen
     *
     * <p>Example code:</p>
     *     {@code
                project.getClip(0,true).getClipEffect().setTitle("Title test");
    }
     * @param str The text to be printed on the screen, as a <tt>String</tt>.
     * 
     * @see #setTitle(int, String)
     * @see #getTitle()
     * @see #getTitle(int)
     * @since version 1.0.0
     * 
     */
    public void setTitle(String str) {
        mUpdated = true;
        setTitle(0, str);
    }

    /**
     * This method gets text from a clip effect. 
     *
     * <p>Example code:</p>
     *     {@code
                String title = project.getClip(0,true).getClipEffect().getTitle();
            }
     * @return Text set to the clip effect.
     *
     * @see #setTitle(String)
     * @see #setTitle(int, String)
     * @see #getTitle(int)
     *
     * @since version 1.0.0
     */
    public String getTitle() {
        return getTitle(0);
    }

    /**
     * This method gets the name of an effect.
     *
     * <p>Example code:</p>
     *     {@code
                name.setText(mClipEffects[index].getName(mContext)+":"+mClipEffects[index].getCategoryTitle(mContext));
            }
     * @param context The Android context of the effect.
     * @return The name of the effect.
     *
     * @see #getDesc(android.content.Context)
     *
     * @since version 1.0.0
     */
    public String getName(Context context) {
        if( mType == kEFFECT_NONE ){
            return "None Clip Effect";
        }

        if( mType == kEFFECT_CLIP_AUTO ){
            return "Theme Clip Effect";
        }

        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(mID);
        if (info != null) {
            if (context != null) {
                mName = StringUtil.getLocalizedString(context, info.getLabel());
                if( mName == null) {
                    mName = info.getLabel().get("en");
                }
            }else {
                mName = info.getLabel().get("en");
            }
        }
        return mName;
    }

    /**
     * This method gets the description of an effect.
     *
     * <p>Example code:</p>
     *     {@code
                name.setText(mClipEffects[index].getName(mContext)+":"+mClipEffects[index].getDesc(mContext));
            }
     * @param context The Android context.
     * @return The description of an effect. If context is <tt>null</tt>, returns <tt>null</tt>.
     * 
     * @see #getName(android.content.Context)
     * @since version 1.0.0
     * 
     */
    public String getDesc(Context context){
        if( mType == kEFFECT_NONE ){
            return "None Clip Effect";
        }

        if( mType == kEFFECT_CLIP_AUTO ){
            return "Theme Clip Effect";
        }

        if( context == null) {
            return null;
        }

        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(mID);
        if (info != null) {
            return info.getLabel().get("en");
        }
        return null;
    }

    /**
     * This method gets the icon of an effect.
     *
     * <p>Example code:</p>
     *     {@code     if( mClipEffects[index].getIcon() != null ){
                    iv.setVisibility(View.VISIBLE);
                    iv.setImageBitmap(mClipEffects[index].getIcon());
                }
            }
     * @return An instance of the effect icon bitmap.
     * @since version 1.0.0
     */
    public Bitmap getIcon(){
            if( mID != null) {
                ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(mID);
                if( info != null ) {
                    try {
                        return IconHelper.getIconBitmap(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), info,0,0);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
            return null;
    }

    /** 
     * This method sets the start time of a clip effect.
     *
     * <p>Example code:</p>
     *     {@code
                project.getClip(0,true).getClipEffect().setShowStartTime(0);
            }
     * @param time The start time of the clip effect, in <tt>msec</tt> (milliseconds). 
     * @throws InvalidRangeException
     *
     * @see #getShowStartTime()
     *
     * @since version 1.1.0
     */
    public void setShowStartTime(int time){
        if( mShowEndTime < time ) {
            throw new InvalidRangeException(time, mShowEndTime);
        }
        mShowStartTime = time;
    }

    /** 
     * This method sets the end time of a clip effect.
     * 
     * <p>Example code:</p>
     *     {@code
                project.getClip(0,true).getClipEffect().setShowEndTime(kClipDuration);
            }
     * @param time The end time of the clip effect, in <tt>msec</tt> (milliseconds). 
     * @throws InvalidRangeException
     *
     * @see #getShowEndTime()
     * @since version 1.1.0
     */
    public void setShowEndTime(int time){
        if( mShowStartTime > time ) {
            throw new InvalidRangeException(mShowStartTime, time);
        }
        mShowEndTime = time;
    }

    /**
     * This method sets the start time and end time of a clip effect.  
     * The clip effect's duration will automatically adjusted to the clip's duration if the effect duration set with this method is longer than the clip's duration. 
     * 
     * <p>Example code:</p>
     *     {@code
                mProject.getClip(i, true).getClipEffect().setEffectShowTime(0,clip.getTotalTime()); //set full time effect
            }
     * @param starttime The start time of the clip effect, in <tt>msec</tt> (milliseconds). 
     * @param endtime The end time of the clip effect, in <tt>msec</tt> (milliseconds). 
     * @throws InvalidRangeException
     * @since version 1.3.43
     */
    public void setEffectShowTime(int starttime, int endtime){
        if( endtime < starttime ) {
            throw new InvalidRangeException(starttime, endtime);
        }
        mShowStartTime = starttime;
        mShowEndTime = endtime;
    }

    /** 
     * This method gets the start time of a clip effect.
     * 
     * <p>Example code:</p>
     *     {@code
                StringBuilder b = new StringBuilder();
                b.append(clip.getClipEffect().getShowStartTime());
                b.append(',');
                b.append(clip.getClipEffect().getShowEndTime());
                b.append('?');
            }
     * @return  The start time of a clip effect, in <tt>msec</tt> (milliseconds).
     *
     * @see #setShowStartTime(int)
     * @since version 1.1.0
     * @see #setShowStartTime(int)
     */
    public int getShowStartTime(){
        return mShowStartTime;
    }

    /** 
     * This method gets the end time of a clip effect. 
     *
     * <p>Example code:</p>
     *     {@code     StringBuilder b = new StringBuilder();
                b.append(clip.getClipEffect().getShowStartTime());
                b.append(',');
                b.append(clip.getClipEffect().getShowEndTime());
                b.append('?');
            }
     * @return The end time of a clip effect, in <tt>msec</tt> (milliseconds).
     *
     * @see #setShowEndTime(int)
     * @since version 1.1.0
     */
    public int getShowEndTime(){
        return mShowEndTime;
    }

    /**
     * This method sets clip effects on a clip automatically, depending on the theme currently set. 
     * 
     * <p>Example code:</p>
     *     {@code
                project.getClip(0,true).getClipEffect().setAutoTheme();
            }
     * @see nexEffect#setEffectNone()
     * @see #setEffect(String)
     * @since version 1.0.0
     */
    public void setAutoTheme(){
        mType = nexEffect.kEFFECT_CLIP_AUTO;
    }

    /**
     * This method gets the category name of a clip effect. 
     * 
     * <p>Example code:</p>
     *     {@code
                name.setText(mClipEffects[index].getName(mContext)+":"+mClipEffects[index].getCategoryTitle(mContext));
            }
     * @param context The Android context.
     * @return The category name as a <tt>String</tt>. 
     * @since version 1.3.3
     */
    public String getCategoryTitle(Context context){
        if( mType == kEFFECT_NONE ){
            return "None";
        }

        if( mType == kEFFECT_CLIP_AUTO ){
            return "Theme";
        }

        ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(mID);
        if( info != null ){
            return info.getCategory().toString();
        }
        return null;
    }

    nexSaveDataFormat.nexEffectOf getSaveData(){
        nexSaveDataFormat.nexEffectOf data = new nexSaveDataFormat.nexEffectOf();

        data.mID = mID;
        data.mAutoID = mAutoID;
        data.mName = mName;
        data.mType = mType;
        data.mDuration = mDuration;
        data.itemMethodType = itemMethodType;

        //private nexObserver mObserver; --> internal set
        //data.mUpdated;
        //data.mTitles= mTitles; todo

        data.mIsResolveOptions = mIsResolveOptions;

        data.mOptionsUpdate = mOptionsUpdate;
        data.m_effectOptions = m_effectOptions;
        //clip effect
        data.mShowStartTime = mShowStartTime;
        data.mShowEndTime = mShowEndTime;
        //transition effect
        data.mMinDuration = 0;
        data.mMaxDuration = 0;
        data.mEffectOffset = 0;
        data.mEffectOverlap = 0;
        return data;
    }

    nexClipEffect(nexSaveDataFormat.nexEffectOf data){
        mID = data.mID;
        mAutoID = data.mAutoID;
        mName = data.mName;
        mType = data.mType;
        mDuration = data.mDuration;
        itemMethodType = data.itemMethodType;

        //private nexObserver mObserver; --> internal set
        //data.mUpdated;
        //data.mTitles= mTitles; todo

        mIsResolveOptions = data.mIsResolveOptions;

        mOptionsUpdate = data.mOptionsUpdate;
        m_effectOptions = data.m_effectOptions;
        //clip effect
        mShowStartTime = data.mShowStartTime;
        mShowEndTime = data.mShowEndTime;
    }
}
