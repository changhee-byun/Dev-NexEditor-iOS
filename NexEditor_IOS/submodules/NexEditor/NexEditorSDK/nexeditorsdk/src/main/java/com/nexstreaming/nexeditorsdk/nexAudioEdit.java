/******************************************************************************
 * File Name        : nexAudioEdit.java
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

/**
 *
 * @since 1.5.43
 */
/**
 * This class handles playback of voice change, pitch, compressor, music effect, and pan control options of an audio clip in NexEditor&trade;&nbsp;.
 * <p>Example code 1:</p>
 * {@code mEngine.getProject().getClip(0,true).getnexAudioEdit().setPitch(2);
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
 * @since version 1.5.43
 */
public class nexAudioEdit implements Cloneable{
    private final static String TAG = "nexAudioEdit";
    private int VCfactor=0; //0~4
    private int mPitch = 0; //-12 ~ 12
    private int mCompressor = 0; //0~7
    private int mProcessorStrength = -1; //0~6
    private int mBassStrength = -1; //0~6
    private int mMusicEffect = 0; //0~3
    private int mPanLeft = -111; //-100 ~ 100
    private int mPanRight = -111; //-100 ~ 100
    private String mEnhancedAudioFilter = null;
	private String mEqualizer = null;

    private nexClip mClip;

    private nexAudioEdit(){}

    private nexAudioEdit(nexClip clip){
        mClip = clip;
    }

    protected static nexAudioEdit clone(nexClip clip, nexAudioEdit src) {
        nexAudioEdit object = null;
        try {
            object = (nexAudioEdit)src.clone();
            object.mClip = clip;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }


    static nexAudioEdit getnexAudioEdit(nexClip clip){
        if( clip.getClipType() != nexClip.kCLIP_TYPE_VIDEO && clip.getClipType() != nexClip.kCLIP_TYPE_AUDIO ){
            return null;
        }
        return new nexAudioEdit(clip);
    }

    /**
     *
     *
     * @see #setVoiceChangerFactor(int)
     * @see #getVoiceChangerFactor()
     * @since 1.5.43
     */
    public static final int kVoiceFactor_NONE = 0;

    /**
     *
     *
     * @see #setVoiceChangerFactor(int)
     * @see #getVoiceChangerFactor()
     * @since 1.5.43
     */
    public static final int kVoiceFactor_CHIPMUNK = 1;

    /**
     *
     *
     * @see #setVoiceChangerFactor(int)
     * @see #getVoiceChangerFactor()
     * @since 1.5.43
     */
    public static final int kVoiceFactor_ROBOT = 2;

    /**
     *
     *
     * @see #setVoiceChangerFactor(int)
     * @see #getVoiceChangerFactor()
     * @since 1.5.43
     */
    public static final int kVoiceFactor_DEEP = 3;

    /**
     *
     *
     * @see #setVoiceChangerFactor(int)
     * @see #getVoiceChangerFactor()
     * @since 1.5.43
     */
    public static final int kVoiceFactor_MODULATION = 4;

    /**
     *
     *
     * @see #setMusicEffect(int)
     * @see #getMusicEffect()
     * @since 1.5.43
     */
    public static final int kMusicEffect_NONE = 0;

    /**
     *
     *
     * @see #setMusicEffect(int)
     * @see #getMusicEffect()
     * @since 1.5.43
     */
    public static final int kMusicEffect_LIVE_CONCERT = 1;

    /**
     *
     *
     * @see #setMusicEffect(int)
     * @see #getMusicEffect()
     * @since 1.5.43
     */
    public static final int kMusicEffect_STEREO_CHORUS = 2;

    /**
     *
     *
     * @see #setMusicEffect(int)
     * @see #getMusicEffect()
     * @since 1.5.43
     */
    public static final int kMusicEffect_MUSIC_ENHANCER = 3;


    /**
     * This method sets the pitch factor of the audio clip.
     * 
     * @param mPitch : -12 ~ 12
     * @see #getPitch()
     * @since 1.5.43
     */
    public void setPitch(int mPitch) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG,"setPitch getInfo fail!");
            return;
        }

        this.mPitch = mPitch;
    }

    /**
     * This method gets the applied pitch factor of the audio clip.
     * 
     * @return
     * @see #setPitch(int)
     * @since 1.5.43
     */
    public int getPitch() {
        return mPitch;
    }

    /**
     * @brief This method sets the compressor factor of the audio clip.
     * The compressor factor which automatically maintains a constant volume level for all different kinds of sound sources such as movies, music, or mobile TV.
     *
     * @param mCompressor : 0 ~ 7
     * @see #getCompressor()
     * @since 1.5.43
     */
    public void setCompressor(int mCompressor) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG,"setCompressor getInfo fail!");
            return;
        }

        this.mCompressor = mCompressor;
    }

    /**
     * This method gets the compressor factor of the audio clip.
     * 
     * @return
     * @see #setCompressor(int)
     * @since 1.5.43
     */
    public int getCompressor() {
        return mCompressor;
    }

    /**
     * This method sets the processor strength, the first parameter, when applying a music effect to an audio clip. 
     *
     * @param mProcessorStrength : 0 ~ 6
     * @see #getProcessorStrength()
     * @since 1.5.43
     */
    public void setProcessorStrength(int mProcessorStrength) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG,"setProcessorStrength getInfo fail!");
            return;
        }

        this.mProcessorStrength = mProcessorStrength;
    }

    /**
     * This method gets the processor strength, the first parameter, when applying a music effect to an audio clip.
     *
     * @return
     * @see #setProcessorStrength(int)
     * @since 1.5.43
     */
    public int getProcessorStrength() {
        return mProcessorStrength;
    }

    /**
     * This method sets the bass strength, the second parameter, when applying a music effect to an audio clip.
     *
     * @param mBassStrength : 0 ~ 6
     * @see #getBassStrength()
     * @since 1.5.43
     */
    public void setBassStrength(int mBassStrength) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG,"setBassStrength getInfo fail!");
            return;
        }

        this.mBassStrength = mBassStrength;
    }

    /**
     * This method gets the bass strength, the second parameter, when applying a music effect to an audio clip.
     *
     * @return
     * @see #setBassStrength(int)
     * @since 1.5.43
     */
    public int getBassStrength() {
        return mBassStrength;
    }
  /**
    * This method applies the music effect values to an audio clip.
    *
    * @param factor Audio music effect as an <tt>integer</tt>, in the range of 0 ~ 4.
    *               Possible values for music effect factor :
    *               <ul>
    *               <li> {@link #kMusicEffect_NONE} : Default
    *               <li> {@link #kMusicEffect_LIVE_CONCERT}
    *               <li> {@link #kMusicEffect_STEREO_CHORUS}
    *               <li> {@link #kMusicEffect_MUSIC_ENHANCER}
    *               </ul>
    * @since version 1.5.43
    * @see #getMusicEffect()
    */
   public void setMusicEffect(int mMusicEffect) {
       if(!mClip.getAttachmentState()){
           Log.d(TAG,"setMusicEffect getInfo fail!");
           return;
       }

       this.mMusicEffect = mMusicEffect;
   }
    /**
    * This method gets the music effect values applied to the audio clip.
    *
    * @param factor Audio music effect as an <tt>integer</tt>, in the range of 0 ~ 4.
    *               Possible values for music effect factor :
    *               <ul>
    *               <li> {@link #kMusicEffect_NONE} : Default
    *               <li> {@link #kMusicEffect_LIVE_CONCERT}
    *               <li> {@link #kMusicEffect_STEREO_CHORUS}
    *               <li> {@link #kMusicEffect_MUSIC_ENHANCER}
    *               </ul>
    * @since version 1.5.43
    * @see #setMusicEffect(int)
    */
   public int getMusicEffect() {
       return mMusicEffect;
   }
   
    /**
     * This method applies the pan left value to an audio clip.
     *
     * @return The pan left factor, as an <tt>integer</tt> (-100 ~ 100).
     * @see #setPanLeft(int)
     * @since 1.5.43
     */
    public int getPanLeft() {
        return mPanLeft;
    }

    /**
     * This method gets the pan left value of an audio clip.
     *
     * @param panLeft : The left channel's pan value.
     * @see #getPanLeft()
     * @since 1.5.43
     */
    public void setPanLeft(int panLeft) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG, "setPanleft getInfo fail!");
            return;
        }

        this.mPanLeft = panLeft;
    }

    /**
     * This method applies the pan right value to an audio clip.
     *
     * @return The pan right factor, as an <tt>integer</tt> (-100 ~ 100).
     *
     * @since version 1.5.43
     * @see #setPanRight(int)
     * @since 1.5.43
     */
    public int getPanRight() {
        return mPanRight;
    }

    /**
     * This method gets the pan right value of an audio clip.
     *
     * @param panRight : The right channel's pan value.
     * @see #getPanRight()
     * @since 1.5.43
     */
    public void setPanRight(int panRight) {
        if(!mClip.getAttachmentState()){
            Log.d(TAG,"setPanRight getInfo fail!");
            return;
        }

        this.mPanRight = panRight;
    }

    /**
     * This method sets the voice changer factor which is used to modulate the audio clip.
     *
     * @param factor Audio modulate value as an <tt>integer</tt>, in the range of 0 ~ 4.
     *               Possible values for the voice changer factor :
     *               <ul>
     *               <li> {@link #kVoiceFactor_NONE} : Default
     *               <li> {@link #kVoiceFactor_CHIPMUNK} : Chipmunk
     *               <li> {@link #kVoiceFactor_ROBOT} : Robot
     *               <li> {@link #kVoiceFactor_DEEP} : Deep
     *               <li> {@link #kVoiceFactor_MODULATION} : Modulation
     *               </ul>
     * @since version 1.5.15
     * @see #getVoiceChangerFactor()
     */
    public void setVoiceChangerFactor(int factor){
        VCfactor = factor;
    }

    /**
     * This method gets the voice changer factor set to a clip.
     *
     * @return The voice changer factor, as an <tt>integer</tt> (0 ~ 4).
     *               <ul>
     *               <li> {@link #kVoiceFactor_NONE} : Default
     *               <li> {@link #kVoiceFactor_CHIPMUNK} : Chipmunk
     *               <li> {@link #kVoiceFactor_ROBOT} : Robot
     *               <li> {@link #kVoiceFactor_DEEP} : Deep
     *               <li> {@link #kVoiceFactor_MODULATION} : Modulation
     *               </ul>
     * @since version 1.5.15
     * @see #setVoiceChangerFactor(int)
     */
    public int getVoiceChangerFactor(){
        return VCfactor;
    }

    /**
     * This method returns a json file which specifies audio filter settings set by setEnhancedAudioFilter().
     * @return json file
     * @since 1.7.7
     * @
     */
    public String getEnhancedAudioFilter()
    {
        return mEnhancedAudioFilter;
    }

    /**
     * This method sets all audio filter settings in json format.
     * @param json json file (Nexstreaming use only)
     * @since 1.7.7
     */
    public void setEnhancedAudioFilter(String json)
    {
        mEnhancedAudioFilter = json;
    }

    /**
     * This method returns a json file which specifies audio filter settings set by setEqualizer().
     * @return json file
     * @since 1.7.7
     * @
     */
    public String getEqualizer()
    {
        return mEqualizer;
    }

    /**
     * This method sets all audio filter settings in json format.
     * @param json json file (Nexstreaming use only)
     * @since 1.7.7
     */
    public void setEqualizer(String json)
    {
        mEqualizer = json;
    }

    nexSaveDataFormat.nexAudioEditOf getSaveData(){
        nexSaveDataFormat.nexAudioEditOf data = new nexSaveDataFormat.nexAudioEditOf();
        data.VCfactor=VCfactor; //0~4
        data.mPitch = mPitch; //-12 ~ 12
        data.mCompressor = mCompressor; //0~7
        data.mProcessorStrength = mProcessorStrength; //0~6
        data.mBassStrength = mBassStrength; //0~6
        data.mMusicEffect = mMusicEffect; //0~3
        data.mPanLeft = mPanLeft; //-100 ~ 100
        data.mPanRight = mPanRight; //-100 ~ 100
        return data;
    }

    nexAudioEdit( nexClip clip, nexSaveDataFormat.nexAudioEditOf data){
        VCfactor = data.VCfactor; //0~4
        mPitch = data.mPitch; //-12 ~ 12
        mCompressor = data.mCompressor; //0~7
        mProcessorStrength = data.mProcessorStrength; //0~6
        mBassStrength = data.mBassStrength; //0~6
        mMusicEffect = data.mMusicEffect; //0~3
        mPanLeft = data.mPanLeft; //-100 ~ 100
        mPanRight = data.mPanRight; //-100 ~ 100
        mClip = clip;
    }
}
