#include "NexCAL.h"
//#include "NxMPEG2DecAPI.h"
#include "NexSAL_Com.h"
#include "NexSAL_Internal.h"
#include <string.h>

#define	NEXCALBODY_MPEG2_REG_KEY			0x470a7f20

//namespace android { 

typedef struct VideoDecoderInfo
{
	void*	m_hDecoder;
	
	unsigned int m_uDTS;
	unsigned int m_uPTS;
	unsigned int m_uWidth;
	unsigned int m_uHeight;
	unsigned int m_uPitch;
	int m_uCount;
	
	unsigned char *m_pY;
	unsigned char *m_pU;
	unsigned char *m_pV;	
	unsigned int m_uiDecodeFrameCount ;
	unsigned int m_uiTotalDecodeProcTime ;
} VideoDecoderInfo;

NXINT32 nexCALBody_Video_MPEG2_Init( NEX_CODEC_TYPE eCodecType, NXUINT8* pConfig, NXINT32 iLen, 
									   NXUINT8* pConfigEnhance, NXINT32 iEnhLen, NXVOID *pInitInfo, NXVOID *pExtraInfo, NXINT32 iNALHeaderLengthSize, NXINT32* piWidth, 
									   NXINT32* piHeight, NXINT32* piPitch, NXUINT32 uMode, NXUINT32 uUserDataType,  NXVOID **ppUserData )
{
//	VideoDecoderInfo* pVDI = NULL;	
//	NxMPEG2VideoDecStruct* hMPEG2Dec = NULL;
//	NXUINT8* pTmpConfig = pConfig;
//	NXINT32 retval;
//	
//	pVDI = (VideoDecoderInfo*)nexSAL_MemAlloc(sizeof(VideoDecoderInfo));
//	if(!pVDI)
//	{
//		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] nexCALBody_Video_MPEG2_Init - malloc() Fail\n",__LINE__);
//		return NEXCAL_ERROR_FAIL;
//	}
//	memset(pVDI, 0x00, sizeof(VideoDecoderInfo));
//	
//	hMPEG2Dec = NxMPEG2VideoDecInit();
//
//	retval = NxMPEG2VideoDecConfig(hMPEG2Dec, pTmpConfig, iLen);
//	if (retval) 
//	{
//		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] Error Video Dec Init %d \n", __LINE__, retval );
//		NxMPEG2VideoDecClose(hMPEG2Dec);
//		*ppUserData = NULL;
//		return 1;
//	}
//
//	*piWidth = hMPEG2Dec->width;
//	*piHeight = hMPEG2Dec->height;
//	if ( hMPEG2Dec->pitch == 0 )
//		*piPitch = hMPEG2Dec->width;
//	else
//		*piPitch = hMPEG2Dec->pitch;
//
//	pVDI->m_uWidth = *piWidth;
//	pVDI->m_uHeight = *piHeight;
//	pVDI->m_uPitch = *piPitch;
//	pVDI->m_hDecoder = (void*)hMPEG2Dec;
//
//	*ppUserData = (NXVOID *)pVDI;
//
//	pVDI->m_uiDecodeFrameCount = 0;
//	pVDI->m_uiTotalDecodeProcTime = 0;
//
//#ifdef _DUMP_VIDEO_INPUT_DUMP_
//	nexCALBody_DumpInit(NEXCALBODY_VIDEO_INPUT, pConfig, iLen, eCodecType);
//#endif
//
//	return retval;	
    return 0;
}

NXINT32 nexCALBody_Video_MPEG2_Close( NXVOID *pUserData )
{
//	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)pUserData;
//	if ( pVDI != NULL )
//	{
//		NxMPEG2VideoDecClose((NxMPEG2VideoDecStruct*)pVDI->m_hDecoder);
//		nexSAL_MemFree( pVDI );
//	}
//	else
//	{
//		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] MPEG2 Decoder Handle is NULL. \n", __LINE__ );
//	}
//
//	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"\n\n ==> Ave MPEG2 Dec time : %d msec (%d / %d)\n\n", pVDI->m_uiTotalDecodeProcTime/(pVDI->m_uiDecodeFrameCount==0?1:pVDI->m_uiDecodeFrameCount), pVDI->m_uiTotalDecodeProcTime, pVDI->m_uiDecodeFrameCount);
//
//#ifdef _DUMP_VIDEO_INPUT_DUMP_
//	nexCALBody_DumpDeInit(NEXCALBODY_VIDEO_INPUT);
//#endif

	return 0;
}

NXINT32 nexCALBody_Video_MPEG2_Dec( NXUINT8* pData, NXINT32 iLen, NXVOID *pExtraInfo, NXUINT32 uDTS, NXUINT32  uPTS, NXINT32 iFlag, 
									  NXUINT32* puDecodeResult, NXVOID *pUserData )
{
//	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)pUserData;
//	NXINT32 retval = 0;
//
//	NXUINT32 uiEndTime = 0;
//	NXUINT32 uiStartTime = 0;
//	if(puDecodeResult)
//	{
//		NEXCAL_INIT_VDEC_RET(*puDecodeResult);
//	}
//
//	if ( pVDI == NULL )
//	{
//		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] Decoder Handle is NULL \n", __LINE__ );
//		NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
//	}
//	else if(iFlag & NEXCAL_VDEC_FLAG_OUTPUT_ONLY)
//	{
//		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//		return 0;
//	}
//	else
//	{
//		NxMPEG2VideoDecStruct *hCodec = (NxMPEG2VideoDecStruct *)pVDI->m_hDecoder;
//
//		//pVDI->m_uCount++;
//		
//		if ( pData == NULL || iLen == 0)
//		{
//			if(NEXCAL_VDEC_FLAG_END_OF_STREAM & iFlag)
//			{
//	 			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//	 			return NEXCAL_ERROR_NONE;			
//			}
//			else
//			{
//				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] Frame data is NULL \n", __LINE__ );
//				NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
//				return NEXCAL_ERROR_FAIL;
//			}
//		}
//		else
//		{
//			pVDI->m_uPTS = uDTS; //[shoh][2012.01.30] Because MPEG2 decoder does not consider PTS, calbody temporarily saves DTS.
//			//NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[CALBody.cpp, %d]\t <<< Video Decode [0x%X|%7u|%7u] >>>\n", __LINE__, pData, iLen, uPTS);
//			uiEndTime = 0;
//			uiStartTime = nexSAL_GetTickCount();
//			retval = NxMPEG2VideoDecDecode(hCodec, pData, iLen);
//			uiEndTime = nexSAL_GetTickCount();
//			
//			pVDI->m_uiDecodeFrameCount++;
//			pVDI->m_uiTotalDecodeProcTime += (uiEndTime - uiStartTime);	
//			//NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[MPEG2 CALBody %d] Elapsed time : %d", __LINE__, uiEndTime - uiStartTime);
//
//			if ( retval ) {
//				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[CALBody.cpp, %d] Error Video Dec Decode \n", __LINE__ );
//				NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
//				retval = 1;
//			}
//			else
//			{
//				NxMPEG2VideoDecGetOutputFrame( hCodec, &pVDI->m_pY, &pVDI->m_pU, &pVDI->m_pV);
//				
//				if(pVDI->m_pY && pVDI->m_pU && pVDI->m_pV)
//				{
//					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
//					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
//				}
//				else
//				{
//					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
//					NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
//				}
//			}
//		}
//	}
//
//#ifdef _DUMP_VIDEO_INPUT_DUMP_
//	if (NEXCAL_CHECK_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME) == TRUE)
//		nexCALBody_DumpDecode(NEXCALBODY_VIDEO_INPUT, pData, iLen, uDTS);
//#endif
//
//#ifdef _DUMP_VIDEO_OUTPUT_DUMP_
//	if (NEXCAL_CHECK_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST) == TRUE)
//		nexCALBody_VIdeoOutputDump(NEXCALBODY_VIDEO_YUV420, pVDI->m_pY, pVDI->m_pU, pVDI->m_pV, pVDI->m_uPitch, pVDI->m_uHeight, pVDI->m_uPTS);
//#endif
//
//	return retval;
    return 0;
}

NXINT32 nexCALBody_Video_MPEG2_GetOutput( NXUINT8** ppBits1, NXUINT8** ppBits2, NXUINT8** ppBits3, NXUINT32 *puPTS, NXVOID *pUserData )
{	
//	VideoDecoderInfo* pVDI = (VideoDecoderInfo*)pUserData;
//
//	//NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[nexCALBody_Video_AVC_GetOutput] YUV (0x%X, 0x%X, 0x%X)\n", g_pY, g_pU, g_pV);
//	
//	if(pVDI)
//	{
//	
//		if ( pVDI->m_pY && pVDI->m_pY && pVDI->m_pV )
//		{
//			*ppBits1=pVDI->m_pY;
//			*ppBits2=pVDI->m_pU;
//			*ppBits3=pVDI->m_pV;
//			*puPTS = pVDI->m_uPTS;
//			pVDI->m_pY=0;
//			pVDI->m_pU=0;
//			pVDI->m_pV=0;
//		}
//		else
//		{
//			*ppBits1=NULL;
//			*ppBits2=NULL;
//			*ppBits3=NULL;
//		}
//
//		*puPTS = pVDI->m_uPTS;
//	}
	return 0;
}

NXINT32 nexCALBody_Video_MPEG2_Reset( NXVOID *pUserData )
{
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[CALBody.cpp, %d] nexCALBody_Video_MPEG2_Reset \n", __LINE__ );
	return 0;
}