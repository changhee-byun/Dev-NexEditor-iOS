/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_MsgDef.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/

#ifndef _NEXVIDEOEDITOR_MSGDEF__H_
#define _NEXVIDEOEDITOR_MSGDEF__H_

typedef enum
{
	MESSAGE_NONE = 0,
	MESSAGE_STATE_CHANGE,					// 1
	MESSAGE_STATE_CHANGE_DONE,				// 2
	MESSAGE_UPDATE_CURRENTIME,				// 3
	MESSAGE_OPEN_PROJECT,					// 4
	MESSAGE_OPEN_PROJECT_PROGRESS,			// 5
	MESSAGE_OPEN_PROJECT_DONE,				// 6
	MESSAGE_SAVE_PROJECT,					// 7
	MESSAGE_SAVE_PROJECT_DONE,				// 8
	MESSAGE_CLOSE_PROJECT,					// 9
	MESSAGE_CLOSE_PROJECT_DONE,				// 10
	MESSAGE_ENCODE_PROJECT,					// 11
	MESSAGE_ENCODE_PROJECT_PROGRESS,		// 12
	MESSAGE_ENCODE_PROJECT_DONE,			// 13
	MESSAGE_ADD_VISUAL_CLIP,				// 14
	MESSAGE_ADD_VISUAL_CLIP_DONE,			// 15
	MESSAGE_ADD_AUDIO_CLIP,					// 16
	MESSAGE_ADD_AUDIO_CLIP_DONE,			// 17
	MESSAGE_ADD_CLIP_PROGRESS,				// 18
	MESSAGE_ADD_CLIP_DONE,					// 19
	MESSAGE_MOVE_CLIP,						// 20
	MESSAGE_MOVE_CLIP_PROGRESS,				// 21
	MESSAGE_MOVE_CLIP_DONE,					// 22
	MESSAGE_DELETE_CLIP,					// 23
	MESSAGE_DELETE_CLIP_PROGRESS,			// 24
	MESSAGE_DELETE_CLIP_DONE,				// 25
	MESSAGE_UPDATE_CLIP,					// 26
	MESSAGE_UPDATE_CLIP_DONE,				// 27
	MESSAGE_CLIP_PLAY,						// 28
	MESSAGE_CLIP_PLAY_END,					// 29
	MESSAGE_GET_DURATION,					// 30
	MESSAGE_GET_DURATION_DONE,				// 31
	MESSAGE_SET_TIME,						// 32
	MESSAGE_SET_TIME_DONE,					// 33
	MESSAGE_AUDIO_RENDERING_INFO,			// 34
	MESSAGE_VIDEO_RENDERING_INFO,			// 35
	MESSAGE_ADD_TRACK,						// 36
	MESSAGE_DELETE_TRACK,					// 37
	MESSAGE_VIDEO_GETTHUMBNAIL,				// 38
	MESSAGE_VIDEO_RENDER_POSITION,			// 39
	MESSAGE_VIDEO_RENDER_POSITION_DONE,		// 40
	MESSAGE_TEMP_DIRECTORY,					// 41
	MESSAGE_THUMBNAIL_DIRECTORY,			// 42
	MESSAGE_LOAD_THEME_FILE,				// 43
	MESSAGE_LOAD_THEME_FILE_DONE,			// 44
	MESSAGE_SET_AUDIOTRACK,					// 45
	MESSAGE_SET_THEME,						// 46
	MESSAGE_SET_EFFECT_INFO,				// 47
	MESSAGE_SEND_EVENT,						// 48
	MESSAGE_SET_FILEWRITER,					// 49
	MESSAGE_RESET_FILEWRITER,				// 50
	MESSAGE_VIDEO_STARTED,					// 51
	MESSAGE_STOP_PLAYTASK,					// 52
	MESSAGE_AUDIO_ON,						// 53
	MESSAGE_AUDIO_OFF,						// 54
	MESSAGE_BACKGROUND_MUSIC,				// 55
	MESSAGE_BACKGROUND_MUSIC_VOLUME,		// 56
	MESSAGE_INIT_DRAW_RENDER,				// 57
	MESSAGE_CMD_MARKER,						// 58
	MESSAGE_CMD_MARKER_CHECKED,				// 59
	MESSAGE_CMD_CLEAR_PROJECT,				// 60
	MESSAGE_VDEC_INIT_FAIL,					// 61
	MESSAGE_ADEC_INIT_FAIL,					// 62
	MESSAGE_ARAL_INIT_FAIL,					// 63
	MESSAGE_ADEC_DEC_FAIL,					// 64
	MESSAGE_VDEC_DEC_FAIL,					// 65
	MESSAGE_AUDIO_RENDER_FAIL,				// 66
	MESSAGE_VIDEO_RENDER_CLEAR,				// 67
	MESSAGE_GETCLIPINFO_BACKGROUND,			// 68
	MESSAGE_GETCLIPINFO_BACKGROUND_DONE,	// 69
	MESSAGE_CAPTURE,						// 70
	MESSAGE_CAPTURE_DONE,					// 71
	MESSAGE_CLEAR_SCREEN,					// 72
	MESSAGE_CLEAR_SCREEN_DONE,				// 73
	MESSAGE_FAST_OPTION_PREVIEW,			// 74
	MESSAGE_FAST_OPTION_PREVIEW_DONE,		// 75
	MESSAGE_PREPARE_SURFACE,				// 76
	MESSAGE_PREPARE_SURFACE_DONE,			// 77
	MESSAGE_PREPARE_CLIP_LOADING,			// 78
	MESSAGE_CLEAR_TEXTURE,					// 79
	MESSAGE_CLEAR_SURFACETEXTURE,			// 80
	MESSAGE_PAUSE_RENDERER,					// 81
	MESSAGE_RESUME_RENDERER,				// 82
	MESSAGE_CLEAR_TRACK_CACHE,				// 83
	MESSAGE_TRANSCODING_START,				// 84
	MESSAGE_TRANSCODING_STOP,				// 85
	MESSAGE_TRANSCODING_PROGRESS,			// 86
	MESSAGE_TRANSCODING_DONE,				// 87
	MESSAGE_DELETE_TRACK_ALL,				// 88
	MESSAGE_SET_PROJECT_EFFECT,				// 89
	MESSAGE_SET_PROJECT_VOLUME_FADE,		// 90
	
	MESSAGE_SET_ADD_LAYER_ITEM,				// 91
	MESSAGE_SET_DELETE_LAYER_ITEM,			// 92
	MESSAGE_SET_CLEAR_LAYER_ITEM,			// 93

	MESSAGE_ADD_LAYER_ITEM,					// 94
	MESSAGE_DELETE_LAYER_ITEM,				// 95

	MESSAGE_UPDATE_LOADLIST,				// 96

	MESSAGE_GETTHUMB_RAWDATA,				// 97
	MESSAGE_GETTHUMB_RAWDATA_DONE,			// 98

	MESSAGE_LOAD_THEMEANDEFFECT,			// 99
	MESSAGE_MAKE_HIGHLIGHT_START,			// 100
	MESSAGE_MAKE_HIGHLIGHT_START_DONE,		// 101
	MESSAGE_MAKE_HIGHLIGHT_STOP,			// 102
	MESSAGE_MAKE_HIGHLIGHT_STOP_DONE,		// 103
	MESSAGE_MAKE_HIGHLIGHT_PROGRESS_INDEX,	// 104
	MESSAGE_MAKE_HIGHLIGHT_PROGRESS,		// 105


	MESSAGE_MAKE_CHECK_DIRECT_EXPORT,		// 106
	MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE,	// 107

	MESSAGE_MAKE_DIRECT_EXPORT_START,		// 108
	MESSAGE_MAKE_DIRECT_EXPORT_DONE,		// 109
	MESSAGE_MAKE_DIRECT_EXPORT_PROGRESS,	// 110

	MESSAGE_GETCLIPINFO_BACKGROUND_STOP,	// 111
	MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE,// 112

	MESSAGE_ASYNC_UPDATE_CLIPLIST,			// 113
	MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE,		// 114

	MESSAGE_FAST_PREVIEW_START,				// 115
	MESSAGE_FAST_PREVIEW_START_DONE,		// 116

	MESSAGE_FAST_PREVIEW_STOP,				// 117
	MESSAGE_FAST_PREVIEW_STOP_DONE,			// 118

	MESSAGE_FAST_PREVIEW_TIME,				// 119
	MESSAGE_FAST_PREVIEW_TIME_DONE,			// 120

	MESSAGE_FAST_PREVIEW_RENDER_INIT,		// 121
	MESSAGE_FAST_PREVIEW_RENDER_DEINIT,		// 122
	MESSAGE_FAST_PREVIEW_ADD_FRAME,			// 123

	MESSAGE_HIGHLIGHT_THUMBNAIL_PROGRESS,	// 124

	MESSAGE_WAIT_COMMAND,					// 125

	MESSAGE_MAKE_REVERSE_START,				// 126
	MESSAGE_MAKE_REVERSE_START_DONE,		// 127
	MESSAGE_MAKE_REVERSE_STOP,				// 128
	MESSAGE_MAKE_REVERSE_STOP_DONE,			// 129
	MESSAGE_MAKE_REVERSE_PROGRESS,			// 130

	MESSAGE_SET_VOLUME_WHILE_PLAYING,		// 131
	MESSAGE_RELEASE_RENDERER_CACHE,			// 132

	MESSAGE_SET_PROJECT_VOLUME,				//133
	MESSAGE_SET_MANUAL_VOLUME_CONTROL,		//134

	MESSAGE_ASYNC_UPDATE_EFFECTLIST,		// 113
	MESSAGE_ASYNC_UPDATE_EFFECTLIST_DONE,	// 114

	MESSAGE_PREVIEW_PEAKMETER,				// 115

	MESSAGE_DUMMY,							// 116
	MESSAGE_DUMMY_DONE,						// 117
	
	MESSAGE_LOAD_RENDERITEM,				// 118
	MESSAGE_CLEAR_RENDERITEMS,				// 119

	MESSAGE_SET_DRAWINFO_LIST,				// 120
	MESSAGE_SET_DRAWINFO_LIST_DONE,			// 121

	MESSAGE_SET_DRAWINFO,					// 120
	MESSAGE_SET_DRAWINFO_DONE,				// 121
	MESSAGE_TASK_SLEEP,						// 122
	MESSAGE_TASK_SLEEP_DONE,				// 123
	MESSAGE_UPDATE_RENDERINFO,				// 124
	MESSAGE_RESET_FACEDETECTINFO,			// 125

	MESSAGE_ENCODE_PROJECT_JPEG,			// 126
	MESSAGE_ENCODE_PROJECT_JPEG_DONE,		// 127

	MESSAGE_LOAD_ORIGINAL_IMAGE,			// 128
	MESSAGE_DRAWIDLE,						// 129

	MESSAGE_SET_GIF_MODE,					// 130
	MESSAGE_SET_GIF_MODE_DONE,				// 131

	MESSAGE_MAKE_VAS_HIGHLIGHT_START_DONE,		// 132 Don't use.
	MESSAGE_UPDATE_CLIP_FOR_EXPORT,			// 133

    MESSAGE_GET_BASERENDERER_INFO,//134
    MESSAGE_SET_THEMERENDER,//135
    MESSAGE_PREPARESURFACE,//136
    MESSAGE_GETSURFACETEXTURE,//137
	MESSAGE_RELEASESURFACETEXTURE,//138
    MESSAGE_DESTROYSURFACETEXTURE,//139
    MESSAGE_SETSEPARATEEFFECT,//140
    MESSAGE_SETVISUALCOUNTATTIME,//141
    MESSAGE_SETGLOPERATIONALLOWED,//142
    MESSAGE_CHECKISLOSTNATIVEWINDOW,//143
    MESSAGE_SET_MAPPER,//144

} NEXVIDEOEDITOR_MESSAGE;

#endif // _NEXVIDEOEDITOR_MSGDEF__H_