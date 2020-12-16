package com.nexstreaming.kminternal.nexvideoeditor;

/**
 * Provides information on event type.
 *
 * @author Nextreaming Corporation
 */

public final class NexEditorEventType {
    public static final int VIDEOEDITOR_EVENT_STATE_CHANGE = 0;

    // uiParam1 : Current Time
    public static final int VIDEOEDITOR_EVENT_CURRENTTIME = 1;

    // uiParam1 : Open Progress
    public static final int VIDEOEDITOR_EVENT_OPEN_PROJECT = 2;

    public static final int VIDEOEDITOR_EVENT_CLOSE_PROJECT = 3;

    public static final int VIDEOEDITOR_EVENT_ADDCLIP_PROCESS = 4;
    // uiParam1 : Return code
    // uiParam2 : Total duration
    // uiParam3 : added clip id
    // uiParam4 : added clip type  (0=None, 1=Image, 2=Text, 3=Audio, 4=Video)
    public static final int VIDEOEDITOR_EVENT_ADDCLIP_DONE = 5;

    // uiParam1 : Return code
    // uiParam2 : Move Clip ID
    public static final int VIDEOEDITOR_EVENT_MOVECLIP_DONE = 6;
    public static final int VIDEOEDITOR_EVENT_DELETECLIP_DONE = 7;
    public static final int VIDEOEDITOR_EVENT_EFFECTCHANGE_DONE = 8;

    // uiParam1 : 1 is update exist, 0 is update do not exist
    public static final int VIDEOEDITOR_EVENT_UPDATECLIP_DONE = 9;

    // uiParam1 : Current Time
    // uiParam2 : Result ignore if param2 is 1.
    public static final int VIDEOEDITOR_EVENT_SETTIME_DONE = 10;
    // uiParam1 : Error reason
    public static final int VIDEOEDITOR_EVENT_PLAY_ERROR = 11;
    // uiParam1 : Clip ID
    public static final int VIDEOEDITOR_EVENT_THUMBNAIL_CREATE_DONE = 12;
    // uiParam1 : Encode progress
    public static final int VIDEOEDITOR_EVENT_ENCODE_PROJECT = 13;

    public static final int VIDEOEDITOR_EVENT_ENCODE_ERROR = 14;

    // uiParam1 : Result
    public static final int VIDEOEDITOR_EVENT_THEME_LOAD_DONE = 15;

    // uiParam1 : Result
    public static final int VIDEOEDITOR_EVENT_CREATE_AUDIO_TRACK = 16;

    public static final int VIDEOEDITOR_EVENT_PLAY_END = 17;

    public static final int VIDEOEDITOR_EVENT_CMD_MARKER_CHECKED = 18;

    public static final int VIDEOEDITOR_EVENT_VIDEO_STARTED = 19;

    public static final int VIDEOEDITOR_EVENT_GETCLIPINFO_DONE = 20;

    // uiParam1 : Result
    public static final int VIDEOEDITOR_EVENT_CAPTURE_DONE = 21;

    // uiParam1 : ClipID
    public static final int VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING = 22;

    // uiParam1 : Result
    // uiParam2 : user tag
    public static final int VIDEOEDITOR_EVENT_TRANSCODING_DONE = 23;

    // uiParam1 : progress value percent (0~100)
    // uiParam2 : current time
    // uiParam3 : total time
    public static final int VIDEOEDITOR_EVENT_TRANSCODING_PROGRESS = 24;

    // uiParam1 : 0 : success, 1 : fail
    public static final int VIDEOEDITOR_EVENT_PROJECT_VOLUME_FADE = 25;

    public static final int VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE = 26;

    public static final int VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_DONE = 27;

    public static final int VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX = 28;

    public static final int VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS = 29;

    public static final int VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT = 30;

    public static final int VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE = 31;

    public static final int VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS = 32;

    public static final int VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE = 33;

    public static final int VIDEOEDITOR_EVENT_ASYNC_LOADLIST = 34;

    public static final int VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE = 35;

    public static final int VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE = 36;

    public static final int VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE = 37;

    public static final int VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS = 38;

    public static final int VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE = 39;

    public static final int VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS = 40;

    public static final int VIDEOEDITOR_EVENT_PREVIEW_PEAKMETER = 51;

    public static final int THUMBNAIL_GEETTHUMB_RAWDATA_DONE = 101;

    public static final int KM_EVENT_DIAGNOSIS = 5001;
}

