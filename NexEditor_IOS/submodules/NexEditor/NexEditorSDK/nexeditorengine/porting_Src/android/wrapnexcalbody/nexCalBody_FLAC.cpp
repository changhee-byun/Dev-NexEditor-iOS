#include "NexCAL.h"
#include "nexCalBody.h"
#include "nexCalBody_FLAC.h"
#include "NxFLACDecAPI.h"

#include <stdlib.h>
#include <unistd.h>
#include <android/log.h>

//namespace android {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// FLAC
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define NEX_MAX_FLAC_PCM_SIZE (1024*132)
#define NEX_PCM_NO_MEMCPY (1)

typedef struct FLACAudioDecoderInfo
{
	PNXFLACDEC_HANDLE hFLACDec;
#ifndef NEX_PCM_NO_MEMCPY
	char* stPCMBuffer;
#endif
} FLACAudioDecoderInfo;

unsigned int nexCALBody_Audio_FLAC_Init( unsigned int uCodecObjectTypeIndication, unsigned char* pConfig, int iConfigLen,
										 unsigned char* pFrame, int iFrameLen, void *pInitInfo, void *pExtraInfo, unsigned int* piSamplingRate, unsigned int* piNumOfChannels,
										 unsigned int* puBitsPerSample, unsigned int* piNumOfSamplesPerChannel, unsigned int uMode, unsigned int uUserDataType, void** ppUserData )
{
	PNEXADECUsrData pAudioUsrData = NULL;
	FLACAudioDecoderInfo* pADI = NULL;

	int iRet;

	pAudioUsrData = (PNEXADECUsrData)nexCAL_MemAlloc( sizeof(NEXADECUsrData) );
	if (!pAudioUsrData)
	{
		nexCAL_DebugPrintf("Error Memory Allocation\n");
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

	const char *pVersion = NxFLACDecGetVersionInfo(1);
	nexCAL_DebugLogPrintf("[FlacBody.cpp %d] NxFLACDec Version (%s) \n", __LINE__, pVersion);

	pADI = (FLACAudioDecoderInfo*)nexCAL_MemAlloc(sizeof(FLACAudioDecoderInfo));
	if (!pADI)
	{
		nexCAL_DebugLogPrintf("Error Memory Allocation\n");
		nexCAL_MemFree( pAudioUsrData );
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

	memset( pAudioUsrData, 0x00, sizeof(NEXADECUsrData) );
	memset(pADI, 0x00, sizeof(FLACAudioDecoderInfo));

#ifndef NEX_PCM_NO_MEMCPY
	pADI->stPCMBuffer = (char*)nexCAL_MemAlloc(NEX_MAX_FLAC_PCM_SIZE);
#endif
	pADI->hFLACDec = NxFLACDecOpen();

	if (pADI->hFLACDec == NULL)
	{
#ifndef NEX_PCM_NO_MEMCPY
		nexCAL_MemFree(pADI->stPCMBuffer);
#endif
		nexCAL_MemFree(pADI);
		nexCAL_MemFree( pAudioUsrData );
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

	pAudioUsrData->m_puExtraData = (void *)pADI;

	pADI->hFLACDec->bDownSampleFlag = 0;

	pADI->hFLACDec->pBitstream = pFrame;
	pADI->hFLACDec->pPCMBuffer      = NULL;
	pADI->hFLACDec->nInput = iFrameLen;

	// 24 bits PCM 출력을 원할 경우 셋팅.
	if (pAudioUsrData->m_puAudio24bitEnable)
	{
		pADI->hFLACDec->bOutBPS24 = 1;
#ifdef NEX_FLAC_WAV_24BIT_AND_8BIT_PADDING_NOT_USE
		pADI->hFLACDec->bPadding32bps = 0;	//24bps output
#else
		pADI->hFLACDec->bPadding32bps = 1;	//24bps + 8bit padding = 32bps output
#endif
	}
	else
	{
		pADI->hFLACDec->bOutBPS24 = 0;
		pADI->hFLACDec->bPadding32bps = 0;
	}

	iRet = NxFLACDecInit(pADI->hFLACDec);

	if (iRet == NX_FLAC_USE_STREAMINFO)
		pADI->hFLACDec->nSampleRate = *piSamplingRate;
	else
		*piSamplingRate	= pADI->hFLACDec->nSampleRate;

	//*piNumOfSamplesPerChannel = pADI->hFLACDec->nSamplesPerChannel;
	*piNumOfChannels = pADI->hFLACDec->nChannels;
	*puBitsPerSample = pADI->hFLACDec->nBitPerSample;
	*piNumOfSamplesPerChannel = 1024;
    
	pAudioUsrData->m_uNumOfChannels = *piNumOfChannels;
	pAudioUsrData->m_uSamplingRate = *piSamplingRate;
	pAudioUsrData->m_uBitsPerSample = *puBitsPerSample;
	pAudioUsrData->m_uNumOfSamplesPerChannel = *piNumOfSamplesPerChannel;// = 2048;

	*ppUserData = (void *)pAudioUsrData;

	nexCAL_DebugLogPrintf("[nexCalBody_FLAC %d] NxFLACDecInit() success. Audio Information(%d,%d,0x%x,%d\n", __LINE__, *piSamplingRate, *piNumOfChannels, pFrame, iFrameLen);

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_INPUT, pConfig, iConfigLen, uCodecObjectTypeIndication, (unsigned int*)pAudioUsrData);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_OUTPUT, NULL, 0, uCodecObjectTypeIndication, (unsigned int*)pAudioUsrData);
#endif

	return 0;
}

unsigned int nexCALBody_Audio_FLAC_Deinit( void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;

	if (pAudioUsrData)
	{
		FLACAudioDecoderInfo* pADI = (FLACAudioDecoderInfo* )pAudioUsrData->m_puExtraData;
		unsigned int temp = pAudioUsrData->m_puAudio24bitEnable;
		NxFLACDecClose(pADI->hFLACDec);

#ifndef NEX_PCM_NO_MEMCPY
		nexCAL_MemFree(pADI->stPCMBuffer);
#endif
		nexCAL_MemFree(pADI);

		memset(pAudioUsrData, 0x00, sizeof(NEXADECUsrData));
		pAudioUsrData->m_puAudio24bitEnable = temp;
		nexCAL_MemFree( pAudioUsrData );
	}
	else
	{
		nexCAL_DebugLogPrintf("FLAC Decoder Handle is NULL.\n");
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_INPUT);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_OUTPUT);
#endif

	return 0;
}

unsigned int nexCALBody_Audio_FLAC_Dec( unsigned char* pSource, int iLen, void *pExtraInfo, void* pDest, int* piWrittenPCMSize, unsigned int uDTS, unsigned int* puOutputTime, int nFlag, unsigned int* puDecodeResult, void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;
	FLACAudioDecoderInfo* pADI =  (FLACAudioDecoderInfo*)pAudioUsrData->m_puExtraData;
	int iRet;
	unsigned char *pOut = 0;
	unsigned long ulSize = 0;
	unsigned int uiMAXSizePCMBuffer = (unsigned int )(*piWrittenPCMSize);

	if (puDecodeResult)
	{
		*puDecodeResult = 0;
	}

	if (pAudioUsrData->m_uSettingChanged)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);
		pAudioUsrData->m_uSettingChanged = false;
		return 0;
	}

	*piWrittenPCMSize = 0;

#ifndef NEX_PCM_NO_MEMCPY
	if (pAudioUsrData->m_uWrittenPCMSize)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);

		if (pAudioUsrData->m_uWrittenPCMSize <= uiMAXSizePCMBuffer)
		{
			memcpy( pDest, pADI->stPCMBuffer, pAudioUsrData->m_uWrittenPCMSize);
			*piWrittenPCMSize = pAudioUsrData->m_uWrittenPCMSize;
			pAudioUsrData->m_uWrittenPCMSize = 0;
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
		}
		else
		{
			memcpy( pDest, pADI->stPCMBuffer, uiMAXSizePCMBuffer );
			*piWrittenPCMSize = uiMAXSizePCMBuffer;
			pAudioUsrData->m_uWrittenPCMSize -= uiMAXSizePCMBuffer;
			memmove ( pADI->stPCMBuffer, pADI->stPCMBuffer + uiMAXSizePCMBuffer, pAudioUsrData->m_uWrittenPCMSize );
		}

	#ifdef _DUMP_AUDIO_INPUT_DUMP_
		if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME) == TRUE)
			nexCALBody_DumpDecode(NEXCALBODY_AUDIO_INPUT, pSource, iLen, uDTS);
	#endif

	#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
		if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST) == TRUE)
			nexCALBody_DumpDecode(NEXCALBODY_AUDIO_OUTPUT, (unsigned char *)pDest, *piWrittenPCMSize , uDTS);
	#endif

		return 0;
	}
#endif

	if(nFlag == NEXCAL_ADEC_FLAG_END_OF_STREAM)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);
		return 0;
	}

	pADI->hFLACDec->pBitstream      = pSource;
	pADI->hFLACDec->nInput = iLen;

#ifdef NEX_PCM_NO_MEMCPY
	pADI->hFLACDec->pPCMBuffer      = (unsigned char *)pDest;
#else
	pADI->hFLACDec->pPCMBuffer      = (unsigned char *)pADI->stPCMBuffer;
#endif

	iRet=NxFLACDecDecode(pADI->hFLACDec);


	if (!iRet)
	{
		ulSize =  pADI->hFLACDec->nSamplesPerChannel * pADI->hFLACDec->nChannels * (pADI->hFLACDec->nBitPerSample>>3);
	}
	else
	{
		NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		return iRet;
	}


	if (ulSize)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);

		if (ulSize <= uiMAXSizePCMBuffer)
		{
#ifndef NEX_PCM_NO_MEMCPY
			memcpy( pDest, pADI->stPCMBuffer, ulSize);
#endif
			*piWrittenPCMSize = ulSize;
			nexCAL_DebugLogPrintf("[nexCalBody_FLAC %d] FLAC Out of *piWrittenPCMSize !!! %d, %d \n", __LINE__, *piWrittenPCMSize, ulSize);

			//pAudioUsrData->m_uWrittenPCMSize = 0; // ???
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
		}
		else
		{
#ifdef NEX_PCM_NO_MEMCPY
			nexCAL_DebugLogPrintf("[nexCalBody_FLAC %d] FLAC Out of MaxPCMSize !!! %d, %d \n", __LINE__, uiMAXSizePCMBuffer, ulSize);
			*piWrittenPCMSize = uiMAXSizePCMBuffer;
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
			memcpy( pDest, pADI->stPCMBuffer, uiMAXSizePCMBuffer );
			*piWrittenPCMSize = uiMAXSizePCMBuffer;
			pAudioUsrData->m_uWrittenPCMSize = ulSize - uiMAXSizePCMBuffer;
			memmove ( pADI->stPCMBuffer, pADI->stPCMBuffer + uiMAXSizePCMBuffer, pAudioUsrData->m_uWrittenPCMSize );
#endif
		}
	}
	else
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME) == TRUE)
		nexCALBody_DumpDecode(NEXCALBODY_AUDIO_INPUT, pSource, iLen, uDTS);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST) == TRUE)
		nexCALBody_DumpDecode(NEXCALBODY_AUDIO_OUTPUT, (unsigned char *)pDest, *piWrittenPCMSize , uDTS);
#endif

	return 0;
}

unsigned int nexCALBody_Audio_FLAC_Reset( void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;
	FLACAudioDecoderInfo* pADI = (FLACAudioDecoderInfo*)pAudioUsrData->m_puExtraData;
	pAudioUsrData->m_uWrittenPCMSize = 0;

	nexCAL_DebugLogPrintf("[nexCalBody_FLAC %d] CAL_FLACDEC_Reset()\n", __LINE__);

	NxFLACDecReset(pADI->hFLACDec);

	return 0;
}

unsigned int nexCALBody_Audio_FLAC_GetInfo	( unsigned int uIndex
												, unsigned int* puResult
												, void* pUserData )
{

	nexCAL_DebugLogPrintf("[nexCalBody_FLAC.cpp, %d] GetInfo called!\n", __LINE__);
	return 0;
}

unsigned int nexCALBody_Audio_FLAC_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* pUserData )
{
	switch (uProperty)
	{
		case NEXCAL_PROPERTY_CODEC_IMPLEMENT:
			*puValue = (NXINT64)NEXCAL_PROPERTY_CODEC_IMPLEMENT_SW;
			break;
		case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
			*puValue = (NXINT64)NEXCAL_PROPERTY_ANSWERIS_YES;
			break;
		case NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE :
			*puValue = (NXINT64)NEXCAL_PROPERTY_AUDIO_BUFFER_PCM;
			break;
		case NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL:
			*puValue = (NXINT64)NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL_NO;
			break;
		default:
			*puValue = 0;
			break;
	}

	return 0;
}

unsigned int nexCALBody_Audio_FLAC_SetProperty( unsigned int uProperty, NXINT64 uValue, void* pUserData )
{
	return 0;
}

//};
