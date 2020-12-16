/******************************************************************************
 * File Name        : nexTransitionEffect.java
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
import android.util.LruCache;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.IconHelper;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDef;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDefReader;
import com.nexstreaming.app.common.util.StringUtil;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;


import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

/**
 * This class sets transition effects to be used by the NexEditor&trade;&nbsp;SDK.
 * With transition effects, the user can set various effects to switch from one clip to the next.
 * <p>Example code 1:</p>
 * 
 * <b>class sample code</b>
 * {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
    nexTransitionEffect[] transitionEffects = fxlib.getTransitionEffects();
    for (nexTransitionEffect e : transitionEffects) {
        mEffectItem.add(e.getId());
    }}
 * <p>Example code 2:</p>
 * 
 * <b>class sample code</b>
 * {@code project.getClip(0,true).getTransitionEffect().setTransitionEffect(effectID);}
 * @see nexClip#getTransitionEffect()
 * @see {@link com.nexstreaming.nexeditorsdk.nexEffectLibrary#findTransitionEffectById findTransitionEffectById}
 * @see nexEffectLibrary#getTransitionEffects()
 * @since version 1.0.0
 */
public final class nexTransitionEffect extends nexEffect implements Cloneable {
    private int mMinDuration;
    private int mMaxDuration;
    int mEffectOffset;
    int mEffectOverlap;

    private static final int TRANSITON_MIN_DURATION = 500;
    private static final int TRANSITION_MAX_DURATION = 30000;
    private static LruCache<String,Property> s_cachedProperty = new LruCache<>(100);
    private class Property{
        int offset;
        int overlap;

        public Property(int offset, int overlap) {
            this.offset = offset;
            this.overlap = overlap;
        }
    }


    protected static nexTransitionEffect clone(nexTransitionEffect src) {
        //Log.d("clone", "clone work./nexTransitionEffect");
        nexTransitionEffect object = null;
        try {
            object = (nexTransitionEffect)src.clone();
            object.mMinDuration = src.mMinDuration;
            object.mMaxDuration = src.mMaxDuration;
            object.mEffectOffset = src.mEffectOffset;
            object.mEffectOverlap = src.mEffectOverlap;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return  object;
    }

    nexTransitionEffect(){
        mType = com.nexstreaming.nexeditorsdk.nexEffect.kEFFECT_TRANSITION_AUTO;
        mDuration = nexProject.kAutoThemeTransitionDuration;
    }

    nexTransitionEffect(nexObserver project){
        mType = com.nexstreaming.nexeditorsdk.nexEffect.kEFFECT_TRANSITION_AUTO;
        mDuration = nexProject.kAutoThemeTransitionDuration;
        setObserver(project);
        //setClip(clip);
    }

    nexTransitionEffect(String ID){
        setTransitionEffect(ID);
        mDuration = nexProject.kAutoThemeTransitionDuration;
    }


    /**
     * This method sets a transition effect, that is not part of any existing theme, on a clip.  
     *
     * <p>Example code :</p>
     * 
     * {@code clip.getTransitionEffect().setTransitionEffect("com.nexstreaming.unkownid");}
     * @param ID The transition effect ID as a <tt>String</tt>. 
     * @throws com.nexstreaming.nexeditorsdk.exception.InvalidEffectIDException
     * @see #setAutoTheme()
     * @see #setEffectNone()
     * @since version 1.0.0
     * @see #getId()
     */
    public void setTransitionEffect(String ID ){
        if( setEffect(ID, kEFFECT_TRANSITION_USER) ){
            setModified(false);
            if( ID.compareTo("none") == 0 ){
                mEffectOffset = 0;
                mEffectOverlap = 0;
                return;
            }
            Property property = s_cachedProperty.get(ID);
            if( property == null ) {
                XMLItemDef itemDef = null;
                try {
                    itemDef = XMLItemDefReader.getItemDef(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), ID);
                } catch (XmlPullParserException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                if (itemDef == null) {
//                    Log.d("setTransitionEffect", "Transition is null");
                    mEffectOffset = 0;
                    mEffectOverlap = 0;
                } else {
                    mEffectOffset = itemDef.getTransitionOffset();
                    mEffectOverlap = itemDef.getTransitionOverlap();
//                    Log.d("setTransitionEffect", "Transition is not null :: " + mEffectOffset + " :: " + mEffectOverlap);
                }
                s_cachedProperty.put(ID,new Property(mEffectOffset,mEffectOverlap));
            }else{
                //Log.d("setTransitionEffect", "cache return:: "+property.offset+ "::"+ property.overlap);
                mEffectOffset = property.offset;
                mEffectOverlap = property.overlap;
            }
            mMinDuration = TRANSITON_MIN_DURATION;
            mMaxDuration = TRANSITION_MAX_DURATION;

        }
    }

    /**
     *  This method sets a transition effect on a clip matching the theme currently set.
     *
     * <p>Example code :</p>
     * 
     * {@code project.getClip(0,true).getTransitionEffect().setAutoTheme();}
     * @since version 1.0.0
     * @see #setTransitionEffect(String)
     * @see #setEffectNone()
     */
    public void setAutoTheme(){
        if( mType != nexEffect.kEFFECT_TRANSITION_AUTO ){
            mUpdated = true;
            setModified(false);
        }
        mType = nexEffect.kEFFECT_TRANSITION_AUTO;
    }

    /**
     * This method gets the minimum duration for a particular transition effect.
     *
     * <p>Example code :</p>
     * 
     * {@code int minDuration = project.getClip(0,true).getTransitionEffect().getMinDuration();}
     * @return The minimum duration of the transition effect in <tt>msec</tt> (milliseconds).
     * @since version 1.0.0
     * @see #getMaxDuration()
     */
    public int getMinDuration() {
        return mMinDuration;
    }

    /**
     * This method gets the maximum duration for a particular transition effect.
     *
     * <p>Example code :</p>
     * 
     * {@code int maxDuration = project.getClip(0,true).getTransitionEffect().getMaxDuration();}
     * @return The maximum duration of the transition effect in <tt>msec</tt> (milliseconds).
     * @since version 1.0.0
     * @see #getMinDuration()
     */
    public int getMaxDuration() {
        return mMaxDuration;
    }

    /**
     * This method gets the beginning position of a transition effect, relative to the end of the first clip in the transition.
     * 
     * The value returned is a percentage of the total duration of the transition effect.  This means that if an 
     * example transition effect has a total duration of 4 seconds, and begins 1 second before the end of the first clip
     * in a transition, then the value returned here would be 25 (percent).
     * 
     * The offset depends on the duration of time that neighboring clips overlap during the transition effect, so this 
     * value depends on the overlap percentage that can be retrieved by calling the method <tt>getOverlap()</tt>.
     *
     * <p>Example code :</p>
     * 
     * {@code visualClip.mEffectOffset = clipItem.getTransitionEffect().getOffset();}
     * @return The offset of the beginning of the transition effect from the end of the first clip in the transition, as a percentage of the 
     *         total duration of the transition effect.  
     * 
     * @see #getOverlap()
     * @since version 1.0.0
     */
    public int getOffset() {
        return mEffectOffset;
    }

    /**
     * This method gets the duration of time that clips overlap during a transition effect, as a percentage of the total transition time.
     * 
     * The overlap is indicated as a percentage of the total duration of the transition effect, so for example, if a transition effect takes
     * 4 seconds to complete, and the neighboring clips overlap for 2 seconds of that transition, the value returned here for the
     * example transition effect will be 50.
     *
     * <p>Example code :</p>
     * 
     * {@code visualClip.mEffectOverlap = clipItem.getTransitionEffect().getOverlap();}
     * @return The duration of time that clips overlap during the transition effect as a percentage of the total time of the transition effect, as an <tt>integer</tt>.
     * 
     * @see #getOffset()
     * @since version 1.0.0
     */
    public int getOverlap() {
        return mEffectOverlap;
    }

    /**
     *  This method gets the name of a transition effect. 
     *
     * <p>Example code :</p>
     * 
     * {@code name.setText(mTransitionEffects[index].getName(mContext)+":"+mTransitionEffects[index].getCategoryTitle(mContext));}
     * @param context The Android context. 
     * @return Returns one of:
     *           <ul>
     *           <li>The name of the transition effect if it is not part of any existing theme,
     *           <li>"None Transition" if there is no transition effect set between the clips, or
     *           <li>"Theme Transition" if the transition effect that is set between the clips is part of an existing theme.
     *           </ul>
     * @since version 1.0.0
     */
    public String getName(Context context) {
        if( mType == kEFFECT_NONE ){
            return "None Transition";
        }

        if( mType == nexEffect.kEFFECT_TRANSITION_AUTO ){
            return "Theme Transition";
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
     *  This method gets the description of a transition effect.
     *
     * <p>Example code :</p>
     * 
     * {@code desc.setText(mTransitionEffects[index].getDesc(mContext));}
     * @param context The Android context.
     * @return Returns one of:
     *          <ul>
     *           <li>Description of the effect if it is not included in any existing theme,
     *           <li>"None Transition" if there is no transition effect set between the clips, or
     *           <li>"Theme Transition" if the transition effect that is set between the clips is part of an existing theme.
     *           </ul>
     * @since version 1.0.0
     */
    public String getDesc(Context context){
        if( mType == kEFFECT_NONE ){
            return "None Transition Effect";
        }

        if( mType == nexEffect.kEFFECT_TRANSITION_AUTO ){
            return "Theme Transition Effect";
        }

        if( context == null) {
            return null;
        }

            ItemInfo info = AssetPackageManager.getInstance().getInstalledItemById(mID);
            if( info != null ){
                return info.getLabel().get("en");
            }

        return null;
    }

    /**
     * This method gets the icon of a transition effect.
     *
     * <p>Example code :</p>
     * 
     * {@code if( mTransitionEffects[index].getIcon() != null ){
            iv.setVisibility(View.VISIBLE);
            iv.setImageBitmap(mTransitionEffects[index].getIcon());
        }}
     * @return An instance of the transition effect icon as a bitmap.
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
     * This method gets the category name of a transition effect. 
     *
     * <p>Example code :</p>
     * 
     * {@code name.setText(mTransitionEffects[index].getName(mContext)+":"+mTransitionEffects[index].getCategoryTitle(mContext));}
     * @param context The Android context.
     * 
     * @return The category name as a <tt>String</tt>.
     * @since version 1.3.3
     */
    public String getCategoryTitle(Context context){
        if( mType == kEFFECT_NONE ){
            return "None";
        }

        if( mType == nexEffect.kEFFECT_TRANSITION_AUTO ){
            return "Theme";
        }

        if( context == null) {
            return null;
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
        data.mShowStartTime = 0;
        data.mShowEndTime = 0;
        //transition effect
        data.mMinDuration = mMinDuration;
        data.mMaxDuration = mMaxDuration;
        data.mEffectOffset = mEffectOffset;
        data.mEffectOverlap = mEffectOverlap;
        return data;
    }

    nexTransitionEffect(nexObserver project, nexSaveDataFormat.nexEffectOf data){
        mID = data.mID;
        mAutoID = data.mAutoID;
        mName = data.mName;
        mType = data.mType;
        mDuration = data.mDuration;
        itemMethodType = data.itemMethodType;

        //data.mUpdated;
        //data.mTitles= mTitles; todo

        mIsResolveOptions = data.mIsResolveOptions;

        mOptionsUpdate = data.mOptionsUpdate;
        m_effectOptions = data.m_effectOptions;
        //transition effect
        mMinDuration = data.mMinDuration;
        mMaxDuration = data.mMaxDuration;
        mEffectOffset = data.mEffectOffset;
        mEffectOverlap = data.mEffectOverlap;

        setObserver(project);
    }
}
