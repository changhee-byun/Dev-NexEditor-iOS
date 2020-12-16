/******************************************************************************
 * File Name        : nexVideoClipEdit.java
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

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;

/**
 * This class handles playback speed and trim options of a video clip in the NexEditor&trade;&nbsp;.
 * <p>Example code 1:</p>
 * {@code mEngine.getProject().getClip(0,true).getVideoClipEdit().setSpeedControl(200);
    mEngine.play();}
 * <p>Example code 2:</p>
 * {@code if( project.getTotalClipCount(true) == 1 ){
        nexClip clip = project.getClip(0,true);
        if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            if(clip.getTotalTime() > 12000 ) {
                clip.getVideoClipEdit().setAutoTrim(nexVideoClipEdit.kAutoTrim_Divided, 3);
            }
        }
    }}
 * @since version 1.0.0
 */
public final class nexVideoClipEdit implements Cloneable {
    //private int mTotalTime;
    private nexClip mClip;

    int mTrimStartDuration ;
    int mTrimEndDuration;

    private int mMasterSpeedControl = 100;
    private int mKeepPitch = 1;
    int mFreezeDuration = 0;
    boolean mUpdated;

    protected static nexVideoClipEdit clone(nexVideoClipEdit src) {
        //Log.d("clone", "clone work./nexVideoClipEdit");
        nexVideoClipEdit object = null;
        try {
            object = (nexVideoClipEdit)src.clone();
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    /**
     * One of the possible values of the first parameter, trimMode, of the method {@link #setAutoTrim(int, int)}.
     * If trimMode is set to this value, the clip will be divided equally into the number of videos set by the second parameter, value.
     * For example, if value = 5, the original video clip will be equally divided into 5 shorter clips.
     * @since version 1.0.0
     * @see #setAutoTrim(int, int)
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    static public int kAutoTrim_Divided = 1;

    /**
     * One of the possible values of the first parameter, trimMode, of the method {@link #setAutoTrim(int, int)}.
     * If trimMode is set to this value, the second parameter, value, is the interval time in <tt>milliseconds</tt> used to save thumbnail images 
     * from the original clip at determined intervals.
     * 
     *  @since version 1.0.0
     *  @see #setAutoTrim(int, int)
     *  @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    static public int kAutoTrim_Interval = 2;

    private nexVideoClipEdit(){}

    private nexVideoClipEdit(nexClip clip){
        mClip = clip;
    }

    static nexVideoClipEdit getnexVideoClipEdit(nexClip clip){
        if( clip.getClipType() != nexClip.kCLIP_TYPE_VIDEO ){
            return null;
        }
        return new nexVideoClipEdit(clip);
    }

    /**
     * This method clears previous trim history and starts a new trim.
     *
     * <p>Example code :</p>
     * {@code mProject.getClip(0, true).getVideoClipEdit().setTrim(0, trim);}
     * @param startTime  The start time of the video clip in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of the video clip in <tt>msec</tt> (milliseconds).
     *
     * @see #addTrim(int, int, int)
     * @throws com.nexstreaming.nexeditorsdk.exception.InvalidRangeException
     * @since version 1.0.0
     */
    public void setTrim(int startTime , int endTime){

        if( endTime <=  startTime ){
            throw new InvalidRangeException(startTime, endTime);
        }

// rooney 2015. 08. 16 for temporary release, blank code
//        if( (endTime - startTime) < 500 ){
//            throw new InvalidRangeException(500,(endTime - startTime),true);
//        }

        mTrimStartDuration = startTime;
        mTrimEndDuration = mClip.getTotalTime() - endTime;

        if(mTrimEndDuration < 0 || mTrimStartDuration < 0 ){
            throw new InvalidRangeException(mTrimStartDuration, mTrimEndDuration);
        }
        
        mUpdated = true;
        mClip.setProjectUpdateSignal(false);
    }

    /**
     * This method gets the start time of the trimmed section of a clip.
     *
     * A clip can be trimmed into a desired section by adjusting both ends (beginning and ending) of the clip.
     * The start time of this method is based on the total play time of the clip.
     *
     * <p>Example code :</p>
     * {@code trim = mPrimaryItems.get(postion).getVideoClipEdit().getStartTrimTime();}
     * @return The start time of the trimmed section, in <tt>msec</tt> (milliseconds), as an <tt>integer</tt>.
     *
     * @see #getEndTrimTime()
     * @since version 1.1.0
     *
     */
    public int getStartTrimTime(){
        return mTrimStartDuration;
    }

    /**
     * This method gets the end time of the trimmed section of a clip. 
     *
     * A clip can be trimmed into a desired section by adjusting both ends (beginning and ending) of the clip.
     * The end time of this method is based on the total play time of the clip. 
     *
     * <p>Example code :</p>
     * {@code trim = mPrimaryItems.get(postion).getVideoClipEdit().getEndTrimTime();}
     * @return The end time of the trimmed section, in <tt>msec</tt> (milliseconds), as an <tt>integer</tt>.     
     *
     * @see #getStartTrimTime()
     * @since version 1.1.0
     * 
     */
    public int getEndTrimTime(){
        return mClip.getTotalTime() - mTrimEndDuration;
    }

    /**
     * @deprecated
     * This method can be used to trim several parts of a video clip in different ways.
     * 
     * Trimmed video clips can be set to different playback speeds, which ignore the overall video playback speed set by {@link #setSpeedControl(int)}.
     *
     * @param startTime The start time of a video clip in <tt>msec</tt> (milliseconds).
     * @param endTime The end time of a video clip in <tt>msec</tt> (milliseconds).
     * @param speed The playback speed of the trimmed clip as a percentage of the original speed (from 25 (1/4 the original speed) up to 200 (twice the original speed)).
     * 
     * @see #removeTrim(int)
     * @since version 1.0.0
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public void addTrim(int startTime , int endTime, int speed){
    }

    /**
     * @deprecated
     * This method removes a trim set on a video clip.
     * @param loc The location of the trim to be removed.
     * @since version 1.0.0
     * @see #addTrim(int, int, int)
     * @see #setTrim(int, int)
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int removeTrim(int loc){
        return -1;
    }

    /**
     * @deprecated
     * This method gets the number of times trim has been set with the method addTrim() within a specific clip.
     * @return The number of trims set for the specific clip.
     * @since version 1.0.0
     * @see #addTrim(int, int, int)
     * @see #removeTrim(int)
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int getTrimCount(){
        return 0;
    }

    /**
     * The possible minimum speed, as a percentage or the original speed, to set as the parameter <tt>speed</tt> of the method <tt>setSpeedControl()</tt>. 
     * 
     * @since version 1.3.0
     * @see #setSpeedControl(int)
     */
    public static final int kSpeedControl_MinValue = 3;

    /**
     * The possible maximum speed, as a percentage or the original speed, to set as the parameter <tt>speed</tt> of the method <tt>setSpeedControl()</tt>. 
     * 
     * @since version 1.3.0
     * @see #setSpeedControl(int)
     */
    public static final int kSpeedControl_MaxValue = 1600;

    /**
     * This method sets the playback speed of a video clip.
     * The value set by this method will be ignored if the video clip was trimmed at any time.
     *
     * <p>Example code :</p>
     * {@code clip.getVideoClipEdit().setSpeedControl(200);}
     * @param speed The new playback speed of the video clip as a percentage of the original speed.  
     *              ( {@value #kSpeedControl_MinValue}% (slow) ~ 100%(standard) ~ {@value #kSpeedControl_MaxValue}%(fast)
     * 
     * @see #getSpeedControl()
     * @since version 1.0.0
     * 
     */
    public void setSpeedControl(int speed){
/*
        MediaInfo info = mClip.getMediaInfo();
        if(info == null){
            Log.d("nexVideoClipEdit","setSpeedControl getInfo fail!");
            return;
        }
        */
//        Log.d("nexVideoClipEdit","setSpeedControl info.getAudioSamplingRate("+info.getAudioSamplingRate()+")!");
//        if(info.getAudioSamplingRate() > 48000){
//            throw new InvalidRangeException(0, 48000, info.getAudioSamplingRate());
//        }
        if ( mClip.getAudioOnOff() == true ) {
            speed = speedControlTab(speed);
        }
        
        /*
        if( kSpeedControl_MinValue > speed ){
            speed = kSpeedControl_MinValue;
        }else if( kSpeedControl_MaxValue < speed  ){
            speed = kSpeedControl_MaxValue;
        }
        */
        if(mMasterSpeedControl != speed){
            mUpdated = true;
            mClip.setProjectUpdateSignal(false);
            mMasterSpeedControl = speed;
        }
    }

    /**
     * This method gets the playback speed set for a video clip.
     *
     * <p>Example code :</p>
     * {@code int speedcontrol = mVideoEdit.getSpeedControl();}
     * @return The playback speed of the video clip as a percentage of the original speed (100). 
     * 
     * @see #setSpeedControl(int)
     * @since version 1.0.0
     * 
     */
    public int getSpeedControl(){
        return mMasterSpeedControl;
    }

    public void setKeepPitch(int iKeepPitch){
        mKeepPitch = iKeepPitch;
    }

    public int getKeepPitch(){
        return mKeepPitch;
    }

    /**
     * @deprecated
     * This method trims a video clip automatically, based on the mode chosen with the parameter, <tt>trimMode</tt>.
     * @param trimMode  How to automatically trim video clips.  This should be one of:
     * <ul>
     *     <li>{@link #kAutoTrim_Divided} = {@value #kAutoTrim_Divided}
     *     <li>{@link #kAutoTrim_Interval} = {@value #kAutoTrim_Interval}
     * </ul>
     * @param value Additional information on how the clip should be trimmed based on the value of <tt>trimMode</tt>.
     * <ul>
     *      <li>When trimMode = <tt>kAutoTrim_Divided</tt>, this value is the number of equally sized clips that should be created (Min 2 ~ Max 10).  The duration of each clip should not be less than 4 seconds, otherwise an error code will be returned.</li>
     *      <li>When trimMode = <tt>kAutoTrim_Interval</tt>, this value is the interval time in milliseconds that thumbnail images will be extracted from the original clip.(Min 1000)</li>
     * </ul>
     * @return 0 if successful, -1 if the value of the parameter <tt>value</tt> is not in the range or -2 if the value is not applicable on the current video clip.
     * 
     * @see #getTrimCount()
     * @since version 1.0.0
     * @deprecated For internal use only. Please do not use.
     * 
     */
    @Deprecated
    public int setAutoTrim(int trimMode, int value){
        return 0;
    }

    /**
     * This method clears the trim information set to a clip. 
     * When trim information is cleared from the clip, the clip will return to its original state. 
     *
     * <p>Example code :</p>
     * {@code clip.getVideoClipEdit().clearTrim();}
     * @see #setTrim(int, int)
     * @since version 1.3.43
     */
    public void clearTrim(){
        mTrimStartDuration = 0;
        mTrimEndDuration = 0;
        mUpdated = true;
        mClip.setProjectUpdateSignal(false);
    }

    protected  void setFreezeDuration(int duration) {
        mFreezeDuration = duration;
    }

    /**
     * This method gets the duration of a video clip within a project after being edited.  
     * For example, the video clip could be trimmed or speed adjusted so that the duration is different from the original duration. 
     *
     * <p>Example code :</p>
     * {@code int duration = clip.getVideoClipEdit().getDuration();}
     * @return The duration of the video clip within a project in <tt>msec</tt> (milliseconds).   
     * @since version 1.3.43
     */
    public int getDuration(){
        int duration = 0;
        if( mTrimStartDuration != 0 || mTrimEndDuration != 0){
            duration = (mClip.getTotalTime() - mTrimStartDuration -mTrimEndDuration);
        }else{
            duration = mClip.getTotalTime();
        }

        if( mMasterSpeedControl != 100 ){

            if( mMasterSpeedControl == 2 ) {
                duration = duration * 50;
            }else if( mMasterSpeedControl == 3 ) {
                duration = duration * 32;
            }else if( mMasterSpeedControl == 6 ) {
                duration = duration * 16;
            }else if( mMasterSpeedControl == 13 ){
                duration = duration * 8;
            }else {
                duration = Math.round((float)(duration*100/mMasterSpeedControl));
            }
        }

        if( duration < 500 ){
            Log.w("nexVideoClipEdit","clip duration under 500! duration="+duration+", speed="+mMasterSpeedControl+", trim_start="+mTrimStartDuration+", trim_end="+mTrimEndDuration);
        }

        duration += mFreezeDuration;
        return duration;
    }

    private int speedControlTab(int value){
        if(EditorGlobal.PROJECT_NAME.equals("Gionee")) {
            int reval = value;
            if(value < 13) reval = 13;
            else if(value > kSpeedControl_MaxValue) reval = kSpeedControl_MaxValue;
            return reval;
        } else {
            int tab[] = {3,6,13,25,50,75,100,125,150,175,200,400, 800,kSpeedControl_MaxValue};
            int reval = 1600;
            for( int i = 0 ; i < tab.length ; i++ ){
                if( tab[i] >= value ){
                    reval = tab[i];
                    break;
                }
            }
            return reval;
        }
    }

    nexSaveDataFormat.nexVideoClipEditOf getSaveData(){
        nexSaveDataFormat.nexVideoClipEditOf data = new nexSaveDataFormat.nexVideoClipEditOf();
        data.mTrimStartDuration = mTrimStartDuration;
        data.mTrimEndDuration = mTrimEndDuration;
        data.mMasterSpeedControl = mMasterSpeedControl;
        data.mKeepPitch = mKeepPitch;

        return data;
    }

    nexVideoClipEdit(nexClip clip, nexSaveDataFormat.nexVideoClipEditOf data){
        mTrimStartDuration = data.mTrimStartDuration;
        mTrimEndDuration = data.mTrimEndDuration;
        mMasterSpeedControl = data.mMasterSpeedControl;
        mKeepPitch = data.mKeepPitch;
        mClip = clip;
    }

}
