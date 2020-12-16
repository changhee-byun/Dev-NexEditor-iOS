/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_EventTypes.h
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_EVENTTYPES__H_
#define _NEXVIDEOEDITOR_EVENTTYPES__H_
//---------------------------------------------------------------------------

typedef enum _VIDEOEDITOR_EVENT
{
	// uiParam1 : Pre state
	// uiParam2 : Current state
	VIDEOEDITOR_EVENT_STATE_CHANGE				= 0,

	// uiParam1 : Current Time
	VIDEOEDITOR_EVENT_CURRENTTIME				= 1,

	// uiParam1 : Open Progress
	VIDEOEDITOR_EVENT_OPEN_PROJECT				= 2,

	VIDEOEDITOR_EVENT_CLOSE_PROJECT				= 3,

	VIDEOEDITOR_EVENT_ADDCLIP_PROCESS			= 4,

	// uiParam1 : Return code
	// uiParam2 : Total duration
	// uiParam3 : Inserted clip id
	// uiParam4 : Inserted clip type
	VIDEOEDITOR_EVENT_ADDCLIP_DONE				= 5,
	
	// uiParam1 : Return code
	// uiParam2 : Move Clip ID
 	VIDEOEDITOR_EVENT_MOVECLIP_DONE			= 6,
	VIDEOEDITOR_EVENT_DELETECLIP_DONE			= 7,
	VIDEOEDITOR_EVENT_EFFECTCHANGE_DONE		= 8,

	// uiParam1 : 1 is update exist, 0 is update do not exist
	VIDEOEDITOR_EVENT_UPDATECLIP_DONE			= 9,

	// uiParam1 : Current Time
	// uiParam2 : Ignore flag, Ignore if uiParam2 is 1
	// uiParam3 : Tag value if setted value
	VIDEOEDITOR_EVENT_SETTIME_DONE				= 10,
	
	// uiParam1 : Error reason
	VIDEOEDITOR_EVENT_PLAY_ERROR				= 11,

	// uiParam1 : Clip ID
	VIDEOEDITOR_EVENT_THUMBNAIL_CREATE_DONE	= 12,

	// uiParam1 : Encode progress
	VIDEOEDITOR_EVENT_ENCODE_PROJECT			= 13,

	// uiParam1 : Error reason
	VIDEOEDITOR_EVENT_ENCODE_ERROR				= 14,

	// uiParam1 : Result
	VIDEOEDITOR_EVENT_THEME_LOAD_DONE			= 15,
	// uiParam1 : Result
	VIDEOEDITOR_EVENT_CREATE_AUDIO_TRACK		= 16,
	VIDEOEDITOR_EVENT_PLAY_END					= 17,
	VIDEOEDITOR_EVENT_CMD_MARKER_CHECKED		= 18,
	VIDEOEDITOR_EVENT_VIDEO_STARTED				= 19,

	VIDEOEDITOR_EVENT_GETCLIPINFO_DONE			= 20,
	VIDEOEDITOR_EVENT_CAPTURE_DONE				= 21,
	VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING		= 22,
	VIDEOEDITOR_EVENT_TRANSCODING_DONE			= 23,
	VIDEOEDITOR_EVENT_TRANSCODING_PROGRESS		= 24,

	VIDEOEDITOR_EVENT_PROJECT_VOLUME_FADE		= 25,

	VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE	= 26,
	
	VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_DONE		= 27,
	
	VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX		= 28,
	VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS		= 29,
	
	VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT			= 30,
	
	VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE			= 31,
	VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS		= 32,
	
	VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE		= 33,
	
	VIDEOEDITOR_EVENT_ASYNC_LOADLIST				= 34,

	VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE			= 35,

	VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE				= 36,

	VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE				= 37,
	
	VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS		= 38,	

	VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE			= 39,
	
	VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS		= 40,

	VIDEOEDITOR_EVENT_PREVIEW_PEAKMETER			= 51,

	// add thumb module
	THUMBNAIL_GEETTHUMB_RAWDATA_DONE	= 101,
	
		// for KineMaster
	KM_EVENT_ENTRY = 5000,
	KM_EVENT_DIAGNOSIS,
	
}VIDEOEDITOR_EVENT;

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_EVENTTYPES__H_

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/
