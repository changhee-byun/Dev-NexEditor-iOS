/******************************************************************************
 * File Name        : nexClip.java
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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.os.Environment;
import android.util.Log;

import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.task.ResultTask.OnResultAvailableListener;
import com.nexstreaming.app.common.task.Task.Event;
import com.nexstreaming.app.common.task.Task.OnFailListener;
import com.nexstreaming.app.common.task.Task.TaskError;
import com.nexstreaming.app.common.util.FileType;
import com.nexstreaming.app.common.util.FreeSpaceChecker;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.mediainfo.PCMLevels;
import com.nexstreaming.kminternal.kinemaster.mediainfo.nexCodecType;
import com.nexstreaming.nexeditorsdk.exception.ClipIsNotVideoException;
import com.nexstreaming.nexeditorsdk.exception.ProjectNotAttachedException;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.kminternal.kinemaster.mediainfo.ThumbnailCallbackBitmap;
import com.nexstreaming.kminternal.kinemaster.mediainfo.ThumbnailCallbackRaw;
import com.nexstreaming.kminternal.kinemaster.mediainfo.Thumbnails;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader;
import com.nexstreaming.kminternal.nexvideoeditor.NexVisualClipChecker;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * This class manages image clips, video clips and audio clips to be edited with the NexEditor&trade;&nbsp;SDK.
 * NexEditor&trade;&nbsp;SDK supports the following file formats:
 * <ul>
 * <li>Supported video file formats : MP4, 3GP and MOV.
 * <li>Supported video codec : H.264
 * <li>Supported audio file formats : MP3 and AAC.
 * <li>Supported image file formats : Android dependency.
 * </ul>
 * The clips will be saved in the path of the original media content which the clip was created from 
 * and if the format of the clip is unsupported, {@link #kCLIP_TYPE_NONE} will be returned from {@link #getClipType()}. 
 * Another reason for a clip to be unsupported may be because of the resolution limitations from the hardware performance. 
 * Only after a clip is added to {@link com.nexstreaming.nexeditorsdk.nexProject} can it be edited or have effects applied.
 * When editing a clip that is not added to <tt>nexProject</tt>, <tt>ProjectNotAttachedException</tt> will occur.
 *
 * <p>Example code :</p>
 * 			 
 *     {@code nexClip clip = nexClip.getSupportedClip("/mnt/sdcard/1.jpg");
        if( clip ){
            if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ){
                Log.i(Tag,"wid = "+ nexClip.getWidth());
            }else if( clip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                Log.i(Tag,"time = "+ nexClip.getTotalTime());
            }else if( clip.getClipType() == nexClip.kCLIP_TYPE_NONE ){
                Log.e(Tag,"not supported clip = "+ clip.getSupportedResult());
            }
        }
    }
 * @since version 1.0.0
 *
 * @see com.nexstreaming.nexeditorsdk.nexProject# add(int, boolean, nexClip)
 * @see com.nexstreaming.nexeditorsdk.nexProject#getClip(int, boolean)
 */
public class nexClip implements Cloneable {
    private final static String TAG = "nexClip";
    private String mPath = null;
    private String mTransCodingPath = null;
    private boolean mProjectAttachment;
    private nexClipEffect mClipEffect = null;
    private nexTransitionEffect mTransitionEffect = null;
    private boolean misMustDownSize;
    private transient WeakReference<Bitmap> mSingleThumbnail=null;
    private Thumbnails mThumbnails = null;
    private boolean m_gettingThumbnails=false;
    private boolean m_getThumbnailsFailed=false;
    private nexObserver mObserver;
    private boolean  mAudioOnOff = true;
//color adjust
    private int m_Brightness;
    private int m_Contrast;
    private int m_Saturation;
    private int m_Hue;
    private boolean mVignette;
    private boolean mFacedetectProcessed = false;
    private int     mFaceDetected = 0;
    private Rect    mFaceRect = new Rect();

    private nexCrop mCrop;
    private int index = 0;
    private int count = 0;
    protected int mTitleEffectStartTime;
    protected int mTitleEffectEndTime;

    private nexAudioEnvelop mAudioEnvelop = null;

    private MediaInfo mediainfo = null;
    private nexAudioEdit mAudioEdit;

    private int mTemplateEffectID = 0;
    private String mCollageDrawInfoID;
    private int mTemplateAudioPos = 0;
    private List<nexDrawInfo> mDrawInfos = new ArrayList<>();
    private boolean mOverlappedTransition = true;
    private boolean mMediaInfoUseCache = true;
    protected int mStartTime = 0;
    protected int mEndTime = 0;
    protected int mDuration = nexProject.kAutoThemeClipDuration;
    private ClipInfo mInfo;
    private nexVideoClipEdit mVideoEdit;
    private int mCustomLUT_A = 0;
    private int mCustomLUT_B = 0;
    private int mCustomLUT_Power = 100000;
    private nexColorEffect mColorEffect = nexColorEffect.NONE;
    boolean mPropertySlowVideoMode;
    boolean  mIframePlay;
    private int mAVSyncAudioStartTime = 0;
    private int m_ClipVolume = 100;
    private int m_BGMVolume = 100;
    private int mRotate = kClip_Rotate_0;
    private boolean isMotionTrackedVideo = false;
    /**
     * @deprecated For internal use only. Please do not use.
     * 
     */
    @Deprecated
    public static void setThumbTempDir(String path){
        if( path == null ) {
            MediaInfo.setThumbnailTempDir(null);
        }else{
            File dir = new File(path);
            if( dir.isDirectory() ){
                MediaInfo.setThumbnailTempDir(dir);
            }else{
                Log.e(TAG,"setThumbTempDir not dir="+path);
            }
        }
    }

    /**
     * This method sets the brightness of a clip.
     * 
     * <p>Example code:</p>
     *     {@code
                project.getClip(1 + i, true).setBrightness(100);
            }
     * @param value The brightness to set on the clip, as an <tt>integer</tt>.  This must be in the range of -255 to 255.
     * 
     * @return <tt>TRUE</tt> if the value is set successfully; <tt>FALSE</tt> if the value is not in range.
     * 
     * @see #getBrightness() 
     * @since version 1.1.0
     * 
     */
    public boolean setBrightness(int value){
        if( value < -255 ||  value > 255 )
            return false;
        m_Brightness = value;
        setProjectUpdateSignal(true);
        return true;
    }

    /** 
     * This method sets the contrast of a clip. 
     *
     * <p>Example code:</p>
     *     <b>class sample code</b>
     *     {@code
                project.getClip(1 + i, true).setContrast(-25);
            }
     * @param value The contrast to set on the clip, as an <tt>integer</tt>.  This must be in the range of -255 to 255.
     * @return <tt>TRUE</tt> if the value is set successfully; <tt>FALSE</tt> if the value is not in range. 
     * 
     * @see #getContrast()
     * @since version 1.1.0
     * 
     */
    public boolean setContrast(int value){
        if( value < -255 ||  value > 255 )
            return false;
        m_Contrast = value;
        setProjectUpdateSignal(true);
        return true;
    }

    /** 
     * This method sets the saturation of a clip. 
     *
     * <p>Example code:</p>
     *     {@code
                project.getClip(1 + i, true).setSaturation(-25);
            }
     * @param value The saturation to set on the clip, as an <tt>integer</tt>. This must be in the range of -255 to 255.
     * @return <tt>TRUE</tt> if the value is set successfully; <tt>FALSE</tt> if the value is not in range.
     * 
     * @see #getSaturation() 
     * @since version 1.1.0
     * 
     */
    public boolean setSaturation(int value){
        if( value < -255 ||  value > 255 )
            return false;
        m_Saturation = value;
        setProjectUpdateSignal(true);
        return true;
    }

    public boolean setHue(int value){
        if( value < -180 ||  value > 180 )
            return false;
        m_Hue = value;
        setProjectUpdateSignal(true);
        return true;
    }
    /**
     * This method sets a color effect on a clip.
     * 
     * @param effect  The color effect to set on the clip.
     * <p>Example code :</p>
     *  {@code clip.setColorEffect(nexColorEffect.ALIEN_INVASION); }
     * @since version 1.1.0
     * @see #getColorEffect()
     */
    public void setColorEffect(nexColorEffect effect){
        mColorEffect = effect;
        setProjectUpdateSignal(true);
    }

    /**
     * This method gets the value of a color effect on a clip.  
     *
     * <p>Example code:</p>
     * {@code if(color_filter != -1) {
            clip2.setColorEffect(getColorEffect(color_filter - 1));
        }}
     * @return The value of the color effect set on the clip.
     * 
     * @see #setColorEffect(nexColorEffect)
     * @since version 1.1.0
     * 
     */
    public nexColorEffect getColorEffect(){
        return mColorEffect;
    }

    String getColorEffectID(){
        if( mColorEffect == null ){
            return nexColorEffect.NONE.getKineMasterID();
        }
        return mColorEffect.getKineMasterID();
    }
    /**
     * This method gets the brightness of a clip. 
     * 
     * <p>Example code:</p>
     *     {@code
                mBrightness = clip.getBrightness();
            }
     * @return The brightness set to the clip, in the range of -255 to 255.
     * 
     * @see #setBrightness(int) 
     * @since version 1.1.0
     * 
     */
    public int getBrightness(){
        return m_Brightness;
    }

    /**
     * This method gets the contrast of a clip.
     *
     * <p>Example code:</p>
     *     {@code
                mContrast = clip.getContrast();
            }
     * @return The contrast set to the clip, in the range of -255 to 255. 
     * 
     * @see #setContrast(int)
     * @since version 1.1.0
     * 
     */
    public int getContrast(){
        return m_Contrast;
    }

    /**
     * This method gets the color saturation of a clip. 
     *
     * <p>Example code:</p>
     *     {@code
                mSaturation = clip.getSaturation();
            }
     * @return The color saturation set on the clip, in the range of -255 to 255. 
     * 
     * @see #setSaturation(int)
     * @since version 1.1.0
     * 
     */
    public int getSaturation(){
        return m_Saturation;
    }

    public int getHue(){
        return m_Hue;
    }

    protected int getCombinedBrightness() {
        if( mColorEffect==null )
            return m_Brightness;
        return m_Brightness + (int)(255*mColorEffect.getBrightness());
    }

    protected int getCombinedContrast() {
        if( mColorEffect==null )
            return m_Contrast;
        return m_Contrast + (int)(255*mColorEffect.getContrast());
    }

    protected int getCombinedSaturation() {
        if( mColorEffect==null )
            return m_Saturation;
        return m_Saturation + (int)(255*mColorEffect.getSaturation());
    }

    protected int getCombinedHue() {
        if( mColorEffect==null )
            return m_Hue;
        return m_Hue + (int)(180*mColorEffect.getHue());
    }

    protected int getTintColor(){
        if( mColorEffect==null )
            return 0;
        return  mColorEffect.getTintColor();
    }

    protected int getLUTId(){
        if( mColorEffect==null )
            return 0;
        return  mColorEffect.getLUTId();
    }

    protected int getCustomLUTA(){

        return mCustomLUT_A;
    }

    protected int getCustomLUTB(){

        return mCustomLUT_B;
    }

    protected int getCustomLUTPower(){

        return mCustomLUT_Power;
    }

    /**
     * @hide
     * @param lut_uid
     */
    public void setCustomLUTA(int lut_uid){

        mCustomLUT_A = lut_uid;
    }

    /**
     * @hide
     * @param lut_uid
     */
    public void setCustomLUTB(int lut_uid){

        mCustomLUT_B = lut_uid;
    }

    /**
     * @hide
     * @param power from 0 to 100000 and it will be mapped as from 0 to 1 float value.
     *              final color = mCustomLUT_A * m_CustomLUT_Power / 100000 + mCustomLUT_B * ( 1.0 - m_CustomLUT_Power / 100000)
     */
    public void setCustomLUTPower(int power){

        mCustomLUT_Power = power;
    }

    /**
     * This indicates the H.264 including an unknown profile.    
     *
     * 
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_Unknown = 0;

    /**
     * This indicates the H.264 including Base Line profile.
     * 
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_Baseline = 66;

    /**
     * This indicates the H.264 including Main profile. 
     * 
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_Main = 77;

    /**
     * This indicates the H.264 including Extended profile.
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_Extended = 88;

    /**
     * This indicates the H.264 including High profile.
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_High = 100;

    /**
     * This indicates the H.264 including High 10 profile.
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_High10 = 100;

    /**
     * This indicates the H.264 including High 422 profile.
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_High422 = 122;

    /**
     * This indicates the H.264 including High 444 profile.
     * @since version 1.3.4
     * @see #getAVCProfile()
     */
    public static final int AVC_Profile_High444 = 244;

    /**
     * This indicates Hdr10 PQ type.
     * @since 2.0.0
     * @see #getVideoHDRType()
     */
    public static final int HDR_TYPE_10PQ = 16;

    /**
     * This indicates Hdr10 HLG type.
     * @since 2.0.0
     * @see #getVideoHDRType()
     */
    public static final int HDR_TYPE_10HLG = 18;

    protected static class ClipInfo implements Cloneable {
        public String mMimeType;
        public int mClipType = kCLIP_TYPE_NONE;
        public int mWidth;
        public int mHeight;
        public int mDisplayWidth;
        public int mDisplayHeight;
        public boolean mExistVideo;
        public boolean mExistAudio;
        public int mTotalTime;
        public int mFramesPerSecond;
        public float mFramesPerSecondFloat;
        public int mRotateDegreeInMeta;
        public int mH264Profile;
        public int mH264Level;
        public int mSuppertedResult = kClip_Supported_Unknown;
        public int mVideoBitrate;
        public int mAudioBitrate;
        public int mVideoTotalTime;
        public int mAudioTotalTime;
        public int mSeekPointCount;
        public int mVideoRenderMode;
        public String mVideoCodecType;
        public String mAudioCodecType;
        public byte[] mVideoUUID;
        public int mVideoHDRType;
//        public boolean mSupported;

        protected static ClipInfo clone(ClipInfo src) {
            ClipInfo object = null;
            try {
                object = (ClipInfo)src.clone();
                object.mMimeType = src.mMimeType;
                object.mVideoCodecType = src.mVideoCodecType;
                object.mAudioCodecType = src.mAudioCodecType;
                if( src.mVideoUUID == null ){
                    object.mVideoUUID = null;
                }else{
                    object.mVideoUUID = new byte[src.mVideoUUID.length];
                    System.arraycopy(src.mVideoUUID,0,object.mVideoUUID,0,object.mVideoUUID.length);
                }

            } catch (CloneNotSupportedException e) {
                e.printStackTrace();
            }

            return object;
        }
    }


    /** 
     * This member indicates no clip rotation. 
     *
     * @see #setRotateDegree(int)
     * @see #getRotateDegree()
     * @since version 1.0.1
     */
    public static final int kClip_Rotate_0 = 0;

    /** 
     * This member indicates a rotation of a clip by 90 degrees counterclockwise.  
     * 
     * @see #setRotateDegree(int)
     * @see #getRotateDegree()
     * @since version 1.0.1
     */
    public static final int kClip_Rotate_90 = 90;

    /** 
     * This member indicates a rotation of a clip by 180 degrees counterclockwise.  
     * 
     * @see #setRotateDegree(int)
     * @see #getRotateDegree()
     * @since version 1.0.1
     */
    public static final int kClip_Rotate_180 = 180;

    /** 
     * This member indicates a rotation of a clip by 270 degrees counterclockwise.
     * @see #setRotateDegree(int)
     * @see #getRotateDegree()
     * @since version 1.0.1
     */
    public static final int kClip_Rotate_270 = 270;



    /**
     * This member indicates that a clip is supported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kCLIP_Supported = 0;

    /**
     * This member indicates that it is not known whether or not a clip is supported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_Supported_Unknown = 1;

    /**
     * This member indicates that a clip includes an audio file unsupported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_AudioCodec = 2;

    /**
     * This member indicates that a clip includes an unsupported audio profile.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_AudioProfile = 3;

    /**
     * This member indicates that a clip is in a format unsupported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_Container = 4;

    /**
     * This member indicates that a clip is unsupported because the resolution is too high.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_ResolutionTooHigh = 5;

    /**
     * This member indicates that a video or audio clip is too short to be supported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_DurationTooShort = 6;

    /**
     * This member indicates that a clip is unsupported because the resolution is too low.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_ResolutionTooLow = 7;

    /**
     * This member indicates that a clip includes an unsupported video profile.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_VideoProfile = 8;

    /**
     * This member indicates that a clip includes an unsupported video file.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_VideoCodec = 9;

    /**
     * This member indicates that a clip has an unsupported video FPS.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_VideoFPS = 10;

    /**
     * This member indicates that a clip has an unsupported video level.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported_VideoLevel = 11;

    /**
     * This member indicates that a clip is unsupported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_NotSupported = 12;

    /**
     * This member indicates that a clip needs to be changed to a lower resolution in order to be edited in the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public static final int kClip_Supported_NeedResolutionTranscoding = 13;

    /**
     *  This member indicates a clip that needs to be changed to a lower FPS.
     *  
     *  @see #getSupportedResult()
     *  @since version 1.0.0
     *  
     */
    public static final int kClip_Supported_NeedFPSTranscoding = 14;

    /**
     * The status of the clip has not yet been clarified.
     * 
     * @see #getClipType()
     * @since version 1.0.0
     * 
     */
    public static final int kCLIP_TYPE_NONE = 0x00000000;

    /**
     * Image files of photos or pictures.
     * 
     * @see #getClipType()
     * @since version 1.0.0
     * 
     */
    public static final int kCLIP_TYPE_IMAGE = 0x00000001;

//    public static final int kCLIP_TYPE_TEXT = 0x00000002;

    /**
     * Audio files with file formats such as MP3 and AAC.
     * 
     * @see #getClipType()
     * @since version 1.0.0
     * @see #getClipType()
     */
    public static final int kCLIP_TYPE_AUDIO = 0x00000003;

    /**
     * Video files with file formats such as MP4.
     * 
     * @see #getClipType()
     * @since version 1.0.0
     * @see #getClipType()
     */
    public static final int kCLIP_TYPE_VIDEO = 0x00000004;
//    public static final int kCLIP_TYPE_LAYER = 0x00000005;

    /**
     * @brief This is a return value of getVideoRenderMode().
     * @note Normal video.
     * @see #getVideoRenderMode()
     * @since 1.5.30
     */
    public static final int kCLIP_VIDEORENDERMODE_NORMAL = 0;

    /**
     * @brief This is a return value of getVideoRenderMode().
     * @note 360 video.
     * @see #getVideoRenderMode()
     * @since 1.5.30
     */
    public static final int kCLIP_VIDEORENDERMODE_360VIDE = 1;
    /**
     * This method creates a clip as an instance of <tt>nexClip</tt> and gets the media information about that clip.
     * Therefore, it may take a while to create a particular <tt>nexClip</tt>, depending on the size and format of the file.
     * 
     * <p>Example code:</p>
     *     <b>class sample code</b>
     *     {@code
                mClip = new nexClip(m_listfilepath.get(0));
           }
     * @param path The location where the media clip is saved.
     * 
     * @see #getSupportedClip(String)
     * @see #getSupportedResult()
     * @since version 1.0.0
     * 
     */
    public nexClip(String path) {
        if( path.startsWith("@assetItem:") ){
            mPath = path.substring(11);
            mTransCodingPath = path;
            mMediaInfoUseCache = false;
            mInfo = new ClipInfo();
            mInfo.mClipType = kCLIP_TYPE_IMAGE;
            Rect r = nexAssetPackageManager.getAssetPackageMediaOptions(mPath);
            mInfo.mWidth = r.width();
            mInfo.mHeight = r.height();

        }else {
            mPath = path;
            mTransCodingPath = transCodingPath(path);
            if( mTransCodingPath != null ) {
                if (mTransCodingPath.startsWith("@assetItem:")) {
                    mMediaInfoUseCache = false;
                    mInfo = new ClipInfo();
                    mInfo.mClipType = kCLIP_TYPE_IMAGE;
                    Rect r = nexAssetPackageManager.getAssetPackageMediaOptions(mPath);
                    mInfo.mWidth = r.width();
                    mInfo.mHeight = r.height();
                    return;
                }
            }
            mMediaInfoUseCache = true;
            setMediaInfo(MediaInfo.getInfo(getRealPath(), mMediaInfoUseCache));
            mInfo = resolveClip(getRealPath(), mMediaInfoUseCache, getMediaInfo(), false);
        }
    }

    /**
     * This method creates a clip as an instance of <tt>nexClip</tt> and gets the media information about that clip.
     * Therefore, it may take a while to create a particular <tt>nexClip</tt>, depending on the size and format of the file.
     * 
     * This method allows to decide whether to cache retrieved clip's media information. 
     * Once a clip's media information is cached, this method will operate faster when the same clip is called again.
     * It is recommended to set the parameter <tt>useCache</tt> to <tt>FALSE</tt> when a clip is for temporary use 
     * (for example, when a panorama image is made with number of trailing images, these images will be deleted once a new panorama image is made).
     * 
     * <p>Example code:</p>
     *     {@code
               mClip = new nexClip(m_listfilepath.get(0), true);
           }
     * @param path The location where the media clip is saved.
     * @param useCache Set to <tt>TRUE</tt> to cache retrieved media information; otherwise set to <tt>FALSE</tt>. 
     * @since version 1.3.43
     */
    public nexClip(String path, boolean useCache) {
        if( path.startsWith("@assetItem:") ){
            mPath = path.substring(11);
            mTransCodingPath = path;
            mMediaInfoUseCache = false;
            mInfo = new ClipInfo();
            mInfo.mClipType = kCLIP_TYPE_IMAGE;
            Rect r = nexAssetPackageManager.getAssetPackageMediaOptions(mPath);
            mInfo.mWidth = r.width();
            mInfo.mHeight = r.height();

        }else {
            mPath = path;
            mTransCodingPath = transCodingPath(path);
            if( mTransCodingPath != null ) {
                if (mTransCodingPath.startsWith("@assetItem:")) {
                    mMediaInfoUseCache = false;
                    mInfo = new ClipInfo();
                    mInfo.mClipType = kCLIP_TYPE_IMAGE;
                    Rect r = nexAssetPackageManager.getAssetPackageMediaOptions(mPath);
                    mInfo.mWidth = r.width();
                    mInfo.mHeight = r.height();
                    return;
                }
            }
            mMediaInfoUseCache = true;
            setMediaInfo(MediaInfo.getInfo(getRealPath(), mMediaInfoUseCache));
            mInfo = resolveClip(getRealPath(), mMediaInfoUseCache, getMediaInfo(), false);
        }
    }


    private nexClip(){

    }

    private static String transCodingPath(String path){
        if( path == null){
            Log.d(TAG,"transCodingPath path null");
            return null;
        }

        if( path.startsWith("nexasset://") ){
            Log.d(TAG,"transCodingPath path asset");
            return null;
        }

        if( path.startsWith("@solid:") ){
            Log.d(TAG,"transCodingPath path solid");
            return null;
        }

        File file = new File(path);
        if( file.isFile() ){
            Log.d(TAG,"transCodingPath path is file");
            return null;
        }

            return nexAssetPackageManager.getAssetPackageMediaPath(KineMasterSingleTon.getApplicationInstance().getApplicationContext(), path);

    }

    protected nexClip(nexClip src){
        mPath = src.mPath;
        mTransCodingPath = src.mTransCodingPath;
        misMustDownSize = src.misMustDownSize;
        mMediaInfoUseCache = src.mMediaInfoUseCache;
        mPropertySlowVideoMode = src.mPropertySlowVideoMode;
        if( src.mInfo != null ){
            if( mInfo == null ){
                mInfo = new ClipInfo();
            }
            mInfo.mMimeType = src.mInfo.mMimeType;
            mInfo.mClipType = src.mInfo.mClipType;
            mInfo.mWidth = src.mInfo.mWidth;
            mInfo.mHeight = src.mInfo.mHeight;
            mInfo.mDisplayWidth = src.mInfo.mDisplayWidth;
            mInfo.mDisplayHeight = src.mInfo.mDisplayHeight;
            mInfo.mExistVideo = src.mInfo.mExistVideo;
            mInfo.mExistAudio = src.mInfo.mExistAudio;
            mInfo.mTotalTime = src.mInfo.mTotalTime;
            mInfo.mFramesPerSecond = src.mInfo.mFramesPerSecond;
            mInfo.mFramesPerSecondFloat = src.mInfo.mFramesPerSecondFloat;
            mInfo.mRotateDegreeInMeta = src.mInfo.mRotateDegreeInMeta;
            mInfo.mH264Profile = src.mInfo.mH264Profile;
            mInfo.mH264Level = src.mInfo.mH264Level;
            mInfo.mSuppertedResult = src.mInfo.mSuppertedResult;
            mInfo.mVideoBitrate = src.mInfo.mVideoBitrate;
            mInfo.mAudioBitrate = src.mInfo.mAudioBitrate;
            mInfo.mVideoTotalTime = src.mInfo.mVideoTotalTime;
            mInfo.mAudioTotalTime = src.mInfo.mAudioTotalTime;
            mInfo.mSeekPointCount = src.mInfo.mSeekPointCount;
            mInfo.mVideoRenderMode = src.mInfo.mVideoRenderMode;
            mInfo.mVideoHDRType = src.mInfo.mVideoHDRType;
            if( src.mInfo.mAudioCodecType != null )
                mInfo.mAudioCodecType = new String(src.mInfo.mAudioCodecType);
            if( src.mInfo.mVideoCodecType != null )
                mInfo.mVideoCodecType = new String(src.mInfo.mVideoCodecType);

            if( src.mInfo.mVideoUUID != null ) {
                mInfo.mVideoUUID = new byte[src.mInfo.mVideoUUID.length];
                System.arraycopy(src.mInfo.mVideoUUID, 0, mInfo.mVideoUUID, 0, mInfo.mVideoUUID.length);
            }

        }
        if( src.mediainfo != null ) {
            mediainfo = src.mediainfo;
        }
    }

    /** 
     * This method replaces clip's content with another content. 
     * 
     * <p>Example code:</p>
     *     {@code if(project.getTotalClipCount(false) !=0) {
                project.getClip(0, false).replaceClip(m_soundfilepath);
            }
            }
     * @param path  The path of the content to replace the existing content of the clip.
     * 
     * @return 0 if successful; -1 if the new content type doesn't match or if the new content is not supported. 
     * @since version 1.1.0
     */
    public int replaceClip(String path){
        mTransCodingPath = transCodingPath(path);
        String replacePath = path;

        if( mTransCodingPath != null ){
            replacePath = mTransCodingPath;
        }
        ClipInfo info = resolveClip(replacePath,mMediaInfoUseCache, getMediaInfo(), false);

        if( info.mSuppertedResult != kCLIP_Supported ){
            return info.mSuppertedResult;
        }

        if( mInfo.mClipType != info.mClipType ){
            return -1;
        }
        mPath = path;
        mInfo = info;
        setProjectUpdateSignal(false);
        return 0;
    }

    protected void setProjectUpdateSignal(boolean onlyLoadList){
        if( mProjectAttachment ) {
            if( mObserver != null ){
                mObserver.updateTimeLine(false);
            }
        }
    }

    /**
     *  This method creates a new <tt>nexClip</tt> by duplicating an existing <tt>nexClip</tt> passed with this method.
     *  
     *  When a <tt>nexClip</tt> is duplicated, only the content information will be duplicated with default settings.
     *  Effects or other property settings from the original nexClip, that the new nexClip is being copied from, will not be duplicated.
     * <p>Example code :</p>
     *      {@code nexClip c = project.getClip(0,true);
            for(int i = 0 ; i < 3 ; i++){
                project.add(nexClip.dup(c));
            }
            }
     * @param src An existing instance of <tt>nexClip</tt> to be duplicated.
     * @return The new instance of the <tt>nexClip</tt> clip.
     * @since version 1.0.0
     */
    public static nexClip dup(nexClip src){
        return new nexClip(src);
    }

    protected static nexClip clone(nexClip src) {
        //Log.d("clone", "clone work./nexClip");
//        nexClip object = new nexClip(src);
        nexClip object = null;
        try {
            object = (nexClip)src.clone();
            if(src.mClipEffect != null) {
                object.mClipEffect = nexClipEffect.clone(src.mClipEffect);
            }
            if(src.mTransitionEffect != null) {
                object.mTransitionEffect = nexTransitionEffect.clone(src.mTransitionEffect);
            }
            if(src.mCrop != null) {
                object.mCrop = nexCrop.clone(src.mCrop);
            }
            if(src.mVideoEdit != null) {
                object.mVideoEdit = nexVideoClipEdit.clone(src.mVideoEdit);
            }
            if(src.mColorEffect != null) {
                object.mColorEffect = nexColorEffect.clone(src.mColorEffect);
            }
            if(src.mInfo != null) {
                object.mInfo = ClipInfo.clone(src.mInfo);
            }
            if(src.mAudioEnvelop != null) {
                object.mAudioEnvelop = nexAudioEnvelop.clone(src.mAudioEnvelop);
            }

            if( src.mAudioEdit != null ){
                object.mAudioEdit = nexAudioEdit.clone(object,src.mAudioEdit);
            }


            object.mTransCodingPath = src.mTransCodingPath;
            object.mSingleThumbnail = src.mSingleThumbnail;
            object.mThumbnails = src.mThumbnails;
//        object.misMustDownSize = src.misMustDownSize;
//        object.m_gettingThumbnails = src.m_gettingThumbnails;
//        object.m_getThumbnailsFailed = src.m_getThumbnailsFailed;
//        object.mAudioOnOff = src.mAudioOnOff;
//        object.m_Brightness = src.m_Brightness;
//        object.m_Contrast = src.m_Contrast;
//        object.m_Saturation = src.m_Saturation;
//        tmpClip.mStartTime = src.mStartTime;
//        tmpClip.mEndTime = src.mEndTime;
//        object.mDuration = src.mDuration;
//        object.m_BGMVolume = src.m_BGMVolume;
//        object.m_ClipVolume = src.m_ClipVolume;
//        object.mRotate = src.mRotate;
//        object.mMediaInfoUseCache = src.mMediaInfoUseCache;
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return  object;
    }

    nexClip(String path, String realPath){
        mPath = path;
        mTransCodingPath = realPath;
        mMediaInfoUseCache = false;
        mInfo = new ClipInfo();
        mInfo.mClipType = kCLIP_TYPE_IMAGE;
        Rect r = nexAssetPackageManager.getAssetPackageMediaOptions(mPath);
        mInfo.mWidth = r.width();
        mInfo.mHeight = r.height();
        Log.d("nexClip","@assetItem: w="+mInfo.mWidth+", h="+mInfo.mHeight);
    }

    private nexClip(String path, String realPath, ClipInfo info, boolean useCache, MediaInfo Minfo){
        mPath = path;
        mInfo = info;
        if( path.compareTo(realPath) != 0 ){
            mTransCodingPath = realPath;
        }
        mMediaInfoUseCache = useCache;
        if( Minfo != null ) {
            setMediaInfo(Minfo);
        }else{
            setMediaInfo(MediaInfo.getInfo(realPath, mMediaInfoUseCache));
        }
    }

    /**
     * This method only creates a media clip that is supported by the NexEditor&trade;&nbsp; SDK.
     *
     * <p>Example code :</p>
     *     {@code nexClip clip = nexClip.getSupportedClip("/mnt/sdcard/1.jpg");
     *      if( clip == null ){
     *          Log.e(Tag,"not supported clip");
     *      }
     *     }
     * @param path The location where the media clip is saved. 
     * @return A <tt>nexClip</tt> instance if the media clip is supported by the NexEditor&trade;&nbsp;SDK.
     * 
     * @see #nexClip(String)
     * @since version 1.0.0
     * 
     */
    public static nexClip getSupportedClip(String path){
        return getSupportedClip(path, true);
    }

    /**
     * This method only creates a media clip that is supported by the NexEditor&trade;&nbsp; SDK.
     *
     * <p>Example code:</p>
     *     {@code
                nexClip clip2= nexClip.getSupportedClip(m_soundfilepath.get(0));
           }
     * @param path The location where the media clip is saved. 
     * @param useCache Set to <tt>TRUE</tt> to cache retrieved media information; otherwise set to <tt>FALSE</tt>.  
     *                 This method allows to decide whether to cache retrieved clip's media information. 
     *                 Once a clip's media information is cached, this method will operate faster when the same clip is called again.
     *                 It is recommended to set the parameter <tt>useCache</tt> to <tt>FALSE</tt> when a clip is for temporary use 
     *                 (for example, when a panorama image is made with number of trailing images, these images will be deleted once a new panorama image is made).
     * @return A <tt>nexClip</tt> instance if the media clip is supported by the NexEditor&trade;&nbsp;SDK.
     * @since version 1.3.43
     */
    public static nexClip getSupportedClip(String path, boolean useCache){
        return getSupportedClip( path,  useCache, 0);
    }

    /**
     * This method only creates a media clip that is supported by the NexEditor&trade;&nbsp; SDK.
     *
     * <p>Example code:</p>
     *     {@code
    nexClip clip2= nexClip.getSupportedClip(m_soundfilepath.get(0));
    }
     * @param path The location where the media clip is saved.
     * @param useCache Set to <tt>TRUE</tt> to cache retrieved media information; otherwise set to <tt>FALSE</tt>.
     *                 This method allows to decide whether to cache retrieved clip's media information.
     *                 Once a clip's media information is cached, this method will operate faster when the same clip is called again.
     *                 It is recommended to set the parameter <tt>useCache</tt> to <tt>FALSE</tt> when a clip is for temporary use
     *                 (for example, when a panorama image is made with number of trailing images, these images will be deleted once a new panorama image is made).
     * @param  ableToHWResource  H/W codec resource limit value. (cf. width X height X fps X decoder count X (1/ endoer count) )
     * @return A <tt>nexClip</tt> instance if the media clip is supported by the NexEditor&trade;&nbsp;SDK.
     * @since version 1.7.1
     */
    public static nexClip getSupportedClip(String path, boolean useCache, int ableToHWResource){
        return getSupportedClip( path,  useCache, ableToHWResource, false);
    }
    /**
     * This method only creates a media clip that is supported by the NexEditor&trade;&nbsp; SDK.
     *
     * <p>Example code:</p>
     *     {@code
    nexClip clip2= nexClip.getSupportedClip(m_soundfilepath.get(0));
    }
     * @param path The location where the media clip is saved.
     * @param useCache Set to <tt>TRUE</tt> to cache retrieved media information; otherwise set to <tt>FALSE</tt>.
     *                 This method allows to decide whether to cache retrieved clip's media information.
     *                 Once a clip's media information is cached, this method will operate faster when the same clip is called again.
     *                 It is recommended to set the parameter <tt>useCache</tt> to <tt>FALSE</tt> when a clip is for temporary use
     *                 (for example, when a panorama image is made with number of trailing images, these images will be deleted once a new panorama image is made).
     * @param  ableToHWResource  H/W codec resource limit value. (cf. width X height X fps X decoder count X (1/ endoer count) )
     * @param  decoding Set to <tt>TRUE</tt> to get result that can be decode to bitmap; otherwise set to <tt>FALSE</tt> to get result without bitmap decoding.
     * @return A <tt>nexClip</tt> instance if the media clip is supported by the NexEditor&trade;&nbsp;SDK.
     * @since version 1.7.42
     */
    public static nexClip getSupportedClip(String path, boolean useCache, int ableToHWResource, boolean decoding){

        if(path == null)
            return null;
        
        if( path.startsWith("@solid:") ){
            return getSolidClip(path);
        }

        if( path.startsWith("@assetItem:") ){
            return new nexClip(path.substring(11),path);
        }

        String realPath = transCodingPath(path);

        if( realPath == null ){
            realPath = path;
        }

        if( realPath.startsWith("@assetItem:") ){
            return new nexClip(path,realPath);
        }

        MediaInfo Minfo = MediaInfo.getInfo(realPath, useCache);

        ClipInfo info = resolveClip(realPath, useCache, Minfo, decoding);
//        boolean create = true;
        if( info.mSuppertedResult == kCLIP_Supported ){
            if( info.mClipType == kCLIP_TYPE_IMAGE ) {
                return new nexClip(path,realPath, info,useCache,Minfo);
            } else if( info.mClipType == kCLIP_TYPE_VIDEO ) {
                if( ableToHWResource > 0  ){
                    int need = info.mWidth * info.mHeight *info.mFramesPerSecond;
                    if( need > ableToHWResource ){

                        if(EditorGlobal.PROJECT_NAME.equals("LGE")) {
                            if( info.mExistAudio) {
                                if (!info.mAudioCodecType.contains("aac")) {
                                    return null;
                                }
                            }
                            return new nexClip(path,realPath, info,useCache,Minfo);
                        }
                        return null;
                    }
                }
                return new nexClip(path,realPath, info,useCache,Minfo);
            } else if( info.mClipType == kCLIP_TYPE_AUDIO ) {
                return new nexClip(path,realPath, info,useCache,Minfo);
            }
        }

        if( info.mSuppertedResult == kClip_Supported_NeedResolutionTranscoding
                ||  info.mSuppertedResult == kClip_Supported_NeedFPSTranscoding){
            //TODO:
        }
        return null;
    }

    /**
     *
     * @param is
     * @param hashCode
     * @return
     * @since 2.0.14
     */
    public static nexClip getSupportedClip(InputStream is, int hashCode){
        int size;
        byte[] w = new byte[1024];
        FileOutputStream out = null;
        File outFile =  null;
        boolean ok = true;
        if( hashCode != 0 ) {
            String name = String.format(EditorGlobal.PrefixTempFile +"_%X.tmp", hashCode);
            outFile = new File(KineMasterSingleTon.getApplicationInstance().getApplicationContext().getFilesDir(), name);
        }else{
            try {
                outFile = File.createTempFile(EditorGlobal.PrefixTempFile, "tmp", KineMasterSingleTon.getApplicationInstance().getApplicationContext().getFilesDir());
            }catch (IOException e){
                ;
            }
        }

        if( outFile == null ){
            Log.d(TAG,"crateTempFile fail!");
            return null;
        }

        if( !outFile.exists() ) {
            try {
                out = new FileOutputStream(outFile);
                while (true) {
                    size = is.read(w);
                    if (size <= 0)
                        break;
                    out.write(w, 0, size);
                }
                out.close();
            }catch (FileNotFoundException e1){
                ok = false;
            }catch (IOException e2){
                e2.printStackTrace();
                ok = false;
                try {
                    out.close();
                } catch (IOException e) {
                    ;
                }
            }
        }
        if( ok ) {
            outFile.deleteOnExit();
            return getSupportedClip(outFile.getAbsolutePath(), false);
        }
        Log.d(TAG,"outFile="+outFile.getAbsolutePath()+" fail!");
        return null;
    }

    private static ClipInfo resolveClip(String path, boolean useCache, MediaInfo minfo, boolean decoding) {
        ClipInfo info = new ClipInfo();

        FileType fileType = FileType.fromFile(path);

        if (fileType!=null && fileType.isImage()) {
            if( !fileType.isSupportedFormat() ){
                info.mSuppertedResult = kClip_NotSupported;
                info.mClipType = kCLIP_TYPE_IMAGE;
                return info;
            }
            BitmapFactory.Options opts = new BitmapFactory.Options();
            if(decoding) {
                opts.inJustDecodeBounds = false;
                opts.inSampleSize = 16;
            }else{
                opts.inJustDecodeBounds = true;
            }
            Bitmap bm = BitmapFactory.decodeFile(path, opts);
            //TODO : exception no image?
            info.mWidth = opts.outWidth;
            info.mHeight = opts.outHeight;

            if(decoding && bm == null) {
                info.mClipType = kCLIP_TYPE_NONE;
                info.mSuppertedResult = kClip_NotSupported;
                Log.e(TAG,"error : Image file="+path+" decode fail!");
            }else if(info.mWidth <= 1 || info.mHeight <= 1 ){
                info.mClipType = kCLIP_TYPE_NONE;
                info.mSuppertedResult = kClip_NotSupported_ResolutionTooLow;
                Log.e(TAG,"error : Image file="+path+" decoded resolution is too low!");
            }else{
                info.mClipType = kCLIP_TYPE_IMAGE;
                info.mSuppertedResult = kCLIP_Supported;
            }

            String lcpath = path.toLowerCase(Locale.US);

            int orientation = ExifInterface.ORIENTATION_UNDEFINED;
            if( lcpath.endsWith(".jpeg") || lcpath.endsWith(".jpg") ) {
                try {
                    ExifInterface exif = new ExifInterface(path);
                    orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);
                } catch (IOException e) {
                    orientation = ExifInterface.ORIENTATION_UNDEFINED;
                }
            }

            switch( orientation ) {
                case ExifInterface.ORIENTATION_ROTATE_180:
                    info.mRotateDegreeInMeta = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_90:
                    info.mRotateDegreeInMeta = 90;
                    //info.mWidth = opts.outHeight;
                    //info.mHeight = opts.outWidth;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    info.mRotateDegreeInMeta = 270;
                    //info.mWidth = opts.outHeight;
                    //info.mHeight = opts.outWidth;
                    break;
                default:
                    info.mRotateDegreeInMeta = 0;
                    break;
            }

        } else {
            if(minfo==null) {
                minfo = MediaInfo.getInfo(path, useCache);
            }
            int mediaSupportType = minfo.getMediaSupportType();
            // wapping.
            if(mediaSupportType == -1/*MediaSupportType.Unknown*/) {
                info.mSuppertedResult = kClip_Supported_Unknown;
            } else if(mediaSupportType == -2/*MediaSupportType.NotSupported_AudioCodec*/) {
                info.mSuppertedResult = kClip_NotSupported_AudioCodec;
            } else if(mediaSupportType == -3/*MediaSupportType.NotSupported_AudioProfile*/) {
                info.mSuppertedResult = kClip_NotSupported_AudioProfile;
            } else if(mediaSupportType == -4/*MediaSupportType.NotSupported_Container*/) {
                info.mSuppertedResult = kClip_NotSupported_Container;
            } else if(mediaSupportType == -5/*MediaSupportType.NotSupported_ResolutionTooHigh*/) {
                info.mSuppertedResult = kClip_NotSupported_ResolutionTooHigh;
            } else if(mediaSupportType == -6/*MediaSupportType.NotSupported_DurationTooShort*/) {
                info.mSuppertedResult = kClip_NotSupported_DurationTooShort;
            } else if(mediaSupportType == -7/*MediaSupportType.NotSupported_ResolutionTooLow*/) {
                info.mSuppertedResult = kClip_NotSupported_ResolutionTooLow;
            } else if(mediaSupportType == -8/*MediaSupportType.NotSupported_VideoProfile*/) {
                info.mSuppertedResult = kClip_NotSupported_VideoProfile;
            } else if(mediaSupportType == -9/*MediaSupportType.NotSupported_VideoCodec*/) {
                info.mSuppertedResult = kClip_NotSupported_VideoCodec;
            } else if(mediaSupportType == -10/*MediaSupportType.NotSupported_VideoFPS*/) {
                info.mSuppertedResult = kClip_NotSupported_VideoFPS;
            } else if(mediaSupportType == -11/*MediaSupportType.NotSupported_VideoLevel*/) {
                info.mSuppertedResult = kClip_NotSupported_VideoLevel;
            } else if(mediaSupportType == -12/*MediaSupportType.NotSupported*/) {
                info.mSuppertedResult = kClip_NotSupported;
            } else {
                info.mSuppertedResult = kCLIP_Supported;
            }

            if (info.mExistVideo && info.mSuppertedResult == kClip_Supported_Unknown) {
                NexEditor editor = EditorGlobal.getEditor();
                if (editor != null) {
                    NexVisualClipChecker checker = editor.getVisualClipChecker();
                    if (checker != null) {
                        int iSupport = checker.checkSupportedClip(
                                minfo.getVideoH264Profile(),
                                minfo.getVideoH264Level(),
                                minfo.getVideoWidth(), minfo.getVideoHeight(), minfo.getFPS(),
                                minfo.getVideoBitrate(),
                                minfo.getAudioSamplingRate(),
                                minfo.getAudioChannels());
                        switch (iSupport) {
                            case 0: // Supported
                                info.mSuppertedResult = kCLIP_Supported;
                                break;
                            case 1: // Not supported but supported after resolution transcoding.
                                info.mSuppertedResult = kClip_Supported_NeedResolutionTranscoding;
                                break;
                            case 2: // Not supported but supported after FPS transcoding.
                                info.mSuppertedResult = kClip_Supported_NeedFPSTranscoding;
                                break;
                            case 3: // Not supported (profile not supported)
                                info.mSuppertedResult = kClip_NotSupported_VideoProfile;
                                break;
                            case 4: // Not supported (resolution over HW codec max resolution)
                                info.mSuppertedResult = kClip_NotSupported_ResolutionTooHigh;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            info.mRotateDegreeInMeta = minfo.getOrientation();
            info.mH264Level = minfo.getVideoH264Level();
            info.mH264Profile = minfo.getVideoH264Profile();
            info.mFramesPerSecond = minfo.getFPS();
            info.mFramesPerSecondFloat = minfo.getFPSFloat();
            info.mExistAudio = minfo.hasAudio();
            info.mExistVideo = minfo.hasVideo();
            info.mTotalTime = minfo.getDuration();
            info.mVideoTotalTime = minfo.getVideoDuration();
            info.mAudioTotalTime = minfo.getAudioDuration();
            info.mSeekPointCount = minfo.getSeekPointCount();
            info.mVideoRenderMode = minfo.getVideoRenderType();
            info.mVideoHDRType = minfo.getVideoHDRType();

            if (info.mExistVideo) {
                info.mVideoCodecType = nexCodecType.fromValue(minfo.getVideoCodecType()).getType();
            }else{
                info.mVideoCodecType = "none";
            }

            if( info.mExistAudio ){
                info.mAudioCodecType = nexCodecType.fromValue(minfo.getAudioCodecType()).getType();
            }else{
                info.mAudioCodecType = "none";
            }

            if (info.mExistVideo) {

                info.mWidth = minfo.getVideoWidth();
                info.mHeight = minfo.getVideoHeight();
                info.mClipType = kCLIP_TYPE_VIDEO;
                info.mVideoBitrate = minfo.getVideoBitrate();

                int diff = info.mAudioTotalTime - info.mVideoTotalTime;
                if( diff > 200 || diff < -200 ){
                    Log.w(TAG,"AudioTotalTime="+info.mAudioTotalTime+", VideoTotalTime="+info.mVideoTotalTime+", diff="+diff);
                }

                if( info.mTotalTime == 0 ){
                    info.mTotalTime = info.mVideoTotalTime;
                }

                if( diff > 0 ){
                    Log.w(TAG,"different video and audio TotalTime. TotalTime("+info.mTotalTime+") was set videoTotalTime="+info.mVideoTotalTime);
                    info.mTotalTime =  info.mVideoTotalTime;
                }

                if( info.mExistAudio ){
                    info.mAudioBitrate = minfo.getAudioBitrate();
                }
            } else if(info.mExistAudio ) {
                info.mClipType = kCLIP_TYPE_AUDIO;
                info.mAudioBitrate = minfo.getAudioBitrate();
            }

            if( info.mTotalTime < 0 ){
                info.mSuppertedResult = kClip_NotSupported_DurationTooShort;
            }
        }
        return info;
    }

    /** 
     * This method gets the FPS (Frames Per Second) value of a clip.
     *  
     * <p>Example code:</p>
     *     {@code mFps = (TextView)findViewById(R.id.textView5);
            mFps.setText("Clip FPS : "+mClip.getFramesPerSecond());
           }
     * @return The value of FPS, as an <tt>integer</tt> . 
     * @since version 1.1.0
     */
    public int getFramesPerSecond(){
        return mInfo.mFramesPerSecond;
    }
    public float getFramesPerSecondFloat(){
        return mInfo.mFramesPerSecondFloat;
    }

    /**
     * This method checks if there is any audio file included in the media content.
     *
     * <p>Example code:</p>
     *     <b>class sample code</b>
     *     {@code
                if(mClip.hasAudio() && mClip.hasVideo())
                mExistaudiovideo.setText("Clip exist Audio and Video");
           }
     * @return <tt>TRUE</tt> if there is any audio file, otherwise <tt>FALSE</tt>.
     * 
     * @see #hasVideo()
     * @since version 1.0.0
     * 
     */
    public boolean hasAudio(){
        return mInfo.mExistAudio;
    }

    /**
     * This method checks if there is any video file included in the media content.
     *
     * <p>Example code:</p>
     *     {@code if(mClip.hasAudio() && mClip.hasVideo())
                mExistaudiovideo.setText("Clip exist Audio and Video");
            }
     * @return <tt>TRUE</tt> if there is any video file, otherwise <tt>FALSE</tt>.
     * 
     * @see #hasAudio()
     * @since version 1.0.0
     * 
     */
    public boolean hasVideo(){
        return mInfo.mExistVideo;
    }

    /**
     * This method gets the path of the current clip.
     *
     * <p>Example code:</p>
     *     {@code
                if (project.getClip(j, false).getPath() != clip.getPath())
           }
     * @return The location of the clip saved.
     * @since version 1.0.0
     */
    public String getPath(){
        return mPath;
    }

    String getRealPath(){
        if( mTransCodingPath != null){
            return mTransCodingPath;
        }
        return mPath;
    }

    /**
     * This method gets the type of a clip.
     *
     * <p>Example code:</p>
     *     {@code
         if( mClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                        mClip.loadVideoClipThumbnails( new nexClip.OnLoadVideoClipThumbnailListener(){
                        @Override
                            public void onLoadThumbnailResult(int event) {}
                        }
                    }}
     * @return One of:
     * <ul>
     * <li>{@link #kCLIP_TYPE_VIDEO} = {@value #kCLIP_TYPE_VIDEO} : Video
     * <li>{@link #kCLIP_TYPE_AUDIO} = {@value #kCLIP_TYPE_AUDIO} : Audio
     * <li>{@link #kCLIP_TYPE_IMAGE} = {@value #kCLIP_TYPE_IMAGE} : Image
     * <li>{@link #kCLIP_TYPE_NONE} = {@value #kCLIP_TYPE_NONE} : None
     * </ul>
     * @since version 1.0.0
     */
    public int getClipType() {
        return mInfo.mClipType;
    }

    /**
     * This method gets the playback time if the clip is either an audio clip or a video clip.
     *
     * <p>Example code:</p>
     *     {@code if( clip != null ){
                clip.setImageClipDuration(clip2.getTotalTime());
            }
            }
     * @return Playback time of the clip in <tt> msec</tt> (milliseconds).
     * @since version 1.0.0
     */
    public int getTotalTime() {
        return mInfo.mTotalTime;
    }

    /**
     * This method gets the vertical resolution information if the clip is either an image clip or a video clip.
     *
     * <p>Example code:</p>
     *     {@code
                mResolution.setText("Width="+mClip.getWidth()+", Height="+mClip.getHeight());
           }
    }
     * @return The vertical resolution of the clip, in <tt>pixels</tt>.
     * 
     * @see #getHeight()
     * @since version 1.0.0
     * 
     */
    public int getWidth() {
        return mInfo.mWidth;
    }

    /**
     * This method gets the horizontal resolution information if the clip is either an image clip or a video clip.
     *
     * <p>Example code:</p>
     *     {@code
                mResolution.setText("Width="+mClip.getWidth()+", Height="+mClip.getHeight());
           }
    }
     * @return The horizontal resolution of the clip, in <tt>pixels</tt>.
     * 
     * @see #getWidth()
     * @since version 1.0.0
     * 
     */
    public int getHeight() {
        return mInfo.mHeight;
    }

    /**
     * Preparing...
     * @return
     * @since 1.5.30
     */
    public int getDisplayWidth() {
        return mInfo.mDisplayWidth;
    }

    /**
     * Preparing...
     * @return
     * @since 1.5.30
     */
    public int getDisplayHeight() {
        return mInfo.mDisplayHeight;
    }

    /**
     * This methods checks whether a clip is supported by the NexEditor&trade;&nbsp;SDK.
     *
     * <p>Example code:</p>
     *     {@code     switch (mClip.getSupportedResult())
                {
                    case 0 : //mClip.kCLIP_Supported :
                    mSupportedresult.setText("Clip is supported");
                    break;
                }
    }
     * @return  One of:
     * <ul>
     * <li>{@link #kClip_NotSupported} = {@value #kClip_NotSupported}
     * <li>{@link #kClip_NotSupported_AudioCodec} = {@value #kClip_NotSupported_AudioCodec}
     * <li>{@link #kClip_NotSupported_AudioProfile} = {@value #kClip_NotSupported_AudioProfile}
     * <li>{@link #kClip_NotSupported_Container} = {@value #kClip_NotSupported_Container}
     * <li>{@link #kClip_NotSupported_DurationTooShort} = {@value #kClip_NotSupported_DurationTooShort}
     * <li>{@link #kClip_NotSupported_ResolutionTooHigh} = {@value #kClip_NotSupported_ResolutionTooHigh}
     * <li>{@link #kClip_NotSupported_ResolutionTooLow} = {@value #kClip_NotSupported_ResolutionTooLow}
     * <li>{@link #kClip_NotSupported_VideoCodec} = {@value #kClip_NotSupported_VideoCodec}
     * <li>{@link #kClip_NotSupported_VideoFPS} = {@value #kClip_NotSupported_VideoFPS}
     * <li>{@link #kClip_NotSupported_VideoLevel} = {@value #kClip_NotSupported_VideoLevel}
     * <li>{@link #kClip_NotSupported_VideoProfile} = {@value #kClip_NotSupported_VideoProfile}
     * <li>{@link #kClip_Supported_NeedFPSTranscoding} = {@value #kClip_Supported_NeedFPSTranscoding}
     * <li>{@link #kClip_Supported_NeedResolutionTranscoding} = {@value #kClip_Supported_NeedResolutionTranscoding}
     * <li>{@link #kCLIP_Supported} = {@value #kCLIP_Supported}
     * <li>{@link #kClip_Supported_Unknown} = {@value #kClip_Supported_Unknown}
     * </ul>
     * @since version 1.0.0
     */
    public int getSupportedResult(){
        return mInfo.mSuppertedResult;
    }

    /**
     * This method gets the profile value of a video file if the video file format is H.264. 
     *
     * <p>Example code:</p>
     *     {@code
                mSupportedresult.setText("AVC Profile:"+mClip.getAVCProfile() + ", Level:"+mClip.getAVCLevel());
           }
     * @return The profile value of the video file. 
     * @since version 1.3.4
     */
    public int getAVCProfile(){
        return mInfo.mH264Profile;
    }

    /**
     * This method gets the level of a video file if the video file format is H.264.
     *
     * <p>Example code:</p>
     *     {@code
                mSupportedresult.setText("AVC Profile:"+mClip.getAVCProfile() + ", Level:"+mClip.getAVCLevel());
           }
     * @return The level of the video file. 
     * @since version 1.3.4
     */
    public int getAVCLevel(){
        return mInfo.mH264Level;
    }

    /**
     * This method gets the orient angle of the metadata of a file if the file format is MP4. 
     *
     * <p>Example code:</p>
     *     {@code
                mProject.getClip(i, true).setRotateDegree(clip.getRotateInMeta());
           }
     * @return The orient angle of the metadata. 
     * @since version 1.3.4
     */
    public int getRotateInMeta(){
        return mInfo.mRotateDegreeInMeta;
    }

    /**
     * This method gets the average bitrate value of the video file of a clip.
     * The average bitrate value should follow what's specified on the header file of the video file even if the specified value is 0. 
     * 
     * <p>Example code:</p>
     *     {@code
                mEngine.export(mFile.getAbsolutePath(), clip.getWidth(), clip.getHeight(), clip.getVideoBitrate(), Long.MAX_VALUE, rotateInMeta);
           }
     * @return Average bitrate value if the media content is a video file; otherwise 0. 
     * @since version 1.3.43
     */
    public int getVideoBitrate() {
        return mInfo.mVideoBitrate;
    }

    /**
     * This method gets the average bitrate value of the audio file of a clip. 
     * The average bitrate value should follow what's specified on the header file of the audio file even if the specified value is 0. 
     * 
     * <p>Example code:</p>
     *     {@code     mAudioBitrate = mClip.getAudioBitrate();
                Log.d("ClipInfo", "Video bitrate ="+mVideoBitrate + "Audio bitrate ="+mAudioBitrate);
           }
     * @return Average bitrate value if the media content is an audio file; otherwise 0. 
     * @since version 1.3.43
     */
    public int getAudioBitrate() {
        return mInfo.mAudioBitrate;
    }

    /**
     * This method gets the duration of a video in <tt> msec</tt> (milliseconds).
     * The duration value should follow what's specified on the header file of the video file even if the specified value is 0.
     *
     * <p>Example code:</p>
     *     {@code     mVideoDuration = mClip.getVideoDuration();
                Log.d("ClipInfo", "Video duration ="+mVideoDuration + "Audio duration ="+mAudioDuration);
            }
     * @return  The duration of a video file in <tt>msec</tt> (milliseconds). 
     * @since version 1.3.43
     */
    public int getVideoDuration() { return mInfo.mVideoTotalTime; }

    /**
     * This method gets the duration of an audio file in <tt>msec</tt> (milliseconds.) 
     * The duration value should follow what's specified on the header file of the audio file even if the specified value is 0.
     *
     * <p>Example code:</p>
     *     {@code     mAudioDuration = mClip.getAudioDuration();
                Log.d("ClipInfo", "Video duration ="+mVideoDuration + "Audio duration ="+mAudioDuration);
            }
     * @return The duration of an audio file in <tt>msec</tt> (milliseconds) 
     * @since version 1.3.43
     */
    public int getAudioDuration() { return mInfo.mAudioTotalTime; }

    /**
     * This method finds out the number of I-frames of the media content.
     * @return The number of IFrames.
     * @see #getSeekPointInterval()
     * @since 1.5.42
     */
    public int getSeekPointCount() {
        return mInfo.mSeekPointCount;
    }

    /**
     * This method finds out the average interval of IFrames of the media content.
     * @return The average interval of I-frames in <tt>msec</tt> (milliseconds).
     * @see #getSeekPointCount()
     * @since 1.5.42
     */
    public int getSeekPointInterval() {
        if( mInfo.mSeekPointCount == 0 )
            return mInfo.mVideoTotalTime;
        return mInfo.mVideoTotalTime/mInfo.mSeekPointCount;
    }

    /**
     * @brief This method finds out whether or not a clip is 360 video or not.
     * @return kCLIP_VIDEORENDERMODE_360VIDE - 360 video; kCLIP_VIDEORENDERMODE_NORMAL - normal video.
     * @since 1.7.1
     */
    public int getVideoRenderMode(){
        return mInfo.mVideoRenderMode;
    }

    /**
     * @brief This method returns the video HDR type.
     * @return {@link #HDR_TYPE_10PQ} or {@link #HDR_TYPE_10HLG} is HDR content. 0 is normal SDR content
     * @since 2.0.0
     */
    public int getVideoHDRType(){
        return mInfo.mVideoHDRType;
    }

    /**
     * @brief This method returns the video codec type as a \c String.
     * @return A value in the form of "video/XXXX".
     * @since 1.7.1
     */
    public String getVideoCodecType() {
        return mInfo.mVideoCodecType;
    }

    /**
     * @brief This method returns the audio codec type as a \c String.
     * @return A value in the form of "audio/XXXX". 
     * @since 1.7.1
     */
    public String getAudioCodecType() {
        return mInfo.mAudioCodecType;
    }

    /**
     * @brief This method returns the clip start time
     * @return The number of millisecond
     * @since 2.0.15.0
     */
    public int getmStartTime() {
        return mStartTime;
    }

    public void setmStartTime(int startTime) {
        mStartTime = startTime;
    }

    /**
     * @brief This method returns the clip end time
     * @return The number of millisecond
     * @since 2.0.15.0
     */
    public int getmEndTime() {
        return mEndTime;
    }

    public void setmEndTime(int endTime) {
        mEndTime = endTime;
    }

    protected final boolean getAttachmentState(){
        return mProjectAttachment;
    }

    protected final void setAttachmentState( boolean set , nexObserver project) {
        mObserver = project;
        mProjectAttachment = set;
        if( !mProjectAttachment ){
            if( mTransitionEffect != null ){
                mTransitionEffect.setObserver(null);
            }
        }
    }

    /**
     * This method gets an instance of a clip effect to set on a clip that is currently added to a project.
     * If this method is called and there are currently no clips added to a project, <tt>ProjectNotAttachedException</tt> will occur.
     *
     * <p>Example code:</p>
     *     {@code
                mProject.getClip(i, true).getClipEffect().setEffectShowTime(0,clip.getTotalTime()); //set full time effect
            }
     * @return The <tt>nexClipEffect</tt> instance.
     * @throws ProjectNotAttachedException
     * @since version 1.0.0
     */
    public nexClipEffect getClipEffect(){
        if (mProjectAttachment) {
            if (mClipEffect == null) {
                mClipEffect = new nexClipEffect();
            }
            return mClipEffect;
        }
        throw new ProjectNotAttachedException();
    }

    /**
     * This method gets an instance of a transition effect to set on a clip that is currently added to a project.
     * 
     * If this method is called and there are currently no clips added to a project, <tt>ProjectNotAttachedException</tt> will occur.
     *
     * <p>Example code:</p>
     *     {@code
                clip.getTransitionEffect().setTransitionEffect("com.nexstreaming.unkownid");
            }
     * @return The <tt>nexTransitionEffect</tt> instance.
     * @throws ProjectNotAttachedException
     * @since version 1.0.0
     */
    public nexTransitionEffect getTransitionEffect(){
        if( mProjectAttachment ) {
            if( mTransitionEffect == null ) {
                mTransitionEffect = new nexTransitionEffect(mObserver);
            }
            return mTransitionEffect;
        }
        throw new ProjectNotAttachedException();
    }

    /**
     * This method gets the editing related properties for a clip, except for trimming or speed control, if the clip is a video clip.
     * 
     * <p>Example code:</p>
     *     {@code
                mProject.getClip(0, true).getVideoClipEdit().setTrim(0, trim);
            }
     * @return The <tt>nexVideoClipEdit</tt> that has been set for the clip.
     * @throws ProjectNotAttachedException
     * @since version 1.0.0
     */
    public nexVideoClipEdit getVideoClipEdit(){
        if( mProjectAttachment ) {
            if ( mInfo.mClipType != kCLIP_TYPE_VIDEO ){
                return null;
            }

            if( mVideoEdit == null ){
                mVideoEdit = nexVideoClipEdit.getnexVideoClipEdit(this);
            }
            return mVideoEdit;
        }
        throw new ProjectNotAttachedException();
    }
/* 1.0.1
    public nexColorEffect getColorEffect(){
        if ( mInfo.mClipType == kCLIP_TYPE_IMAGE || mInfo.mClipType == kCLIP_TYPE_VIDEO){
            if( mColorEffect ==null ){
                mColorEffect = nexColorEffect.NONE;
            }
        }
        return mColorEffect;
    }
*/
    /**
     * This method sets the duration of an image clip to be played within the project.
     *
     * <p>Example code:</p>
     *     {@code
                clip.setImageClipDuration(clip2.getTotalTime());
            }
     * @param ms The duration an image clip should be displayed, in <tt>msec</tt> (milliseconds).
     * @see #getImageClipDuration()
     * @since version 1.0.0
     */
    public void setImageClipDuration(int ms){
        if( mDuration != ms) {
            setProjectUpdateSignal(false);
        }

        mDuration = ms;
    }

    /**
     * This method gets the duration of an image clip to be played within the project.
     *
     * <p>Example code:</p>
     *     {@code
                visualClip.mTotalTime = clipItem.getImageClipDuration();
    }
     * @return The duration of an image clip in <tt>msec</tt> (milliseconds).
     * @see #setImageClipDuration(int)
     * @since version  1.0.0
     */
    public int getImageClipDuration(){
        return mDuration;
    }

    /**
     * This method gets the start time of a clip when playing a project.
     * This can be considered the location of a clip in a project's timeline.
     * 
     * <p>Example code:</p>
     *     {@code
                if (project.getClip(i, false).getProjectStartTime() <timeline1_endtime) {
                    gototimeline2 = true;
                    break;
                }
            }
     * @return The clip start time in <tt>msec</tt> (milliseconds).
     * 
     * @see #getProjectEndTime()
     * @since version 1.0.0
     * 
     */
    public int getProjectStartTime(){
        if( mProjectAttachment ) {
            return mStartTime;
        }
        return 0;
    }

    /**
     * This method gets the end time of a clip when playing a project.
     * This can be considered the location of a clip in a project's timeline.
     * 
     * <p>Example code:</p>
     *     {@code
                int end = mProject.getClip(i, true).getProjectEndTime();
            }
     * @return The clip end time in <tt>msec</tt> (milliseconds).
     * 
     * @see #getProjectStartTime()
     * @since version 1.0.0
     * 
     */
    public int getProjectEndTime(){
        if( mProjectAttachment ) {
            return mEndTime;
        }
        return 0;
    }

    /**
     * This method can turn the audio on and off while a clip is being played within a project.
     * 
     * <p>Example code:</p>
     *     {@code
                clip.setAudioOnOff(false);
            }
     * @param on  Whether or not audio should be on. Set to <tt>TRUE</tt> to turn on the audio, otherwise set to <tt>FALSE</tt> to turn the audio off.
     * 
     * @see #getAudioOnOff()
     * @since version 1.0.0
     * 
     */
    public void setAudioOnOff(boolean on){
        mAudioOnOff = on;
        setProjectUpdateSignal(true);
    }

    /**
     * This method checks whether a clip's audio is on or off.
     *
     * <p>Example code:</p>
     *     {@code
                visualClip.mAudioOnOff = item.getAudioOnOff()?1:0;
           }
     * @return <tt>TRUE</tt> if the audio is on; otherwise <tt>FALSE</tt> if the audio is off.
     * 
     * @see #setAudioOnOff(boolean)
     * @since version 1.0.0
     * 
     */
    public boolean getAudioOnOff(){
        return mAudioOnOff;
    }

    /**
     * This method registers a thumbnail image.
     * 
     * <p>Example code:</p>
     *     {@code     bmlist.add(mClip.getMainThumbnail(240f ,getBaseContext().getResources().getDisplayMetrics().density));
                mClip.setMainThumbnail(bmlist.get(0));
            }
     * @param thumb An instance of the thumbnail bitmap to be registered.
     * 
     * @see #getMainThumbnail(float, float)
     * @since version  1.0.0
     * 
     */
    public void setMainThumbnail(Bitmap thumb ) {
        if( mSingleThumbnail != null ) {
            mSingleThumbnail.clear();
        }
        mSingleThumbnail = new WeakReference<Bitmap>(thumb);
        //mSingleThumbnail = thumb;
    }

    /**
     * This method gets the main thumbnail image of a clip.
     * Run <tt>loadVideoClipThumbnails()</tt> to get a thumbnail image from a video clip.
     * 
     * To use this API to get a video clip thumbnail image, <tt>loadVideoClipThumbnails()</tt> should be called successfully first,
     * which is indicated by the event:
     * {@link #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)}
     * <p>Example code :</p>
     {@code mClip = new nexClip(m_listfilepath.get(0));
         Bitmap bm = mClip.getMainThumbnail(240f ,getBaseContext().getResources().getDisplayMetrics().density);
         mIV = (ImageView)findViewById(R.id.imageView);
         mIV.setImageBitmap(bm);
     }
     * @param height  The height of the thumbnail image.
     * @param scaleDIPtoPX The value of DIP.
     * @return The thumbnail image for the clip as a bitmap.
     * 
     * @see #getVideoClipTimeLineOfThumbnail()
     * @see #setMainThumbnail(android.graphics.Bitmap)
     * @see #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)
     * @since version 1.0.0
     * 
     */
    public Bitmap getMainThumbnail(float height, float scaleDIPtoPX){
        if( mInfo.mClipType ==  kCLIP_TYPE_VIDEO ) {
            if (mThumbnails != null) {
                return mThumbnails.getThumbnailBitmap(0, mInfo.mTotalTime / 2, false, false);
            }
            return null;
        }else {
            Bitmap bm = null;
            if( mSingleThumbnail != null ){
                bm = mSingleThumbnail.get();
            }
            if( bm == null ){
                bm = makeThumbnail(height, scaleDIPtoPX);
                if( bm != null ) {
                    mSingleThumbnail = new WeakReference<Bitmap>(bm);
                }
            }
            return bm;
        }
    }

    /** 
     * This method gets a thumbnail image's extracted time in sequence.
     *  
     * To use this API, <tt>loadVideoClipThumbnails()</tt> should first be called successfully, which is indicated by the event: 
     * {@link #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)}
     * In the case that the clip is a video, a maximum of 50 thumbnail images can be saved.
     * @return The extracted time of a thumbnail image in sequence as an <tt>integer</tt>, in <tt>msec</tt> (milliseconds).  
     * 
     * @throws ClipIsNotVideoException
     * <p>Example code :</p>
     *     {@code int timeLine[] = mClip.getVideoClipTimeLineOfThumbnail();
     *      for( int i = 0 ; i < timeLine.length ; i++){
     *          Bitmap bm2 = mClip.getVideoClipTimeLineThumbnail(0,timeLine[i],false,false);
     *          mIV.setImageBitmap(bm2);
     *      }
     * }
     *
     * @see #getVideoClipTimeLineThumbnail(int, int, boolean, boolean)
     * @see #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)
     * @since version 1.0.0
     * 
     */
    public int[] getVideoClipTimeLineOfThumbnail(){
        if( mInfo.mClipType ==  kCLIP_TYPE_VIDEO ) {
            if (mThumbnails != null) {
                return mThumbnails.getThumbnailTimeLine();
            }
        }
        throw new ClipIsNotVideoException();
    }

    /**
     * This method gets a thumbnail image, that's been extracted from a video clip, matching the input parameter value. 
     * 
     * To use this API, <tt>loadVideoClipThumbnails()</tt> should be called successfully first, which is indicated by the event: 
     * {@link #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)}
     * In the case of a video clip, a maximum of 50 thumbnails can be saved.  One thumbnail image will be extracted depending on the parameter values input.
     * 
     * @param rotation Input the rotation value of a thumbnail to be printed, in degrees. (only values accepted: 0, 90, 180, 270 )
     * @param time  The duration of a thumbnail to get.
     * @param fliph Set to <tt>TRUE</tt> to make a thumbnail based on the horizontal resolution.
     * @param flipv Set to <tt>TRUE</tt> to make a thumbnail based on the vertical resolution.
     * 
     * @return Bitmap of a thumbnail.
     * @since version 1.0.0
     * @throws ClipIsNotVideoException
     * <p>Example code :</p>
     *     {@code int timeLine[] = mClip.getVideoClipTimeLineOfThumbnail();
     *      for( int i = 0 ; i < timeLine.length ; i++){
     *          Bitmap bm2 = mClip.getVideoClipTimeLineThumbnail(0,timeLine[i],false,false);
     *          mIV.setImageBitmap(bm2);
     *      }
     * }
     * 
     * @see #getVideoClipTimeLineOfThumbnail()
     * @see #loadVideoClipThumbnails(OnLoadVideoClipThumbnailListener)
     * @since version 1.0.0
     * 
     */
    public Bitmap getVideoClipTimeLineThumbnail( int rotation, int time, boolean fliph, boolean flipv ){
        if( mInfo.mClipType ==  kCLIP_TYPE_VIDEO ) {
            if (mThumbnails != null) {
                return mThumbnails.getThumbnailBitmap(rotation, time, fliph, flipv);
            }
        }

       throw new ClipIsNotVideoException();
    }

   /**
     * This class operates asynchronously and defines the events sent when <tt>loadVideoClipThumbnails()</tt> takes too much time getting thumbnails from a clip.
     * The event, {@link #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)}, indicates
     * when a thumbnail has been loaded successfully.
     * 
     * @see #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)
     * @since version 1.0.0
     */
    public static abstract class OnLoadVideoClipThumbnailListener {

        /**
         * This event indicates that the thumbnail was successfully retrieved.
         * 
         * @since version 1.3.0
         */
        public static int kEvent_Ok = 0;

        /**
         * This event indicates that the thumbnail has already retrieved.
         * 
         * @since version 1.3.0
         */
        public static int kEvent_loadCompleted = 1;

        /**
         * This event indicates that the thumbnail is currently being retrieved. 
         * 
         * @since version 1.3.0
         */
        public static int kEvent_Running = 2;

        /**
         * This event indicates that the thumbnail retrieving operation was unsuccessful due to other engine operation. Try again later. 
         * 
         * @since version 1.3.0
         */
        public static int kEvent_mustRetry = 3;

        /**
         * This event indicates that thumbnail retrieving has failed. 
         * 
         * @since version 1.3.0
         */
        public static int kEvent_loadFail = -1;

        /**
         * This event indicates that a new thumbnail task couldn't be generated. 
         *
         * @since version 1.3.0
         */
        public static int kEvent_systemError = -2;


        /**
         * This event is called when <tt>loadVideoClipThumbnails()</tt> operation is complete.
         * @param event Set to 0 if works normally, otherwise set to -1.
         * @see #loadVideoClipThumbnails(com.nexstreaming.nexeditorsdk.nexClip.OnLoadVideoClipThumbnailListener)
         * @since version 1.0.0
         */
        public abstract void onLoadThumbnailResult( int event );
    }


    /** 
     * This method gets a thumbnail image and caches it. 
     * Since this method operates asynchronously, its success can be checked with <tt>OnGetThumbnailListener</tt>.
     * The thumbnail cannot be retrieved when <tt>nexEngine</tt> is in the <tt>play</tt> state.
     * 
     * If the clip is not a type of video, it returns -1.
     * 
     <p> Example code :</p>
     *     {@code 
        mClip = new nexClip(m_listfilepath.get(0));
        mIV = (ImageView)findViewById(R.id.imageView);

        if( mClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            mClip.loadVideoClipThumbnails( new nexClip.OnLoadVideoClipThumbnailListener(){
                @Override
                public void onLoadThumbnailResult(int event) {
                    if( event == OnLoadVideoClipThumbnailListener.kEvent_Ok ) {
                        Bitmap bm = mClip.getMainThumbnail(240f, getBaseContext().getResources().getDisplayMetrics().density);
                        mIV.setImageBitmap(bm);
                        int timeLine[] = mClip.getVideoClipTimeLineOfThumbnail();
                        for( int i = 0 ; i < timeLine.length ; i++){
                            Bitmap bm2 = mClip.getVideoClipTimeLineThumbnail(0,timeLine[i],false,false);
                            mIV.setImageBitmap(bm2);
                        }
                    }
                }
            }
             );
        }else{
            ....
        }

    }
     * @param listener OnLoadVideoClipThumbnailListener
     * @return -1 if the clip is not a video clip, otherwise 0.
     * 
     * @see #getVideoClipTimeLineOfThumbnail()
     * @see #getMainThumbnail(float, float)
     * @see #getVideoClipTimeLineThumbnail(int, int, boolean, boolean)
     * @since version 1.0.0
     * 
     * 
     */
    public int loadVideoClipThumbnails( final OnLoadVideoClipThumbnailListener listener ){
        if( mInfo.mClipType !=  kCLIP_TYPE_VIDEO ) {
            return -1;
        }

        if( mThumbnails==null && !m_gettingThumbnails && !m_getThumbnailsFailed ) {
            MediaInfo mediaInfo = getMediaInfo();
            if( mediaInfo ==null ){
                if(listener!=null) listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_systemError);
                return OnLoadVideoClipThumbnailListener.kEvent_systemError;
            }
            m_gettingThumbnails = true;
            mediaInfo.getThumbnails().onResultAvailable(new OnResultAvailableListener<Thumbnails>() {

                @Override
                public void onResultAvailable(ResultTask<Thumbnails> task, Event event, Thumbnails result) {
                    m_gettingThumbnails = false;
                    mThumbnails = result;
                    if(listener!=null) listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_Ok);
                }
            }).onFailure(new OnFailListener() {
                @Override
                public void onFail(Task t, Event e, TaskError reason) {
                    // FOR Thumbnail not load issue.
                    m_gettingThumbnails = false;
                    if (reason != NexEditor.ErrorCode.INPROGRESS_GETCLIPINFO)
                        m_getThumbnailsFailed = true;
                    if(listener!=null) {
                        if( m_getThumbnailsFailed )
                            listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_loadFail);
                        else
                            listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_mustRetry);
                    }
                }
            });
        }else{
            if( m_gettingThumbnails ) {
                if (listener != null) listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_Running);
                return OnLoadVideoClipThumbnailListener.kEvent_Running;
            }

            if( m_getThumbnailsFailed ){
                if(listener!=null) listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_loadFail);
                return OnLoadVideoClipThumbnailListener.kEvent_loadFail;
            }

            if( mThumbnails != null){
                if(listener!=null) listener.onLoadThumbnailResult(OnLoadVideoClipThumbnailListener.kEvent_loadCompleted);
            }
        }
        return 0;
    }

    /**
     * This class is a listener used with {@link #onGetDetailThumbnailResult(int, android.graphics.Bitmap, int, int, int)}.
     *
     * @see #onGetDetailThumbnailResult(int, android.graphics.Bitmap, int, int, int)
     * @since version 1.3.0
     */
    public static abstract class OnGetVideoClipDetailThumbnailsListener {
        public static int kEvent_Ok = 0;
        public static int kEvent_Completed = 1;
        public static int kEvent_Fail = -1;
        public static int kEvent_systemError = -2;
        public static int kEvent_UserCancel = -3;


  
        /**
         * This method returns whether retrieving process of <tt>detailThumbnail</tt> was successful or not. 
         * 
         * @param event      A <tt>kEvents</tt> of the class <tt>OnGetVideoClipDetailThumbnailsListener</tt> to tell the result of <tt>detailThumbnail</tt> retrieving.
         * @param bm         Retrieved thumbnails in a bitmap format.
         * @param index      The index of retrieved thumbnail as <tt> integer</tt>. 
         * @param totalCount The total number of retrieved thumbnails as <tt> integer</tt>. 
         * @param timestamp  The timestamps of each of retrieved thumbnails as <tt> integer</tt>. 
         * 
         * @since version 1.3.0
         */
        public abstract void onGetDetailThumbnailResult( int event , Bitmap bm, int index, int totalCount, int timestamp );
    }

    /**
     * @deprecated For internal use only. Please do not use.
     * @since version 1.3.43
     */
    @Deprecated
    public static abstract class OnGetVideoClipIDR2YOnlyThumbnailsListener {
        /**
         * This event indicates that the thumbnail was successfully retrieved.
         *
         * @since version 1.3.0
         */
        public static int kEvent_Ok = 0;

        /**
         * This event indicates that all the thumbnail retrieving operation has been completed.
         *
         * @since version 1.3.0
         */
        public static int kEvent_Completed = 1;

        /**
         * This event indicates that the thumbnail retrieving was unsuccessful.
         *
         * @since version 1.3.0
         */
        public static int kEvent_Fail = -1;

        /**
         * This event indicates that a new thumbnail task couldn't be generated.
         *
         * @since version 1.3.0
         */
        public static int kEvent_systemError = -2;

        public static int kEvent_UserCancel = -3;
        /**
         * This method gets the details of a retrieved thumbnail.
         *
         * This method is called asynchronously by {@link #onGetVideoClipIDR2YOnlyThumbnailsResult(int, byte[], int, int, int)}.
         * 
         * @param event The kEvents of the class <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
         * @param data The instance of the thumbnail bitmap returned from the <tt>nexEngine</tt>. This instance operates in singleton so it should be saved separately to avoid overwrite with other bitmaps.
         * @param index  The index of the current thumbnail from the total thumbnails retrieved.  
         * @param totalCount The total number of parsed thumbnails.
         * @param timestamp The timestamp of the thumbnails returned from the <tt>nexEngine</tt>.
         * 
         * @since version 1.3.0
         */
        public abstract void onGetVideoClipIDR2YOnlyThumbnailsResult( int event , byte[] data, int index, int totalCount, int timestamp );
    }

    /**
     * This method gets the thumbnails(maximum of 3600 thumbnails) from all the I-frames of a clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.   
     * 
     * @note : The bitmaps of <tt>onGetDetailThumbnailResult()</tt> returned from the listener operates in singleton, 
     * therefore it should be saved separately to avoid overwrite with other bitmaps.
     *
     * @param thumbWidth The width resolution of the thumbnail. 
     * @param thumbHeight The height resolution of the thumbnail.
     * @param listener  The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     *
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, boolean, int[], com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int[], int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @since version 1.3.0
     *
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int getVideoClipIFrameThumbnails( int thumbWidth, int thumbHeight, final OnGetVideoClipDetailThumbnailsListener listener ){
        return getVideoClipDetailThumbnails( thumbWidth, thumbHeight, 0,0,3600,0, listener);
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int getVideoClipIDR2YOnlyThumbnails( int thumbWidth, int thumbHeight, final int startTime, final int endTime, int maxCount, final OnGetVideoClipIDR2YOnlyThumbnailsListener listener){
        if( mInfo.mClipType !=  kCLIP_TYPE_VIDEO ) {
            return -1;
        }

        if( listener == null){
            return -1;
        }

        MediaInfo info = getMediaInfo();
        if( info ==null ){
            if(listener!=null) listener.onGetVideoClipIDR2YOnlyThumbnailsResult(OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_systemError, null, 0, 0, 0);
            return OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_systemError;
        }
        int flags = 0x20002;
        final int[] seekPoint = info.getSeekPointsSync();
        info.getDetailThumbnails(thumbWidth, thumbHeight, startTime, endTime, maxCount ,flags,null, new ThumbnailCallbackRaw() {
            private int lastIndex = 0;
            @Override
            public void processThumbnail(byte[] data, int index, int totalCount, int timestamp) {
                if( data==null ) {
                    Log.d(TAG,"YonlyThumbTest processThumbnail : Y=null index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                } else {
                    Log.d(TAG,"YonlyThumbTest processThumbnail : Y=["+data.length +"] index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                }

                int i = getIndexSeekTabNearTimeStamp(seekPoint, lastIndex, timestamp);
                if( i >= 0  ){
                    Log.d(TAG,"YonlyThumbTest processThumbnail : timestamp =" + timestamp + ", seektable=" + seekPoint[i]+", lastIndex="+lastIndex+", index="+i);
                    timestamp = seekPoint[i];
                    lastIndex = i+1;
                }else{
                    Log.d(TAG,"YonlyThumbTest processThumbnail : timestamp =" + timestamp + ", lastIndex=" + lastIndex );
                }
                listener.onGetVideoClipIDR2YOnlyThumbnailsResult(OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_Ok, data, index, totalCount, timestamp);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                Log.d(TAG,"YonlyThumbTest onFail : " + failureReason.getMessage());
                if (failureReason == NexEditor.ErrorCode.GETCLIPINFO_USER_CANCEL) {
                    listener.onGetVideoClipIDR2YOnlyThumbnailsResult(OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_UserCancel, null, 0, 0, 0);
                } else {
                    listener.onGetVideoClipIDR2YOnlyThumbnailsResult(OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_Fail, null, 0, 0, 0);
                }
            }
        }).onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                Log.d(TAG,"YonlyThumbTest onComplete");
                listener.onGetVideoClipIDR2YOnlyThumbnailsResult(OnGetVideoClipIDR2YOnlyThumbnailsListener.kEvent_Completed, null, 0, 0, 0);
            }
        });

        return 0;
    }

    /**
     * This method gets the thumbnails from I-frames within the specific section of the clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.   
     * 
     * @note The bitmaps of <tt>onGetDetailThumbnailResult()</tt> returned from the listener operates in singleton, 
     * therefore it should be saved separately to avoid overwrite with other bitmaps.
     * If both <tt>startTime</tt> and <tt>endTime</tt> value is set to 0, only I-frames will be retrieved.  
     * 
     * <p>Example code:</p>
     *     {@code
                project.getClip(0, true).getVideoClipDetailThumbnails(project.getClip(0, true).getWidth(), project.getClip(0, true).getHeight(), 0, thumbendtime, 100, 0, new nexClip.OnGetVideoClipDetailThumbnailsListener() {}
            }
     * @param thumbWidth   The width resolution of the thumbnail as integer. 
     * @param thumbHeight   The height resolution of the thumbnail as integer. 
     * @param startTime   The start time of the specific section of the clip to retrieve thumbnails. 
     * @param endTime    The end time of the specific section of the clip to retrieve thumbnails. 
     * @param maxCount  Maximum number of thumbnails to get. 
     * @param rotate  The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
     * @param listener The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, boolean, int[], com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int[], int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)

     * @since version 1.3.0
     */
    public int getVideoClipDetailThumbnails( int thumbWidth, int thumbHeight, final int startTime, final int endTime, int maxCount, int rotate , final OnGetVideoClipDetailThumbnailsListener listener ) {
        long dataSize = FreeSpaceChecker.getFreeSpace(Environment.getDataDirectory().getAbsoluteFile());
        boolean noCache = false;
        if( dataSize < sVideoClipDetailThumbnailsDiskLimitSize ){
            Log.d(TAG,"getVideoClipDetailThumbnails() disk low. run no cache mode. disk size="+dataSize+", limit="+sVideoClipDetailThumbnailsDiskLimitSize);
            noCache = true;
        }
        return getVideoClipDetailThumbnails(thumbWidth, thumbHeight, startTime, endTime, maxCount, rotate, noCache, null, listener);
    }

    /**
     * This method gets the thumbnails from I-frames within the specific section of the clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.   
     *
     * <p>Example code:</p>
     *     {@code
                project.getClip(0, true).getVideoClipDetailThumbnails(project.getClip(0, true).getWidth(), project.getClip(0, true).getHeight(), seekTab, 0, new nexClip.OnGetVideoClipDetailThumbnailsListener() {}
            }
     * @param thumbWidth The width resolution of the thumbnail as integer. 
     * @param thumbHeight The height resolution of the thumbnail as integer. 
     * @param arrayTimeTab  An array of thumbnails within the specific section of the clip as integer. The thumbnails must be retrieved only from I-frames which can be found with <tt>getSeekPointsSync()</tt>.
     * @param rotate The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
     * @param listener The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, boolean, int[], com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @since version 1.3.43
     */
    public int getVideoClipDetailThumbnails( int thumbWidth, int thumbHeight, int [] arrayTimeTab , int rotate , final OnGetVideoClipDetailThumbnailsListener listener ) {
        long dataSize = FreeSpaceChecker.getFreeSpace(Environment.getDataDirectory().getAbsoluteFile());
        boolean noCache = false;
        if( dataSize < sVideoClipDetailThumbnailsDiskLimitSize ){
            Log.d(TAG,"getVideoClipDetailThumbnails() disk low. run no cache mode. disk size="+dataSize+", limit="+sVideoClipDetailThumbnailsDiskLimitSize);
            noCache = true;
        }
        int maxCount = 0;
        if( arrayTimeTab != null ){
            maxCount = arrayTimeTab.length;
        }
        return getVideoClipDetailThumbnails(thumbWidth, thumbHeight, 0, 0, maxCount, rotate, noCache,arrayTimeTab, listener);
    }

    private static long sVideoClipDetailThumbnailsDiskLimitSize = 100000000;

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public static void setVideoClipDetailThumbnailsDiskLimit(long size ){
        sVideoClipDetailThumbnailsDiskLimitSize = size;
    }

    /**
     * This method gets the thumbnails from I-frames within the specific section of the clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.   
     * This method allows to set all the options available for retrieving thumbnails. 
     *
     * <p>Example code:</p>
     *     {@code
                mClip.getVideoClipDetailThumbnails(mWidth, mHeight, startTime, endTime, maxCount, mAngle, noCache, seekTab,new nexClip.OnGetVideoClipDetailThumbnailsListener() {}
            }
     * @param thumbWidth The width resolution of the thumbnail as integer. 
     * @param thumbHeight The height resolution of the thumbnail as integer. 
     * @param startTime The start time of the specific section of the clip to retrieve thumbnails. 
     * @param endTime The end time of the specific section of the clip to retrieve thumbnails. 
     * @param maxCount Maximum number of thumbnails to get. 
     * @param rotate  The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
     * @param noCache  Set to <tt>TRUE</tt> to retrieve thumbnails without caching them; otherwise <tt>FALSE</tt>. 
     * @param arrayTimeTab An array of thumbnails within the specific section of the clip as integer. The thumbnails must be retrieved only from I-frames which can be found with <tt>getSeekPointsSync()</tt>.
     * @param listener The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int[], int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @since version 1.3.43
     */
    public int getVideoClipDetailThumbnails( int thumbWidth, int thumbHeight, final int startTime, final int endTime, int maxCount, int rotate ,boolean noCache, int[] arrayTimeTab , final OnGetVideoClipDetailThumbnailsListener listener ){
        if( mInfo.mClipType !=  kCLIP_TYPE_VIDEO ) {
            return -1;
        }

        if( listener == null){
            return -1;
        }

        MediaInfo info = getMediaInfo();
        if( info ==null ){
            if(listener!=null) listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_systemError,null,0,0,0);
            return OnGetVideoClipDetailThumbnailsListener.kEvent_systemError;
        }
        int flags = 0;

        switch (rotate){
            case 90:
                flags |= 0x10;
                break;
            case 180:
                flags |= 0x20;
                break;
            case 270:
                flags |= 0x40;
                break;
        }

        if( noCache ) {
            flags |= 0x40000;
        }

        info.getDetailThumbnails(thumbWidth, thumbHeight, startTime, endTime, maxCount ,flags, arrayTimeTab, new ThumbnailCallbackBitmap() {
            @Override
            public void processThumbnail(Bitmap bm, int index, int totalCount, int timestamp) {
                if( bm==null ) {
                    Log.d(TAG,"detailThumbTest processThumbnail : bm=null index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                } else {
                    Log.d(TAG,"detailThumbTest processThumbnail : bm=[" + bm.getWidth() + "x" + bm.getHeight() + " cfg=" + bm.getConfig().name() + "] index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                }
                listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Ok,bm,index,totalCount,timestamp);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                Log.d(TAG,"detailThumbTest onFail : " + failureReason.getMessage());
                if (failureReason == NexEditor.ErrorCode.GETCLIPINFO_USER_CANCEL) {
                    listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel, null, 0, 0, 0);
                } else {
                    listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Fail, null, 0, 0, 0);
                }
            }
        }).onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                Log.d(TAG,"detailThumbTest onComplete");
                listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Completed, null, 0, 0, 0);
            }
        });
        return 0;
    }

    /**
     * This method gets the thumbnails from I-frames within the specific section of the clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.
     * This method allows to set all the options available for retrieving thumbnails.
     *
     * <p>Example code:</p>
     *     {@code
    mClip.getVideoClipDetailThumbnails(mWidth, mHeight, startTime, endTime, maxCount, mAngle, noCache, withPFrame, seekTab,new nexClip.OnGetVideoClipDetailThumbnailsListener() {}
    }
     * @param thumbWidth The width resolution of the thumbnail as integer.
     * @param thumbHeight The height resolution of the thumbnail as integer.
     * @param startTime The start time of the specific section of the clip to retrieve thumbnails.
     * @param endTime The end time of the specific section of the clip to retrieve thumbnails.
     * @param maxCount Maximum number of thumbnails to get.
     * @param rotate  The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
     * @param noCache  Set to <tt>TRUE</tt> to retrieve thumbnails without caching them; otherwise <tt>FALSE</tt>.
     * @param withPFrame  Set to <tt>TRUE</tt> to retrieve thumbnails with p-frame; otherwise <tt>FALSE</tt>.
     * @param arrayTimeTab An array of thumbnails within the specific section of the clip as integer. The thumbnails must be retrieved only from I-frames which can be found with <tt>getSeekPointsSync()</tt>.
     * @param listener The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int[], int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @since version 1.3.43
     */
    public int getVideoClipDetailThumbnails( int thumbWidth, int thumbHeight, final int startTime, final int endTime, int maxCount, int rotate ,boolean noCache, boolean withPFrame, int[] arrayTimeTab , final OnGetVideoClipDetailThumbnailsListener listener ) {
        return getVideoClipDetailThumbnails(thumbWidth, thumbHeight, startTime, endTime, maxCount, rotate, noCache, withPFrame, false, false, null, listener);
    }

    /**
     * This method gets the thumbnails from I-frames within the specific section of the clip.
     * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.
     * This method allows to set all the options available for retrieving thumbnails.
     *
     * <p>Example code:</p>
     *     {@code
    mClip.getVideoClipDetailThumbnails(mWidth, mHeight, startTime, endTime, maxCount, mAngle, noCache, withPFrame, seekTab,new nexClip.OnGetVideoClipDetailThumbnailsListener() {}
    }
     * @param thumbWidth The width resolution of the thumbnail as integer.
     * @param thumbHeight The height resolution of the thumbnail as integer.
     * @param startTime The start time of the specific section of the clip to retrieve thumbnails.
     * @param endTime The end time of the specific section of the clip to retrieve thumbnails.
     * @param maxCount Maximum number of thumbnails to get.
     * @param rotate  The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
     * @param noCache  Set to <tt>TRUE</tt> to retrieve thumbnails without caching them; otherwise <tt>FALSE</tt>.
     * @param withPFrame  Set to <tt>TRUE</tt> to retrieve thumbnails with p-frame; otherwise <tt>FALSE</tt>.
     * @param withFirst  Set to <tt>TRUE</tt> to retrieve thumbnails match to starttime; otherwise <tt>FALSE</tt>.
     * @param withLast  Set to <tt>TRUE</tt> to retrieve thumbnails match to endtime; otherwise <tt>FALSE</tt>.
     * @param arrayTimeTab An array of thumbnails within the specific section of the clip as integer. The thumbnails must be retrieved only from I-frames which can be found with <tt>getSeekPointsSync()</tt>.
     * @param listener The instance of <tt>OnGetVideoClipDetailThumbnailsListener</tt>.
     * @return -1 if there is no listener or the clip is not a video clip; otherwise 0.
     * @see #getVideoClipDetailThumbnails(int, int, int, int, int, int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @see #getVideoClipDetailThumbnails(int, int, int[], int, com.nexstreaming.nexeditorsdk.nexClip.OnGetVideoClipDetailThumbnailsListener)
     * @since version 1.3.43
     */
    public int getVideoClipDetailThumbnails( int thumbWidth, int thumbHeight, final int startTime, final int endTime, int maxCount, int rotate ,boolean noCache, boolean withPFrame, boolean withFirst, boolean withLast, int[] arrayTimeTab , final OnGetVideoClipDetailThumbnailsListener listener ){
        if( mInfo.mClipType !=  kCLIP_TYPE_VIDEO ) {
            return -1;
        }

        if( listener == null){
            return -1;
        }

        MediaInfo info = getMediaInfo();
        if( info ==null ){
            if(listener!=null) listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_systemError,null,0,0,0);
            return OnGetVideoClipDetailThumbnailsListener.kEvent_systemError;
        }
        int flags = 0;

        switch (rotate){
            case 90:
                flags |= 0x10;
                break;
            case 180:
                flags |= 0x20;
                break;
            case 270:
                flags |= 0x40;
                break;
        }

        if( noCache ) {
            flags |= 0x40000;
        }

        if( withPFrame ) {
            int pframe = 1;
            flags = flags | (pframe << 8 );
            arrayTimeTab = null;
        }
        if ( withFirst ) {
            flags = flags | 0x00080000;
        }
        if ( withLast ) {
            flags = flags | 0x00100000;
        }

        info.getDetailThumbnails(thumbWidth, thumbHeight, startTime, endTime, maxCount ,flags, arrayTimeTab, new ThumbnailCallbackBitmap() {
            @Override
            public void processThumbnail(Bitmap bm, int index, int totalCount, int timestamp) {
                if( bm==null ) {
                    Log.d(TAG, "detailThumbTest processThumbnail : bm=null index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                } else {
                    Log.d(TAG,"detailThumbTest processThumbnail : bm=[" + bm.getWidth() + "x" + bm.getHeight() + " cfg=" + bm.getConfig().name() + "] index=" + index + " totalCount=" + totalCount + " timestamp=" + timestamp);
                }
                listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Ok,bm,index,totalCount,timestamp);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                Log.d(TAG,"detailThumbTest onFail : " + failureReason.getMessage());
                if (failureReason == NexEditor.ErrorCode.GETCLIPINFO_USER_CANCEL) {
                    listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel, null, 0, 0, 0);
                } else {
                    listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Fail, null, 0, 0, 0);
                }
            }
        }).onComplete(new Task.OnTaskEventListener() {
            @Override
            public void onTaskEvent(Task t, Task.Event e) {
                Log.d(TAG,"detailThumbTest onComplete");
                listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_Completed, null, 0, 0, 0);
            }
        });
        return 0;
    }

    private Bitmap makeThumbnail(float height, float scaleDIPtoPX){
        if( mInfo.mClipType ==  kCLIP_TYPE_IMAGE ) {
            Bitmap bm = null;
            if( isSolid() ){
                int color = (int)Long.parseLong(mPath.substring(7, 15),16);
                int w = 32;
                int h = 18;
                int[] pixels = new int[w*h];
                for( int i=0; i<pixels.length; i++ )
                    pixels[i] = color;
                bm = Bitmap.createBitmap(pixels, w, h, Bitmap.Config.ARGB_8888);
                //mSingleThumbnail = new WeakReference<Bitmap>(thumb);
            }else {

                BitmapFactory.Options opt = new BitmapFactory.Options();
                opt.inJustDecodeBounds = true;
                BitmapFactory.decodeFile(mPath, opt);
                opt.inJustDecodeBounds = false;

                int thumbWidth = Math.min((int) (100 * scaleDIPtoPX), (int) ((float) opt.outWidth / (float) opt.outHeight * (float) height));
                int thumbHeight = (int) height;
                NexImageLoader.LoadedBitmap loadedBitmap = NexImageLoader.loadBitmap(mPath, thumbWidth * 2, thumbHeight * 2);
                bm = loadedBitmap.getBitmap(); //NexImage.loadBitmap(m_mediaPath, thumbWidth*2, thumbHeight*2).getBitmap();
            }
            if (bm != null) {
                return Bitmap.createScaledBitmap(bm, Math.min((int) (100 * scaleDIPtoPX), (int) ((float) bm.getWidth() / (float) bm.getHeight() * (float) height)), (int) height, true);
            }
            return null;

        }
        return null;
    }

    /**
     * This method sets the degrees of rotation of a clip. 
     * A video file has its rotation degree information saved as metadata when it's being filmed. 
     * Use this method to rotate the video file before playing or exporting. 
     * As of a JPEG image, this method is not necessary because NexEditor&trade;&nbsp; SDK will automatically adjust the rotation degree. 
     *
     * <p>Example code:</p>
     *     {@code     rt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        mEngine.getProject().getClip(0, true).setRotateDegree(mEngine.getProject().getClip(0, true).getRotateDegree() + 90);
                        setDimension();
                    }
                });
            }
     * @param degree The number of degrees to set to a video clip as integer. This will be one of the following:
     * <ul>
     * <li>{@link #kClip_Rotate_0} = {@value #kClip_Rotate_0}
     * <li>{@link #kClip_Rotate_90} = {@value #kClip_Rotate_90}
     * <li>{@link #kClip_Rotate_180} = {@value #kClip_Rotate_180}
     * <li>{@link #kClip_Rotate_270} = {@value #kClip_Rotate_270}
     * </ul>
     * @see #getRotateDegree()
     * @since 1.0.1
     */
    public void setRotateDegree(int degree){
        if( degree >= 360 ){
            degree = 0;
        }

        mRotate = degree;

        if( mCrop != null )
            mCrop.setRotate(mRotate);
        setProjectUpdateSignal(true);
    }

    /** 
     * This method gets the degrees of rotation set on a clip, rotated counterclockwise, if the clip is an image or video clip. 
     *
     * <p>Example code:</p>
     *     {@code     rt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        mEngine.getProject().getClip(0, true).setRotateDegree(mEngine.getProject().getClip(0, true).getRotateDegree() + 90);
                        setDimension();
                    }
                });
          }
     * @return The number of degrees the clip has been rotated counterclockwise.  This will be one of the following:
     * <ul>
     * <li>{@link #kClip_Rotate_0} = {@value #kClip_Rotate_0}
     * <li>{@link #kClip_Rotate_90} = {@value #kClip_Rotate_90}
     * <li>{@link #kClip_Rotate_180} = {@value #kClip_Rotate_180}
     * <li>{@link #kClip_Rotate_270} = {@value #kClip_Rotate_270}
     * </ul>
     * @see #setRotateDegree(int)
     * @since version 1.0.1
     */
    public int getRotateDegree(){
        return mRotate;
    }

    protected int runDuration(){
        if( mInfo.mClipType == kCLIP_TYPE_VIDEO ){
            if( mVideoEdit == null ) {
                return mInfo.mTotalTime;
            }
            return  (mInfo.mTotalTime - mVideoEdit.mTrimStartDuration - mVideoEdit.mTrimEndDuration)*(Math.round((float)(mVideoEdit.getSpeedControl()/100)));
        }else if( mInfo.mClipType == kCLIP_TYPE_IMAGE ){
            return mDuration;
        }
        return mInfo.mTotalTime;
    }

    boolean isFaceDetectProcessed(){

        return mFacedetectProcessed;
    }

    Rect getFaceRect(){

        return mFaceRect;
    }

    boolean isFaceDetected(){

        return mFaceDetected > 0?true:false;
    }

    void resetFaceDetectProcessed(){

        mFacedetectProcessed = false;
    }

    void setFaceDetectProcessed(boolean face_detected, Rect face){

        mFacedetectProcessed = true;
        mFaceDetected = face_detected?1:0;
        mFaceRect = face;
    }

    /** 
     * This method crops a clip. 
     * 
     * <p>Example code:</p>
     *     {@code     for( int i = 0 ; i < project.getTotalClipCount(true) ; i++ ){
                    project.getClip(i, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PAN_RAND);
                }
            }
     * @return  A cropped section of the clip, as an instance of <tt>nexCrop</tt>.
     * @see nexCrop
     * @since version 1.1.0
     */
    public nexCrop getCrop() {
        if( mCrop == null ){
            int w = mInfo.mWidth;
            int h = mInfo.mHeight;
            if( mInfo.mClipType == kCLIP_TYPE_IMAGE ) {
                if( mInfo.mRotateDegreeInMeta == 90 ||  mInfo.mRotateDegreeInMeta == 270 ){
                    w = mInfo.mHeight;
                    h = mInfo.mWidth;
                }
            }
            int totaltime = getProjectDuration();
            mCrop = new nexCrop(mPath,w,h,mRotate, totaltime);
        }else{
            mCrop.setRotate(mRotate);
            mCrop.setClipDuration(getProjectDuration());
        }
        return mCrop;
    }


    /** 
     * This method sets the volume of the background music (BGM) for a project. 
     *
     * <p>Example code:</p>
     *     {@code     if( mBGMOnOff.isChecked() ) {
                    mEngin.getProject().getClip(0, true).setBGMVolume(mBGMVol.getProgress());
                } else {
                    mEngin.getProject().getClip(0, true).setBGMVolume(0);
                }
            }
     * @param volume The volume of the BGM, as an <tt>integer</tt>.  This value can range from 0 to 200 and the default value is 100. 
     * @return <tt>FALSE</tt> if the input value is not in the range of 0 to 200, otherwise <tt>TRUE</tt>.
     * @see #getBGMVolume()
     * @since version 1.1.0
     */
    public boolean setBGMVolume(int volume){
        if( volume < 0 || volume > 100 )
            return false;
        m_BGMVolume = volume;
        setProjectUpdateSignal(true);
        return true;
    }

    /** 
     * This method gets the volume of a project's background music (BGM). 
     * 
     * <p>Example code:</p>
     *     {@code
                visualClip.mBGMVolume = clipItem.getBGMVolume();
            }
     * @return The volume of BGM, as an <tt>integer</tt>.  This value will be in the range of 0 to 200. 
     * @see #setBGMVolume(int)
     * @since version 1.1.0
     */
    public int getBGMVolume(){
        return m_BGMVolume;
    }

    /** 
     * This method sets the volume of a clip. 
     * 
     * <p>Example code:</p>
     *     {@code
                project.getClip(0, false).setClipVolume(mBGMVol);
            }
     * @param volume The volume of the clip, as an <tt>integer</tt>.  This value can range from 0 to 200 and the default value is 100. 
     * @return <tt>FALSE</tt> if the input value is not in the range of 0 to 200, otherwise <tt>TRUE</tt>. 
     * @see #getClipVolume()
     * @since version 1.1.0
     */
    public boolean setClipVolume(int volume){
        if( volume < 0 || volume > 200 )
            return false;
        m_ClipVolume = volume;
        setProjectUpdateSignal(true);
        return true;
    }

    /** 
     * This method gets the volume of a clip. 
     * 
     * <p>Example code:</p>
     *     {@code
                item.mClipVolume = aClip.getClipVolume();
            }
     * @return The volume of the clip, as an <tt>integer</tt>. This value will be in the range of 0 to 200 and the default value is 100.  
     * @see #setClipVolume(int)
     * @since version 1.1.0
     */
    public int getClipVolume(){
        return m_ClipVolume;
    }

    protected int getProjectDuration(){
        int duration = 0;
        switch ( mInfo.mClipType ){
            case kCLIP_TYPE_VIDEO:
                duration = mInfo.mTotalTime;
                if( mVideoEdit != null ){
                    if( mVideoEdit.mTrimStartDuration != 0 || mVideoEdit.mTrimEndDuration != 0){
                        duration = (mInfo.mTotalTime - mVideoEdit.mTrimStartDuration -mVideoEdit.mTrimEndDuration);
                    }
                    int speedcontrol = mVideoEdit.getSpeedControl();
                    if( speedcontrol != 100 ){
                        duration = Math.round((float)(duration*100/speedcontrol));
                    }
                }
                break;
            case kCLIP_TYPE_IMAGE:
                duration = mDuration;
                break;
            case kCLIP_TYPE_AUDIO:
                duration = mInfo.mTotalTime;
                break;
        }
        return duration;
    }

    /**
     * This method gets the location of the seek table (including I-frame and IDR) if the file is a video file. 
     * 
     * <p>Example code:</p>
     *     {@code     int tab[] = mClip.getSeekPointsSync();
                seekTab = new int[2];
                if( tab.length > 2 ){
                    seekTab[0] = tab[1];
                    Log.d(TAG,"user tab time1 ="+seekTab[0]);
                    seekTab[1] = tab[tab.length-1];
                    Log.d(TAG,"user tab time2 ="+seekTab[1]);
                    startTime = 0;
                    endTime= 0;
                    maxCount = seekTab.length;
                }
            }
     * @return The CTS value of the I-frame as an <tt>array</tt> .
     * @see com.nexstreaming.nexeditorsdk.nexEngine#getIDRSeekTabSync(nexClip)
     * @since version 1.3.4
     */
    public int[] getSeekPointsSync(){
        if( mInfo.mClipType !=  kCLIP_TYPE_VIDEO ) {
            return null;
        }

        MediaInfo info = getMediaInfo();
        if( info ==null ){
            return null;
        }
        return info.getSeekPointsSync();
    }
 
    /**
     * This method cancels the thumbnail retrieving process. 
     * (all clip, all thumbnail)
     * @return <tt>TRUE</tt> if successful, otherwise <tt>FALSE</tt>.  
     * @since version 1.3.14
     *
     *
     */
    public static boolean cancelThumbnails(){
        //return false;
        return MediaInfo.cancelAllGetThumbnails();
    }

    private int getIndexSeekTabNearTimeStamp(int[] seekPoint,int lastIndex, int timestamp){
        int i = lastIndex;
        for( i=lastIndex; i< seekPoint.length; i++){
            if( seekPoint[i] == timestamp ){
                return i;
            }else if( seekPoint[i] > timestamp ){
                if( i == 0 ){
                    return i;
                }

                if( (seekPoint[i] - timestamp) > ( timestamp - seekPoint[i-1] ) ){
                    return (i-1);
                }
                return i;
            }
        }

        if( i >=  seekPoint.length ){
            return seekPoint.length-1;
        }
        return -1;
    }

    /**
     * This method creates a clip with a solid color specified to the parameter <tt>argbColor</tt>. 
     * The clip will be made independently by the NexEditor&trade;&nbsp; SDK without using any external media content, in the purpose of making a background color.
     * 
     * <p>Example code:</p>
     *     {@code
                nexClip clip = nexClip.getSolidClip(Color.RED);
            }
     * @param argbColor An ARGB value of a color to set to a clip as <tt>integer</tt>.
     * @return  A clip with a solid background color.
     * @see #getSolidColor()
     * @see #setSolidColor(int)
     * @since version 1.3.43
     */
    public static nexClip getSolidClip(int argbColor){
        return getSolidClip(String.format("@solid:%08X.jpg",argbColor));
    }

    private static nexClip getSolidClip(String path){
        nexClip clip = new nexClip();
        clip.mPath = path;
        clip.mInfo = new ClipInfo();
        clip.mInfo.mClipType = kCLIP_TYPE_IMAGE;
        clip.mInfo.mWidth = 32;
        clip.mInfo.mHeight = 18;
        return clip;
    }
    /**
     * This method indicates whether or not a clip created by {@link #getSolidClip(int)} is solid. 
     *
     * @return <tt>TRUE</tt> if it is a solid clip; otherwise, <tt>FALSE</TT> if it is an image, video, or an audio clip.
     * @see #getSolidClip(int)
     * @since 1.5.23
     */
    public boolean isSolid() {
        return mPath!=null && (mPath.startsWith("@solid:") && mPath.endsWith(".jpg"));
    }

    /**
     * This method changes the color of a clip only created by the method <tt>getSolidClip()</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                boolean isColor = clip.setSolidColor(Color.BLUE);
            }
     * @param color  The ARGB value of a new color to set to a clip as integer.
     * @return A clip with a new solid background color; <tt>FALSE</tt> if the clip was not created by <tt>getSolidClip()</tt>.
     * @see #getSolidColor()
     * @since version 1.3.43
     */
    public boolean setSolidColor(int color){
        if( isSolid() ){
            mPath = String.format("@solid:%08X.jpg",color);
            return true;
        }
        return false;
    }

    /**
     * This method gets the ARGB value of the color set to a clip if the clip was made by the method <tt>getSolidClip()</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                int color = clip.getSolidColor();
            }
     * @return  The ARGB value of the color if the clip is made by <tt>getSolidClip()</tt>; otherwise 0.
     * @see #setSolidColor(int)
     * @since version 1.3.43
     */
    public int getSolidColor() {
        if( !isSolid() )
            return 0;
        int color = (int)Long.parseLong(mPath.substring(7, 15),16);
        return color;
    }

    protected boolean isMotionTrackedVideo() {
        return isMotionTrackedVideo;
    }

    protected void setMotionTrackedVideo(boolean motionTrackedVideo) {
        isMotionTrackedVideo = motionTrackedVideo;
    }

    /**
     * This method gets an audio item to add or adjust the audio envelop.  
     *
     * <p>Example code:</p>
     *     {@code
                for(int i=0; i<mTime.size(); i++){
                    project.getClip(0,false).getAudioEnvelop().addVolumeEnvelope(Integer.parseInt(mTime.get(i)),Integer.parseInt(mVol.get(i)));
                }
            }
     * @return nexAudioEnvelop  An audio item. 
     * @since version 1.3.52
     */
    public nexAudioEnvelop getAudioEnvelop(){
        if( mAudioEnvelop == null ) {
            mAudioEnvelop = new nexAudioEnvelop(this);
        }
        return mAudioEnvelop;
    }

    /**
     * This method gets the clip effect
     * @param internal Set to \c TRUE when using internally.
     * @return clip effect
     * @since version 1.5.15
     */
    public nexClipEffect getClipEffect( boolean internal ){
        if( !mProjectAttachment ){
            Log.e(TAG,"getClipEffect not project attachment startTime="+mStartTime);
        }
        if (mClipEffect == null) {
            mClipEffect = new nexClipEffect();
        }
        return mClipEffect;
    }

    /**
     * This method gets the transition effect
     * @param internal Set to \c TRUE when using internally.
     * @return transition effect
     * @since version 1.5.15
     */
    public nexTransitionEffect getTransitionEffect( boolean internal ){
        if( !mProjectAttachment ){
            Log.e(TAG,"getTransitionEffect not project attachment startTime="+mStartTime);
        }

        if (mTransitionEffect == null) {
            mTransitionEffect = new nexTransitionEffect(mObserver);
        }
        return mTransitionEffect;
    }

    /**
     * This method sets the voice changer factor which is used to modulate the audio of a clip.
     *  
     * @param factor Audio modulate value as an <tt>integer</tt>, in range of 0 ~ 4. 
     *               Possible values for voice changer factor : 
     *               - 0 : Default
     *               - 1 : Chipmunk
     *               - 2 : Robot
     *               - 3 : Deep
     *               - 4 : Modulation 
     * @since version 1.5.15
     * @see #getVoiceChangerFactor()
     */
    @Deprecated
    public void setVoiceChangerFactor(int factor){
        /*
        MediaInfo info = getMediaInfo();
        if( info ==null ){
            info = MediaInfo.getInfo(getPath(),mMediaInfoUseCache);
        }
        if(info == null){
            Log.d("nexClip","setVoiceChangerFactor getInfo fail!");
            return;
        }
        Log.d("nexClip","setVoiceChangerFactor info.getAudioSamplingRate("+info.getAudioSamplingRate()+")!");
        */
//        if(info.getAudioSamplingRate() > 48000){
//            throw new InvalidRangeException(0, 48000, info.getAudioSamplingRate());
//        }
        if ( mInfo.mClipType == kCLIP_TYPE_VIDEO || mInfo.mClipType == kCLIP_TYPE_AUDIO) {
            getAudioEdit().setVoiceChangerFactor(factor);
        }

    }
 
    /**
     * This method gets the voice changer factor set to a clip. 
     *
     * @return The voice changer factor, as an <tt>integer</tt> (0 ~ 4).
     *
     * @since version 1.5.15
     * @see #setVoiceChangerFactor(int)
     */
    @Deprecated
    public int getVoiceChangerFactor(){
        if ( mInfo.mClipType == kCLIP_TYPE_VIDEO || mInfo.mClipType == kCLIP_TYPE_AUDIO) {
            return getAudioEdit().getVoiceChangerFactor();
        }
        return 0;
    }

    private int setMediaInfo(MediaInfo info){
        mediainfo = info;
        return 1;
    }

    MediaInfo getMediaInfo(){
        if( mediainfo ==null ){
            mediainfo = MediaInfo.getInfo(getRealPath(),mMediaInfoUseCache);
        }

        return mediainfo;
    }

    /**
     * This method returns a nexAudioEdit instance if there is an audio in the clip.
     * @return The audio edit class of the clip.
     * @since 1.5.43
     */
    public nexAudioEdit getAudioEdit(){
        if( mProjectAttachment ) {
            if ( mInfo.mClipType == kCLIP_TYPE_VIDEO || mInfo.mClipType == kCLIP_TYPE_AUDIO) {

                if (mAudioEdit == null) {
                    mAudioEdit = nexAudioEdit.getnexAudioEdit(this);
                }
                return mAudioEdit;
            }else{
                return null;
            }
        }
        throw new ProjectNotAttachedException();
    }

    /**
     * @deprecated
     * @return UUID
     * @since 1.5.43
     */
    @Deprecated
    public byte[] getVideoUUID(){
        if( mInfo.mVideoRenderMode == 0 ){
            return null;
        }
        return mInfo.mVideoUUID;
    }

    /**
     * @deprecated
     * @return
     * @since 1.5.43
     */
    @Deprecated
    public void setVignetteEffect(boolean on){
        mVignette = on;
    }

    /**
     * @deprecated
     * @return
     * @since 1.5.43
     */
    @Deprecated
    public boolean getVignetteEffect(){
        return mVignette;
    }

    private boolean mIsAssetResource = false;

    /**
     * This method get that clip is asset resource or not.
     * @return True : This was added from template or collage manager automatically. False : This was add from user.
     * @since 2.0.5
     */
    public boolean isAssetResource() {
        return mIsAssetResource;
    }

    protected void setAssetResource(boolean isAssetResource) {
        mIsAssetResource = isAssetResource;
    }


    int getTemplateEffectID()
    {
        return mTemplateEffectID;
    }

    void setTemplateEffectID(int id)
    {
        mTemplateEffectID = id;
    }

    String getCollageDrawInfoID()
    {
        return mCollageDrawInfoID;
    }

    void setCollageDrawInfoID(String id)
    {
        mCollageDrawInfoID = id;
    }

    int getTemplateAudioPos()
    {
        return mTemplateAudioPos;
    }

    void setTemplateAudioPos(int Pos)
    {
        mTemplateAudioPos = Pos;
    }

    boolean getTemplateOverlappedTransition()
    {
        return mOverlappedTransition;
    }

    void setTemplateOverlappedTransition(boolean overlapped)
    {
        mOverlappedTransition = overlapped;
    }

    void clearDrawInfos() {
        mDrawInfos.clear();
    }

    public void addDrawInfo(nexDrawInfo drawInfo) {
        mDrawInfos.add(drawInfo);
    }

    void addDrawInfos(List<nexDrawInfo> drawInfos) {
        for(nexDrawInfo i : drawInfos )
            mDrawInfos.add(i);
    }

    void removeDrawInfos(int subID) {
        for(nexDrawInfo i : mDrawInfos ) {
            if( i.getSubEffectID() == subID ) {
                mDrawInfos.remove(i);
                return;
            }
        }
        return;
    }

    List<nexDrawInfo> getDrawInfos()
    {
        return mDrawInfos;
    }

    /**
     * @since 1.7.29
     * @see #getAudioPcmLevels(OnGetAudioPcmLevelsResultListener)
     */
    public static abstract class OnGetAudioPcmLevelsResultListener {

        /**
         *
         * @param pcmLevels
         * @since 1.7.29
         */
        public abstract void onGetAudioPcmLevelsResult(byte[] pcmLevels);
    }

    //private transient byte[] m_pcmData;
    //private transient boolean m_gotPCMData = false;
    private transient boolean m_gettingPcmData;

    /**
     *
     * @param listener
     * @return false - fail! no response listener.
     * @since 1.7.29
     */
    public boolean getAudioPcmLevels( final OnGetAudioPcmLevelsResultListener listener){
        if( !hasAudio() ){
            return false;
        }

        MediaInfo info = getMediaInfo();
        if (info == null) {
            //if(listener!=null) listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_systemError,null,0,0,0);
            return false;
        }

        if(  !m_gettingPcmData  ) {

            m_gettingPcmData = true;

            info.getPCMLevels().onResultAvailable(new OnResultAvailableListener<PCMLevels>() {
                @Override
                public void onResultAvailable(ResultTask<PCMLevels> task, Event event, PCMLevels result) {
                    m_gettingPcmData = false;

                    if (listener != null) {
                        listener.onGetAudioPcmLevelsResult(result.getLevels());
                    }
                }
            });
            return true;
        }
        return false;
    }

    public boolean getAudioPcmLevels( int startTime, int endTime, int useCount, int skipCount,  final OnGetAudioPcmLevelsResultListener listener){
        if( !hasAudio() ){
            return false;
        }

        MediaInfo info = getMediaInfo();
        if (info == null) {
            //if(listener!=null) listener.onGetDetailThumbnailResult(OnGetVideoClipDetailThumbnailsListener.kEvent_systemError,null,0,0,0);
            return false;
        }

        info.getPCMLevels(startTime,endTime,useCount,skipCount).onResultAvailable(new OnResultAvailableListener<PCMLevels>() {
            @Override
            public void onResultAvailable(ResultTask<PCMLevels> task, Event event, PCMLevels result) {

                if (listener != null) {
                    listener.onGetAudioPcmLevelsResult(result.getLevels());
                }
            }
        }).onFailure(new OnFailListener() {
            @Override
            public void onFail(Task t, Event e, TaskError failureReason) {
                if (listener != null) {
                    listener.onGetAudioPcmLevelsResult(null);
                }
            }
        });
        return true;
    }

    /**
     *
     * @param audioStartTime
     * @since 1.7.57
     */
    public void setAVSyncTime(int audioStartTime){
        if( getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
            if( hasAudio() ) {
                if (audioStartTime < -500) {
                    mAVSyncAudioStartTime = -500;
                    return;
                } else if (audioStartTime > 500) {
                    mAVSyncAudioStartTime = 500;
                } else {
                    mAVSyncAudioStartTime = audioStartTime;
                }
            }
        }
    }

    /**
     *
     * @return
     * @since 1.7.57
     */
    public int getAVSyncTime(){
        return mAVSyncAudioStartTime;
    }

    /**
     * @deprecated
     * @return
     * @since 1.7.57
     */
    public boolean ableToDecoding(){
        boolean video = false;
        boolean audio = false;

        if( hasVideo() ){
            video = true;
        }

        if( hasAudio() ){
            audio = true;
        }

        if( video == false && audio == false ){
            Log.d(TAG,"ableToDecoding video not found!");
            return true; //TODO
        }

        MediaInfo info = getMediaInfo();
        if( info == null ){
            Log.d(TAG,"ableToDecoding getMediaInfo fail!");
            return false;
        }

        NexEditor.ErrorCode err = info.ableToDecoding(video,audio);
        if( err == NexEditor.ErrorCode.NONE ){
            return true;
        }
        Log.d(TAG,"ableToDecoding fail!="+err);
        return false;
    }


    /**
     *
     * set property slow recording video.
     *
     * @param on true - 240fps , 120fps slow camcoded video. false - normal video.
     * @return true - OK, false - NOK ( No slow video )
     * @since 2.0.0
     */
    public boolean setClipPropertySlowVideoMode(boolean on ){
        if( mInfo.mClipType != kCLIP_TYPE_VIDEO){
            Log.d(TAG,"was  not video");
            return false;
        }

        if( mInfo.mFramesPerSecond < 60 ){
            Log.d(TAG,"not supported fps="+mInfo.mFramesPerSecond);
            return false;
        }

        int seekCount = 1;
        if( mInfo.mSeekPointCount > 0 )
            seekCount = mInfo.mSeekPointCount;
        int seekDuration = mInfo.mVideoTotalTime / seekCount;
        if( seekDuration > 600 ){
            Log.d(TAG,"not supported idr dur="+seekDuration);
            return false;
        }
        mPropertySlowVideoMode = on;
        return true;
    }


    public boolean  setClipIFamePlayMode(boolean on ){

	mIframePlay =on;
	return true;

    }

    /**
     *
     * @return true - 240fps , 120fps slow camcoded video. false - normal video.
     * @since 2.0.0
     * @see #setClipPropertySlowVideoMode(boolean)
     */
    public boolean getClipPropertySlowVideoMode(){
        return mPropertySlowVideoMode;
    }

    /**
     *
     * @return
     * @since 2.0.11
     */
    public boolean isEncryptedAssetClip(){
        if( mTransCodingPath == null ){
            return false;
        }

        if( mTransCodingPath.startsWith("@assetItem:")){
            return true;
        }
        return false;
    }

    nexSaveDataFormat.nexClipOf getSaveData(){
        nexSaveDataFormat.nexClipOf data = new nexSaveDataFormat.nexClipOf();
        data.m_BGMVolume = m_BGMVolume;
        data.mPath = mPath;
        data.mTransCodingPath = mTransCodingPath;
        //public boolean mProjectAttachment; --> internal set
        if(mClipEffect == null) {
            data.mClipEffect = null;
        }else{
            data.mClipEffect = mClipEffect.getSaveData();
        }

        if( mTransitionEffect == null) {
            data.mTransitionEffect = null;
        }else{
            data.mTransitionEffect = mTransitionEffect.getSaveData();
        }
        data.misMustDownSize = misMustDownSize;
        //private boolean m_gettingThumbnails=false; -->internal false set
        //private boolean m_getThumbnailsFailed=false; -->internal false set
        //private nexObserver mObserver; -->internal set
        data.mAudioOnOff = mAudioOnOff;
        //color adjust
        data.m_Brightness = m_Brightness;
        data.m_Contrast = m_Contrast;
        data.m_Saturation = m_Saturation;
        data.mVignette = mVignette;
        data.mFacedetectProcessed = mFacedetectProcessed;
        data.mFaceDetected = mFaceDetected;
        data.mFaceRect = mFaceRect;//= new Rect(); internal set

        if( mCrop == null) {
            data.mCrop = null;
        }else{
            data.mCrop =  mCrop.getSaveData();
        }
        //private int index = 0; --> no use
        //private int count = 0; --> no use
        data.mTitleEffectStartTime = mTitleEffectStartTime;
        data.mTitleEffectEndTime = mTitleEffectStartTime;

        if( mAudioEnvelop == null ) {
            data.mAudioEnvelop = null;
        }else{
            data.mAudioEnvelop = mAudioEnvelop.getSaveData();
        }

        if( mAudioEdit == null) {
            data.mAudioEdit = null;
        }else{
            data.mAudioEdit = mAudioEdit.getSaveData();
        }

        data.mTemplateEffectID = mTemplateEffectID;
        data.mCollageDrawInfoID = mCollageDrawInfoID;
        data.mTemplateAudioPos = mTemplateAudioPos;
        data.mDrawInfos = mDrawInfos; //todo
        data.mOverlappedTransition = mOverlappedTransition;
        data.mMediaInfoUseCache = mMediaInfoUseCache;
        data.mStartTime = mStartTime;
        data.mEndTime = mEndTime;
        data.mDuration = mDuration;
        data.mInfo = mInfo;
        if( mVideoEdit == null) {
            data.mVideoEdit = null;
        }else{
            data.mVideoEdit = mVideoEdit.getSaveData();
        }
        data.mCustomLUT_A = mCustomLUT_A;
        data.mCustomLUT_B = mCustomLUT_B;
        data.mCustomLUT_Power = mCustomLUT_Power;

        if( mColorEffect == null) {
            data.mColorEffect = null;
        }else{
            data.mColorEffect = mColorEffect.getSaveData();
        }
        data.mPropertySlowVideoMode = mPropertySlowVideoMode;
        data.mAVSyncAudioStartTime = mAVSyncAudioStartTime;
        data.m_ClipVolume = m_ClipVolume;
        data.m_BGMVolume = m_BGMVolume;
        data.mRotate = mRotate;
        return data;
    }

    void loadSaveData(nexSaveDataFormat.nexClipOf data){
        if( mPath.compareTo(data.mPath) != 0 ){
            Log.d(TAG,"loadSaveData invaild path="+mPath+", input="+data.mPath);
            return;
        }

        m_BGMVolume = data.m_BGMVolume;

        //mTransCodingPath = data.mTransCodingPath;
        //mProjectAttachment = true;
        if(data.mClipEffect == null) {
            mClipEffect = null;
        }else{
            mClipEffect = new nexClipEffect(data.mClipEffect);
        }

        if( data.mTransitionEffect == null) {
            mTransitionEffect = null;
        }else{
            mTransitionEffect = new nexTransitionEffect(mObserver,data.mTransitionEffect);
        }
        misMustDownSize = data.misMustDownSize;
        //private boolean m_gettingThumbnails=false; -->internal false set
        //private boolean m_getThumbnailsFailed=false; -->internal false set
        mAudioOnOff = data.mAudioOnOff;
        //color adjust
        m_Brightness = data.m_Brightness;
        m_Contrast = data.m_Contrast;
        m_Saturation = data.m_Saturation;
        mVignette = data.mVignette;
        mFacedetectProcessed = data.mFacedetectProcessed;
        mFaceDetected = data.mFaceDetected;
        mFaceRect = data.mFaceRect;//= new Rect(); internal set

        if( data.mCrop == null) {
            mCrop = null;
        }else{
            mCrop =  new nexCrop(mPath, data.mCrop);
        }
        //private int index = 0; --> no use
        //private int count = 0; --> no use
        mTitleEffectStartTime = data.mTitleEffectStartTime;
        mTitleEffectStartTime = data.mTitleEffectEndTime;

        if( data.mAudioEnvelop == null ) {
            mAudioEnvelop = null;
        }else{
            mAudioEnvelop = new nexAudioEnvelop(data.mAudioEnvelop);
        }

        mTemplateEffectID = data.mTemplateEffectID;
        mCollageDrawInfoID = data.mCollageDrawInfoID;
        mTemplateAudioPos = data.mTemplateAudioPos;
        mDrawInfos = data.mDrawInfos; //todo
        mOverlappedTransition = data.mOverlappedTransition;
        mMediaInfoUseCache = data.mMediaInfoUseCache;
        mStartTime = data.mStartTime;
        mEndTime = data.mEndTime;
        mDuration = data.mDuration;
        mInfo = data.mInfo;
        mCustomLUT_A = data.mCustomLUT_A;
        mCustomLUT_B = data.mCustomLUT_B;
        mCustomLUT_Power = data.mCustomLUT_Power;
        mRotate = data.mRotate;

        if( data.mColorEffect == null) {
            mColorEffect = null;
        }else{
            mColorEffect = new nexColorEffect(data.mColorEffect);
        }
        mPropertySlowVideoMode = data.mPropertySlowVideoMode;
        mAVSyncAudioStartTime = data.mAVSyncAudioStartTime;
        m_ClipVolume = data.m_ClipVolume;
        m_BGMVolume = data.m_BGMVolume;

        if( data.mVideoEdit == null) {
            mVideoEdit = null;
        }else{
            mVideoEdit = new nexVideoClipEdit(this,data.mVideoEdit );
        }

        if( data.mAudioEdit == null) {
            mAudioEdit = null;
        }else{
            mAudioEdit = new nexAudioEdit(this,data.mAudioEdit);
        }
    }

    nexClip( nexObserver project, nexSaveDataFormat.nexClipOf data ){
        m_BGMVolume = data.m_BGMVolume;
        mPath = data.mPath;
        mTransCodingPath = data.mTransCodingPath;
        mProjectAttachment = true;
        if(data.mClipEffect == null) {
            mClipEffect = null;
        }else{
            mClipEffect = new nexClipEffect(data.mClipEffect);
        }

        if( data.mTransitionEffect == null) {
            mTransitionEffect = null;
        }else{
            mTransitionEffect = new nexTransitionEffect(project,data.mTransitionEffect);
        }
        misMustDownSize = data.misMustDownSize;
        //private boolean m_gettingThumbnails=false; -->internal false set
        //private boolean m_getThumbnailsFailed=false; -->internal false set
        mObserver = project;
        mAudioOnOff = data.mAudioOnOff;
        //color adjust
        m_Brightness = data.m_Brightness;
        m_Contrast = data.m_Contrast;
        m_Saturation = data.m_Saturation;
        mVignette = data.mVignette;
        mFacedetectProcessed = data.mFacedetectProcessed;
        mFaceDetected = data.mFaceDetected;
        mFaceRect = data.mFaceRect;//= new Rect(); internal set

        if( data.mCrop == null) {
            mCrop = null;
        }else{
            mCrop =  new nexCrop(mPath, data.mCrop);
        }
        //private int index = 0; --> no use
        //private int count = 0; --> no use
        mTitleEffectStartTime = data.mTitleEffectStartTime;
        mTitleEffectStartTime = data.mTitleEffectEndTime;

        if( data.mAudioEnvelop == null ) {
            mAudioEnvelop = null;
        }else{
            mAudioEnvelop = new nexAudioEnvelop(data.mAudioEnvelop);
        }

        mTemplateEffectID = data.mTemplateEffectID;
        mCollageDrawInfoID = data.mCollageDrawInfoID;
        mTemplateAudioPos = data.mTemplateAudioPos;
        mDrawInfos = data.mDrawInfos; //todo
        mOverlappedTransition = data.mOverlappedTransition;
        mMediaInfoUseCache = data.mMediaInfoUseCache;
        mStartTime = data.mStartTime;
        mEndTime = data.mEndTime;
        mDuration = data.mDuration;
        mInfo = data.mInfo;
        mCustomLUT_A = data.mCustomLUT_A;
        mCustomLUT_B = data.mCustomLUT_B;
        mCustomLUT_Power = data.mCustomLUT_Power;
        mRotate = data.mRotate;

        if( data.mColorEffect == null) {
            mColorEffect = null;
        }else{
            mColorEffect = new nexColorEffect(data.mColorEffect);
        }
        mPropertySlowVideoMode = data.mPropertySlowVideoMode;
        mAVSyncAudioStartTime = data.mAVSyncAudioStartTime;
        m_ClipVolume = data.m_ClipVolume;
        m_BGMVolume = data.m_BGMVolume;

        if( data.mVideoEdit == null) {
            mVideoEdit = null;
        }else{
            mVideoEdit = new nexVideoClipEdit(this,data.mVideoEdit );
        }

        if( data.mAudioEdit == null) {
            mAudioEdit = null;
        }else{
            mAudioEdit = new nexAudioEdit(this,data.mAudioEdit);
        }
    }

}
