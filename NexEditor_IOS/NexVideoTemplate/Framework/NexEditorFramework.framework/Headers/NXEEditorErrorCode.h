/******************************************************************************
 * File Name   :	NXEditorErrorCode.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif


/**
 * \ingroup types
 * \brief Error Code
 */
typedef NS_ENUM(NSUInteger, ERRORCODE) 
{
    ERROR_NONE = 0,
    ERROR_GENERAL,
    ERROR_UNKNOWN,
    ERROR_NO_ACTION,
    ERROR_INVALID_INFO,
    ERROR_INVALID_STATE,
    ERROR_VERSION_MISMATCH,
    ERROR_CREATE_FAILED,
    ERROR_MEMALLOC_FAILED,
    ERROR_ARGUMENT_FAILED,
    ERROR_NOT_ENOUGH_NEMORY, 		// 10
    ERROR_EVENTHANDLER,
    ERROR_FILE_IO_FAILED,
    ERROR_FILE_INVALID_SYNTAX,
    ERROR_FILEREADER_CREATE_FAIL,
    ERROR_FILEWRITER_CREATE_FAIL,
    ERROR_AUDIORESAMPLER_CREATE_FAIL,
    ERROR_UNSUPPORT_FORMAT,
    ERROR_FILEREADER_FAILED,
    ERROR_PLAYSTART_FAILED,
    ERROR_PLAYSTOP_FAILED,			// 20
    ERROR_PROJECT_NOT_CREATE,
    ERROR_PROJECT_NOT_OPEN,
    ERROR_CODEC_INIT,
    ERROR_RENDERER_INIT,
    ERROR_THEMESET_CREATE_FAIL,
    ERROR_ADD_CLIP_FAIL,
    ERROR_ENCODE_VIDEO_FAIL,
    ERROR_INPROGRESS_GETCLIPINFO,
    ERROR_THUMBNAIL_BUSY,
    ERROR_UNSUPPORT_MIN_DURATION,		// 30
    ERROR_UNSUPPORT_MAX_RESOLUTION,
    ERROR_UNSUPPORT_MIN_RESOLUTION,
    ERROR_UNSUPPORT_VIDEO_PROFILE,
    ERROR_UNSUPPORT_VIDEO_LEVEL,
    ERROR_UNSUPPORT_VIDEO_FPS,
    ERROR_TRANSCODING_BUSY,
    ERROR_TRANSCODING_NOT_SUPPORTED_FORMAT,
    ERROR_TRANSCODING_USER_CANCEL,
    ERROR_TRANSCODING_NOT_ENOUGHT_DISK_SPACE,
    ERROR_TRANSCODING_CODEC_FAILED,		// 40
    ERROR_EXPORT_WRITER_INVAILED_HANDLE,
    ERROR_EXPORT_WRITER_INIT_FAIL,
    ERROR_EXPORT_WRITER_START_FAIL,
    ERROR_EXPORT_AUDIO_DEC_INIT_FAIL,
    ERROR_EXPORT_VIDEO_DEC_INIT_FAIL,
    ERROR_EXPORT_VIDEO_ENC_FAIL,
    ERROR_EXPORT_VIDEO_RENDER_INIT_FAIL,
    ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE,
    ERROR_UNSUPPORT_AUDIO_PROFILE,
    ERROR_THUMBNAIL_INIT_FAIL,			// 50
    ERROR_UNSUPPORT_AUDIO_CODEC,
    ERROR_UNSUPPORT_VIDEO_CODEC,
    ERROR_HIGHLIGHT_FILEREADER_INIT_ERROR,
    ERROR_HIGHLIGHT_TOO_SHORT_CONTENTS,
    ERROR_HIGHLIGHT_CODEC_INIT_ERROR,
    ERROR_HIGHLIGHT_CODEC_DECODE_ERROR,
    ERROR_HIGHLIGHT_RENDER_INIT_ERROR,
    ERROR_HIGHLIGHT_WRITER_INIT_ERROR,
    ERROR_HIGHLIGHT_WRITER_WRITE_ERROR,
    ERROR_HIGHLIGHT_GET_INDEX_ERROR,	// 60
    ERROR_HIGHLIGHT_USER_CANCEL,
    ERROR_BACKGROUND_USER_CANCEL,
    ERROR_DIRECTEXPORT_CLIPLIST_ERROR,
    ERROR_DIRECTEXPORT_CHECK_ERROR,
    ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR,
    ERROR_DIRECTEXPORT_FILEWRITER_INIT_ERROR,
    ERROR_DIRECTEXPORT_DEC_INIT_ERROR,
    ERROR_DIRECTEXPORT_DEC_INIT_SURFACE_ERROR,
    ERROR_DIRECTEXPORT_DEC_DECODE_ERROR,
    ERROR_DIRECTEXPORT_ENC_INIT_ERROR,					// 70
    ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR,
    ERROR_DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR,
    ERROR_DIRECTEXPORT_ENC_FUNCTION_ERROR,
    ERROR_DIRECTEXPORT_ENC_DSI_DIFF_ERROR,
    ERROR_DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR,
    ERROR_DIRECTEXPORT_RENDER_INIT_ERROR,
    ERROR_DIRECTEXPORT_WRITER_WRITE_ERROR,
    ERROR_DIRECTEXPORT_WRITER_UNKNOWN_ERROR,
    ERROR_FASTPREVIEW_USER_CANCEL,
    ERROR_FASTPREVIEW_CLIPLIST_ERROR,					// 80
    ERROR_FASTPREVIEW_FIND_CLIP_ERROR,
    ERROR_FASTPREVIEW_FIND_READER_ERROR,
    ERROR_FASTPREVIEW_VIDEO_RENDERER_ERROR,
    ERROR_FASTPREVIEW_DEC_INIT_SURFACE_ERROR,
    ERROR_HW_NOT_ENOUGH_MEMORY,
    ERROR_EXPORT_USER_CANCEL, //yoon
    ERROR_FASTPREVIEW_DEC_INIT_ERROR,
    ERROR_FASTPREVIEW_FILEREADER_INIT_ERROR,
    ERROR_FASTPREVIEW_TIME_ERROR,
    ERROR_FASTPREVIEW_RENDER_INIT_ERROR,				// 90
    ERROR_FASTPREVIEW_OUTPUTSURFACE_INIT_ERROR,
    ERROR_FASTPREVIEW_BUSY,
    
    ERROR_IMAGE_PROCESS	= 0x00001001,
    
    ERROR_SET_TIME_IGNORED = 7001,
    ERROR_SET_TIME_CANCELED,
    ERROR_CAPTURE_FAILED,
    ERROR_SOURCE_FILE_NOT_FOUND,
    ERROR_TRANSCODING_ABORTED,
    ERROR_DESTINATION_FILE_ALREADY_EXISTS,
    ERROR_TEMP_FILE_ALREADY_EXISTS,
    ERROR_NO_INSTANCE_AVAILABLE,
    ERROR_EXPORT_NO_SUCCESS,
    ERROR_PLAY_SUPERCEEDED,
    ERROR_WRAPPER_BUSY,
    ERROR_NOT_READY_TO_PLAY,
    ERROR_SEEKING_LOCKED,
    ERROR_NO_PROJECT_LOADED,
    ERROR_ALREADY_EXPORTING,
    ERROR_EMPTY_PROJECT,
    ERROR_MISSING_RESOURCES,
    ERROR_EXPORT_UNEXPECTED_STOP,
    ERROR_PROJECT_LOAD_FAIL,
    ERROR_RENAME_FAIL,
    ERROR_FAST_PREVIEW_IGNORED,
    ERROR_DUMMY,
    ERROR_CAPTURE_FAIL_ENOSPC,
    ERROR_CAPTURE_FAIL_OTHER,
    ERROR_CAPTURE_SCANNING,
    ERROR_UNRECOGNIZED_CODE,
    ERROR_EDITOR_INSTANCE_DESTROYED,
    ERROR_FILE_MISSING,
    
} _DOC_ENUM(ERRORCODE);