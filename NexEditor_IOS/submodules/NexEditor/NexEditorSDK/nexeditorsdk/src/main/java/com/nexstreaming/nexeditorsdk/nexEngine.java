/******************************************************************************
 * File Name        : nexEngine.java
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

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.AsyncTask;
import android.os.Build;
import android.util.Log;
import android.util.SparseIntArray;
import android.view.View;
import android.graphics.Bitmap;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.utils.facedetect.FaceInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexCache;
import com.nexstreaming.kminternal.nexvideoeditor.NexDrawInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.FastPreviewOption;
import com.nexstreaming.kminternal.nexvideoeditor.NexThemeView;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;
import com.nexstreaming.nexeditorsdk.exception.ProjectNotAttachedException;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;
import com.nexstreaming.kminternal.nexvideoeditor.NexAudioClip;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditorListener;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditorType;
import com.nexstreaming.kminternal.nexvideoeditor.NexRectangle;
import com.nexstreaming.kminternal.nexvideoeditor.NexVisualClip;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;
import com.nexstreaming.nexeditorsdk.module.nexExternalExportProvider;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * 
 * This class previews {@link com.nexstreaming.nexeditorsdk.nexProject nexProject} with {@link com.nexstreaming.nexeditorsdk.nexClip nexClip} and
 * {@link com.nexstreaming.nexeditorsdk.nexEffect nexEffect} applied to it, or encodes into MP4 files. 
 * This acts as a media player and uses many resources, therefore working with a singleton pattern for the application is suggested.
 * <p>Example code :</p>
 *     {@code protected void onCreate(Bundle savedInstanceState) {
            m_editorView = (nexEngineView)findViewById(R.id.videoView);
            m_editorView.setBlackOut(true);

            mEngine = KMSDKApplication.getApplicationInstance().getEngin();
            mEngine.setView(m_editorView);
            mEngine.setEventHandler(mEditorListener);
            mEngine.setProject(project);
            mEngine.play();
       }
 *     }
 *
 * @see {@link com.nexstreaming.nexeditorsdk.nexProject nexProject}
 * @see {@link com.nexstreaming.nexeditorsdk.nexEngineListener nexEngineListener}
 * @see {@link com.nexstreaming.nexeditorsdk.nexEngineView nexEngineView}
 * @since version 1.0.0
 */
public final class nexEngine implements SurfaceHolder.Callback{
    private static final String TAG = "nexEngine";

    private nexProject mProject = null;
    private Context mAppContext;

    private NexEditor mVideoEditor;
    private NexEditorListener mEditorListener = null;
//    private NexEditorEventListener m_listener = null;
    private OnSurfaceChangeListener m_onSurfaceChangeListener = null;
    private int mState = kState_idle;
    private int mCurrentPlayTime;
    //private volatile nexEngineListener mEventListener = null;
    private nexEngineListener mEventListener = null;
    private nexEngineView.NexViewListener mSurfaceViewListener = null;

    private final static int kState_idle = 0;
    private final static int kState_load = 1;
    private final static int kState_export = 2;

    private static boolean sLoadListAsync = true;

    private static int sExportVideoTrackUUIDMode = 0;

//    private Exporter ex;
    private NexEditor.PlayState mPlayState = NexEditor.PlayState.IDLE ;

    private static boolean sTranscodeMode = false;
    private static nexProject sTranscodeProject = null;
    private static nexEngineListener sTranscodeListener = null;
    private Surface mSurface;

    private boolean mForceMixExportMode;

    private Set<nexOverlayItem> mActiveRenderLayers = new HashSet<>();
    private List<nexOverlayItem> mRenderInCurrentPass = new ArrayList<>();

    private int mExportTotalTime;
    private String mExportFilePath;
    private int mEncodeWidth;
    private int mEncodeHeight;
    private int mEncodeBitrate;
    private long mEncodeMaxFileSize;

    private static final int export_audio_sampling_rate = 44100;
    private static final int export_fps = 3000;

    private static final int PREWAKE_INTERVAL = 2000;
    private static final int PERSIST_INTERVAL = 500;

    private static int sNextId = 1;
    private int mId;
    private int lastCheckDirectExportOption = 0;
    private int lastSeekTime = 0;
    private boolean bLetterBox = false;

	// Below value is referenced by nexCAL.h
    public static final int ExportProfile_AVCBaseline = 1;
    public static final int ExportProfile_AVCMain = 2;
    public static final int ExportProfile_AVCExtended = 3;
    public static final int ExportProfile_AVCHigh = 4;
    public static final int ExportProfile_AVCHigh10 = 5;
    public static final int ExportProfile_AVCHigh422 = 6;
    public static final int ExportProfile_AVCHigh444 = 7;

    public static final int ExportCodec_AVC = 0x10010300;
    public static final int ExportCodec_HEVC = 0x10010400;
    public static final int ExportCodec_MPEG4V = 0x10020100;

    public static final int ExportProfile_HEVCMain = MediaCodecInfo.CodecProfileLevel.HEVCProfileMain;
    public static final int ExportProfile_HEVCMain10 = MediaCodecInfo.CodecProfileLevel.HEVCProfileMain10;

    public static final int ExportProfile_MPEG4VSimple = MediaCodecInfo.CodecProfileLevel.MPEG4ProfileSimple;
    public static final int ExportProfile_MPEG4VASP=MediaCodecInfo.CodecProfileLevel.MPEG4ProfileAdvancedSimple;

    public static final int ExportAVCLevel1 = MediaCodecInfo.CodecProfileLevel.AVCLevel1;
    public static final int ExportAVCLevel1b = MediaCodecInfo.CodecProfileLevel.AVCLevel1b;
    public static final int ExportAVCLevel11 = MediaCodecInfo.CodecProfileLevel.AVCLevel11;
    public static final int ExportAVCLevel12 = MediaCodecInfo.CodecProfileLevel.AVCLevel12;
    public static final int ExportAVCLevel13 = MediaCodecInfo.CodecProfileLevel.AVCLevel13;
    public static final int ExportAVCLevel2 = MediaCodecInfo.CodecProfileLevel.AVCLevel2;
    public static final int ExportAVCLevel21 = MediaCodecInfo.CodecProfileLevel.AVCLevel21;
    public static final int ExportAVCLevel22 = MediaCodecInfo.CodecProfileLevel.AVCLevel22;
    public static final int ExportAVCLevel3 = MediaCodecInfo.CodecProfileLevel.AVCLevel3;
    public static final int ExportAVCLevel31 = MediaCodecInfo.CodecProfileLevel.AVCLevel31;
    public static final int ExportAVCLevel32 = MediaCodecInfo.CodecProfileLevel.AVCLevel32;
    public static final int ExportAVCLevel4 = MediaCodecInfo.CodecProfileLevel.AVCLevel4;
    public static final int ExportAVCLevel41 = MediaCodecInfo.CodecProfileLevel.AVCLevel41;
    public static final int ExportAVCLevel42 = MediaCodecInfo.CodecProfileLevel.AVCLevel42;
    public static final int ExportAVCLevel5 = MediaCodecInfo.CodecProfileLevel.AVCLevel5;
    public static final int ExportAVCLevel51 = MediaCodecInfo.CodecProfileLevel.AVCLevel51;
    public static final int ExportAVCLevel52 = MediaCodecInfo.CodecProfileLevel.AVCLevel52;

    public static final int ExportHEVCMainTierLevel1 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel1;
    public static final int ExportHEVCHighTierLevel1 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel1;
    public static final int ExportHEVCMainTierLevel2 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel2;
    public static final int ExportHEVCHighTierLevel2 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel2;
    public static final int ExportHEVCMainTierLevel21 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel21;
    public static final int ExportHEVCHighTierLevel21 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel21;
    public static final int ExportHEVCMainTierLevel3 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel3;
    public static final int ExportHEVCHighTierLevel3 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel3;
    public static final int ExportHEVCMainTierLevel31 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel31;
    public static final int ExportHEVCHighTierLevel31 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel31;
    public static final int ExportHEVCMainTierLevel4 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel4;
    public static final int ExportHEVCHighTierLevel4 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel4;
    public static final int ExportHEVCMainTierLevel41 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel41;
    public static final int ExportHEVCHighTierLevel41 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel41;
    public static final int ExportHEVCMainTierLevel5 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel5;
    public static final int ExportHEVCHighTierLevel5 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel5;
    public static final int ExportHEVCMainTierLevel51 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel51;
    public static final int ExportHEVCHighTierLevel51 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel51;
    public static final int ExportHEVCMainTierLevel52 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel52;
    public static final int ExportHEVCHighTierLevel52 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel52;
    public static final int ExportHEVCMainTierLevel6 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel6;
    public static final int ExportHEVCHighTierLevel6 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel6;
    public static final int ExportHEVCMainTierLevel61 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel61;
    public static final int ExportHEVCHighTierLevel61 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel61;
    public static final int ExportHEVCMainTierLevel62 = MediaCodecInfo.CodecProfileLevel.HEVCMainTierLevel62;
    public static final int ExportHEVCHighTierLevel62 = MediaCodecInfo.CodecProfileLevel.HEVCHighTierLevel62;

    public static final int ExportMPEG4Level0 = MediaCodecInfo.CodecProfileLevel.MPEG4Level0;
    public static final int ExportMPEG4Level0b = MediaCodecInfo.CodecProfileLevel.MPEG4Level0b;
    public static final int ExportMPEG4Level1 = MediaCodecInfo.CodecProfileLevel.MPEG4Level1;
    public static final int ExportMPEG4Level2 = MediaCodecInfo.CodecProfileLevel.MPEG4Level2;
    public static final int ExportMPEG4Level3 = MediaCodecInfo.CodecProfileLevel.MPEG4Level3;
    public static final int ExportMPEG4Level4 = MediaCodecInfo.CodecProfileLevel.MPEG4Level4;
    public static final int ExportMPEG4Level4a = MediaCodecInfo.CodecProfileLevel.MPEG4Level4a;
    public static final int ExportMPEG4Level5 = MediaCodecInfo.CodecProfileLevel.MPEG4Level5;

    /**
     * Use {@link #checkDirectExport(int)} , {@link #directExport(String, long, long, int)}
     * @since 1.7.50
     * @see #checkDirectExport(int)
     * @see #directExport(String, long, long, int)
     */
    public static final int DirectExportOption_None = 0;

    /**
     * Use {@link #checkDirectExport(int)} , {@link #directExport(String, long, long, int)}
     *
     * @since 1.7.50
     * @see #checkDirectExport(int)
     * @see #directExport(String, long, long, int)
     */
    public static final int DirectExportOption_AudioEncode = 1;

    /**
     *
     * @param value
     * @since 2.0.0
     */
    public void setBrightness(int value){

        if(mVideoEditor != null)
            mVideoEditor.setBrightness(value);
    }

    /**
     *
     * @param value
     * @since 2.0.0
     */
    public void setContrast(int value){

        if(mVideoEditor != null)
            mVideoEditor.setContrast(value);
    }

    /**
     *
     * @param value
     * @since 2.0.0
     */
    public void setSaturation(int value){

        if(mVideoEditor != null)
            mVideoEditor.setSaturation(value);
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public int getBrightness(){

        if(mVideoEditor != null)
            return mVideoEditor.getBrightness();
        return 0;
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public int getContrast(){

        if(mVideoEditor != null)
            return mVideoEditor.getContrast();
        return 0;
    }

    /**
     *
     * @return
     * @since 2.0.0
     */
    public int getSaturation(){

        if(mVideoEditor != null)
            return mVideoEditor.getSaturation();
        return 0;
    }

    /**
     *
     * @param value
     * @since 2.0.0
     */
    public void setVignette(int value) {

        if (mVideoEditor != null)
            mVideoEditor.setVignette(value);
    }

    /**
     *
     * @return
     * @since 2.0.10
     */
    public int getVignette() {

        if (mVideoEditor != null)
            return mVideoEditor.getVignette();
        return 0;
    }

    /**
     *
     * @return
     * @since 2.0.10
     */
    public int getVignetteRange() {

        if (mVideoEditor != null)
            return mVideoEditor.getVignetteRange();
        return 0;
    }

    /**
     *
     * @param value
     * @since 2.0.10
     */
    public void setVignetteRange(int value) {

        if (mVideoEditor != null)
            mVideoEditor.setVignetteRange(value);
    }   

    /**
     *
     * @param value
     * @since 2.0.10
     */
    public void setSharpness(int value) {

        if (mVideoEditor != null)
            mVideoEditor.setSharpness(value);
    }

    /**
     *
     * @return
     * @since 2.0.10
     */
    public int getSharpness() {

        if (mVideoEditor != null)
            return mVideoEditor.getSharpness();
        return 0;
    }

    /**
     *
     * @param lightLevel
     * @since 2.0.0
     */
    public void setDeviceLightLevel(int lightLevel)
    {
        if(mVideoEditor != null)
            mVideoEditor.setDeviceLightLevel(lightLevel);
    }

    /**
     *
     * @param gamma
     * @since 2.0.0
     */
    public void setDeviceGamma(float gamma)
    {
        if(mVideoEditor != null)
            mVideoEditor.setDeviceGamma(gamma);
    }

    /**
     *
     * @param listener
     * @since 2.0.14
     */
    public void setSurfaceViewListener(nexEngineView.NexViewListener listener){
        mSurfaceViewListener = listener;
    }

    int surfaceView_init_Width = 0;
    int surfaceView_init_Height = 0;

    public void setDefaultSurfaceSize(int width, int height){
        surfaceView_init_Width = width;
        surfaceView_init_Height = height;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated called() getMeasuredWidth="+mSurfaceView.getMeasuredWidth()+",getMeasuredHeight="+mSurfaceView.getMeasuredHeight());
        if( mSurfaceView == null ){
            return;
        }
        mSurface = holder.getSurface();

        if(mVideoEditor != null) {
            mVideoEditor.prepareSurfaceForView(mSurface);

            mVideoEditor.notifySurfaceChanged();

            if( surfaceView_init_Width == 0 ) {
                surfaceView_init_Width = mSurfaceView.getMeasuredWidth();
            }

            if( surfaceView_init_Height == 0 ) {
                surfaceView_init_Height = mSurfaceView.getMeasuredHeight();
            }

            if( mSurfaceViewListener != null ) {
                mSurfaceViewListener.onEngineViewAvailable(surfaceView_init_Width,surfaceView_init_Height);
            }
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

        if (width == 0 || height == 0) {
            Log.e(TAG, "invalid video width(" + width + ") or height(" + height
                    + ")");
            return;
        }

        if( surfaceView_init_Width == 0 ) {
            surfaceView_init_Width = width;
        }

        if( surfaceView_init_Height == 0 ) {
            surfaceView_init_Height = height;
        }

        float aspectRatio = nexApplicationConfig.getAspectRatio();
        Log.v(TAG, "surfaceChanged called("+width+","+height+"), init size=("+surfaceView_init_Width+","+surfaceView_init_Height+")  aspect="+aspectRatio);
        if( (aspectRatio * height ) != width ) {

            int aspectWidth = surfaceView_init_Width;
            int aspectHeight = surfaceView_init_Height;
            if (aspectRatio > 0) {
                if (surfaceView_init_Height  > surfaceView_init_Width) {
                    aspectHeight = Math.round(surfaceView_init_Width / aspectRatio);
                    if( aspectHeight > surfaceView_init_Height){
                        aspectHeight = surfaceView_init_Height;
                        aspectWidth = Math.round(aspectHeight * aspectRatio);
                    }
                } else {
                    aspectWidth = Math.round(surfaceView_init_Height * aspectRatio);
                    if( aspectWidth > surfaceView_init_Width){
                        aspectWidth = surfaceView_init_Width;
                        aspectHeight = Math.round(aspectWidth / aspectRatio);
                    }
                }
            }

            //Log.d(TAG, "surfaceChanged aspect view size " + aspectWidth + "x" + aspectHeight);

            if (aspectWidth != width || aspectHeight != height) {
                Log.d(TAG, "surfaceChanged new view size " + aspectWidth + "x" + aspectHeight);
                holder.setFixedSize(aspectWidth, aspectHeight);
                return;
            }
        }

        Log.d(TAG, "surfaceChanged new surface.");
        mSurface = holder.getSurface();
        if (mVideoEditor != null) {
            mVideoEditor.prepareSurfaceForView(mSurface);
            mVideoEditor.notifySurfaceChanged();
        }

        if( mSurfaceViewListener != null ) {
            mSurfaceViewListener.onEngineViewSizeChanged(width,height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v(TAG, "surfaceDestroyed called()");
        if( holder != null) {
            if( mSurface == holder.getSurface() ){
                mSurface = null;
            }
        }
        if(mVideoEditor != null) {
            mVideoEditor.prepareSurfaceForView(null);
        }

        if( mSurfaceViewListener != null ) {
            mSurfaceViewListener.onEngineViewDestroyed();
        }

    }

    /**
     * This abstract class handles the listener used when <tt>stop()</tt> is called. 
     * @since version 1.0.0
     * @see #stop()
     * @see #exportSaveStop(OnCompletionListener)
     */
    public static abstract class OnCompletionListener {
        public abstract void onComplete( int resultCode );
    }
  
    /**
     * This abstract class handles the listener used when <tt>seekIDROnly()</tt> is called.
     * @see #seekIDROnly(int, OnSeekCompletionListener)
     * @since version 1.3.34
     */
    public static abstract class OnSeekCompletionListener {
        public abstract void onSeekComplete( int resultCode , int seekTime, int movedTime);
    }


    /**
     *  This class is a listener for <tt>setOnSurfaceChangeListener</tt>.
     * @since version 1.0.0
     * @see #setOnSurfaceChangeListener(OnSurfaceChangeListener)
     */
    public static abstract class OnSurfaceChangeListener {
        public abstract void onSurfaceChanged();
    }

   /**
     * The initial generator of <tt>nexEngine</tt>.
     *
     * <p>Example code :</p>
     *      {@code   private nexEngine mEngine;
               mEngin = new nexEngine(mContext);
            }
     * @param context The android context.
     * @since version 1.0.0
     */
    public nexEngine(Context context){
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexEngine create");

        mAppContext = context;
        mVideoEditor = EditorGlobal.getEditor();
        //mVideoEditor.setAspectMode(nexApplicationConfig.getAspectRatioMode());
        NexEditor.setLayerScreen(nexApplicationConfig.getAspectProfile().getWidth(), nexApplicationConfig.getAspectProfile().getHeight(),nexApplicationConfig.getOverlayCoordinateMode());
        mVideoEditor.setScreenMode(nexApplicationConfig.getScreenMode());
        //Layer
        mVideoEditor.setCustomRenderCallback(m_layerRenderCallback);
        //mVideoEditor.setTheme(EditorGlobal.DEFAULT_THEME_ID, true);
        //loadEffectsInEditor(true);

            if (mVideoEditor.getSDKInfo_WM() == 1) {
                mVideoEditor.setWatermark(true);
            }else{
                mVideoEditor.setWatermark(false);
            }

    }

    nexEngine(Context context, boolean setMark){
        mId = sNextId++;
        Log.d(TAG,"["+mId+"] nexEngine create internal");

        mAppContext = context;
        mVideoEditor = EditorGlobal.getEditor();
        //mVideoEditor.setAspectMode(nexApplicationConfig.getAspectRatioMode());
        NexEditor.setLayerScreen(nexApplicationConfig.getAspectProfile().getWidth(), nexApplicationConfig.getAspectProfile().getHeight(),nexApplicationConfig.getOverlayCoordinateMode());
        mVideoEditor.setScreenMode(nexApplicationConfig.getScreenMode());
        //Layer
        mVideoEditor.setCustomRenderCallback(m_layerRenderCallback);
        if( setMark ){
            setMark();
        }
    }

    void setMark(){

            if (mVideoEditor.getSDKInfo_WM() == 1) {
                mVideoEditor.setWatermark(true);
            }else{
                mVideoEditor.setWatermark(false);
            }
    }

    /**
     * This method registers <tt>nexEngineListener</tt> of <tt>nexEngine</tt>.
     * <p>For Example :</p>
     {@code mEngine.setEventHandler( new nexEngineListener(){
            @literal @Override
             public void onStateChange(int oldState, int newState) {

             }
             ...............
             @literal @Override
             public void onEncodingProgress(int percent) {

             }
     };);

     }
     * @param listener <tt>nexEngineListener</tt>
     * @since version 1.0.0
     */
    public void setEventHandler( nexEngineListener listener ) {
        Log.d(TAG,"["+mId+"] setEventHandler ="+listener);
        mEventListener = listener;
        setEditorListener();
    }

    /**
     * This method allows to choose whether to operate <tt>updateProject()</tt> asynchronously or synchronously.  
     * When this method operates as default synchronously, <tt>updateProject()</tt> must be called after all commands are completed and the engine is in the state of <tt>Stop</tt>.
     *
     * <p>Example code :</p>
     *      {@code
                mEngin.setLoadListAsync(true);
            }
     * @param  set <tt>TRUE</tt> to operate asynchronously, or <tt>FALSE</tt> to operate synchronously.
     * @since version 1.3.43
     */
    public static void setLoadListAsync(boolean set){
        sLoadListAsync = set;
    }

    private void setEditorListener(){
        if( mEditorListener != null )
            return;
        mEditorListener = new NexEditorListener() {
            @Override
            public void onStateChange(NexEditor.PlayState oldState, NexEditor.PlayState newState) {
                Log.i(TAG, "["+mId+"]onStateChange() oldState=" + oldState + ", newState=" + newState+",curState="+mState+", trans="+sTranscodeMode);
                if( oldState == NexEditor.PlayState.NONE &&  newState == NexEditor.PlayState.IDLE) {
                    mState = kState_load;
                }else{
                    cacheSeekMode = false;
                }

                if( sTranscodeMode ){
                    if (mState == kState_export) {
                        if (oldState == NexEditor.PlayState.RECORD && newState != NexEditor.PlayState.RECORD) {
                            if (sTranscodeListener != null) {
                                sTranscodeListener.onStateChange(oldState.getValue(), newState.getValue());
                            }
                        }
                    }
                }else {
                    mPlayState = newState;
                    if (mState == kState_export) {
                        if (oldState == NexEditor.PlayState.RECORD && newState != NexEditor.PlayState.RECORD) {
                            if (newState == NexEditor.PlayState.PAUSE) {
                                Log.d(TAG, "["+mId+"]new State is " + newState);
                            } else if (newState == NexEditor.PlayState.RESUME) {
                                Log.d(TAG, "["+mId+"]new State is " + newState);
                            } else {
                                mState = kState_load;
                            }
                        }
                    }

                    if (mEventListener != null) {
                        mEventListener.onStateChange(oldState.getValue(), newState.getValue());
                    }
                }
            }

            @Override
            public void onTimeChange(int currentTime) {
                if( externalImageExportProcessing ){
                    return;
                }

                if( sTranscodeMode ){
                    if( sTranscodeListener != null ){
                        int percent = 0;
                        int duration = mExportTotalTime;
                        if (duration <= 0) {
                            duration = sTranscodeProject.getTotalTime();
                        }
                        if (duration <= 0) {
                            percent = currentTime / 10;
                        } else {
                            percent = currentTime * 100 / duration;
                        }
                        if (percent >= 0 && percent <= 100)
                            sTranscodeListener.onEncodingProgress(percent);
                    }
                }else {
                    // Log.i(TAG, "onTimeChange() =" + currentTime);
                    mCurrentPlayTime = currentTime;
                    if (mEventListener != null) {
                        if( mState == kState_export || mPlayState == NexEditor.PlayState.RECORD){
                            int percent = 0;
                            int duration = mExportTotalTime;
                            if (duration <= 0) {
                                duration = mProject.getTotalTime();
                            }

                            if (duration <= 0) {
                                //percent = currentTime / 10;
                                percent = 0;
                            }else {
                                percent = currentTime * 100 / duration;
                            }

                            Log.d(TAG,"["+mId+"]export progress = "+percent+", duration="+duration+", currentTime="+currentTime);

                            if (percent >= 0 && percent <= 100)
                                mEventListener.onEncodingProgress(percent);
                        }else {
                            mEventListener.onTimeChange(currentTime);
                        }
                    }
                }
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                if( externalImageExportProcessing ){
                    return;
                }

                if( mState != kState_export
                        && mEventListener != null )
                    mEventListener.onSetTimeDone(currentTime);
                mCurrentPlayTime = currentTime;
            }

            @Override
            public void onSetTimeFail(NexEditor.ErrorCode err) {
                if( externalImageExportProcessing ){
                    return;
                }

                if( mEventListener != null )
                    mEventListener.onSetTimeFail(err.getValue());
            }

            @Override
            public void onSetTimeIgnored() {
                if( externalImageExportProcessing ){
                    return;
                }

                if( mEventListener != null )
                    mEventListener.onSetTimeIgnored();
            }

            @Override
            public void onEncodingDone(NexEditor.ErrorCode result) {
                mState = kState_load;
                Log.i(TAG, "["+mId+"]onEncodingDone() ="+result+", trans="+sTranscodeMode+", forceMix="+mForceMixExportMode);
/*
                if( mForceMixExportMode ){
                    resolveProject(true);
                }
*/
                if( sTranscodeMode ){
                    if( sTranscodeListener != null ){
                        if (!result.isError()) {
                            sTranscodeListener.onEncodingProgress(100);
                        }
                        sTranscodeListener.onEncodingDone(result.isError(), result.getValue());
                    }
                    sTranscodeMode = false;
                    resolveProject(true,true);
                }else {
                    if (mEventListener != null) {
                        if (!result.isError()) {
                            mEventListener.onEncodingProgress(100);
                        }
                        mEventListener.onEncodingDone(result.isError(), result.getValue());
                    }
                }
                mExportTotalTime = 0;
            }

            @Override
            public void onPlayEnd() {
                mState = kState_load;
                if( mEventListener != null )
                    mEventListener.onPlayEnd();
            }

            @Override
            public void onPlayFail(NexEditor.ErrorCode err, int iClipID) {
                if( mEventListener != null )
                    mEventListener.onPlayFail(err.getValue(),iClipID);
            }

            @Override
            public void onPlayStart() {
                if( mEventListener != null )
                    mEventListener.onPlayStart();
            }

            @Override
            public void onClipInfoDone() {
                if( mEventListener != null )
                    mEventListener.onClipInfoDone();
            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {
                if( mEventListener != null )
                    mEventListener.onSeekStateChanged(isSeeking);
            }

            @Override
            public void onCheckDirectExport(NexEditor.ErrorCode err, int result) {
                if( mEventListener != null ) {
                    mEventListener.onCheckDirectExport(result);
                }
            }

            @Override
            public void onProgressThumbnailCaching(int progress, int maxValue) {
                if( mEventListener != null ) {
                    if(progress > maxValue) {
                        progress = 100;
                    }
                    mEventListener.onProgressThumbnailCaching(progress, maxValue);
                }
            }

            @Override
            public void onReverseProgress(int progress, int maxValue) {
                if(mEventListener != null) {
                    if(progress > maxValue) {
                        progress = 100;
                    }

                    // TODO: 2015. 08. 27 rooney
                    // this method must be changed to mEventListener.onReverseProgress(progress)
                    // but reverse method(onReverseProgress) is not used in LGE. so temporarily we use onEncodingProgress(progress)
                    mEventListener.onEncodingProgress(progress);
                }
            }

            @Override
            public void onReverseDone(NexEditor.ErrorCode result) {
                // TODO: 2015. 08. 27 rooney
                // this method must be changed to mEventListener.onReverseDone(result)
                // but reverse method(onReverseDone) is not used in LGE. so temporarily we use onEncodingDone(result)
                if( mEventListener != null ) {
                    mEventListener.onEncodingDone(result.isError(), result.getValue());
                }
            }

            @Override
            public void onFastPreviewStartDone(NexEditor.ErrorCode result, int iStartTime, int iEndTime) {
                if( mEventListener != null ) {
                    mEventListener.onFastPreviewStartDone(result.getValue(), iStartTime, iEndTime);
                }
            }

            @Override
            public void onFastPreviewStopDone(NexEditor.ErrorCode result) {
                if( mEventListener != null ) {
                    mEventListener.onFastPreviewStopDone(result.getValue());
                }
            }

            @Override
            public void onFastPreviewTimeDone(NexEditor.ErrorCode result) {
                if( mEventListener != null ) {
                    mEventListener.onFastPreviewTimeDone(result.getValue());
                }
            }

            @Override
            public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {
                if( mEventListener != null ) {
                    mEventListener.onPreviewPeakMeter(iCts, iLeftValue, iRightValue);
                }
            }
        };
        mVideoEditor.setEventHandler(mEditorListener);
    }

    /**
     *  This method registers a listener to receive an event when the surface changes.
     *
     * <p>Example code :</p>
     *      {@code    mEngine.setOnSurfaceChangeListener(new nexEngine.OnSurfaceChangeListener() {
                @Override
                public void onSurfaceChanged() {
                    if( !mSeek ){
                        mSeek = true;
                        mEngine.updateProject();
                        mEngine.seek(1000);
                        }
                    }
                });
            }
     * @param listener A listener made by inheriting <tt>nexEngine.OnSurfaceChangeListener</tt>.
     * @since version 1.0.0
     */
    public void setOnSurfaceChangeListener(OnSurfaceChangeListener listener) {
        m_onSurfaceChangeListener = listener;
        mVideoEditor.setOnSurfaceChangeListener(new NexEditor.OnSurfaceChangeListener() {
            @Override
            public void onSurfaceChanged() {
                if (m_onSurfaceChangeListener != null)
                    m_onSurfaceChangeListener.onSurfaceChanged();
            }
        });
    }

    /**
     * This method registers a view of an application. 
     *
     * <p>Example code :</p>
     *      {@code    private nexEngineView m_editorView;

                m_editorView = (nexEngineView)findViewById(R.id.videoView_bgmtest);
                m_editorView.setBlackOut(true);
                mEngine = KMSDKApplication.getApplicationInstance().getEngin();
                mEngine.setView(m_editorView);
            }
     * @param view The surface view.
     * @since version 1.0.0
     * @see #getView()
     */
    public int setView(nexEngineView view) {
        Log.d(TAG, "[" + mId + "] setView nexEngineView=" + view);
        if( mSurfaceView != null ){
            mSurfaceView.getHolder().removeCallback(this);
            mSurfaceView = null;
        }
        mVideoEditor.setView(view);
        return 0;
    }

    private SurfaceView mSurfaceView;
    public int setView(SurfaceView view) {
        Log.d(TAG,"["+mId+"] setView SurfaceView="+view);
        mVideoEditor.setView(null);

        if( view != mSurfaceView ){
            if( mSurfaceView != null ){
                mSurfaceView.getHolder().removeCallback(this);
            }
        }
        mSurfaceView = view;
        if( mSurfaceView != null) {
            mSurfaceView.getHolder().addCallback(this);
        }
        return 0;
    }

    /**
     * This method returns a nexEngineView.
     * @return nexEngineView set by setView()
     * @since 1.7.16
     * @see #setView(nexEngineView)
     */
    public nexEngineView getView( ) {
        return (nexEngineView)mVideoEditor.getView();
    }


    /**
     * This method sets a project to be played and encoded.
     *
     * <p>Example code :</p>
     *      {@code    nexClip clip=nexClip.getSupportedClip(m_listfilepath.get(0));
                if( clip != null ){
                    project.add(clip);
                }
                mEngine.setProject(project);
            }
     * @param project The project to set.
     * @since version 1.0.0
     * @see #getProject()
     */
    public void setProject(nexProject project){
        Log.d(TAG,"["+mId+"] setProject");

        stopAsyncFaceDetect();

        mProject = project;
        defaultFaceDetectSetting();
        //mProjectOverlays = null;
    }
    public void clearFaceDetectInfo(){

        if(null != mProject){

            mProject.clearFaceDetectInfo();
        }
    }

    /**
     * This method gets a project currently set to <tt>nexEngine</tt>.
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.getProject().getClip(0,true).setBrightness(value);
            }
     * @return The project set to <tt>nexEngine</tt>.
     * @throws ProjectNotAttachedException
     * @since version 1.0.0
     * @see #setProject(nexProject)
     */
    public nexProject getProject(){
        return mProject;
    }

    private boolean isSetProject(boolean exception){
        if( mProject != null ){
            if( mProject.getTotalClipCount(true) > 0 ){
                return true;
            }
        }

        if( sTranscodeMode){
            if( sTranscodeProject != null ){
                if( sTranscodeProject.getTotalClipCount(true) > 0 ){
                    return true;
                }
            }
        }

        if(exception) {
            throw new ProjectNotAttachedException();
        }
        return false;
    }

    /**
     *  This method will play content from the beginning if the project is already set.
     *
     * <p>Example code :</p>
     *      {@code    mEngine.setProject(project);
                mEngine.play();
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     * @see #pause()
     * @see #resume()
     * @see #stop()
     * @see #seek(int)
     */
    public void play(){
        cacheSeekMode = false;
        if( isSetProject(false) ) {
            if( mState == kState_export) {
                Log.w(TAG, "["+mId+"]export state");
                return;
            }
            stopAsyncFaceDetect();
            setOverlays(OverlayCommand.upload);
            resolveProject(sLoadListAsync,true);
            faceDetect_internal(facedetect_asyncmode, facedetect_syncclip_count, facedetect_undetected_clip_cropping_mode);

            //mVideoEditor.setTheme(mProject.getThemeId(), false);
            loadEffectsInEditor(false);
            setEditorListener();
            mVideoEditor.startPlay();
        }
    }


    /**
     * This is the same play() method, but without throwing any exception.
     * @param noException true - no exception. false - the same as void play().
     * @return true- no exception thrown,  false - exception thrown.
     * @since 1.7.8
     * @see #play()
     */
    public boolean play(boolean noException){
        cacheSeekMode = false;
        if( noException ) {
            try {
                play();
            } catch (Exception e) {
                return false;
            }
        }else{
            play();
        }
        return true;
    }
    /**
     *  This method will stop the content if the project is already set.
     *
     * <p>Example code :</p>
     *      {@code    if( mExportStat ) {
                    mEngine.stop();
                    mExportStat = false;
                }
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     * @see #pause()
     * @see #resume()
     * @see #play()
     * @see #seek(int)
     */
    public void stop(){
        if( externalImageExportProcessing) {
            mVideoEditor.exportImageFormatCancel();
            mState = kState_load;
            return;
        }

        cacheSeekMode = false;
        if( isSetProject(false) ) {
            if(mState == kState_export) {
                mState = kState_load;
                mVideoEditor.stop();
            } else {
                mState = kState_load;
                mVideoEditor.stop();
            }
        }
    }

    /**
     * This method allows to stop and save what's been exported so far while <tt>export()</tt>.
     * 
     * <p>Example code :</p>
     *      {@code    mEngine.exportSaveStop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        Log.d(TAG, "Save export : "+resultCode);
                    }
                });
            }
     * @param listener OnCompletionListener
     * @see #stop()
     * @since version 1.3.43
     */
    public void exportSaveStop(final nexEngine.OnCompletionListener listener){
        if( externalImageExportProcessing ){
            return;
        }
        if( isSetProject(false) ) {
            mState = kState_load;
            mVideoEditor.stop(1,new NexEditor.OnCompletionListener() {
                @Override
                public void onComplete(NexEditor.ErrorCode resultCode) {
                    listener.onComplete(resultCode.getValue());
                    mState = kState_load;
                }
            });
        }else{
            listener.onComplete(21);
        }
    }

    /**
     * This method returns the completion time of <tt>Stop()</tt> using <tt>OnCompletionListener</tt>. 
     * 
     * <p>Example code :</p>
     *      {@code    mEngine.stop(new nexEngine.OnCompletionListener() {
                    @Override
                    public void onComplete(int resultCode) {
                        mEngine.getProject().getClip(0,true).getClipEffect().setEffectShowTime(mStartTime, mEndTime);
                        mEngine.play();
                    }
                });
            }
     * @param listener OnCompletionListener
     * @see #play()
     * @see #resume()
     * @see #export(String, int, int, int, long, int)
     * @since version 1.3.43
     */
    public void stop( final nexEngine.OnCompletionListener listener){
        if( externalImageExportProcessing) {
            mVideoEditor.exportImageFormatCancel();
            if( listener != null ){
                listener.onComplete(0);
            }
            mState = kState_load;
            return;
        }

        cacheSeekMode = false;
        if( isSetProject(false) ) {
            mState = kState_load;
            mVideoEditor.stop(new NexEditor.OnCompletionListener() {
                @Override
                public void onComplete(NexEditor.ErrorCode resultCode) {
                    listener.onComplete(resultCode.getValue());
                }
            });
        }else{
            listener.onComplete(21);
        }
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public void stopSync() {
        if( isSetProject(false) ) {
            //
            Log.i(TAG,"["+mId+"]stopSync start");
            if (mPlayState == NexEditor.PlayState.NONE || mPlayState == NexEditor.PlayState.IDLE) {
                Log.i(TAG,"["+mId+"]stopSync IDLE state");
                mState = kState_load;
                return;
            }

            mVideoEditor.setSyncMode(true);
            mVideoEditor.stop();

            while(true){
                if (mPlayState == NexEditor.PlayState.IDLE ){
                    break;
                }

                try {
                    //Log.i(TAG,"stopSync sleep");
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            mState = kState_load;
            mVideoEditor.setSyncMode(false);
            Log.i(TAG, "["+mId+"]stopSync End!");
        }
    }

    /**
     *  This method will pause the content if the project is already set.
     *
     * <p>Example code :</p>
     *      {@code    if( !isSeeking ){
                    mEngine.pause();
                    isSeeking = true;
                }
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     * @see #resume()
     * @see #play()
     * @see #stop()
     * @see #seek(int)
     */
    public void pause(){
        if( isSetProject(false) ) {
            mVideoEditor.stop();
        }
    }

    /**
     *  This method will resume the content if the project is already set.
     *
     * <p>Example code :</p>
     *      {@code    if( mCurrentPlayTime > 0 ){
                    mEngine.seek(mCurrentPlayTime);
                }
                mEngine.resume();
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     * @see #pause()
     * @see #play()
     * @see #stop()
     * @see #seek(int)
     */
    public void resume(){
        cacheSeekMode = false;
        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            faceDetect_internal(facedetect_asyncmode, facedetect_syncclip_count, facedetect_undetected_clip_cropping_mode);
            mVideoEditor.startPlay();
        }
    }

    /**
     * This method seeks for content if the project is already set.
     * For this method to work properly, methods like <tt>stop()</tt> or <tt>pause()</tt> should be called before.
     * Otherwise, <tt>seek()</tt> method will fail and an event listener <tt>onSetTimeFail()</tt> will occur.
     *
     * <p>Example code :</p>
     *      {@code    if( mCurrentPlayTime > 0 ){
                    mEngine.seek(mCurrentPlayTime);
                }
                mEngine.resume();
            }
     * @param time The seek time in <tt>msec</tt> (milliseconds).
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     * @see #pause()
     * @see #resume()
     * @see #play()
     * @see #stop()
     */
    public void seek(int time){
        if( cacheSeekMode ){
            Log.d(TAG,"["+mId+"] seek fail! cacheSeekMode");
            return ;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            stopAsyncFaceDetect();
            faceDetect_for_seek(time, facedetect_undetected_clip_cropping_mode);
            lastSeekTime = time;
            mVideoEditor.seek(time);
        }
    }


    public void seek(int time, final OnSeekCompletionListener listener){
        if( cacheSeekMode ){
            Log.d(TAG,"["+mId+"] seek fail! cacheSeekMode");
            return ;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            stopAsyncFaceDetect();
            faceDetect_for_seek(time, facedetect_undetected_clip_cropping_mode);
            lastSeekTime = time;
            mVideoEditor.seek(time, new NexEditor.OnSetTimeDoneListener() {
                @Override
                public void onSetTimeDone(int time, int iFrameTime) {
                    if( listener != null){
                        listener.onSeekComplete(0,time,iFrameTime);
                    }
                }

                @Override
                public void onSetTimeFail(NexEditor.ErrorCode returnCode) {
                    if( listener != null){
                        listener.onSeekComplete(returnCode.getValue(),0,0);
                    }
                }

                @Override
                public String getSetTimeLabel() {
                    return "seekLis";
                }
            });
        }
    }


    /**
     * This method seeks for the closest I-frame, before the input seek time value by the parameter <tt>time</tt>, of video content.
     * Therefore, the actual seek result shown on the preview might be different from the input seek time. 
     * 
     * <p>Example code :</p>
     *      {@code    if(mSeekMode == 1) {
                    mEngine.seek(ms);
                } else {
                    mEngine.seekIDRorI(ms);
                }
            }
     * @param time The seek time in <tt>msec</tt> millisecond.
     * @see #seekIDROnly(int, com.nexstreaming.nexeditorsdk.nexEngine.OnSeekCompletionListener OnSeekCompletionListener)
     * @see #seekIDROnly(int)
     * @see #seek(int)
     * @since version 1.3.43
     */
    public void seekIDRorI(int time){
        if( cacheSeekMode ){
            Log.d(TAG,"["+mId+"] seekIDRorI fail! cacheSeekMode");
            return ;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            lastSeekTime = time;
            mVideoEditor.seekIOnly(time, null);
        }
    }

    /**
     * This method seeks for the closest IDR-frame, before the input seek time value by the parameter <tt>time</tt>, of video content.
     * Therefore, the actual seek result shown on the preview might be different from the input seek time. 
     * 
     * <p>Example code :</p>
     *      {@code
                mEngine.seekIDROnly(seekTime);
            }
     * @param time The seek time in <tt>msec</tt> millisecond.
     * @see #seekIDROnly(int, com.nexstreaming.nexeditorsdk.nexEngine.OnSeekCompletionListener OnSeekCompletionListener)
     * @see #seekIDRorI(int)
     * @see #seek(int)
     * @since version 1.3.43
     */
    public void seekIDROnly(int time){
        if( cacheSeekMode ){
            Log.d(TAG,"["+mId+"] seekIDROnly fail! cacheSeekMode");
            return ;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            lastSeekTime = time;
            mVideoEditor.seekIDROnly(time,null);
        }
    }

    /**
     * This method seeks for the closest IDR-frame, before the input seek time value by the parameter <tt>time</tt>, of video content.
     * Therefore, the actual seek result shown on the preview might be different from the input seek time. 
     * To get the actual seek frame, use the parameter <tt>listener</tt> to get the value from parameter <tt>iFrameTime</tt> of <tt>onSeekComplete(int time, int iFrameTime)</tt>.
     *
     * <p>Example code :</p>
     *      {@code    mEngine.seekIDROnly(seek, new nexEngine.OnSeekCompletionListener() {
                    @Override
                    public void onSeekComplete(int resultCode, int seekTime, int movedTime) {
                        if( resultCode == 0 ){
                            Log.d(TAG,"onSeekComplete: seekTime="+seekTime+", movedTime="+movedTime);
                        }
                    }
                });
            }
     * @param time The seek time in <tt>msec</tt> millisecond.
     * @param listener OnSeekCompletionListener. Notifies when the <tt>seek</tt> has completed and returns the actual seek location of an IDR-frame.
     * @see #seekIDROnly(int)
     * @see #seekIDRorI(int)
     * @see #seek(int)
     * @since version 1.3.43
     */
    public void seekIDROnly(int time , final OnSeekCompletionListener listener){
        if( cacheSeekMode ){
            Log.d(TAG,"["+mId+"] seekIDROnly fail! cacheSeekMode");
            return ;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);
            lastSeekTime = time;
            mVideoEditor.seekIDROnly(time, new NexEditor.OnSetTimeDoneListener() {
                @Override
                public void onSetTimeDone(int time, int iFrameTime) {
                    if( listener != null ){
                        listener.onSeekComplete(0,time,iFrameTime);
                    }
                }

                @Override
                public void onSetTimeFail(NexEditor.ErrorCode returnCode) {
                    if( listener != null ){
                        listener.onSeekComplete(returnCode.getValue(),0,0);
                    }
                }

                @Override
                public String getSetTimeLabel() {
                    return "seekIDROnly";
                }
            });
        }
    }

    private boolean cacheSeekMode = false;

    /**
     *
     * @return
     * @since 1.7.58
     */
    public boolean isCacheSeekMode(){
        return cacheSeekMode;
    }

    /**
     *
     * @return
     * @since 1.7.58
     */
    public boolean startCollectCache(int time, final OnCompletionListener listener){
        if( isSetProject(false) ) {

            if( MediaInfo.isMediaTaskBusy() ){
                if( listener != null ){
                    listener.onComplete(NexEditor.ErrorCode.THUMBNAIL_BUSY.getValue());
                }
                return false;
            }

            cacheSeekMode = true;
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);

            mVideoEditor.startSeekCollectCache(time, new NexEditor.OnSetTimeDoneListener() {
                @Override
                public void onSetTimeDone(int time, int iFrameTime) {
                    if( listener != null ){
                        listener.onComplete(0);
                    }
                }

                @Override
                public void onSetTimeFail(NexEditor.ErrorCode returnCode) {
                    cacheSeekMode = false;
                    if( listener != null ){
                        listener.onComplete(returnCode.getValue());
                    }
                }

                @Override
                public String getSetTimeLabel() {
                    return null;
                }
            });
            return true;
        }
        return false;
    }

    /**
     *
     * @return
     * @since 1.7.58
     */
    public boolean seekFromCache(int time){
        if( !cacheSeekMode ){
            return false;
        }

        if( isSetProject(false) ) {
            undoForceMixProject();
            setEditorListener();
            setOverlays(OverlayCommand.upload);

            mVideoEditor.seekFromCache(time,null);
            return true;
        }
        return false;
    }

    /**
     *
     * @return
     * @since 1.7.58
     */
    public Bitmap getThumbnailCache(int time,int rotation){
        if( !cacheSeekMode ){
            return null;
        }

        Bitmap src = mVideoEditor.getCacheThumbData(time);
        if( src == null ){
            return null;
        }
        int width = src.getWidth();
        int height = src.getHeight();
        Rect rect = new Rect(0,0,width,height);
        if( rotation==180 ) {
            Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            canvas.rotate(180,width/2,height/2);
            canvas.drawBitmap(src,rect,  new Rect(0,0,width,height), null);
            return bm;
        } else if( rotation==0 ) {
            Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            canvas.drawBitmap(src,rect, new Rect(0,0,width,height), null);
            return bm;
        } else if( rotation==270 ) {
            Bitmap bm = Bitmap.createBitmap(height, width, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            canvas.rotate(90,0,0);
            canvas.drawBitmap(src,rect, new Rect(0,-height,width,0), null);
            return bm;
        } else if( rotation==90 ) {
            Bitmap bm = Bitmap.createBitmap(height, width, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            canvas.rotate(270,0,0);
            canvas.drawBitmap(src,rect, new Rect(-width,0,0,height), null);
            return bm;
        }
        return null;
    }

    /**
     *
     * @return
     * @since 1.7.58
     */
    public void stopCollectCache(int time){
        cacheSeekMode = false;
        seek(time);
    }

    /**
     * This method encodes a project.
     *
     * <p>Example code :</p>
     *      {@code    File file = getExportFile(width, height);
                mEngine.export(file.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0);
            }
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @param iSamplingRate The Sampling rate tag that matches the sampling rate in the mp4 file.(Hz) (ex: 44100 )
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @return -1 if the SDK is already in export state; 0 if successful.
     * @since version 1.5.38
     */
    public int export(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate, final int iSamplingRate){
        return export( strExportFile,  iEncodeWidth,  iEncodeHeight,  iEncodeBitrate,  iEncodeMaxFileSize,  iRotate,  iSamplingRate, 0,0, export_fps, ExportCodec_AVC);
    }

    /**
     * This method encodes a project.
     *
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @param iSamplingRate The Sampling rate tag that matches the sampling rate in the mp4 file.
     * @param fps The value is (fps * 1000) (ex: 30fps -> 30 * 100 = 3000 )
     * @return -1 if the SDK is already in export state; 0 if successful.
     * @since version 1.5.38
     */
    public int export(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate, final int iSamplingRate, final int fps){
         return export( strExportFile,  iEncodeWidth,  iEncodeHeight,  iEncodeBitrate,  iEncodeMaxFileSize,  iRotate,  iSamplingRate,  0,  0, fps, ExportCodec_AVC);
    }

    /**
     * This method encodes a project.
     *
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @param iSamplingRate The Sampling rate tag that matches the sampling rate in the mp4 file.
     * @param iEncodeProfile H.264 profile
     * <ul>
     *     <li>1 - baseline</li>
     *     <li>2 - main</li>
     *     <li>4 - high</li>
     * </ul>
     * @param iEncodeLevel H.264 level {@link android.media.MediaCodecInfo.CodecProfileLevel}
     * @param fps The value is (fps * 1000) (ex: 30fps -> 30 * 100 = 3000 )
     * @param iVideoCodecType encoding codec type
     * <ul>
     *     <li>0x10010300 - H264</li>
     *     <li>0x10010400 - H265</li>
     * </ul>
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @return -1 if the SDK is already in export state; 0 if successful.
     * @since 1.7.2
     */
 public int export(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate, final int iSamplingRate, final int iEncodeProfile, final int iEncodeLevel, final int fps, final int iVideoCodecType){
        if( mState == kState_export){
            Log.w(TAG,"["+mId+"]already export state");
            return -1;
        }

        if( isSetProject(true) ) {
            stopAsyncFaceDetect();
            setOverlays(OverlayCommand.upload);
            resolveProject(sLoadListAsync,true);
	     if(mProject.getTemplateApplyMode() == 3)		 
	         facedetect_asyncmode = false;

            faceDetect_internal(facedetect_asyncmode, facedetect_syncclip_count, facedetect_undetected_clip_cropping_mode);
            mState = kState_export;
            setThumbnailRoutine(2);
            mExportTotalTime = mProject.getTotalTime();
            mExportFilePath = strExportFile;
            mEncodeWidth = iEncodeWidth;
            mEncodeHeight = iEncodeHeight;
            mEncodeBitrate = iEncodeBitrate;
            mEncodeMaxFileSize = iEncodeMaxFileSize;

            mVideoEditor.detectAndSetEditorColorFormat(mAppContext).onComplete(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    //mVideoEditor.setTheme(mProject.getThemeId(), true);
                    loadEffectsInEditor(true);
                    setEditorListener();

                    int iFlag = 0;
                    switch (iRotate){
                        case 90:
                            iFlag |= 0x10;
                            break;
                        case 180:
                            iFlag |= 0x20;
                            break;
                        case 270:
                            iFlag |= 0x40;
                            break;
                    }
                    if(bNeedScaling) {
                        bNeedScaling = false;
                        iFlag |= 0x00100000/*HQ_SCALE_FLAG*/;
                    }
                    mVideoEditor.setExportListener(null);
                    mVideoEditor.setExportVideoTrackUUID(sExportVideoTrackUUIDMode,null);
                    mVideoEditor.export(mExportFilePath, mEncodeWidth, mEncodeHeight, mEncodeBitrate, mEncodeMaxFileSize, 0, false, iSamplingRate, iEncodeProfile, iEncodeLevel, fps, iVideoCodecType ,iFlag).onFailure(new Task.OnFailListener() {
                        @Override
                        public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                            Log.e(TAG,"["+mId+"]export fail!="+failureReason);
                        }
                    });

                }
            });
            return 0;
        }
        return -1;
    }

    /**
     * This is the same export() method, but without throwing any exception.
     *
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @param iSamplingRate The Sampling rate tag that matches the sampling rate in the mp4 file.
     * @param iEncodeProfile H.264 profile
     * <ul>
     *     <li>1 - baseline</li>
     *     <li>2 - main</li>
     *     <li>4 - high</li>
     * </ul>
     * @param iEncodeLevel H.264 level {@link android.media.MediaCodecInfo.CodecProfileLevel}
     * @param fps The value is (fps * 1000) (ex: 30fps -> 30 * 100 = 3000 )
     * @param iVideoCodecType encoding codec type
     * <ul>
     *     <li>0x10010300 - H264</li>
     *     <li>0x10010400 - H265</li>
     * </ul>
     * @return -1 if the SDK is already in export state;  -2 called Exception, 0 if successful.
     * @since 1.7.51
     */
    public int exportNoException(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate, final int iSamplingRate, final int iEncodeProfile, final int iEncodeLevel, final int fps, final int iVideoCodecType){
        int ret = 0;
        try {
            ret = export( strExportFile,  iEncodeWidth,  iEncodeHeight,  iEncodeBitrate,
             iEncodeMaxFileSize,  iRotate,  iSamplingRate,
             iEncodeProfile,  iEncodeLevel, fps, iVideoCodecType);
        }catch (Exception e){
            return -2;
        }
        return ret;
    }

    /**
     * This is the same export() method, but without throwing any exception.
     *
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @param iSamplingRate The Sampling rate tag that matches the sampling rate in the mp4 file.
     * @param iEncodeProfile H.264 profile
     * <ul>
     *     <li>1 - baseline</li>
     *     <li>2 - main</li>
     *     <li>4 - high</li>
     * </ul>
     * @param iEncodeLevel H.264 level {@link android.media.MediaCodecInfo.CodecProfileLevel}
     * @param fps The value is (fps * 1000) (ex: 30fps -> 30 * 100 = 3000 )
     * @return -1 if the SDK is already in export state;  -2 called Exception, 0 if successful.
     * @since 1.7.16
     */
    public int exportNoException(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate, final int iSamplingRate, final int iEncodeProfile, final int iEncodeLevel, final int fps){
        int ret = 0;
        try {
            ret = export( strExportFile,  iEncodeWidth,  iEncodeHeight,  iEncodeBitrate,
             iEncodeMaxFileSize,  iRotate,  iSamplingRate,
             iEncodeProfile,  iEncodeLevel, fps, ExportCodec_AVC);
        }catch (Exception e){
            return -2;
        }
        return ret;
    }
	
    /**
     * @brief This method encodes a project.
     *
     * <p>Example code :</p>
     *      {@code    File file = getExportFile(width, height);
    mEngine.export(file.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0);
    }
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iEncodeWidth The width of the content to be encoded.
     * @param iEncodeHeight The height of the content to be encoded.
     * @param iEncodeBitrate The bitrate of the content to be encoded.
     * @param iEncodeMaxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
     * @param iRotate The rotate tag that matches the iRotate angle in the MP4 file.
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @return -1 if already export state; 0 if successful.
     * @since version 1.0.0
     */
    public int export(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iRotate){
        return export( strExportFile,  iEncodeWidth,  iEncodeHeight,  iEncodeBitrate,  iEncodeMaxFileSize,  iRotate,  export_audio_sampling_rate,  0,  0, export_fps, ExportCodec_AVC);
    }

    /**
     * @brief This method encodes a project to jpeg.
     *
     * <p>Example code :</p>
     *      {@code    File file = getExportFile(width, height);
                        mEngine.exportJpeg(file.getAbsolutePath(), width, height, quality);
    }
     * @param strExportFile This is a string indicating the path to where the exported file is saved.
     * @param iWidth The width of the content to be encoded.
     * @param iHeight The height of the content to be encoded.
     * @param iQuality The quality of the content to be encoded.
     * @param listener OnCaptureListener.
     * @return -1 if already export state; 0 if successful.
     * @since version 2.0.0
     */
    public nexErrorCode exportJpeg(String strExportFile, int iWidth, int iHeight, int iQuality, final OnCaptureListener listener){
        if( listener == null ){
            return nexErrorCode.ARGUMENT_FAILED;
        }

        NexEditor.ErrorCode errorCode = mVideoEditor.exportJpeg(iWidth, iHeight, 0, new NexEditor.OnCaptureListener() {
            @Override
            public void onCapture(Bitmap bm) {
                listener.onCapture(bm);
            }

            @Override
            public void onCaptureFail(NexEditor.ErrorCode error) {
                listener.onCaptureFail(nexErrorCode.fromValue(error.getValue()));
            }
        });
        return nexErrorCode.fromValue(errorCode.getValue());
    }

    private boolean externalImageExportProcessing = false;

    private class externalExport extends NexEditor.externalExportImageCallback{
        private nexExternalExportProvider externalCallback;
        private nexExportListener listener;
        public externalExport(nexExternalExportProvider callback, nexExportListener listener ){
            externalCallback = callback;
            this.listener = listener;
        }

        @Override
        public boolean OnCaptuerData(byte[] data, int format) {
            return externalCallback.OnPushData(format,data);
        }

        @Override
        public void OnEos() {
            externalCallback.OnLastProcess();
        }

        @Override
        public boolean OnEnd(int error) {
            // 0 : ok , -1 : timeout , -2 : cancel , -3 : OnCaptuerData fail! , -4 ~ -6 : surface fail!
            nexErrorCode replaceError = nexErrorCode.EXPORT_WRITER_START_FAIL;
            switch (error){
                case 0:
                    replaceError = nexErrorCode.NONE;
                    break;
                case -1:
                    replaceError = nexErrorCode.INVALID_STATE;
                    break;
                case -2:
                    replaceError = nexErrorCode.EXPORT_USER_CANCEL;
                    break;
                case -3:
                    replaceError = nexErrorCode.EXPORT_WRITER_INIT_FAIL;
                    break;
            }

            boolean reval = externalCallback.OnEnd(error);

            if( error < 0 ) {
                if( listener != null) {
                    listener.onExportFail(replaceError);
                }
            }

            if( listener != null) {
                listener.onExportDone(null);
            }else {
                if (mEventListener != null)
                    mEventListener.onEncodingDone(error == 0 ? false:true,replaceError.getValue());
            }

            externalImageExportProcessing = false;
            mState = kState_load;
            return reval;
        }

        @Override
        public void OnProgress(int precent) {

            if( listener != null) {
                listener.onExportProgress(precent);
            }else {
                if( mEventListener != null )
                    mEventListener.onEncodingProgress(precent);
            }
        }

        @Override
        public int waitCount() {
            return async_facedetect_worker_list_.size();
        }
    }

    private NexEditor.externalExportImageCallback getExternalExport(nexExportFormat format, String name, String uuid, final nexExportListener listener){
        if( uuid != null ){
            Object callback = nexExternalModuleManager.getInstance().getModule(uuid);

            if( callback != null){
                if( nexExternalExportProvider.class.isInstance(callback)) {
                    if (((nexExternalExportProvider)callback).OnPrepare(format)) {
                        return new externalExport((nexExternalExportProvider)callback, listener);
                    }
                }
            }
            return null;
        }

        nexExternalExportProvider callback = (nexExternalExportProvider)nexExternalModuleManager.getInstance().getModule(name, nexExternalExportProvider.class);
        if( callback != null){
            if( callback.OnPrepare(format) ) {
                return new externalExport(callback, listener);
            }
        }
        return null;
    }

    /**
     * @brief This method encodes a project to jpeg.
     *
     * <p>Example code :</p>
     *      {@code    File file = getExportFile(width, height);
    mEngine.exportJpeg(file.getAbsolutePath(), width, height, quality);
    }
     * @param format This is a string indicating the path to where the exported file is saved.
     * @param listener nexExportListener.
     * @return -1 if already export state; 0 if successful.
     * @since version 2.0.0
     */
    public nexErrorCode export(nexExportFormat format, final nexExportListener listener){

        String type = format.getString(nexExportFormat.TAG_FORMAT_TYPE);

        if( type == null ) {
            return nexErrorCode.ARGUMENT_FAILED;
        }

        if( type.startsWith("external-") ){
            //
            if( mState == kState_export){
                Log.w(TAG,"["+mId+"]already external export state");
                return nexErrorCode.INVALID_STATE;
            }

            int width = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_WIDTH));
            int height = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_HEIGHT));
            int intervalTime = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_INTERVAL_TIME));
            int startTime = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_START_TIME));
            int endTime = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_END_TIME));

            if( endTime > mProject.getTotalTime() ){
                endTime = mProject.getTotalTime();
            }

            if( startTime > endTime ){
                return nexErrorCode.ARGUMENT_FAILED;
            }

            String uuid = format.formats.get(nexExportFormat.TAG_FORMAT_UUID);
            NexEditor.externalExportImageCallback callback = getExternalExport(format,type.substring(9),uuid,listener);
            if( callback == null) {
                return nexErrorCode.DIRECTEXPORT_ENC_ENCODE_ERROR;
            }

            // mVideoEditor.setExternalExportImageCallback(callback);
            externalImageExportProcessing = true;
            if( NexEditor.ErrorCode.NONE == mVideoEditor.exportImageFormat(width,height,intervalTime,startTime,endTime, callback) ){
                mState = kState_export;
                return nexErrorCode.NONE;
            }

            externalImageExportProcessing = false;

            callback.OnEos();
            callback.OnEnd(-1);

            if( listener != null )
                listener.onExportFail(nexErrorCode.INVALID_STATE);
        }

        if( type.compareTo("bitmap") == 0 ) {

            int width = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_WIDTH));
            int height = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_HEIGHT));
            int tag = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_TAG));

            if(  listener == null) {
                return nexErrorCode.ARGUMENT_FAILED;
            }

            int flag = tag;
            NexEditor.ErrorCode errorCode = mVideoEditor.exportJpeg(width, height, flag, new NexEditor.OnCaptureListener() {
                @Override
                public void onCapture(Bitmap bm) {
                    listener.onExportDone(bm);
                }

                @Override
                public void onCaptureFail(NexEditor.ErrorCode error) {
                    listener.onExportFail(nexErrorCode.fromValue(error.getValue()));
                }
            });
            return nexErrorCode.fromValue(errorCode.getValue());
        }

        if( type.compareTo("jpeg") == 0 ) {

            final String path = format.formats.get(nexExportFormat.TAG_FORMAT_PATH);
            int width = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_WIDTH));
            int height = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_HEIGHT));
            final int quality = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_QUALITY));

            if (path == null || path.length() <= 0 || width <= 0 || height <= 0 || quality <= 0 || quality > 100 ) {
                return nexErrorCode.ARGUMENT_FAILED;
            }

            NexEditor.ErrorCode errorCode = mVideoEditor.exportJpeg(width, height, 0, new NexEditor.OnCaptureListener() {
                @Override
                public void onCapture(Bitmap bm) {
                    NexEditor.ErrorCode code = NexEditor.ErrorCode.NONE;
                    File f = new File(path);

                    if (f.exists())
                        f.delete();
                    try {
                        FileOutputStream out = new FileOutputStream(f);
                        bm.compress(Bitmap.CompressFormat.JPEG, quality, out);
                        out.flush();
                        out.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                        code = NexEditor.ErrorCode.EXPORT_WRITER_INIT_FAIL;
                    }
                    if( listener != null) {
                        listener.onExportDone(null);
                    }else{
                        if( mEditorListener != null)
                            mEditorListener.onEncodingDone(code);
                    }
                }

                @Override
                public void onCaptureFail(NexEditor.ErrorCode error) {
                    if( listener != null) {
                        listener.onExportFail(nexErrorCode.fromValue(error.getValue()));
                    }else{
                        if( mEditorListener != null)
                            mEditorListener.onEncodingDone(error);
                    }
                }
            });
            return nexErrorCode.fromValue(errorCode.getValue());
        }

        if( type.compareTo("mp4") == 0 ) {
            String path;
            int width;
            int height;

            int videocodec;
            int videobitrate;
            int videoprofile;
            int videolevel;
            int videorotate;
            int videofps;

            int audiosamplerate;
            long maxfilesize;

            try {
                path = format.formats.get(nexExportFormat.TAG_FORMAT_PATH);
                width = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_WIDTH));
                height = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_HEIGHT));

                videocodec = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_CODEC));
                videobitrate = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_BITRATE));
                videoprofile = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_PROFILE));
                videolevel = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_LEVEL));
                videorotate = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_ROTATE));
                videofps = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_VIDEO_FPS));

                audiosamplerate = Integer.parseInt(format.formats.get(nexExportFormat.TAG_FORMAT_AUDIO_SAMPLERATE));

                maxfilesize = Long.parseLong(format.formats.get(nexExportFormat.TAG_FORMAT_MAX_FILESIZE));
            }catch (Exception e) {
                return nexErrorCode.ARGUMENT_FAILED;
            }

            int ret = 0;
            try {
                ret = export(path, width, height, videobitrate, maxfilesize, videorotate, audiosamplerate, videoprofile, videolevel, videofps, videocodec);
            }catch (Exception e){
                return nexErrorCode.UNKNOWN;
            }

            if( ret == 0 ) {
                mVideoEditor.setExportListener(new NexEditor.OnExportListener() {
                    @Override
                    public void onExportFail(NexEditor.ErrorCode error) {
                        listener.onExportFail(nexErrorCode.fromValue(error.getValue()));
                    }

                    @Override
                    public void onExportProgress(int cur) {
                        int p = cur * 100 /  mProject.getTotalTime();
                        listener.onExportProgress(p);
                    }

                    @Override
                    public void onExportDone() {
                        listener.onExportDone(null);
                    }
                });
                return nexErrorCode.NONE;
            }
            return nexErrorCode.UNKNOWN;
        }

        return nexErrorCode.UNSUPPORT_FORMAT;
    }


    private int transcode(String strExportFile, int iEncodeWidth, int iEncodeHeight, int iEncodeBitrate, long iEncodeMaxFileSize, final int iSamplingRate, final int iRotate){
        if( mState == kState_export){
            Log.w(TAG,"["+mId+"]already export state");
            return -1;
        }

        if( isSetProject(true) ) {
            setOverlays(OverlayCommand.upload);
            resolveProject(sLoadListAsync,true);

            mState = kState_export;
            mExportTotalTime = sTranscodeProject.getTotalTime();
            mExportFilePath = strExportFile;
            mEncodeWidth = iEncodeWidth;
            mEncodeHeight = iEncodeHeight;
            mEncodeBitrate = iEncodeBitrate;
            mEncodeMaxFileSize = iEncodeMaxFileSize;

            mVideoEditor.detectAndSetEditorColorFormat(mAppContext).onComplete(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    //mVideoEditor.setTheme(mProject.getThemeId(), true);
                    loadEffectsInEditor(true);
                    setEditorListener();

                    int iFlag = 0;
                    switch (iRotate){
                        case 90:
                            iFlag |= 0x10;
                            break;
                        case 180:
                            iFlag |= 0x20;
                            break;
                        case 270:
                            iFlag |= 0x40;
                            break;
                    }
                    if(bNeedScaling) {
                        bNeedScaling = false;
                        iFlag |= 0x00100000/*HQ_SCALE_FLAG*/;
                    }

                    iFlag |= 0x1000; //transcode
                    mVideoEditor.setExportVideoTrackUUID(sExportVideoTrackUUIDMode,null);
                    mVideoEditor.export(mExportFilePath, mEncodeWidth, mEncodeHeight, mEncodeBitrate, mEncodeMaxFileSize, 0, false, 3000, 0, 0, iSamplingRate, iFlag).onFailure(new Task.OnFailListener() {
                        @Override
                        public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                            Log.e(TAG,"["+mId+"]transcode fail!="+failureReason);
                        }
                    });

                }
            });
            return 0;
        }
        return -1;
    }

    boolean bNeedScaling = false;

    /**
     * This is an API that can scale the sticker size, which is normally set to 720p.
     *
     * @param bNeedScaling Set to <tt>TRUE</tt> to turn on the scaling motion; otherwise, <tt>FALSE</tt>. 
     *
     * @note Please note that scaling may cause a delay in exporting time.
     *
     * @since version 1.5.25
     */
    public void setScalingFlag2Export(boolean bNeedScaling) {
        this.bNeedScaling = bNeedScaling;
    }

    /**
     * This method pauses the application during an export.
     *
     * <p>Example code :</p>
     *     {@code
                mEngine.exportPause();
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.5.19
     * @see #exportResume()
     * @return Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
     */
    public int exportPause(){
        if( externalImageExportProcessing){
            return -1;
        }
        return mVideoEditor.encodePause();
    }

    /**
     * This method resumes the export that has been paused.
     *
     * <p>Example code :</p>
     *     {@code
                mEngine.exportResume();
            }
     * @throws ProjectNotAttachedException The case where there is no project created or no clip is created within a project.
     * @since version 1.5.19
     * @see #exportPause()
     * @return Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
     */
    public int exportResume(){
        if( externalImageExportProcessing){
            return -1;
        }

        return mVideoEditor.encodeResume();
    }

    /**
     * This is a possible return value for {@link #checkDirectExport()} when the export is possible.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_OK = 0;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is no engine handle available.
     *
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_InvalidHandle = 1;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is an invalid clip in the project.
     *
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_InvalidClipList = 2;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is no clip in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_ClipCountZero = 3;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is an invalid video file in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_InvalidVideoInfo = 4;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is a video with unsupported file format in the project.
     *
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_UnmatchedVideoCodec = 5;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when a clip is trimmed into two separate clips, the first frame of the second clip is not an I-frame.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_NotStartIFrame = 6;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is an image in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_HasImageClip = 7;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is an overlay image in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_HasVideoLayer = 8;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is a transition effect in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_SetTransitionEffect = 9;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when there is a clip effect in the project.
     * 
     * @since version 1.3.4
     */
    public static final int retCheckDirectExport_SetClipEffect = 10;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when DSI is not supported by the encoder.
     *
     * @since version 1.3.43
     */
    public static final int retCheckDirectExport_EncoderDSIMismatch = 11;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when a clip's speed has been adjusted.
     *
     * @since version 1.3.43
     */
    public static final int retCheckDirectExport_HasSpeedControl = 12;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when the codec is not supported by the {@link #checkDirectExport()}.
     *
     * @since version 1.3.43
     */
    public static final int retCheckDirectExport_UnsupportedCodec = 13;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when the rotate information of clips is different.
     *
     * @since version 1.3.43
     */
    public static final int retCheckDirectExport_InvalidRotate = 14;

    /**
     * This is a possible return value for {@link #checkDirectExport()} when a filter is applied on a clip in the project.
     *
     * @since version 2.0.0.2
     */
    public static final int retCheckDirectExport_FilterApplied = 15;

    /**
     * This method checks whether encoding operation is necessary for the export. 
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.checkDirectExport(1);
            }
     * @param iOption options.
     *                0 : Default.
     *                1 : Support speed control while checkDirectExport.
     * @return One of the value listed above.
     * @since version 1.7.48
     */

    public int checkDirectExport(int iOption){
        if(mProject.getOverlayItems().isEmpty() == false) {
            return retCheckDirectExport_HasVideoLayer;
        }
        setEditorListener();
        lastCheckDirectExportOption = iOption;
        return mVideoEditor.checkDirectExport(iOption);
    }

    /**
     * This method checks whether encoding operation is necessary for the export.
     *
     * <p>Example code :</p>
     *      {@code
    mEngine.checkDirectExport();
    }
     * @return One of the value listed above.
     * @since version 1.3.4
     */
    public int checkDirectExport(){
        return checkDirectExport(0);
    }


    /**
     * This method operates an export without encoding. 
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.directExport(mFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime());
            }
     * @param strExportFile   The path of the file to be exported. 
     * @param iEncodeMaxFileSize  The maximum file size of the file to be exported. 
     * @param iMaxFileDuration The total duration of the original video content. 
     * @param iOption options.
     *                0 : Default.
     *                1 : Support speed control while directExport.
     * @return <tt>false</tt> if there is no project or failed to {@link #checkDirectExport(int iOption)}; otherwise <tt>true</tt>.
     * @since version 1.7.48
     */
    public boolean directExport(String strExportFile,long iEncodeMaxFileSize, long iMaxFileDuration, int iOption ){
        if( isSetProject(true) ) {
//            resolveProject(sLoadListAsync);

            mState = kState_export;
            if(strExportFile == null){
                return false;
            }
            mExportTotalTime = mProject.getTotalTime();
            mExportFilePath = strExportFile;
            mEncodeMaxFileSize = iEncodeMaxFileSize;
            setEditorListener();
            mVideoEditor.setExportVideoTrackUUID(sExportVideoTrackUUIDMode, null);
            mVideoEditor.directExport(mExportFilePath, mEncodeMaxFileSize, iMaxFileDuration,EditorGlobal.getProjectEffect("up"), iOption);
            return true;
        }
        return false;
    }

    /**
     * This method operates an export without encoding.
     *
     * <p>Example code :</p>
     *      {@code
    mEngine.directExport(mFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime());
    }
     * @param strExportFile   The path of the file to be exported.
     * @param iEncodeMaxFileSize  The maximum file size of the file to be exported.
     * @param iMaxFileDuration The total duration of the original video content.
     * @return <tt>false</tt> if there is no project or failed to {@link #checkDirectExport(int iOption)}; otherwise <tt>true</tt>.
     * @since version 1.3.4
     */
    public boolean directExport(String strExportFile,long iEncodeMaxFileSize, long iMaxFileDuration ){
        return directExport( strExportFile, iEncodeMaxFileSize, iMaxFileDuration, lastCheckDirectExportOption );
    }

    /**
     * This method get to support directExport from p-frame of content on current device.
     *
     * <p>Example code :</p>
     *  {@code
                if( mEngine.checkPFrameDirectExportSync(mFile.getAbsolutePath() )
                {
                    // support p-frame direct export.
                }
                else
                {
                    // unsupport p-frame direct export.
                }
        }
     * @param strFilePath   The path of the file to compared.
     * @return <tt>true</tt> if there is supported content otherwise <tt>false</tt>.
     * @since version 1.7.8
     */
    public boolean checkPFrameDirectExportSync(String strFilePath )
    {
        int iRet = mVideoEditor.checkPFrameDirectExportSync(strFilePath);

        if( iRet == 0 )
            return true;
        return false;
    }

    /**
     * This method initializes the editor engine for fast preview.
     * This method operates asynchronously.
     * 
     * <p>Example code :</p>
     *      {@code
                mEngine.fastPreviewStart(iStartTime, iStartTime + 6000, 1920, 1080);
            }
     * @param iStartTime    The start time of the preview. 
     * @param iEndTime      The end time of the preview.  
     * @param displayWidth  The width of the preview output screen. 
     * @param displayHeight The height of the preview output screen. 
     *
     * @return Always return <tt>true</tt> and the results will be returned to <tt>onFastPreviewStartDone</tt>.
     * @see #fastPreviewTime(int)
     * @see #fastPreviewStop()
     * @since version 1.3.29
     */
    @Deprecated
    private boolean fastPreviewStart(int iStartTime, int iEndTime, int displayWidth, int displayHeight) {
        mVideoEditor.fastPreviewStart(iStartTime, iEndTime, displayWidth, displayHeight);
        return true;
    }
 
    /**
     * This method sets the display time (timestamp) that is initialized by the <tt>fastPreviewStart</tt>.
     * This method operates asynchronously. 
     * 
     * <p>Example code :</p>
     *      {@code    int iTime = progress + mStartTime;
                mEngine.fastPreviewTime(iTime);
            }
     * @param iSetTime  The desired display time (timestamp) of fast preview.
     * @return Always return <tt>true</tt> and the result will be returned to <tt>onFastPreviewTimeDone</tt>.
     * @see #fastPreviewStart(int, int, int, int)
     * @see #fastPreviewStop()
     * @since version 1.3.29
     */
    @Deprecated
    private boolean fastPreviewTime(int iSetTime) {
        mVideoEditor.fastPreviewTime(iSetTime);
        return true;
    }
 
    /**
     * This method de-initialize the fast preview state of editor engine.
     * This method operates asynchronously. 
     * 
     * <p>Example code :</p>
     *      {@code
                mEngine.fastPreviewStop();
            }
     * @return  Always return <tt>true</tt> and the result will be returned to <tt>onFastPreviewStopDone</tt>.
     *
     * @see #fastPreviewStart(int, int, int, int)
     * @see #fastPreviewTime(int)
     * @since version 1.3.52
     */
    @Deprecated
    private boolean fastPreviewStop() {
        mVideoEditor.fastPreviewStop();
        return true;
    }
  
    /**
     * This method allows to reverse play video content using the information set with the parameters. 
     * This method operates asynchronously. 
     * 
     * <p>Example code :</p>
     *      {@code    String srcPath = mListFilePath.get(0);
                mDstFilePath = getReverseDstFilePath();
                String temppath = getReverseTempFilePath();
                int width = mProject.getClip(0, true).getWidth();
                int height = mProject.getClip(0, true).getHeight();
                int bitrate = mProject.getClip(0, true).getVideoBitrate();
                long maxFileSize = Long.MAX_VALUE;
                mEngine.reverseStart(srcPath, mDstFilePath, temppath, width, height, bitrate, maxFileSize, mStartT, mEndT);
            }
     * @param strSrcClipPath    The location where the original video content is stored. 
     * @param strDstClipPath    The location where the output file is saved. 
     * @param strTempClipPath   The path temporarily needed for operating this method. 
     * @param iWidth            The width of the output content. 
     * @param iHeight           The height of the output content.
     * @param iBitrate          The bitrate of the output content.
     * @param lMaxFileSize      The maximum file size for the output content.
     * @param iStartTime        The timestamp of the original video content where the reverse playback will start from. 
     * @param iEndTime          The timestamp of the original video content where the reverse playback will end. 
     * 
     * @return <tt>true</tt> if the task for <tt>reverseStart</tt> starts, otherwise <tt>false</tt>. 
     * @see #reverseStop()
     * @since version 1.3.32
     */
    public boolean reverseStart(String strSrcClipPath, String strDstClipPath, String strTempClipPath, int iWidth, int iHeight, int iBitrate, long lMaxFileSize, int iStartTime, int iEndTime, int iDecodeMode)
    {
        if(iEndTime-iStartTime < 500){
            throw new InvalidRangeException(500, iEndTime-iStartTime, true);
        }
        int ret = mVideoEditor.reverseStart(strSrcClipPath, strDstClipPath, strTempClipPath, iWidth, iHeight, iBitrate, lMaxFileSize, iStartTime, iEndTime, iDecodeMode);
        if(ret == 0)
            return true;
        else
            return false;
    }
 
    /**
     * This method stops the reverse play of video content that's currently reverse playing. 
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.reverseStop();
            }
     * @return <tt>true</tt> if successful, otherwise <tt>false</tt>. 
     * @see #reverseStart(String, String, String, int, int, int, long, int, int, int)
     * @since version 1.3.32
     */
    public boolean reverseStop()
    {
        int ret = mVideoEditor.reverseStop();
        if(ret == 0)
            return true;
        else
            return false;
    }

    /**
     * This method starts transcoding video content using the information set with the parameters.
     * @param strSrcClipPath  The location where the original video content is stored.
     * @param strDstClipPath  The location where the output file is saved.
     * @param iWidth          The width of the output content.
     * @param iHeight         The height of the output content.
     * @param iBitrate        The bitrate of the output content.
     * @param lMaxFileSize    The maximum file size for the output content.
     * @param iFPS            The FPS of the output content.
     * @param iTag            The Rotate flag.
     * @return                 <tt>true</tt> if successful, otherwise <tt>false</tt>.
     * @see #transcodingStop()
     *
     * @since version 1.5.15
     */
	public boolean transcodingStart(String strSrcClipPath, String strDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long lMaxFileSize, int iFPS, int iTag)
	{
		// public ErrorCode transcodingStart_internal(String strSrcClipPath, String strDstClipPath, int iWidth, int iHeight, int iBitrate, long iMaxFileSize, int iFPS, String strUserData) {

		NexEditor.ErrorCode ret = mVideoEditor.transcodingStart_internal(strSrcClipPath, strDstClipPath, iWidth, iHeight, iDisplayWidth, iDisplayHeight, iBitrate, lMaxFileSize, iFPS, iTag, EditorGlobal.getProjectEffect("up"));
		if( !ret.isError() )
			return true;
		else
			return false;
	}

    /**
     * This method stops transcoding video content that's currently transcoding.
     * @return <tt>true</tt> if successful, otherwise <tt>false</tt>.
     * @see #transcodingStart(String, String, int, int, int, int, int, long, int, int)
     *
     * @since version 1.5.15
     */
	public boolean transcodingStop()
	{
		NexEditor.ErrorCode ret = mVideoEditor.transcodingStop_internal();
		if( !ret.isError() )
			return true;
		else
			return false;
	}

    /**
     * This method controls the volume while preview is playing.
     *
     * @param masterVolume The video clip volume.
     * @param slaveVolume The audio clip volume.
     * @return <tt>TRUE</tt> if successful, otherwise <tt>FALSE</tt>.
     * @see #setTotalAudioVolumeResetWhilePlay()
     *
     * @since version 1.5.15
     */
    public boolean setTotalAudioVolumeWhilePlay(int masterVolume, int slaveVolume)
    {
        if(masterVolume == 100) {
            masterVolume = 101;
        }

        if(slaveVolume == 100) {
            slaveVolume = 101;
        }

        if( masterVolume < 0 || masterVolume > 200 )
            return false;

        if( slaveVolume < 0 || slaveVolume > 200 )
            return false;

        int ret = mVideoEditor.setVolumeWhilePlay(masterVolume, slaveVolume);
        if( ret == 0 )
            return true;
        return false;
    }

    /**
     * This method gets the volume while preview is playing.
     *
     * @return <tt>int[]-> 0:masterVolume, 1:slaveVolume </tt> if successful, otherwise <tt>NULL</tt>.
     * @see #setTotalAudioVolumeWhilePlay()
     *
     * @since version 2.0.13
     */

    public int[] getTotalAudioVolumeWhilePlay()
    {
        return mVideoEditor.getVolumeWhilePlay();
    }

    /**
     * This method sets the volume to default while preview is playing.
     *
     * @return <tt>TRUE</tt> if successful, otherwise <tt>FALSE</tt>.
     * @see #setTotalAudioVolumeWhilePlay(int, int)
     *
     * @since version 1.5.15
     */
    public boolean setTotalAudioVolumeResetWhilePlay()
    {
        int ret = mVideoEditor.setVolumeWhilePlay(100, 100);
        if( ret == 0 )
            return true;
        return false;
    }

    /**
     * This method sets all the volumes in the project. 
     *
     * @param masterVolume The video clip volume.
     * @param slaveVolume The audio clip volume.
     * 
     * @since version 1.5.15
     */
    public void setTotalAudioVolumeProject(int masterVolume, int slaveVolume){

        for( nexClip prime :  mProject.getPrimaryItems() ){
            prime.setClipVolume(masterVolume);
        }

        for( nexAudioItem second :  mProject.getAudioItems() ){
            second.getClip().setClipVolume(slaveVolume);
        }
        mProject.setBGMMasterVolumeScale( (float)slaveVolume/200f );
    }

    /**
     * This method gets all clip's volumes in the project. 
     *
     * @return <tt>all clip's volume</tt> if successful, otherwise <tt>NULL</tt>.
     * 
     * @since version 2.0.13
     */
    public int[] getTotalAudioVolumeProject_LastApplied(){
		int iClipCount = 0, iIndex = 0;
		iClipCount += mProject.getTotalClipCount(true);
		iClipCount += mProject.getTotalClipCount(false);

		int[] sTotalClipVolume = new int[iClipCount];

        for( nexClip prime :  mProject.getPrimaryItems() ){
            sTotalClipVolume[iIndex] = prime.getClipVolume();
			iIndex++;
        }

        for( nexAudioItem second :  mProject.getAudioItems() ){
            sTotalClipVolume[iIndex] = second.getClip().getClipVolume();
		iIndex++;
        }

	return sTotalClipVolume;
    }

    private static int bsearch(int arr[], int x)
    {
        int i;
        int p = 0;
        int q = arr.length -1;
        while(p<=q) {
            //if( p==q ) return p;
            i = (p+q)/2;
            if(arr[i] == x) return i;
            if(arr[i] < x)  p = i+1;
            else q = i-1;
        }
        return (p>0)?p-1:0;
    }

    /**
     * This method allows to export a project without using any encoder when all the clips are referencing a single source file.
     * Exporting with this method is faster because there is no use of encoder. 
     * This method must be called when <tt>nexEngine</tt> is in the <tt>Stop</tt> state. 
     *
     * <p>Example code :</p>
     *      {@code
                if(mEngine.forceMixExport(mFile.getAbsolutePath()) == false)
                {
                    Log.e(TAG, "forceMixExport error");
                }
            }
     * @param strExportFile   The path where the output files are saved. 
     * 
     * @return  Exported output video file; otherwise <tt>FALSE</tt> if the project is not in the suitable condition to be used by this method. 
     * For example, when there is an image clip in the project or when the clips in the project are not referencing a single source file.
     * 
     * @since version 1.3.43
     */
    public boolean forceMixExport( String strExportFile ){
        if( isSetProject(true) ) {
            int speedOption = 0;
            if( mProject.getClip(0,true).getClipType() != nexClip.kCLIP_TYPE_VIDEO ){
                Log.d(TAG,"["+mId+"]forceMixExport: no video clip.");
                return false;
            }

            nexProject mixProject = new nexProject();

            //int seekTab[] = mProject.getClip(0,true).getSeekPointsSync();
            if( mProject.getClip(0,true).hasAudio() ){
                if( !mProject.getClip(0,true).getAudioCodecType().contains("aac") ){
                    Log.d(TAG,"["+mId+"]forceMixExport: audio is not aac");
                    return false;
                }
            //}else{
            //    Log.d(TAG,"["+mId+"]forceMixExport: audio not found!");
            //    return false;
            }

            String path = mProject.getClip(0,true).getRealPath();

            if( mVideoEditor.checkIDRStart(path) != 0 ){
                Log.d(TAG,"["+mId+"]forceMixExport: idr finder start fail!");
                return false;
            }

            for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                if( path.compareTo(mProject.getClip(i, true).getRealPath()) == 0 ) {
                    mixProject.add(nexClip.dup(mProject.getClip(i, true)));
                    int startTrimTime = mProject.getClip(i, true).getVideoClipEdit().getStartTrimTime();
                    int endTrimTime = mProject.getClip(i, true).getVideoClipEdit().getEndTrimTime();
                    //int index = bsearch(seekTab, startTrimTime);
                    //Log.d(TAG, "forceMixExport: startTrimTime=" + startTrimTime + ", endTrimTime=" + endTrimTime + ", new startTime=" + seekTab[index]);
                    //mixProject.getClip(i, true).getVideoClipEdit().setTrim(seekTab[index], endTrimTime);
                    if( mProject.getClip(i, true).getVideoClipEdit().getSpeedControl() != 100 ) {
                        speedOption = 1;
                        Log.d(TAG,"["+mId+"]forceMixExport: set speed clip index="+i);
                    }

                    int idrTime = mVideoEditor.checkIDRTime(startTrimTime);
                    if( idrTime == -1 ){
                        Log.d(TAG,"["+mId+"]forceMixExport: idr finder fail startTrimTime="+startTrimTime);
                        mVideoEditor.checkIDREnd();
                        return false;
                    }

                    Log.d(TAG, "["+mId+"]forceMixExport: startTrimTime=" + startTrimTime + ", endTrimTime=" + endTrimTime + ", new idrTime=" + idrTime);
                    mixProject.getClip(i, true).getVideoClipEdit().setTrim(idrTime, endTrimTime);

                }else{
                    Log.d(TAG,"["+mId+"]forceMixExport: ["+i+"] clip invaild path="+path);
                    mVideoEditor.checkIDREnd();
                    return false;
                }
            }
            mVideoEditor.checkIDREnd();
            nexProject tmp = mProject;
            mProject = mixProject;

            try {
                resolveProject(sLoadListAsync,true);
            }catch (Exception e){
                mProject = tmp;
                return false;
            }

            mForceMixExportMode = true;
            mExportTotalTime = mixProject.getTotalTime();
            mState = kState_export;
            mExportFilePath = strExportFile;
            mEncodeMaxFileSize = Long.MAX_VALUE;
            setEditorListener();
            mVideoEditor.setExportVideoTrackUUID(sExportVideoTrackUUIDMode, null);
            mVideoEditor.directExport(mExportFilePath, mEncodeMaxFileSize, mProject.getTotalTime(), EditorGlobal.getProjectEffect("up"), speedOption);
            mProject = tmp;

            return true;
        }
        return false;
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public boolean checkKineMixExport(String srcFilePath, String dstFilePath){
        return false;
/*        KineMixSDK nex = new KineMixSDK();
        KineMixSDK.NexMediaInfo media1 = new KineMixSDK.NexMediaInfo();
        KineMixSDK.NexMediaInfo media2 = new KineMixSDK.NexMediaInfo();
        nex.getMediaInfo(srcFilePath, media1);
        nex.getMediaInfo(dstFilePath, media2);
        boolean isMergeable = media1.equals(media2);
        return isMergeable;*/
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public int checkKineMixExport( boolean dummy ){
        return retCheckDirectExport_InvalidClipList;
/*        if( isSetProject(true) ) {
            KineMixSDK nex = new KineMixSDK();
            boolean ismergeable = true;
            String None = "none";
            if( mProject.getTotalClipCount(true) == 0 ){
                return retCheckDirectExport_ClipCountZero;
            }
            if(mProject.getOverlayItems().isEmpty() == false) {
                return retCheckDirectExport_HasVideoLayer;
            }

            if( mProject.getBackgroundMusicPath() != null ){
                return retCheckMixExport_HasMusic;
            }

            if( mProject.getTotalClipCount(true) == 1 ){
                if(mProject.getClip(0, true).getClipType()!=nexClip.kCLIP_TYPE_VIDEO) {
                    return retCheckDirectExport_HasImageClip;
                } else if(mProject.getClip(0, true).getVideoClipEdit().getSpeedControl()!=100) {
                    return retCheckMixExport_HasSpeedControl;
                }else if( None.compareTo(mProject.getClip(0, true).getClipEffect().getId()) !=0 ){
                    return retCheckDirectExport_SetClipEffect;
                } else {
                    return retCheckDirectExport_OK;
                }
            }else {
                for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                    if(None.compareTo(mProject.getClip(i, true).getClipEffect().getId())==0 &&
                            (mProject.getClip(i, true).getClipType()==nexClip.kCLIP_TYPE_VIDEO && mProject.getClip(i, true).getVideoClipEdit().getSpeedControl() == 100)) {
                        if (ismergeable == true) {
                            if (i + 1 < mProject.getTotalClipCount(true)) {
                                ismergeable = checkKineMixExport(mProject.getClip(i, true).getPath(), mProject.getClip(i + 1, true).getPath());
                            }
                        }else{
                            break;
                        }
                    }else{
                        ismergeable = false;
                        break;
                    }
                }
            }
            return ismergeable?retCheckDirectExport_OK:retCheckDirectExport_InvalidClipList;
        }
        return retCheckDirectExport_ClipCountZero;*/
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public boolean checkKineMixExport( ){
        return false;
/*        int ret = checkKineMixExport(true);
        if(ret == 0 ){
            Log.d(TAG, "KineMixExport is available");
            return true;
        }else{
            Log.d(TAG, "KineMixExport is not available("+ret+")");
            return false;
        }*/
    }

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    public boolean KineMixExport( String strExportFile ){
        return false;
/*        if( isSetProject(true) ) {
            resolveProject(sLoadListAsync);

            if( ex != null ){
                ex.release();
            }

            ex = new Exporter(strExportFile);
            ex.init(mAppContext, Environment.getExternalStorageDirectory().getUsableSpace(), new ExportListener());
            for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                String filepath = mProject.getClip(i, true).getPath();
                int startTrimTime = mProject.getClip(i, true).getVideoClipEdit().getStartTrimTime();
                int duration = mProject.getClip(i, true).getVideoClipEdit().getEndTrimTime() - startTrimTime;
                ex.mKineMixHandle.addClip(filepath, startTrimTime, duration, KineMixSDK.KINEMIXSDK_CLIP_TYPE_VIDEO, null);
            }
            setEditorListener();
            ex.start();
        }
        return true;*/
    }

    /**
     * @deprecated  For internal use only. Please do not use.
     */
    @Deprecated
    public void cancelKineMixExport(){
/*        if( ex != null ) {
            ex.cancel();
        }*/
    }
    /** 
     * This method gets the duration of a project.
     * To get the exact duration value, this method should be used after calling {@link #play()} then {@link nexEngineListener#onPlayStart()}.
     * 
     * The method {@link nexProject#getTotalTime()} of the class <tt>nexProject</tt> can be used instead of this method.
     *
     * <p>Example code :</p>
     *      {@code
                int duration = mEngine.getDuration();
            }
     * @return The duration of the project in <tt>msec</tt> (milliseconds); 0 if no clip in the project.
     * @throws ProjectNotAttachedException in the case where there is no project created or no clip is created within a project.
     * @since version 1.0.0
     */
    public int getDuration(){
        if( isSetProject(false) ) {
            return mVideoEditor.getDuration()*1000;
        }
        return 0;
    }

    /**
     * This method gets the current play time, on the timeline, of a clip. 
     * 
     * <p>Example code :</p>
     *      {@code
                int currentplaytime = mEngine.getCurrentPlayTimeTime();
            }
     * @return The amount of time played already since the start time of a clip.
     * @see #setEventHandler(nexEngineListener)
     * @since version 1.0.1
     */
    public int getCurrentPlayTimeTime(){
        return mCurrentPlayTime;
    }

    /** 
     * This method updates the project when there is any change made to the project settings.  
     *
     * <p>Example code :</p>
     *      {@code    mEngine = KMSDKApplication.getApplicationInstance().getEngin();
                mEngine.setView(m_editorView);
                mEngine.setProject(project);
                mEngine.updateProject();
            }
     * @since version 1.0.1
     */
    public void updateProject(){
        if( mProject != null ) {
            setOverlays(OverlayCommand.upload);
            int rval = resolveProject(sLoadListAsync,false);

            if( mState == kState_export)
            {
                //loadEffectsInEditor(true);
            }else{
                if (rval == 1) {
                    if (mProject.getTotalClipCount(true) > 0)
                    loadEffectsInEditor(false);
                }
            }
        }
    }

    /**
     * This is the same updateProject() method, but ignoring any exception.
     * @param noException true - ignore any exception thrown in updateProject(), false - the same as void updateProject().
     * @return true - no exception thrown, false - exception thrown.
     * @since 1.7.16
     * @see #updateProject()
     */
    public boolean updateProject(boolean noException){
        if( noException ) {
            try {
                updateProject();
            } catch (Exception e) {
                return false;
            }
        }else{
            updateProject();
        }
        return true;
    }
/*
    public void updateVisualClip(int clipIndex){
        if( mProject != null ) {
            NexVisualClip visualClip = mVideoEditor.getVisualClip(clipIndex+1);
            if( visualClip != null ){
                nexClip clipItem = mProject.getClip(clipIndex,true);
                visualClip.mRotateState = clipItem.getRotateDegree();
                mVideoEditor.updateVisualClip(visualClip);
            }
        }
    }
*/
    private void clearList(){
        Log.i(TAG, "["+mId+"]clearList()");
        if( sLoadListAsync ) {
            mVideoEditor.loadClipsAsync(null, null, 0);
            mVideoEditor.asyncDrawInfoList(null, null);
        }else{
            mVideoEditor.loadClips(null, null, 0);
            mVideoEditor.asyncDrawInfoList(null, null);
        }
    }

    private void encodeEffectOptions( StringBuilder b, Map<String,String> optionMap ) {
        boolean first = true;
        try {
            for( Map.Entry<String, String> entry: optionMap.entrySet() ) {
                if( first ) {
                    first = false;
                } else {
                    b.append('&');
                }
                b.append(URLEncoder.encode(entry.getKey(), "UTF-8"));
                b.append("=");
                b.append(URLEncoder.encode(entry.getValue(),"UTF-8"));
            }
        } catch (UnsupportedEncodingException e) {
            throw new IllegalStateException(e);
        }
    }

    String getEncodedEffectOptions( nexClip clip, String title) {
        StringBuilder b = new StringBuilder();
        b.append(clip.getClipEffect(true).getShowStartTime());
        b.append(',');
        b.append(clip.getClipEffect(true).getShowEndTime());
        b.append('?');
        encodeEffectOptions(b, clip.getTransitionEffect(true).getEffectOptions(title));
        b.append('?');
        encodeEffectOptions(b, clip.getClipEffect(true).getEffectOptions(title));
        //Log.d(TAG, "=============Encoded Effect 2 : " + b.toString());
        return b.toString();
    }

    /**
     * This method clears the project loaded on the <tt>nexEngine</tt>. 
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.clearProject();
            }
     * @see #setProject(nexProject)
     * @since version 1.3.43
     */
    public void clearProject(){
        stopAsyncFaceDetect();
        clearList();
        mCachedNexVisualClips = null;
        mCachedNexAudioClips = null;
        mProject = null;
    }

    /**
     * This method clears display.
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.clearScreen();
            }
     * @see #setProject(nexProject)
     * @since version 1.3.43
     */
    public int clearScreen(){
        if( mVideoEditor != null ) {
            mVideoEditor.clearScreen();
            return 0;
        }
        return 1;
    }

    private List<NexVisualClip> mCachedNexVisualClips;
    private List<NexAudioClip> mCachedNexAudioClips;

    private boolean checkUpdateProject(List<NexVisualClip> vclips , List<NexAudioClip> aclips ){
        boolean update = false;
        if( mCachedNexVisualClips != null  ){
            if( mCachedNexVisualClips.size() == vclips.size() ) {
                int len = vclips.size();
                for(int i = 0; i < len ; i++ ){
                    if( !mCachedNexVisualClips.get(i).equals(vclips.get(i)) ){
                        Log.d(TAG,"checkUpdateProject video not equals");
                        update = true;
                        break;
                    }
                }
            }else{
                update = true;
                Log.d(TAG,"checkUpdateProject video diff size");
            }
        }else{
            if( vclips.size() > 0 ){
                update = true;
            }
        }

        if( !update ){
            if( mCachedNexAudioClips != null ){
                if( mCachedNexAudioClips.size() == aclips.size() ){
                    int len = aclips.size();
                    for(int i = 0; i < len ; i++ ){
                        if( !mCachedNexAudioClips.get(i).equals(aclips.get(i)) ){
                            update = true;
                            Log.d(TAG,"checkUpdateProject audio not equals");
                            break;
                        }
                    }
                }else{
                    update = true;
                    Log.d(TAG, "checkUpdateProject audio diff size");
                }
            }else{
                if( aclips.size() > 0 ){
                    update = true;
                }
            }
        }

        if( update ){
            mCachedNexVisualClips = vclips;
            mCachedNexAudioClips = aclips;
        }

        return update;
    }

    private boolean loadClipToEngine(List<NexVisualClip> vclips , List<NexAudioClip> aclips,boolean async,boolean force, int flag){
        boolean load = false;
        if( force ){
            mCachedNexVisualClips = vclips;
            mCachedNexAudioClips = aclips;
            load = true;
            Log.d(TAG, "loadClipToEngine update force");
        }else {
            if (checkUpdateProject(vclips, aclips)) {
                Log.d(TAG, "loadClipToEngine update loadlist call");
                load = true;
            } else {
                Log.d(TAG, "loadClipToEngine No update");
            }
        }

        if( load ){
            int vsize = mCachedNexVisualClips.size();
            NexVisualClip[] visualClips = new NexVisualClip[vsize];

            int asize = mCachedNexAudioClips.size();
            if (asize == 0) {
                if( async ) {
                    mVideoEditor.loadClipsAsync(mCachedNexVisualClips.toArray(visualClips), null, flag);
                }else{
                    mVideoEditor.loadClips(mCachedNexVisualClips.toArray(visualClips), null, flag);
                }
            } else {
                NexAudioClip[] audioClips = new NexAudioClip[asize];
                if( async ) {
                    mVideoEditor.loadClipsAsync(mCachedNexVisualClips.toArray(visualClips), mCachedNexAudioClips.toArray(audioClips), flag);
                }else{
                    mVideoEditor.loadClips(mCachedNexVisualClips.toArray(visualClips), mCachedNexAudioClips.toArray(audioClips), flag);
                }
            }
        }else{
            //mVideoEditor.seek(0,false,null);
            mVideoEditor.clearProject();
        }
        return load;
    }

    private int mLastProjectFadeIn = -1;
    private int mLastProjectFadeOut = -1;
    private int mLastProjectVolume = -1;
    private int mLastManualVolCtl = -1;

    private int resolveProject(boolean async, boolean force ) {
        nexProject project = mProject;
        int retval = 0;
        if (sTranscodeMode) {
            project = sTranscodeProject;
        }

        if( mForceMixExportMode ){
            mForceMixExportMode = false;
        }

        if (project == null) {
            Log.i(TAG, "["+mId+"]resolveProject() Project is null");
            clearList();
            //yoon
            mCachedNexVisualClips = null;
            mCachedNexAudioClips = null;
            return retval;
        }

        project.updateProject();
        mVideoEditor.setBaseFilterRenderItem(project.getLetterboxEffect());
        List<nexClip> list = project.getPrimaryItems();
//        int nVisualTotal = mProject.getTotalVisualClipCount();
        int ntotal = list.size();
        int id = 1;

        if (ntotal == 0) {
            Log.i(TAG, "["+mId+"]resolveProject() Project["+project.getId()+"] clip is zero");
            clearList();
            mCachedNexVisualClips = null;
            mCachedNexAudioClips = null;
            return retval;
        }
        Log.i(TAG, "["+mId+"]resolveProject() Project["+project.getId()+"]");
        int overlayVideoTotal = getOverlayVideoCount();


        nexClip clipItem;
        int endTime = 0;
        int ThemeMode = 0; //0 - not 1st or last , 1 - 1st , 2 - last
        //int index = 0;

        List <NexAudioClip> audioSyncClips = new ArrayList<>();
        List <NexAudioClip> audioClipList = new ArrayList<>();
        List <NexVisualClip> visualClipList = new ArrayList<>();
        int avSyncId = 30001;

        List<nexDrawInfo> subDrawInfos = new ArrayList<>();

        for (int i = 0; i < ntotal; i++) {
            clipItem = list.get(i);
            String strTitle = null;

            if( i == 0 ){
                ThemeMode = 1;
                strTitle = project.getOpeningTitle();
            }else if(i == (ntotal - 1) ){
                ThemeMode = 2;
                strTitle = project.getEndingTitle();
            }else{
                ThemeMode = 0;
            }
            NexVisualClip visualClip = new NexVisualClip();

            List<nexDrawInfo> subs = clipItem.getDrawInfos();
            for(nexDrawInfo info : subs ) {
                info.setClipID(id);
                subDrawInfos.add(info);
            }

            visualClip.mClipPath = clipItem.getRealPath();
            visualClip.mClipID = id;
            visualClip.mWidth = clipItem.getWidth();
            visualClip.mHeight = clipItem.getHeight();

            visualClip.mAudioOnOff = clipItem.getAudioOnOff() ? 1: 0;
            visualClip.mBGMVolume = clipItem.getBGMVolume();
            visualClip.mClipEffectID = clipItem.getTransitionEffect(true).getId();

            if( ThemeMode == 2 ) {
                visualClip.mClipEffectID = "none";
                visualClip.mEffectDuration = 0;
            }else {
                visualClip.mClipEffectID = clipItem.getTransitionEffect(true).getId();
                if( visualClip.mClipEffectID.compareTo("none") == 0 ){
                    visualClip.mEffectDuration = 0;
                }else {
                    visualClip.mEffectDuration = clipItem.getTransitionEffect(true).getDuration();
                }
            }
            visualClip.mEffectOffset = clipItem.getTransitionEffect(true).getOffset();
            visualClip.mEffectOverlap = clipItem.getTransitionEffect(true).getOverlap();

            visualClip.mTitleEffectID = clipItem.getClipEffect(true).getId();
            if( visualClip.mTitleEffectID.compareTo("none") == 0 ){
                visualClip.mTitleEffectID = null;
            }

            visualClip.mTitleStartTime = clipItem.mTitleEffectStartTime;
            visualClip.mTitleEndTime = clipItem.mTitleEffectEndTime;

            switch (clipItem.getClipType()) {
                case nexClip.kCLIP_TYPE_IMAGE:
                    visualClip.mStartTime = clipItem.mStartTime;
                    visualClip.mEndTime = clipItem.mEndTime;
                    visualClip.mClipType = NexEditorType.NEXCLIP_TYPE_IMAGE;
                    visualClip.mTotalTime = clipItem.getImageClipDuration();
                    visualClip.mSpeedControl = 100;
                    break;
                case nexClip.kCLIP_TYPE_VIDEO:
                    visualClip.mClipType = NexEditorType.NEXCLIP_TYPE_VIDEO;
                    visualClip.mExistAudio = clipItem.hasAudio() ? 1 : 0;
                    visualClip.mExistVideo = clipItem.hasVideo() ? 1 : 0;
                    visualClip.mTotalTime = clipItem.getTotalTime();
                    visualClip.mTotalVideoTime = clipItem.getTotalTime();
                    visualClip.mClipVolume = clipItem.getClipVolume();

                    if (clipItem.getVideoClipEdit().mTrimStartDuration != 0 || clipItem.getVideoClipEdit().mTrimEndDuration != 0) {
                        visualClip.mStartTrimTime = clipItem.getVideoClipEdit().mTrimStartDuration;
                        visualClip.mEndTrimTime = clipItem.getVideoClipEdit().mTrimEndDuration;
                    }
                    visualClip.mFreezeDuration = clipItem.getVideoClipEdit().mFreezeDuration;
                    visualClip.mStartTime = clipItem.mStartTime;
                    visualClip.mEndTime = clipItem.mEndTime;
                    visualClip.mSpeedControl = clipItem.getVideoClipEdit().getSpeedControl();
                    visualClip.mKeepPitch = clipItem.getVideoClipEdit().getKeepPitch();
                    visualClip.mVoiceChanger = clipItem.getAudioEdit().getVoiceChangerFactor();
                    visualClip.mCompressor = clipItem.getAudioEdit().getCompressor();
                    visualClip.mPitchFactor = clipItem.getAudioEdit().getPitch();
                    visualClip.mMusicEffector = clipItem.getAudioEdit().getMusicEffect();
                    visualClip.mPanLeft = clipItem.getAudioEdit().getPanLeft();
                    visualClip.mPanRight = clipItem.getAudioEdit().getPanRight();
                    visualClip.mProcessorStrength = clipItem.getAudioEdit().getProcessorStrength();
                    visualClip.mBassStrength = clipItem.getAudioEdit().getBassStrength();
                    visualClip.mEnhancedAudioFilter = clipItem.getAudioEdit().getEnhancedAudioFilter();
					visualClip.mEqualizer = clipItem.getAudioEdit().getEqualizer();
                    visualClip.mSlowMotion = clipItem.mPropertySlowVideoMode?1:0;
		      visualClip.mIframePlay = clipItem.mIframePlay?1:0;
                    break;
            }
            id++;
            endTime =  visualClip.mEndTime;
            visualClip.mRotateState = clipItem.getRotateDegree();

            visualClip.mTitle = getEncodedEffectOptions(clipItem, strTitle);

            visualClip.mVignette = clipItem.getVignetteEffect()?1:0;


            if( mEnhancedCropMode == ExportCropMode_Enhanced ) {
                Rect rs = new Rect();
                Rect re = new Rect();

                clipItem.getCrop().getStartPositionRaw(rs);
                clipItem.getCrop().getEndPositionRaw(re);

                nexCrop.enhancedCrop(rs, clipItem.getWidth(), clipItem.getHeight(), mEnhancedCropOutputWidth, mEnhancedCropOutputHeight);
                nexCrop.enhancedCrop(re, clipItem.getWidth(), clipItem.getHeight(), mEnhancedCropOutputWidth, mEnhancedCropOutputHeight);

                visualClip.mStartRect = new NexRectangle(rs.left, rs.top, rs.right, rs.bottom);
                visualClip.mEndRect = new NexRectangle(re.left, re.top, re.right, re.bottom);
            }else if( mEnhancedCropMode == ExportCropMode_Fill ){
                visualClip.mStartRect = new NexRectangle(0, 0, nexCrop.ABSTRACT_DIMENSION, nexCrop.ABSTRACT_DIMENSION );
                visualClip.mEndRect = new NexRectangle(0, 0, nexCrop.ABSTRACT_DIMENSION, nexCrop.ABSTRACT_DIMENSION );
            }else {
                if (clipItem.getCrop().m_rotation == 90 || clipItem.getCrop().m_rotation == 270) {
                    visualClip.mStartRect = new NexRectangle(clipItem.getCrop().m_rotatedStartPositionLeft, clipItem.getCrop().m_rotatedStartPositionTop,
                            clipItem.getCrop().m_rotatedStartPositionRight, clipItem.getCrop().m_rotatedStartPositionBottom);
                    visualClip.mEndRect = new NexRectangle(clipItem.getCrop().m_rotatedEndPositionLeft, clipItem.getCrop().m_rotatedEndPositionTop,
                            clipItem.getCrop().m_rotatedEndPositionRight, clipItem.getCrop().m_rotatedEndPositionBottom);
                } else {
                    visualClip.mStartRect = new NexRectangle(clipItem.getCrop().m_startPositionLeft, clipItem.getCrop().m_startPositionTop,
                            clipItem.getCrop().m_startPositionRight, clipItem.getCrop().m_startPositionBottom);
                    visualClip.mEndRect = new NexRectangle(clipItem.getCrop().m_endPositionLeft, clipItem.getCrop().m_endPositionTop,
                            clipItem.getCrop().m_endPositionRight, clipItem.getCrop().m_endPositionBottom);
                }
            }

            visualClip.mBrightness = clipItem.getCombinedBrightness();
            visualClip.mSaturation = clipItem.getCombinedSaturation();
            visualClip.mHue = clipItem.getCombinedHue();
            visualClip.mContrast = clipItem.getCombinedContrast();
            visualClip.mTintcolor = clipItem.getTintColor();
            visualClip.mLUT = clipItem.getLUTId();
            visualClip.mCustomLUT_A = clipItem.getCustomLUTA();
            visualClip.mCustomLUT_B = clipItem.getCustomLUTB();
            visualClip.mCustomLUT_Power = clipItem.getCustomLUTPower();
            if(clipItem.getAudioEnvelop().getVolumeEnvelopeTimeList() != null) {
                visualClip.mVolumeEnvelopeTime = clipItem.getAudioEnvelop().getVolumeEnvelopeTimeList();
            } else {
                visualClip.mVolumeEnvelopeTime = new int[]{0, visualClip.mEndTime - visualClip.mStartTime};
            }
            if(clipItem.getAudioEnvelop().getVolumeEnvelopeLevelList() != null) {
                visualClip.mVolumeEnvelopeLevel = clipItem.getAudioEnvelop().getVolumeEnvelopeLevelList();
            } else {
                visualClip.mVolumeEnvelopeLevel = new int[]{100, 100};
            }

            visualClip.mMotionTracked = clipItem.isMotionTrackedVideo() ? 1 : 0;

            visualClipList.add(visualClip);

            //TODO:
            int syncTime = clipItem.getAVSyncTime();
            if( syncTime != 0 ){

                NexAudioClip item = new NexAudioClip();
                item.mClipID = avSyncId++;
                item.mVisualClipID = visualClip.mClipID;
                item.mTotalTime =visualClip.mTotalTime;
                item.mClipPath = visualClip.mClipPath;
                item.mAudioOnOff = visualClip.mAudioOnOff;
                visualClip.mAudioOnOff = 0;
                item.mClipType = NexEditorType.NEXCLIP_TYPE_AUDIO;
                item.mClipVolume = visualClip.mClipVolume;
                visualClip.mClipVolume = 0;
                item.mAutoEnvelop = 0;
                item.mSpeedControl = visualClip.mSpeedControl;
                int startGap = (int)((float)syncTime * (float)item.mSpeedControl/100f);

                if( syncTime > 0 ) {
                    if( visualClip.mStartTime == 0 ){
                        item.mStartTime = visualClip.mStartTime+startGap;
                        item.mStartTrimTime = visualClip.mStartTrimTime;
                    }else{
                        item.mStartTime = visualClip.mStartTime;

                        if( visualClip.mStartTrimTime == 0 ){
                            item.mStartTime = visualClip.mStartTime+startGap;
                            item.mStartTrimTime = 0;
                        }else {
                            item.mStartTrimTime = visualClip.mStartTrimTime - syncTime;
                            if (item.mStartTrimTime < 0) {
                                item.mStartTrimTime = 0;
                                //ToDo: //start gap process.
                            }
                        }
                    }
                }else{
                    item.mStartTime = visualClip.mStartTime;
                    item.mStartTrimTime = visualClip.mStartTrimTime-syncTime;
                }
                item.mEndTrimTime =  0;
                item.mEndTime = visualClip.mEndTime;
                int realDuration = (int)((item.mEndTime - item.mStartTime) * (float)visualClip.mSpeedControl/100f);
                item.mEndTrimTime = item.mTotalTime - (item.mStartTrimTime + realDuration);
                if( item.mEndTrimTime < 0 ){
                    item.mEndTrimTime = 0;
                }
                audioSyncClips.add(item);
            }
        }

        if (!sTranscodeMode) {
            //overlay video
            if (overlayVideoTotal > 0) {
                //int i = 0;
                if( mProject != null ){
                    for (nexOverlayItem item : mProject.getOverlayItems()) {
                        if (item.isVideo()) {
                            NexVisualClip visualClip = new NexVisualClip();
                            visualClip.mClipID = id;
                            item.mVideoEngineId = id;
                            visualClip.mClipType = NexEditorType.NEXCLIP_TYPE_VIDEO_LAYER;
                            visualClip.mTotalTime = item.getOverlayImage().getVideoClipInfo().getTotalTime();
                            visualClip.mStartTime = item.getStartTime();

                            visualClip.mEndTime = item.getEndTime();

                            if (item.getStartTrimTime() != 0 || item.getEndTrimTime() != 0) {
                                visualClip.mStartTrimTime = item.getStartTrimTime();
                                visualClip.mEndTrimTime = item.getEndTrimTime();
                            } else {
                                visualClip.mStartTrimTime = 0;
                                visualClip.mEndTrimTime = 0;
                            }
                            visualClip.mWidth = item.getOverlayImage().getVideoClipInfo().getWidth();
                            visualClip.mHeight = item.getOverlayImage().getVideoClipInfo().getHeight();
                            visualClip.mExistVideo = item.getOverlayImage().getVideoClipInfo().hasVideo() ? 1 : 0;
                            visualClip.mExistAudio = item.getOverlayImage().getVideoClipInfo().hasAudio() ? 1 : 0;
                            visualClip.mTitleStartTime = item.getStartTime();
                            visualClip.mTitleEndTime = item.getEndTime();

                            visualClip.mBGMVolume = 100;//m_muteMusic?0:m_musicVolume;
                            visualClip.mAudioOnOff = item.getAudioOnOff() ? 1 : 0;
                            visualClip.mClipVolume = item.getVolume();
                            visualClip.mEffectDuration = 0;
                            visualClip.mClipEffectID = "none";
                            visualClip.mTitleEffectID = null;
                            visualClip.mStartRect = new NexRectangle(0, 0, nexCrop.ABSTRACT_DIMENSION, nexCrop.ABSTRACT_DIMENSION);
                            visualClip.mEndRect = new NexRectangle(0, 0, nexCrop.ABSTRACT_DIMENSION, nexCrop.ABSTRACT_DIMENSION);
                            visualClip.mClipPath = item.getOverlayImage().getVideoClipInfo().getPath();
                            visualClip.mThumbnailPath = null;
                            visualClip.mRotateState = 0;
                            visualClip.mEffectOffset = 0;
                            visualClip.mEffectOverlap = 0;
                            visualClip.mSpeedControl = item.getSpeedControl();

                            visualClipList.add(visualClip);
                            //i++;
                            id++;
                        }
                    }
                }
            }

            id = 10001;
            nexClip bgm = project.getBackgroundMusic();
            if (bgm != null) {
                NexAudioClip item = new NexAudioClip();
                item.mClipID = id++;

                //item.mVisualClipID = 1; //todo : loadClipList
                item.mTotalTime = bgm.getTotalTime();
                item.mClipPath = bgm.getRealPath();
                item.mAudioOnOff = bgm.getAudioOnOff() ? 1 : 0;
                item.mClipType = NexEditorType.NEXCLIP_TYPE_AUDIO;
                item.mClipVolume = (int) (project.getBGMMasterVolumeScale() * 200);
                item.mAutoEnvelop = 1;
                item.mStartTime = project.mStartTimeBGM;


                item.mStartTrimTime = project.mBGMTrimStartTime;
                if (project.mBGMTrimEndTime == 0) {
                    item.mEndTrimTime = 0;
                } else {
                    item.mEndTrimTime = item.mTotalTime - project.mBGMTrimEndTime;
                    if (item.mEndTrimTime < 0) {
                        item.mEndTrimTime = 0;
                    }
                }

                int bgmDuration = project.mBGMTrimEndTime - project.mBGMTrimStartTime;
                if (bgmDuration <= 0) {
                    bgmDuration = bgm.getTotalTime();
                }
                item.mEndTime = project.mLoopBGM ? project.getTotalTime() + item.mStartTime : bgmDuration + item.mStartTime;

                item.mVolumeEnvelopeTime = bgm.getAudioEnvelop().getVolumeEnvelopeTimeList();
                item.mVolumeEnvelopeLevel = bgm.getAudioEnvelop().getVolumeEnvelopeLevelList();

                //audioClips[0] = item;
                audioClipList.add(item);
            }

            for( NexAudioClip item :audioSyncClips){
                audioClipList.add(item);
            }

            for (nexAudioItem audioItem: project.getAudioItems()) {
                NexAudioClip item = new NexAudioClip();
                nexClip aClip = audioItem.mClip;

                item.mClipID = id;
                //item.mVisualClipID = 1; //todo : loadClipList
                item.mTotalTime = aClip.getTotalTime();
                item.mClipPath = aClip.getRealPath();
                item.mAudioOnOff = aClip.getAudioOnOff() ? 1 : 0;
                item.mClipType = NexEditorType.NEXCLIP_TYPE_AUDIO;
                item.mClipVolume = aClip.getClipVolume();
                item.mStartTime = aClip.mStartTime;
                item.mEndTime = aClip.mEndTime;
                item.mStartTrimTime = audioItem.mTrimStartDuration;
                item.mEndTrimTime = audioItem.mTrimEndDuration;

                item.mSpeedControl = audioItem.getSpeedControl(); //yoon

                item.mVolumeEnvelopeTime = aClip.getAudioEnvelop().getVolumeEnvelopeTimeList();
                item.mVolumeEnvelopeLevel = aClip.getAudioEnvelop().getVolumeEnvelopeLevelList();
                item.mVoiceChanger = aClip.getAudioEdit().getVoiceChangerFactor();
                item.mCompressor = aClip.getAudioEdit().getCompressor();
                item.mPitchFactor = aClip.getAudioEdit().getPitch();
                item.mMusicEffector = aClip.getAudioEdit().getMusicEffect();
                item.mPanLeft = aClip.getAudioEdit().getPanLeft();
                item.mPanRight = aClip.getAudioEdit().getPanRight();
                item.mProcessorStrength = aClip.getAudioEdit().getProcessorStrength();
                item.mBassStrength = aClip.getAudioEdit().getBassStrength();
                item.mEnhancedAudioFilter = aClip.getAudioEdit().getEnhancedAudioFilter();
				item.mEqualizer = aClip.getAudioEdit().getEqualizer();

                audioClipList.add(item);
                id++;
            }

            if( project.getTemplateApplyMode() == 3 ) {
                if( loadClipToEngine(visualClipList,audioClipList, async, true, 0x00000101) ){
                    retval = 1;
                }
            }
            else {
                if( loadClipToEngine(visualClipList,audioClipList,async,force, 0) ){
                    retval = 1;
                }
            }

            List<nexDrawInfo> topEffect = project.getTopDrawInfo();

            NexDrawInfo[] top = null;
            NexDrawInfo[] sub = null;
            if( topEffect != null && topEffect.size() > 0 ) {
                Log.d(TAG, String.format("resolve Project for top drawInfo(%d) ++++++++++++++++++++", topEffect.size()));

                top = new NexDrawInfo[topEffect.size()];
                int idx = 0;
                for( nexDrawInfo info : topEffect )
                {
                    NexDrawInfo drawInfo = new NexDrawInfo();

                    drawInfo.mID = info.getID();
                    drawInfo.mTrackID = info.getClipID();
                    drawInfo.mSubEffectID = info.getSubEffectID();

                    drawInfo.mEffectID = info.getEffectID();
                    drawInfo.mTitle = info.getTitle();
                    drawInfo.mIsTransition = info.getIsTransition();

                    drawInfo.mStartTime = info.getStartTime();
                    drawInfo.mEndTime = info.getEndTime();

                    // drawInfo.mRotateState = info.getRotateState();
                    // drawInfo.mUserRotateState = info.getUserRotateState();
                    // drawInfo.mTranslateX = info.getTranslateX();
                    // drawInfo.mTranslateY = info.getTranslateY();
                    // drawInfo.mBrightness = info.getBrightness();
                    // drawInfo.mContrast = info.getContrast();
                    // drawInfo.mSaturation = info.getSaturation();
                    // drawInfo.mTintcolor = info.getTintcolor();
                    // drawInfo.mLUT = info.getLUT();


                    // drawInfo.mStartRect.setRect(info.getStartRect().left, info.getStartRect().top, info.getStartRect().right, info.getStartRect().bottom);
                    // drawInfo.mEndRect.setRect(info.getEndRect().left, info.getEndRect().top, info.getEndRect().right, info.getEndRect().bottom);

                    top[idx] = drawInfo;
                    idx++;
                    endTime = info.getEndTime();

                    // info.print();
                }
                Log.d(TAG, String.format("resolve Project for top drawInfo(%d) --------------------", topEffect.size()));
            }

            if( top != null && subDrawInfos != null && subDrawInfos.size() > 0 ) {

                Log.d(TAG, String.format("resolve Project for sub drawInfo(%d) ++++++++++++++++++++", subDrawInfos.size()));
                sub = new NexDrawInfo[subDrawInfos.size()];
                int idx = 0;
                for (nexDrawInfo info : subDrawInfos) {
                    NexDrawInfo drawInfo = new NexDrawInfo();
                    drawInfo.mID = info.getID();
                    drawInfo.mTrackID = info.getClipID();
                    drawInfo.mSubEffectID = info.getSubEffectID();

                    drawInfo.mEffectID = info.getEffectID();
                    drawInfo.mTitle = info.getTitle();

                    drawInfo.mStartTime = info.getStartTime();
                    drawInfo.mEndTime = info.getEndTime();

                    drawInfo.mRotateState = info.getRotateState();
                    drawInfo.mUserRotateState = info.getUserRotateState();
                    drawInfo.mTranslateX = info.getUserTranslateX();
                    drawInfo.mTranslateY = info.getUserTranslateY();
                    drawInfo.mBrightness = info.getBrightness();
                    drawInfo.mContrast = info.getContrast();
                    drawInfo.mSaturation = info.getSaturation();
                    drawInfo.mTintcolor = info.getTintcolor();
                    drawInfo.mLUT = info.getLUT();
                    drawInfo.mCustomLUT_A = info.getCustomLUTA();
                    drawInfo.mCustomLUT_B = info.getCustomLUTB();
                    drawInfo.mCustomLUT_Power = info.getCustomLUTPower();

                    drawInfo.mStartRect.setRect(info.getStartRect().left, info.getStartRect().top, info.getStartRect().right, info.getStartRect().bottom);
                    drawInfo.mEndRect.setRect(info.getEndRect().left, info.getEndRect().top, info.getEndRect().right, info.getEndRect().bottom);
                    if( facedetect_undetected_clip_cropping_mode == 0 )
                        drawInfo.mFaceRect.setRect(info.getEndRect().left, info.getEndRect().top, info.getEndRect().right, info.getEndRect().bottom);
                    else
                        drawInfo.mFaceRect.setRect(info.getFaceRect().left, info.getFaceRect().top, info.getFaceRect().right, info.getFaceRect().bottom);

                    sub[idx] = drawInfo;
                    idx++;

                    // info.print();
                }
                Log.d(TAG, String.format("resolve Project for sub drawInfo(%d) --------------------", subDrawInfos.size()));

            }
            mVideoEditor.asyncDrawInfoList(top, sub);

            long fadeIn = project.getProjectAudioFadeInTime();
            long fadeOut = project.getProjectAudioFadeOutTime();
            int totalTime = endTime;
            if (fadeIn + fadeOut > totalTime) {
                fadeIn = fadeIn * totalTime / (fadeIn + fadeOut);
                fadeOut = totalTime - fadeIn;
            }

            if( mLastProjectFadeIn != fadeIn || mLastProjectFadeOut != fadeOut ){
                mLastProjectFadeIn = (int)fadeIn;
                mLastProjectFadeOut = (int)fadeOut;
                mVideoEditor.setProjectVolumeFade((int)fadeIn, (int)fadeOut);
            }

            // set Project volume
            int projectVolume = project.getProjectVolume();
            if( mLastProjectVolume != projectVolume ) {
                mLastProjectVolume = projectVolume;
                mVideoEditor.setProjectVolume(projectVolume);
            }

            // set ProjectManualVolumeControl
            int iManualVolCtl = project.getManualVolumeControl();
            if( mLastManualVolCtl != iManualVolCtl ) {
                mLastManualVolCtl = iManualVolCtl;
                mVideoEditor.setProjectManualVolumeControl(iManualVolCtl);
            }
        }else{
            if( loadClipToEngine(visualClipList, audioClipList, async, force, 0) ){
                retval = 1;
            }
        }
        mState = kState_load;
        return retval;
    }

    private Object m_layerRenderLock = new Object();

    //private List<nexOverlayItem> mProjectOverlays = null;

    //private List<nexOverlayItem> mProjectOverlays = new ArrayList<nexOverlayItem>();

    private boolean m_layerLock = false;

    /**
     * This method sets whether to lock or unlock an overlay clip.
     *
     * <p>Example code :</p>
     *      {@code
                mEngine.overlayLock(true);
            }
     * @note This method is for the debugging purpose. Please do not use. 
     * @param lock  Set to <tt>TRUE</tt> to lock; otherwise set to <tt>FALSE</tt> to unlock the overlay clip. 
     */
    public void overlayLock(boolean lock ){
        if( lock ){
            setOverlays(OverlayCommand.lock);
        }else{
            setOverlays(OverlayCommand.unlock);
        }
    }

    /**
     * This class builds an overlay item preview.
     *
     * @since version 1.5.19
     * @see #buildOverlayPreview(int)
     */
    public class OverlayPreviewBuilder {
        private nexOverlayItem mItem;

        private OverlayPreviewBuilder (int id){
            //for (nexOverlayItem temp : mProjectOverlays) {
            for (nexOverlayItem temp : mProject.getOverlayItems()) {
                if (temp.getId() == id) {
                    mItem = temp;
                }
            }
        }

        /**
         * This method decides whether to show the outline of an overlay item. 
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(false).display();
                }
         *
         * @param show Set to <tt>true</tt> to show outline. 
         *
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setOutline(boolean show){
            if( mItem != null  ) {
                mItem.showOutline(show);
            }
            return this;
        }
        /**
         * This method sets the X-axis of an overlay item.
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setPositionX((int) (event.getX() * 1280 / v.getWidth()))
                }
         * @param pos The X-axis of the overlay item.
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setPositionX(int pos){
            if( mItem != null  ){
                mItem.setPosition(pos, mItem.getPositionY());
            }
            return this;
        }
        /**
         * This method sets the Y-axis of an overlay item.
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setPositionY((int) (event.getY() * 720 / v.getHeight()))
                }
         * @param pos The Y-axis of the overlay item. 
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setPositionY(int pos){
            if( mItem != null  ){
                mItem.setPosition(mItem.getPositionX(), pos);
            }
            return this;
        }
        

        /**
         * This method sets the degree value to rotate an overlay item around its X-axis.
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateX(angle).display();
                }
         *
         * @param degree The degree value to rotate.
         *
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setRotateX(int degree){
            if( mItem != null  ){
                mItem.setRotate(degree, mItem.getRotateY(), mItem.getRotateZ());
            }
            return this;
        }
        /**
         * This method sets the degree value to rotate an overlay item around its Y-axis.
         *
         * <p>Example code :</p>
         *      {@code
	            mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateY(angle).display();
	        }
         * @param degree The degree value to rotate.
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setRotateY(int degree){
            if( mItem != null  ){
                mItem.setRotate(mItem.getRotateX(), degree,mItem.getRotateZ());
            }
            return this;
        }
        /**
         * This method sets the degree value to rotate an overlay item around its Z-axis.
         * <p>Example code :</p>
         *      {@code
        	    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setRotateZ(angle).display();
	        }
         *
         * @param degree    The degree value to rotate. 
         *
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setRotateZ(int degree){
            if( mItem != null  ){
                mItem.setRotate(mItem.getRotateX(), mItem.getRotateY(), degree);
            }
            return this;
        }
        /**
         * This method sets the scale value to scale an overlay item along the X-axis. 
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleX(scale).display();
                }
         *
         * @param ratio   The degree value to rotate.
         *
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setScaleX(float ratio){
            if( mItem != null  ){
                mItem.setScale(ratio, mItem.getScaledY());
            }
            return this;
        }
        /**
         * This method sets the scale value to scale an overlay item along the Y-axis.
         * <p>Example code :</p>
         *      {@code
	            mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleY(scale).display();
	        }
         * @param ratio The degree value to rotate.
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setScaleY(float ratio){
            if( mItem != null  ){
                mItem.setScale(mItem.getScaledX(),ratio);
            }
            return this;
        }
        /**
         * This method sets the scale value to scale an overlay item along the Y-axis.
         * <p>Example code :</p>
         *      {@code
        mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleAll(scale).display();
        }
         * @param ratio The degree value to rotate.
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setScaleAll(float ratio){
            if( mItem != null  ){
                mItem.setScale(ratio,ratio);
            }
            return this;
        }


        /**
         * This method sets the transparency of an overlay item.
         * <p>Example code :</p>
         *      {@code
                    mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setAlpha(lastProgress).display();
                }
         * @param value The transparency value.
         * @return OverlayPreviewBuilder
         * @since version 1.5.15
         */
        public OverlayPreviewBuilder setAlpha(float value){
            if( mItem != null  ){
                mItem.setAlpha(value);
            }
            return this;
        }

        /**
         * This method displays an overlay item.
         * <p>Example code :</p>
         *  {@code    mEngine.buildOverlayPreview(mOverlayId).setOutline(false);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setScaleY(scale);
                mEngine.buildOverlayPreview(mOverlayId).setOutline(true).setAlpha(lastProgress);
                mEngine.buildOverlayPreview(mOverlayId).display();
                }
         * @since version 1.5.15
         */
        public void display(){
            if( mItem != null  ){
                fastPreview(nexEngine.FastPreviewOption.normal, 0);
            }
        }
        /**
         * This method clears all the overlay items.
         * <p>Example code :</p>
         *  {@code    mEngine.buildOverlayPreview(mOverlayId).clear();
                }
         * @since version 1.5.15
         */
        public void clear(){
            if( mItem != null  ){
                mItem = null;
            }
        }
    }


    /**
     * This method tells whether any overlay item has been selected from the touched location on the current screen. 
     *
     * @param point     Touched location on the screen.
     * @return <tt>true</tt> if any overlay item has been selected.
     *
     * @since version 1.5.15
     * @see {@link com.nexstreaming.nexeditorsdk.nexOverlayItem.HitPoint HitPoint}
     */
    public boolean getOverlayHitPoint(nexOverlayItem.HitPoint point){
        //for( nexOverlayItem temp : mProjectOverlays ){
        for( nexOverlayItem temp : mProject.getOverlayItems() ){
            if (temp.getStartTime() <= point.mTime && temp.getEndTime() > point.mTime) {
                if( temp.isPointInOverlayItem(point) ){
                    return true;
                }
            }
        }
        return false;
    }

/*
    public int getOverlayID(int time, float viewX, float viewY, float viewWidth , float viewHeight){
        for( nexOverlayItem temp : mProjectOverlays ){
            if (temp.getStartTime() <= time && temp.getEndTime() > time) {
                if( temp.isPointInOverlayItem(time,viewX,viewY,viewWidth,viewHeight) ){
                    return temp.getId();
                }
            }
        }
        return -1;
    }
*/

    /**
     * This method gets the <tt>OverlayPreviewBuilder</tt> with the given ID. 
     *
     * @param id The ID to get <tt>OverlayPreviewBuilder</tt>.
     * @return OverlayPreviewBuilder
     * @since version 1.5.15
     */
    public OverlayPreviewBuilder buildOverlayPreview(int id){
        return new OverlayPreviewBuilder(id);
    }

    private enum OverlayCommand {
        clear, upload , lock, unlock
    };

    private void setOverlays( OverlayCommand cmd  ){
        synchronized (m_layerRenderLock) {
            switch (cmd){
                case clear: //clear
//                    for( nexOverlayItem temp : mProjectOverlays ){
//                        Log.d(TAG,"Overlay clear id = "+temp.getId());
//                        temp.clearCache();
//                    }
                    //mProjectOverlays.clear();
                    mActiveRenderLayers.clear();
                    NexCache.getInstance().releaseCache();
                    break;

                case upload: //upload
                    if( mProject == null )
                        return;
// mjkong: Bitmap's resource is managed by LruCache, entryRemoved listener.                     
//                    for( nexOverlayItem temp : mProjectOverlays ){
//                        temp.clearCache();
//                    }
                    //mProjectOverlays.clear();
                    //mProjectOverlays.addAll(mProject.getOverlayItems());
                    mActiveRenderLayers.clear();
                    break;
                case lock: //overlayLock
                    m_layerLock = true;
                    break;

                case unlock: //overlayunLock
                    m_layerLock = false;
                    break;

            }
        }
    }

    private int getOverlayVideoCount(){
        int count = 0;
        //for(  nexOverlayItem item : mProjectOverlays ){
        if( mProject == null)
            return 0;

        for(  nexOverlayItem item : mProject.getOverlayItems() ){
            if( item.isVideo() ){
                count++;
            }
        }
        //Log.d(TAG,"getOverlayVideoCount="+count+", mProjectOverlays="+mProject.getOverlayItems().size());
        return count;
    }

    private static Comparator<nexOverlayItem> layerZOrderComparator = new Comparator<nexOverlayItem>() {
        @Override
        public int compare(nexOverlayItem lhs, nexOverlayItem rhs) {
            int z = lhs.getZOrder() - rhs.getZOrder();
            if( z < 0 ) return -1;
            else if( z > 0 ) return 1;
            else return 0;
        }
    };


    private NexEditor.LayerRenderCallback m_layerRenderCallback = new NexEditor.LayerRenderCallback() {
        @Override
        public void renderLayers(LayerRenderer renderer) {
            synchronized (m_layerRenderLock) {
                nexOverlayItem preWakeItem = null;
                int wokenItems = 0;
                int sleptItems = 0;

                if( m_layerLock ){
                    return;
                }

                mRenderInCurrentPass.clear();

                if( mProject == null )
                    return;

                if( mProject.getOverlayItems() == null ){
                    return;
                }
                /*
                if( mProjectOverlays == null )
                    mProjectOverlays = mProject.getOverlayItems();
                */
                //if( mProjectOverlays.size() > 0 ) {
                if( mProject.getOverlayItems().size() > 0 ) {
                    int cts = renderer.getCurrentTime();
                    for (nexOverlayItem item : mProject.getOverlayItems()) {
                    //for (nexOverlayItem item : mProjectOverlays) {
                        int startTime = item.getStartTime();
                        int endTime = item.getEndTime();

                        boolean isActive = mActiveRenderLayers.contains(item);
                        /*
                        if( isActive ){
                            if( mProject.getOverlayItems() != null ) {
                                boolean isInclude = mProject.getOverlayItems().contains(item);
                                if( isInclude ){
                                    //item.updated(false);
                                }else{
                                    mActiveRenderLayers.remove(item);
                                    renderer.save();
                                    item.onRenderAsleep(renderer);
                                    renderer.restore();
                                    isActive = false;
                                }
                            }
                        }
                        */

//            Log.d(TAG,"["+temp.getId()+"]LayerRenderCallback getStartTime()="+temp.getStartTime()+", time="+time+", getEndTime()="+temp.getEndTime());
                        if (startTime <= cts && endTime > cts) {
                            if (!isActive) {
                                mActiveRenderLayers.add(item);
                                item.onRenderAwake(renderer);
                                wokenItems++;
                            }
                            //item.mShowRangemode = 1;
                            //item.renderOverlay(renderer,mAppContext);
                            //item.onRender(renderer);
                            mRenderInCurrentPass.add(item);
/* Fixed for OverlayText flashing before ending from Xiaomi
                        } else if (cts >= startTime - PREWAKE_INTERVAL && cts <= endTime + PERSIST_INTERVAL) {
                            if (!isActive && cts <= endTime && preWakeItem == null) {
                                preWakeItem = item;
                            }
*/
                        }else{
                            if (isActive) {
                                mActiveRenderLayers.remove(item);
                                item.onRenderAsleep(renderer);
                                sleptItems++;
                                isActive = false;
                            }
                            /*
                            if( item.mShowRangemode == 1 ){
                                item.mShowRangemode = 2;
                                if(cts - item.getEndTime() < 1000) {
                                    item.renderOverlay(renderer, mAppContext);
                                }
                            }else if( item.mShowRangemode == 2 ){
                                item.mShowRangemode = 0;
                            }
                        */
                        }
                    }
                }


                Collections.sort(mRenderInCurrentPass,layerZOrderComparator);

                int numToRender = mRenderInCurrentPass.size();
                for( int i=0; i<numToRender; i++ ) {
                    nexOverlayItem item = mRenderInCurrentPass.get(i);
                    item.onRender(renderer);
                }
            }
        }
    };

    /** 
     * This enumeration defines the fast preview options to be used by the method <tt>fastPreview()</tt>.
     *  
     * The preview options include: 
     * <ul>
     *   <li> <b>normal</b>, 
     *   <li> <b>brightness</b>,
     *   <li> <b>contrast</b>, and
     *   <li> <b>saturation</b>.
     *   
     * @note <tt>Normal</tt> effect is not used in the current version.
     * 
     * @see #fastPreview(FastPreviewOption, int)
     * @since version 1.1.0
     */
    public static enum FastPreviewOption {
        normal, brightness, contrast, saturation, adj_brightness, adj_contrast, adj_saturation, tintColor, cts, adj_vignette, adj_vignetteRange, adj_sharpness, customlut_clip, customlut_power
    }

    /** 
     * This method allows a color effect to be previewed in real time without skipping, when changing its value.
     *
     * @param option  The preview options to set for the clip.  This will be one of: <b>normal</b>, <b>brightness</b>, <b>contrast</b>, and <b>saturation</b>.
     * @param value  The value to set to the parameter <tt>option</tt>, in range from -255 to 255. 
     * <p>Example code :</p>
    {@code mEngine.fastPreview(nexEngine.FastPreviewOption.brightness,value); }
     * @see {@link com.nexstreaming.nexeditorsdk.nexEngine.FastPreviewOption FastPreviewOption}
     * @since version 1.1.0
     */
    public void fastPreview( FastPreviewOption option, int value ) {
        NexEditor.FastPreviewOption opt = NexEditor.FastPreviewOption.normal;

        switch( option ){
            case normal:
                opt = NexEditor.FastPreviewOption.normal;
                break;
            case brightness:
                opt = NexEditor.FastPreviewOption.brightness;
                break;
            case contrast:
                opt = NexEditor.FastPreviewOption.contrast;
                break;
            case saturation:
                opt = NexEditor.FastPreviewOption.saturation;
                break;
            case adj_brightness:
                opt = NexEditor.FastPreviewOption.adj_brightness;
                break;
            case adj_contrast:
                opt = NexEditor.FastPreviewOption.adj_contrast;
                break;
            case adj_saturation:
                opt = NexEditor.FastPreviewOption.adj_saturation;
                break;
            case tintColor:
                opt = NexEditor.FastPreviewOption.tintColor;
                break;

            case cts:
                opt = NexEditor.FastPreviewOption.cts;
                break;
                
            case adj_vignette:
                opt = NexEditor.FastPreviewOption.adj_vignette;
                break;
            case adj_vignetteRange:
                opt = NexEditor.FastPreviewOption.adj_vignetteRange;
                break;
            case adj_sharpness:
                opt = NexEditor.FastPreviewOption.adj_sharpness;
                break;
            case customlut_clip:
                opt = NexEditor.FastPreviewOption.customlut_clip;
                break;
            case customlut_power:
                opt = NexEditor.FastPreviewOption.customlut_power;
                break;
        }
        mVideoEditor.fastPreview(opt,value);
    }

    /**
     *
     * @param power
     * @since 2.0.10
     */
    public void fastPreviewCustomlut(int power){
        if( mProject == null ){
            return;
        }
        int index = mProject.getClipPosition(mCurrentPlayTime);
        fastPreviewCustomlut(index+1, power);
    }

    /**
     *
     * @param clipid
     * @param power
     * @since 2.0.10
     */
    public void fastPreviewCustomlut(int clipid, int power){

        mVideoEditor.buildFastPreview().option(NexEditor.FastPreviewOption.customlut_clip, clipid).option(NexEditor.FastPreviewOption.customlut_power, power).execute();
    }

    /**
     * This method previews the cropped output screen. 
     * 
     * <p>Example code :</p>
     *      {@code    rect.left = (int) (rect.left / mRatio);
                rect.top = (int) (rect.top / mRatio);
                rect.right = (int) (rect.right / mRatio);
                rect.bottom = (int) (rect.bottom / mRatio);
                mEngine.getProject().getClip(0, true).getCrop().setStartPosition(rect);
                Rect raw = new Rect();
                mEngine.getProject().getClip(0, true).getCrop().getStartPositionRaw(raw);
                mEngine.fastPreviewCrop(raw);
            }
     * @param cropRaw The value of parameters <tt>getStartPositionRaw()</tt> or <tt>setEndPosition()</tt> of <tt>nexCrop()</tt>.
     * @see {@link com.nexstreaming.nexeditorsdk.nexCrop#getEndPositionRaw getEndPositionRaw}(android.graphics.Rect)
     * @see {@link com.nexstreaming.nexeditorsdk.nexCrop#getStartPositionRaw getStartPositionRaw}(android.graphics.Rect)
     * @since version 1.3.43
     */
    public void fastPreviewCrop(Rect cropRaw){
        mVideoEditor.buildFastPreview().option(NexEditor.FastPreviewOption.nofx, 1).cropRect(cropRaw).execute();
    }

    /**
     * This method previews effects applied to current texture.
     * @param cts project time
     * @param swap set it to true when image transition is used in Transition Effect.
     * @since 1.7.0
     */
    public void fastPreviewEffect(int cts, boolean swap){
        mVideoEditor.buildFastPreview().cts(cts).swapv(swap).execute();
    }

    private void loadEffectsInEditor(boolean bExport) {
        loadEffectsInEditor_usingAssetPackageManager(bExport);
    }

    private void loadEffectsInEditor_usingAssetPackageManager(boolean bExport) {

        boolean TransitonMode = false;
        Set<String> effectIds = new HashSet<>();

        Log.d(TAG,"TranscoderMode ="+sTranscodeMode);
        if( sTranscodeMode ){
            if(mProject == null)
                TransitonMode = false;
        } else {

            if( mProject.getTemplateApplyMode() == 3 ) {
            // if( mProject.getTemplateApplyMode() == 2 || mProject.getTemplateApplyMode() == 3 ) {

                List<nexDrawInfo> drawinfo = mProject.getTopDrawInfo();
                if( drawinfo != null && drawinfo.size() > 0 ) {
                    Iterator<nexDrawInfo> draws = drawinfo.iterator();
                    while( draws.hasNext() ) {
                        nexDrawInfo info = draws.next();
                        effectIds.add(info.getEffectID());
                    }
                }
            }
            else {
                TransitonMode = mProject.getTemplageOverlappedTransitionMode();
                List<nexClip> list = mProject.getPrimaryItems();
                for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                    String id = list.get(i).getClipEffect(true).getId();

                    if( id != null ) {
                        if (id.compareToIgnoreCase("none") != 0) {
                            if ( effectIds.contains(id) == false)
                                effectIds.add(id);
                        }
                    }

                    id = list.get(i).getTransitionEffect(true).getId();

                    if (id != null) {
                        if (id.compareToIgnoreCase("none") != 0) {
                            if (effectIds.contains(id) == false)
                                effectIds.add(id);
                        }
                    }
                }
            }
        }

        AssetPackageManager.getInstance().loadEffectsInEditor(effectIds, mVideoEditor, bExport, TransitonMode );
        AssetPackageManager.getInstance().loadRenderItemsInEditor(effectIds, mVideoEditor, bExport);
    }

    /**
     * This method clears the hardware codec and other system sources used by the <tt>nexEngine</tt>.
     * 
     * <p>Example code :</p>
     *      {@code    if( mEngin != null) {
                    mEngin.stop();
                    mEngin.clearTrackCache();
                    Log.d(TAG,"clearTrackCache Call!!!");
                }
            }
     * @since version 1.3.4
     */
    public void clearTrackCache() {
        if( mVideoEditor != null ) {
            mVideoEditor.clearTrackCache();
            setOverlays(OverlayCommand.clear);
        }
    }

    /**
     * @since 2.0.9
     */
    public void clearOverlayCache(){
        setOverlays(OverlayCommand.clear);
    }

    /**
     * This method set to property to nexEditor.
     *
     * @param key Property key name.
     * @param value Property value.
     * @since version 1.7.19
     */
    public void setProperty(String key, String value) {
        if( mVideoEditor != null ) {
            mVideoEditor.setProperty(key, value);
        }
    }

    public void updateDrawInfo(nexDrawInfo info) {
        if( mVideoEditor != null ) {

            NexDrawInfo drawInfo = new NexDrawInfo();

            drawInfo.mID = info.getID();
            drawInfo.mTrackID = info.getClipID();
            drawInfo.mSubEffectID = info.getSubEffectID();

            drawInfo.mEffectID = info.getEffectID();
            drawInfo.mTitle = info.getTitle();
            drawInfo.mIsTransition = info.getIsTransition();

            drawInfo.mStartTime = info.getStartTime();
            drawInfo.mEndTime = info.getEndTime();

            drawInfo.mRotateState = info.getRotateState();

            drawInfo.mUserRotateState = info.getUserRotateState();

            drawInfo.mTranslateX = info.getUserTranslateX();
            drawInfo.mTranslateY = info.getUserTranslateY();

            drawInfo.mLUT = info.getLUT();
            drawInfo.mCustomLUT_A = info.getCustomLUTA();
            drawInfo.mCustomLUT_B = info.getCustomLUTB();
            drawInfo.mCustomLUT_Power = info.getCustomLUTPower();

            drawInfo.mBrightness = info.getBrightness();
            drawInfo.mContrast = info.getContrast();
            drawInfo.mSaturation = info.getSaturation();
            drawInfo.mTintcolor = info.getTintcolor();

            drawInfo.mStartRect.setRect(info.getStartRect().left, info.getStartRect().top, info.getStartRect().right, info.getStartRect().bottom);
            drawInfo.mEndRect.setRect(info.getEndRect().left, info.getEndRect().top, info.getEndRect().right, info.getEndRect().bottom);
            drawInfo.mFaceRect.setRect(info.getFaceRect().left, info.getFaceRect().top, info.getFaceRect().right, info.getFaceRect().bottom);

            mVideoEditor.updateDrawInfo(drawInfo);
        }
    }

    protected void removeClip(int clipID) {
        if( mVideoEditor != null ) {
            mVideoEditor.deleteClipID(clipID, null);
        }
    }


    /*class ExportListener implements Exporter.OnExportListener {

        @Override
        public void onPrepare(Exporter exporter) {
            Log.d("TAG", "ExportListener:onPrepare()");
        }

        @Override
        public void onStart(Exporter exporter) {
            Log.d("TAG", "ExportListener:onStart()");
        }

        @Override
        public void onProgress(Exporter exporter, int progress, int currentSegment, int totalSegment) {
            mEventListener.onEncodingProgress(progress);
            Log.d("TAG", "ExportListener:onProgress()");
        }

        @Override
        public void onStop(final Exporter exporter, final String outputName) {
            if( mEventListener != null )
                mEventListener.onEncodingDone(false, 0);
            Log.d("TAG", "ExportListener:onStop() " + outputName);
        }

        @Override
        public void onAbort(Exporter exporter) {
            Log.d("TAG", "ExportListener:onAbort()");
            if( mEventListener != null )
                mEventListener.onEncodingDone(true, 27);
        }

        @Override
        public void onError(Exporter exporter, int result) {
            Log.d("TAG", "ExportListener:onError()");
        }
    }*/

    /**
     * This enumeration defines the possible state values of the <tt>nexEngine</tt>. 
     *
     * @since version 1.3.43
     */
    public enum nexPlayState {

        NONE(0),
        IDLE(1),
        RUN(2),
        RECORD(3);

        private int mValue;

        nexPlayState( int value ) {
            mValue=value;
        }

        public int getValue() {
            return mValue;
        }

        public static nexPlayState fromValue( int value ) {
            for( nexPlayState v : nexPlayState.values() ) {
                if( v.getValue()==value )
                    return v;
            }
            return null;
        }
    }

    /**
     * This enumeration defines the possible error values of the <tt>nexEngine</tt>.  
     * 
     * 
     * @since version 1.3.43
     */
    public enum nexErrorCode{
        NONE(								0),
        GENERAL(							1),
        UNKNOWN(							2),
        NO_ACTION(							3),
        INVALID_INFO(						4),
        INVALID_STATE(						5),
        VERSION_MISMATCH(					6),
        CREATE_FAILED(						7),
        MEMALLOC_FAILED(					8),
        ARGUMENT_FAILED(					9),
        NOT_ENOUGH_NEMORY(					10),
        EVENTHANDLER(						11),
        FILE_IO_FAILED(						12),
        FILE_INVALID_SYNTAX(				13),
        FILEREADER_CREATE_FAIL(				14),
        FILEWRITER_CREATE_FAIL(				15),
        AUDIORESAMPLER_CREATE_FAIL(			16),
        UNSUPPORT_FORMAT(					17),
        FILEREADER_FAILED(					18),
        PLAYSTART_FAILED(					19),
        PLAYSTOP_FAILED(					20),
        PROJECT_NOT_CREATE(					21),
        PROJECT_NOT_OPEN(					22),
        CODEC_INIT(							23),
        RENDERER_INIT(						24),
        THEMESET_CREATE_FAIL(				25),
        ADD_CLIP_FAIL(						26),
        ENCODE_VIDEO_FAIL(					27),
        INPROGRESS_GETCLIPINFO(     		28),
        THUMBNAIL_BUSY(						29),

        UNSUPPORT_MIN_DURATION(				30),
        UNSUPPORT_MAX_RESOLUTION(			31),
        UNSUPPORT_MIN_RESOLUTION(			32),
        UNSUPPORT_VIDEIO_PROFILE(			33),
        UNSUPPORT_VIDEO_LEVEL(				34),
        UNSUPPORT_VIDEO_FPS(				35),

        TRANSCODING_BUSY(					36),
        TRANSCODING_NOT_SUPPORTED_FORMAT(	37),
        TRANSCODING_USER_CANCEL(			38),

        TRANSCODING_NOT_ENOUGHT_DISK_SPACE(39),
        TRANSCODING_CODEC_FAILED(           40),

        EXPORT_WRITER_INVAILED_HANDLE(		41),
        EXPORT_WRITER_INIT_FAIL(            42),
        EXPORT_WRITER_START_FAIL(		    43),
        EXPORT_AUDIO_DEC_INIT_FAIL(			44),
        EXPORT_VIDEO_DEC_INIT_FAIL(         45),
        EXPORT_VIDEO_ENC_FAIL(              46),
        EXPORT_VIDEO_RENDER_INIT_FAIL(		47),
        EXPORT_NOT_ENOUGHT_DISK_SPACE(		48),

        UNSUPPORT_AUDIO_PROFILE(            49),

        THUMBNAIL_INIT_FAIL(                50),

        UNSUPPORT_AUDIO_CODEC(              51),
        UNSUPPORT_VIDEO_CODEC(              52),

        HIGHLIGHT_FILEREADER_INIT_ERROR(	53),
        HIGHLIGHT_TOO_SHORT_CONTENTS(		54),
        HIGHLIGHT_CODEC_INIT_ERROR(			55),
        HIGHLIGHT_CODEC_DECODE_ERROR(		56),
        HIGHLIGHT_RENDER_INIT_ERROR(		57),
        HIGHLIGHT_WRITER_INIT_ERROR(		58),
        HIGHLIGHT_WRITER_WRITE_ERROR(		59),
        HIGHLIGHT_GET_INDEX_ERROR(			60),
        HIGHLIGHT_USER_CANCEL(				61),

        GETCLIPINFO_USER_CANCEL(				62),

        DIRECTEXPORT_CLIPLIST_ERROR	(				63),
        DIRECTEXPORT_CHECK_ERROR(					64),
        DIRECTEXPORT_FILEREADER_INIT_ERROR(		65),
        DIRECTEXPORT_FILEWRITER_INIT_ERROR(		66),
        DIRECTEXPORT_DEC_INIT_ERROR(				67),
        DIRECTEXPORT_DEC_INIT_SURFACE_ERROR(		68),
        DIRECTEXPORT_DEC_DECODE_ERROR(				69),
        DIRECTEXPORT_ENC_INIT_ERROR(				70),
        DIRECTEXPORT_ENC_ENCODE_ERROR(				71),
        DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR(		72),
        DIRECTEXPORT_ENC_FUNCTION_ERROR(			73),
        DIRECTEXPORT_ENC_DSI_DIFF_ERROR(			74),
        DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR(		75),
        DIRECTEXPORT_RENDER_INIT_ERROR(			76),
        DIRECTEXPORT_WRITER_WRITE_ERROR(			77),
        DIRECTEXPORT_WRITER_UNKNOWN_ERROR(			78),

        FASTPREVIEW_USER_CANCEL(					79),
        FASTPREVIEW_CLIPLIST_ERROR(				80),
        FASTPREVIEW_FIND_CLIP_ERROR(			81),
        FASTPREVIEW_FIND_READER_ERROR(			82),
        FASTPREVIEW_VIDEO_RENDERER_ERROR(		83),
        FASTPREVIEW_DEC_INIT_SURFACE_ERROR(	84),

        HW_NOT_ENOUGH_MEMORY(					85),
        EXPORT_USER_CANCEL(						86),

        FASTPREVIEW_DEC_INIT_ERROR(				87),
        FASTPREVIEW_FILEREADER_INIT_ERROR(		88),
        FASTPREVIEW_TIME_ERROR(					89),
        FASTPREVIEW_RENDER_INIT_ERROR(			90),
        FASTPREVIEW_OUTPUTSURFACE_INIT_ERROR(	91),
        FASTPREVIEW_BUSY(						92),

        CODEC_DECODE(							93),        
        RENDERER_AUDIO(						94);

        private final int errno;

        nexErrorCode(int value){
            errno = value;
        }
        public int getValue() {
            return errno;
        }

        public static nexErrorCode fromValue( int value ) {
            for( nexErrorCode v : nexErrorCode.values() ) {
                if( v.getValue()==value )
                    return v;
            }
            return null;
        }
    }

    /**
     * This method gets the IDR-frame locations of a video file in an <tt>array</tt>. 
     * This method operates slower than <tt>getSeekPointsSync()</tt>. 
     * If the length of the content gets too long, ANR may occur. 
     * 
     * <p>Example code :</p>
     *      {@code int seektab[] = mEngine.getIDRSeekTabSync(mProject.getClip(0, true).getPath());
            }
     *
     * @param clip  The path where the video content, to retrieve IDR-frames, is saved.
     * @return  The time location of the IDR-frames in <tt>array</tt>.
     * @see nexClip#getSeekPointsSync()
     * @since version 1.3.43
     */
    public int[] getIDRSeekTabSync(nexClip clip){
        List <Integer> newTab = new ArrayList<Integer>();
        MediaInfo info = clip.getMediaInfo();
        if(info == null){
            Log.d(TAG,"["+mId+"]getIDRSeekTabSync() getinfo fail!");
            return null;
        }

        int seektab[] = info.getSeekPointsSync();
        int lastTime = -1;
        if( mVideoEditor.checkIDRStart(clip.getRealPath()) != 0 ){
            Log.d(TAG,"["+mId+"]getIDRSeekTabSync() checkIDRStart fail!");
            return null;
        }
        
        for( int i = 0 ; i < seektab.length ; i++ ){
            int idrTime = mVideoEditor.checkIDRTime(seektab[i]);
            Log.d(TAG, "["+mId+"]getIDRSeekTabSync() : seektab=" + seektab[i] + ", idrTime=" + idrTime);
            if( idrTime < 0 ){
                Log.d(TAG,"["+mId+"]getIDRSeekTabSync() idrTime fail! seekTime="+seektab[i]);
                mVideoEditor.checkIDREnd();
                return null;
            }

            if( lastTime !=  idrTime ){
                newTab.add(idrTime);
                lastTime = idrTime;
            }
        }
        mVideoEditor.checkIDREnd();

        int ret[] = new int[newTab.size()];
        for(int i = 0 ; i < newTab.size() ; i++ ){
            ret[i] = newTab.get(i).intValue();
            Log.d(TAG, "["+mId+"]getIDRSeekTabSync() : new seektab=" + ret[i]);
        }
        return ret;
    }

    /**
     * This method adds the desired data in the UDAT box of the MP4 file when exporting.
     *
     * @param type The sub-boxes of the asset meta data such as <tt>auth</tt>, <tt>perf</tt>, <tt>gnre</tt>.
     * @param data The user data.
     *
     * @return Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
     *
     * @see #clearUdta()
     * @since version 1.5.16
     */
    public int addUdta(int type, String data){
        return mVideoEditor.addUDTA(type, data);
    }

    /**
     * This method clears all the user UDAT added using the {@link #addUdta(int, String)}.
     *
     * @return Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
     *
     * @see #addUdta(int, String)
     * @since version 1.5.16
     */
    public int clearUdta(){
        return mVideoEditor.clearUDTA();
    }

    /**
     * This method updates the screen mode with the values set to <tt>getScreenMode</tt> and <tt>getAspectRatioMode</tt>.
     *
     * <p>Example code :</p>
     *      {@code    nexApplicationConfig.setScreenMode(nexApplicationConfig.kScreenMode_horizonDual);
                mEngine.updateScreenMode();
            }
     * @see nexApplicationConfig#getScreenMode()
     * @see nexApplicationConfig#getAspectRatioMode()
     * 
     * @since version 1.3.52
     */
    public void updateScreenMode(){
        if( mVideoEditor != null ) {
            //if( mVideoEditor.getAspectMode() != nexApplicationConfig.getAspectRatioMode() ) {
                NexEditor.setLayerScreen(nexApplicationConfig.getAspectProfile().getWidth(),nexApplicationConfig.getAspectProfile().getHeight(),nexApplicationConfig.getOverlayCoordinateMode());
                mVideoEditor.setScreenMode(nexApplicationConfig.getScreenMode());
                NexThemeView.setAspectRatio(nexApplicationConfig.getAspectRatio());
                setMark();
            //}
        }
    }

    /**
     * This abstract class manages the <tt> AutoTrim</tt> results received from the background. 
     *
     * The AutoTrim is a feature supported by the NexEditor&tm; that returns requested number of trimmed sections from one original video content.
     * The returned value is in array of start timestamps which are the start time position of each of the trimmed sections. 
     *
     * @since version 1.5.15
     */
    public static abstract class OnAutoTrimResultListener {
        
        /**
         * This method receives the <tt> AutoTrim</tt> result sent from the background.
         * 
         * @param arrayData An array of requested start timestamps.
         * 
         * @since version 1.5.15 
         */
        public abstract void onAutoTrimResult(int errcode ,int[] arrayData);
    }

    /**
     * This method make automatic trim data using input parameters <tt>durations</tt> and <tt>count</tt>. 
     * @param path Source content path (for video only)
     * @param interval Trim duration, in <tt>msec</tt> (milliseconds)
     * @param count Trim count.
     * @param listener listener for AutoTrim result.
     * @return 0 always. 
     * @since version 1.5.15
     */
    public int autoTrim(String path,boolean IDRunit , int interval, int count, int decodeMode, final OnAutoTrimResultListener listener ){
        if( listener != null ) {
            mVideoEditor.setOnHighLightDoneListener(new NexEditor.OnHighLightDoneListener() {
                @Override
                public int onHighLightDoneListener(int iCount, int[] arrayData) {
                    listener.onAutoTrimResult(0, arrayData);
                    return 0;
                }
                @Override
                public int onHighLightErrorListener(int errcode) {
                    listener.onAutoTrimResult(errcode, null);
                    return 0;
                }
            });
        }
        mVideoEditor.autoTrim(path ,IDRunit?1:0, interval, count, decodeMode);
        return 0;
    }

    /**
     * This method stops the Autotrim function.
     * @return Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
     */
    public int autoTrimStop() {
        return mVideoEditor.autoTrimStop();
    }

    /**
     * This method sets the UUID to the mp4 file of the export video. If the original clip video shows a UUID during exporting, that means it was taken by a 360 camcorder.
     *
     * If there is no UUID, the method will not add anything to the export result file.
     *
     * @param on The availability of the UUID.
     * - <b>TRUE</b> Sets the UUID of the original clip video taken by a 360 camcorder to the export video.
     * - <b>FALSE</b> Do not check the UUID.
     *
     * @since version 1.5.25
     */
    public static void setExportVideoTrackUUID(boolean on){
        Log.i(TAG, "setExportVideoTrackUUID()="+on);
        if( on ){
            sExportVideoTrackUUIDMode = 1;

        }else{
            sExportVideoTrackUUIDMode = 0;
        }
    }

    Context getAppContext(){
        return mAppContext;
    }

    void runTranscodeMode( nexTranscode.Option option ,String path ,nexEngineListener listener) {
        if( sTranscodeMode ){
            listener.onEncodingDone(true,nexEngine.nexErrorCode.TRANSCODING_BUSY.getValue());
            return;
        }

        if(mState == kState_export) {
            listener.onEncodingDone(true,nexEngine.nexErrorCode.TRANSCODING_BUSY.getValue());
            return;
        }

        nexClip clip = nexClip.getSupportedClip(path);

        if( clip == null ){
            listener.onEncodingDone(true, nexErrorCode.TRANSCODING_NOT_SUPPORTED_FORMAT.getValue() );
            return;
        }
        sTranscodeMode = true;
        sTranscodeProject = new nexProject();
        sTranscodeProject.add(clip);
        sTranscodeListener = listener;

        int rotateDegree = 0;
        if( option.outputRotate == nexTranscode.Rotate.CW_0 ){
            rotateDegree = 0;
        }else if( option.outputRotate == nexTranscode.Rotate.CW_90 ){
            rotateDegree = 270;
        }else if( option.outputRotate == nexTranscode.Rotate.CW_180 ){
            rotateDegree = 180;
        }else if( option.outputRotate == nexTranscode.Rotate.CW_270 ){
            rotateDegree = 90;
        }

        if( rotateDegree != 0){
            sTranscodeProject.getClip(0, true).setRotateDegree(rotateDegree);
        }

        if( option.outputFitMode == 0 ) {
            sTranscodeProject.getClip(0, true).getCrop().resetStartEndPosition();
        }else{
            sTranscodeProject.getClip(0, true).getCrop().fitStartEndPosition(option.outputWidth,option.outputHeight);
        }

        resolveProject(true,true);
        int rotateMeta = 0;
        if( option.outputRotateMeta == nexTranscode.Rotate.BYPASS ) {
            rotateMeta = 360 - clip.getRotateInMeta();
            if (rotateMeta == 360) {
                rotateMeta = 0;
            }
        }else if( option.outputRotateMeta == nexTranscode.Rotate.CW_0 ){
            rotateMeta = 0;
        }else if( option.outputRotateMeta == nexTranscode.Rotate.CW_90 ){
            rotateMeta = 90;
        }else if( option.outputRotateMeta == nexTranscode.Rotate.CW_180 ){
            rotateMeta = 180;
        }else if( option.outputRotateMeta == nexTranscode.Rotate.CW_270 ){
            rotateMeta = 270;
        }

        transcode(option.outputFile.getAbsolutePath(),option.outputWidth,option.outputHeight,option.outputBitRate,Long.MAX_VALUE,option.outputSamplingRate,rotateMeta);
    }

    void stopTranscode( ) {
        if( sTranscodeMode ){
            if( mState == kState_export ) {
                stop();
            }else {
                sTranscodeMode = false;
                resolveProject(true,true);
            }
        }
    }

    /**
     * For internal use only. Please do not use.
     * @param on
     * @since 1.5.42
     */
    @Deprecated
    public static void prepareSurfaceSetToNull(boolean on){
        NexEditor.setPrepareSurfaceSetToNull(on);
    }

    /**
     * For internal use only. Please do not use.
     *
     * @return
     * @since 1.5.42
     */
    @Deprecated
    public int getAudioSessionID(){
        return mVideoEditor.getAudioSessionID(true);
    }

    /**
     * This moves the view position when playing 360 video content.
     * @param angleX -360 ~ 360
     * @param angleY -360 ~ 360
     * @return
     *
     * @since 1.5.42
     */
    public boolean set360VideoViewPosition(int angleX , int angleY ){
        return mVideoEditor.set360VideoViewPosition(angleX, angleY);
    }

    /**
     * This moves the view position when 360 video content is paused during playback.
     * @param angleX -360 ~ 360
     * @param angleY -360 ~ 360
     * @since 1.5.42
     */
    public void set360VideoViewStopPosition(int angleX , int angleY ){
        mVideoEditor.buildFastPreview().option(NexEditor.FastPreviewOption.nofx,1).video360(angleX,angleY).execute();
    }

    /**
     * In order to play regular video content from a screen on 360 video mode, the screen needs to swtich to regular view mode.
     * This forces 360 mode to switch to normal view mode. 
     * @since 1.7.0
     */
    public void set360VideoForceNormalView(){
        mVideoEditor.set360VideoForceNormalView();
    }

    /**
     * @brief This method gets the width of an Overlay screen.
     * @return The width of an Overlay screen.
     * @since 1.7.4
     */
    public int getLayerWidth(){
        return nexApplicationConfig.getAspectProfile().getWidth();
    }

    /**
     * @brief This method gets the height of an Overlay screen.
     * @return The height of an Overlay screen.
     * @since 1.7.4
     */
    public int getLayerHeight(){
        return nexApplicationConfig.getAspectProfile().getHeight();
    }

    private int mEnhancedCropMode;
    private int mEnhancedCropOutputWidth;
    private int mEnhancedCropOutputHeight;

    /**
     * This static variable is declared for normal Crop mode
     * @since 1.7.14
     * @see #setExportCrop(int, int, int)
     */
    public static final int ExportCropMode_None = 0;

    /**
     * This static variable is declared for enhanced Crop mode
     * Enhanced Crop mode
     * @since 1.7.14
     * @see #setExportCrop(int, int, int)
     */
    public static final int ExportCropMode_Enhanced = 1;

    /**
     * This static variable is declared for displaying the original video to full screen
     * @since 1.7.14
     * @see #setExportCrop(int, int, int)
     */
    public static final int ExportCropMode_Fill = 2;

    /**
     * This method sets export parameters to generate better output video.
     * @param Mode ExportCropMode_None - normal Crop mode, ExportCropMode_Enhanced - enhanced Crop mode, ExportCropMode_Fill - Fill mode.
     * @param OutputWidth width of an exported video
     * @param OutputHeight height of an exported video
     * @since 1.7.14
     */
    public void setExportCrop(int Mode, int OutputWidth, int OutputHeight){
        Log.d(TAG,"["+mId+"] setExportCrop mode="+Mode+", ("+OutputWidth+"X"+OutputHeight+")");
        mEnhancedCropMode = Mode;
        mEnhancedCropOutputWidth = OutputWidth;
        mEnhancedCropOutputHeight = OutputHeight;
    }

    /**
     * This method sets internal task sleep mode for power consumption.
     * This function must be called with FALSE after use it with TRUE.
     * @param bSleep <tt>TRUE</tt> to sleep internal task, or <tt>FALSE</tt> to wake up internal task.
     * @since 1.7.56
     */
    public void setTaskSleep(boolean bSleep){
        mVideoEditor.setTaskSleep(bSleep ? 1 :0);
    }

    /**
     *
     * @param scaleFactor
     * @return
     * @since 1.7.58
     */
    public boolean setPreviewScaleFactor(float scaleFactor){
        return mVideoEditor.setPreviewScale(scaleFactor);
    }

    /**
     * @since 1.7.58
     */
    public static abstract class OnCaptureListener {
        public abstract void onCapture( Bitmap bm );
        public abstract void onCaptureFail( nexEngine.nexErrorCode error );
    }

    /**
     *
     * @param listener
     * @return
     * @since 1.7.58
     */
    public nexErrorCode captureCurrentFrame( final OnCaptureListener listener){

        if( listener == null ){
            return nexErrorCode.ARGUMENT_FAILED;
        }

        if( mVideoEditor.isSeeking() ){
            NexEditor.ErrorCode errorCode = mVideoEditor.capture(lastSeekTime,new NexEditor.OnCaptureListener() {
                @Override
                public void onCapture(Bitmap bm) {
                    listener.onCapture(bm);
                }

                @Override
                public void onCaptureFail(NexEditor.ErrorCode error) {
                    listener.onCaptureFail(nexErrorCode.fromValue(error.getValue()));
                }
            });

            return nexErrorCode.fromValue(errorCode.getValue());
        }else {
            NexEditor.ErrorCode errorCode = mVideoEditor.captureCurrentFrame(new NexEditor.OnCaptureListener() {
                @Override
                public void onCapture(Bitmap bm) {
                    listener.onCapture(bm);
                }

                @Override
                public void onCaptureFail(NexEditor.ErrorCode error) {
                    listener.onCaptureFail(nexErrorCode.fromValue(error.getValue()));
                }
            });

            return nexErrorCode.fromValue(errorCode.getValue());
        }
    }

    private boolean facedetect_asyncmode = true;
    private int facedetect_syncclip_count = 1;
    private int facedetect_undetected_clip_cropping_mode = 0;

    public enum nexUndetectedFaceCrop {

        NONE(0),
        ZOOM(1);
        //FIT(2);

        private int mValue;

        nexUndetectedFaceCrop( int value ) {
            mValue=value;
        }

        public int getValue() {
            return mValue;
        }

        public static nexUndetectedFaceCrop fromValue( int value ) {
            for( nexUndetectedFaceCrop v : nexUndetectedFaceCrop.values() ) {
                if( v.getValue()==value )
                    return v;
            }
            return null;
        }
    }

    private void defaultFaceDetectSetting(){

        facedetect_asyncmode = true;
        facedetect_syncclip_count = 1;
        facedetect_undetected_clip_cropping_mode = 0;
    }

    ArrayList<AsyncTask<String,Void,FaceInfo>> async_facedetect_worker_list_ = new ArrayList<>();

    public void stopAsyncFaceDetect(){

        for(AsyncTask<String,Void,FaceInfo> m:async_facedetect_worker_list_){

            m.cancel(true);
        }
        async_facedetect_worker_list_.clear();
    }

    /**
     *
     * @param asyncmode
     * @param syncclip_count
     * @param crop
     * @return
     */
    @Deprecated
    public int faceDetect(boolean asyncmode, int syncclip_count, nexUndetectedFaceCrop crop){

        facedetect_asyncmode = true;//asyncmode;
        facedetect_syncclip_count = 0;//syncclip_count;
        facedetect_undetected_clip_cropping_mode = crop.getValue();
        // stopAsyncFaceDetect();
        if(FaceInfo.getFaceModule() != null && facedetect_undetected_clip_cropping_mode !=0){
            return 1;
        }else{
            return 0;
        }
    }

    /**
     *
     * @param uuid
     * @since 2.0.0
     */
    public void setFaceModule(String uuid){
    	stopAsyncFaceDetect();
        if( uuid == null ){
            FaceInfo.setFaceModule(null);
            return;
        }

        nexFaceDetectionProvider module = FaceInfo.getFaceModule();
        if( module != null ) {
            if( module.uuid().compareTo(uuid) == 0 ){
                return;
            }
        }
        if(mProject!=null) {
            for (int i = 0; i < mProject.getTotalClipCount(true); i++) {
                nexClip clip = mProject.getClip(i, true);
                if (clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    clip.resetFaceDetectProcessed();
                }
            }
        }

        Object newModule = nexApplicationConfig.getExternalModuleManager().getModule(uuid);

        if( newModule == null ){
            FaceInfo.setFaceModule(null);
            return;
        }

        if( nexFaceDetectionProvider.class.isInstance(newModule) ) {
            FaceInfo.setFaceModule((nexFaceDetectionProvider)newModule);
        }
    }

    /**
     *
     * @param letterbox
     * @since 1.7.58
     */
    public void setLetterBox(boolean letterbox){
        bLetterBox = letterbox;
    }

    /**
     *
     * @return
     * @since 1.7.58
     */
    public boolean getLetterBox(){
        return bLetterBox;
    }

    private void updateFaceInfo2Clip(nexClip clip, int index, RectF face, int faceMode)
    {
        Rect facebounds = new Rect();
        Rect sbounds = new Rect();
        Rect fullbounds = new Rect();

        boolean face_detected = true;

        int width = clip.getCrop().getWidth();
        int height = clip.getCrop().getHeight();

        int speed = clip.getCrop().getFaceDetectSpeed();
        Log.d(TAG,"Face Detection speed: "+speed);
        fullbounds.set(0, 0, width, height);

        sbounds.set((int)(face.left*width), (int)(face.top*height), (int)(face.right*width), (int)(face.bottom*height));
        facebounds.set((int)(face.left*width), (int)(face.top*height), (int)(face.right*width), (int)(face.bottom*height));

        if(sbounds.isEmpty())
        {
            sbounds.set(0, 0, width, height);
            sbounds.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
            face_detected = false;
            Log.d(TAG,"Face Detection Empty ");
        }
        else
        {
            int addSpace;

            addSpace = width-sbounds.width();
            if(addSpace>=2) {

                sbounds.left -= addSpace/8;
                sbounds.right += addSpace/8;
                Log.d(TAG,"Face Detection width addSpace > 0");
            }

            addSpace = height-sbounds.height();
            if( addSpace>=2 ) {

                sbounds.top -= addSpace/8;
                sbounds.bottom += addSpace/8;
                Log.d(TAG,"Face Detection height addSpace>0");
            }
        }


        if( clip.getDrawInfos() == null || clip.getDrawInfos().size() <= 0 ) {
            Rect tmpFace = new Rect(facebounds);
            Rect tmpStart = new Rect(sbounds);
            Rect tmpFull = new Rect(fullbounds);

            if(face.isEmpty() || clip.getCrop().getEndPosionLock()){
                clip.getCrop().getStartPositionRaw(sbounds);
                clip.getCrop().getEndPositionRaw(fullbounds);

                mVideoEditor.updateRenderInfo(index, clip.isFaceDetected()?1:0, sbounds, fullbounds, fullbounds);
                clip.setFaceDetectProcessed(false, fullbounds);
                return;
            }

            //if( tmpFace.isEmpty() == false ) {
            clip.getCrop().growToAspect(tmpStart, nexApplicationConfig.getAspectRatio());
            clip.getCrop().applyCropSpeed(tmpFull, tmpStart, width, height, speed, clip.getProjectDuration());
/*            if( tmpStart.intersect(0, 0, width, height) == false ) {
                tmpStart.set(0, 0, width*2/3, height*2/3);
                tmpStart.offset((int)(width*Math.random()/3), (int)(height*Math.random()/3));
                Log.d(TAG,"Face Detection intersect not  ");
            }

            int distance = (int)(speed*((float)clip.getProjectDuration()*0.5/1000));
            if(speed < 101) {
                tmpFull.set(tmpStart.left - width*distance/1000, tmpStart.top - height*distance/1000, tmpStart.right + width*distance/1000, tmpStart.bottom + height*distance/1000*3);
                //tmpFull.set(tmpStart.left - distance, tmpStart.top - distance, tmpStart.right + distance, tmpStart.bottom + distance);
            }else {
                tmpFull.set(tmpStart.left - (width/4), tmpStart.top - (height/4), tmpStart.right + (width/4), tmpStart.bottom + (height/4));
            }
            if( tmpFull.intersect(0, 0, width, height) == false) {
                tmpFull.set(0, 0, width*3/4, height*3/4);
                tmpFull.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
            }
*/
            if( face_detected ){
                clip.getCrop().shrinkToAspect(tmpStart, nexApplicationConfig.getAspectRatio());
                clip.getCrop().shrinkToAspect(tmpFull, nexApplicationConfig.getAspectRatio());

                Log.d(TAG,"Face Detection true");
                if(tmpFace.top < tmpFull.top){
                    int dis = tmpFull.top - tmpFace.top;
                    tmpFull.top -= dis;
                    tmpFull.bottom -= dis;
                }
                if(bLetterBox) {
                    int addspace = tmpStart.right - tmpStart.left;
                    tmpStart.left -= addspace / 4;
                    tmpStart.right += addspace / 4;
                    addspace = tmpStart.bottom - tmpStart.top;
                    tmpStart.top -= addspace / 4;
                    tmpStart.bottom += addspace / 4;
                    clip.getCrop().growToAspect(tmpStart, nexApplicationConfig.getAspectRatio());
                }
                clip.getCrop().setStartPosition(tmpFull);
                clip.getCrop().setEndPosition(tmpStart);
                clip.getCrop().setFacePosition(tmpFace);

                clip.getCrop().getStartPositionRaw(tmpFull);
                clip.getCrop().getEndPositionRaw(tmpStart);
                clip.getCrop().getFacePositionRaw(tmpFace);
            }
            else
            {
                if(faceMode == 2) {
                    clip.getCrop().shrinkToAspect(tmpStart, nexApplicationConfig.getAspectRatio());
                    clip.getCrop().growToAspect(tmpFull, nexApplicationConfig.getAspectRatio());

                    clip.getCrop().setStartPosition(tmpFull);
                    clip.getCrop().setEndPosition(tmpStart);
                    clip.getCrop().setFacePosition(tmpFace);
                    clip.getCrop().getStartPositionRaw(tmpFull);
                    clip.getCrop().getEndPositionRaw(tmpStart);
                    clip.getCrop().getFacePositionRaw(tmpFace);
                }else {
                    clip.getCrop().shrinkToAspect(tmpStart, nexApplicationConfig.getAspectRatio());
                    clip.getCrop().shrinkToAspect(tmpFull, nexApplicationConfig.getAspectRatio());
                    clip.getCrop().setStartPosition(tmpStart);
                    clip.getCrop().setEndPosition(tmpFull);
                    clip.getCrop().setFacePosition(tmpFace);
                    clip.getCrop().getStartPositionRaw(tmpStart);
                    clip.getCrop().getEndPositionRaw(tmpFull);
                    clip.getCrop().getFacePositionRaw(tmpFace);
                }
                Log.d(TAG,"Face Detection false  ");
            }
            mVideoEditor.updateRenderInfo(index, face_detected?1:0, tmpFull, tmpStart, tmpFace);
        }
        else
        {
            for( nexDrawInfo info : clip.getDrawInfos() ) {
                if( info.getFaceRect().isEmpty() == false )
                    continue;

                Rect tmpFace = new Rect(facebounds);
                Rect tmpStart = new Rect(sbounds);
                Rect tmpFull = new Rect(fullbounds);
                if(face.isEmpty() || clip.getCrop().getEndPosionLock()){
                    clip.getCrop().getStartPositionRaw(sbounds);
                    clip.getCrop().getEndPositionRaw(fullbounds);

                    info.setStartRect(sbounds);
                    info.setEndRect(fullbounds);
                    info.setFaceRect(fullbounds);
                    updateDrawInfo(info);
                    clip.setFaceDetectProcessed(false, fullbounds);
                    continue;
                }
                clip.getCrop().growToAspect(tmpStart, info.getRatio());
                clip.getCrop().applyCropSpeed(tmpFull, tmpStart, width, height, speed, clip.getProjectDuration());
/*                if( tmpStart.intersect(0, 0, width, height) == false ) {
                    tmpStart.set(0, 0, width*2/3, height*2/3);
                    tmpStart.offset((int)(width*Math.random()/3), (int)(height*Math.random()/3));
                    Log.d(TAG,"Face Detection intersect not  ");
                }

                int distance = (int)(speed*((float)clip.getProjectDuration()*0.5/1000));
                if(speed < 101) {
                    tmpFull.set(tmpStart.left - width*distance/1000, tmpStart.top - height*distance/1000, tmpStart.right + width*distance/1000, tmpStart.bottom + height*distance/1000);
                    //tmpFull.set(tmpStart.left - distance, tmpStart.top - distance, tmpStart.right + distance, tmpStart.bottom + distance);
                }else {
                    tmpFull.set(tmpStart.left - (width/4), tmpStart.top - (height/4), tmpStart.right + (width/4), tmpStart.bottom + (height/4));
                }

                if( tmpFull.intersect(0, 0, width, height) == false) {
                     tmpFull.set(0, 0, width*3/4, height*3/4);
                     tmpFull.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
                }
*/
                if( face_detected )
                {
                    clip.getCrop().shrinkToAspect(tmpStart, info.getRatio());
                    clip.getCrop().shrinkToAspect(tmpFull, info.getRatio());
                    Log.d(TAG,"Face Detection true");
                    if(tmpFace.top < tmpFull.top){
                        int dis = tmpFull.top - tmpFace.top;
                        tmpFull.top -= dis;
                        tmpFull.bottom -= dis;
                    }
                    if(bLetterBox) {
                        int addspace = tmpStart.right - tmpStart.left;
                        tmpStart.left -= addspace / 4;
                        tmpStart.right += addspace / 4;
                        addspace = tmpStart.bottom - tmpStart.top;
                        tmpStart.top -= addspace / 4;
                        tmpStart.bottom += addspace / 4;
                        clip.getCrop().growToAspect(tmpStart, info.getRatio());
                    }

                    clip.getCrop().setStartPosition(tmpFull);
                    clip.getCrop().setEndPosition(tmpStart);
                    clip.getCrop().setFacePosition(tmpFace);

                    clip.getCrop().getStartPositionRaw(tmpFull);
                    clip.getCrop().getEndPositionRaw(tmpStart);
                    clip.getCrop().getFacePositionRaw(tmpFace);
                }
                else
                {
                    if(faceMode == 2) {
                        clip.getCrop().shrinkToAspect(tmpStart, info.getRatio());
                        clip.getCrop().growToAspect(tmpFull, info.getRatio());

                        clip.getCrop().setStartPosition(tmpFull);
                        clip.getCrop().setEndPosition(tmpStart);
                        clip.getCrop().setFacePosition(tmpFace);
                        clip.getCrop().getStartPositionRaw(tmpFull);
                        clip.getCrop().getEndPositionRaw(tmpStart);
                        clip.getCrop().getFacePositionRaw(tmpFace);
                    }else {
                        clip.getCrop().shrinkToAspect(tmpStart, info.getRatio());
                        clip.getCrop().shrinkToAspect(tmpFull, info.getRatio());
                        clip.getCrop().setStartPosition(tmpStart);
                        clip.getCrop().setEndPosition(tmpFull);
                        clip.getCrop().setFacePosition(tmpFace);
                        clip.getCrop().getStartPositionRaw(tmpStart);
                        clip.getCrop().getEndPositionRaw(tmpFull);
                        clip.getCrop().getFacePositionRaw(tmpFace);
                    }
                    tmpFace.set(0, 0, 1, 1);
                    Log.d(TAG,"Face Detection false  ");
                }
                info.setStartRect(tmpStart);
                info.setEndRect(tmpFull);
                info.setFaceRect(tmpFace);
                updateDrawInfo(info);
            }

            clip.getCrop().setFacePosition(facebounds);
            clip.getCrop().getFacePositionRaw(facebounds);
        }

        clip.setFaceDetectProcessed(face_detected, facebounds);
    }

    public int faceDetect_for_seek(int time, final int facemode)
    {
        int iImageClip = 0;
        int continueClipID = 0;

        Log.d(TAG,String.format("Face Detection Mode = %d",facemode));
        if(facemode == 0)
            return -1;

        int iTotalClipCount = mProject.getTotalClipCount(true);

        for(int i = 0; i < iTotalClipCount; i++){

            nexClip clip = mProject.getClip(i, true);
          
            if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE){

                if(clip.mStartTime <= time && time <= clip.mEndTime && clip.isAssetResource() == false){

                    FaceInfo face = FaceInfo.getFaceInfo(clip.getPath());
                    if(null == face){

                        face = new FaceInfo(new File(clip.getPath()), true, getAppContext());
                        FaceInfo.putFaceInfo(clip.getPath(), face);
                    }

                    RectF _sbounds = new RectF();
                    face.getBounds(_sbounds);
                    updateFaceInfo2Clip(clip, i+1, _sbounds, facemode);
                }                
            }
        }
        return 0;
    }
 	    
    public int faceDetect_internal(boolean asyncmode, int num, final int facemode)
    {
        // stopAsyncFaceDetect();

        int iImageClip = 0;
        int continueClipID = 0;
        int syncModeNum = 0;
        Log.d(TAG,String.format("Face Detection Mode = %d",facemode));
        if(facemode == 0)
            return -1;

        int iTotalClipCount = mProject.getTotalClipCount(true);

        for(int i = 0; i < iTotalClipCount; i++)
        {
            nexClip clip = mProject.getClip(i, true);

            if(mProject.getTemplateApplyMode() == 3 || clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE )
            {
                iImageClip++;
                continue;
            }
        }

        if(true == asyncmode) 
            syncModeNum = num;
        else 
            syncModeNum = iImageClip;

        for(int i = 0; i < iTotalClipCount; i++)
        {

            Log.d(TAG,String.format("Face Detection ResetInfo clip ID = %d",i+1));

            nexClip clip = mProject.getClip(i, true);
          
            if(clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE){
                mVideoEditor.resetFaceDetectInfoJ(i+1);
            }
        }

        for(int i =0; i < iTotalClipCount; i++)
        {
            RectF _sbounds = new RectF();
            Log.d(TAG,String.format("Face Detection sync clip ID = %d",i+1));

            nexClip clip = mProject.getClip(i, true);
          
            if(clip.getClipType() != nexClip.kCLIP_TYPE_IMAGE || clip.isFaceDetectProcessed() || clip.isAssetResource() ) {

                if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                    Rect sbounds = new Rect();
                    Rect facebounds = new Rect();
                    Rect fullbounds = new Rect();

                    Log.d(TAG,String.format("Face Detection skip clip ID = %d",i+1));
                    if( clip.getDrawInfos() != null && clip.getDrawInfos().size() > 0) {
                        for( nexDrawInfo info : clip.getDrawInfos() ) {
                            if( info.getFaceRect().isEmpty() ) {
                                clip.getCrop().getStartPositionRaw(sbounds);
                                clip.getCrop().getEndPositionRaw(fullbounds);
                                clip.getCrop().getFacePositionRaw(facebounds);

                                info.setStartRect(sbounds);
                                info.setEndRect(fullbounds);
                                info.setFaceRect(facebounds);
                                updateDrawInfo(info);
                                Log.d(TAG,String.format("Face Detection info update for draw info(clip ID = %d)",i+1));
                            }
                        }
                    }
                    else
                    {

                        clip.getCrop().getStartPositionRaw(sbounds);
                        clip.getCrop().getEndPositionRaw(fullbounds);
                        clip.getCrop().getFacePositionRaw(facebounds);

                        mVideoEditor.updateRenderInfo(i+1, clip.isFaceDetected()?1:0, sbounds, fullbounds, facebounds);
                    }
                    continueClipID++;
                }
                continue;
            }

            FaceInfo face = FaceInfo.getFaceInfo(clip.getPath());
            Log.d(TAG,"Face Detection sync total num ="+syncModeNum+" continueClip num= "+continueClipID);
            if(null == face){

                if(i > syncModeNum-1){

                    // Log.d(TAG,"Face Detection sync total num ="+syncModeNum+" continueClip num= "+continueClipID);
                    break;
                }
                face = new FaceInfo(new File(clip.getPath()), true, getAppContext());

                FaceInfo.putFaceInfo(clip.getPath(), face);
            }

            face.getBounds(_sbounds);

            updateFaceInfo2Clip(clip, i+1, _sbounds, facemode);

            continueClipID++;
        }

        for(int i = continueClipID; i<iTotalClipCount; i++){

            final nexClip clip = mProject.getClip(i, true);
            final int inner_i = i;

            if(clip.getClipType() != nexClip.kCLIP_TYPE_IMAGE || clip.isFaceDetectProcessed() == true || clip.isAssetResource() == true){

                if( clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                    Rect sbounds = new Rect();
                    Rect fullbounds = new Rect();
                    Rect facebounds = new Rect();

                    Log.d(TAG,String.format("Face Detection skip clip ID = %d",i+1));
                    if( clip.getDrawInfos() != null && clip.getDrawInfos().size() > 0) {
                        for( nexDrawInfo info : clip.getDrawInfos() ) {
                            if( info.getFaceRect().isEmpty() ) {
                                clip.getCrop().getStartPositionRaw(sbounds);
                                clip.getCrop().getEndPositionRaw(fullbounds);
                                clip.getCrop().getFacePositionRaw(facebounds);

                                info.setStartRect(sbounds);
                                info.setEndRect(fullbounds);
                                info.setFaceRect(facebounds);
                                updateDrawInfo(info);
                            }
                        }
                    }
                    else
                    {
                        clip.getCrop().getStartPositionRaw(sbounds);
                        clip.getCrop().getEndPositionRaw(fullbounds);
                        clip.getCrop().getFacePositionRaw(facebounds);

                        mVideoEditor.updateRenderInfo(i+1, clip.isFaceDetected()?1:0, sbounds, fullbounds, facebounds);
                    }
                    continueClipID++;
                }
                continue;
            }

            Log.d(TAG,String.format("Face Detection async clip ID = %d",i+1));

            AsyncTask<String,Void,FaceInfo> worker = new AsyncTask<String,Void,FaceInfo>(){
            
                Task.TaskError taskError = null;

                @Override
                protected void onCancelled(){

                    Log.d(TAG, "Face Detection was cancelled");
                }

                @Override
                protected FaceInfo doInBackground(String... params) {

                    Log.d(TAG,"Face Detection async thread start ="+params[0]);

                    if(inner_i == 0){

                        mVideoEditor.waitFaceThread();
                    }

                    FaceInfo face = FaceInfo.getFaceInfo(params[0]);
                    if(null == face){

                        face = new FaceInfo(new File(params[0]), 1, getAppContext());
                        FaceInfo.putFaceInfo(params[0], face);
                    }
                    return face;
                }

                @Override
                protected void onPostExecute(FaceInfo faceInfo) {

                    async_facedetect_worker_list_.remove(this);

                    Log.d(TAG, "Face Detection worker list size:" + async_facedetect_worker_list_.size());

                    RectF _sbounds = new RectF();

                    faceInfo.getBounds(_sbounds);

                    updateFaceInfo2Clip(clip, inner_i+1, _sbounds, facemode);
                    Log.d(TAG,"Face Detection aync thread end ="+clip.getPath());
                }
            };
            async_facedetect_worker_list_.add(worker);
            worker.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR,clip.getPath());
        }
        return 0;
    }
    private void applyCropSpeed(){

    }

    private void undoForceMixProject(){

        if( mForceMixExportMode ){
            if( isSetProject(false) ){
                Log.d(TAG,"["+mId+"]undoForceMixProject");
                resolveProject(true,true);
            }
            mForceMixExportMode = false;
        }
        //mVideoEditor.re
    }


    /**
     *
     * @since 1.7.58
     */
    public static class ProfileAndLevel implements Cloneable{
        private int profile;
        private int level;

        private ProfileAndLevel(int profile, int level ) {
            this.profile = profile;
            this.level = level;
        }

        public int getProfile(){
            return profile;
        }

        public int getLevel(){
            return level;
        }

        @Override
        protected Object clone() throws CloneNotSupportedException {
            return super.clone();
        }

        @Override
        public String toString() {
            return "ProfileAndLevel{" +
                    "level=" + level +
                    ", profile=" + profile +
                    '}';
        }
    }

    /**
     *
     * @since 1.7.58
     */
    public static class ExportProfile{
        private int mimeType;
        private ProfileAndLevel[] proFileAndLevel;

        public int getMimeType(){
            return mimeType;
        }

        public ProfileAndLevel[] getProFileAndLevel(){
            return proFileAndLevel.clone();
        }

        public boolean isSupported(int profile, int level){
            for( ProfileAndLevel pl : proFileAndLevel ){
                if( pl.getProfile() == profile && pl.getLevel() <= level ){
                    return true;
                }
            }
            return false;
        }

        @Override
        public String toString() {
            return "ExportProfile{" +
                    "mimeType=" + mimeType +
                    ", proFileAndLevel=" + Arrays.toString(proFileAndLevel) +
                    '}';
        }
    }

    private static ExportProfile[] s_exportProfiles;

    /**
     *
     * @return
     * @since 1.7.58
     */
    public static ExportProfile[] getExportProfile(){
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP )
            return new ExportProfile[0];

        if( s_exportProfiles == null ){
            MediaCodecList mcl = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
            ArrayList<ExportProfile> eplist = new ArrayList<>();
            for(  MediaCodecInfo codecInfo : mcl.getCodecInfos()) {

                if (!codecInfo.isEncoder()) continue;
                //if (codecInfo.getName().contains("google")) continue; => Remove this code by Robin. On G Flex 2 device, there is no OMX.qcom.video.encoder.avc, but when sdk try to use OMX.google.h264.encoder, OMX change to OMX.qcom.video.encoder.avc.
                String[] types = codecInfo.getSupportedTypes();

                for (int i = 0; i < types.length; i++) {
                    if (types[i].equalsIgnoreCase("video/avc") || types[i].equalsIgnoreCase("video/hevc") || types[i].equalsIgnoreCase("video/mp4v-es")) {
                        ExportProfile exportProfile = new ExportProfile();
                        if( types[i].equalsIgnoreCase("video/avc") ) {
                            exportProfile.mimeType = ExportCodec_AVC;
                        }else if ( types[i].equalsIgnoreCase("video/hevc") ) {
                            exportProfile.mimeType = ExportCodec_HEVC;
                        } else if ( types[i].equalsIgnoreCase("video/mp4v-es") ) {
                            exportProfile.mimeType = ExportCodec_MPEG4V;
                        }
                        Log.d(TAG,"codec name="+codecInfo.getName());
                        try {
                            MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType(types[i]);
                            SparseIntArray profileArray = new SparseIntArray();
                            for (MediaCodecInfo.CodecProfileLevel pl : capabilities.profileLevels) {
                                if( exportProfile.mimeType == ExportCodec_AVC ){
                                    int profile = 0;
                                    switch (pl.profile){
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileBaseline:
                                            profile = ExportProfile_AVCBaseline;
                                            break;
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileMain:
                                            profile = ExportProfile_AVCMain;
                                            break;
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileHigh:
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileHigh10:
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileHigh422:
                                        case MediaCodecInfo.CodecProfileLevel.AVCProfileHigh444:
                                            profile = ExportProfile_AVCHigh;
                                            break;

                                    }

                                    if( profile == 0  )
                                        continue;
                                    Log.d(TAG,"codec profile="+profile+", level="+pl.level);
                                    if( profileArray.get(profile) < pl.level ) {
                                        profileArray.put(profile, pl.level);
                                    }
                                }else if (exportProfile.mimeType == ExportCodec_HEVC) {
                                    int profile = pl.profile;

                                    if( profile == 0  )
                                        continue;

				                    Log.d(TAG,"codec profile="+profile+", level="+pl.level);

                                    if( profileArray.get(profile) < pl.level ) {
                                        profileArray.put(profile, pl.level);
                                    }
                                } else if (exportProfile.mimeType == ExportCodec_MPEG4V) {
                                    int profile=pl.profile;

                                    if( profile == 0  )
                                        continue;

                                    Log.d(TAG,"codec profile="+profile+", level="+pl.level);

                                    if( profileArray.get(profile) < pl.level ) {
                                        profileArray.put(profile, pl.level);
                                    }
                                }
                            }
                            //Log.d(TAG,"codec profileArray.size="+profileArray.size());
                            exportProfile.proFileAndLevel = new ProfileAndLevel[profileArray.size()];
                            for( int j = 0 ; j < profileArray.size() ; j++ ){
                                exportProfile.proFileAndLevel[j] = new ProfileAndLevel(profileArray.keyAt(j),profileArray.valueAt(j));
                            }
                            eplist.add(exportProfile);
                        } catch (Exception e) {
                            Log.wtf(TAG, e);
                        }
                    }
                }
                //eplist.add(exportProfile);
            }
            //Log.d(TAG,"codec eplist.size="+eplist.size());
            s_exportProfiles = new ExportProfile[eplist.size()];
            for( int k = 0 ; k < eplist.size() ;  k++ ){
                s_exportProfiles[k] = eplist.get(k);
            }
        }
        return s_exportProfiles;
    }

    /**
     *
     * @param on
     * @since 2.0.5
     */
    public void setHDRtoSDR(boolean on){
        mVideoEditor.setProperty("HDR2SDR",on ?"1":"0");
    }
    // setThumbnailRoutine value : 0 is thumbnail to original, 1 is thumbnail, 2 is original image
    public void setThumbnailRoutine(int value){
        mVideoEditor.setThumbnailRoutineFirst(value);
    }
}
