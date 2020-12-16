package com.nexstreaming.kminternal.nexvideoeditor;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexEditorType
{
	public static final int PLAY_STATE_NONE		= 0;
	public static final int PLAY_STATE_IDLE		= 1;
	public static final int PLAY_STATE_PREVIEW	= 2;
	public static final int PLAY_STATE_RUN		= 3;
	public static final int PLAY_STATE_RECORD	= 4;
	
	public static final int NEXCLIP_BASE_ID = 0x00000000;

	public static final int NEXCLIP_TYPE_NONE = 0x00000000;
	public static final int NEXCLIP_TYPE_IMAGE = 0x00000001;
	public static final int NEXCLIP_TYPE_TEXT = 0x00000002;
	public static final int NEXCLIP_TYPE_AUDIO = 0x00000003;
	public static final int NEXCLIP_TYPE_VIDEO = 0x00000004;
    public static final int NEXCLIP_TYPE_EFFECT = 0x00000005;
    public static final int NEXCLIP_TYPE_LAYER = 0x00000006;
    public static final int NEXCLIP_TYPE_VIDEO_LAYER = 0x00000007;

	public static final int NEXTITLE_STYLE_NONE = 0x00000000;
	public static final int NEXTITLE_STYLE_HEADLINE = 0x00000001;
	public static final int NEXTITLE_STYLE_OPENNING = 0x00000002;
	public static final int NEXTITLE_STYLE_PLAYING = 0x00000003;
	public static final int NEXTITLE_STYLE_ENDING = 0x00000004;

    public static final int GET_CLIPINFO_INCLUDE_SEEKTABLE = 0x00000001;
    public static final int GET_CLIPINFO_CHECK_AUDIO_DEC = 0x00000010;
    public static final int GET_CLIPINFO_CHECK_VIDEO_DEC = 0x00000100;
}

