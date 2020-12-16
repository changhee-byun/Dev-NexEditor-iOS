/******************************************************************************
 * File Name        : nexEngineListener.java
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

/**
 * The application must implement this interface, from nexEngine {@link nexEngine#setEventHandler(nexEngineListener)},
 * in order to receive events from NexEditor&trade;&nbsp;SDK.
 <p><b>State Value</b></p>
 <ul>
    <li>NONE(0)
    <li>IDLE(1)
    <li>RUN(2)
    <li>RECORD(3)
 </ul>
 <p><b>Error Value</b></p>
 <ul>
 <li>NONE(								0),
 <li>GENERAL(							1),
 <li>UNKNOWN(							2),
 <li>NO_ACTION(							3),
 <li>INVALID_INFO(						4),
 <li>INVALID_STATE(						5),
 <li>VERSION_MISMATCH(					6),
 <li>CREATE_FAILED(						7),
 <li>MEMALLOC_FAILED(					8),
 <li>ARGUMENT_FAILED(					9),
 <li>NOT_ENOUGH_NEMORY(					10),
 <li>EVENTHANDLER(						11),
 <li>FILE_IO_FAILED(						12),
 <li>FILE_INVALID_SYNTAX(				13),
 <li>FILEREADER_CREATE_FAIL(				14),
 <li>FILEWRITER_CREATE_FAIL(				15),
 <li>AUDIORESAMPLER_CREATE_FAIL(			16),
 <li>UNSUPPORT_FORMAT(					17),
 <li>FILEREADER_FAILED(					18),
 <li>PLAYSTART_FAILED(					19),
 <li>PLAYSTOP_FAILED(					20),
 <li>PROJECT_NOT_CREATE(					21),
 <li>PROJECT_NOT_OPEN(					22),
 <li>CODEC_INIT(							23),
 <li>RENDERER_INIT(						24),
 <li>THEMESET_CREATE_FAIL(				25),
 <li>ADD_CLIP_FAIL(						26),
 <li>ENCODE_VIDEO_FAIL(					27),
 <li>INPROGRESS_GETCLIPINFO(     		28),
 <li>THUMBNAIL_BUSY(						29),
 <li>UNSUPPORT_MIN_DURATION(				30),
 <li>UNSUPPORT_MAX_RESOLUTION(			31),
 <li>UNSUPPORT_MIN_RESOLUTION(			32),
 <li>UNSUPPORT_VIDEIO_PROFILE(			33),
 <li>UNSUPPORT_VIDEO_LEVEL(				34),
 <li>UNSUPPORT_VIDEO_FPS(				35),
 <li>TRANSCODING_BUSY(					36),
 <li>TRANSCODING_NOT_SUPPORTED_FORMAT(	37),
 <li>TRANSCODING_USER_CANCEL(			38),
 <li>TRANSCODING_NOT_ENOUGHT_DISK_SPACE(39),
 <li>TRANSCODING_CODEC_FAILED(           40),
 <li>EXPORT_WRITER_INVAILED_HANDLE(		41),
 <li>EXPORT_WRITER_INIT_FAIL(            42),
 <li>EXPORT_WRITER_START_FAIL(		    43),
 <li>EXPORT_AUDIO_DEC_INIT_FAIL(			44),
 <li>EXPORT_VIDEO_DEC_INIT_FAIL(         45),
 <li>EXPORT_VIDEO_ENC_FAIL(              46),
 <li>EXPORT_VIDEO_RENDER_INIT_FAIL(		47),
 <li>EXPORT_NOT_ENOUGHT_DISK_SPACE(		48),
 <li>UNSUPPORT_AUDIO_PROFILE(            49),
 <li>THUMBNAIL_INIT_FAIL(                50),
 <li>UNSUPPORT_AUDIO_CODEC(              51),
 <li>UNSUPPORT_VIDEO_CODEC(              52),
 <li>HIGHLIGHT_FILEREADER_INIT_ERROR(	53),
 <li>HIGHLIGHT_TOO_SHORT_CONTENTS(		54),
 <li>HIGHLIGHT_CODEC_INIT_ERROR(			55),
 <li>HIGHLIGHT_CODEC_DECODE_ERROR(		56),
 <li>HIGHLIGHT_RENDER_INIT_ERROR(		57),
 <li>HIGHLIGHT_WRITER_INIT_ERROR(		58),
 <li>HIGHLIGHT_WRITER_WRITE_ERROR(		59),
 <li>HIGHLIGHT_GET_INDEX_ERROR(			60),
 <li>HIGHLIGHT_USER_CANCEL(				61),
 <li>GETCLIPINFO_USER_CANCEL(				62),
 <li>DIRECTEXPORT_CLIPLIST_ERROR	(				63),
 <li>DIRECTEXPORT_CHECK_ERROR(					64),
 <li>DIRECTEXPORT_FILEREADER_INIT_ERROR(		65),
 <li>DIRECTEXPORT_FILEWRITER_INIT_ERROR(		66),
 <li>DIRECTEXPORT_DEC_INIT_ERROR(				67),
 <li>DIRECTEXPORT_DEC_INIT_SURFACE_ERROR(		68),
 <li>DIRECTEXPORT_DEC_DECODE_ERROR(				69),
 <li>DIRECTEXPORT_ENC_INIT_ERROR(				70),
 <li>DIRECTEXPORT_ENC_ENCODE_ERROR(				71),
 <li>DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR(		72),
 <li>DIRECTEXPORT_ENC_FUNCTION_ERROR(			73),
 <li>DIRECTEXPORT_ENC_DSI_DIFF_ERROR(			74),
 <li>DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR(		75),
 <li>DIRECTEXPORT_RENDER_INIT_ERROR(			76),
 <li>DIRECTEXPORT_WRITER_WRITE_ERROR(			77),
 <li>DIRECTEXPORT_WRITER_UNKNOWN_ERROR(			78),
 <li>FASTPREVIEW_USER_CANCEL(					79),
 <li>FASTPREVIEW_CLIPLIST_ERROR(				80),
 <li>FASTPREVIEW_FIND_CLIP_ERROR(			81),
 <li>FASTPREVIEW_FIND_READER_ERROR(			82),
 <li>FASTPREVIEW_VIDEO_RENDERER_ERROR(		83),
 <li>FASTPREVIEW_DEC_INIT_SURFACE_ERROR(	84),
 <li>HW_NOT_ENOUGH_MEMORY(					85),
 <li>EXPORT_USER_CANCEL(						86),
 <li>FASTPREVIEW_DEC_INIT_ERROR(				87),
 <li>FASTPREVIEW_FILEREADER_INIT_ERROR(		88),
 <li>FASTPREVIEW_TIME_ERROR(					89),
 <li>FASTPREVIEW_RENDER_INIT_ERROR(			90),
 <li>FASTPREVIEW_OUTPUTSURFACE_INIT_ERROR(	91),
 <li>FASTPREVIEW_FASTPREVIEW_BUSY(			92),
 
 <li>CODEC_DECODE(							93),
 <li>RENDERER_AUDIO(						94);

 </ul>
 * @since version 1.0.0
 *
 */

public interface nexEngineListener {
    /**
     *  This method is called when there is a change of status.
     * @param oldState
     * @param newState
     * @since version 1.0.0
     */
    void onStateChange(int oldState, int newState);

    /**
     *  This method is called when time changes while {@link nexEngine#play()}.
     * @param currentTime
     * @since version 1.0.0
     */
    void onTimeChange(int currentTime);

    /**
     *  This method is called when {@link nexEngine#seek(int)} is successful.
     * @param currentTime
     * @since version 1.0.0
     */
    void onSetTimeDone(int currentTime);

    /**
     *  This method is called when {@link nexEngine#seek(int)} has failed.
     * @param err
     * @since version 1.0.0
     */
    void onSetTimeFail(int err);

    /**
     *  This method is called when ignored by {@link nexEngine#seek(int)}.
     * @since version 1.0.0
     */
    void onSetTimeIgnored();

    /**
     *  This method is called when {@link nexEngine#export(String, int, int, int, long, int)} operation has completed.
     * @param iserror
     * @param result
     * @since version 1.0.0
     */
    void onEncodingDone(boolean iserror, int result);

    /**
     *  This method is called when {@link nexEngine#play()} ends.
     * @since version 1.0.0
     */
    void onPlayEnd();

    /**
     * This method is called when {@link nexEngine#play()} fails.
     * @param err
     * @param iClipID
     * @since version 1.0.0
     */
    void onPlayFail(int err, int iClipID);

    /**
     *  This method is called when {@link nexEngine#play()} starts.
     * @since version 1.0.0
     */
    void onPlayStart();

    /**
     * @deprecated For internal use only. Please do not use.
     */
    @Deprecated
    void onClipInfoDone();

    /**
     * This method notifies the application if the <tt>nexEngine</tt> is currently seeking.
     * 
     * @param isSeeking  set to <tt>TRUE</tt> if seeking; <tt>FALSE</tt> if seeking has completed. 
     *
     * @since version 1.0.0
     */
    void onSeekStateChanged(boolean isSeeking);

    /**
     *  This method returns the current process percentage after running {@link nexEngine#export(String, int, int, int, long, int)}.
     * 
     * @param percent   The current export process in percentage. 
     * 
     * @since version 1.0.0
     */
    void onEncodingProgress(int percent);

    /**
     * This method returns the result of {@link nexEngine#checkDirectExport()}.
     * @param result      The value input as the parameter <tt>result</tt> of the method {@link nexEngine#checkDirectExport()}.
     * @since 1.3.17
     */
    void onCheckDirectExport(int result);

    /**
     * This method returns the current process percentage when retrieving thumbnails. 
     * 
     * @param progress The current retrieving process in percentage. 
     * @param maxValue  The maximum number of thumbnails to be retrieved by this method. 
     * @since 1.3.21
     */
    void onProgressThumbnailCaching(int progress, int maxValue);

    /**
     * This method returns the status of <tt>fastPreviewStart</tt> function.
     *
     * @param err An error code.
     * @param startTime  The start time of the fast preview. 
     * @param endTime The end time of the fast preview. 
     * @since version 1.3.21
     */
    void onFastPreviewStartDone(int err, int startTime, int endTime);
     
    /**
     * This method returns the status of <tt>fastPreviewStop</tt> function.
     * @param err An error code.
     * @since version 1.3.21
     */
    void onFastPreviewStopDone(int err);

    /**
     * This method returns the status of <tt>fastPreviewTime</tt> function.
     *
     * @param err An error code.
     * @since version 1.3.21
     */
    void onFastPreviewTimeDone(int err);

    /**
     *  
     * This method returns the Peak value of Audio in preview.
     * @param iCts
     * @param iLeftValue
     * @param iRightValue
     * 
     * @since version
     */
    void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue);
}
