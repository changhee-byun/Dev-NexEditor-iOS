#include "nexCalBody.h"
#include "NexCAL.h"
#include "NxMPEG4ASPVideoDecAPI.h"
#include "nexCalBody_DIVX.h"
#include "NexMediaDef.h"

#include <stdlib.h>
#include <stdio.h>
//namespace android {


unsigned char g_bActiveDIVX = TRUE;

// extern unsigned char *g_pY;
// extern unsigned char *g_pU;
// extern unsigned char *g_pV;

static int g_iCnt = 0;
static unsigned int g_uiDTS = 0;

static unsigned int g_uiFrameCount = 0;
static unsigned int g_uiTotalProcTime = 0;

// unsigned int g_uiDivxOutputFormat = NEXCAL_PROPERTY_OUTPUTBUFFER_TYPE_SEPARATED_YUV420;
unsigned int g_uiDivxOutputColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
 
typedef struct DIVXDecoderInfo
{
	void*		m_hDecoder;
	void*		m_pDivxImg;
	unsigned int 	m_uDTS;
	unsigned int 	m_uCount;
	unsigned int	m_uiCodecObjectType;

	int			m_iWidth;
	int			m_iHeight;
	int			m_iPitch;

	unsigned char*	m_pY;
	unsigned char*	m_pU;
	unsigned char*	m_pV;

	unsigned int m_uiOutPutFormat;
	unsigned int m_uiGetOutPutMode;
	
} DIVXDecoderInfo;

//#define _YUV_DUMP_
#ifdef _YUV_DUMP_
	FILE* g_YUVDump = NULL;

	unsigned int g_uiWidth = 0;
	unsigned int g_uiHeight = 0;
	unsigned int g_uiPitch = 0;
#endif 


//#define _FRAME_DUMP_
//#define _FRAME_DUMP_
#ifdef _FRAME_DUMP_
#define _DUMP_PATH_ "/sdcard/DivixDump.dmp"
#endif 

// #define SUPPORT_DIVX311

unsigned int nexCALBody_Video_DIVX_Init(	unsigned int uCodecObjectTypeIndication,
												unsigned char* pConfig,
												int iLen,
												unsigned char* pConfigEnhance,
												int iEnhLen,
												void *pInitInfo,
												void *pExtraInfo,
												int iNALHeaderLengthSize,
												int* piWidth,
												int* piHeight,
												int* piPitch,
												int a_uMode, 
												int a_uUserDataType, 
												void** puUserData )
{
//	PNEXVDECUsrData pVideoUsrData;
	DIVXDecoderInfo* pVDI = NULL;	
	int retval = 0;

	if(g_bActiveDIVX == FALSE)
	{
		nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]Can't initialize DIVX decoder.\n",__LINE__, __FUNCTION__);
		*puUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}
	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]\n", __LINE__, __FUNCTION__);

	
	pVDI = (DIVXDecoderInfo *)nexCAL_MemAlloc( sizeof(DIVXDecoderInfo) );
	if ( !pVDI )
	{
		nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Error Memory Allocation \n", __LINE__, __FUNCTION__ );
		*puUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

#ifdef _FRAME_DUMP_
	nexSAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] DIVX DUMP ", __LINE__, __FUNCTION__);
	nexCALBody_Dump_VideoInit(_DUMP_PATH_, iLen, pConfig);
#endif 

	memset( pVDI, 0x00, sizeof(DIVXDecoderInfo) );

	pVDI->m_uiCodecObjectType = uCodecObjectTypeIndication;

	if(uCodecObjectTypeIndication != eNEX_CODEC_V_MSMPEG4V3)
	{
		NxMPEG4ASPVideoDecStruct	*hCodec = NULL;

		char *pVersion = NxMPEG4ASPVideoDecGetVersionInfo(1);
		nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] NxMPEG4ASPVideoDec Version (%s) \n", __LINE__, __FUNCTION__, pVersion);

		
		hCodec = NxMPEG4ASPVideoDecInit();

		if ( hCodec == NULL ) {
			nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Error Memory Allocation \n", __LINE__ , __FUNCTION__);
			nexCAL_MemFree( pVDI );
			*puUserData = NULL;
			return 1;
		}

		if ( pConfig )
		{
			retval = NxMPEG4ASPVideoDecConfig(hCodec, pConfig, iLen, CODEC_TYPE_MPEG4H263);
			if (retval)
			{
				nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Error NxMPEG4ASPVideoDecConfig %d \n", __LINE__, __FUNCTION__,  retval );
				NxMPEG4ASPVideoDecClose(hCodec);
				nexCAL_MemFree( pVDI );
				nexCAL_MemDump(pConfig, iLen);
				*puUserData = NULL;
				return NEXCAL_ERROR_CORRUPTED_FRAME;
			}
		}
		else if ( pConfigEnhance )
		{
			retval = NxMPEG4ASPVideoDecConfig(hCodec, pConfigEnhance, iEnhLen, CODEC_TYPE_MPEG4H263);
			if (retval)
			{
				retval = NxMPEG4ASPVideoDecGuessVOL(hCodec, pConfigEnhance, iEnhLen, *piWidth, *piHeight);
				if ( retval )
				{
					nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Error NxMPEG4ASPVideoDecGuessVOL %d \n", __LINE__, __FUNCTION__, retval );
					NxMPEG4ASPVideoDecClose(hCodec);
					nexCAL_MemFree( pVDI );
					*puUserData = NULL;
					return NEXCAL_ERROR_CORRUPTED_FRAME;
				}
			}
		}
		else
		{
			nexCAL_DebugPrintf( "[nexCalBody_DIVX.cpp %d][%s] No Config and EnhanceConfig Data. nexCALBODY_MPEG4V_Init() fail.\n", __LINE__, __FUNCTION__);
			nexCAL_MemFree( pVDI );
			*puUserData = NULL;
			return NEXCAL_ERROR_INVALID_PARAMETER;
		}
		*piWidth = hCodec->width;
		*piHeight = hCodec->height;
		if ( hCodec->pitch == 0 )
			*piPitch = hCodec->width;
		else
			*piPitch = hCodec->pitch;

		pVDI->m_hDecoder = (void *)hCodec;
	}
#ifdef SUPPORT_DIVX311
	else
	{
		NxDivX311VideoDecStruct*	hCodec;

		char *pVersion = NxdivX311DecGetVersionInfo(1);
		nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] NxdivX311Dec Version (%s) \n", __LINE__, __FUNCTION__, pVersion);
		
		hCodec = NxDivX311VideoDecInit(*piWidth, *piHeight);
		if(hCodec)
		{
			*piWidth	= hCodec->width;
			*piHeight	= hCodec->height;
			if ( hCodec->pitch == 0 )
				*piPitch = hCodec->width;
			else
				*piPitch = hCodec->pitch;
		}
		else
		{
			*piWidth	= 0;
			*piHeight	= 0;
			*piPitch 		= 0;
		}

		pVDI->m_hDecoder = (void *)hCodec;
	}
#endif	// 0

	g_uiFrameCount = 0;
	g_uiTotalProcTime = 0;

	if( g_uiDivxOutputColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 )
	{
		pVDI->m_iWidth		= *piWidth;
		pVDI->m_iHeight		= *piHeight;
		pVDI->m_iPitch		= *piPitch;
		*piPitch				= pVDI->m_iWidth;
	}

	*puUserData = (void*)pVDI;

#ifdef _YUV_DUMP_
	g_uiWidth = *piWidth;
	g_uiHeight = *piHeight;
	g_uiPitch = *piPitch;
	g_YUVDump = fopen("/sdcard/yuvdump.dmp", "wb");
#endif 

	return retval;
}

unsigned int nexCALBody_Video_DIVX_Close( void* uUserData )
{
	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]nexCALBody_Video_DIVX_Close\n", __LINE__, __FUNCTION__);

#ifdef _YUV_DUMP_

	if( g_YUVDump != NULL)
		fclose(g_YUVDump);

#endif 

#ifdef _FRAME_DUMP_

	if( g_FrameDump != NULL)
		fclose(g_FrameDump);
#endif 
		
	DIVXDecoderInfo* pVDI = (DIVXDecoderInfo*)uUserData;
	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]  ==> pVDI : 0x%X \n", __LINE__, __FUNCTION__, pVDI);
	
	if(pVDI)
	{
		if(pVDI->m_uiCodecObjectType != eNEX_CODEC_V_MSMPEG4V3)
		{
			NxMPEG4ASPVideoDecStruct* pDecoder = (NxMPEG4ASPVideoDecStruct*)pVDI->m_hDecoder;

			nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]  ==> pDecoder : 0x%X\n",__LINE__, __FUNCTION__,  pDecoder);
			NxMPEG4ASPVideoDecClose( pDecoder);
		}
#ifdef SUPPORT_DIVX311		
		else
		{
			NxDivX311VideoDecStruct*	hCodec = (NxDivX311VideoDecStruct *)pVDI->m_hDecoder;
			NxDivX311VideoDecClose(hCodec);
		}
#endif	// SUPPORT_DIVX311		
		nexCAL_MemFree(pVDI);
	}

	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] ==> Ave DIVX Dec Close", __LINE__, __FUNCTION__);
	if( g_uiFrameCount > 0 )
		nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] ==> Ave DIVX Dec time : %d msec (%d / %d)", __LINE__, __FUNCTION__,  g_uiTotalProcTime/g_uiFrameCount, g_uiTotalProcTime, g_uiFrameCount);
	return 0;
}

unsigned int nexCALBody_Video_DIVX_Dec( unsigned char* pData, int iLen, void *pExtraInfo, unsigned int uDTS, unsigned int uPTS, int iEnhancement, 
										unsigned int* puDecodeResult, void* uUserData )
{
	DIVXDecoderInfo* pVDI 	= (DIVXDecoderInfo*)uUserData;
	
	int retval = 0;

	if ( pVDI )
	{
		
		int iPos = 0;
		long uConsumedBytes;
		unsigned int	uTick, uTime;

		unsigned int uiEndTime = 0;
		unsigned int uiStartTime = nexCAL_GetTickCount();
		
		if ( pData == NULL )
		{
			nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Frame data is NULL \n", __LINE__, __FUNCTION__ );
			NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
			retval = 0;
		}
		else
		{

#ifdef _FRAME_DUMP_
	nexCALBody_Dump_Video(_DUMP_PATH_, iLen, uDTS, pData);
#endif 		
			pVDI->m_uDTS = uPTS;
			
			if(pVDI->m_uiCodecObjectType != eNEX_CODEC_V_MSMPEG4V3)
			{
				NxMPEG4ASPVideoDecStruct *hCodec = (NxMPEG4ASPVideoDecStruct *)pVDI->m_hDecoder;
				
				if ( pVDI->m_uCount == 0)
				{
					retval = NxMPEG4ASPVideoDecDecode(hCodec, pData, iLen, 0, uDTS);
					//nexCAL_TraceCat(NEXCAL_TRACE_CATEGORY_E_VIDEO, 0, "[ASPDEC %d]\t <<< One More ASPDEC Decode [0x%X|%7u|%7u] IDR[%d]>>>\n", __LINE__, pData, iLen, uDTS, nexCALTools_IsIDRFrame(NEXOTI_MPEG4V, pData, iLen, 4));
				}

				
				
				//nexCAL_TraceCat(NEXCAL_TRACE_CATEGORY_E_VIDEO, 0, "[ASPDEC %d]\t <<< ASPDEC Decode [0x%X|%7u|%7u] >>>\n", __LINE__, pData, iLen, uDTS);
				while (1)
				{
					pVDI->m_uCount++;
					uTick=nexCAL_GetTickCount();
					retval = NxMPEG4ASPVideoDecDecodeStream(hCodec, pData+iPos, iLen-iPos, 0, uDTS, (int*)&uConsumedBytes);
					uTime = nexCAL_GetTickCount()-uTick;

					if ( retval != 0 )
					{
						nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] Video Decode(0x%x) Error(%d). [Video:%u], Consumed[%u/%u]\n",
										__LINE__, __FUNCTION__,
										pData+iPos,
										retval,
										uDTS,
										uConsumedBytes,
										iLen);
						NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
						NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
						retval = 1;

						pVDI->m_pY = NULL;
						pVDI->m_pU = NULL;
						pVDI->m_pV = NULL;
					}
					else
					{
						unsigned int CTS;
						unsigned int nRet;

						nRet = NxMPEG4ASPVideoDecGetOutputFrame(hCodec,  &pVDI->m_pY, &pVDI->m_pU, &pVDI->m_pV, &CTS, 0);
						nRet &= 0xF;
						//*puDecodeResult = 0;
						if ( nRet == FRAME_STATUS_OK || nRet == FRAME_STATUS_AFFECTED )	// FRAME_STATUS_AFFECTED 1은 덜깨진 화면까지 display, FRAME_STATUS_ERROR은 많이 깨진것 까지 display, FRAME_STATUS_NO_IMAGE은 절대 display 하면 안됨
						{
							NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
							NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
							NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
						}
						else
						{
							NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
							NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
						}

						retval = 0;
						/*
						nexCAL_DebugPrintf("[ASPDEC.c %d] Video Decode(0x%x) Success. [Video:%u], Consumed[%u/%u]\n",
										__LINE__,
										pData+iPos,
										uDTS,
										uConsumedBytes,
										iLen);
						*/
						uiEndTime = nexCAL_GetTickCount();

						
					}

					iPos+=uConsumedBytes;
					if ( iPos + 4 >= iLen ) // 4 => Start Code. 4보다 작는 경우는 Garbage Data
						break;
					else
					{
						//nexCAL_DebugPrintf("[ASPDEC.c %d] Video(%5d) : Second Decode(%7d, %7d)\n", __LINE__, uDTS, iLen-iPos, iLen);
					}
				}	
			}
#ifdef SUPPORT_DIVX311			
			else
			{
				NxDivX311VideoDecStruct*	hCodec = (NxDivX311VideoDecStruct *)pVDI->m_hDecoder;
				
				retval = NxDivX311VideoDecDecode(hCodec, pData, iLen);
				if( retval == FRAME_STATUS_OK)
				{
					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
					NxDivX311VideoDecGetOutputFrame(hCodec, &pVDI->m_pY, &pVDI->m_pU, &pVDI->m_pV);

					//nexCAL_DebugPrintf("[ASPDEC.c %d] NxDivX311VideoDecDecode Success(%d) \r\n", __LINE__, *puDecodeResult);
				}
				else
				{
					uDTS = 0;
					NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
					nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s]  NxDivX311VideoDecDecode failed(%d) \r\n", __LINE__, __FUNCTION__, retval);
				}
			}
#endif	// SUPPORT_DIVX311			

			g_uiFrameCount++;
			g_uiTotalProcTime += (uiEndTime - uiStartTime);	
		}
	}

#ifdef _YUV_DUMP_

	if( g_YUVDump)
	{
		fwrite(pVDI->m_pY, g_uiPitch*g_uiHeight , 1, g_YUVDump);
		fwrite(pVDI->m_pU, g_uiPitch*g_uiHeight/4, 1, g_YUVDump);
		fwrite(pVDI->m_pV, g_uiPitch*g_uiHeight/4, 1, g_YUVDump);
	}

#endif

#ifdef _FRAME_DUMP_
	if( g_FrameDump)
	{
		fwrite(&iLen, 1, sizeof(unsigned int), g_FrameDump);
		fwrite(pData, 1, iLen, g_FrameDump);
	}
#endif 


#ifdef _DUMP_VIDEO_INPUT_DUMP_

	if ( 	NEXCAL_CHECK_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME) )
		nexCALBody_DumpDecode(NEXCALBODY_VIDEO_INPUT, pData, iLen, uDTS);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	if ( NEXCAL_CHECK_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST) )
		nexCALBody_VIdeoOutputDump(NEXCALBODY_VIDEO_YUV420, pVideoUsrData->m_pY, pVideoUsrData->m_pU, pVideoUsrData->m_pV, pVideoUsrData->m_uPitch, pVideoUsrData->m_uHeight, pVideoUsrData->m_uCount);
#endif

	return retval;
}

//extern short g_pcmbuffer[];
//static char g_tempbuffer[1024*124];

unsigned int nexCALBody_Video_DIVX_GetOutput( unsigned char** ppBits1, unsigned char** ppBits2, unsigned char** ppBits3, unsigned int *pDTS, void* uUserData )
{	
	DIVXDecoderInfo* pVDI = (DIVXDecoderInfo*)uUserData;

	if ( pVDI )
	{
		if( g_uiDivxOutputColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 )
		{
			if( *ppBits1 == NULL || pVDI->m_pY == NULL || pVDI->m_pU == NULL || pVDI->m_pV == NULL )
			{
				return 1;
			}
			
			unsigned char* pSrcY	= pVDI->m_pY;
			unsigned char* pSrcU	= pVDI->m_pU;
			unsigned char* pSrcV	= pVDI->m_pV;
			
			unsigned char* pDstY = *ppBits1 ;
			unsigned char* pDstUV = pDstY + (pVDI->m_iWidth*pVDI->m_iHeight);

			for( int y = 0; y < pVDI->m_iHeight; y++)
			{
				memcpy(pDstY, pSrcY, pVDI->m_iWidth);
				pDstY += pVDI->m_iWidth;
				pSrcY += pVDI->m_iPitch;
			}

			for( int y = 0; y < pVDI->m_iHeight / 2; y++)
			{
				for( int x = 0; x < pVDI->m_iWidth / 2; x++)
				{
					*(pDstUV)	= *(pSrcU + x);
					pDstUV++;
					*(pDstUV)	= *(pSrcV + x);
					pDstUV++;
				}
				pSrcU += pVDI->m_iPitch / 2;
				pSrcV += pVDI->m_iPitch / 2;
			}

			*pDTS = pVDI->m_uDTS;

			pVDI->m_pY=NULL;
			pVDI->m_pU=NULL;
			pVDI->m_pV=NULL;
			return 0;
		}		
	
		if ( pVDI->m_pY && pVDI->m_pY && pVDI->m_pV )
		{
			*ppBits1=pVDI->m_pY;
			*ppBits2=pVDI->m_pU;
			*ppBits3=pVDI->m_pV;
			pVDI->m_pY=0;
			pVDI->m_pU=0;
			pVDI->m_pV=0;
		}
		else
		{
			*ppBits1=NULL;
			*ppBits2=NULL;
			*ppBits3=NULL;
		}
	}
	else
	{
		*ppBits1=NULL;
			*ppBits2=NULL;
			*ppBits3=NULL;
	}

	return 0;
}

unsigned int nexCALBody_Video_DIVX_Reset( void* uUserData )
{
	nexCAL_DebugPrintf("[nexCalBody_DIVX %d][%s]\n", __LINE__ , __FUNCTION__);
	return 0;
}

unsigned int nexCALBody_Video_DIVX_GetProperty( unsigned int uProperty, NXINT64* puValue, void* uUserData )
{
	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
		*puValue = (NXINT64)g_uiDivxOutputColorFormat;
		break;
	case NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_VOP;
		break;
	case NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME:
		*puValue = (NXINT64)NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME_NOT_OK;
		break;
	case NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_YES;
		break;
	case NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE_YES;
		break;
	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_NO;
		break;
	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_NO;
		break;
	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_NO;
		break;
	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL:
		*puValue = (NXINT64)NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_NO;
		break;
	default:
		return nexCALBody_Video_GetProperty( uProperty, puValue, uUserData );
	}
	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s](%d:%d)", __LINE__, __FUNCTION__, uProperty, *puValue);
	return 0;
}


unsigned int nexCALBody_Video_DIVX_SetProperty( unsigned int uProperty, NXINT64 uValue, void* uUserData )
{
	DIVXDecoderInfo* pVDI = (DIVXDecoderInfo*)uUserData;
	nexCAL_DebugPrintf("[nexCalBody_DIVX.cpp %d][%s] SetProperty(%d:%d)", __LINE__, __FUNCTION__, uProperty, uValue);
	
	if( pVDI == NULL )
	{
		if( uProperty == NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE)
		{
			g_uiDivxOutputColorFormat = (unsigned int)uValue;
			return 0;
		}
		return nexCALBody_Video_SetProperty( uProperty, uValue, uUserData);
	}
	
	switch ( uProperty )
	{
 	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
		pVDI->m_uiOutPutFormat = (unsigned int)uValue;
		break;	

 	case NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE:
		pVDI->m_uiGetOutPutMode = (unsigned int)uValue;
 	default:
		return nexCALBody_Video_SetProperty( uProperty, uValue, uUserData);
	};
	return 0;
}


