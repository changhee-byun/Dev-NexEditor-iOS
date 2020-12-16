/******************************************************************************
 * File Name        : nexOverlayItem.java
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
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.overlay.AwakeAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayMotion;
import com.nexstreaming.app.common.thememath.NexTheme_Math;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.nexvideoeditor.NexCache;
import com.nexstreaming.nexeditorsdk.exception.ClipIsNotVideoException;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;
import com.nexstreaming.nexeditorsdk.exception.NoSuchObjectException;
import com.nexstreaming.nexeditorsdk.exception.NotSupportedAPILevel;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/** 
 * This class is used to define and register an overlay image to a project. 
 *
 * This class allows the start time and duration of an overlay image to be set as well as setting possible animation effects. 
 * Also, simple color effects and adjustments can be made with this class. 
 * 
 * Effects and animations on clips are set in different layers which can be adjusted independently.
 * 
 * The coordinates of an overlay image with ratio of 16:9: upper left (0,0) and bottom right (1280,720).
 * The coordinates of an overlay image with ratio of 1:1: upper left (0,0) and bottom right (720,720).
 * <p>Example code :</p>
 * {@code nexOverlayImage overlay = new nexOverlayImage( "testOverlay",R.drawable.myavatar_17004763_1);
 nexOverlayImage.registerOverlayImage(overlay);
 nexOverlayItem overlayItem = new nexOverlayItem("testOverlay",200,200,0,0+6000);
 nexAnimate ani = nexAnimate.getAnimateImages(
    0,5000,R.drawable.myavatar_17004763_1
    ,R.drawable.myavatar_17004763_2
    ,R.drawable.myavatar_17004763_3
    ,R.drawable.myavatar_17004763_4
    ,R.drawable.myavatar_17004763_5
    ,R.drawable.myavatar_17004763_6
    ,R.drawable.myavatar_17004763_7
    ,R.drawable.myavatar_17004763_8
    ,R.drawable.myavatar_17004763_9
    ,R.drawable.myavatar_17004763_10
    );
    overlayItem.addAnimate(ani);
    project.addOverlay(overlayItem);}
 * @since version 1.1.0
 */
public final class nexOverlayItem implements Cloneable {
    private static final String TAG = "nexOverlayItem";

    private boolean mUpdated = true;
    private boolean showItem = true;

    int mStartTime;
    int mEndTime;

    private static int sLastId = 1;
    private int mId = 0;
    private nexOverlayImage mOverLayImage;
    private nexOverlayFilter mOverLayFilter;
    List<nexAnimate> mAniList;
    int mTime;
    private float mAnimateTranslateXpos = 0;
    private float mAnimateTranslateYpos = 0;
    private float mAnimateTranslateZpos = 0;
    private float mAnimateLastAlpha;
    private float mAnimateLastRotateDegreeX;
    private float mAnimateLastRotateDegreeY;
    private float mAnimateLastRotateDegreeZ;
    private float mAnimateLastScaledX;
    private float mAnimateLastScaledY;
    private float mAnimateLastScaledZ;
    private ChromaKey mChromaKey;
    private int anchorPoint = AnchorPoint_LeftTop;

    private int lastAnchorPoint;
    private int lastLayerWidth;
    private int lastLayerHeight;

    int anchorPointX;
    int anchorPointY;
    private boolean relationCoordinates;

    private int animateResourceId;
    private Set<nexAnimate> mActiveAnimateList = new HashSet<>();
    /**
     * This static variable defines the default AnchorPoint location at top left 
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_LeftTop = 0;

    /**
     * This static variable defines the default AnchorPoint location at top middle
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_MiddleTop = 1;

    /**
     * This static variable defines the default AnchorPoint location at top right
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_RightTop = 2;

    /**
     * This static variable defines the default AnchorPoint location at middle left
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_LeftMiddle = 3;

    /**
     * This static variable defines the default AnchorPoint location at middle middle
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_MiddleMiddle = 4;

    /**
     * This static variable defines the default AnchorPoint location at middle right
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_RightMiddle = 5;

    /**
     * This static variable defines the default AnchorPoint location at middle left
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_LeftBottom = 6;

    /**
     * This static variable defines the default AnchorPoint location at bottom middle
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_MiddleBottom = 7;

    /**
     * This static variable defines the default AnchorPoint location at bottom right
     *
     * @since 1.7.8
     * @see #nexOverlayItem(nexOverlayFilter, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(nexOverlayImage, int, boolean, float, float, int, int)
     * @see #nexOverlayItem(String, int, boolean, float, float, int, int)
     * @see #setAnchor(int)
     * @see #getAnchor()
     */
    public final static int AnchorPoint_RightBottom = 8;

    float mX;
    float mY;
    float mZ;

    float mAlpha = 1;
    float mScaledX = 1;
    float mScaledY = 1;
    float mScaledZ = 1;
    float mRotateDegreeX = 0;
    float mRotateDegreeY = 0;
    float mRotateDegreeZ = 0;

    private int mBrightness = 0;
    private int mContrast = 0;
    private int mSaturation = 0;
    private int mHue = 0;

    private nexColorEffect mColorEffect = nexColorEffect.NONE;

    private int mLayerExpressionDuration = 1000;
    int mVideoEngineId;
    private Rect mMaskRect = new Rect();
    //use only video
    private int mTrimStartDuration = 0;
    private int mTrimEndDuration = 0;
    private boolean mAudioOnOff = true;
    private int mVolume = 100;
    private int mSpeedControl = 100;
    private float[] scratchPoint = {0,0};
    private Matrix scratchMatrix = new Matrix();
    private OverlayMotion cacheMotion;
    private nexOverlayKineMasterExpression mLayerExpression = nexOverlayKineMasterExpression.NONE;

    private boolean mOverlayTitle = false;

    private boolean updateCoordinates(boolean checkOnly){
        boolean update = false;
        if( lastAnchorPoint != anchorPoint ){
            if( !checkOnly ){
                lastAnchorPoint = anchorPoint;
            }
            update = true;
        }

        if( lastLayerWidth != nexApplicationConfig.getAspectProfile().getWidth() ){
            if( !checkOnly ){
                lastLayerWidth = nexApplicationConfig.getAspectProfile().getWidth();
            }
            update = true;
        }

        if( lastLayerHeight !=  nexApplicationConfig.getAspectProfile().getHeight()){
            if( !checkOnly ){
                lastLayerHeight = nexApplicationConfig.getAspectProfile().getHeight();
            }
            update = true;
        }
        return update;
    }

    private void getAnchorPosition(boolean renderOverlay){
        boolean update = renderOverlay;
        if( !update ){
            update = updateCoordinates(false);
        }

        if( update ) {

            switch (anchorPoint) {
                case 0:
                    anchorPointX = 0;
                    anchorPointY = 0;
                    break;

                case 1:
                    anchorPointX = lastLayerWidth / 2;
                    anchorPointY = 0;
                    break;

                case 2:
                    anchorPointX = lastLayerWidth;
                    anchorPointY = 0;
                    break;

                case 3:
                    anchorPointX = 0;
                    anchorPointY = lastLayerHeight / 2;
                    break;

                case 4:
                    anchorPointX = lastLayerWidth / 2;
                    anchorPointY = lastLayerHeight / 2;
                    break;

                case 5:
                    anchorPointX = lastLayerWidth;
                    anchorPointY = lastLayerHeight / 2;
                    break;

                case 6:
                    anchorPointX = 0;
                    anchorPointY = lastLayerHeight;
                    break;

                case 7:
                    anchorPointX = lastLayerWidth / 2;
                    anchorPointY = lastLayerHeight;
                    break;

                case 8:
                    anchorPointX = lastLayerWidth;
                    anchorPointY = lastLayerHeight;
                    break;

                default:
                    break;
            }
        }
    }

    private int[] getRealPositions(boolean renderOverlay){
        int [] pos = new int[3];
        getAnchorPosition(renderOverlay);

        if( relationCoordinates ){
            pos[0] = anchorPointX + (int)((float)lastLayerWidth*mX);
            pos[1] = anchorPointY + (int)((float)lastLayerHeight*mY);
            pos[2] = 0;
        }else{
            pos[0] = anchorPointX +  (int)mX;
            pos[1] = anchorPointY + (int)mY;
            pos[2] = (int)mZ;
        }

        return pos;
    }


    protected static nexOverlayItem clone(nexOverlayItem src) {
        //Log.d("clone", "clone work./nexOverlayItem");
        nexOverlayItem object = null;
        try {
            object = (nexOverlayItem)src.clone();
            object.mColorEffect = nexColorEffect.clone(src.mColorEffect);
            if( src.mOverLayImage != null ) {
                object.mOverLayImage = nexOverlayImage.clone(src.mOverLayImage);
            }
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    /**
     * @deprecated
     * This method sets an effect for a preset animation. 
     * If the parameter <tt>layerExpression</tt> is set to 0, an animation effect made by the developer should be used instead of a preset animation effect. 
     *
     * <p>Example code :</p>
     *      {@code
                mEngin.getProject().getOverlay(mOverlayItemId).setLayerExpression(nexOverlayKineMasterExpression.NONE);
            }
     * @param layerExpressionIndex The index of the label from the array that is to be retrieved from <tt>getLayerExpressions()</tt>, as <tt>integer</tt>.
     * 
     * @throws com.nexstreaming.nexeditorsdk.exception.InvalidRangeException
     * @since version 1.1.0
     * @see #getLayerExpression()
     */
    @Deprecated
    public void setLayerExpression(nexOverlayKineMasterExpression layerExpressionIndex) {
        mLayerExpression = layerExpressionIndex;
    }

    /**
     * @deprecated
     * This method gets the index of a preset animation effect.
     * Use <tt>getLayerExpressions()</tt> to get the label names. 
     *
     * <p>Example code :</p>
     * {@code ArrayAdapter<String> adapter = new ArrayAdapter<String>(getApplicationContext(),
        R.layout.simple_spinner_dropdown_item_1, nexOverlayItem.getLayerExpressions());}
     * @return The index of the preset animation effect, as an <tt>integer</tt>. 
     *
     * @since version 1.1.0
     * @see #setLayerExpression(nexOverlayKineMasterExpression)
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public nexOverlayKineMasterExpression getLayerExpression() {
        return mLayerExpression;
    }

    /**
     * @deprecated
     * This method sets the duration of a preset animation effect for a clip. 
     *  
     * <p>Example code :</p>
     * {@code overlayItem.setLayerExpressionDuration(2000);}
     * @param duration  The duration of the preset animation in <tt>msec</tt> (milliseconds), as an <tt>integer</tt>. 
     * 
     * @see #getLayerExpressionDuration()
     * @since version 1.1.0
     *
     */
    @Deprecated
    public void setLayerExpressionDuration(int duration) {
        mLayerExpressionDuration = duration;
    }

    /**
     * @deprecated
     * This method gets the duration of a preset animation effect for a clip. 
     *
     * <p>Example code :</p>
     * {@code int overlay_duration = overlayItem.getLayerExpressionDuration();}
     * @return The duration of the preset animation in <tt>msec</tt> (milliseconds), as an <tt>integer</tt>.
     * 
     * @see #setLayerExpressionDuration(int)
     * @since version 1.1.0
     *
     */
    @Deprecated
    public int getLayerExpressionDuration() {
        return mLayerExpressionDuration;
    }

   /** 
     * This method sets a color effect on an overlay image. 
     *
    * <p>Example code :</p>
    * {@code mEngine.getProject().getClip(0,true).setColorEffect( nexColorEffect.getPresetList().get(position) );}
     * @param effect The color effect, <tt>nexColorEffect</tt>, to set on an overlay image.
     * @since version 1.1.0
     */
    public void setColorEffect(nexColorEffect effect) {
        mColorEffect = effect;
    }

    private int getCombinedBrightness() {
        if (mColorEffect == null)
            return mBrightness;
        return mBrightness + (int) (255 * mColorEffect.getBrightness());
    }

    private int getCombinedContrast() {
        if (mColorEffect == null)
            return mContrast;
        return mContrast + (int) (255 * mColorEffect.getContrast());
    }

    private int getCombinedSaturation() {
        if (mColorEffect == null)
            return mSaturation;
        return mSaturation + (int) (255 * mColorEffect.getSaturation());
    }

    private int getCombinedHue() {
        if (mColorEffect == null)
            return mHue;
        return mHue + (int) (180 * mColorEffect.getHue());
    }

    private int getTintColor() {
        if (mColorEffect == null)
            return 0;
        return mColorEffect.getTintColor();
    }

    /** 
     * This method gets the ID of an <tt>nexOverlayItem</tt> instance.
     *  
     * The unique ID of an overlay item is made when a project is created.  
     *
     * <p>Example code :</p>
     * {@code int overlay_id = overlayItem.getId();}
     * @return The overlay item ID provided when a project is created, as an <tt>integer</tt>. 
     *
     * @since version 1.1.0
     */
    public int getId() {
        return mId;
    }

    /**
     * @deprecated
     * This method gets an overlay image as an <tt>nexOverlayItem</tt> object.
     * 
     * <p>Example code :</p>
     * {@code visualClip.mTotalTime = item.getOverlayImage().getVideoClipInfo().getTotalTime();}
     * @return The overlay image, <tt>nexOverlayImage</tt>, set for an overlay item.
     * @since version 1.1.0
     * @deprecated For internal use only. Please do not use.
     *
     */
    @Deprecated
    nexOverlayImage getOverlayImage() {
        return mOverLayImage;
    }

    /**
     * This method creates an overlay item with the ID used to register an image to <tt>nexOverlayImage</tt>.
     *
     * The coordinates of an overlay image with ratio of 16:9 : upper left (0,0) and bottom right (1280,720).
     * The coordinates of an overlay image with ratio of 1:1 : upper left (0,0) and bottom right (720,720).
     *  
     * @param overlayImageId  The ID used to register an image to <tt>nexOverlayImage</tt>.
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center. 
     * @param startTime The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds). 
     * @param endTime The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds). 
     * <p>Example code :</p>
     * {@code      nexOverlayImage overlay2 = new nexOverlayImage( "testOverlay2",R.drawable.ilove0001);
    nexOverlayImage.registerOverlayImage(overlay2);
        nexOverlayItem overlayItem2 = new nexOverlayItem("testOverlay2",800,200,0,0+6000);}
     * @since version 1.1.0
     */
    public nexOverlayItem(String overlayImageId, int posX, int posY, int startTime, int endTime) {
        //mOverLayImage = nexOverlayImage.getOverlayImage(overlayImageId);
        nexOverlayPreset ovlib = nexOverlayPreset.getOverlayPreset();
        if( ovlib == null ){
            throw new nexSDKException("nexOverlayPreset is null");
        }
        mOverLayImage = ovlib.getOverlayImage(overlayImageId);

        if (mOverLayImage == null) {
            throw new nexSDKException("Not found OverlayImage. id=" + overlayImageId);
        }

        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

        mId = sLastId;
        sLastId++;

        mX = posX;
        mY = posY;
        mStartTime = startTime;
        mEndTime = endTime;
        resetAnimate();
    }

    /**
     * This method creates an overlay item with the ID used to register an image to <tt>nexOverlayImage</tt>.
     *
     * A coordiante is set with a combination of anchorPoint and relationCoordinates.
     *
     * @param overlayImageId The ID used to register an image to <tt>nexOverlayImage</tt>.
     * @param anchorPoint This sets the zero coordinate of an Overlay.
     * @param relationCoordinates true - set coordinates based on (0~1) calue. , false - set coordinates based on pixel resolution set in Aspect profile.
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center.
     * @param startTime The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @since 1.7.8
     */
    public nexOverlayItem(String overlayImageId,int anchorPoint, boolean relationCoordinates, float posX, float posY, int startTime, int endTime) {
        //mOverLayImage = nexOverlayImage.getOverlayImage(overlayImageId);
        nexOverlayPreset ovlib = nexOverlayPreset.getOverlayPreset();
        if( ovlib == null ){
            throw new nexSDKException("nexOverlayPreset is null");
        }
        mOverLayImage = ovlib.getOverlayImage(overlayImageId);

        if (mOverLayImage == null) {
            throw new nexSDKException("Not found OverlayImage. id=" + overlayImageId);
        }

        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

        mId = sLastId;
        sLastId++;
        this.anchorPoint = anchorPoint;
        this.relationCoordinates = relationCoordinates;
        mX = posX;
        mY = posY;
        mStartTime = startTime;
        mEndTime = endTime;
        resetAnimate();
    }


    /**
     * This method creates an overlay item, that uses an overlay image made by the developer, which can be later added to the project.  
     *  
     * The coordinates of an overlay image with ratio of 16:9 : upper left (0,0) and bottom right (1280,720).
     * The coordinates of an overlay image with ratio of 1:1 : upper left (0,0) and bottom right (720,720).
     * 
     * @param olImage  An <tt>nexOverlayImage</tt> instance of the overlay image.
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center. 
     * @param startTime  The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds). 
     * @param endTime  The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds). 
     * {@code      nexOverlayImage textOverlay = new nexOverlayImage( "text",new TextOverlay(this,"Nexstreaming"));
        nexOverlayItem overlayItem3 = new nexOverlayItem(textOverlay,200,400,0,6000);}
     * @since version 1.1.0
     * <table border=1><tr><td bgcolor=#ff><font color=#ffffff>nexEditorSDK API Limited Level</font></td><td>{@value EditorGlobal#kOverlayImageLimited}</td></tr></table>
     */
    public nexOverlayItem(nexOverlayImage olImage, int posX, int posY, int startTime, int endTime) {
        if( EditorGlobal.ApiLevel < EditorGlobal.kOverlayImageLimited ) {
            mId = sLastId;
            sLastId++;

            if (endTime <= startTime) {
                throw new InvalidRangeException(startTime, endTime);
            }

            mOverLayImage = olImage;

            mX = posX;
            mY = posY;
            mStartTime = startTime;
            mEndTime = endTime;
            resetAnimate();
        }else{
            throw new NotSupportedAPILevel();
        }
    }

    /**
     * This method creates an overlay item, that uses an overlay image made by the developer, which can be later added to the project.
     *
     * A coordiante is set with a combination of anchorPoint and relationCoordinates.
     *
     * @param olImage An <tt>nexOverlayImage</tt> instance of the overlay image.
     * @param anchorPoint This sets the zero coordinate of an Overlay.
     * @param relationCoordinates true - set coordinates based on (0~1) calue. , false - set coordinates based on pixel resolution set in Aspect profile.
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center.
     * @param startTime The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @since 1.7.8
     */
    public nexOverlayItem(nexOverlayImage olImage,int anchorPoint, boolean relationCoordinates , float posX, float posY, int startTime, int endTime) {
        if( EditorGlobal.ApiLevel < EditorGlobal.kOverlayImageLimited ) {
            mId = sLastId;
            sLastId++;

            if (endTime <= startTime) {
                throw new InvalidRangeException(startTime, endTime);
            }

            mOverLayImage = olImage;
            this.anchorPoint = anchorPoint;
            this.relationCoordinates = relationCoordinates;
            mX = posX;
            mY = posY;
            mStartTime = startTime;
            mEndTime = endTime;
            resetAnimate();
        }else{
            throw new NotSupportedAPILevel();
        }
    }

    /**
     *
     * @param kmText
     * @param posX
     * @param posY
     * @param startTime
     * @param endTime
     * @since 1.7.8
     */
    @Deprecated
      public nexOverlayItem(nexOverlayKineMasterText kmText, int posX, int posY, int startTime, int endTime)
      {
          if( kmText.serialKMText ) {
              mId = sLastId;
              sLastId++;

              if (endTime <= startTime) {
                  throw new InvalidRangeException(startTime, endTime);
              }

              mOverLayImage = kmText;

              mX = posX;
              mY = posY;
              mStartTime = startTime;
              mEndTime = endTime;
              resetAnimate();
          }else{
              throw new NotSupportedAPILevel();
          }
      }

    /**
     *
     * @param kmText
     * @param anchorPoint
     * @param relationCoordinates
     * @param posX
     * @param posY
     * @param startTime
     * @param endTime
     * @since 1.7.8
     */
    @Deprecated
    public nexOverlayItem(nexOverlayKineMasterText kmText,int anchorPoint, boolean relationCoordinates, float posX, float posY, int startTime, int endTime)
    {
        if( kmText.serialKMText ) {
            mId = sLastId;
            sLastId++;

            if (endTime <= startTime) {
                throw new InvalidRangeException(startTime, endTime);
            }

            mOverLayImage = kmText;
            this.anchorPoint = anchorPoint;
            this.relationCoordinates = relationCoordinates;
            mX = posX;
            mY = posY;
            mStartTime = startTime;
            mEndTime = endTime;
            resetAnimate();
        }else{
            throw new NotSupportedAPILevel();
        }
    }

    /**
     * This method creates an OverlayItem for nexOverlayFilter
     *
     *
     * @param filter {@link nexOverlayFilter}
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center.
     * @param startTime The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @since 1.7.8
     */
    public nexOverlayItem(nexOverlayFilter filter, int posX, int posY, int startTime, int endTime) {
        //mOverLayImage = nexOverlayImage.getOverlayImage(overlayImageId);
        mOverLayFilter = filter;

        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

        mId = sLastId;
        sLastId++;

        mX = posX;
        mY = posY;
        mStartTime = startTime;
        mEndTime = endTime;
        resetAnimate();
    }

    /**
     *
     * @param filter {@link nexOverlayFilter}
     * @param anchorPoint This sets the zero coordinate of an Overlay.
     * @param relationCoordinates true - set coordinates based on (0~1) calue. , false - set coordinates based on pixel resolution set in Aspect profile.
     * @param posX The x-coordinate of the overlay image's center.
     * @param posY The y-coordinate of the overlay image's center.
     * @param startTime The start time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the overlay item (based on the total playtime of the project) in <tt>msec</tt> (milliseconds).
     * @since 1.7.8
     */
    public nexOverlayItem(nexOverlayFilter filter,int anchorPoint, boolean relationCoordinates, float posX, float posY, int startTime, int endTime) {
        //mOverLayImage = nexOverlayImage.getOverlayImage(overlayImageId);
        mOverLayFilter = filter;

        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

        mId = sLastId;
        sLastId++;
        this.anchorPoint = anchorPoint;
        this.relationCoordinates = relationCoordinates;
        mX = posX;
        mY = posY;
        mStartTime = startTime;
        mEndTime = endTime;
        resetAnimate();
    }

    /**
     * This method gets the audio volume state of a clip, whether it's <tt>on</tt> or <tt>off</tt>. 
     * Default value is <tt>on</tt>. 
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            visualClip.mAudioOnOff = item.getAudioOnOff()?1:0;
            ...
        }}
     * @return The audio volume state. <tt>TRUE</tt> if the audio state is <tt>on</tt>; otherwise <tt>FALSE</tt>. 
     * @see #setAudioOnOff(boolean)
     * @since version 1.5.0
     */
    public boolean getAudioOnOff(){
        return mAudioOnOff;
    }

 
    /**
     * This method sets the audio volume state of a clip, whether it's <tt>on</tt> or <tt>off</tt>.  
     * Default value is <tt>on</tt>.
     *
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            item.setAudioOnOff(false);
            ...
        }}
     * @param audioOnOff The audio volume state. <tt>TRUE</tt> if the audio state is <tt>on</tt>, otherwise <tt>FALSE</tt>. 
     * @see #getAudioOnOff()
     * @since version 1.5.0
     */
    public void setAudioOnOff( boolean audioOnOff){
        if( !isVideo() ){
            throw new ClipIsNotVideoException();
        }
        if( mAudioOnOff != audioOnOff)
            mUpdated = true;
        mAudioOnOff = audioOnOff;
    }

    /**
     * This method gets the audio volume level ( in range of 0 ~ 200 ) of a clip as an <tt>integer</tt>.
     * Default value is 100. 
     *
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            visualClip.mClipVolume = item.getVolume();
            ...
        }}
     * @return The volume level of the clip. 
     * @see #setVolume(int)
     * @since version 1.5.0
     */
    public int getVolume() {
        return mVolume;
    }

    /**
     * This method sets the audio volume level ( in range of 0 ~ 200 ) of a clip.
     * Default value is 100.
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            item.setVolume(false);
            ...
        }}
     * @param volume Clip volume level
     * @see #getVolume()
     * @since version 1.5.0
     */
    public void setVolume(int volume) {
        if( !isVideo() ){
            throw new ClipIsNotVideoException();
        }
        if(  mVolume != volume ) {
            mUpdated = true;
        }
        mVolume = volume;
    }

    /**
     * This method sets the playback speed ( in range of 13 ~ 400 ) of a clip. 
     * Default value is 100.
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            item.setSpeedControl(200);
            ...
        }}
     * @param speed The playback speed of the clip. 
     * @see #getSpeedControl()
     * @since version 1.5.0
     */
    public void setSpeedControl(int speed) {
        if( !isVideo() ){
            throw new ClipIsNotVideoException();
        }
        speed = speedControlTab(speed);
        if(mSpeedControl != speed){
            mUpdated = true;
            mSpeedControl = speed;
        }
    }

    /**
     * This method gets the playback speed ( in range of 13 ~ 400 ) of a clip in an <tt>integer</tt>
     * Default is value 100.
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem item : mProjectOverlays ) {
            visualClip.mClipSpeed = item.getSpeedControl();
            ...
        }}
     * @return The playback speed of the clip as an <tt>integer</tt>. 
     * @see #setSpeedControl(int)
     * @since version 1.5.0
     */
    public int getSpeedControl() {
        return mSpeedControl;
    }
    private int speedControlTab(int value){
        int tab[] = {13,25,50,75,100,125,150,175,200,400};
        int reval = 400;
        for( int i = 0 ; i < tab.length ; i++ ){
            if( tab[i] >= value ){
                reval = tab[i];
                break;
            }
        }
        return reval;
    }

    void resetAnimate() {
        mAnimateTranslateXpos = 0;
        mAnimateTranslateYpos = 0;
        mAnimateTranslateZpos = 0;
        mAnimateLastAlpha = mAlpha;
        mAnimateLastRotateDegreeX = mRotateDegreeX;
        mAnimateLastRotateDegreeY = mRotateDegreeY;
        mAnimateLastRotateDegreeZ = mRotateDegreeZ;
        mAnimateLastScaledX = mScaledX;
        mAnimateLastScaledY = mScaledY;
        mAnimateLastScaledZ = mScaledZ;
        mActiveAnimateList.clear();
    }

    /** 
     * This method gets the x-coordinate of an overlay image's location.
     *
     * <p>Example code :</p>
     * {@code mEngine.getProject().getOverlay(mOverlayId).setPosition(mEngine.getProject().getOverlay(mOverlayId).getPositionX(), lastProgress);}
     * @return The value of x-coordinate of the center of the overlay image. 
     * 
     * @see #setPosition(int, int)
     * @since version 1.1.0
     * 
     */
    public int getPositionX() {
        int[] points =getRealPositions(false);
        return points[0];
    }

    /**
     *  This method gets the y-coordinate of an overlay image's location. 
     * 
     * <p>Example code :</p>
     * {@code mEngine.getProject().getOverlay(mOverlayId).setPosition(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getPositionY());}
     * @return The value of y-coordinate of the center of the overlay image. 
     * 
     * @see #setPosition(int, int)
     * @since version 1.1.0
     * 
     */
    public int getPositionY() {
        int[] points =getRealPositions(false);
        return points[1];

    }

    /**
     * This method sets the location of an overlay image.
     * 
     * For overlay images with aspect ratio of 16:9, set the coordinate values based on 1280 X 720 resolution.
     * For overlay images with aspect ratio of 1:1, set the coordinate values based on 720 X 720 resolution.
     * 
     * <p>Example code :</p>
     * {@code mEngine.getProject().getOverlay(mOverlayId).setPosition(lastProgress, mEngine.getProject().getOverlay(mOverlayId).getPositionY());}
     * @param x The x-coordinate of the overlay image's center.
     * @param y The y-coordinate of the overlay image's center.
     * 
     * @see #getPositionX()
     * @see #getPositionY()
     * @since version 1.1.0
     */
    public void setPosition(int x, int y) {
        //anchorPoint = 0;
        //relationCoordinates = false;
        updateCoordinates(false);
        if(  relationCoordinates ){
            mX = (float)(x - anchorPointX)/(float)lastLayerWidth;
            mY = (float)(y - anchorPointY)/(float)lastLayerHeight;
        }else {
            mX = x - anchorPointX;
            mY = y - anchorPointY;
        }
        mUpdated = true;
        resetAnimate();
    }

    /**
     * This method moves the position of an item.
     * In setPosition(), parameters are pixel coordinate value of an Overlay, but in movePosition(), relationCoordinates value (0~1) may be used in parameters.
     *
     * @param x The x-coordinate of the overlay image's center.
     * @param y The y-coordinate of the overlay image's center.
     * @since 1.7.8
     * @see #setPosition(int, int)
     */
    public void movePosition(float x, float y ){
        mX = x;
        mY = y;
        mUpdated = true;
        resetAnimate();
    }

    /**
     * This method sets the Anchor to zero coordinate.
     * @param point AnchorPoint
     * <ul>
     *     <li>{@link #AnchorPoint_LeftTop} = {@value #AnchorPoint_LeftTop}</li>
     *     <li>{@link #AnchorPoint_MiddleTop} = {@value #AnchorPoint_MiddleTop}</li>
     *     <li>{@link #AnchorPoint_RightTop} = {@value #AnchorPoint_RightTop}</li>*
     *     <li>{@link #AnchorPoint_LeftMiddle} = {@value #AnchorPoint_LeftMiddle}</li>
     *     <li>{@link #AnchorPoint_MiddleMiddle} = {@value #AnchorPoint_MiddleMiddle}</li>
     *     <li>{@link #AnchorPoint_RightMiddle} = {@value #AnchorPoint_RightMiddle}</li>
     *     <li>{@link #AnchorPoint_LeftBottom} = {@value #AnchorPoint_LeftBottom}</li>
     *     <li>{@link #AnchorPoint_MiddleBottom} = {@value #AnchorPoint_MiddleBottom}</li>
     *     <li>{@link #AnchorPoint_RightBottom} = {@value #AnchorPoint_RightBottom}</li>
     * </ul>
     * @since 1.7.8
     */
    public void setAnchor(int point ){
        anchorPoint = point;
        mUpdated = true;
        resetAnimate();
    }

    /**
     *
     * This method returns the current anchorPoint position.
     * @return AnchorPoint
     * <ul>
     *     <li>{@link #AnchorPoint_LeftTop} = {@value #AnchorPoint_LeftTop}</li>
     *     <li>{@link #AnchorPoint_MiddleTop} = {@value #AnchorPoint_MiddleTop}</li>
     *     <li>{@link #AnchorPoint_RightTop} = {@value #AnchorPoint_RightTop}</li>*
     *     <li>{@link #AnchorPoint_LeftMiddle} = {@value #AnchorPoint_LeftMiddle}</li>
     *     <li>{@link #AnchorPoint_MiddleMiddle} = {@value #AnchorPoint_MiddleMiddle}</li>
     *     <li>{@link #AnchorPoint_RightMiddle} = {@value #AnchorPoint_RightMiddle}</li>
     *     <li>{@link #AnchorPoint_LeftBottom} = {@value #AnchorPoint_LeftBottom}</li>
     *     <li>{@link #AnchorPoint_MiddleBottom} = {@value #AnchorPoint_MiddleBottom}</li>
     *     <li>{@link #AnchorPoint_RightBottom} = {@value #AnchorPoint_RightBottom}</li>
     * </ul>
     * @since 1.7.8
     */
    public int getAnchor( ){
        return anchorPoint;
    }

    /**
     * This methods sets coordinates to relation coordinate, value of (0~1).
     * @param on true - Relation Coordinate , false - Overlay Pixel Coordinate
     * @since 1.7.8
     */
    public void setRelationCoordinates(boolean on ){
        if(relationCoordinates != on)
            mUpdated = true;
        relationCoordinates = on;
    }

    /**
     * This methods returns whether current coordinates are set to Relation Coordinate or not.
     * @return true -  Relation Coordinate , false - Overlay Pixel Coordinate
     * @since 1.7.8
     */
    public boolean getRelationCoordinates( ){
        return relationCoordinates;
    }

    /**
     * This method gets the alpha value or transparency of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code nexOverlayItem item = mProject.getOverlay(mSelectedPIP.get(position).getId());
        float alpha = item.getAlpha();}
     * @return The value of the alpha set for the overlay image.
     *
     * @see #setAlpha(float)
     * @since version 1.1.0
     */
    public float getAlpha() {
        return mAlpha;
    }

    /**
     *  This method sets the alpha value or transparency of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code mProject.getOverlay(selectedPIPId).setAlpha(mSelectedItemAlpha);}
     * @param value The value of alpha to set to the overlay image, in range of 0 to 1.
     *
     * @see #getAlpha()
     * @since version 1.1.0
     * 
     */
    public void setAlpha(float value) {
        if( mAlpha != value )
            mUpdated = true;
        mAlpha = value;
        resetAnimate();
    }

    /**
     * This method gets the scale factor for width of an overlay image.
     * 
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setScale(mItem.getScaledX(),ratio);}
     * @return The scale factor for the width of the overlay image.
     * 
     * @see #setScale(float, float)
     * @since version 1.1.0
     * 
     */
    public float getScaledX() {
        return mScaledX;
    }

    /**
     * This method gets the scale factor for height of an overlay image.
     * 
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setScale(mItem.getScaledY(),ratio);}
     * @return The scale factory for the height of the overlay image.
     * 
     * @see #setScale(float, float)
     * @since version 1.1.0
     * 
     */
    public float getScaledY() {
        return mScaledY;
    }

    /**
     *
     * @return
     * @since version 1.5.1
     */
    public float getScaledZ() {
        return mScaledZ;
    }

    /**
     * This method sets the scale ratio of an overlay image to be scaled. 
     *
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setScale(mItem.getScaledX(),ratio);}
     * @param scaledX  The scale ratio of the width. 
     * @param scaledY  The scale ratio of the height. 
     * 
     * @see #getScaledX()
     * @see #getScaledY()
     * @since version 1.1.0
     * 
     */
    public void setScale(float scaledX, float scaledY) {
        setScale(scaledX, scaledY, 1);
    }
 
    /**
     * This method sets the scale ratio of an overlay image to be scaled. 
     *
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setScale(mItem.getScaledX(),ratio, mItem.getScaledZ());}
     * @param scaledX  The scale ratio of the width. 
     * @param scaledY  The scale ratio of the height. 
     * @param scaledZ  The scale ratio of the depth. 
     *
     * @see #getScaledX()
     * @see #getScaledY()
     * @see #getScaledZ()
     *
     * @since version 1.5.1
     */
    public void setScale(float scaledX, float scaledY, float scaledZ) {
        mScaledX = scaledX;
        mScaledY = scaledY;
        mScaledZ = scaledZ;
        mUpdated = true;
        resetAnimate();
    }

    /** 
     * This method gets the rotation value of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code nexOverlayItem item = mProject.getOverlay(mSelectedPIP.get(position).getId());
        int rotate = item.getRotate();}
     * @return The rotation value of an overlay image as an <tt>integer</tt>. 
     * 
     * @see #setRotate(int)
     * @since version 1.1.0
     * 
     */
    public int getRotate() {
        return getRotateZ();
    }
 
    /**
     * This method gets the x-axis rotation value of an overlay image. 
     * 
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setRotate(mItem.getRotateX(), degree,mItem.getRotateZ());}
     * @return The x-axis rotation value.
     * @see #getRotateY()
     * @see #getRotateZ()
     * 
     * @since version 1.3.42
     */
    public int getRotateX() {
        return (int) mRotateDegreeX;
    }

     
     /**
     * This method gets the y-axis rotation value of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setRotate(mItem.getRotateY(), degree,mItem.getRotateX());}
     * @return The y-axis rotation value. 
     * @see #getRotateX()
     * @see #getRotateZ()
     * @since version 1.3.42
     */
    public int getRotateY() {
        return (int) mRotateDegreeY;
    }

     
    /**
     * This method gets the z-axis rotation value of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code private nexOverlayItem mItem;
        mItem.setRotate(mItem.getRotateX(), degree,mItem.getRotateZ());}
     * @return The z-axis rotation value. 
     * @see #getRotateY()
     * @see #getRotateX()
     * @since version 1.3.42
     */
    public int getRotateZ() {
        return (int) mRotateDegreeZ;
    }

    /**
     * This method sets the section to be trimmed from an overlay item. 
     * 
     * <p>Example code :</p>
     * {@code if( mEndTrimTime <=  mStartTrimTime ){
            mProject.getOverlay(selectedPIPId).clearTrim();
        } else {
            mProject.getOverlay(selectedPIPId).setTrim(mStartTrimTime, mEndTrimTime);
        }}
     * @param startTime The start time of the section to be trimmed. 
     * @param endTime The end time of the section to be trimmed. 
     * @see #clearTrim()
     * @since version 1.5.0
     */
    public void setTrim(int startTime, int endTime) {
        if( !isVideo() ){
            throw new ClipIsNotVideoException();
        }
        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }
        mTrimStartDuration = startTime;
        mTrimEndDuration = mOverLayImage.getVideoClipInfo().getTotalTime() - endTime;
        mUpdated = true;
        if(mTrimEndDuration < 0 || mTrimStartDuration < 0 ){
            throw new InvalidRangeException(mTrimStartDuration, mTrimEndDuration);
        }
    }
  
    /**
     * This method clears the start time and end time set to the method <tt>setTrim</tt>. 
     * 
     * <p>Example code :</p>
     * {@code if( mEndTrimTime <=  mStartTrimTime ){
            mProject.getOverlay(selectedPIPId).clearTrim();
        }else {
            mProject.getOverlay(selectedPIPId).setTrim(mStartTrimTime, mEndTrimTime);
        }}
     * @see #setTrim(int, int)
     * @since version 1.5.0
     */
    public void clearTrim() {
        if( !isVideo() ){
            throw new ClipIsNotVideoException();
        }
        mTrimStartDuration = 0;
        mTrimEndDuration = 0;
    }
 
    /**
     * This method gets the start time of the section to be trimmed from an overlay item.  
     * 
     * <p>Example code :</p>
     * {@code int startTrim = mProject.getOverlay(selectedPIPId).getStartTrimTime();
        int endTrim = mProject.getOverlay(selectedPIPId).getEndTrimTime();}
     * @return The start time of the trimmed section. 
     * @see #setTrim(int, int)
     * @see #getEndTrimTime()
     * @since version 1.5.0
     */
    public int getStartTrimTime(){
        return mTrimStartDuration;
    }
 
    /**
     * This method gets the end time of the section to be trimmed from an overlay item.  
     * 
     * <p>Example code :</p>
     * {@code int startTrim = mProject.getOverlay(selectedPIPId).getStartTrimTime();
        int endTrim = mProject.getOverlay(selectedPIPId).getEndTrimTime();}
     * @return The end time of the trimmed section. 
     * @see #setTrim(int, int)
     * @see #getStartTrimTime()
     * @since version 1.5.0
     */
    public int getEndTrimTime(){
        return mTrimEndDuration;
    }
    /**
     * This method rotates an overlay image by one of four specific angles (0, 90, 180, 270).
     *  
     * <p>Example code :</p>
     * {@code mEngine.getProject().getOverlay(mOverlayId).setRotate(lastProgress);}
     * @param degree  The amount to rotate the overlay image in degrees, as an <tt>integer</tt>.  This should be one of four possible angles (0, 90, 180, 270).
     * 
     * @see #getRotate()
     * @since version 1.1.0
     * 
     */
    public void setRotate(int degree) {
        setRotate(0, 0, degree);
    }
 
    /**
     * This method gets the rotation value of an overlay image.
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
    overlayItem.setRotate(lastProgress, overlayItem.getRotateY(), overlayItem.getRotateZ}
     * @param degreeX The x-axis rotation value.
     * @param degreeY The y-axis rotation value.
     * @param degreeZ The z-axis rotation value.
     * @see #setRotate(int)
     * @since version 1.3.42
     */
    public void setRotate(int degreeX , int degreeY, int degreeZ) {
        mRotateDegreeX = degreeX;
        mRotateDegreeY = degreeY;
        mRotateDegreeZ = degreeZ;
        mUpdated = true;
        resetAnimate();
    }

    /**
     * This method is the same as setRotate(int degree), but it accepts float parameter value.
     * @param degree The amount to rotate the overlay image in degrees, as an <tt>float</tt>.
     * @since 1.7.7
     */
    public void setRotate(float degree) {
        setRotate(0, 0, degree);
    }

    /**
     * This method is the same as setRotate(int degreeX , int degreeY, int degreeZ), but it accepts float parameter values.
     * @param degreeX The x-axis rotation value.
     * @param degreeY The y-axis rotation value.
     * @param degreeZ The z-axis rotation value.
     * @since 1.7.7
     */
    public void setRotate(float degreeX , float degreeY, float degreeZ) {
        mRotateDegreeX = degreeX;
        mRotateDegreeY = degreeY;
        mRotateDegreeZ = degreeZ;
        mUpdated = true;
        resetAnimate();
    }

    /**
     * This method gets the brightness of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        int Brightness = overlayItem.getBrightness();}
     * @return The brightness of the overlay image, as an <tt>integer</tt> . 
     * 
     * @see #setBrightness(int)
     * @since version 1.1.0
     * 
     */
    public int getBrightness() {
        return mBrightness;
    }

    /** 
     * This method gets the contrast of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        int Contrast = overlayItem.getContrast();}
     * @return The contrast of the overlay image, as an <tt>integer</tt> between -255 and 255. 
     * 
     * @see #setContrast(int)
     * @since version 1.1.0
     * 
     */
    public int getContrast() {
        return mContrast;
    }

    /**
     * This method gets the saturation of an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        int Saturation = overlayItem.getSaturation();}
     * @return The saturation of the overlay image, as an <tt>integer</tt> between -255 and 255. 
     * 
     * @see #setSaturation(int)
     * @since version 1.1.0
     * 
     */
    public int getSaturation() {
        return mSaturation;
    }

    public int getHue(){

        return mHue;
    }
    /**
     * This method sets the brightness for an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        overlayItem.setBrightness(lastProgress);}
     * @param value  The brightness to set for the overlay image, in the range of -255 to 255.
     *
     * @return <tt>TRUE</tt> when successful, otherwise <tt>FALSE</tt> when the value is not in the permitted range. 
     *
     * @see #getBrightness()
     * @since version 1.1.0
     * 
     */
    public boolean setBrightness(int value) {
        if (value < -255 || value > 255)
            return false;
        if( mBrightness != value )
            mUpdated = true;
        mBrightness = value;
        return true;
    }

    /**
     *  This method sets the contrast for an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        overlayItem.setContrast(lastProgress);}
     * @param value  The contrast to set for the overlay image, in the range of -255 to 255.
     * 
     * @return <tt>TRUE</tt> when successful, otherwise <tt>FALSE</tt> when the value is not in the permitted range. 
     *
     * @see #getContrast()
     * @since version 1.1.0
     * 
     */
    public boolean setContrast(int value) {
        if (value < -255 || value > 255)
            return false;
        if( mContrast != value )
            mUpdated = true;
        mContrast = value;
        return true;
    }

    /**
    *  This method sets the saturation for an overlay image. 
     *
     * <p>Example code :</p>
     * {@code final nexOverlayItem overlayItem = new nexOverlayItem(overlay,640,360,0,0+6000);
        overlayItem.setSaturation(lastProgress);}
     * @param value  The saturation value to set for the overlay image, in the range of -255 to 255.
     * 
     * @return <tt>TRUE</tt> when successful, otherwise <tt>FALSE</tt> when the value is not in the permitted range. 
     *
     * @see #getSaturation()
     * @since version 1.1.0
     * 
     */
    public boolean setSaturation(int value) {
        if (value < -255 || value > 255)
            return false;
        if( mSaturation != value )
            mUpdated = true;
        mSaturation = value;
        return true;
    }

    public boolean setHue(int value){

        if(value < -180 || value > 180)
            return false;
        return true;
    }

    void setTime(int time) {
        //TODO:
        if (mTime > time) {
            resetAnimate();
        }
        mTime = time;
    }

    boolean bApplayLayerExpression = true;

    /**
     * @deprecated
     * @param bApplayLayerExpression
     */
    @Deprecated
    public void setLayerExpressionParam(boolean bApplayLayerExpression) {
        if( this.bApplayLayerExpression != bApplayLayerExpression )
            mUpdated = true;
        this.bApplayLayerExpression = bApplayLayerExpression;
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public boolean getLayerExpressiontParam() {
        return bApplayLayerExpression;
    }

    private void runAnimate(nexAnimate item, int cts){

        if (item instanceof nexAnimate.AnimateImages) {
            animateResourceId = item.getImageResourceId(cts);
            if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]AnimateImages=(" + animateResourceId + ")");
        } else if (item instanceof nexAnimate.Move) {
            if (mLayerExpression.getID() == 0 ) {
                mAnimateTranslateXpos = item.getTranslatePosition(cts, nexAnimate.kCoordinateX);
                mAnimateTranslateYpos = item.getTranslatePosition(cts, nexAnimate.kCoordinateY);
                mAnimateTranslateZpos = item.getTranslatePosition(cts, nexAnimate.kCoordinateZ);
                if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]Move to=(" + mAnimateTranslateXpos + "," + mAnimateTranslateYpos +","+ mAnimateTranslateZpos + ")");
            }
        } else if (item instanceof nexAnimate.Alpha) {
            if (mLayerExpression.getID() == 0) {
                mAnimateLastAlpha = item.getAlpha(cts);
                if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]Alpha =(" + mAnimateLastAlpha + ")");
            }
        } else if (item instanceof nexAnimate.Rotate) {
            if (mLayerExpression.getID() == 0) {
                mAnimateLastRotateDegreeX = item.getAngleDegree(cts, mRotateDegreeX, nexAnimate.kCoordinateX);
                mAnimateLastRotateDegreeY = item.getAngleDegree(cts, mRotateDegreeY, nexAnimate.kCoordinateY);
                mAnimateLastRotateDegreeZ = item.getAngleDegree(cts, mRotateDegreeZ, nexAnimate.kCoordinateZ);
                if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]Rotate =("  + mAnimateLastRotateDegreeX + "," + mAnimateLastRotateDegreeY +","+ mAnimateLastRotateDegreeZ + ")");
            }
        } else if (item instanceof nexAnimate.Scale) {
            if (mLayerExpression.getID() == 0) {
                mAnimateLastScaledX = item.getScaledRatio(cts,mScaledX,nexAnimate.kCoordinateX);
                mAnimateLastScaledY = item.getScaledRatio(cts,mScaledY,nexAnimate.kCoordinateY);
                mAnimateLastScaledZ = item.getScaledRatio(cts,mScaledY,nexAnimate.kCoordinateZ);
                if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]Scale =(" + mAnimateLastScaledX + "," + mAnimateLastScaledY +","+mAnimateLastScaledZ+ ")");
            }
        } else {
            if (mLayerExpression.getID() == 0) {
                if( item.onFreeTypeAnimate(cts, this) ) {
                    mAnimateTranslateXpos = item.mdX;
                    mAnimateTranslateYpos = item.mdY;
                    mAnimateTranslateZpos = item.mdZ;

                    mAnimateLastAlpha = item.mAlpha;

                    mAnimateLastRotateDegreeX = item.mRotateDegreeX;
                    mAnimateLastRotateDegreeY = item.mRotateDegreeY;
                    mAnimateLastRotateDegreeZ = item.mRotateDegreeZ;

                    mAnimateLastScaledX = item.mScaledX;
                    mAnimateLastScaledY = item.mScaledY;
                    mAnimateLastScaledZ = item.mScaledZ;
                    if(LL.D) Log.d(TAG, "["+getId()+"]["+cts+"]FreeType =(" + mAnimateTranslateXpos + "," + mAnimateTranslateYpos +","+mAnimateTranslateZpos+ ")");
                }
            }
        }
    }

    private float getRelativeScale(int layerWidth, int layerHeight){
        float widthScale = 1;
        float heightScale = 1;
        int aspectWidth = nexApplicationConfig.getAspectProfile().getWidth();
        int aspectHeigh = nexApplicationConfig.getAspectProfile().getHeight();


        if( aspectWidth > layerWidth ){
            widthScale = ((float)layerWidth/(float)aspectWidth);
        }
        if( aspectHeigh > aspectHeigh){
            heightScale = ((float)aspectHeigh/(float)aspectHeigh);
        }

        if( heightScale > widthScale ){
            return widthScale;
        }
        return heightScale;
    }

    void renderOverlay(LayerRenderer renderer, Context context) {
        String lruId = null;

        if( !showItem ){
            return;
        }
        lastLayerWidth = (int)renderer.getOutputWidth();
        lastLayerHeight = (int)renderer.getOutputHeight();

        float baseScale = 1;
        if( relationCoordinates )
            baseScale = getRelativeScale(lastLayerWidth,lastLayerHeight);

        setTime(renderer.getCurrentTime());

        int [] positions = getRealPositions(true);
        int x = positions[0];
        int y = positions[1];
        int z = positions[2];

        animateResourceId = 0;

        if (mAniList != null) {

            for( nexAnimate temp : mAniList ){
                int aniTime = mTime - mStartTime;
                boolean isActive = mActiveAnimateList.contains(temp);
                if (temp.mStartTime <= aniTime && temp.getEndTime() > aniTime) {
                    if( !isActive ){
                        mActiveAnimateList.add(temp);
                        if( aniTime - temp.mStartTime < 33 ) {
                            runAnimate(temp, 0);
                        }else{
                            runAnimate(temp, aniTime);
                        }
                    }else{
                        runAnimate(temp,aniTime);
                    }
                }else{
                    if( isActive ){
                        mActiveAnimateList.remove(temp);
                        runAnimate(temp,temp.getEndTime());
                    }
                }
            }
        }

       renderer.save();

       if (mMask != null) {
           if (mMask.onOff && !mMask.syncAnimationOverlayItem) {

               renderer.clearMask();
               renderer.setRenderTarget(LayerRenderer.RenderTarget.Mask);
               mMask.getPosition(mMaskRect);
               if( mMask.getMaskImage() != null ){
                   renderer.drawBitmap(mMask.getMaskImage(),mMaskRect.left, mMaskRect.top, mMaskRect.right, mMaskRect.bottom);
               }else {
                   renderer.fillRect(0xFFFFFFFF, mMaskRect.left, mMaskRect.top, mMaskRect.right, mMaskRect.bottom);
               }
               renderer.setMaskEnabled(true);
               renderer.setRenderTarget(LayerRenderer.RenderTarget.Normal);
           }
       }

       renderer.translate(x,y);
       renderer.translate(mAnimateTranslateXpos, mAnimateTranslateYpos);
       renderer.scale(mAnimateLastScaledX*baseScale, mAnimateLastScaledY*baseScale);
       //renderer.rotate(mAnimateLastRotateDegreeZ, 0, 0);

        renderer.rotateAroundAxis(mAnimateLastRotateDegreeX,1,0,0);
        renderer.rotateAroundAxis(mAnimateLastRotateDegreeY,0,1,0);
        renderer.rotateAroundAxis(mAnimateLastRotateDegreeZ,0,0,1);
        //renderer.rotate(mAnimateLastRotateDegreeZ, 0, 0);
        if( showOutLien ){
            renderer.setAlpha(1);
        }else {
            renderer.setAlpha(mAnimateLastAlpha);
        }

       if (mMask != null) {
           if (mMask.onOff && mMask.syncAnimationOverlayItem) {

               renderer.clearMask();
               renderer.setRenderTarget(LayerRenderer.RenderTarget.Mask);
               mMask.getPosition(mMaskRect);
               if( mMask.getMaskImage() != null ){
                   renderer.drawBitmap(mMask.getMaskImage(),mMaskRect.left, mMaskRect.top, mMaskRect.right, mMaskRect.bottom);
               }else {
                   renderer.fillRect(0xFFFFFFFF, mMaskRect.left, mMaskRect.top, mMaskRect.right, mMaskRect.bottom);
               }
               renderer.setMaskEnabled(true);
               renderer.setRenderTarget(LayerRenderer.RenderTarget.Normal);
           }
       }

       if( awakeAsset != null ){
           awakeAsset.onRender(renderer,cacheMotion,mStartTime,mEndTime);
       }else if( mOverLayImage != null ) {
           if( mOverLayImage.isVideo() ){
               if (getChromaKey().getChromaKeyEnabled()) {
                   renderer.setChromakeyEnabled(getChromaKey().getChromaKeyEnabled());
                   renderer.setChromakeyMaskEnabled(getChromaKey().getChromaKeyMaskEnabled());
                   renderer.setChromakeyColor(getChromaKey().m_chromaKeyColor
                           , getChromaKey().m_chromaKeyClipFG
                           , getChromaKey().m_chromaKeyClipBG
                           , getChromaKey().m_chromaKeyBlend_x0
                           , getChromaKey().m_chromaKeyBlend_y0
                           , getChromaKey().m_chromaKeyBlend_x1
                           , getChromaKey().m_chromaKeyBlend_y1
                   );
               }
            //    int ret = EditorGlobal.getEditor().getTexNameForVideoLayer(renderer.getRenderMode().id, mVideoEngineId, renderer.getTexMatrix());
            //    int texname = ret&0x00FFFFFF;
            //    int type =    (ret&0xFF000000) >> 24;
               int ret = EditorGlobal.getEditor().getTexNameForVideoLayer(renderer.getRenderMode().id, mVideoEngineId);
               int texname = ret;
               int type =    0;
               
               renderer.setColorMatrix(NexTheme_Math.colorAdjust(getCombinedBrightness() / 255f, getCombinedContrast() / 255f, getCombinedSaturation() / 255f, getTintColor()));
               renderer.drawDirect(texname, type, 0.0f, 0.0f, mOverLayImage.getVideoClipInfo().getWidth(), mOverLayImage.getVideoClipInfo().getHeight(), mFlipMode);
               renderer.setChromakeyEnabled(false);
               renderer.setLUT(null);
           }else{
               Bitmap bitmap = null;
               NexCache nexCache = NexCache.getInstance();

               int resourceId = animateResourceId;
               if( resourceId == 0 ){
                   resourceId = mOverLayImage.mResourceId;
               }

               if( resourceId == 0 ) {
                   lruId = mOverLayImage.getUserBitmapID();

                   if( mOverLayImage.mUpdate ){
                       mOverLayImage.mUpdate = false;
                       bitmap = null;
                       nexCache.removeBitmapFromMemoryCache(lruId);
                   }else {
                       bitmap = nexCache.getBitmapFromMemoryCache(lruId);
                   }

                   if( bitmap == null ) {
                       bitmap = mOverLayImage.getUserBitmap();
                       if (bitmap != null) {
                           try {
                               nexCache.addBitmapToMemoryCache( lruId , bitmap);
                           } catch(NullPointerException npe) {
                               if(LL.D) Log.d(TAG, "exception: message="+npe.getMessage());
                           }
                       }
                   }
               } else {
                   lruId = mOverLayImage.getUserBitmapID()+resourceId;
                   bitmap = nexCache.getBitmapFromMemoryCache(lruId);
                   if (bitmap == null) {
                       bitmap = BitmapFactory.decodeResource(context.getResources(), resourceId);
                       if (bitmap != null) {
                           try {
                               nexCache.addBitmapToMemoryCache(lruId, bitmap);
                           } catch(NullPointerException npe) {
                               if(LL.D) Log.d(TAG, "exception: message="+npe.getMessage());
                           }
                       }
                   }
               }

               if(bitmap != null) {
                   Log.d(TAG, "renderOverlay bitmap id = " + lruId + ", wid = " + bitmap.getWidth() + ", hei = " + bitmap.getHeight() +
                           ", X=" + x + ", Y=" + y+", Z="+z+", ScaledX="+mAnimateLastScaledX+ ", Alpha="+mAnimateLastAlpha+
                           ", Rx="+mAnimateLastRotateDegreeX+", Ry="+mAnimateLastRotateDegreeY+", Rz="+mAnimateLastRotateDegreeZ+
                           ", flip="+mFlipMode+", baseScale="+baseScale+
                           ", cts="+renderer.getCurrentTime());
                   renderer.setColorMatrix(NexTheme_Math.colorAdjust(getCombinedBrightness() / 255f, getCombinedContrast() / 255f, getCombinedSaturation() / 255f, getTintColor()));
                   if( mOverLayImage != null ) {
                       Rect rc = getDrawBitmapPosition(mOverLayImage.getAnchorPoint(), bitmap.getWidth(), bitmap.getHeight());
                       renderer.drawBitmap(bitmap, rc.left, rc.top, rc.right, rc.bottom,mFlipMode);
                   }else{
                       renderer.drawBitmap(bitmap,mFlipMode);
                   }
               }
           }
       }

       renderer.restore();
       if(showOutLien) {
           if( overlayOutLine != null ){
               overlayOutLine.renderOutLine(this,renderer);
           }
       }
    }

    private static Rect getDrawBitmapPosition(int anchorPoint, int bitmapWidth, int bitmapHeight ){
        Rect rc = new Rect(-bitmapWidth / 2, -bitmapHeight / 2, bitmapWidth / 2, bitmapHeight / 2);
        switch (anchorPoint){
            case AnchorPoint_LeftTop:
                rc.set(0,0,bitmapWidth,bitmapHeight);
                break;

            case AnchorPoint_LeftMiddle:
                rc.set(0,-bitmapHeight / 2,bitmapWidth,bitmapHeight/2);
                break;
            case AnchorPoint_LeftBottom:
                rc.set(0,-bitmapHeight ,bitmapWidth,0);
                break;
            case AnchorPoint_MiddleTop:
                rc.set(-bitmapWidth / 2,0,bitmapWidth/2,bitmapHeight);
                break;

            case AnchorPoint_MiddleMiddle:
                break;
            case AnchorPoint_MiddleBottom:
                rc.set(-bitmapWidth / 2,-bitmapHeight,bitmapWidth/2,0);
                break;
            case AnchorPoint_RightTop:
                rc.set(-bitmapWidth ,0,0,bitmapHeight);
                break;

            case AnchorPoint_RightMiddle:
                rc.set(-bitmapWidth ,-bitmapHeight / 2,0,bitmapHeight/2);
                break;
            case AnchorPoint_RightBottom:
                rc.set(-bitmapWidth ,-bitmapHeight ,0, 0);
                break;

        }
        return rc;
    }

    /** 
     * This method gets the start time of an overlay image (based on the total playtime of the project).
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem temp : mProjectOverlays ){
            if (temp.getStartTime() <= point.mTime && temp.getEndTime() > point.mTime) {
                return temp.isPointInOverlayItem(point);
            }
        }}
     * @return The start time of the overlay image. 
     * 
     * @see #setTimePosition(int, int)
     * @since version 1.1.0
     * 
     */
    public int getStartTime(){
        return mStartTime;
    }
 
    /** 
     * This method gets the end time of an overlay image (based on the total playtime of the project).
     * 
     * <p>Example code :</p>
     * {@code for( nexOverlayItem temp : mProjectOverlays ){
            if (temp.getStartTime() <= point.mTime && temp.getEndTime() > point.mTime) {
                return temp.isPointInOverlayItem(point);
            }
        }}
     * @return The end time of the overlay image in <tt>msec</tt> (milliseconds), as an <tt>integer</tt>. 
     * 
     * @see #setTimePosition(int, int)
     * @since version 1.1.0
     * 
     */
    public int getEndTime(){
        return mEndTime;
    }

    /** 
     * This method sets the duration of an overlay image (based on the total playtime of the project). 
     * 
     * The parameter <tt>startTime</tt> sets when the overlay image will be displayed, relative to the
     * project timeline, and <tt>endTime</tt> sets when the overlay image will stop being displayed.
     * 
     * <p>Example code :</p>
     * {@code overlayItem.setTimePosition(startTime, endTime);}
     * @param startTime The start time to set for the overlay image on the project timeline in <tt>msec</tt> (milliseconds).  
     * @param endTime The end time to set for the overlay image on the project timeline in <tt>msec</tt> (milliseconds). 
     * 
     * @see #getStartTime()
     * @see #getEndTime()
     * @since version 1.1.0
     */
    public void setTimePosition(int startTime, int endTime){
        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }
        mUpdated = true;
        mStartTime = startTime;
        mEndTime = endTime;
    }

    /**
     * This method gets the end time of an animation effect customized by the developer. 
     * 
     * <p>Example code :</p>
     * {@code int endTime = overlayItem.getAnimateEndTime();}
     * @return The end time of the developer customized animation effect. 
     * @since version 1.1.0
     */
    public int getAnimateEndTime(){
        if( mAniList == null ){
            return 0;
        }
        int duration = 0;
        for( nexAnimate tmp : mAniList ){
            if( duration < tmp.getEndTime() ){
                duration = tmp.getEndTime();
            }
        }
        return duration;
    }

    /**
     * This method adds any animation effect customized by the developer to the class <tt>nexOverlayItem</tt>.
     * 
     * <p>Example code :</p>
     * {@code nexAnimate ani = nexAnimate.getAnimateImages(
            0, 5000, R.drawable.ilove0001
            , R.drawable.ilove0002
            , R.drawable.ilove0003
            , R.drawable.ilove0004
            , R.drawable.ilove0005
            , R.drawable.ilove0006
            , R.drawable.ilove0007
            , R.drawable.ilove0008
            , R.drawable.ilove0009
            , R.drawable.ilove0010
            );
        overlayItem.addAnimate(ani);}
     * @param animate An instance of the animation effect to add to the NexEditor&trade;&nbsp;SDK. 
     *
     * @see #clearAnimate()
     * @since version 1.1.0
     */
    public void addAnimate(nexAnimate animate){
        if( mAniList == null ){
            mAniList = new ArrayList<nexAnimate>();
        }

//        animate.setStartPos(mX,mY);
        mAniList.add(animate);
        mUpdated = true;
        Collections.sort(mAniList, new StartTimeDesCompare());
    }

    /**
     * This method clears all the customized animation effects added by a developer. 
     * 
     * <p>Example code :</p>
     * {@code overlayItem.clearAnimate();}
     * @see #addAnimate(nexAnimate)
     * @since version 1.1.0
     */
    public void clearAnimate(){
        if( mAniList != null ){
            mAniList.clear();
            mUpdated = true;
            resetAnimate();
        }
    }

    private static class StartTimeDesCompare implements Comparator<nexAnimate> {
        @Override
        public int compare(nexAnimate arg0, nexAnimate arg1) {
            return arg0.mStartTime > arg1.mStartTime ? -1 : arg0.mStartTime < arg1.mStartTime ? 1:0;
        }
    }
 
    /**
     * This method clears the overlay item cache. 
     *
     * <p>Example code :</p>
     * {@code for( nexOverlayItem temp : mProjectOverlays ){
            Log.d(TAG,"Overlay clear id = "+temp.getId());
            temp.clearCache();
        }
        mProjectOverlays.clear();}
     * @since version 1.3.23
     */
    @Deprecated
    public void clearCache(){
    }

    boolean isVideo(){
        if(  mOverLayImage == null )
            return false;
        return mOverLayImage.isVideo();
    }

    void setOverlayTitle(boolean overlayTitle)
    {
        mOverlayTitle = overlayTitle;
    }

    boolean getOverlayTitle()
    {
        return mOverlayTitle;
    }

    /**
     * This class manages colors to control transparency level.
     * @since 1.7.0
     */
    static public class ChromaKey{
        private static final float CHROMA_DEF_CLIP_BG = 0.50f;
        private static final float CHROMA_DEF_CLIP_FG = 0.72f;
        private static final float CHROMA_DEF_BLEND_X0 = 0.25f;
        private static final float CHROMA_DEF_BLEND_Y0 = 0.25f;
        private static final float CHROMA_DEF_BLEND_X1 = 0.75f;
        private static final float CHROMA_DEF_BLEND_Y1 = 0.75f;
        private static final int CHROMA_UNSET_COLOR = 0x00000000;
        private static final int CHROMA_DEF_COLOR = 0xFF00FF00;
        private float[] m_chromaKeyDivisions = new float[]{0.05f,0.3f,0.5f,0.65f};
        private float[] m_chromaKeyStrengths = new float[]{0, 0.25f, 0.75f, 1f};
        private int m_chromaKeyColor = CHROMA_UNSET_COLOR;
        private boolean m_chromaKeyEnabled;
        private boolean m_chromaKeyMaskEnabled;
        private float m_chromaKeyClipFG = CHROMA_DEF_CLIP_FG;
        private float m_chromaKeyClipBG = CHROMA_DEF_CLIP_BG;
        private float m_chromaKeyBlend_x0 = CHROMA_DEF_BLEND_X0;
        private float m_chromaKeyBlend_y0 = CHROMA_DEF_BLEND_Y0;
        private float m_chromaKeyBlend_x1 = CHROMA_DEF_BLEND_X1;
        private float m_chromaKeyBlend_y1 = CHROMA_DEF_BLEND_Y1;

        /**
         * This method returns a list of colors, which transparency is to be applied, from a video recorded with blue screen background.
         * For better result, background portion of a video should be big enough.
         * @param thumb thumbnail image of a video.
         * @return an array of colors which transparency is to be applied.
         * @since 1.7.0
         */
        static public int[] getChromaKeyRecommendedColors(Bitmap thumb ) {
                if( thumb!=null ) {
                    final int DimH = 360;
                    float[] hsv = new float[3];
                    int[] histogram = new int[DimH];
                    int[] pixels = new int[thumb.getWidth() * thumb.getHeight()];
                    thumb.getPixels(pixels,0,thumb.getWidth(),0,0,thumb.getWidth(),thumb.getHeight());
                    for( int i=0; i<pixels.length; i++ ) {
                        Color.colorToHSV(pixels[i],hsv);
                        if( hsv[1] < 0.3f || hsv[2]<0.2f )
                            continue;
                        int h = (int)(hsv[0] / 360.0f * (float)DimH) % DimH;
                        histogram[h]++;
                    }
                    if( LL.D ) {
                        for (int i = 0; i < histogram.length; i++) {
                            //Log.d(LOG_TAG, "Chroma:Hist[" + i + "]=" + histogram[i]);
                        }
                    }
                    int reccount = 0;
                    int[] reccolor = new int[14];
                    for( int i=0; i<reccolor.length; i++ ) {

                        int peakIdxH = -1;
                        int peakValue = -1;
                        for(int ih=0; ih<DimH; ih++){
                            if(histogram[ih] > peakValue){
                                peakValue = histogram[ih];
                                peakIdxH = ih;
                            }
                        }

                        if( peakIdxH < 0 || peakValue<5 ) {
                            break;
                        }
                        hsv[0] = peakIdxH*360/DimH;
                        hsv[1] = 1.0f;
                        hsv[2] = 1.0f;
                        reccolor[reccount] = Color.HSVToColor(hsv);
                        reccount++;
                        for(int ih=peakIdxH-3; ih<peakIdxH+3; ih++){
                            histogram[(ih+DimH)%DimH] = -1;
                        }
                    }
                    if( reccount < reccolor.length ) {
                        int[] trunc = new int[reccount];
                        System.arraycopy(reccolor,0,trunc,0,reccount);
                        reccolor = trunc;
                    }
                    return reccolor;
                }
            return new int[0];
        }

        /**
         *
         * @param divisions
         * @since 1.7.8
         */
        public void getChromaKeyDivisions(float[] divisions) {
            System.arraycopy(m_chromaKeyDivisions,0,divisions,0,m_chromaKeyDivisions.length);
        }

        /**
         *
         * @param strengths
         * @since 1.7.8
         */
        public void getChromaKeyStrengths(float[] strengths) {
            System.arraycopy(m_chromaKeyStrengths,0,strengths,0,m_chromaKeyStrengths.length);
        }

        /**
         *
         * @param strengths
         * @since 1.7.8
         */
        public void setChromaKeyStrengths(float[] strengths) {
            System.arraycopy(strengths,0,m_chromaKeyStrengths,0,m_chromaKeyStrengths.length);
        }

        /**
         * This method returns a color to make it transparent.
         * @return
         * @since 1.7.8
         */
        public int getChromaKeyColor() {
            /*
            if( m_chromaKeyColor==CHROMA_UNSET_COLOR ) {
                int[] colors = getChromaKeyRecommendedColors();
                if( colors.length > 0 ) {
                    m_chromaKeyColor = colors[0];
                } else {
                    m_chromaKeyColor = CHROMA_DEF_COLOR;
                }
            }
            */
            return m_chromaKeyColor;
        }

        /**
         * This method sets the color to make it transparent.
         * @param color
         * @since 1.7.8
         */
        public void setChromaKeyColor(int color) {
            m_chromaKeyColor = color;
        }

        /**
         * This method checks whether ChromaKey is enabled or not.
         * @return
         * @since 1.7.8
         */
        public boolean getChromaKeyEnabled() {
            return m_chromaKeyEnabled;
        }

        /**
         *
         * @param enabled
         * @since 1.7.8
         */
        public void setChromaKeyMaskEnabled(boolean enabled) {
            m_chromaKeyMaskEnabled = enabled;
        }

        /**
         *
         * @return
         * @since 1.7.8
         */
        public boolean getChromaKeyMaskEnabled() {
            return m_chromaKeyMaskEnabled;
        }

        /**
         * This method sets whether to enable ChromaKey or not.
         * @param enabled true - ChromeKey on , false - ChromeKey off
         * @since 1.7.0
         */
        public void setChromaKeyEnabled(boolean enabled) {
            m_chromaKeyEnabled = enabled;
        }

        /**
         *
         * @param fgClip
         * @since 1.7.8
         */
        public void setChromaKeyFGClip(float fgClip) {
            m_chromaKeyClipFG = fgClip;
        }

        /**
         *
         * @param bgClip
         * @since 1.7.8
         */
        public void setChromaKeyBGClip(float bgClip) {
            m_chromaKeyClipBG = bgClip;
        }

        /**
         *
         * @return
         * @since 1.7.8
         */
        public float getChromaKeyFGClip() {
            return m_chromaKeyClipFG;
        }

        /**
         *
         * @return
         * @since 1.7.8
         */
        public float getChromaKeyBGClip() {
            return m_chromaKeyClipBG;
        }

        /**
         *
         * @param xyxy
         * @since 1.7.8
         */
        public void setChromaKeyBlend(float[] xyxy) {
            m_chromaKeyBlend_x0 = xyxy[0];
            m_chromaKeyBlend_y0 = xyxy[1];
            m_chromaKeyBlend_x1 = xyxy[2];
            m_chromaKeyBlend_y1 = xyxy[3];
        }

        /**
         *
         * @param xyxy
         * @since 1.7.8
         */
        public void getChromaKeyBlend(float[] xyxy) {
            xyxy[0] = m_chromaKeyBlend_x0;
            xyxy[1] = m_chromaKeyBlend_y0;
            xyxy[2] = m_chromaKeyBlend_x1;
            xyxy[3] = m_chromaKeyBlend_y1;
        }
    }


    /**
     * This method returns a ChromaKey instance to be used in an Overlay.
     * @return ChromaKey instance
     * @since 1.7.0
     */
    public ChromaKey getChromaKey(){
        if( mChromaKey == null ){
            mChromaKey = new ChromaKey();
        }
        return mChromaKey;
    }

    /**
     * This class allows masking parts of the overlay image. 
     *
     * @note Only rectangular masks are supported in version 1.5.3.
     *
     * @since version 1.5.3
     */
    public class Mask{
        private int angle;
        private boolean onOff;
        private Rect rectPosition = new Rect();
        private Bitmap maskImage;
        private int splitMode;
        private boolean vertical;
        private boolean syncAnimationOverlayItem;
        /**
         * This method splits the overlay image into 2 columns and only shows the left, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_Left = 1;

        /**
         * This method splits the overlay image into 2 columns and only shows the right, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_Right = 2;

        /**
         * This method splits the overlay image into 2 rows and only shows the top, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_Top = 3;

        /**
         * This method splits the overlay image into 2 rows and only shows the bottom, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_Bottom = 4;

        /**
         * This method splits the overlay image into 2 x 2 blocks and only shows the top-left, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_LeftTop = 5;

        /**
         * This method splits the overlay image into 2 x 2 blocks and only shows the top-right, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_RightTop = 6;

        /**
         * This method splits the overlay image into 2 x 2 blocks and only shows the bottom-left, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_LeftBottom = 7;

        /**
         * This method splits the overlay image into 2 x 2 blocks and only shows the bottom-right, if using {@link #setSplitMode(int, boolean)}.
         *
         * @since version 1.5.3
         */
        static public final int kSplit_RightBottom = 8;

        /**
         * This method sets the activation state of the mask.
         *
         * @param onOff Decides whether or not to activate the mask.
         * - <b>TRUE</b> Activate.
         * - <b>FALSE</b> Deactivate.
         *
         * @since version 1.5.3
         */
        public void setState( boolean onOff ){
            this.onOff = onOff;
        }

        /**
         * This method gets the activation state of the mask.
         * @return <tt>TRUE</tt> if activated; otherwise, <tt>FALSE</tt>.
         * @since version 1.5.3
         */
        public boolean getState( ){
            return onOff;
        }

        /**
         * This method rotates the masked image counterclockwise.
         * @param degree ( 0 - 360 )
         * @since version 1.5.3
         */
        public void setAngle(int degree){
            angle = degree;
        }

        /**
         * This method gets the set rotation value to the {@link #setAngle(int)}. 
         * @return The value of the angle.
         * @since version 1.5.3
         */
        public int getAngle(){
            return angle;
        }

        /**
         * This method gets the value of the SplitMode, which is a predefined mask.
         * @return The SplitMode value.
         * @since version 1.5.3
         */
        public int getSplitMode(){
            return splitMode;
        }

        /**
         * This method indicates the width of the mask.
         * @return The width of the mask.
         * @since version 1.5.3
         */
        public int width(){
            return rectPosition.width();
        }

        /**
         * This method indicates the height of the mask.
         * @return The height of the mask.
         * @since version 1.5.3
         */
        public int height(){
            return rectPosition.height();
        }

        public void setMaskImage(Bitmap maskImage){
            this.maskImage = maskImage;
        }

        public Bitmap getMaskImage(){
            return  maskImage;
        }

        /**
         * This method sets the splitting mode for the overlay image.
         *
         * @param splitMode The splitting mode defined in the class <tt>Mask</tt>.
         * @param vertical Decides which way to split the overlay image.
         * - <b>TRUE</b> Split into 2 columns.
         * - <b>FALSE</b> Split into 2 rows.
         *
         * @since version 1.5.3
         */
        public void setSplitMode(int splitMode , boolean vertical){

            if( this.splitMode == splitMode &&  this.vertical == vertical ){
                return;
            }

            this.splitMode = splitMode;
            this.vertical = vertical;
            int width = nexApplicationConfig.getAspectProfile().getWidth();
            int height =nexApplicationConfig.getAspectProfile().getHeight();
            switch (splitMode){
                case kSplit_Left:
                    rectPosition.left = 0;
                    rectPosition.top = 0;
                    rectPosition.right = width/2;
                    rectPosition.bottom = height;
                    /*
                    if( vertical ) {
                        rectPosition.right = 360;
                        rectPosition.bottom = 1280;
                    }else{
                        rectPosition.right = 640;
                        rectPosition.bottom = 720;
                    }
                    */
                    break;
                case kSplit_Right:
                    //Log.d(TAG, "kSplit_Right -> vertical:" + vertical);
                    rectPosition.top = 0;
                    rectPosition.left = width/2;
                    rectPosition.right = width;
                    rectPosition.bottom = height;

                    /*
                    if( vertical ) {
                        rectPosition.left = 360;
                        rectPosition.right = 720;
                        rectPosition.bottom = 1280;
                    }else{
                        rectPosition.left = 640;
                        rectPosition.right = 1280;
                        rectPosition.bottom = 720;
                    }
                    */
                    break;
                case kSplit_Top:
                    rectPosition.top = 0;
                    rectPosition.left = 0;
                    rectPosition.right = width;
                    rectPosition.bottom = height/2;

                    /*
                    if( vertical ) {
                        rectPosition.right = 720;
                        rectPosition.bottom = 640;
                    }else{
                        rectPosition.right = 1280;
                        rectPosition.bottom = 360;
                    }
                    */
                    break;
                case kSplit_Bottom:
                    rectPosition.top = height/2;
                    rectPosition.left = 0;
                    rectPosition.right = width;
                    rectPosition.bottom = height;

                    /*
                    if( vertical ) {
                        rectPosition.top = 640;
                        rectPosition.left = 0;
                        rectPosition.right = 720;
                        rectPosition.bottom = 1280;
                    }else{
                        rectPosition.top = 360;
                        rectPosition.left = 0;
                        rectPosition.right = 1280;
                        rectPosition.bottom = 720;
                    }
                    */
                    break;
                case kSplit_LeftTop:
                    rectPosition.top = 0;
                    rectPosition.left = 0;
                    rectPosition.right = width/2;
                    rectPosition.bottom = height/2;

                    /*
                    if( vertical ) {
                        rectPosition.right = 360;
                        rectPosition.bottom = 640;
                    }else{
                        rectPosition.right = 640;
                        rectPosition.bottom = 360;
                    }
                    */
                    break;
                case kSplit_RightTop:
                    rectPosition.top = 0;
                    rectPosition.left = width/2;
                    rectPosition.right = width;
                    rectPosition.bottom = height/2;

                    /*
                    if( vertical ) {
                        rectPosition.left = 360;
                        rectPosition.right = 720;
                        rectPosition.bottom = 640;
                    }else{
                        rectPosition.left = 640;
                        rectPosition.right = 1280;
                        rectPosition.bottom = 360;
                    }
                    */
                    break;
                case kSplit_LeftBottom:
                    rectPosition.top = height/2;
                    rectPosition.left = 0;
                    rectPosition.right = width/2;
                    rectPosition.bottom = height;

                    /*
                    if( vertical ) {
                        rectPosition.top = 640;
                        rectPosition.left = 0;
                        rectPosition.right = 360;
                        rectPosition.bottom = 1280;
                    }else{
                        rectPosition.top = 360;
                        rectPosition.left = 0;
                        rectPosition.right = 640;
                        rectPosition.bottom = 720;
                    }
                    */
                    break;
                case kSplit_RightBottom:
                    rectPosition.top = height/2;
                    rectPosition.left = width/2;
                    rectPosition.right = width;
                    rectPosition.bottom = height;

                    /*
                    if( vertical ) {
                        rectPosition.top = 640;
                        rectPosition.left = 360;
                        rectPosition.right = 720;
                        rectPosition.bottom = 1280;
                    }else{
                        rectPosition.top = 360;
                        rectPosition.left = 640;
                        rectPosition.right = 1280;
                        rectPosition.bottom = 720;
                    }
                    */
                    break;
            }
        }

        /**
         * This method sets the mask position in the overlay coordinates. 
         * Used when freely setting the mask position without using the {@link #setSplitMode(int, boolean)}.
         *
         * @param left The left part of the mask.
         * @param top The top part of the mask.
         * @param right The right part of the mask.
         * @param bottom The bottom part of the mask. 
         *
         * @since version 1.5.3
         */
        public void setPosition(int left, int top, int right , int bottom){
            splitMode = 0;
            rectPosition.bottom = bottom;
            rectPosition.top = top;
            rectPosition.left = left;
            rectPosition.right = right;
        }

        /**
         * This method sets the mask position in the overlay coordinates.
         * Used when freely setting the mask position without using the {@link #setSplitMode(int, boolean)}.
         *
         * @param rect The area to mask.
         *
         * @since version 1.5.3
         */
        public void setPosition(Rect rect){
            splitMode = 0;
            rectPosition.bottom = rect.bottom;
            rectPosition.top = rect.top;
            rectPosition.left = rect.left;
            rectPosition.right = rect.right;
        }

        /**
         * This method sets the mask position in the overlay coordinates.
         *
         * @param rect The area to mask.
         *
         * @since version 1.5.3
         */
        public void getPosition(Rect rect){
            rect.bottom = rectPosition.bottom;
            rect.top = rectPosition.top;
            rect.left = rectPosition.left;
            rect.right = rectPosition.right;
        }

        public void setAnimateSyncFromOverlayItem(boolean on) {
            syncAnimationOverlayItem = on;
        }

        public boolean getAnimateSyncFromOverlayItem() {
            return syncAnimationOverlayItem;
        }

    }

    private Mask mMask;

 
    /**
     * This method gets the mask value where an overlay item will be printed. 
     *
     * <p>Example code :</p>
     * {@code mProject.getOverlay(selectedPIPId).getMask().setState(false);}
     * @return The mask
     * @since version 1.5.0
     */
    public Mask getMask(){
        if( mMask == null ){
            mMask = new Mask();
        }
        return mMask;
    }

    /**
     * This class indicates the position and state of the overlay item in the coordinates.
     * @since version 1.5.3
     */
    public class BoundInfo{
        private float scaleX;
        private float scaleY;
        private float x;
        private float y;
        private float angleX;
        private float angleY;
        private float angleZ;

        private int width;
        private int height;
        private int time;
        private boolean maskOn;
        private Rect mask;
        private Rect drawPosition;

        @Override
        public String toString() {
            return "BoundInfo{" +
                    "scaleX=" + scaleX +
                    ", scaleY=" + scaleY +
                    ", x=" + x +
                    ", y=" + y +
                    ", angleX=" + angleX +
                    ", angleY=" + angleY +
                    ", angleZ=" + angleZ +
                    ", width=" + width +
                    ", height=" + height +
                    ", time=" + time +
                    ", maskOn=" + maskOn +
                    ", mask=" + mask +
                    ", drawPosition=" + drawPosition +
                    '}';
        }

        /**
         * This method gets the x-scale value of the overlay item as of now. 
         *
         * @return The x scale value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public float getScaleX(){
            return scaleX;
        }

        /**
         * This method gets the y-scale value of the overlay item as of now. 
         *
         * @return The y scale value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public float getScaleY(){
            return scaleY;
        }

        /**
         * This method gets the x-axis value of the overlay item as of now. 
         *
         * @return The x-axis value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public float getTranslateX(){
            return x;
        }

        /**
         * This method gets the y-axis value of the overlay item as of now.
         *
         * @return The y-axis value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public float getTranslateY(){
            return y;
        }

        /**
         * This method gets the rotation value of the overlay item as of now.
         *
         * @return The rotation value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public float getAngle(){
            return angleZ;
        }

        /**
         * This method gets the width value of the overlay item as of now.
         *
         * @return The width value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public int getWidth(){
            return width;
        }

        /**
         * This method gets the height value of the overlay item as of now.
         *
         * @return The height value of the overlay item at a set time.
         *
         * @since version 1.5.3
         */
        public int getHeight(){
            return height;
        }

        /**
         * This method gets the time value of the project timeline as of now.
         *
         * @return The set time.
         *
         * @since version 1.5.3
         */
        public int getTime(){
            return time;
        }

        public float getAngleX(){
            return angleX;
        }

        public float getAngleY(){
            return angleY;
        }

        public void getMaskBound(Rect bound) {
            bound.set(mask.left,mask.top,mask.right,mask.bottom);
        }

        public void getDrawBound(Rect bound){
            bound.set(drawPosition.left,drawPosition.top,drawPosition.right,drawPosition.bottom);
        }

        public boolean getMaskState() {
            return maskOn;
        }

        private BoundInfo(){

        }

        private BoundInfo(int t){
            mask = new Rect();
            time = t;
            scaleX  = mScaledX;          // >0
            scaleY  = mScaledY;
            int [] positions = getRealPositions(false);
            x      = positions[0];   // 0 ~ 1280
            y      = positions[1];    // 0 ~ 720

            angleX  = mRotateDegreeX;
            angleY  = mRotateDegreeY;
            angleZ  = mRotateDegreeZ;

            if( mOverLayImage != null ) {
                width = mOverLayImage.getWidth();
                height = mOverLayImage.getHeight();
                drawPosition = getDrawBitmapPosition(mOverLayImage.getAnchorPoint(),width,height);
            }else {
                width = mOverLayFilter.getWidth();
                height = mOverLayFilter.getHeight();
                drawPosition = new Rect(-width/2, -height/2, width/2 , height/2);
            }

            maskOn = false;
            if( mMask != null ){
                if( mMask.onOff ){
                    mMask.getPosition(mask);
                }
            }

            if (mAniList != null) {
                for (nexAnimate temp : mAniList) {
                    int aniTime = time - mStartTime;
                    if (temp.mStartTime <= aniTime && temp.getEndTime() > aniTime) {
                        if (temp instanceof nexAnimate.Move) {
                            if (mLayerExpression.getID() == 0) {
                                x += temp.getTranslatePosition(aniTime,nexAnimate.kCoordinateX);
                                y += temp.getTranslatePosition(aniTime,nexAnimate.kCoordinateY);
                            }
                        } else if (temp instanceof nexAnimate.Rotate) {
                            if (mLayerExpression.getID() == 0) {
                                angleX = temp.getAngleDegree(aniTime, mRotateDegreeX , nexAnimate.kCoordinateX);
                                angleY = temp.getAngleDegree(aniTime, mRotateDegreeY , nexAnimate.kCoordinateY);
                                angleZ = temp.getAngleDegree(aniTime, mRotateDegreeZ , nexAnimate.kCoordinateZ);
                            }
                        } else if (temp instanceof nexAnimate.Scale) {
                            if (mLayerExpression.getID() == 0) {
                                scaleX = temp.getScaledRatio(aniTime, mScaledX, nexAnimate.kCoordinateX);
                                scaleY = temp.getScaledRatio(aniTime, mScaledY, nexAnimate.kCoordinateY);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * This method gets the {@link com.nexstreaming.nexeditorsdk.nexOverlayItem.BoundInfo BoundInfo} from the timeline of this overlay item.
     * 
     * @param time The time of the overlay timeline.
     * 
     * @return The BoundInfo that includes the current coordinates.
     *
     * @since version 1.5.3
     */
    public BoundInfo getBoundInfo(int time){
        return new BoundInfo(time);
    }

    /**
     * This method gets the information of touched location on the screen.
     * The information includes X and Y coordinates, width and height of the view.
     *
     * <p>Example code :</p>
     *  {@code    m_editorView = (nexEngineView)findViewById(R.id.engineview_overlay_fastview);
            m_editorView.setOnTouchListener(new View.OnTouchListener() {
                nexOverlayItem.HitPoint mPosition = new nexOverlayItem.HitPoint();
                mPosition.mTime = 0;
                mPosition.mViewX = event.getX();
                mPosition.mViewY = event.getY();
                mPosition.mViewWidth = v.getWidth();
                mPosition.mViewHeight = v.getHeight();
                //
            }}
     *   
     * @since version 1.5.3
     */
    static public class HitPoint{
        /**
         * This method gets the selected time from the overlay timeline.
         * 
         * @warning The time inside the set duration of this overlay, not the project timeline.
         * 
         * @since version 1.5.3
         */
        public int mTime;

        /**
         * This method gets the view x value entered in the overlay coordinates.
         *
         * @since version 1.5.3
         */
        public float mViewX;

        /**
         * This method gets the view y value entered in the overlay coordinates.
         *
         * @since version 1.5.3
         */
        public float mViewY;

        /**
         * This method gets the view width value entered in the overlay coordinates.
         *
         * @since version 1.5.3
         */
        public float mViewWidth;

        /**
         * This method gets the view height value entered in the overlay coordinates.
         *
         * @since version 1.5.3
         */
        public float mViewHeight;

        private int id;
        private int position;

        /**
         * This method gets the ID of the selected overlay item. 
         *
         * @return Zero for when there is no selected overlay items, or a non-zero ID value for the selected overlay item.
         *
         * @since version 1.5.3
         */
        public int getID(){
            return id;
        }

        /**
         * This method gets the hit position of this overlay.
         * 
         * @return Zero for when the hit position is in the center area; otherwise, a non-zero value.
         *      * {@link #kOutLine_Pos_LeftTop}={@value #kOutLine_Pos_LeftTop}
         *            , {@link #kOutLine_Pos_RightTop}={@value #kOutLine_Pos_RightTop}
         *            , {@link #kOutLine_Pos_LeftBottom}={@value #kOutLine_Pos_LeftBottom}
         *            ,{@link #kOutLine_Pos_RightBottom}={@value #kOutLine_Pos_RightBottom}
         * @since version 1.5.3
         */
        public int getHitInPosition(){
            return position;
        }
    }

    /**
     * This API checks whether or not this overlay exists in the entered {@link com.nexstreaming.nexeditorsdk.nexOverlayItem.HitPoint HitPoint}.
     *
     * <p>Example code :</p>
     *  {@code public boolean onTouch(View v, MotionEvent event) {
        int act = event.getAction();
        if ((act&MotionEvent.ACTION_MASK) == MotionEvent.ACTION_DOWN) {
            mPosition.mTime = 0;
            mPosition.mViewX = event.getX();
            mPosition.mViewY = event.getY();
            mPosition.mViewWidth = v.getWidth();
            mPosition.mViewHeight = v.getHeight();
            item.isPointInOverlayItem(mPosition )
    }
    }
    }
     * @param PointInOut The values of HitPoint.mTime, HitPoint.mViewX, HitPoint.mViewY, HitPoint.mViewWidth, HitPoint.mViewHeight must be present for this class to function.
     *
     * @return <tt>TRUE</tt> if a point is hit; otherwise, <tt>FALSE</tt>.
     *
     * @since version 1.5.3
     */
    public boolean isPointInOverlayItem(HitPoint PointInOut ){
        float[] p = scratchPoint;
        Matrix m = scratchMatrix;

        BoundInfo info = getBoundInfo(PointInOut.mTime);

        m.reset();
        float w = nexApplicationConfig.getAspectProfile().getWidth();
        float h = nexApplicationConfig.getAspectProfile().getHeight();
        m.postScale(w / PointInOut.mViewWidth, h / PointInOut.mViewHeight);
        /*
        int mode = nexApplicationConfig.getAspectRatioMode();
        if( mode == nexApplicationConfig.kAspectRatio_Mode_16v9 ) {
            m.postScale(1280f / PointInOut.mViewWidth, 720f / PointInOut.mViewHeight);
        }else if( mode == nexApplicationConfig.kAspectRatio_Mode_9v16 ) {
            m.postScale(720f / PointInOut.mViewWidth, 1280f / PointInOut.mViewHeight);
        }else if( mode == nexApplicationConfig.kAspectRatio_Mode_1v1 ) {
            m.postScale(720f / PointInOut.mViewWidth, 720f / PointInOut.mViewHeight);
        }else if( mode == nexApplicationConfig.kAspectRatio_Mode_2v1 ) {
            m.postScale(1440f / PointInOut.mViewWidth, 720f / PointInOut.mViewHeight);
        }else if( mode == nexApplicationConfig.kAspectRatio_Mode_1v2 ) {
            m.postScale(720f / PointInOut.mViewWidth, 1440f / PointInOut.mViewHeight);
        }
        */
        m.postTranslate(-info.x, -info.y);
        m.postScale( 1f / info.scaleX, 1f/ info.scaleY);
        m.postRotate(-info.angleZ,0,0);
        p[0] = PointInOut.mViewX;
        p[1] = PointInOut.mViewY;
        m.mapPoints(p);
        float newX = p[0];
        float newY = p[1];
        float handleRadius = 64 * (1f/info.scaleX);//(int) (handleTouchZonePxSize * 1280 / PointInOut.mViewWidth / 2 * (1f/info.scaleX));
        Rect rc = new Rect();
        info.getDrawBound(rc);
        float spaceLeftPixel = 0;
        float spaceRightPixel = 0;
        float spaceTopPixel = 0;
        float spaceBottomPixel= 0;
        if( overlayOutLine != null ){
            spaceLeftPixel = overlayOutLine.spaceLeftPixel;
            spaceRightPixel = overlayOutLine.spaceRightPixel;
            spaceTopPixel = overlayOutLine.spaceTopPixel;
            spaceBottomPixel= overlayOutLine.spaceBottomPixel;
        }

        float left =  rc.left - spaceLeftPixel;
        float right = rc.right + spaceRightPixel;
        float top =   rc.top -  spaceTopPixel;
        float bottom = rc.bottom + spaceBottomPixel;

        Log.d(TAG,"new pos("+newX+","+newY+") , Rect("+left+","+top+","+right+","+bottom+") , handleRadius="+handleRadius);

        if( newX >= left- handleRadius && newX <= left+handleRadius && newY >= top-handleRadius && newY <= top+handleRadius ){
            PointInOut.position = kOutLine_Pos_LeftTop;
        }else if( newX >= right- handleRadius && newX <= right+handleRadius && newY >= top-handleRadius && newY <= top+handleRadius ){
            PointInOut.position = kOutLine_Pos_RightTop;
        }else if( newX >= left- handleRadius && newX <= left+handleRadius && newY >= bottom-handleRadius && newY <= bottom+handleRadius ){
            PointInOut.position = kOutLine_Pos_LeftBottom;
        }else if( newX >= right- handleRadius && newX <= right+handleRadius && newY >= bottom-handleRadius && newY <= bottom+handleRadius ){
            PointInOut.position = kOutLine_Pos_RightBottom;
        }else if( newX >= left && newX <=right && newY >= top && newY <= bottom ){
            PointInOut.position = 0;
        }else{
            PointInOut.id = 0;
            PointInOut.position = 0;
            return false;
        }
        PointInOut.id = this.getId();
        return true;
    }

    private static nexOverlayOutLine overlayOutLine ;
    private boolean showOutLien;

    public static nexOverlayOutLine getOutLineProperty(){
        setOutLine();
        return overlayOutLine;
    }


    /**
     * This method shows a dashed outline to indicate that this overlay item has been selected. 
     *
     * @param show Set to <tt>TRUE</tt> to show an outline, or set to <tt>FALSE</tt> to hide.
     *
     * @note {@link #setOutLine()} must be called first to show a dashed outline properly.
     *
     * @see #setOutLine()
     * @since version 1.5.3
     */
    public void showOutline(boolean show){
        showOutLien = show;
    }

    /**
     * This method sets the dashed outline of the selected overlay item.
     *
     * Use this method to initialize the application when making an application with the overlay edit mode.
     *
     * @see #setOutLineIcon(Context, int, int)
     * @see #setOutlineType(boolean)
     * @see #clearOutLine()
     * @since version 1.5.3
     */
    static public void setOutLine(){
        if( overlayOutLine == null) {
            overlayOutLine = nexOverlayOutLine.builder();
        }
        overlayOutLine.reset();
    }

    /**
     * This method sets the outline type to solid white. The default outline type is dashed black and white.
     *
     * @param solid Set to <tt>TRUE</tt> for solid white, or set to <tt>FALSE</tt> for dashed black and white.
     *
     * @since version 1.5.3
     */
    static public void setOutlineType(boolean solid){
        if( overlayOutLine != null) {
            overlayOutLine.setSolidOutline(solid);
        }
    }
    
    /**
     * This method clears the dashed outline resources for when the overlay edit is finished.
     *
     * @see #setOutLine()
     * @since version 1.5.3
     */
    static public void clearOutLine(){
        overlayOutLine = null;
    }

    /**
     * This method gets the top-left corner position of the dashed outline.
     *
     * @since version 1.5.3
     */
    static public final int kOutLine_Pos_LeftTop = 1;

    /**
     * This method gets the top-right corner position of the dashed outline.
     *
     * @since version 1.5.3
     */
    static public final int kOutLine_Pos_RightTop = 2;

    /**
     * This method gets the bottom-left corner position of the dashed outline.
     *
     * @since version 1.5.3
     */
    static public final int kOutLine_Pos_LeftBottom = 3;

    /**
     * This method gets the bottom-right corner position of the dashed outline.
     *
     * @since version 1.5.3
     */
    static public final int kOutLine_Pos_RightBottom = 4;

    /**
     * This method sets the image and position of the icon during the overlay edit.
     * The icon must be included in the resources.
     *
     * @param context The Android context where the icon image exists.
     * @param pos The position to where the icon will be set.
     * {@link #kOutLine_Pos_LeftTop}={@value #kOutLine_Pos_LeftTop}
     *            , {@link #kOutLine_Pos_RightTop}={@value #kOutLine_Pos_RightTop}
     *            , {@link #kOutLine_Pos_LeftBottom}={@value #kOutLine_Pos_LeftBottom}
     *            ,{@link #kOutLine_Pos_RightBottom}={@value #kOutLine_Pos_RightBottom}
     * @param resID The resource ID of the icon.
     *
     * @note In order to show the icon properly, call {@link #setOutLine()} first.
     *
     * @return <tt>TRUE</tt> if the icon setting was successful; otherwise, <tt>FALSE</tt>.
     * @since version 1.5.3
     */
    static public boolean setOutLineIcon(Context context, int pos , int resID ){
        if( overlayOutLine != null) {
            overlayOutLine.setOutLineIcon(context,pos,resID);
        }
        return true;
    }
//    protected int mShowRangemode; //0 - no , 1-show , 2-clear;
    private int mZOrder = 0;
    public int getZOrder(){
        return mZOrder;
    }

    public void setZOrder(int order){
        if( mZOrder != order )
            mUpdated = true;
        mZOrder= order;

    }

    private transient AwakeAsset awakeAsset;
    void onRenderAwake( LayerRenderer renderer ){
        if( mOverLayFilter != null ){
            try {
                OverlayAsset assetRender = mOverLayFilter.getOverlayAssetFilter();
                Rect bounds = new Rect();
                mOverLayFilter.getBound(bounds);
                awakeAsset = assetRender.onAwake(renderer,new RectF(bounds),mOverLayFilter.getEncodedEffectOptions(),null);

            } catch (IOException e) {
                e.printStackTrace();
            } catch (XmlPullParserException e) {
                e.printStackTrace();
            }
            return;
        }
        if( mOverLayImage != null ) {
            if (mOverLayImage.isAssetManager()) {
                try {
                    OverlayAsset assetRender = mOverLayImage.getOverlayAssetBitmap();
                    Rect bounds = new Rect();
                    mOverLayImage.getBound(bounds);
                    awakeAsset = assetRender.onAwake(renderer, new RectF(bounds), null, null);
                } catch (IOException e) {
                    e.printStackTrace();
                } catch (XmlPullParserException e) {
                    e.printStackTrace();
                }
            }
        }

    }

    void onRenderAsleep( LayerRenderer renderer ){

        renderOverlay(renderer, KineMasterSingleTon.getApplicationInstance().getApplicationContext());

        if( awakeAsset != null ){
            awakeAsset.onAsleep(renderer);
            awakeAsset = null;
        }

    }

    void onRender( LayerRenderer renderer ){
        if( mOverLayFilter != null ){
            if( mOverLayFilter.isUpdated() ) {
                if (awakeAsset != null) {

                    Rect bounds = new Rect();
                    mOverLayFilter.getBound(bounds);

                    awakeAsset.onRefresh(renderer, new RectF(bounds), mOverLayFilter.getEncodedEffectOptions());
                }
            }
        }
        renderOverlay(renderer, KineMasterSingleTon.getApplicationInstance().getApplicationContext());
    }

    boolean updated(boolean set){
        boolean ret = mUpdated;
        mUpdated = set;
        return ret ;
    }

    private int mFlipMode = 0;

    /**
     *
     * @param on
     * @since 2.0.0
     */
    public void flipVertical(boolean on){
        if( on ) {
            mFlipMode |= 1;
        }else{
            mFlipMode &= ~1;
        }
    }

    /**
     *
     * @param on
     * @since 2.0.0
     */
    public void flipHorizontal(boolean on){
        if( on ) {
            mFlipMode |= 2;
        }else{
            mFlipMode &= ~2;
        }
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public boolean isFlipVertical(){
        return ((mFlipMode & 1) == 1);
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public boolean isFlipHorizontal(){
        return ((mFlipMode & 2) == 2);
    }

    /**
     *
     * @param show
     * @since 2.0.0
     */
    public void setEnableShow(boolean show){
        showItem = show;
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public boolean getEnableShow(){
        return showItem;
    }

    /**
     *
     * @return
     * @throws NoSuchObjectException
     */
    public nexOverlayFilter getOverLayFilter() throws NoSuchObjectException {
        if( mOverLayFilter == null ){
            throw new NoSuchObjectException("nexOverlayFilter");
        }
        return mOverLayFilter;
    }
}
