/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Def.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/07/6	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_DEF__H_
#define _NEXVIDEOEDITOR_DEF__H_
//---------------------------------------------------------------------------

// For Test
#define FOR_TEST_IPODLIBRARY

// #define FOR_QRD_TEST

#define FOR_CHECK_MEMSIZE_TEST

#define FOR_JELLYBEAN_TEST

// #define FOR_SURFACEMEDIASOURCE_MTK6589
// #define TEST_MEDIASOURCE_WITH_CC

// #define TEST_MEDIASOURCE_WITHOUT_CC

#define FOR_TEST_PREPARE_VIDEO	// for msm8974 and msm8x26

#define FOR_TEST_MEDIACODEC_DEC

#define FOR_TEST_THEME_CACHE
// comments related to FOR_TEST_FIRST_DECODE_SKIP will be removed.
//#define FOR_TEST_FIRST_DECODE_SKIP

// #define FOR_TEST_MEDIACODEC_ENCODER_DSI

#define FOR_TEST_QUALCOMM_CHIP

// #define FOR_TEST_AVI_FORMAT

#define FOR_LOADLIST_FEATURE
// #define FOR_LAYER_FEATURE
#define FOR_TEST_MP4V_VIDEO

#define FOR_VIDEO_LAYER_FEATURE

#define FOR_DIRECT_EXPORT_WITH_SPEED

#define FOR_IMAGE_LOADING_TASK

// For NxFFReader decrypt
#define	ENC_INFO_SIZE	16
#define	ENC_INFO_CNT	12

// For LGE Project
#ifdef FOR_PROJECT_LGE
#define FOR_TEST_AUDIO_CBR_NO_SEEKTAB
#define FOR_TEST_ENCODER_PROFILE
#define FOR_CHECK_HW_RESOURCE_SIZE
#endif

//////////////////////////////////////
// #define FOR_TEST_AUDIO_DEC_DUMP
// #define FOR_TEST_AUDIO_RESAMPLER_DUMP
// #define FOR_TEST_AUDIO_SPEED_DUMP
// #define FOR_TEST_AUDIO_TRACK_DUMP
// #define FOR_TEST_AUDIO_MIX_DUMP

// #define FOR_TEST_THUMBNAIL_FILE_FAILED


//RYU 20130605
#define FOR_TEST_AUDIOENVELOP

#define FOR_RESAMPLING_FRAMERATE

#define FOR_USE_CUSTOM_CAL_VIDEO_GETPROPERTY

#define TRANSCODING_MAX_SUPPORT_FPS_GAP					(33)
#define TRANSCODING_MAX_SUPPORT_FPS_GAP_INTERVAL		(3333)

#define PREVIEW_BFRAME_CHECK_FPS						(30)

#define FOR_FASTPREVIEW_MAX_BUFFER_COUNT				90

//Support Content size
//---------------------------------------------------------------------------
#define MODEL_NAME_MAX_LENGTH							(255)
#define CONTENT_FILEPATH_MAX_LENGTH						(255)

#ifdef FOR_PROJECT_LGE
#define SUPPORTED_CONTENT_DURATION						(960)
#else
#define SUPPORTED_CONTENT_DURATION						(1000)
#endif

#define CONTENT_1080P_WIDTH								(1920)
#define CONTENT_1080P_HEIGHT							(1080)

#define SUPPORTED_H264_MAX_LEVEL						(52)
#define SUPPORTED_H264_BASELINE_LEVEL					(40)

#define SUPPORTED_VIDEO_MAX_FPS							(240)

#define SUPPORTED_AUDIO_MAX_CHANNEL						(2)
#define SUPPORTED_AUDIO_MAX_SAMPLINGRATE				(48000)

#define SUPPORT_AAC_MAIN								0x00000001
#define SUPPORT_AAC_SSR									0x00000002
#define SUPPORT_AAC_LTP									0x00000004
#define SUPPORT_AAC_SBR									0x00000008
#define SUPPORT_AAC_PS									0x00000010


#define SUPPORTED_CONTENT_WIDTH							(4096)
#define SUPPORTED_CONTENT_HEIGHT						(2176)
#define SUPPORTED_ENCODE_WIDTH							(1920)
#define SUPPORTED_ENCODE_HEIGHT							(1088)
#define SUPPORTED_ENCODE_HEIGHT_1080					(1080)

#ifdef FOR_PROJECT_Gionee
//mjkong_160223 : to support QCIF.
#define SUPPORTED_CONTENT_MIN_WIDTH					(176)
#define SUPPORTED_CONTENT_MIN_HEIGHT					(144)
#else
#ifdef FOR_PROJECT_LGE
#define SUPPORTED_CONTENT_MIN_WIDTH					(320)
#define SUPPORTED_CONTENT_MIN_HEIGHT					(240)
#else
#define SUPPORTED_CONTENT_MIN_WIDTH					(1)
#define SUPPORTED_CONTENT_MIN_HEIGHT					(1)
#endif
#endif

#define CONTENT_720P_WIDTH								(1280)
#define CONTENT_720P_HEIGHT								(720)

#define SUPPORTED_VIEW_WIDTH							(0)
#define SUPPORTED_VIEW_HEIGHT							(0)

//Support Thumbnail info
//---------------------------------------------------------------------------
#define THUMBNAIL_MAX_COUNT								(50)
// #define THUMBNAIL_VIDEO_MODE_VIDEO_WIDTH				(320)
// #define THUMBNAIL_VIDEO_MODE_VIDEO_HEIGHT			(176)

#define THUMBNAIL_VIDEO_MODE_VIDEO_WIDTH				(640)
#define THUMBNAIL_VIDEO_MODE_VIDEO_HEIGHT				(360)

#define THUMBNAIL_VIDEO_MODE_RAW_WIDTH					(320)
#define THUMBNAIL_VIDEO_MODE_RAW_HEIGHT					(240)

#define VIDEO_DEFAULT_THUMBNAIL_WIDTH					(120)
#define VIDEO_DEFAULT_THUMBNAIL_HEIGHT					(90)
#define VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16				(16)
#define VIDEO_DEFAULT_THUMBNAIL_BITCOUNT32				(32)


#define eNEX_CODEC_V_H264_S								0x40010300
#define eNEX_CODEC_V_H264_MC_S							0x50010300
#define eNEX_CODEC_V_MPEG4V_S							0x40020100


// RYU 20130620
#define eNEX_CODEC_A_AAC_S								0x40020000

//Audio info
//---------------------------------------------------------------------------
#define EDITOR_DEFAULT_SAMPLERATE						44100
#define EDITOR_DEFAULT_AUDIO_BITRATE					128*1024
#define EDITOR_DEFAULT_CHANNELS							2
#define EDITOR_DEFAULT_SAMPLEFORCHANNEL					1024
#define EDITOR_DEFAULT_BITFORSAMPLE						16
#define EDITOR_DEFAULT_AUDIOSAMPLESIZE					2048
#define EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE				1024
//Video encoder info
//---------------------------------------------------------------------------
#define EDITOR_DEFAULT_FRAME_TIME						10000000
#define EDITOR_DEFAULT_FRAME_RATE						3000
#define EDITOR_DEFAULT_CODEC_TYPE						(0x10010300)

//Hardware Resource info
//---------------------------------------------------------------------------
#define SUPPORTED_HARDWARE_MAXWIDTH						(1920 )
#define SUPPORTED_HARDWARE_MAXHEIGHT					(1088 )

#define SUPPORTED_DECODE_MAXCOUNT						(4)
#define SUPPORTED_ENCODE_MAXCOUNT						(1)

#define SUPPORTED_1080PDECODE_EXPORTING					(0)

#define THEME_RENDERER_DEFAULT_EFFECT					("none")
#define THEME_RENDERER_DEFAULT_TITLE_EFFECT				("")

#define GET_CLIPINFO_INCLUDE_SEEKTABLE					(0x00000001)
#define GET_CLIPINFO_CHECK_AUDIO_DEC					(0x00000010)
#define GET_CLIPINFO_CHECK_VIDEO_DEC					(0x00000100)
#define GET_CLIPINFO_CHECK_IDR_MODE						(0x00001000)

#define GET_THUMBRAW_RGB								(0x00000000)
#define GET_THUMBRAW_YUV								(0x00010000)
#define GET_THUMBRAW_Y_ONLY								(0x00020000)
#define GET_THUMBRAW_RGB_CALLBACK						(0x00040000)
#define GET_THUMBRAW_FIRST_FRAME							(0x00080000)
#define GET_THUMBRAW_LAST_FRAME							(0x00100000)

#define GET_THUMBNAIL_FLAG_IFRAME_WITHOUT_SKIP			(0x00000001)
#define GET_THUMBNAIL_FLAG_IFRAME_WHILE_PFRAME_MODE		(0x00000002)
#define GET_THUMBNAIL_FLAG_PFRAME						(0x00000004)

#define GET_FLAG_ROTATE_0_MODE							(0x00000000)
#define GET_FLAG_ROTATE_90_MODE							(0x00000010)
#define GET_FLAG_ROTATE_180_MODE						(0x00000020)
#define GET_FLAG_ROTATE_270_MODE						(0x00000040)

#define GET_THUMBNAIL_INTERVAL_MASK						(0x0000FF00)




#define HQ_SCALE_FLAG									(0x00100000)
#define ENCODE2TRANSCODE                                (0x001000) //yoon

#define WRITER_RAW_FRAME_TEMP_SIZE						(1024*1024*3)

#define DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL				(0x00000001)

//yoon
#define RENDERMODE_360VIDEO	(0x1)
#define HDR_TYPE_PQ (16)
#define HDR_TYPE_HLG (18)

//For NESA-843, NESA-1159, CHC-177
#define	DISABLE_MIXED_RAW_AND_ANNEXB_NAL

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_DEF__H_
