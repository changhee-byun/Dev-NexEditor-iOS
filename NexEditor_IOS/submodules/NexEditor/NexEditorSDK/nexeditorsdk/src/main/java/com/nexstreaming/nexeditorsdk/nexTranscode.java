/******************************************************************************
 * File Name        : nexTranscode.java
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

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.NexExportProfile;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.kminternal.kinemaster.transcoder.Transcoder;
import com.nexstreaming.kminternal.kinemaster.transcoder.TranscodingTask;
import com.nexstreaming.nexeditorsdk.exception.InvalidRangeException;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;

import java.io.File;

/**
 * This method is used when changing the bitrate and resolution of one file.
 * Any work related to transcoding uses the device's hardware resources. Therefore, editing, previewing, or exporting will interrupt transcoding.
 * Also, the same rule applies to making nexClips and extracting thumbnails from existing nexClips.
 *
 * <p>Example code :</p>
 * 
 * {@code mEngine.stop();
 * nexTranscode.init(mEngine);
 *  mTransCoder = new nexTranscode("/sdard/input.mp4",false);
    mTransCoder.setTransCoderListener(new nexTranscode.OnTransCoderListener() {

        public void onProgress(int progress, int maxProgress) {
            //TODO: UI progress
        }

        public void onTransCodeDone(nexTranscode.Error error, int suberror) {
            //TODO: Done
        }
    });
    nexTranscode.Option opt = new nexTranscode.Option(new File("/sdard/output.mp4"), outputWidth, outputHeight, outputBitrate, true);
    mTransCoder.run(opt);
 * }
 *
 * @since version 1.5.29
 */
public class nexTranscode {
    final private static String TAG = "nexTranscode";
    private File source;
    private NexEditor mVideoEditor;
    private Error lastError = Error.NONE;
    private State state = State.NONE;
    private static nexEngine sEngine;
    final private static int sRunMode = 0;

    private int progressPercent;

    private OnTransCoderListener listener;
    private static nexEngineListener sEngineListener;

    /**
     * The state of nexTranscode.
     * @see #getCurrentState()
     * @since version 1.5.29
     */
    public enum State{
        NONE,
        IDLE,
        WAIT,
        RUNNING,
        COMPLETE
    }

    /**
     * The error value of nexTranscode.
     * @see #getLastError()
     * @since version 1.5.29
     */
    public enum Error{
        NONE,
        NOTSUPPORTEDFILE,
        ENGINEFAIL,
        SOURCEFAIL,
        BUSY,
        RUNFAIL,
        CANCEL,

    }

    /**
     * The listener of nexTranscode.
     * @see #setTransCoderListener(OnTransCoderListener)
     * @since version 1.5.29
     */
    public static abstract class OnTransCoderListener {
        public abstract void onProgress( int progress, int maxProgress );
        public abstract void onTransCodeDone( Error error , int suberror);
    }

    /**
     *  The value of Rotation.
     *  @see #Option(File outputFile, int outputWidth, int outputHeight, int outputBitRate, boolean outputFit, nexTranscode.Rotate outputRotateMeta, nexTranscode.Rotate outputRotate)
     *  @since vesion 1.5.35
     */
    public enum Rotate{
        BYPASS,
        CW_0,
        CW_90,
        CW_180,
        CW_270
    }

    /**
     * The output option of nexTranscode.
     * @see #run(Option)
     * @since version 1.5.29
     */
    public static class Option{
        File outputFile;
        int outputWidth;
        int outputHeight;
        int outputBitRate;
        int outputFitMode;
        Rotate outputRotateMeta;
        Rotate outputRotate;
        int outputSamplingRate;
        private int labelResource;

        /**
         * The constructor of the output option in nexTranscode.
         * @param outputFile The output file. The user must create directories in advance.
         * @param outputWidth The width of the output.
         * @param outputHeight The height of the output.
         * @param outputBitRate The bitrate of the output.
         * @param outputFit Set to <tt>TRUE</tt> to create black bars(letterbox or pillarbox)
         * to prevent any distortions in the original. Set to <tt>FALSE</tt> to
         * distort the original to fit to the output resolution.
         * @since version 1.5.29
         */
        public Option(File outputFile , int outputWidth, int outputHeight, int outputBitRate, int outputSamplingRate, boolean outputFit){
            this.outputFile = outputFile;
            this.outputWidth = outputWidth;
            this.outputHeight = outputHeight;
            this.outputBitRate = outputBitRate;
            this.outputSamplingRate = outputSamplingRate;
            labelResource = NexExportProfile.getLabelResource(outputWidth, outputHeight);
            if( outputFit ) {
                outputFitMode = 1;
            }else{
                outputFitMode = 0;
            }
            outputRotateMeta = nexTranscode.Rotate.BYPASS;
        }

        /**
         * The constructor of the output option in nexTranscode.
         * @param outputFile The output file. The user must create directories in advance.
         * @param outputWidth The width of the output.
         * @param outputHeight The height of the output.
         * @param outputBitRate The bitrate of the output.
         * @param outputFit Set to <tt>TRUE</tt> to create black bars(letterbox or pillarbox)
         * to prevent any distortions in the original. Set to <tt>FALSE</tt> to
         * distort the original to fit to the output resolution.
         * @param outputRotateMeta The metaData rotation value(BYPASS,CW_0,CW_90,CW_180,CW_270)
         * @param outputRotate The image rotation value(clockwise standard:CW_0,CW_90,CW_180,CW_270)
         * @since version 1.5.35
         */


        public Option(File outputFile , int outputWidth, int outputHeight, int outputBitRate,int outputSamplingRate, boolean outputFit, nexTranscode.Rotate outputRotateMeta, nexTranscode.Rotate outputRotate){
            this.outputFile = outputFile;
            this.outputWidth = outputWidth;
            this.outputHeight = outputHeight;
            this.outputBitRate = outputBitRate;
            this.outputSamplingRate = outputSamplingRate;
            labelResource = NexExportProfile.getLabelResource(outputWidth, outputHeight);
            if( outputFit ) {
                outputFitMode = 1;
            }else{
                outputFitMode = 0;
            }
            this.outputRotateMeta = outputRotateMeta;
            this.outputRotate = outputRotate;
        }


        public void setOutputRotateMeta(nexTranscode.Rotate value){
            outputRotateMeta = value;
        }

        public void setOutputRotate(nexTranscode.Rotate value){
            if( value == Rotate.BYPASS ) {
                outputRotate = Rotate.CW_0;
            }else {
                outputRotate = value;
            }
        }
    }

    /**
     * This method sets the engine needed for transcoding.
     * Any work related to transcoding uses the engine: the user must call this method first.
     * If there are no further changes to the engine, calling is unnecessary.
     * @param engine The engine instance used in the editor.
     * @since version 1.5.29
     */
    public static void init(nexEngine engine ){
        sEngine = engine;
    }

    /**
     * This method sets the transcoding listener.
     * @param listener The UI listener.
     * @return this nexTranscode.
     * @since version 1.5.29
     */
    public nexTranscode setTransCoderListener( OnTransCoderListener listener){
        this.listener = listener;
        return this;
    }

    /**
     * A constructor.
     * @param sourcePath The source content path.
     * @param useMediaInfo
     *        -<b>TRUE</b>: Checks the content first.
     *        -<b>FALSE</b>: Does not check.
     * @since version 1.5.29
     */
    public nexTranscode( String sourcePath , boolean useMediaInfo){
        if( useMediaInfo ) {
            MediaInfo info = MediaInfo.getInfo(sourcePath);
            if (!info.hasVideo()) {
                lastError = Error.NOTSUPPORTEDFILE;
            }
            if( info.isError() ){
                lastError = Error.NOTSUPPORTEDFILE;
            }
        }
        source = new File(sourcePath);
        state = State.IDLE;
    }

    /**
     * This method gets the nexTranscode instance through the source path after setting the listener.
     * In the future version, a newly created nexTranscode instance from this method will be managed separately.
     * @param sourcePath The source content path.
     * @param listener The UI listener.
     * @return A newly created nexTranscode instance.
     * @since version 1.5.29
     */
    public static nexTranscode getTranscode(String sourcePath, OnTransCoderListener listener ){
        return new nexTranscode(sourcePath,false).setTransCoderListener(listener);
    }

    /**
     * Executes transcoding. Calling this API will start transcoding via nexEngine, which means nexEngine must be in the 'stop' status.
     * @param opt The option for transcoding.
     * @return this
     * @since version 1.5.29
     */
    public nexTranscode run(Option opt){
        if( sEngine == null ){
            Log.e(TAG,"must call setEngin()!");
            lastError = Error.ENGINEFAIL;
            if (listener != null) {
                listener.onTransCodeDone(lastError, 1);
            }

            return this;
        }

        if( source == null ){
            Log.e(TAG,"source is null!");
            lastError = Error.SOURCEFAIL;
            if (listener != null) {
                listener.onTransCodeDone(lastError, 1);
            }
            return this;
        }

        if( opt == null ){
            Log.e(TAG,"option is null!");
            lastError = Error.RUNFAIL;
            if (listener != null) {
                listener.onTransCodeDone(lastError, 2);
            }
            return this;
        }

        if( sRunMode == 1 ) {

            TranscodingTask task = Transcoder.findTranscodeTask(source);
            if (task != null) {
                lastError = Error.SOURCEFAIL;
                if (listener != null) {
                    listener.onTransCodeDone(lastError, 2);
                }
                Log.e(TAG, "task is exist!");
                return this;
            }
            progressPercent = 0;
            NexExportProfile exportProfile = new NexExportProfile(opt.outputWidth, opt.outputHeight, opt.outputHeight, opt.outputBitRate, opt.labelResource);
            Log.d(TAG, "( " + exportProfile.width() + " X " + exportProfile.height() + " ) bitrate=" + exportProfile.bitrate() + ", resource=" + exportProfile.labelResource());
            task = Transcoder.transcodeFile(sEngine.getAppContext(), source, opt.outputFile, exportProfile);
            Log.e(TAG, "Transcoder start");
            state = State.WAIT;
            task.onFailure(new Task.OnFailListener() {
                @Override
                public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                    state = State.COMPLETE;
                    lastError = Error.RUNFAIL;
                    if (listener != null) {
                        listener.onTransCodeDone(lastError, 0);
                    }
                }
            }).onProgress(new Task.OnProgressListener() {
                @Override
                public void onProgress(Task t, Task.Event e, int progress, int maxProgress) {
                    state = State.RUNNING;
                    progressPercent = progress*100/maxProgress;
                    if (listener != null) {
                        listener.onProgress(progress, maxProgress);
                    }
                }
            }).onCancel(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    state = State.COMPLETE;
                    lastError = Error.CANCEL;
                    if (listener != null) {
                        listener.onTransCodeDone(lastError, 0);
                    }
                }
            }).onComplete(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    state = State.COMPLETE;
                    if (listener != null) {
                        listener.onTransCodeDone(lastError, 0);
                    }
                }
            }).onSuccess(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    state = State.COMPLETE;
                    if (listener != null) {
                        listener.onTransCodeDone(lastError, 0);
                    }
                }
            });
        }else{

            sEngineListener = new nexEngineListener() {
                @Override
                public void onStateChange(int oldState, int newState) {

                }

                @Override
                public void onTimeChange(int currentTime) {
                    state = State.RUNNING;
                }

                @Override
                public void onSetTimeDone(int currentTime) {

                }

                @Override
                public void onSetTimeFail(int err) {

                }

                @Override
                public void onSetTimeIgnored() {

                }

                @Override
                public void onEncodingDone(boolean iserror, int result) {
                    state = State.COMPLETE;
                    lastError = Error.NONE;
                    if(mVideoEditor != null) mVideoEditor.setProjectEffect(EditorGlobal.getProjectEffect("std"));
                    if(sEngine != null) sEngine.setScalingFlag2Export(false);
                    Log.d(TAG,"onEncodingDone()="+result);
                    if( result == nexEngine.nexErrorCode.EXPORT_USER_CANCEL.getValue() ) {
                        lastError = Error.CANCEL;
                    }else if( result == nexEngine.nexErrorCode.TRANSCODING_BUSY.getValue()  ) {
                        lastError = Error.BUSY;
                    }else if( result == nexEngine.nexErrorCode.TRANSCODING_NOT_SUPPORTED_FORMAT.getValue()  ){
                        lastError = Error.NOTSUPPORTEDFILE;
                    }else if( iserror ){
                        lastError = Error.RUNFAIL;
                    }
                    if (listener != null) {
                        listener.onTransCodeDone(lastError, result);
                    }
                }

                @Override
                public void onPlayEnd() {

                }

                @Override
                public void onPlayFail(int err, int iClipID) {

                }

                @Override
                public void onPlayStart() {

                }

                @Override
                public void onClipInfoDone() {

                }

                @Override
                public void onSeekStateChanged(boolean isSeeking) {

                }

                @Override
                public void onEncodingProgress(int percent) {
                    state = State.RUNNING;
                    progressPercent = percent;
                    if (listener != null) {
                        listener.onProgress(percent,100);
                    }

                }

                @Override
                public void onCheckDirectExport(int result) {

                }

                @Override
                public void onProgressThumbnailCaching(int progress, int maxValue) {

                }

                @Override
                public void onFastPreviewStartDone(int err, int startTime, int endTime) {

                }

                @Override
                public void onFastPreviewStopDone(int err) {

                }

                @Override
                public void onFastPreviewTimeDone(int err) {

                }

                @Override
                public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

                }
            };
            Log.d(TAG, "( " + opt.outputWidth + " X " + opt.outputHeight + " ) bitrate=" + opt.outputBitRate + ", resource=" + opt.labelResource + sEngine );
            if(sEngine != null) sEngine.setScalingFlag2Export(true);
        	  {
	    				mVideoEditor = EditorGlobal.getEditor();
            	mVideoEditor.setProjectEffect(EditorGlobal.getProjectEffect("up"));
          	}
            sEngine.runTranscodeMode(opt,source.getAbsolutePath(), sEngineListener);
        }
        return this;
    }

    /**
     * {@link #run(Option)} stops transcoding which was running.
     * @return <tt>TRUE</tt> if the transcoding is successful; otherwise, <tt>FALSE</tt>.
     * @since version 1.5.29
     */
    public boolean cancel(){
        if( sRunMode == 1 ) {
            TranscodingTask task = Transcoder.findTranscodeTask(source);
            if (task != null) {
                return false;
            }

            task.cancel();
        }else{
            if(mVideoEditor != null) mVideoEditor.setProjectEffect(EditorGlobal.getProjectEffect("std"));
            if(sEngine != null) sEngine.setScalingFlag2Export(false);
            sEngine.stopTranscode();
        }
        return true;
    }

    /**
     * This method shows the transcoding progress in percentages.
     * @return Transcoding The progress.
     * @since version 1.5.29
     */
    public int getProgress(){
        return progressPercent;
    }

    /**
     * This method gets the current state.
     * @return {@link com.nexstreaming.nexeditorsdk.nexTranscode.State State}
     * @since version 1.5.29
     */
    public State getCurrentState(){
        return state;
    }

    /**
     * This method gets the last error value.
     * @return {@link com.nexstreaming.nexeditorsdk.nexTranscode.Error Error}
     * @since version 1.5.29
     */
    public Error getLastError(){
        if(mVideoEditor != null)mVideoEditor.setProjectEffect(EditorGlobal.getProjectEffect("std"));
        if(sEngine != null)sEngine.setScalingFlag2Export(false);
        return lastError;
    }
}
