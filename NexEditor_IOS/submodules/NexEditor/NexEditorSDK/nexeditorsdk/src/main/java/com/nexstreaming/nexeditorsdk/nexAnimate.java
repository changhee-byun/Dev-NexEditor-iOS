/******************************************************************************
 * File Name        : nexAnimate.java
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

import android.animation.TimeInterpolator;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;
import com.nexstreaming.nexeditorsdk.exception.NotSupportedAPILevel;


/** 
 * This class is used to set an animation effect of an overlay item. 
 *
 * This is a list of the animation effects available:
 * <ul>
 * <li> Image animation : Prints sequence of images fast enough to make it look like the image is moving. 
 * <li> Move animation : Moves an overlay item to a desired location on the screen. 
 * <li> Rotate animation : Rotates an overlay item.
 * <li> Alpha animation : Sets the transparency of an overlay item.
 * <li> Scale animation : Sets the size of an overlay item.
 * </ul>
 * The animation uses the linear functional equation and a time range of 0 to 1 to create an animation effect. 
 * The x-coordinate and y-coordinate indicate the center of the overlay item being animated. 
 * @see com.nexstreaming.nexeditorsdk.nexOverlayItem
 * @since version 1.1.0
 * <table border=1><tr><td><tt>nexEditorSDK API Limited Level</tt></td><td>{@value EditorGlobal#kOverlayAnimateLimited}</td></tr></table>
 */
public abstract class nexAnimate {
    protected int mStartTime;
    protected int mDuration;
    private TimeInterpolator mTimeInterpolator;
    /** 
     * The value of the parameter <tt>coordinate</tt> of the interface <tt>MoveTrackingPath</tt>.
     * This indicates the x-coordinate of the center of an overlay item.
     *
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.1.0
     */
    public static final int kCoordinateX = 1;

    /**
     * The value of the parameter <tt>coordinate</tt> of the interface <tt>MoveTrackingPath</tt>.
     * This indicates the y-coordinate.
     *
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.1.0
     */
    public static final int kCoordinateY = 2;

    /**
     * The value of the parameter <tt>coordinate</tt> of the interface <tt>MoveTrackingPath</tt>.
     * This indicates the z-coordinate.
     *
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.43
     */
    public static final int kCoordinateZ = 3;

//    public static final int kCoordinateSpeed = 4;

    /** 
     *  This interface is used as a parameter of move animation and rotate animation. 
     * As for move animation, this will be the parameter <tt>path</tt>, the location value for the method getMove().
     * As for rotate animation, this will be the parameter <tt>center</tt>, the pivot value for the method getRotate().
     *
     * @see #getMove(int, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getRotate(int, int, boolean, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @since version 1.1.0
     */
    public interface MoveTrackingPath{
        /**
         *
         * @param coordinate
         * <ul>
         * <li>{@link #kCoordinateX} = {@value #kCoordinateX} : X-coordinate
         * <li>{@link #kCoordinateY} = {@value #kCoordinateY} : Y-coordinate
         * </ul>
         * @param timeRatio In range of 0 to 1, where 0 is start time and 1 is end time of the animation effect. 
         * @return The coordinate values returned, depending on the value of the parameter <tt>coordinate</tt>.
         */
        float getTranslatePosition(int coordinate, float timeRatio);
    }

    /**
     *  This method sets the start time and the duration of an animation that's set to an overlay item. 
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
        ani.setTime(0, 2000);}
     *
     * @param startTime The start time of the animation in <tt> milliseconds</tt>. 
     * @param duration The duration of the animation in <tt> milliseconds</tt>. 
     *
     * @throws com.nexstreaming.nexeditorsdk.exception.InvalidRangeException
     * @since version 1.1.0
     */
    public void setTime(int startTime, int duration ){
        if( duration <= 0 ){
            throw new InvalidRangeException( duration);
        }

        mStartTime = startTime;
        mDuration = duration;
    }

    /**
     * In <tt>nexAnimate</tt>, the time interpolator increments by the ratio of 1:1 in the type of the first order linear equation.W
     *
     * For a new time increment in a different type of interpolator other than linear, set a new interpolator using this method.
     *
     * <p>Example code :</p>
     * {@code nexAnimate sc = nexAnimate.getAlpha(0, 1000, 0, 1).setInterpolator(new AccelerateDecelerateInterpolator());
    overlayItem.addAnimate(sc);}
     * @param Interpolator The Time Interpolator provided by the Android SDK API. 
     * @return this. The set instance. Setting is available after calling the setInterpolator() that has been created as shown in the example above. 
     * @since version 1.5.23
     */
    public nexAnimate setInterpolator(TimeInterpolator Interpolator){
        mTimeInterpolator = Interpolator;
        return this;
    }

    protected nexAnimate(int startTime, int duration){
        if( duration <= 0 ){
            throw new InvalidRangeException( duration);
        }

        mStartTime = startTime;
        mDuration = duration;
    }

    protected float timeRatio(int time ){
        float f = ((float)(time - mStartTime)/(float)mDuration);
        Log.d("timeRatio", "timeRatio ="+f +",mTime = "+time);
        if( f > 1.0f){
            f = 1f;
        }

        if( f < 0 ){
            f = 0;
        }
        if( mTimeInterpolator != null ){
            return mTimeInterpolator.getInterpolation(f);
        }

        return f;
    }

    protected int getEndTime(){
        return mStartTime + mDuration;
    }

    int getImageResourceId(int currTime){
        return 0;
    }

    protected float getTranslatePosition( int currTime, int axis){
        if( axis == kCoordinateX  ){
            return mdX;
        }else if( axis == kCoordinateY ){
            return mdY;
        }else if( axis == kCoordinateZ ){
            return mdZ;
        }
        return 0;
    }

    protected float getAlpha(int time){
        return mAlpha;
    }

    protected float getAngleDegree(int time, float startDegree , int axis){
        if( axis == kCoordinateX  ){
            return mRotateDegreeX;
        }else if( axis == kCoordinateY ){
            return mRotateDegreeY;
        }else if( axis == kCoordinateZ ){
            return mRotateDegreeZ;
        }
        return 0;
    }

    protected float getScaledRatio(int time, float startScaledRatio , int axis ){
        if( axis == kCoordinateX  ){
            return mScaledX;
        }else if( axis == kCoordinateY ){
            return mScaledY;
        }else if( axis == kCoordinateZ ){
            return mScaledZ;
        }
        return 1;
    }

    protected int mdX = 0;
    protected int mdY = 0;
    protected int mdZ = 0;
    protected float mAlpha = 1;
    protected float mScaledX = 1;
    protected float mScaledY = 1;
    protected float mScaledZ = 1;
    protected float mRotateDegreeX = 0;
    protected float mRotateDegreeY = 0;
    protected float mRotateDegreeZ = 0;

    /**
    * This method initiates the variables that are related to animation movements. 
    *
    * @since version 1.1.0
    */
    public void resetFreeTypeAnimate(){
        mdX = 0;
        mdY = 0;
        mdZ = 0;
        mAlpha = 1;
        mScaledX = 1;
        mScaledY = 1;
        mScaledZ = 1;
        mRotateDegreeX = 0;
        mRotateDegreeY = 0;
        mRotateDegreeZ = 0;
    }

    protected boolean onFreeTypeAnimate(int time, nexOverlayItem overlay  ){
        return false;
    }
    /**
     *  This method creates an image animation with the parameter values of this method.  
     *
     * @param startTime The start time of the animation in <tt> milliseconds</tt>.
     * @param duration The duration of the animation in <tt> milliseconds</tt>. 
     * @param ids The android resource ID of the image animation. 
     *
     * @return A newly created image animation. 
     *
     * <p>Example code :</p>
     * {@code       nexAnimate ani = nexAnimate.getAnimateImages(
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
         overlayItem.addAnimate(ani);}
     * @see #getRotate(int, int, boolean, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getMove(int, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getAlpha(int, int, float, float)
     * @see #getScale(int, int, float, float)
     * @throws NotSupportedAPILevel nexEditorSDK API Limited Level is {@value EditorGlobal#kOverlayAnimateLimited}
     * @since version 1.1.0
     *
     */
    public static nexAnimate getAnimateImages(int startTime, int duration, int... ids){
        if( EditorGlobal.ApiLevel < EditorGlobal.kOverlayAnimateLimited ) {
            return new AnimateImages(startTime, duration, ids);
        }else{
            throw new NotSupportedAPILevel();
        }
    }

    protected static class AnimateImages extends nexAnimate {
        private final int[] resourceIds;

        protected AnimateImages(int startTime, int duration, int... ids){
            super( startTime,duration);
            resourceIds = ids;
        }

        @Override
        protected int getImageResourceId(int currTime) {
            int index = ((currTime - mStartTime )/33)%resourceIds.length;
            if( index < 0 )
                index = 0;
            return resourceIds[index];
        }
    }
 
    /** 
     *  This method creates a move animation with the parameter values of this method. 
     * The overlay item will be moved to a location of coordinate values returned from the parameter <tt>path</tt>. 
     * Range of 0 to 1, where 0 is start time and 1 is end time of the animation effect, the overlay image moves gradually over time. 
     *
     * @param startTime The start time of the animation in <tt> milliseconds</tt>.
     * @param duration The duration of the animation in <tt> milliseconds</tt>. 
     * @param path <tt>MoveTrackingPath</tt> to get the x-coordinate and y-coordinate value with given playtime of the animation. 
     *
     * @return A newly created move animation. 
     * <p>Example code :</p>
     *  {@code        nexAnimate mv = nexAnimate.getMove(0,5000,new nexAnimate.MoveTrackingPath(){
                public float getTranslatePosition(int coordinate, float timeRatio) {
                    if( coordinate == nexAnimate.kCoordinateX ){
                        return 700*timeRatio;
                    } else if( coordinate == nexAnimate.kCoordinateY ) {
                        return 400*timeRatio;
                    }
                    return 0;
                }
            });
            overlayItem.addAnimate(mv);}
     * @see #getRotate(int, int, boolean, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getAlpha(int, int, float, float)
     * @see #getAnimateImages(int, int, int...)
     * @see #getScale(int, int, float, float)
     *
     * @since version 1.1.0
     */
    public static nexAnimate getMove(int startTime, int duration, MoveTrackingPath path ){
        return new Move(startTime, duration, path);
    }

    /**
     *  This method create an alpha animation with the parameter values of this method. 
     * This method starts with the alpha value set to parameter <tt> start</tt>, and end with the alpha value set to parameter <tt> end</tt>.
     * Range of 0 to 1, where 0 is start time and 1 is end time of the animation effect, the alpha value changes gradually over time. 
     *
     * @param startTime The start time of the animation in <tt>milliseconds</tt>. 
     * @param duration The duration of the animation in <tt>milliseconds.</tt>  
     * @param start The alpha value of start time of the animation. 
     * @param end The alpha value of end time of the animation.
     *
     * @return A newly created alpha animation. 
     * <p>Example code :</p>
     *  {@code nexAnimate ap = nexAnimate.getAlpha(4000,2000,1,0);
            overlayItem.addAnimate(ap);
        }
     * @see #getRotate(int, int, boolean, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getMove(int, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getAnimateImages(int, int, int...)
     * @see #getScale(int, int, float, float)
     *
     * @since version 1.1.0
     */
    public static nexAnimate getAlpha(int startTime, int duration, float start, float end){
        return new Alpha(startTime, duration, start, end);
    }

    protected static class Move extends nexAnimate {
        private MoveTrackingPath mPath;

        protected Move(int startTime, int duration , MoveTrackingPath path){
            super( startTime,duration);
            mPath = path;
            if( mPath == null ){
                mPath = new MoveTrackingPath(){
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        return 0;
                    }
                };
            }
        }

        @Override
        protected float getTranslatePosition( int currTime, int axis){
            return mPath.getTranslatePosition(axis, timeRatio(currTime));
        }

    }

    protected static class Alpha extends nexAnimate {
        private final float mStart;
        private final float mEnd;
        protected Alpha(int startTime, int duration, float start, float end ){
            super( startTime,duration);
            mStart = start;
            mEnd = end;
        }

        @Override
        protected float getAlpha(int time) {
            float a = (mEnd-mStart)*timeRatio(time)+ mStart;
            if( a > 1f ){
                a =1;
            }else if( a < 0 ){
                a = 0;
            }
            return a;
        }
    }

    /**
     * This method creates a rotating animation.
     *  
     * The rotating animation created with this method will pivot on the coordinates that are set for the parameter <tt>center</tt>.
     * Over the range of 0 to 1, where 0 is the start and 1 is the end of the animation effect, the overlay image rotates gradually over time. 
     * 
     * @param startTime  The start time of the animation in <tt>msec</tt> (milliseconds).
     * @param duration  The duration of the animation in <tt>msec</tt> (milliseconds). 
     * @param clockwise  The rotation direction.  Set to <tt>TRUE</tt> for clockwise or <tt>FALSE</tt> for counterclockwise. 
     * @param rotateDegree  The number of rotation times, in degrees. (ex: To make one rotation = 360, and to make three rotations = 1080)
     * @param center  A <tt>MoveTrackingPath</tt> to provide the x- and y-coordinate values for the given playtime of the animation. 
     * 
     * @return The newly created rotating animation. 
     * <p>Example code :</p>
     * {@code     nexAnimate ro = nexAnimate.getRotate(2000,3000,true,360,null);
       overlayItem.addAnimate(ro);}
     * @see #getMove(int, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getAlpha(int, int, float, float)
     * @see #getAnimateImages(int, int, int...)
     * @see #getScale(int, int, float, float)
     *
     * @since version 1.1.0
     */
    public static nexAnimate getRotate(int startTime, int duration, boolean clockwise, float rotateDegree, MoveTrackingPath center){
        return new Rotate(startTime, duration, clockwise, rotateDegree, kCoordinateZ, center);
    }

    protected static class Rotate extends nexAnimate {
        private final boolean mClockWise;
        private final float mXAxisRotateDegree;
        private final float mYAxisRotateDegree;
        private final float mZAxisRotateDegree;
        private MoveTrackingPath mCenter;

        protected Rotate(int startTime, int duration, boolean clockwise, float rotateDegree, int axis ,MoveTrackingPath center ){
            super( startTime,duration);
            mClockWise = clockwise;

            if( axis == kCoordinateX ){
                mXAxisRotateDegree = rotateDegree;
                mYAxisRotateDegree = 0;
                mZAxisRotateDegree = 0;
            }else if( axis == kCoordinateY ){
                mXAxisRotateDegree = 0;
                mYAxisRotateDegree = rotateDegree;
                mZAxisRotateDegree = 0;
            }else{
                mXAxisRotateDegree = 0;
                mYAxisRotateDegree = 0;
                mZAxisRotateDegree = rotateDegree;
            }

            mCenter = center;
            if( mCenter == null ){
                mCenter = new MoveTrackingPath(){
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                     return 0;
                    }
                };
            }
        }

        @Override
        protected float getAngleDegree(int time, float startDegree , int axis) {
            float angle = 0;
            if( axis == kCoordinateZ ) {
                angle = (float) mZAxisRotateDegree * timeRatio(time);
            }else if( axis == kCoordinateX ){
                angle = (float) mXAxisRotateDegree * timeRatio(time);
            }else if( axis == kCoordinateY ){
                angle = (float) mYAxisRotateDegree * timeRatio(time);
            }

            if(!mClockWise){
                angle *= -1;
            }
            return ((startDegree+angle) % 360);
        }

        @Override
        protected float getTranslatePosition( int currTime, int axis) {
            return mCenter.getTranslatePosition(axis, timeRatio(currTime));
        }

    }

    /** 
     * This method creates a scaling animation. 
     * 
     * A scaling animation changes the original size of an item by a scaling value set to both parameters <tt>lastScaledX</tt> and <tt>lastScaledY</tt>. 
     * Over the range of 0 to 1, where 0 is start time and 1 is end time of the animation effect, the overlay image changes its size gradually over time.  
     *
     * @param startTime The start time of the animation effect in <tt>msec</tt> (milliseconds). 
     * @param duration The duration of the animation effect in <tt>msec</tt> (milliseconds). 
     * @param lastScaledX The scaled width of the overlay item at the end of the animation.  
     * @param lastScaledY The scaled height of the overlay item at the end of the animation. 
     *
     * @return A newly created scaled animation. 
     * <p>Example code :</p>
     * {@code     nexAnimate sc = nexAnimate.getScale(0,3000,2,2);
       overlayItem.addAnimate(sc);}
     * @see #getRotate(int, int, boolean, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getMove(int, int, com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath)
     * @see #getAlpha(int, int, float, float)
     * @see #getAnimateImages(int, int, int...)
     * @see #getScale(int, int, float, float, float, float)
     * @see #getScale(int, int, MoveTrackingPath)
     * @since version 1.1.0
     */
    public static nexAnimate getScale(int startTime, int duration, float lastScaledX, float lastScaledY){
        return new Scale(startTime, duration, lastScaledX, lastScaledY, 1);
    }

    /**
     * This method ignores the set initial scaled values in <tt>OverlayImage</tt> and uses new initial scale values to create a scaled animation. 
     *
     * <p>Example code :</p>
     * {@code     nexAnimate sc = nexAnimate.getScale(0, expressDuration, 0, 0, 1, 1);
        overlayItem.addAnimate(sc);}

     * @param startTime The start time of the animation effect in <tt>msec</tt> (milliseconds).
     * @param duration The duration of the animation effect in <tt>msec</tt> (milliseconds).
     * @param startScaledX The scaled width of the overlay item at the start of the animation.
     * @param startScaledY The scaled height of the overlay item at the start of the animation.
     * @param lastScaledX The scaled width of the overlay item at the end of the animation.
     * @param lastScaledY The scaled height of the overlay item at the end of the animation.
     * @return A newly created scaled animation.
     * @see #getScale(int, int, float, float)
     * @see #getScale(int, int, MoveTrackingPath)
     * @since version 1.5.23
     */
    public static nexAnimate getScale(int startTime, int duration,float startScaledX, float startScaledY, float lastScaledX, float lastScaledY){
        return new Scale(startTime, duration, startScaledX, startScaledY ,1,lastScaledX, lastScaledY, 1);
    }

    /**
     * This method is used to create a different scaled animation for each start time.
     *
     * <p>Example code :</p>
     * {@code item.addAnimate(nexAnimate.getScale(0, duration, new nexAnimate.MoveTrackingPath() {
        int time = 0;
        @Override
        public float getTranslatePosition(int coordinate, float timeRatio) {
            if (coordinate == nexAnimate.kCoordinateX){
                return (float) (Math.sin((float)time/400f)*0.2f );
            }else if( coordinate == nexAnimate.kCoordinateY ){
                return (float) (Math.cos((float)time/400f)*0.2f );
            }else{
                time += 33;
            }
            return 1;
        }
     })}
     * @param startTime The start time of the animation effect in <tt>msec</tt> (milliseconds).
     * @param duration The duration of the animation effect in <tt>msec</tt> (milliseconds).
     * @param scale The scale values, which depends on the coodinate parameters that are passed in using the <tt>MoveTrackingPath</tt> interface.
     * @return A newly created scaled animation.
     * @see #getScale(int, int, float, float, float, float)
     * @see #getScale(int, int, float, float)
     * @since version 1.5.23
     */
    public static nexAnimate getScale(int startTime, int duration, MoveTrackingPath scale){
        return new Scale(startTime, duration, scale);
    }

    protected static class Scale extends nexAnimate {
        protected final float mLastScaledX;
        protected final float mLastScaledY;
        protected final float mLastScaledZ;
        protected final float mStartScaledX;
        protected final float mStartScaledY;
        protected final float mStartScaledZ;
        protected final boolean mSetStart;
        private MoveTrackingPath mPath;

        protected Scale(int startTime, int duration, float lastScaledX, float lastScaledY, float lastScaledZ ){
            super( startTime,duration);
            mStartScaledX = 0;
            mStartScaledY = 0;
            mStartScaledZ = 0;
            mLastScaledX = lastScaledX;
            mLastScaledY = lastScaledY;
            mLastScaledZ = lastScaledZ;
            mSetStart = false;
        }

        protected Scale(int  startTime, int duration, float startScaledX, float startScaledY, float startScaledZ, float lastScaledX, float lastScaledY, float lastScaledZ){
            super( startTime,duration);
            mStartScaledX = startScaledX;
            mStartScaledY = startScaledY;
            mStartScaledZ = startScaledZ;

            mLastScaledX = lastScaledX;
            mLastScaledY = lastScaledY;
            mLastScaledZ = lastScaledZ;
            mSetStart = true;
        }

        protected Scale(int  startTime, int duration, MoveTrackingPath scale){
            super( startTime,duration);
            mStartScaledX = 0;
            mStartScaledY = 0;
            mStartScaledZ = 0;

            mLastScaledX = 0;
            mLastScaledY = 0;
            mLastScaledZ = 0;

            mPath = scale;
            mSetStart = false;
        }

        @Override
        protected float getScaledRatio(int time, float startScaledRatio, int axis) {

            if( mPath != null ){
                return startScaledRatio + mPath.getTranslatePosition(axis,timeRatio(time));
            }


            float start = startScaledRatio;
            float last = 0;

            if (axis == kCoordinateX) {
                last = mLastScaledX;

                if (mSetStart) {
                    start = mStartScaledX;
                }
            }else if( axis == kCoordinateY ){
                last = mLastScaledY;
                if( mSetStart ){
                    start = mStartScaledY;
                }
            }else if( axis == kCoordinateZ ){
                last = mLastScaledZ;
                if( mSetStart ){
                    start = mStartScaledZ;
                }
            }

            if (last > start) {
                return start + (last - start) * timeRatio(time);
            } else {
                return start - (start - last) * timeRatio(time);
            }

        }
    }
}

