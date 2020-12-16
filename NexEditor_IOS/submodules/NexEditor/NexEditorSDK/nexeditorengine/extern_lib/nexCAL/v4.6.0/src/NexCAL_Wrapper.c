/*-----------------------------------------------------------------------------
	File Name   :	nexCAL_Wrapper.c
	Description :	See the following code. ^^
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2007 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#include "NexCAL.h"
#include "only4CAL.h"
#include "NexCodecEntry.h"
#include "NexCodecUtil.h"
#include "NexCAL_Wrapper.h"
#include "NexUtil.h"

/*-----------------------------------------------------------------------------
		Video Decoder Wrapper
-----------------------------------------------------------------------------*/

//#define _DECODE_INPUT_AUDIO_DUMP_ //dump frame from input decoder
//#define _DECODE_OUTPUT_AUDIO_DUMP_ //dump pcm from output decoder
//#define _DECODE_INPUT_VIDEO_DUMP_ //dump frame from input decoder

#ifdef _DECODE_INPUT_AUDIO_DUMP_
static const NXUINT8 *s_strDecodeInputAudioPath = "c:\\temp\\decode_input_audio.dump";
#endif

#ifdef _DECODE_OUTPUT_AUDIO_DUMP_
static const NXUINT8 *s_strDecodeOutputAudioPath = "c:\\temp\\decode_output_audio.dump";
#endif

#ifdef _DECODE_INPUT_VIDEO_DUMP_
static const NXUINT8 *s_strDecodeInputVideoPath = "c:\\temp\\decode_input_video.dump";
#endif

#define NEXCAL_I_FRAME_DECODED	0x01
#define NEXCAL_P_FRAME_DECODED	0x02
#define NEXCAL_IP_FRAME_DECODED	0x03
#define NEXCAL_B_FRAME_DECODED	0x04

#define IS_BFRAME_SKIP_TYPE(codec) (((codec)==eNEX_CODEC_V_MPEG1V) || ((codec)==eNEX_CODEC_V_MPEG2V) || ((codec)==eNEX_CODEC_V_MPEG4V) \
									|| ((codec)==eNEX_CODEC_V_DIVX) || ((codec)==eNEX_CODEC_V_WVC1))

static NXINT32 _ConvertH26xProfileEnum2Property(NEX_H26x_PROFILE a_eProfile)
{
	NXUINT32 nProfile = 0;

	switch (a_eProfile)
	{
		//HEVC
		case eNEX_HEVC_PROFILE_MAIN:
			nProfile = NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL;
			break;
		case eNEX_HEVC_PROFILE_MAIN10:
			nProfile = NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL;
			break;
		case eNEX_HEVC_PROFILE_MAINSTILLPICTURE:
			nProfile = NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL;
			break;

			//AVC
		case eNEX_AVC_PROFILE_BASELINE:
			nProfile = NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_MAIN:
			nProfile = NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_EXTENDED:
			nProfile = NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_HIGH:
			nProfile = NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_HIGH10:
			nProfile = NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_HIGH422:
			nProfile = NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL;
			break;
		case eNEX_AVC_PROFILE_HIGH444:
			nProfile = NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL;
			break;

		default:
			nProfile = 0;
			break;
	}

	return nProfile;    
}

static NEX_H26x_PROFILE _ConvertH26xProperty2ProfileEnum(NXINT32 a_nProfile)
{
	NEX_H26x_PROFILE eProfile = eNEX_H26x_PROFILE_UNKNOWN;

	switch (a_nProfile)
	{
		//HEVC
		case NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL:
			eProfile = eNEX_HEVC_PROFILE_MAIN;
			break;
		case NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL:
			eProfile = eNEX_HEVC_PROFILE_MAIN10;
			break;
		case NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL:
			eProfile = eNEX_HEVC_PROFILE_MAINSTILLPICTURE;
			break;

			//AVC
		case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_BASELINE;
			break;
		case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_MAIN;
			break;
		case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_EXTENDED;
			break;
		case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_HIGH;
			break;
		case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_HIGH10;
			break;
		case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_HIGH422;
			break;
		case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
			eProfile = eNEX_AVC_PROFILE_HIGH444;
			break;

		default:
			eProfile = eNEX_H26x_PROFILE_UNKNOWN;
			break;
	}

	return eProfile;    
}

NEXCAL_API NXINT32 nexCAL_SetClientIdentifier(NEXCALCodecHandle a_hCodec, NXVOID *a_pClientIdentifier)
{
	if (NULL == a_hCodec)
	{
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	a_hCodec->m_pUserData = a_pClientIdentifier;

	return NEXCAL_ERROR_NONE;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{
		if (NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE == a_uProperty && a_hCodec->m_uOutBufferFormat > NEXCAL_PROPERTY_VIDEO_BUFFER_UNKNOWN)
		{
			*a_pqValue = (NXINT64)a_hCodec->m_uOutBufferFormat;
			return NEXCAL_ERROR_NONE;
		}
		else if (NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE == a_uProperty && a_hCodec->m_uAVCMaxProfile > eNEX_H26x_PROFILE_UNKNOWN)
		{
			*a_pqValue = (NXINT64)a_hCodec->m_uAVCMaxProfile;
			return NEXCAL_ERROR_NONE;
		}
		else if (NEXCAL_PROPERTY_GET_DECODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		*a_pqValue = a_hCodec->m_eCodecType;//simple calbody add

		return a_hCodec->m_cfs.vd.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.vd.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderInit(NEXCALCodecHandle a_hCodec, 
										   NEX_CODEC_TYPE a_eCodecType, 
										   NXUINT8 *a_pConfig, 
										   NXINT32 a_nConfigLen, 
										   NXUINT8 *a_pFrame, 
										   NXINT32 a_nFrameLen, 
										   NXVOID *a_pInitInfo,
										   NXVOID *a_pExtraInfo,
										   NXINT32 a_nNALHeaderSize, 
										   NXINT32 *a_pnWidth, 
										   NXINT32 *a_pnHeight, 
										   NXINT32 *a_pnPitch, 
										   NXUINT32 a_uMode, 
										   NXUINT32 a_uUserDataType, 
										   NXVOID *a_pClientIdentifier)
{
	NXINT32 nNewConfigLen = 0;
	NXINT32 nRet = 0;
	NXINT64 qFormat = 0;
	NXUINT8* pNewConfig = (NXUINT8*)0;
	NEXSALFileHandle hFile;
	NXINT64 qConfigRecorder;
	NXINT32 bAlreadyLimitChecked = FALSE;

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_FAIL;
	}

	if (NULL != (hFile = ((NEXCALRegistrationInfo*)(a_hCodec->m_hCAL))->m_ahDump[NEXCAL_MEDIATYPE_VIDEO]))
	{
		if (a_pConfig && a_nConfigLen)
		{
			nexSAL_FileWrite(hFile, &a_nConfigLen, sizeof(a_nConfigLen));
			nexSAL_FileWrite(hFile, a_pConfig, a_nConfigLen);
		}
		else
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] a_pConfiga is NULL\n", _FUNLINE_);
		}
	}

	a_hCodec->m_pDSI = (NXVOID*)nexSAL_MemAlloc(a_nConfigLen);
	if (a_hCodec->m_pDSI)
	{
		memcpy(a_hCodec->m_pDSI, a_pConfig, a_nConfigLen);
		a_hCodec->m_uDSILen = a_nConfigLen;
	}

	if (a_eCodecType == eNEX_CODEC_V_H264)
	{
		NXINT32 nSPSRet = 0;
		NXINT64 qLevel;
		NEXCODECUTIL_SPS_INFO SPS_Info;

		a_hCodec->m_uNALLenSize = a_nNALHeaderSize;
		a_hCodec->m_uByteFormat4Frame = NexCodecUtil_CheckByteFormat(a_pFrame, a_nFrameLen);
		a_hCodec->m_uByteFormat4DSI = NexCodecUtil_CheckByteFormat(a_pConfig, a_nConfigLen);
		a_hCodec->m_bByteFormatConverted4Frame = FALSE;

		//[shoh][2011.07.21] If a_pConfiga is NULL, a_pConfiga should be made from pConfigEnhance before pConfigEnhance is trimmed.
		if ((0 == a_nConfigLen) && (0 < a_nFrameLen))
		{
			NXINT32 n = NexCodecUtil_AVC_SPSExists(a_pFrame, a_nFrameLen, a_hCodec->m_uByteFormat4Frame, a_nNALHeaderSize);
			if (0 < n)
			{
				a_pConfig = a_pFrame + n;
				a_nConfigLen = a_nFrameLen - n;
				a_hCodec->m_uByteFormat4DSI =  a_hCodec->m_uByteFormat4Frame;
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS/PPS not exist. Corrupt file\n", _FUNLINE_);
				return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
			}
		}

		nSPSRet = NexCodecUtil_AVC_GetSPSInfo((NXCHAR *)a_pConfig, a_nConfigLen, &SPS_Info, a_hCodec->m_uByteFormat4DSI);

		//[kwangsik.lee] On the GingerBread, Max Profile & Level of HW Decoder is incorrect.  Limit Check is already done On GetCodec.
		if (NEXCAL_CHECK_STATIC_PROPERTY(a_hCodec->m_uProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC) && a_hCodec->m_fnQueryHandler)
		{
			bAlreadyLimitChecked = TRUE;
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] AVC Decoder Type(%s)\n", _FUNLINE_, bAlreadyLimitChecked?"HW Decoder":"SW Decoder");

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_ANSWERIS_NO)
			{
				if (!nSPSRet)
				{
					*a_pnWidth = SPS_Info.usWidth;
					*a_pnHeight = SPS_Info.usHeight;
					*a_pnPitch = SPS_Info.usWidth;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_AVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE, &qFormat) == NEXCAL_ERROR_NONE)
		{
			NEX_H26x_PROFILE eProfile = _ConvertH26xProperty2ProfileEnum((NXINT32)qFormat);

			if (eNEX_H26x_PROFILE_UNKNOWN != eProfile)
			{
				if (!nSPSRet)
				{
					if (bAlreadyLimitChecked == FALSE && (NXINT64)SPS_Info.eProfile > eProfile)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AVC Profile[support=%d,content=%d]\n", _FUNLINE_, eProfile, SPS_Info.eProfile);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
				else
				{
					/*
					**NexCodecUtil_AVC_GetSPSInfo()에서 Return이 0이 아닌 경우 에러처리하지 말고 통과시켜야 한다. (Bitstream이 손상된 경우 고려)
					*/
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_AVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
					//return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
				}
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Skipped the available check of AVC profile.\n", _FUNLINE_);
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, _ConvertH26xProfileEnum2Property(SPS_Info.eProfile), &qLevel) == NEXCAL_ERROR_NONE)
			nexCAL_VideoEncoderSetProperty(a_hCodec, NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL, 11);
		{
			if (0 == qLevel)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 0, "[%s %d] Skipped the available check of AVC level.\n", _FUNLINE_);   
			}
			else
			{
				if (!nSPSRet)
				{
					if (SPS_Info.uLevel > NEXCAL_PROPERTY_AVC_MAX_LEVEL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS uLevel(%d) is invalid\n", _FUNLINE_, SPS_Info.uLevel);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
					else if (bAlreadyLimitChecked == FALSE && SPS_Info.uLevel > (NXUINT32)qLevel)
					{
						NXUINT32 uMBLevel = NexCodecUtil_AVC_TotalMacroBlocks_To_Level_min30fps(SPS_Info.uWidthMBS*SPS_Info.uHeightMBS);
						if (uMBLevel > (NXUINT32)qLevel)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AVC level(%d) level in MBS(%D)is higher than the supportable level of codec(%ll)\n", _FUNLINE_, SPS_Info.uLevel, uMBLevel, qLevel);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
					}
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_AVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER, &qConfigRecorder) == NEXCAL_ERROR_NONE)
		{
			if (qConfigRecorder == NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_NOT_OK)
			{
				const NXINT32 nConfigRecorderLen = 5; // [shoh][2012.04.06] Configuration recorder length is 5 bytes and master 3bits of the 6th byte should be 0.

				if ((NexCodecUtil_AVC_IsConfigFromRecordType(a_pConfig, a_nConfigLen) == TRUE) && (a_nConfigLen > nConfigRecorderLen))
				{
					a_pConfig += nConfigRecorderLen;
					a_nConfigLen -= nConfigRecorderLen;
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_INITPS_FORMAT, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
			{
				if ((a_nConfigLen > 0) && (qFormat != a_hCodec->m_uByteFormat4DSI))
				{
					pNewConfig = (NXUINT8 *)nexSAL_MemAlloc(a_nConfigLen*3/2);
					if (pNewConfig)
					{
						nNewConfigLen = NexCodecUtil_AVC_ConvertFormatPSs(pNewConfig, a_nConfigLen*3/2, a_pConfig, a_nConfigLen, a_hCodec->m_uByteFormat4DSI);
						if (nNewConfigLen == 0)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_AVC_ConvertFormatPSs return Zero\n", _FUNLINE_);
							SAFE_FREE(pNewConfig);
							pNewConfig = (NXUINT8 *)0;
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
						a_pConfig = pNewConfig;
						a_nConfigLen = nNewConfigLen;
						a_hCodec->m_uByteFormat4DSI = (NXUINT32)qFormat;
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
						return NEXCAL_ERROR_FAIL;
					}
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_BYTESTREAM_FORMAT, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
			{
				if ((a_nFrameLen > 0) && (qFormat != a_hCodec->m_uByteFormat4Frame))
				{
					NXINT32 nNewDataLen = 0;

					if ((NXINT32)a_hCodec->m_uIntermediateFrameSize < a_nFrameLen)
					{
						if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
						{
							SAFE_FREE(a_hCodec->m_pIntermediateFrame);
							a_hCodec->m_pIntermediateFrame = (NXVOID*)0;
							a_hCodec->m_uIntermediateFrameSize = 0;
						}

						a_hCodec->m_pIntermediateFrame = (NXUINT8*)nexSAL_MemAlloc(a_nFrameLen + 100);
						if (!a_hCodec->m_pIntermediateFrame)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
							return NEXCAL_ERROR_FAIL;
						}
						a_hCodec->m_uIntermediateFrameSize = a_nFrameLen;
					}

					nNewDataLen = NexCodecUtil_ConvertFormat(a_hCodec->m_pIntermediateFrame, a_nFrameLen+100, a_pFrame, a_nFrameLen, a_hCodec->m_uByteFormat4Frame, a_nNALHeaderSize);
					if (nNewDataLen == NEXCAL_ERROR_FAIL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_CheckByteFormat failed\n", _FUNLINE_);
						a_pFrame = NULL;
					}
					else
					{
						a_pFrame = a_hCodec->m_pIntermediateFrame;
						a_nFrameLen = nNewDataLen;
						a_hCodec->m_uByteFormat4Frame = (NXUINT32)qFormat;
						a_hCodec->m_bByteFormatConverted4Frame = TRUE;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 0, "[%s %d] NexCodecUtil_CheckByteFormat a_nEnhLen = %d, nNewDataLen = %d\n", _FUNLINE_, a_nFrameLen, nNewDataLen);
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 0, "[%s %d] NexCodecUtil_CheckByteFormat a_pFrame = 0x%p, m_pIntermediateFrame = 0x%p, m_uNALLenSize = %d\n", _FUNLINE_, a_pFrame, a_hCodec->m_pIntermediateFrame, a_hCodec->m_uNALLenSize);
					}
				}
			}
		}

		//K.Lee add 20090107 add [ I or P frame 앞에 PS가 존재하는 경우 codec의 지원여부 확인
		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM, &a_hCodec->m_qIncludeConfig) == NEXCAL_ERROR_NONE)
		{
			//지원하지 않으면 discard 시킨다.
			if (a_hCodec->m_qIncludeConfig == NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK)
			{
				NXUINT32 uNewFrameSize = 0;
				NXUINT32 uConfigStreamOffset = 0;
				NXUINT8 *pConfigStream = NULL;

				if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)
				{
					pConfigStream = NexCodecUtil_AVC_ANNEXB_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32*)&uNewFrameSize);
				}
				else
				{
					pConfigStream = NexCodecUtil_AVC_NAL_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32)a_hCodec->m_uNALLenSize, (NXINT32*)&uNewFrameSize);
				}

				if (pConfigStream != NULL)
				{
					uConfigStreamOffset = (NXUINT32)(pConfigStream - a_pFrame);
					a_pFrame = pConfigStream + uNewFrameSize;
					a_nFrameLen -= uConfigStreamOffset + uNewFrameSize;
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 1, "[%s %d] %s bitstream has SPSPPS. => => Discard SPSPPS\n", _FUNLINE_,
									(NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)?(NXCHAR*)"ANNEX-B":(NXCHAR*)"NAL");
				}
			}
		}
		//K.Lee ]

		nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME, &a_hCodec->m_qBFrameSkip);
	}
	else if (a_eCodecType == eNEX_CODEC_V_HEVC)
	{
		NXINT32 nSPSRet = 0;
		NXINT64 qLevel = 0;
		NXBOOL  bValidConfig = FALSE;
		NEXCODECUTIL_SPS_INFO SPS_Info;

		a_hCodec->m_uNALLenSize = a_nNALHeaderSize;
		a_hCodec->m_uByteFormat4Frame = NexCodecUtil_CheckByteFormat(a_pFrame, a_nFrameLen);
		a_hCodec->m_uByteFormat4DSI = NexCodecUtil_CheckByteFormat(a_pConfig, a_nConfigLen);
		a_hCodec->m_bByteFormatConverted4Frame = FALSE;

		if (a_nConfigLen >0)
		{
			NXINT32 n = NexCodecUtil_HEVC_SPSExists(a_pConfig, a_nConfigLen, a_hCodec->m_uByteFormat4Frame, a_nNALHeaderSize);

			if (0 < n)
			{
				bValidConfig = TRUE;
			}
			else
			{
				bValidConfig = FALSE;
			}
		}

		if ((0 == a_nConfigLen || FALSE == bValidConfig) && (0 < a_nFrameLen))
		{
			NXUINT8 *pConfigStream = NULL;
			NXUINT32 uNewFrameSize = 0;

			if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)
			{
				pConfigStream = NexCodecUtil_HEVC_ANNEXB_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32*)&uNewFrameSize);
			}
			else
			{
				pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32)a_hCodec->m_uNALLenSize, (NXINT32*)&uNewFrameSize);
			}

			if (pConfigStream != NULL)
			{
				a_pConfig = pConfigStream;
				a_nConfigLen = uNewFrameSize;
				a_hCodec->m_uByteFormat4DSI = a_hCodec->m_uByteFormat4Frame;
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] VPS/SPS/PPS not exist. Corrupt file\n", _FUNLINE_);
				return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
			}
		}

		memset(&SPS_Info, 0x00, sizeof(SPS_Info));
		nSPSRet = NexCodecUtil_HEVC_GetSPSInfo((NXCHAR *)a_pConfig, a_nConfigLen, &SPS_Info, a_hCodec->m_uByteFormat4DSI);

		//[kwangsik.lee] On the GingerBread, Max Profile & Level of HW Decoder is incorrect.  Limit Check is already done On GetCodec.
		if (NEXCAL_CHECK_STATIC_PROPERTY(a_hCodec->m_uProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC)&& a_hCodec->m_fnQueryHandler)
		{
			bAlreadyLimitChecked = TRUE;
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] HEVC Decoder Type(%s)\n", _FUNLINE_, bAlreadyLimitChecked?"HW Decoder":"SW Decoder");

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_ANSWERIS_NO)
			{
				if (!nSPSRet)
				{
					*a_pnWidth = SPS_Info.usWidth;
					*a_pnHeight = SPS_Info.usHeight;
					*a_pnPitch = SPS_Info.usWidth;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_HEVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE, &qFormat) == NEXCAL_ERROR_NONE)
		{
			NEX_H26x_PROFILE eProfile = _ConvertH26xProperty2ProfileEnum((NXINT32)qFormat);

			if (eNEX_H26x_PROFILE_UNKNOWN != eProfile)
			{
				if (!nSPSRet)
				{
					if (bAlreadyLimitChecked == FALSE && (NXINT64)SPS_Info.eProfile > eProfile)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] HEVC Profile[support=%d, content=%d]\n", _FUNLINE_, eProfile, SPS_Info.eProfile);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
				else
				{
					/*
					**NexCodecUtil_AVC_GetSPSInfo()에서 Return이 0이 아닌 경우 에러처리하지 말고 통과시켜야 한다. (Bitstream이 손상된 경우 고려)
					*/
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_HEVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
					//return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
				}
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Skipped the available check of HEVC profile.\n", _FUNLINE_);
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, _ConvertH26xProfileEnum2Property(SPS_Info.eProfile), &qLevel) == NEXCAL_ERROR_NONE)
		{
			if (0 == qLevel)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 0, "[%s %d] Skipped the available check of HEVC level.\n", _FUNLINE_);   
			}
			else
			{
				if (!nSPSRet)
				{
					if (SPS_Info.uLevel > NEXCAL_PROPERTY_HEVC_MAX_LEVEL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS uLevel(%d) is invalid\n", _FUNLINE_, SPS_Info.uLevel);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
					else if (bAlreadyLimitChecked == FALSE && SPS_Info.uLevel > (NXUINT32)qLevel)
					{
						/*
						NXUINT32 uMBLevel = NexCodecUtil_HEVC_TotalMacroBlocks_To_Level_min30fps(SPS_Info.uWidthMBS*SPS_Info.uHeightMBS);
						if (uMBLevel > (NXUINT32)qLevel)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] H.265 level(%d) level in MBS(%D)is higher than the supportable level of codec(%ll)\n", _FUNLINE_, SPS_Info.uLevel, uMBLevel, qLevel);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
						*/
					}
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_HEVC_GetSPSInfo Error ret[%d]\n", _FUNLINE_, nSPSRet);
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM, &a_hCodec->m_qIncludeConfig) == NEXCAL_ERROR_NONE)
		{
			NXUINT32 uNewFrameSize = 0;
			NXUINT32 uConfigStreamOffset = 0;
			NXUINT8 *pConfigStream = NULL;

			if (a_hCodec->m_qIncludeConfig == NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK)
			{
				if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)
				{
					pConfigStream = NexCodecUtil_HEVC_ANNEXB_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32*)&uNewFrameSize);
				}
				else
				{
					pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32)a_hCodec->m_uNALLenSize, (NXINT32*)&uNewFrameSize);
				}

				if (pConfigStream != NULL)
				{
					uConfigStreamOffset = (NXUINT32)(pConfigStream - a_pFrame);
					a_pFrame = pConfigStream + uNewFrameSize;
					a_nFrameLen -= uConfigStreamOffset + uNewFrameSize;
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 1, "[%s %d] %s bitstream has SPSPPS. => => Discard SPSPPS\n", _FUNLINE_,
									(NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)?(NXCHAR*)"ANNEX-B":(NXCHAR*)"NAL");
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS/PPS not exist. Corrupt file \n", _FUNLINE_);
					return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_INITPS_FORMAT, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
			{
				if ((a_nConfigLen > 0) && (qFormat != a_hCodec->m_uByteFormat4DSI))
				{
					pNewConfig = (NXUINT8 *)nexSAL_MemAlloc(a_nConfigLen*3/2);
					if (pNewConfig)
					{
						nNewConfigLen = NexCodecUtil_HEVC_ConvertFormatPSs(pNewConfig, a_nConfigLen*3/2, a_pConfig, a_nConfigLen, a_hCodec->m_uByteFormat4DSI);
						if (nNewConfigLen == 0)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_HEVC_ConvertFormatPSs return Zero\n", _FUNLINE_);
							SAFE_FREE(pNewConfig);
							pNewConfig = (NXUINT8 *)0;
						}
						else
						{
							a_pConfig = pNewConfig;
							a_nConfigLen = nNewConfigLen;
							a_hCodec->m_uByteFormat4DSI = (NXUINT32)qFormat;
						}
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
						return NEXCAL_ERROR_FAIL;
					}
				}
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_BYTESTREAM_FORMAT, &qFormat) == NEXCAL_ERROR_NONE)
		{
			if (qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || qFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
			{
				if ((a_nFrameLen > 0) && (qFormat != a_hCodec->m_uByteFormat4Frame))
				{
					NXINT32 nNewDataLen = 0;

					if ((NXINT32)a_hCodec->m_uIntermediateFrameSize < a_nFrameLen)
					{
						if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
						{
							SAFE_FREE(a_hCodec->m_pIntermediateFrame);
							a_hCodec->m_pIntermediateFrame = (NXVOID*)0;
							a_hCodec->m_uIntermediateFrameSize = 0;
						}

						a_hCodec->m_pIntermediateFrame = (NXUINT8 *)nexSAL_MemAlloc(a_nFrameLen + 100);
						if (!a_hCodec->m_pIntermediateFrame)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
							return NEXCAL_ERROR_FAIL;
						}
						a_hCodec->m_uIntermediateFrameSize = a_nFrameLen;
					}

					nNewDataLen = NexCodecUtil_ConvertFormat(a_hCodec->m_pIntermediateFrame, a_nFrameLen + 100, a_pFrame, a_nFrameLen, a_hCodec->m_uByteFormat4Frame, a_nNALHeaderSize);
					if (nNewDataLen == NEXCAL_ERROR_FAIL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_CheckByteFormat failed\n", _FUNLINE_);
						a_pFrame = NULL;
					}
					else
					{
						a_pFrame = a_hCodec->m_pIntermediateFrame;
						a_nFrameLen = nNewDataLen;
						a_hCodec->m_uByteFormat4Frame = (NXUINT32)qFormat;
						a_hCodec->m_bByteFormatConverted4Frame = TRUE;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 0, "[%s %d] NexCodecUtil_CheckByteFormat iEnhLen = %d, nNewDataLen = %d\n", _FUNLINE_, a_nFrameLen, nNewDataLen);
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 0, "[%s %d] NexCodecUtil_CheckByteFormat a_pFrame = 0x%x, m_pIntermediateFrame = 0x%x, m_uH264NALSize = %d\n", _FUNLINE_, a_pFrame, a_hCodec->m_pIntermediateFrame, a_hCodec->m_uNALLenSize);
					}
				}
			}
		}

		nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_HEVC_SKIP_NOTREFIDC_BFRAME, &a_hCodec->m_qBFrameSkip);
	}
	else if (a_eCodecType == eNEX_CODEC_V_H263 || a_eCodecType == eNEX_CODEC_V_MPEG4V || a_eCodecType == eNEX_CODEC_V_SORENSONH263)
	{
		NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;

		if ((nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pConfig, a_nConfigLen, &DSI_Info)) != 0)
		{
			nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pFrame, a_nFrameLen, &DSI_Info);
		}

		if (0 == nRet)
		{
			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION, &qFormat) == NEXCAL_ERROR_NONE)
			{
				if (qFormat == NEXCAL_PROPERTY_ANSWERIS_NO)
				{
					*a_pnWidth = DSI_Info.usWidth;
					*a_pnHeight = DSI_Info.usHeight;   
					*a_pnPitch = DSI_Info.usWidth;
				}
			}

			if (a_eCodecType == eNEX_CODEC_V_MPEG4V)
			{
				NXINT64 qGMC = 0, qGMCWarpPoints = 0;
				NXINT64 qInterlaceSupport = 0;
				NXINT64 qQPEL = FALSE;

				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT, &qFormat) == NEXCAL_ERROR_NONE)
				{
					if (qFormat == NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_NO)
					{
						if (DSI_Info.uLevel == 3)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 Error(Profile[%u], Level[%u])!!! MPEG4 codec can't support Level 3.\n", _FUNLINE_, DSI_Info.uProfile, DSI_Info.uLevel);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
					}
				}

				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE, &qFormat) == NEXCAL_ERROR_NONE)
				{
					if (qFormat == NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE_NO)
					{
						if (DSI_Info.uProfile == 0xFF && DSI_Info.uLevel == 0xFF)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 Error(Profile[%u], Level[%u])!!! MPEG4 codec can't support Level 3.\n", _FUNLINE_, DSI_Info.uProfile, DSI_Info.uLevel);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
					}
				}

				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC, &qGMC) == NEXCAL_ERROR_NONE)
				{
					if (qGMC != NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_YES && 0 != DSI_Info.uGMC)
					{
						if ((qGMC & ((NXINT64)0x1<<DSI_Info.uGMC)) == 0)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 codec can't support GMC option(%d)!\n", _FUNLINE_, DSI_Info.uGMC);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}

						if (qGMC == NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_2)
						{
							if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT, &qGMCWarpPoints) == NEXCAL_ERROR_NONE)
							{
								if ((qGMCWarpPoints & ((NXINT64)0x1<<DSI_Info.no_of_sprite_warping_points)) == 0)
								{
									NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 codec can't support GMC option(2) GMCWarpPoints(%d) !\n", _FUNLINE_, DSI_Info.no_of_sprite_warping_points);
									return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
								}
							}
						}
					}
				}

				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE, &qInterlaceSupport) == NEXCAL_ERROR_NONE)
				{
					if (qInterlaceSupport == NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_NO)
					{
						if (DSI_Info.bInterlaced == TRUE)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 codec can't support Interlace!\n", _FUNLINE_);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
					}
				}

				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL, &qQPEL) == NEXCAL_ERROR_NONE)
				{
					if (qQPEL  == NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_NO)
					{
						if (DSI_Info.bQPEL == TRUE)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 codec can't support QPEL!\n", _FUNLINE_);
							return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
						}
					}
				}
			}
		}

		if (a_eCodecType == eNEX_CODEC_V_H263 || a_eCodecType == eNEX_CODEC_V_SORENSONH263)
		{
			NXINT64 qCheckEmulatedStartCode = 0;

			if (!nRet && DSI_Info.uSourceFormat == 7) // 나중에 define 정해야 함.
			{
				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_H263_ACCEPT_EXTENDEDPTYPE, &qFormat) == NEXCAL_ERROR_NONE)
				{
					if (qFormat == NEXCAL_PROPERTY_ANSWERIS_NO)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] H.263 Error(Type[%d])!!! H.263 codec can't support H.263 Extended P Type\n", _FUNLINE_, DSI_Info.uSourceFormat);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
			}

			if (DSI_Info.AnnexD)//Nx_robin__140311 H.263 UMV Support
			{
				if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_H263_SUPPORT_UMV, &qFormat) == NEXCAL_ERROR_NONE)
				{
					if (qFormat == NEXCAL_PROPERTY_H263_SUPPORT_UMV_NO)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] H.263 Error(UMV[%d])!!! H.263 codec can't support H.263 UMV! \n", _FUNLINE_, DSI_Info.AnnexD);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
			}

			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_H263_EMULATED_START_CODE_PLAY, &qCheckEmulatedStartCode) == NEXCAL_ERROR_NONE)
			{
				if (qCheckEmulatedStartCode == NEXCAL_PROPERTY_H263_EMULATED_START_CODE_PLAY_NO)
				{
					if (NexCodecUtil_H263_IsEmulatedStartCode(a_pConfig, a_nConfigLen) == TRUE)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] H.263 codec can't support emulated start code!\n", _FUNLINE_);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
			}
		}
	}
	else if (eNEX_CODEC_V_DIVX == a_eCodecType)
	{
		NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;
		NXINT64 qGMC = 0, qGMCWarpPoints = 0;
		NXINT64 qInterlaceSupport = 0;
		NXINT64 qQPEL = FALSE;
		NXINT32 nRet = 0;

		//divx drm 컨텐츠의 경우 frame start 코드 앞에 특정 정보 bit 가 있어서 parcing 에 문제가 생김(NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC 정보값 오류) - a_pConfiga 에서 추풀하도록 수정.  
		if ((a_nFrameLen>4) && (a_pFrame[0] == 0x00 && a_pFrame[1] == 0x00 && a_pFrame[2] == 0x01 && a_pFrame[3] == 0x40))		//DIVX DRM
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] DIVX DRM !\n", _FUNLINE_);

			if ((nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pConfig, a_nConfigLen, &DSI_Info)) != 0)
			{
				nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pFrame, a_nFrameLen, &DSI_Info);
			}
		}
		else
		{
			if ((nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pFrame, a_nFrameLen, &DSI_Info)) != 0)
			{
				nRet = NexCodecUtil_MPEG4V_GetDSIInfo(a_eCodecType, (NXCHAR *)a_pConfig, a_nConfigLen, &DSI_Info);
			}
		}

		if (0 == nRet)
		{
			a_hCodec->m_uVOPTimeIncBits = DSI_Info.uVOPTimeIncBits;

			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC, &qGMC) == NEXCAL_ERROR_NONE)
			{
				if (qGMC != NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_YES && 0 != DSI_Info.uGMC)
				{
					if ((qGMC & ((NXINT64)0x1<<DSI_Info.uGMC)) == 0)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] DivX codec can't support GMC option(%d)!\n", _FUNLINE_, DSI_Info.uGMC);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}

					if (qGMC == NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_2)
					{
						if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT, &qGMCWarpPoints) == NEXCAL_ERROR_NONE)
						{
							if ((qGMCWarpPoints & ((NXINT64)0x1<<DSI_Info.no_of_sprite_warping_points)) == 0)
							{
								NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] DivX codec can't support GMC option(2) GMCWarpPoints(%d) !\n", _FUNLINE_, DSI_Info.no_of_sprite_warping_points);
								return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
							}
						}
					}
				}
			}

			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE, &qInterlaceSupport) == NEXCAL_ERROR_NONE)
			{
				if (qInterlaceSupport == NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_NO)
				{
					if (DSI_Info.bInterlaced == TRUE)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] DivX codec can't support Interlace!\n", _FUNLINE_);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
			}

			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL, &qQPEL) == NEXCAL_ERROR_NONE)
			{
				if (qQPEL == NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_NO)
				{
					if (DSI_Info.bQPEL == TRUE)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MPEG4 codec can't support QPEL!\n", _FUNLINE_);
						return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
					}
				}
			}
		}
		else
		{
			a_hCodec->m_uVOPTimeIncBits = NEXCODECUTIL_TIMEINCBITS_INVALID;
		}
	}
	else if ((eNEX_CODEC_V_WMV3 == a_eCodecType) || (eNEX_CODEC_V_WVC1 == a_eCodecType))
	{
		NEXCODECUTIL_VC1_EXTRADATA_INFO  VC1Info;
		NXINT64 qSupportableProfile = 0;
		NXINT64 qSupportableYUV420 = 0;

		memset(&VC1Info, 0x00, sizeof(NEXCODECUTIL_VC1_EXTRADATA_INFO));

		if (NexCodecUtil_VC1_GetExtraDataInfo((NXCHAR*)a_pConfig, a_nConfigLen, a_eCodecType, &VC1Info) != NEXCAL_ERROR_NONE)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Probably this VC1 profile is wrong(%d)!\n", _FUNLINE_, VC1Info.uProfile);
			return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_VC1_SUPPORTABLE_PROFILE, &qSupportableProfile) == NEXCAL_ERROR_NONE)
		{
			if ((qSupportableProfile & ((NXINT64)0x1<<VC1Info.uProfile)) == 0)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] VC1 codec can't support profile(%d)!\n", _FUNLINE_, VC1Info.uProfile);
				return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
			}
		}

		if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_VC1_SUPPORT_NOYUV420, &qSupportableYUV420) == NEXCAL_ERROR_NONE)
		{
			if (qSupportableYUV420 == NEXCAL_PROPERTY_VC1_SUPPORT_NOYUV420_NO)
			{
				if (VC1Info.bNotYUV420)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] VC1 codec can support only YUV420!\n", _FUNLINE_);
					return NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
				}
			}
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] VC1 profile(%d)!\n", _FUNLINE_, VC1Info.uProfile);

		nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_VC1_SKIP_BFRAME, &a_hCodec->m_qBFrameSkip);
	}

#if 0
	// To Distinguish between Thumbnail and Player. 
	// by kwangsik.lee.  
	//a_hCodec->m_uUserData = a_hCodec->m_uType; 

	// by kwangsik.lee.  2011-09-05
	//Thumbnail decoder resizes to 1/8. But this thumbnail size is too samll.(especially in MMS sender. it raise a problem)
	a_hCodec->m_uUserData = NEXCAL_MEDIATYPE_VIDEO; 

#else
	a_hCodec->m_pUserData = a_pClientIdentifier;
#endif

#ifdef _DECODE_INPUT_VIDEO_DUMP_
	nexSAL_FileRemove(s_strDecodeInputVideoPath);
	if (a_pConfig && a_nConfigLen)
	{
		NexUtil_FrameDump(s_strDecodeInputVideoPath, a_eCodecType, a_pConfig, a_nConfigLen);
	}
#endif

	nRet = a_hCodec->m_cfs.vd.Init
		   (a_eCodecType
			, a_pConfig
			, a_nConfigLen
			, a_pFrame
			, a_nFrameLen
			, a_pInitInfo
			, a_pExtraInfo
			, a_nNALHeaderSize
			, a_pnWidth
			, a_pnHeight
			, a_pnPitch
			, a_uMode
			, a_uUserDataType
			, &a_hCodec->m_pUserData);

	a_hCodec->m_uWidth = *a_pnWidth;
	a_hCodec->m_uHeight = *a_pnHeight;
	a_hCodec->m_uWidthPitch = *a_pnPitch;
	a_hCodec->m_uHeightPitch = *a_pnHeight;

	if (pNewConfig)
	{
		SAFE_FREE(pNewConfig);
		pNewConfig = (NXUINT8 *)0;
	}

	//K.Lee add 20090330 [ init 실패시 VideoDeinit이 호출되지 않기 때문에, 메모리 leak이 발생하여 이를 수정.
	if (nRet != NEXCAL_ERROR_NONE)
	{
		SAFE_FREE(a_hCodec->m_pDSI);

		if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
		{
			SAFE_FREE(a_hCodec->m_pIntermediateFrame);
			a_hCodec->m_pIntermediateFrame = (NXVOID *)0;
			a_hCodec->m_uIntermediateFrameSize = 0;
		}
	}
	else
	{
		if (a_hCodec->m_uOutBufferFormat > NEXCAL_PROPERTY_VIDEO_BUFFER_UNKNOWN)
		{
			a_hCodec->m_cfs.vd.SetProperty(NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, a_hCodec->m_uOutBufferFormat, a_hCodec->m_pUserData);
		}
	}
	//K.Lee ]

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderDeinit(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		NXUINT32 nRet;

		SAFE_FREE(a_hCodec->m_pDSI);

		if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
		{
			SAFE_FREE(a_hCodec->m_pIntermediateFrame);
			a_hCodec->m_pIntermediateFrame = (NXVOID *)0;
			a_hCodec->m_uIntermediateFrameSize = 0;
		}
		nRet = a_hCodec->m_cfs.vd.Deinit(a_hCodec->m_pUserData);
		a_hCodec->m_pUserData = NULL;
		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderDecode(NEXCALCodecHandle a_hCodec, 
											 NXUINT8 *a_pFrame, 
											 NXINT32 a_nFrameLen, 
											 NXVOID *a_pExtraInfo,
											 NXUINT32 a_uDTS, 
											 NXUINT32 a_uPTS, 
											 NXINT32 a_nFlag, 
											 NXUINT32 *a_puDecodeResult)
{
	NXINT32 nNewDataLen = 0;
	NXINT64 qFormat = 0;
	NXINT32 nRet = 0;
	NEXSALFileHandle hFile;
	NXBOOL bIPframeDecoded = FALSE;

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_FAIL;
	}

	if (NEXCAL_VDEC_FLAG_END_OF_STREAM & a_nFlag)
	{
		a_pFrame = NULL;
		a_nFrameLen = 0;
	}
	else if (a_pFrame == NULL || a_nFrameLen == 0)
	{
		NEXCAL_INIT_VDEC_RET(*a_puDecodeResult);
		NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] Video Input Data is NULL.\n", _FUNLINE_);

		return NEXCAL_ERROR_FAIL;
	}

	if (a_pFrame && (a_nFrameLen > 0))
	{
		if (NULL != (hFile = ((NEXCALRegistrationInfo*)(a_hCodec->m_hCAL))->m_ahDump[NEXCAL_MEDIATYPE_VIDEO]))
		{
			nexSAL_FileWrite(hFile, &a_nFrameLen, sizeof(a_nFrameLen));
			nexSAL_FileWrite(hFile, a_pFrame, a_nFrameLen);
		}

		if ((a_hCodec->m_eCodecType == eNEX_CODEC_V_H264 && a_hCodec->m_uNALLenSize == 0) || !NexCodecUtil_IsValidVideo(a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen))
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Video Data(%u) is Invalid\n", _FUNLINE_, a_uDTS);
			NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
			NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
			return NEXCAL_ERROR_FAIL;
		}

		if (a_hCodec->m_eCodecType == eNEX_CODEC_V_H264)
		{
			if (TRUE == a_hCodec->m_bByteFormatConverted4Frame)	//[shoh][2013.06.10] First of all, check  that H264 format was converted.
			{
				if (a_hCodec->m_uByteFormat4Frame == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || a_hCodec->m_uByteFormat4Frame == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
				{
					NXUINT32 uH264OriginalFormat4Frame = (a_hCodec->m_uByteFormat4Frame==NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW) ? NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB : NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;

					if ((NXINT32)a_hCodec->m_uIntermediateFrameSize < a_nFrameLen)
					{
						if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
						{
							SAFE_FREE(a_hCodec->m_pIntermediateFrame);
							a_hCodec->m_pIntermediateFrame = (NXVOID*)0;
							a_hCodec->m_uIntermediateFrameSize = 0;
						}

						a_hCodec->m_pIntermediateFrame = (NXUINT8 *)nexSAL_MemAlloc(a_nFrameLen + 100);
						if (!a_hCodec->m_pIntermediateFrame)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
							NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
							NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
							return NEXCAL_ERROR_FAIL;
						}
						a_hCodec->m_uIntermediateFrameSize = a_nFrameLen;
					}

					nNewDataLen = NexCodecUtil_ConvertFormat(a_hCodec->m_pIntermediateFrame, a_nFrameLen + 100, a_pFrame, a_nFrameLen, uH264OriginalFormat4Frame, a_hCodec->m_uNALLenSize);
					if (nNewDataLen == NEXCAL_ERROR_FAIL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_ConvertFormat failed[%u]\n", _FUNLINE_, a_uDTS);
						a_pFrame = NULL;
						a_nFrameLen = 0;
					}
					else
					{
						a_pFrame = a_hCodec->m_pIntermediateFrame;
						a_nFrameLen = nNewDataLen;
					}
				}
				else if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ENCRYPTED == qFormat)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Encrypted Frame, Skip Format Check\n", _FUNLINE_);
				}
			}
			else
			{
				if (NexCodecUtil_AVC_IsErrorFrame(a_pFrame, a_nFrameLen, a_hCodec->m_uByteFormat4Frame, a_hCodec->m_uNALLenSize) == TRUE)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] This frame is error![%u]\n", _FUNLINE_, a_uDTS);
					a_pFrame = NULL;
					a_nFrameLen = 0;
					NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
					NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);                  
					return NEXCAL_ERROR_NONE;
				}
			}

			//K.Lee add 20090107 add [ I or P frame 앞에 PS가 존재하는 경우 codec의 지원여부 확인
			//지원하지 않으면 discard 시킨다.
			if (a_hCodec->m_qIncludeConfig == NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK)
			{
				NXUINT32 uNewFrameSize = 0;
				NXUINT32 uConfigStreamOffset = 0;
				NXUINT8 *pConfigStream = NULL;

				if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)
				{
					pConfigStream = NexCodecUtil_AVC_ANNEXB_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32*)&uNewFrameSize);
				}
				else
				{
					pConfigStream = NexCodecUtil_AVC_NAL_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32)a_hCodec->m_uNALLenSize, (NXINT32*)&uNewFrameSize);
				}

				if (pConfigStream != NULL)
				{
					if (a_nFrameLen == (NXINT32)uNewFrameSize)
					{
						NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
						NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);

						return NEXCAL_ERROR_NONE;
					}
					else
					{
						uConfigStreamOffset = (NXUINT32)(pConfigStream - a_pFrame);
						a_pFrame = pConfigStream + uNewFrameSize;
						a_nFrameLen -= uConfigStreamOffset + uNewFrameSize;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 1, "[%s %d] %s bitstream has SPSPPS. => => Discard SPSPPS\n", _FUNLINE_,
										(NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)?(NXCHAR*)"ANNEX-B":(NXCHAR*)"NAL");
					}
				}
			}
			//K.Lee ]
		}
		else if (a_hCodec->m_eCodecType == eNEX_CODEC_V_HEVC)
		{
			if (TRUE == a_hCodec->m_bByteFormatConverted4Frame)	//[shoh][2013.06.10] First of all, check  that H264 format was converted.
			{
				if (a_hCodec->m_uByteFormat4Frame == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW || a_hCodec->m_uByteFormat4Frame == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
				{
					NXUINT32 uH264OriginalFormat4Frame = (a_hCodec->m_uByteFormat4Frame==NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW) ? NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB : NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;

					if ((NXINT32)a_hCodec->m_uIntermediateFrameSize < a_nFrameLen)
					{
						if (a_hCodec->m_pIntermediateFrame && a_hCodec->m_uIntermediateFrameSize > 0)
						{
							SAFE_FREE(a_hCodec->m_pIntermediateFrame);
							a_hCodec->m_pIntermediateFrame = (NXVOID*)0;
							a_hCodec->m_uIntermediateFrameSize = 0;
						}

						a_hCodec->m_pIntermediateFrame = (NXUINT8 *)nexSAL_MemAlloc(a_nFrameLen + 100);
						if (!a_hCodec->m_pIntermediateFrame)
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
							NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
							NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
							return NEXCAL_ERROR_FAIL;
						}
						a_hCodec->m_uIntermediateFrameSize = a_nFrameLen;
					}

					nNewDataLen = NexCodecUtil_ConvertFormat(a_hCodec->m_pIntermediateFrame, a_nFrameLen + 100, a_pFrame, a_nFrameLen, uH264OriginalFormat4Frame, a_hCodec->m_uNALLenSize);
					if (nNewDataLen == NEXCAL_ERROR_FAIL)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_ConvertFormat failed[%u]\n", _FUNLINE_, a_uDTS);
						a_pFrame = NULL;       
						a_nFrameLen = 0;
					}
					else
					{
						a_pFrame = a_hCodec->m_pIntermediateFrame;
						a_nFrameLen = nNewDataLen;
					}
				}
			}

			//K.Lee add 20090107 add [ I or P frame 앞에 PS가 존재하는 경우 codec의 지원여부 확인
			//지원하지 않으면 discard 시킨다.
			if (a_hCodec->m_qIncludeConfig == NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK)
			{
				NXUINT32 uNewFrameSize = 0;
				NXUINT32 uConfigStreamOffset = 0;
				NXUINT8 *pConfigStream = NULL;

				if (NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)
				{
					pConfigStream = NexCodecUtil_HEVC_ANNEXB_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32*)&uNewFrameSize);
				}
				else
				{
					pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(a_pFrame, a_nFrameLen, (NXINT32)a_hCodec->m_uNALLenSize, (NXINT32*)&uNewFrameSize);
				}

				if (pConfigStream != NULL)
				{
					if (a_nFrameLen == (NXINT32)uNewFrameSize)
					{
						NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
						NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);

						return NEXCAL_ERROR_NONE;
					}
					else
					{
						uConfigStreamOffset = (NXUINT32)(pConfigStream - a_pFrame);
						a_pFrame = pConfigStream + uNewFrameSize;
						a_nFrameLen -= uConfigStreamOffset + uNewFrameSize;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_CRAL, 1, "[%s %d] %s bitstream has SPSPPS. => => Discard SPSPPS\n", _FUNLINE_,
										(NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == a_hCodec->m_uByteFormat4Frame)?(NXCHAR*)"ANNEX-B":(NXCHAR*)"NAL");
					}
				}
			}
			//K.Lee ]
		}
		else if (a_hCodec->m_eCodecType == eNEX_CODEC_V_MPEG4V)
		{
			NXUINT8 *pTemp;
			NXINT32 nTempSize;

			if (nexCAL_VideoDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE, &qFormat) == NEXCAL_ERROR_NONE)
			{
				if (qFormat == NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_VOP)
				{
					pTemp = NexCodecUtil_FindVOP(a_pFrame, a_nFrameLen, &nTempSize);
					if (pTemp)
					{
						a_pFrame = pTemp;
						a_nFrameLen = nTempSize;
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] There is no VOP[%u]\n", _FUNLINE_, a_uDTS);
						nNewDataLen = NEXCAL_ERROR_FAIL;
					}
				}
			}
		}

		if (NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME_OK == a_hCodec->m_qBFrameSkip ||
			NEXCAL_PROPERTY_HEVC_SKIP_NOTREFIDC_BFRAME_OK == a_hCodec->m_qBFrameSkip ||
			NEXCAL_PROPERTY_VC1_SKIP_BFRAME_OK == a_hCodec->m_qBFrameSkip)
		{
			NXBOOL bBFrame = FALSE;

			if (a_hCodec->m_eCodecType == eNEX_CODEC_V_H264 || a_hCodec->m_eCodecType == eNEX_CODEC_V_HEVC)
			{
				bBFrame = NexCodecUtil_IsBFrame(a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen, (NXVOID*)&a_hCodec->m_uNALLenSize, a_hCodec->m_uByteFormat4Frame, TRUE);
			}
			else
			{
				bBFrame = NexCodecUtil_IsBFrame(a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen, (NXVOID*)a_hCodec->m_pDSI, 0, FALSE);
			}

			if (TRUE == bBFrame)
			{
				NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
				NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
				return NEXCAL_ERROR_NONE;
			}
		}

		//[shoh][2011.05.23] B-frame should be decoded by the condition decoding both I-frame and P-frame after seek.
		bIPframeDecoded = (NEXCAL_IP_FRAME_DECODED == (a_hCodec->m_uMask4Bframe & NEXCAL_IP_FRAME_DECODED));

		if (IS_BFRAME_SKIP_TYPE(a_hCodec->m_eCodecType) && FALSE == bIPframeDecoded && !(NEXCAL_VDEC_FLAG_END_OF_STREAM & a_nFlag))
		{
			//[shoh][2011.08.23] If it is not decoded frame, frame should skip to check frame type.
			if (eNEX_CODEC_V_DIVX == a_hCodec->m_eCodecType)
			{
				NEXCODECUTIL_MPEG4V_DSI_INFO stDSI_Info;
				NXUINT8 *pVOP = NULL;
				NXINT32 nVOPLen = 0;

				pVOP = NexCodecUtil_FindVOP(a_pFrame, a_nFrameLen, &nVOPLen);

				if (pVOP == NULL || nVOPLen < 5)
				{
					NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
					NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
					return NEXCAL_ERROR_NONE;
				}

				//[shoh][2011.09.01] Encrypted data should avoid checking coded-frame.
				if (a_nFlag & NEXCAL_VDEC_FLAG_ENCRYPTED)
				{
					a_hCodec->m_uVOPTimeIncBits = NEXCODECUTIL_TIMEINCBITS_ENCRYPTED;
				}
				else if (a_pFrame != pVOP)	//Nex_Robin__111110 Seek 이후에도 m_uVOPTimeIncBits를 update해야한다. 특정 컨텐츠의 경우 m_uVOPTimeIncBits 값이 변경되는데, 아래에서 m_uVOPTimeIncBits를 update하지 않으면 문제 된다.
				{
					if (NexCodecUtil_MPEG4V_GetDSIInfo(a_hCodec->m_eCodecType, (NXCHAR*)a_pFrame, (a_nFrameLen-nVOPLen), &stDSI_Info) != 0)
					{
						a_hCodec->m_uVOPTimeIncBits = NEXCODECUTIL_TIMEINCBITS_INVALID;
					}
					else
					{
						a_hCodec->m_uVOPTimeIncBits = stDSI_Info.uVOPTimeIncBits;
					}
				}

				if (NexCodecUtil_CheckCodedVOP(pVOP, (unsigned)nVOPLen, a_hCodec->m_uVOPTimeIncBits) == FALSE)
				{
					NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
					NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
					return NEXCAL_ERROR_NONE;
				}
			}

			if (TRUE == NexCodecUtil_IsBFrame(a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen, a_hCodec->m_pDSI, 0, FALSE))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] B-frame is skipped because I-frame and P-frame are not decoded yet. m_uMask4BFrame(0x%x)\n", _FUNLINE_, a_hCodec->m_uMask4Bframe);
				a_hCodec->m_uMask4Bframe |= NEXCAL_B_FRAME_DECODED;
				NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
				NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
				return NEXCAL_ERROR_NONE;               
			}
			else
			{
				NXVOID *pETC = NULL;

				if (eNEX_CODEC_V_DIVX == a_hCodec->m_eCodecType)
				{
					pETC = (NXVOID*)&a_hCodec->m_uVOPTimeIncBits;
				}
				else if (eNEX_CODEC_V_WVC1 == a_hCodec->m_eCodecType)
				{
					pETC = (NXVOID*)a_hCodec->m_pDSI;
				}
				else
				{
					pETC = (NXVOID*)&a_hCodec->m_uNALLenSize;
				}   

				if (NexCodecUtil_IsSeekableFrame(a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen, pETC, a_hCodec->m_uByteFormat4Frame, FALSE))
				{
					NXBOOL bIframeDecoded = (NEXCAL_I_FRAME_DECODED == (NEXCAL_I_FRAME_DECODED & a_hCodec->m_uMask4Bframe));

					if (bIframeDecoded)
					{
						a_hCodec->m_uMask4Bframe |= NEXCAL_P_FRAME_DECODED;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Display due to meeting the second I-frame. m_uMask4BFrame(0x%x)\n", _FUNLINE_, a_hCodec->m_uMask4Bframe);
					}
					else
					{
						a_hCodec->m_uMask4Bframe |= NEXCAL_I_FRAME_DECODED;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Meeting the first I-frame. m_uMask4BFrame(0x%x)\n", _FUNLINE_, a_hCodec->m_uMask4Bframe);
					}
				}
				else
				{
					a_hCodec->m_uMask4Bframe |= NEXCAL_P_FRAME_DECODED;
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Display due to meeting P-frame. m_uMask4BFrame(0x%x)\n", _FUNLINE_, a_hCodec->m_uMask4Bframe);
				}
			}
		}
	}

	if (nNewDataLen >= 0)
	{
		NXUINT32 uStartTime = nexSAL_GetTickCount();

#ifdef _DECODE_INPUT_VIDEO_DUMP_
		if (a_pFrame && a_nFrameLen)
		{
			NexUtil_FrameDump(s_strDecodeInputVideoPath, a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen);
		}
#endif
		nRet = a_hCodec->m_cfs.vd.Decode
			   (a_pFrame
				, a_nFrameLen
				, a_pExtraInfo
				, a_uDTS
				, a_uPTS
				, a_nFlag
				, a_puDecodeResult
				, a_hCodec->m_pUserData);

		if ((NULL == a_hCodec->m_cfs.vd.GetInfo) && (TRUE == NEXCAL_CHECK_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS)))
		{
			a_hCodec->m_uVDecCnt++;
			a_hCodec->m_uTotalVDecTime += nexSAL_GetTickCount() - uStartTime;
		}

		//[shoh][2011.08.18] If it is the end-frame, it does not need a next-frame.
		if (NEXCAL_VDEC_FLAG_END_OF_STREAM & a_nFlag)
		{
			NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		}
	}
	else
	{
		if (NEXCAL_VDEC_FLAG_END_OF_STREAM & a_nFlag)
		{
			NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
		}
		else
		{
			NEXCAL_DEL_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
			NEXCAL_SET_VDEC_RET(*a_puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		}
		nRet = NEXCAL_ERROR_INVALID_PARAMETER;
	}

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderGetOutput(NEXCALCodecHandle a_hCodec, 
												NXUINT8 **a_ppBits1, 
												NXUINT8 **a_ppBits2, 
												NXUINT8 **a_ppBits3, 
												NXUINT32 *a_puPTS)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.vd.GetOutput(a_ppBits1, a_ppBits2, a_ppBits3, a_puPTS, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderReset(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		//[shoh][2011.05.23] B-frame should be decoded by the condition decoding both I-frame and P-frame after seek.
		a_hCodec->m_uMask4Bframe = 0;

		if (a_hCodec->m_cfs.vd.GetInfo == NULL)
		{
			a_hCodec->m_uVDecCnt = 0;
			a_hCodec->m_uTotalVDecTime = 0;
		}

		return a_hCodec->m_cfs.vd.Reset(a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult)
{
	NXINT32 nRet = NEXCAL_ERROR_NONE;

	if (!a_hCodec)
	{
		*a_puResult = 0;
		return NEXCAL_ERROR_FAIL;
	}

	switch (a_uIndex)
	{
		case NEXCAL_VIDEO_GETINFO_CODEC_CLASS:
			if (NEXCAL_CHECK_STATIC_PROPERTY(a_hCodec->m_uProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC))
			{
				*a_puResult = NEXCAL_VIDEO_GETINFO_CODEC_CLASS_HW;
			}
			else
			{
				*a_puResult = NEXCAL_VIDEO_GETINFO_CODEC_CLASS_SW;
			}
			return NEXCAL_ERROR_FAIL;

		default:
			break;
	}

	if (a_hCodec->m_cfs.vd.GetInfo == NULL)
	{
		switch (a_uIndex)
		{
			case NEXCAL_VIDEO_GETINFO_WIDTH :
				*a_puResult = a_hCodec->m_uWidth;
				break;
			case NEXCAL_VIDEO_GETINFO_HEIGHT :
				*a_puResult = a_hCodec->m_uHeight;
				break;
			case NEXCAL_VIDEO_GETINFO_WIDTHPITCH :
				*a_puResult = a_hCodec->m_uWidthPitch;
				break;
			case NEXCAL_VIDEO_GETINFO_HEIGHTPITCH :
				*a_puResult = a_hCodec->m_uHeight;
				break;
			case NEXCAL_VIDEO_GETINFO_NUM_INPUTBUFFERS :
				*a_puResult = 1;
				break;
			case NEXCAL_VIDEO_GETINFO_INPUTBUFFER_SIZE :
				*a_puResult = (NXUINT32)INVALID_VALUE;
				break;
			case NEXCAL_VIDEO_GETINFO_NUM_OUTPUTBUFFERS :
				*a_puResult = 1;
				break;
			case NEXCAL_VIDEO_GETINFO_OUTPUTBUFFER_SIZE :
				*a_puResult = (NXUINT32)INVALID_VALUE;
				break;
			case NEXCAL_VIDEO_GETINFO_RENDERER_RESET:
				*a_puResult = NEX_CAL_VIDEO_GETINFO_NOT_USE_RENDERER_RESET;
				break;
			case NEXCAL_VIDEO_GETINFO_AVERAGE_DECODING_TIME:
				if (a_hCodec->m_uVDecCnt > 0)
				{
					*a_puResult = (a_hCodec->m_uTotalVDecTime / a_hCodec->m_uVDecCnt);
				}
				else
				{
					*a_puResult = 0;
				}
				break;
			case NEXCAL_VIDEO_GETINFO_NEED_CODEC_REINIT:
				*a_puResult = 0;
				break;

			default :
				nRet = NEXCAL_ERROR_INVALID_PARAMETER;
				break;
		}
	}
	else
	{
		return a_hCodec->m_cfs.vd.GetInfo(a_uIndex, a_puResult, a_hCodec->m_pUserData);
	}

	return nRet;
}

/*-----------------------------------------------------------------------------
		Audio Decoder Wrapper
-----------------------------------------------------------------------------*/

NEXCAL_API NXINT32 nexCAL_AudioDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{

		if (NEXCAL_PROPERTY_GET_DECODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		*a_pqValue = a_hCodec->m_eCodecType;//simple calbody add	
		return a_hCodec->m_cfs.ad.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.ad.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderInit(NEXCALCodecHandle a_hCodec, 
										   NEX_CODEC_TYPE a_eCodecType, 
										   NXUINT8 *a_pConfig, 
										   NXINT32 a_nConfigLen, 
										   NXUINT8 *a_pFrame, 
										   NXINT32 a_nFrameLen, 
										   NXVOID *a_pInitInfo,
										   NXVOID *a_pExtraInfo,
										   NXUINT32 *a_puSamplingRate, 
										   NXUINT32 *a_puNumOfChannels, 
										   NXUINT32 *a_puBitsPerSample, 
										   NXUINT32 *a_puNumOfSamplesPerChannel, 
										   NXUINT32 a_uMode,
										   NXUINT32 a_uUserDataType, 
										   NXVOID *a_pClientIdentifier)
{
	NXINT32 nRet = NEXCAL_ERROR_FAIL;
	NEXSALFileHandle hFile;
	NXINT64 a_qValue = 0;

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	if (NULL != (hFile = ((NEXCALRegistrationInfo*)(a_hCodec->m_hCAL))->m_ahDump[NEXCAL_MEDIATYPE_AUDIO]))
	{
		nexSAL_FileWrite(hFile, &a_nConfigLen, sizeof(a_nConfigLen));
		nexSAL_FileWrite(hFile, a_pConfig, a_nConfigLen);
	}

	if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AMR || a_hCodec->m_eCodecType == eNEX_CODEC_A_AMRWB)
	{
		nexCAL_AudioDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SUPPORT_TO_DECODE_AMR_SID, &a_qValue);

		if (NEXCAL_PROPERTY_SUPPORT_TO_DECODE_AMR_SID_NO == a_qValue)
		{
			a_hCodec->m_uType = a_pFrame[0] >> 3;

			if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AMR && a_hCodec->m_uType <= 7)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] AMR NB FT = [%d]\n", _FUNLINE_, a_hCodec->m_uType);
				NexCodecUtil_AMRNB_MuteFrame((NXUINT8**)&a_hCodec->m_pUserFrame, &a_hCodec->m_uUserFrameLen, a_hCodec->m_uType);
			}
			else if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AMRWB && a_hCodec->m_uType <= 8)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] AMR WB FT = [%d]\n", _FUNLINE_, a_hCodec->m_uType);
				NexCodecUtil_AMRWB_MuteFrame((NXUINT8**)&a_hCodec->m_pUserFrame, &a_hCodec->m_uUserFrameLen, a_hCodec->m_uType);
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR %s FT = [%d]\n", _FUNLINE_,(a_hCodec->m_eCodecType == eNEX_CODEC_A_AMR)?(NXCHAR*)"NB":(NXCHAR*)"WB", a_hCodec->m_uType);
			}
		}
	}

	if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AAC || a_hCodec->m_eCodecType == eNEX_CODEC_A_AACPLUS || a_hCodec->m_eCodecType == eNEX_CODEC_A_AACPLUSV2)
	{
		nexCAL_AudioDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER, &a_hCodec->m_qSupportADTSHeader);

		if (a_hCodec->m_qSupportADTSHeader == NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_N0)
		{
			NXINT32 nADTSHeaderSize = NexCodecUtil_GetSizeOfADTSHeader(a_pFrame);

			if (nADTSHeaderSize)
			{
				if (a_nFrameLen > nADTSHeaderSize)
				{
					a_pFrame += nADTSHeaderSize;
					a_nFrameLen -= nADTSHeaderSize;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] ADTS Header Size[%d], a_nFrameLen[%d]\n", _FUNLINE_, nADTSHeaderSize, a_nFrameLen);
				}
			}
		}
	}

	a_hCodec->m_pUserData = a_pClientIdentifier;

	if (a_hCodec)
	{
#ifdef _DECODE_INPUT_AUDIO_DUMP_
		nexSAL_FileRemove(s_strDecodeInputAudioPath);
		if (a_pConfig && a_nConfigLen)
		{
			NexUtil_FrameDump(s_strDecodeInputAudioPath, a_eCodecType, a_pConfig, a_nConfigLen);
		}
#endif
		nRet = a_hCodec->m_cfs.ad.Init(a_eCodecType, 
									   a_pConfig, 
									   a_nConfigLen, 
									   a_pFrame, 
									   a_nFrameLen, 
									   a_pInitInfo,
									   a_pExtraInfo,
									   a_puSamplingRate, 
									   a_puNumOfChannels, 
									   a_puBitsPerSample, 
									   a_puNumOfSamplesPerChannel,
									   a_uMode,
									   a_uUserDataType, 
									   &a_hCodec->m_pUserData);
	}

	if (NEXCAL_ERROR_NONE != nRet) //simple calbody add
	{
		a_hCodec->m_pUserData = 0;
	}
	else
	{
		a_hCodec->uSamplingRate = *a_puSamplingRate;
	}

	a_hCodec->uNumOfChannels = *a_puNumOfChannels;
	a_hCodec->uBitsPerSample = *a_puBitsPerSample;
	a_hCodec->uNumOfSamplesPerChannel = *a_puNumOfSamplesPerChannel;

	nexCAL_AudioDecoderGetProperty(a_hCodec, NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION, &a_qValue);

	if (a_qValue == NEXCAL_PROPERTY_ANSWERIS_NO)
	{
		switch (a_hCodec->m_eCodecType)
		{
			case eNEX_CODEC_A_AAC:
			case eNEX_CODEC_A_AACPLUS:
			case eNEX_CODEC_A_AACPLUSV2:
				{
					NXINT32 isSBR;
					NexCodecUtil_AAC_ParseDSI(a_pConfig, a_nConfigLen, a_puSamplingRate, a_puNumOfChannels, &isSBR);
					if (isSBR)
					{
						*a_puSamplingRate *=2;
						*a_puNumOfSamplesPerChannel = 2048;
					}
					else
					{
						*a_puNumOfSamplesPerChannel = 1024;
					}
					*a_puBitsPerSample = 16;
				}
				break;

			case eNEX_CODEC_A_AMR:
			case eNEX_CODEC_A_AMRWB:
			case eNEX_CODEC_A_EVRC:
				*a_puSamplingRate = 8000;
				*a_puNumOfChannels = 1;
				*a_puBitsPerSample = 16;
				*a_puNumOfSamplesPerChannel = 160; // 8000*20/1000
				break;

			default:
				break;
		}
	}

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderDeinit(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		NXINT32 nRet;
		nRet = a_hCodec->m_cfs.ad.Deinit(a_hCodec->m_pUserData);
		a_hCodec->m_pUserData = 0;
		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderDecode(NEXCALCodecHandle a_hCodec, 
											 NXUINT8 *a_pFrame, 
											 NXINT32 a_nFrameLen, 
											 NXVOID *a_pExtraInfo,
											 NXVOID *a_pDest, 
											 NXINT32 *a_pnWrittenPCMSize, 
											 NXUINT32 a_uDTS, 
											 NXUINT32 *a_puOutputTime, 
											 NXINT32 a_nFlag, 
											 NXUINT32 *a_puDecodeResult)
{
	NEXSALFileHandle hFile;
	NXINT32 nRet = 0;

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	if (NEXCAL_ADEC_FLAG_END_OF_STREAM & a_nFlag)
	{
		a_pFrame = NULL;
		a_nFrameLen = 0;
	}
	else if (a_pFrame == NULL || a_nFrameLen == 0)
	{
		NEXCAL_INIT_ADEC_RET(*a_puDecodeResult);
		NEXCAL_SET_ADEC_RET(*a_puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] Audio Input Data is NULL...[0x%x|%d]\n", _FUNLINE_, a_pFrame, a_nFrameLen);

		return NEXCAL_ERROR_FAIL;
	}

	if (a_pFrame && (a_nFrameLen > 0))
	{
		if (NULL != (hFile = ((NEXCALRegistrationInfo*)(a_hCodec->m_hCAL))->m_ahDump[NEXCAL_MEDIATYPE_AUDIO]))
		{
			nexSAL_FileWrite(hFile, &a_nFrameLen, sizeof(a_nFrameLen));
			nexSAL_FileWrite(hFile, a_pFrame, a_nFrameLen);
		}

		if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AMR || a_hCodec->m_eCodecType == eNEX_CODEC_A_AMRWB)
		{
			if ((a_hCodec->m_pUserFrame != NULL) && (a_hCodec->m_uUserFrameLen != 0)
				&& (((a_hCodec->m_uType != (NXUINT32)(a_pFrame[0] >> 3))||(a_nFrameLen != (NXINT32)a_hCodec->m_uUserFrameLen))))
			{
#ifdef _DECODE_INPUT_AUDIO_DUMP_
				if (a_hCodec->m_pUserFrame && a_hCodec->m_uUserFrameLen)
				{
					NexUtil_FrameDump(s_strDecodeInputAudioPath, a_hCodec->m_eCodecType, a_hCodec->m_pUserFrame, a_hCodec->m_uUserFrameLen);
				}
#endif

				nRet = a_hCodec->m_cfs.ad.Decode((NXUINT8*)a_hCodec->m_pUserFrame, 
												 a_hCodec->m_uUserFrameLen, 
												 a_pExtraInfo,
												 a_pDest, 
												 a_pnWrittenPCMSize, 
												 a_uDTS, 
												 a_puOutputTime, 
												 a_nFlag, 
												 a_puDecodeResult, 
												 a_hCodec->m_pUserData);

#ifdef _DECODE_OUTPUT_AUDIO_DUMP_
				NexUtil_FrameDump(s_strDecodeOutputAudioPath, a_hCodec->m_eCodecType, a_pDest, *a_pnWrittenPCMSize);
#endif			
				return nRet;

			}
		}

		if (a_hCodec->m_eCodecType == eNEX_CODEC_A_AAC || a_hCodec->m_eCodecType == eNEX_CODEC_A_AACPLUS || a_hCodec->m_eCodecType == eNEX_CODEC_A_AACPLUSV2)
		{
			if (a_hCodec->m_qSupportADTSHeader == NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_N0)
			{
				NXINT32 nADTSHeaderSize = NexCodecUtil_GetSizeOfADTSHeader(a_pFrame);
				if (nADTSHeaderSize)
				{
					if (a_nFrameLen > nADTSHeaderSize)
					{
						a_pFrame += nADTSHeaderSize;
						a_nFrameLen -= nADTSHeaderSize;
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0,"[%s %d] ADTS Header Size[%d], a_nFrameLen[%d]\n", _FUNLINE_, nADTSHeaderSize, a_nFrameLen);
					}
				}
			}
		}
	}

	if (a_hCodec)
	{
#ifdef _DECODE_INPUT_AUDIO_DUMP_		
		if (a_pFrame && a_nFrameLen)
		{
			NexUtil_FrameDump(s_strDecodeInputAudioPath, a_hCodec->m_eCodecType, a_pFrame, a_nFrameLen);
		}
#endif

		nRet = a_hCodec->m_cfs.ad.Decode(a_pFrame, 
										 a_nFrameLen, 
										 a_pExtraInfo,
										 a_pDest, 
										 a_pnWrittenPCMSize, 
										 a_uDTS, 
										 a_puOutputTime, 
										 a_nFlag, 
										 a_puDecodeResult, 
										 a_hCodec->m_pUserData);

#ifdef _DECODE_OUTPUT_AUDIO_DUMP_
		NexUtil_FrameDump(s_strDecodeOutputAudioPath, a_hCodec->m_eCodecType, a_pDest, *a_pnWrittenPCMSize);
#endif			
		return nRet;

	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderReset(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.ad.Reset(a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult)
{
	NXINT32 nRet = NEXCAL_ERROR_NONE;

	if (!a_hCodec)
	{
		*a_puResult = 0;
		return NEXCAL_ERROR_FAIL;
	}

	if (a_hCodec->m_cfs.ad.GetInfo == NULL)
	{
		switch (a_uIndex)
		{
			case NEXCAL_AUDIO_GETINFO_SAMPLINGRATE:
				*a_puResult = a_hCodec->uSamplingRate;
				break;

			case NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS:
				*a_puResult = a_hCodec->uNumOfChannels;
				break;

			case NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE:
				*a_puResult = a_hCodec->uBitsPerSample;
				break;

			case NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL:
				*a_puResult = a_hCodec->uNumOfSamplesPerChannel;
				break;

			default:
				nRet = NEXCAL_ERROR_INVALID_PARAMETER;
				break;
		}
	}
	else
	{
		return a_hCodec->m_cfs.ad.GetInfo(a_uIndex, a_puResult, a_hCodec->m_pUserData);
	}

	return nRet;
}

/*-----------------------------------------------------------------------------
		Text Decoder Wrapper
-----------------------------------------------------------------------------*/

NEXCAL_API NXINT32 nexCAL_TextDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{
		if (NEXCAL_PROPERTY_GET_DECODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		*a_pqValue = a_hCodec->m_eCodecType;//simple calbody add	
		return a_hCodec->m_cfs.td.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}
	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.td.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderInit(NEXCALCodecHandle a_hCodec, 
										  NEX_CODEC_TYPE a_eCodecType,
										  NXUINT8 *a_pConfiga,
										  NXINT32 a_nConfigLen,
										  NXVOID *a_pInitInfo,
										  NXUINT32 a_uMode,
										  NXUINT32 a_uUserDataType,
										  NXVOID *a_pClientIdentifier)
{
	NXINT32 nRet = NEXCAL_ERROR_FAIL;
	NEXSALFileHandle hFile = 0;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_TEXT, 0,"[%s %d] Start Text Init[%d]\n", _FUNLINE_, a_eCodecType);

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	if (a_pConfiga && 0 < a_nConfigLen)
	{
		if (NULL != (hFile = ((NEXCALRegistrationInfo*)(a_hCodec->m_hCAL))->m_ahDump[NEXCAL_MEDIATYPE_TEXT]))
		{
			nexSAL_FileWrite(hFile, &a_nConfigLen, sizeof(a_nConfigLen));
			nexSAL_FileWrite(hFile, a_pConfiga, a_nConfigLen);
		}
	}

	a_hCodec->m_pUserData = a_pClientIdentifier;

	if (a_hCodec)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_TEXT, 0,"[%s %d] Before Text Init[%d][%p]\n", _FUNLINE_, a_eCodecType, a_hCodec->m_cfs.td.Init);

		if (a_hCodec->m_cfs.td.Init)
		{
			nRet = a_hCodec->m_cfs.td.Init(a_eCodecType,
										   a_pConfiga,
										   a_nConfigLen,
										   a_pInitInfo,
										   a_uMode,
										   a_uUserDataType,
										   &a_hCodec->m_pUserData);
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_TEXT, 0,"[%s %d] After Text Init[%d]\n", _FUNLINE_, nRet);
	}

	if (NEXCAL_ERROR_NONE != nRet)
	{
		//Todo: memory free
	}

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderDeinit(NEXCALCodecHandle a_hCodec)
{
	NXINT32 nRet = NEXCAL_ERROR_NONE;

	if (a_hCodec)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_TEXT, 0,"[%s %d] td.Deinit [%p]\n", _FUNLINE_, a_hCodec->m_cfs.td.Deinit);
		nRet = a_hCodec->m_cfs.td.Deinit(a_hCodec->m_pUserData);
	}

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderDecode(NEXCALCodecHandle a_hCodec, 
											NXVOID* a_pSource, 
											NXVOID* a_pDest, 
											NXVOID *a_pExtraInfo,
											NXUINT32* a_puDecodeResult)
{
	if (a_pSource == NULL)
	{
		NEXCAL_DEL_TDEC_RET(*a_puDecodeResult, NEXCAL_TDEC_DECODING_SUCCESS);
		NEXCAL_SET_TDEC_RET(*a_puDecodeResult, NEXCAL_TDEC_NEXT_FRAME);
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_TEXT, 0,"[%s %d] Text Input Data is NULL...[0x%x|%d]\n", _FUNLINE_, a_pSource, a_pDest);
		return NEXCAL_ERROR_FAIL;
	}

	if (a_hCodec)
	{
		return a_hCodec->m_cfs.td.Decode(a_pSource, 
										 a_pDest, 
										 a_pExtraInfo,
										 a_puDecodeResult, 
										 a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderReset(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.td.Reset(a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_TextDecoderGetInfo(NEXCALCodecHandle a_hCodec, NXUINT32 a_uIndex, NXUINT32 *a_puResult)
{
	NXINT32 nRet = NEXCAL_ERROR_NONE;

	a_hCodec = NULL;
	a_uIndex = 0;
	a_puResult = NULL;

	return nRet;
}

/*-----------------------------------------------------------------------------
		Image Decoder Wrapper
-----------------------------------------------------------------------------*/

NEXCAL_API NXINT32 nexCAL_ImageDecoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{
		if (NEXCAL_PROPERTY_GET_DECODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		return a_hCodec->m_cfs.id.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_IamgeDecoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.id.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_ImageDecoderGetHeader(NEXCALCodecHandle a_hCodec,
												NEX_CODEC_TYPE a_eCodecType, 
												NXCHAR *a_strFileName, 
												NXUINT8 *a_pSrc, 
												NXUINT32 a_uSrcLen, 
												NXINT32 *a_pnWidth, 
												NXINT32 *a_pnHeight, 
												NXINT32 *a_pnPitch, 
												NXUINT32 a_uUserDataType, 
												NXVOID *a_pClientIdentifier)
{
	a_hCodec->m_pUserData = a_pClientIdentifier;

	if (a_hCodec)
	{
		return a_hCodec->m_cfs.id.GetHeader(a_eCodecType, 
											a_strFileName, 
											a_pSrc, 
											a_uSrcLen, 
											a_pnWidth, 
											a_pnHeight, 
											a_pnPitch, 
											a_uUserDataType, 
											&a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_ImageDecoderDecode(NEXCALCodecHandle a_hCodec, 
											 NXCHAR *a_strFileName, 
											 NXUINT8 *a_pSrc, 
											 NXUINT32 a_uSrcLen, 
											 NXINT32 *a_pnWidth, 
											 NXINT32 *a_pnHeight, 
											 NXINT32 *a_pnPitch, 
											 NXUINT8 *a_pBits1, 
											 NXUINT8 *a_pBits2, 
											 NXUINT8 *a_pBits3, 
											 NXUINT32 *a_puDecodeResult)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.id.Decode(a_strFileName, 
										 a_pSrc, 
										 a_uSrcLen, 
										 a_pnWidth, 
										 a_pnHeight, 
										 a_pnPitch, 
										 a_pBits1, 
										 a_pBits2, 
										 a_pBits3, 
										 a_puDecodeResult,
										 a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

/*-----------------------------------------------------------------------------
		Video Encoder Wrapper
-----------------------------------------------------------------------------*/

NEXCAL_API NXINT32 nexCAL_VideoEncoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{
		if (NEXCAL_PROPERTY_GET_ENCODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		return a_hCodec->m_cfs.ve.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.ve.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderInit(NEXCALCodecHandle a_hCodec,
										   NEX_CODEC_TYPE a_eCodecType,
										   NXUINT8 **a_ppConfig,
										   NXINT32 *a_pnConfigLen,
										   NXINT32 a_nQuality,
										   NXINT32 a_nWidth,
										   NXINT32 a_nHeight,
										   NXINT32 a_nPitch,
										   NXINT32 a_nFPS,
										   NXBOOL a_bCBR,
										   NXINT32    a_nBitRate,
										   NXVOID *a_pClientIdentifier)
{
	NXINT32 nRet;

	if (!a_hCodec)
	{
		return NEXCAL_ERROR_FAIL;
	}

	a_hCodec->m_pUserData = a_pClientIdentifier;

	nRet = a_hCodec->m_cfs.ve.Init(a_eCodecType, 
								   a_ppConfig, 
								   a_pnConfigLen, 
								   a_nQuality, 
								   a_nWidth,
								   a_nHeight,
								   a_nPitch,
								   a_nFPS, 
								   a_bCBR, 
								   a_nBitRate, 
								   &a_hCodec->m_pUserData);

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderDeinit(NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ve.Deinit(a_hCodec->m_pUserData);
		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderEncode(NEXCALCodecHandle a_hCodec, 
											 NXUINT8 *a_pData1, 
											 NXUINT8 *a_pData2, 
											 NXUINT8 *a_pData3, 
											 NXUINT32 a_uPTS, 
											 NXUINT32 *a_puEncodeResult)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ve.Encode(a_pData1, 
										 a_pData2, 
										 a_pData3, 
										 a_uPTS, 
										 a_puEncodeResult, 
										 a_hCodec->m_pUserData);
		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderGetOutput(NEXCALCodecHandle a_hCodec, NXUINT8 **a_ppOutData, NXINT32 *a_pnOutLen, NXUINT32 *a_puPTS)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ve.GetOutput(a_ppOutData, 
											a_pnOutLen, 
											a_puPTS, 
											a_hCodec->m_pUserData);
		return nRet;
	}
	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_VideoEncoderSkip(NEXCALCodecHandle a_hCodec, NXUINT8 **a_ppOutData, NXUINT32 *a_pnOutLen)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ve.Skip(a_ppOutData, a_pnOutLen, a_hCodec->m_pUserData);

		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioEncoderGetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 *a_pqValue)
{
	if (a_hCodec)
	{
		if (NEXCAL_PROPERTY_GET_ENCODER_INFO == a_uProperty)
		{
			*a_pqValue = (NXUSIZE)a_hCodec->m_pUserData;
			return NEXCAL_ERROR_NONE;
		}

		return a_hCodec->m_cfs.ae.GetProperty(a_uProperty, a_pqValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioEncoderSetProperty(NEXCALCodecHandle a_hCodec, NXUINT32 a_uProperty, NXINT64 a_qValue)
{
	if (a_hCodec)
	{
		return a_hCodec->m_cfs.ae.SetProperty(a_uProperty, a_qValue, a_hCodec->m_pUserData);
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API  NXINT32 nexCAL_AudioEncoderInit(NEXCALCodecHandle a_hCodec, 
											NEX_CODEC_TYPE a_eCodecType, 
											NXUINT8 **a_ppConfig, 
											NXINT32 *a_pnConfigLen, 
											NXUINT32 a_uSamplingRate, 
											NXUINT32 a_uNumOfChannels, 
											NXINT32 a_nBitrate, 
											NXVOID *a_pClientIdentifier)
{
	NXINT32 nRet;

	if (!a_hCodec)
		return NEXCAL_ERROR_FAIL;

	a_hCodec->m_pUserData = a_pClientIdentifier;

	nRet = a_hCodec->m_cfs.ae.Init(a_eCodecType, 
								   a_ppConfig, 
								   a_pnConfigLen, 
								   a_uSamplingRate, 
								   a_uNumOfChannels, 
								   a_nBitrate, 
								   &a_hCodec->m_pUserData);

	return nRet;
}

NEXCAL_API NXINT32 nexCAL_AudioEncoderDeinit (NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ae.Deinit(a_hCodec->m_pUserData);
		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}



NEXCAL_API  NXINT32 nexCAL_AudioEncoderEncode(NEXCALCodecHandle a_hCodec, 
											  NXUINT8 *a_pData, 
											  NXINT32 a_nLen, 
											  NXUINT8 **a_ppOutData, 
											  NXUINT32 *a_puOutLen, 
											  NXUINT32 *a_puEncodeResult)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ae.Encode(a_pData, 
										 a_nLen, 
										 a_ppOutData, 
										 a_puOutLen, 
										 a_puEncodeResult, 
										 a_hCodec->m_pUserData);

		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NXINT32 nexCAL_AudioEncoderReset (NEXCALCodecHandle a_hCodec)
{
	if (a_hCodec)
	{
		NXINT32 nRet;

		nRet = a_hCodec->m_cfs.ae.Reset(a_hCodec->m_pUserData);

		return nRet;
	}

	return NEXCAL_ERROR_FAIL;
}

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-06-07	Draft.
 ysh(eric)	08-14-2009	Encoder is added.
 ----------------------------------------------------------------------------*/

