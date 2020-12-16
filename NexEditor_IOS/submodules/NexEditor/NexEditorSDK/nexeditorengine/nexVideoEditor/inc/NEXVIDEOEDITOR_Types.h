/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Types.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_TYPES__H_
#define _NEXVIDEOEDITOR_TYPES__H_
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
typedef enum _PLAY_STATE
{
	PLAY_STATE_NONE = 0,
	PLAY_STATE_IDLE,
	PLAY_STATE_RUN,
	PLAY_STATE_RECORD,
	PLAY_STATE_PAUSE,	
	PLAY_STATE_RESUME,		
	PLAY_STATE_VIDEO_ANALYZE
}PLAY_STATE;

//---------------------------------------------------------------------------
typedef enum _CLIP_TYPE
{
	CLIPTYPE_NONE = 0,
	CLIPTYPE_IMAGE,
	CLIPTYPE_TEXT,
	CLIPTYPE_AUDIO,
	CLIPTYPE_VIDEO,
	CLIPTYPE_EFFECT,
	CLIPTYPE_LAYER,
	CLIPTYPE_VIDEO_LAYER,
}CLIP_TYPE;

//---------------------------------------------------------------------------
typedef enum _EFFECT_TYPE
{
	EFFECT_TYPE_UNKNOWN = 0,
	EFFECT_TYPE_TRANSITION,
	EFFECT_TYPE_TITLE,
}EFFECT_TYPE;

//---------------------------------------------------------------------------
typedef enum _TITLE_STYLE
{
	TITLE_STYLE_NONE = 0,
	TITLE_STYLE_HEADLINE,
	TITLE_STYLE_OPENNING,
	TITLE_STYLE_PLAYING,
	TITLE_STYLE_ENDING
}TITLE_STYLE;

//---------------------------------------------------------------------------
typedef enum _TRANSITION_TYPE
{
	TRANSITION_CONTOUR = 0,
	TRANSITION_DIAGONAL,
	TRANSITION_CROSSFADE,
	TRANSITION_FADETOFROMBLACK,
	TRANSITION_SLIDINGRIGHTOUTLEFTIN,
	TRANSITION_SLIDINGLEFTOUTRIGHTIN,
	TRANSITION_SLIDINGTOPOUTBOTTOMIN,
	TRANSITION_SLIDINGBOTTOMOUTTOPIN
}TRANSITION_TYPE;

//---------------------------------------------------------------------------
typedef enum _MODEL_TYPE
{
	MODEL_TYPE_QUALCOMM_8X60	= 0,
	MODEL_TYPE_QUALCOMM_82XX,
	MODEL_TYPE_NVIDIA_TEGRA,
}MODEL_TYPE;

//---------------------------------------------------------------------------
typedef enum _IMAGE_TYPE
{
	NOT_IMAGE = 0,
	IMAGE_RGB,
	IMAGE_JPEG,
	IMAGE_JPEG_ANDROID,
	IMAGE_PNG,
	IMAGE_SVG,
	IMAGE_WEBP,
	IMAGE_BMP,
	IMAGE_GIF,
    //
    IMAGE_JPEG_IOS,
	IMAGE_HEIF,
}IMAGE_TYPE;


//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_TYPES__H_
