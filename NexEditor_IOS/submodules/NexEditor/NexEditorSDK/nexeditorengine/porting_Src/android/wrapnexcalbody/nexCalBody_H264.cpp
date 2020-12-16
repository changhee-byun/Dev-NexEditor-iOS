#include "nexCalBody.h"
#include "nexCAL.h"
#include "nexCalBody_H264.h"
#include "NxAVCDecAPI.h"
#include "NexCodecUtil.h"
#include <stdlib.h>
#include <stdio.h>

#include <android/log.h>

// #define _DUMP_FRAME_
#ifdef _DUMP_FRAME_
#define _DUMP_PATH_ "/sdcard/H264Frame.dmp"
#endif 

typedef struct VideoDecoderInfo
{
	void*			m_hDecoder;
	void*			m_pDivxImg;
	unsigned int 		m_uDTS;
	unsigned int 		m_bIsH264ANNEXB;
	
	int				m_iWidth;
	int				m_iHeight;
	int				m_iPitch;

	unsigned char*	m_pY;
	unsigned char*	m_pU;
	unsigned char*	m_pV;

	unsigned int		m_uiAVCOutPutFormat;
	unsigned int		m_uiAVCGetOutPutMode;

	unsigned int		m_uiFrameCount;
	unsigned int		m_uiTotalProcTime;

	FILE*			m_pFileDump;
} VideoDecoderInfo;

unsigned int g_uiColorFormat	= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;

unsigned int checkIsH264ANNEXB( unsigned char* pData, int iLen ) 
{
	// nexCAL_DebugPrintf("[checkIsH264ANNEXB] iLen=%d", iLen);
	// nexCAL_MemDump(pData, iLen < 10 ? iLen : 10);
	
	if (pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x00 && pData[3] == 0x01)
	{
		nexCAL_DebugPrintf("[H264Body.cpp %d] 4 byte start code. Annex B Type.\n", __LINE__);
		return 1;
	}
	else if(pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x01)
	{
		int nLen = 0;

		// JDKIM 2011/03/25 : nLen is integer type so this value is bigger than 0xEFFFFF it work as minus value.
              //while( nLen+3 < iLen )
              while( (unsigned int)(nLen+4) < (unsigned int)iLen )                                           
              // JDKIM : end   
		{
		    nLen += (pData[nLen] << 24 | pData[nLen+1] << 16 | pData[nLen+2] << 8 | pData[nLen+3]);
		    nLen += 4;
		 }

		nexCAL_DebugPrintf("[H264Body.cpp %d] Detect 3 byte start code. (nLen:%d, iLen:%d)\n", __LINE__, nLen, iLen);
		return (nLen != iLen);
	}
	else
	{
		// nexCAL_DebugPrintf("[AVCDEC %d] RAW Type.\n", __LINE__);
		return 0;
	}
}

unsigned int nexCALBody_Video_AVC_Init(	unsigned int uCodecObjectTypeIndication, 
												unsigned char* pConfig, 
												int iLen, 
												unsigned char* pConfigEnhance, 
												int iEnhLen, 
												void *pInitInfo,
												void *pExtraInfo,
												int uNALHeaderLen4DSI, 
												int* piWidth,
												int* piHeight, 
												int* piPitch, 
												int a_uMode, 
												int a_uUserDataType, 
												void** puUserData )
{
	int retval;
	VideoDecoderInfo*		pVDI		= NULL;
	NxAVCDecStruct*		pDecoder	= NULL;

	if( puUserData == NULL )
	{
		nexCAL_DebugPrintf("[H264Body.cpp, %d] Can't initialize H.264 decoder because puUserData is null",__LINE__);
		return NEXCAL_ERROR_FAIL;
	}
	*puUserData = NULL;

	char *pVersion = NxAVCDecGetVersionInfo(1);
	nexCAL_DebugLogPrintf("[H264Body.cpp %d] NxAVCDec Version (%s) \n", __LINE__, pVersion);
	
	pVDI = (VideoDecoderInfo*)malloc(sizeof(VideoDecoderInfo));
	if(!pVDI)
	{
		nexCAL_DebugPrintf("[H264Body.cpp, %d] nexCALBody_Video_AVC_Init - malloc() Fail\n",__LINE__);
		return NEXCAL_ERROR_FAIL;
	}
	memset(pVDI, 0x00, sizeof(VideoDecoderInfo));	
	
	NxAVCDecStruct * hDecoder = NULL;
	if(pConfig == NULL)
	{
		nexCAL_DebugPrintf("H264Body.cpp nexCALBody_Video_AVC_Init - pConfig is NULL\n",__LINE__);
		return NEXCAL_ERROR_FAIL;
	}

#ifdef _DUMP_FRAME_
	nexCAL_DebugPrintf("[H264Body.cpp %d] NxAVCDec DUMP VERSION \n", __LINE__);

	char strPath[256];
	sprintf(strPath, "/sdcard/h264_%d.dump", nexCAL_GetTickCount());
	
	pVDI->m_pFileDump = fopen(strPath, "wb");
	if( pVDI->m_pFileDump )
	{
		fwrite( pConfig, iLen, 1, pVDI->m_pFileDump);
	}
#endif 	
		
	hDecoder = NxAVCDecInit(1, 1, nex_malloc, nex_free, nex_malloc, nex_free);//by sylee
 
	nexCAL_DebugPrintf("[H264Body.cpp %d] AVCDecConfig(%d:%d) hDecoder(0x%X) \n",  __LINE__, iLen, uNALHeaderLen4DSI, hDecoder);	
	nexCAL_MemDump(pConfig, iLen);

	if ( iLen > 4 && ( (pConfig[0] == '\0' && pConfig[1] == '\0' && pConfig[2] == '\0' && pConfig[3] == '\1') 
		|| (pConfig[0] == '\0' && pConfig[1] == '\0' && pConfig[2] == '\1') ))
	{
		pVDI->m_bIsH264ANNEXB = checkIsH264ANNEXB(pConfig, iLen); 
		//pVDI->m_bIsH264ANNEXB = 1;
	}
	else
	{
		pVDI->m_bIsH264ANNEXB = 0xFF;				// JDKIM 2010/10/04
	}

	nexCAL_DebugPrintf("[H264Body.cpp %d] AnnexB :%d\n",  __LINE__, pVDI->m_bIsH264ANNEXB);	    

	if(pVDI->m_bIsH264ANNEXB != 1)					// JDKIM 2010/10/04
	{
		retval = NxAVCDecConfig(hDecoder, pConfig, uNALHeaderLen4DSI);
		if (retval) 
		{
			NxAVCDecClose ( hDecoder );
			nexCAL_DebugPrintf("[H264Body.cpp %d] Error Video Dec Init %d \n", __LINE__, retval);
			*puUserData = NULL;
			return 1;
		}
	}
	else
	{
		unsigned int uiConsume = 0;
		int uiDecodeFlag = 0;
		retval = NxAVCDecDecodeAnnexBFrame(hDecoder, pConfig, iLen, 0, &uiConsume, &uiDecodeFlag);

		if(retval)
		{
			NxAVCDecClose ( hDecoder );
			nexCAL_DebugPrintf("[H264Body.cpp %d] Error Video Dec Init %d \n", __LINE__, retval);
			*puUserData = NULL;
			return 1;
		}
	}

	pVDI->m_uiAVCOutPutFormat = g_uiColorFormat;
	
	*piWidth 	= hDecoder->width;
	*piHeight 	= hDecoder->height;
	if ( hDecoder->pitch == 0 )
		*piPitch = hDecoder->width;
	else
		*piPitch = hDecoder->pitch;

	pVDI->m_iWidth		= hDecoder->width;
	pVDI->m_iHeight		= hDecoder->height;;
	pVDI->m_iPitch		= hDecoder->pitch;

	if(	pVDI->m_uiAVCOutPutFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 ||
		pVDI->m_uiAVCOutPutFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420 )
	{
		*piPitch				= pVDI->m_iWidth;
	}

	pVDI->m_hDecoder = (void *)hDecoder;

	*puUserData = (void*)pVDI;

	pVDI->m_uiFrameCount = 0;
	pVDI->m_uiTotalProcTime = 0;

	nexCAL_DebugPrintf("[H264Body.cpp %d] H264 Init Done. w:%d, h:%d, p:%d  hDecoder:0x%X, 0x%x\n", __LINE__, *piWidth, *piHeight, *piPitch, hDecoder, pVDI);
	return retval;
}

unsigned int nexCALBody_Video_AVC_Close( void* uUserData )
{
	nexCAL_DebugPrintf("[H264Body.cpp %d] nexCALBody_Video_AVC_Close\n", __LINE__);
	
	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)uUserData;
	nexCAL_DebugPrintf("[H264Body.cpp %d]   ==> pVDI : 0x%X\n", __LINE__, pVDI);
	
	if(pVDI)
	{
		if( pVDI->m_pFileDump )
		{
			fclose(pVDI->m_pFileDump);
			pVDI->m_pFileDump = NULL;
		}
		
		if( pVDI->m_uiFrameCount > 0 )
			nexCAL_DebugPrintf("[H264Body.cpp %d]  ==> Ave AVC Dec time : %d msec (%d / %d)", __LINE__, 
				pVDI->m_uiTotalProcTime/pVDI->m_uiFrameCount, pVDI->m_uiTotalProcTime, pVDI->m_uiFrameCount);
	
		NxAVCDecStruct* pDecoder = (NxAVCDecStruct*)pVDI->m_hDecoder;

		nexCAL_DebugPrintf("[H264Body.cpp %d]  ==> pDecoder : 0x%X\n", __LINE__, pDecoder);
		
		NxAVCDecClose( pDecoder );
		nexCAL_DebugPrintf("[H264Body.cpp %d]  ==> pVDI : 0x%X\n", __LINE__,  pVDI);
		nexCAL_MemFree(pVDI);
	}

	nexCAL_DebugPrintf("[H264Body.cpp %d] ==> Ave AVC Dec Close End", __LINE__);
	return 0;
}

unsigned int nexCALBody_Video_AVC_Dec(	unsigned char* pData, 
												int iLen, 
												void *pExtraInfo,
												unsigned int uDTS, 
												unsigned int uPTS, 
												int nFlag, 
												unsigned int* puDecodeResult, 
												void* uUserData )
{
	int retval = 0, flagDisplay;
	
	VideoDecoderInfo* pVDI 	= (VideoDecoderInfo*)uUserData;

	//nexCAL_DebugPrintf("[nexCALBody_Video_AVC_Dec] VideoDecInfo(0x%X), iLen(%d), uDTS(%d)\n",  pVDI, iLen, uDTS);

	if( pVDI->m_pFileDump )
	{
		fwrite( pData, iLen, 1, pVDI->m_pFileDump);
	}

	nexCAL_DebugPrintf("[H264Body.cpp %d] flag(%d)", __LINE__, nFlag);
	if( (nFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM ) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
	{
		*puDecodeResult = 0;
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);
		nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decoding end", __LINE__);
		return 0;
	}
	
	int  			bDisplay;
	unsigned int	uConsumed;
	unsigned int	uTick = 0;//nexCAL_GetTickCount();
	pVDI->m_uDTS = uPTS;

	unsigned int uiEndTime = 0;
	unsigned int uiStartTime = 0;
	if(pVDI)
	{
		//nexCAL_MemDump(pData, 20);

		uiEndTime = 0;
		uiStartTime = nexCAL_GetTickCount();

		// JDKIM 2010/10/04
		//if(pVDI->m_bIsH264ANNEXB)
		if(pVDI->m_bIsH264ANNEXB == 0xFF)
		{
        		pVDI->m_bIsH264ANNEXB = checkIsH264ANNEXB(pData, iLen);
		}

		if(pVDI->m_bIsH264ANNEXB == 1)
		// JDKIM : end
		{
			retval = NxAVCDecDecodeAnnexBFrame((NxAVCDecStruct*)pVDI->m_hDecoder, pData, iLen, (unsigned long)uPTS, &uConsumed, &bDisplay);
		}
		else
		{
			retval = NxAVCDecDecodeFrame((NxAVCDecStruct*)pVDI->m_hDecoder, pData, iLen, (unsigned long)uPTS, &bDisplay);
		}

		nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decodend(dts:%d pts:%d)", __LINE__, uDTS, uPTS);
		

	}

	if ( bDisplay > 0 )
	{
		if( pVDI->m_uiAVCGetOutPutMode )
		{
			NxAVCDecGetOutputFrame((NxAVCDecStruct*)pVDI->m_hDecoder, &pVDI->m_pY, &pVDI->m_pU, &pVDI->m_pV);
			nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decoding set display flag(%d 0x%x 0x%x 0x%x)", __LINE__, bDisplay,
				pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
		}
		else
		{
			uiEndTime = nexCAL_GetTickCount();

			pVDI->m_uiFrameCount++;
			pVDI->m_uiTotalProcTime += (uiEndTime - uiStartTime);	

			NxAVCDecGetOutputFrameFromDPB((NxAVCDecStruct*)pVDI->m_hDecoder, 0, (unsigned int*)&pVDI->m_uDTS, &pVDI->m_pY, &pVDI->m_pU, &pVDI->m_pV);
			nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decoding set display flag(%d) pts(%.8d) 0x%x 0x%x 0x%x)", __LINE__, bDisplay, pVDI->m_uDTS,
				pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
		}
	}
	else
	{
		pVDI->m_pY = NULL;
		pVDI->m_pU = NULL;
		pVDI->m_pV = NULL;
		nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decoding not set display flag(%d)", __LINE__, bDisplay);
	}

	*puDecodeResult = 0;

	NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
	if( retval )
	{
		nexCAL_DebugPrintf("[H264Body.cpp %d] h264 decoding failed(%d)", __LINE__, retval);
		pVDI->m_bIsH264ANNEXB = 0xFF;
		NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
	}

	if( bDisplay )
	{
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
	}
	
	//nexCAL_DebugPrintf("[nexCALBody_Video_AVC_Dec] retval(%d), displayFlag(%d), DecRec(%d)\n", retval, bDisplay, *puDecodeResult);
	
	return 0;
}

unsigned int nexCALBody_Video_AVC_GetOutput( unsigned char** ppY, unsigned char** ppU, unsigned char** ppV, unsigned int *puPTS, void* uUserData )
{
	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)uUserData;

	//nexCAL_DebugPrintf("[nexCALBody_Video_AVC_GetOutput] YUV (0x%X, 0x%X, 0x%X)\n", pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
	
	if(pVDI)
	{
		if( pVDI->m_uiAVCOutPutFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 )
		{
			if( *ppY == NULL || pVDI->m_pY == NULL || pVDI->m_pU == NULL || pVDI->m_pV == NULL )
			{
				nexCAL_DebugPrintf("[H264Body.cpp %d] Get output failed (0x%X, 0x%X, 0x%X)", __LINE__, pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
				return 1;
			}
			
			unsigned char* pSrcY	= pVDI->m_pY;
			unsigned char* pSrcU	= pVDI->m_pU;
			unsigned char* pSrcV	= pVDI->m_pV;
			
			unsigned char* pDstY = *ppY ;
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

			*puPTS = pVDI->m_uDTS;

			pVDI->m_pY=NULL;
			pVDI->m_pU=NULL;
			pVDI->m_pV=NULL;
			return 0;
		}

		if( pVDI->m_uiAVCOutPutFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420 )
		{
			if( *ppY == NULL || pVDI->m_pY == NULL || pVDI->m_pU == NULL || pVDI->m_pV == NULL )
			{
				nexCAL_DebugPrintf("[H264Body.cpp %d] Get output failed (0x%X, 0x%X, 0x%X)", __LINE__, pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
				return 1;
			}
			
			unsigned char* pSrcY	= pVDI->m_pY;
			unsigned char* pSrcU	= pVDI->m_pU;
			unsigned char* pSrcV	= pVDI->m_pV;
			
			unsigned char* pDstY = *ppY ;
			unsigned char* pDstUV = pDstY + (pVDI->m_iWidth*pVDI->m_iHeight);

			for( int y = 0; y < pVDI->m_iHeight; y++)
			{
				memcpy(pDstY, pSrcY, pVDI->m_iWidth);
				pDstY += pVDI->m_iWidth;
				pSrcY += pVDI->m_iPitch;
			}

			for( int y = 0; y < pVDI->m_iHeight /2; y++)
			{
				memcpy(pDstUV, pSrcV, pVDI->m_iWidth / 2);
				pDstUV += pVDI->m_iWidth / 2;
				pSrcV += pVDI->m_iPitch / 2;
			}
			for( int y = 0; y < pVDI->m_iHeight /2; y++)
			{
				memcpy(pDstUV, pSrcU, pVDI->m_iWidth / 2);
				pDstUV += pVDI->m_iWidth / 2;
				pSrcU += pVDI->m_iPitch / 2;
			}
			*puPTS = pVDI->m_uDTS;

			pVDI->m_pY=NULL;
			pVDI->m_pU=NULL;
			pVDI->m_pV=NULL;
			return 0;
		}		
		
		if ( pVDI->m_pY && pVDI->m_pY && pVDI->m_pV )
		{
			*ppY=pVDI->m_pY;
			*ppU=pVDI->m_pU;
			*ppV=pVDI->m_pV;
			pVDI->m_pY=0;
			pVDI->m_pU=0;
			pVDI->m_pV=0;
		}
		else
		{
			nexCAL_DebugPrintf("[H264Body.cpp %d] Get output failed (0x%X, 0x%X, 0x%X)", __LINE__, pVDI->m_pY, pVDI->m_pU, pVDI->m_pV);
			*ppY=NULL;
			*ppU=NULL;
			*ppV=NULL;
		}
	}
	*puPTS = pVDI->m_uDTS;	
	return 0;
}

unsigned int nexCALBody_Video_AVC_Reset( void* uUserData )
{
	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)uUserData;
	if( pVDI == NULL ) return 1;
	NxAVCDecResetDPB((NxAVCDecStruct*)pVDI->m_hDecoder);
	nexCAL_DebugPrintf("[H264Body.cpp %d] nexCALBody_Video_AVC_Reset", __LINE__ );
	return 0;
}

unsigned int nexCALBody_Video_AVC_SetProperty( unsigned int uProperty, NXINT64 uValue, void* uUserData )
{
	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)uUserData;
	nexCAL_DebugPrintf("[H264Body.cpp %d] nexCALBody_Video_AVC_SetProperty %d, %d, 0x%x", __LINE__,  uProperty, uValue, uUserData);
	if( pVDI == NULL )
	{
		if( uProperty == NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE)
		{
			g_uiColorFormat = (unsigned int)uValue;
			return 0;
		}
		return nexCALBody_Video_SetProperty( uProperty, uValue, uUserData);
	}
	
	switch ( uProperty )
	{
 	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
		pVDI->m_uiAVCOutPutFormat = (unsigned int)uValue;
		break;	

 	case NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE:
		pVDI->m_uiAVCGetOutPutMode = (unsigned int)uValue;
		break;
 	default:
		return nexCALBody_Video_SetProperty( uProperty, uValue, uUserData);
	};
	return 0;
}

unsigned int nexCALBody_Video_AVC_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* uUserData )
{
	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*puValue = (NXINT64)NEXCAL_PROPERTY_ANSWERIS_YES;
		break;

	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
		*puValue = (NXINT64)g_uiColorFormat;
		break;
		
	case NEXCAL_PROPERTY_INITPS_FORMAT :
		*puValue = (NXINT64)NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
		break;

	case NEXCAL_PROPERTY_BYTESTREAM_FORMAT :
		*puValue = (NXINT64)NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
		break;

	case NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM:
		*puValue = (NXINT64)NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_NOT_OK;
		break;

	case NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT:
		*puValue = (NXINT64)NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES;
		break;

	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
		*puValue = (NXINT64)eNEX_AVC_PROFILE_HIGH;
		break;

	case NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER:
		*puValue = (NXINT64)NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_NOT_OK;
		break;

	case NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME:
		*puValue = (NXINT64)NEXCAL_PROPERTY_AVC_SKIP_NOTREFIDC_BFRAME_OK;
		break;

	default:
		return nexCALBody_Video_GetProperty( uProperty, puValue, uUserData );
	}		

	nexCAL_DebugPrintf("[H264Body.cpp %d] Video_GetProperty(%d:%d)", __LINE__, uProperty, *puValue);
	return 0;
}

