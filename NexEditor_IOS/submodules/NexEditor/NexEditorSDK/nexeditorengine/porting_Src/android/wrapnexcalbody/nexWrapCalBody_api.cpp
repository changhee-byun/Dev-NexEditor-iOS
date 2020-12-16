/******************************************************************************
* File Name   :	nexWrapCalBody_api.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2013/10/15	Draft.
-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "NexCAL.h"
#include "NexMediaDef.h"
#include "nexWrapCalBody_api.h"
#ifdef H264_CALBODY	
#include "nexCalBody_H264.h"
#endif

#ifdef MP4_CALBODY	
#include "nexCalBody_DIVX.h"
#endif

#ifdef PCM_CALBODY	
#include "nexCalBody_PCM.h"
#endif

#ifdef AAC_CALBODY	
#include "nexCalBody_AAC.h"
#endif

#ifdef AACEnc_CALBODY	
#include "nexCalBody_AACEnc.h"
#endif

#ifdef FLAC_CALBODY	
#include "nexCalBody_FLAC.h"
#endif

NEXWRAPCALBody g_nexWrapCalBody;

NEXWRAPCALBody *getNexCAL_Wrap_Body(
	  unsigned int uCodecObjectTypeIndication
	, unsigned int API_Version
	, unsigned int nexCAL_API_MAJOR
	, unsigned int nexCAL_API_MINOR
	, unsigned int nexCAL_API_PATCH_NUM
	)
{
	if( API_Version != NEXWRAP_CALBODY_API_VERSION )
		return 0;

	if( nexCAL_API_MAJOR != NEXCAL_VERSION_MAJOR ||nexCAL_API_MINOR != NEXCAL_VERSION_MINOR )
	{
		return 0;
	}

	memset(&g_nexWrapCalBody, 0x00, sizeof(NEXWRAPCALBody));
	switch(uCodecObjectTypeIndication)
	{
#ifdef MP4_CALBODY		
		case eNEX_CODEC_V_MPEG4V:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Video_DIVX_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Video_DIVX_Close;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Video_DIVX_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Video_DIVX_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)nexCALBody_Video_DIVX_GetOutput;
			g_nexWrapCalBody.func[9] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Video_DIVX_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Video_DIVX_Dec;
			break;
#endif
#ifdef H264_CALBODY		
		case eNEX_CODEC_V_H264:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Video_AVC_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Video_AVC_Close;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Video_AVC_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Video_AVC_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)nexCALBody_Video_AVC_GetOutput;
			g_nexWrapCalBody.func[9] = (void*)0x4f6c54e4;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Video_AVC_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Video_AVC_Dec;
			break;
#endif
#ifdef AACEnc_CALBODY
		case eNEX_CODEC_A_AAC:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Audio_AACEnc_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Audio_AACEnc_Deinit;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Audio_AACEnc_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Audio_AACEnc_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)0x4f7be150;
			g_nexWrapCalBody.func[9] = (void*)0x4f6c54e4;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Audio_AACEnc_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Audio_AACEnc_Encode;
			break;
#endif			
#ifdef AAC_CALBODY
		case eNEX_CODEC_A_AAC:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Audio_AAC_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Audio_AAC_Deinit;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Audio_AAC_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Audio_AAC_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)0x4f7be150;
			g_nexWrapCalBody.func[9] = (void*)0x4f6c54e4;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Audio_AAC_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Audio_AAC_Dec;
			break;
#endif			
#ifdef PCM_CALBODY	
		case eNEX_CODEC_A_PCM_S16LE:
		case eNEX_CODEC_A_PCM_S16BE:		
		case eNEX_CODEC_A_PCM_RAW:
		case eNEX_CODEC_A_PCM_FL32LE:
		case eNEX_CODEC_A_PCM_FL32BE:
		case eNEX_CODEC_A_PCM_FL64LE:
		case eNEX_CODEC_A_PCM_FL64BE:
		case eNEX_CODEC_A_PCM_IN24LE:
		case eNEX_CODEC_A_PCM_IN24BE:
		case eNEX_CODEC_A_PCM_IN32LE:
		case eNEX_CODEC_A_PCM_IN32BE:			
		case eNEX_CODEC_A_PCM_LPCMLE:
		case eNEX_CODEC_A_PCM_LPCMBE:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Audio_PCM_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Audio_PCM_Deinit;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Audio_PCM_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Audio_PCM_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)0x4f7be150;
			g_nexWrapCalBody.func[9] = (void*)nexCALBody_Audio_PCM_GetInfo;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Audio_PCM_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Audio_PCM_Dec;
			break;
#endif			
#ifdef FLAC_CALBODY	
		case eNEX_CODEC_A_FLAC:
			g_nexWrapCalBody.func[0] = (void*)0x4f43e1f1;
			g_nexWrapCalBody.func[1] = (void*)0x4f3a83e0;
			g_nexWrapCalBody.func[2] = (void*)nexCALBody_Audio_FLAC_Init;
			g_nexWrapCalBody.func[3] = (void*)nexCALBody_Audio_FLAC_Deinit;
			g_nexWrapCalBody.func[4] = (void*)0x4f5a3e1f;
			g_nexWrapCalBody.func[5] = (void*)nexCALBody_Audio_FLAC_Reset;
			g_nexWrapCalBody.func[6] = (void*)nexCALBody_Audio_FLAC_SetProperty;
			g_nexWrapCalBody.func[7] = (void*)0x4f6b4f50;
			g_nexWrapCalBody.func[8] = (void*)0x4f7be150;
			g_nexWrapCalBody.func[9] = (void*)nexCALBody_Audio_FLAC_GetInfo;
			g_nexWrapCalBody.func[10] = (void*)0x4f6d6f8;
			g_nexWrapCalBody.func[11] = (void*)nexCALBody_Audio_FLAC_GetProperty;
			g_nexWrapCalBody.func[12] = (void*)nexCALBody_Audio_FLAC_Dec;
			break;
#endif			
		default:
			return 0;
	}
	return &g_nexWrapCalBody;
}
