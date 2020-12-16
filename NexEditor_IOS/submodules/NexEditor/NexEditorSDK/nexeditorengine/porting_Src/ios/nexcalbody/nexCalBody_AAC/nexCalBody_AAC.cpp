#include "NexCAL.h"
#include "NxAACDecAPI.h"
#include "NexSAL_Com.h"
#include "NexSAL_Internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	NEXCALBODY_AAC_REG_KEY			0x3f78a692

#define  LOG_TAG    "NEXCAL_AAC"

#define ADIF_ACC_FRAME_SIZE 3072
#define All_PCM_BUFFER_SIZE 2048*6

unsigned int g_uiSupportES_AAC = 0;

// rooney common api
extern "C" void *nex_malloc(int size)
{
    return malloc(size);
}

extern "C" void *nex_calloc(int count, int size)
{
    return calloc(count, size);
}

extern "C" void nex_free(void *pMem)
{
    free(pMem);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// AAC
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct AudioDecoderInfo
{
	NXUINT32	m_bInit;
	NXUINT32 	m_bReset;
	PNXAACDECHANDLE m_hDecoder;

	NXUINT32 m_uChannels;
	NXUINT32 m_uSamplingRate;
	NXUINT32 m_uBitsPerSample;
	NXUINT32 m_uNumOfSamplesPerChannel;

	NXUINT32	m_uAudioCodecErrorCount;
	NXUINT32 m_uTime;
	NXUINT32 m_uComplementTS;

	/* for ADIF, fake ADIF */
	NXUINT32  uiSize;
	NXUINT32  uiOffset;
	NXUINT32  uiBufferUpdate;
	NXUINT8 *pADIF;
	NXUINT8 *pBuf;
	NXBOOL  isFakeADIF;
} AudioDecoderInfo;


NXINT32 nexCALBody_Audio_AAC_Init ( NEX_CODEC_TYPE eCodecType
										, NXUINT8* pConfig, NXINT32 iConfigLen
										, NXUINT8* pFrame, NXINT32 iFrameLen
										, NXVOID* pInitInfo
										, NXVOID* pExtraInfo										
										, NXUINT32* piSamplingRate
										, NXUINT32* piNumOfChannels
										, NXUINT32* puBitsPerSample
										, NXUINT32* piNumOfSamplesPerChannel
										, NXUINT32 uMode
										, NXUINT32 uUserDataType
										, NXVOID **ppUserData )
{
	AudioDecoderInfo* pADI = NULL;
	NXINT32	iRet;
	NXINT32 tempChannels = *piNumOfChannels;

	NXINT32 iTotalConsumed = 0;
	NXINT32 iNumFrames = 0;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[nexCalBody_AAC.cpp, %d] AAC Decoder version %s released %s \n", 
													__LINE__, 
													NxAACDecGetVersionInfo(1),  
													NxAACDecGetVersionInfo(2));

	pADI = (AudioDecoderInfo*)nexSAL_MemAlloc(sizeof(AudioDecoderInfo));
	if (!pADI)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "Error Memory Allocation\n");
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

	memset(pADI, 0x00, sizeof(AudioDecoderInfo));
	
	pADI->m_uTime = INVALID_VALUE;
	pADI->m_hDecoder = NxAACDecOpen();

	if (pConfig)
	{
		pADI->m_hDecoder->pInputBitStream = pConfig;
		pADI->m_hDecoder->InputBitstreamLength = iConfigLen;
		iRet = NxAACDecParseConfig(pADI->m_hDecoder, (NXUINT32*)piSamplingRate, (NXUINT32 *)piNumOfChannels);
		if (iRet != NXAACDECERR_OK)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[nexCALBody_AAC.cpp, %d] NxAACDecParseConfig. Error(%d)! \n", __LINE__, iRet);
			nexSAL_MemFree( pADI );
			*ppUserData = NULL;
			return NEXCAL_ERROR_FAIL;
		}
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_INPUT, pConfig, iConfigLen, eCodecType);
	nexCALBody_DumpDecode(NEXCALBODY_AUDIO_INPUT, pFrame, iFrameLen, 0);
#endif

	if (*piNumOfChannels == 0 && g_uiSupportES_AAC == 1)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCALBody_AAC.cpp, %d] piNumOfChannels is Zero! Get Channel Number!\n", __LINE__);
		pADI->m_hDecoder->pInputBitStream = pFrame;
		pADI->m_hDecoder->InputBitstreamLength = iFrameLen;
		iRet = NxAACDecInit( pADI->m_hDecoder, piSamplingRate, piNumOfChannels);
		if (iRet != NXAACDECERR_OK)
		{
			nexSAL_MemFree( pADI );
			*ppUserData = NULL;
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[nexCALBody_AAC.cpp, %d] nexCALBody_Audio_AAC_Init. Error(%d)!! (sampling rate: %d, channel number:%d)\n", __LINE__, iRet, *piSamplingRate, *piNumOfChannels);
			return NEXCAL_ERROR_FAIL;
		}
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCALBody_AAC.cpp, %d] Get audio channel number ok : channel_num=%d\n", __LINE__, *piNumOfChannels);

		//Reset Decoder
		NxAACDecReset(pADI->m_hDecoder);
	}

	if (*piNumOfChannels > 2 && g_uiSupportES_AAC == 1)
	{
		pADI->m_hDecoder->bDownMixed_Output_flag = 0;
		pADI->m_hDecoder->bMultiChan_Output_flag = 1;
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCALBody_AAC.cpp, %d] Set Multi-Channel!\n", __LINE__);
	}
	else
	{
		pADI->m_hDecoder->bDownMixed_Output_flag = 1;
		pADI->m_hDecoder->bMultiChan_Output_flag = 0;
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCALBody_AAC.cpp, %d] Set Normal-Channel!\n", __LINE__);
	}

	//chunho.choi_2015.07.08 - for original 3ch or 5ch(not 5.1ch)
	if (*piNumOfChannels == 3 || *piNumOfChannels == 5)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCALBody_AAC.cpp %d] Channel number is 5!!! Set zero padding channel!\n", __LINE__);
		pADI->m_hDecoder->bMultiOut_ZeroPaddig = 1;
	}

	if (pADI->m_hDecoder->AACType != NXAACDEC_AACTYPE_ELD)
	{
		pADI->m_hDecoder->pInputBitStream = pFrame;
		pADI->m_hDecoder->InputBitstreamLength = iFrameLen;
	}

#if 0
	pADI->m_hDecoder->bMultiChan_Output_flag = TRUE;
	pADI->m_hDecoder->bDownMixed_Output_flag = TRUE;
#endif

	iRet = NxAACDecInit( pADI->m_hDecoder, (unsigned int *)piSamplingRate, (unsigned int *)piNumOfChannels);
	if (iRet != NXAACDECERR_OK)
	{
		nexSAL_MemDump( pFrame, iFrameLen );
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[nexCALBody_AAC.cpp, %d] nexCALBody_Audio_AAC_Init. Error(%d)!! (sampling rate: %d, channel number:%d)\n", __LINE__, iRet, *piSamplingRate, *piNumOfChannels);
		nexSAL_MemFree( pADI );
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[nexCALBody_AAC.cpp, %d] AAC Decoder init complete!\n", __LINE__);
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[nexCALBody_AAC.cpp, %d] SamplingRate(%d), NumOfChannel(%d) - After DecInit\n", __LINE__, *piSamplingRate, *piNumOfChannels);

	pADI->m_bInit = TRUE;
	iNumFrames ++;
	iTotalConsumed += pADI->m_hDecoder->InputBitstreamUsedBytes;

	//jbcho 5.1. ch exception 
	if ((pADI->m_hDecoder->AACType != NXAACDEC_AACTYPE_ELD) && (tempChannels < 3))
	{
		if ((pADI->m_hDecoder->InputBitstreamUsedBytes < iFrameLen) && !(pADI->m_hDecoder->bHeaderPresent)
		&& ((iFrameLen - pADI->m_hDecoder->InputBitstreamUsedBytes) >= pADI->m_hDecoder->InputBitstreamUsedBytes))
		{
reinit:
			pADI->m_hDecoder->pInputBitStream += pADI->m_hDecoder->InputBitstreamUsedBytes;
			pADI->m_hDecoder->InputBitstreamLength = iFrameLen - iTotalConsumed;

			iRet=NxAACDecInit(pADI->m_hDecoder, (NXUINT32 *)piSamplingRate, (NXUINT32 *)piNumOfChannels);
			if(iRet == NXAACDECERR_OK)
			{
				iNumFrames++;
				pADI->m_hDecoder->is_adif = TRUE;
				pADI->isFakeADIF = TRUE;
				pADI->m_hDecoder->bReadFrame = TRUE;
				iTotalConsumed += pADI->m_hDecoder->InputBitstreamUsedBytes;

				if(iTotalConsumed < iFrameLen)
					goto reinit;
			}
		}
	}

#if 0
	if( *piNumOfChannels > 2 && g_uiSupportES_AAC )
	{
		//5.1: L, R, C, lfe, LS , RS
		//5   : L, R, C, lfe(0?žâ„¢), LS, RS
		//4   : L, R, C, lfe(0?žâ„¢), LS(0?žâ„¢), RS(0?žâ„¢) :Left SurrondÂ¥Â¬ ???â‰¤ (AS?†Â?Ã¸?°âˆšÂª) 
		//3   : L, R, C, lfe(0?žâ„¢), LS(0?žâ„¢), RS(0?žâ„¢)
		pADI->m_hDecoder->bDownMixed_Output_flag = FALSE; //  bypass to 5.1ch pcm
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[nexCalBody_AAC.cpp, %d] AAC Decoder run with multi-channel output \n", __LINE__);
		//pADI->m_hDecoder->pOutputALLPCM = (short*) malloc( sizeof(short)*All_PCM_BUFFER_SIZE ); // Don't need to allocate a memory for decode. It's passed by the renderer
	}
	else
	{
		pADI->m_hDecoder->bMultiChan_Output_flag = FALSE;
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[nexCalBody_AAC.cpp, %d] AAC Decoder run with down-mixed output(%d->%d)\n",*piNumOfChannels, pADI->m_hDecoder->OutputChannels);
	}
#endif

	if (pADI->m_hDecoder->is_adif)
	{
		pADI->pADIF = (NXUINT8*) nexSAL_MemAlloc(ADIF_ACC_FRAME_SIZE*2);
		pADI->uiOffset = 0;
		pADI->uiSize   = 0;
		pADI->uiBufferUpdate = TRUE;
	}

	pADI->m_uChannels = *piNumOfChannels =  pADI->m_hDecoder->OutputChannels;
	pADI->m_uSamplingRate  = *piSamplingRate = pADI->m_hDecoder->SamplingRate;
	pADI->m_uBitsPerSample = *puBitsPerSample = 16;
	pADI->m_uNumOfSamplesPerChannel = *piNumOfSamplesPerChannel = pADI->m_hDecoder->SamplePerChannel;

	pADI->m_bReset = FALSE;
	*ppUserData = (NXVOID*)pADI;

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_OUTPUT, NULL, 0, eCodecType);
#endif

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[%s %d] AAC codec Init success! Userdata[%p]\n", _FUNLINE_, *ppUserData);
	return 0;
}

NXINT32 nexCALBody_Audio_AAC_Deinit(NXVOID *pUserData)
{
	AudioDecoderInfo* pADI = (AudioDecoderInfo*)pUserData;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0,"[%s %d] AAC codec DeInit start! Userdata[%p]\n", _FUNLINE_, pUserData);
	
	if (pADI)
	{
		if (pADI->m_bInit )
		{
			PNXAACDECHANDLE	hCodec = (PNXAACDECHANDLE)pADI->m_hDecoder;
			if (hCodec)
			{
				if (hCodec->is_adif)
					nexSAL_MemFree(pADI->pADIF);

				NxAACDecClose(hCodec);
			}
		}
		nexSAL_MemFree(pADI);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "AAC Decoder Info. handle is NULL.\n");
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_INPUT);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_OUTPUT);
#endif

	return 0;
}

NXINT32 nexCALBody_Audio_AAC_Dec( NXUINT8* pSource, NXINT32 iLen, NXVOID* pExtraInfo, NXVOID* pDest, NXINT32* piWrittenPCMSize, NXUINT32 uDTS, NXUINT32 *puOutTS, NXINT32 nFlag,  NXUINT32* puDecodeResult, NXVOID *pUserData )
{
	NXINT32 iRet = NEXCAL_ERROR_NONE;
	AudioDecoderInfo* pADI = (AudioDecoderInfo *)pUserData;
	PNXAACDECHANDLE	hCodec;
	NXUINT32	uRet = 0, uMaxPCMSize = *piWrittenPCMSize;

	if (pADI == NULL)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[nexCALBody_AAC.cpp, %d] Audio Decoder Info. handle is NULL!! \n", __LINE__);
		return 0;
	}

	if (puDecodeResult)
	{
		NEXCAL_INIT_ADEC_RET(*puDecodeResult);
	}

	if (nFlag & NEXCAL_ADEC_FLAG_END_OF_STREAM)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);
		return 0;
	}

	hCodec = (PNXAACDECHANDLE)pADI->m_hDecoder;

	if (hCodec->is_adif)
	{
		if (pADI->uiBufferUpdate)
		{
			memcpy(pADI->pADIF, pADI->pADIF + pADI->uiOffset, pADI->uiSize);
			memcpy(pADI->pADIF + pADI->uiSize, pSource, iLen);
			pADI->uiSize		+= iLen;
			pADI->uiOffset		= 0;
			pADI->uiBufferUpdate= FALSE;
			pADI->pBuf			= pADI->pADIF;
		}

		hCodec->pInputBitStream		= pADI->pBuf;
		hCodec->InputBitstreamLength= pADI->uiSize;

		if (hCodec->bDownMixed_Output_flag == 1)
		{
			hCodec->pOutputPCM = (NXINT16 *)pDest;
		}
		else if (hCodec->bMultiChan_Output_flag == 1)
		{
			hCodec->pOutputALLPCM = (NXINT16 *)pDest;
		}
	}
	else
	{
		hCodec->pInputBitStream = pSource;
		hCodec->InputBitstreamLength = iLen;

		if (hCodec->bDownMixed_Output_flag == 1)
		{
			hCodec->pOutputPCM = (NXINT16 *)pDest;
		}
		else if (hCodec->bMultiChan_Output_flag == 1)
		{
			hCodec->pOutputALLPCM = (NXINT16 *)pDest;
		}
	}

	iRet = NxAACDecDecode(hCodec);

    // rooney
//	if (puOutTS)
//	{
//		*puOutTS = uDTS;
//	}

	if (hCodec->is_adif)
	{
		if ((iRet == NXAACDECERR_OK) || (iRet == NXAACDECERR_NEED_MORE_DATA))
		{
			pADI->pBuf	  += (hCodec->InputBitstreamUsedBytes);
			pADI->uiOffset += (hCodec->InputBitstreamUsedBytes);
			pADI->uiSize   -= (hCodec->InputBitstreamUsedBytes);

			//*piWrittenPCMSize = hCodec->TotalSamples * 2;
			*piWrittenPCMSize = hCodec->OutputChannels * hCodec->SamplePerChannel * sizeof(short);

			if (pADI->isFakeADIF)
			{
				NXINT32 temp = 1;
				while (pADI->uiSize)
				{
					hCodec->pInputBitStream = pADI->pBuf;
					hCodec->InputBitstreamLength = pADI->uiSize;

					if (hCodec->bDownMixed_Output_flag == 1)
					{
						hCodec->pOutputPCM = (NXINT16 *)((NXCHAR*)pDest + (*piWrittenPCMSize));
					}
					else if (hCodec->bMultiChan_Output_flag == 1)
					{
						hCodec->pOutputALLPCM = (NXINT16 *)((NXCHAR*)pDest + (*piWrittenPCMSize));
					}

					iRet = NxAACDecDecode(hCodec);

					pADI->pBuf	  += (hCodec->InputBitstreamUsedBytes);
					pADI->uiOffset += (hCodec->InputBitstreamUsedBytes);
					pADI->uiSize   -= (hCodec->InputBitstreamUsedBytes);
					//*piWrittenPCMSize += hCodec->TotalSamples * 2;
					*piWrittenPCMSize += hCodec->OutputChannels * hCodec->SamplePerChannel * sizeof(short);

					temp++;
					if (iRet == NXAACDECERR_NEED_MORE_DATA)
					{
						break;
					}
					else if (iRet != NXAACDECERR_OK)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCalBody_AAC.cpp %d] AAC Decoding Error! \n", __LINE__);
						NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
						NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
						uRet = NEXCAL_ERROR_FAIL;
					}
				}
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "nexCalBody_AAC %ll, %d, %d, %d, %d, %d\n", uDTS, *piWrittenPCMSize, temp, hCodec->TotalSamples * 2, pADI->uiOffset, pADI->uiSize);							
			}

			if ((pADI->uiSize==0) || (iRet == NXAACDECERR_NEED_MORE_DATA))
			{
				pADI->uiBufferUpdate = TRUE;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			}
			else
			{
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			}

			pADI->m_uComplementTS += (NXUINT32)(((*piWrittenPCMSize)*1000)/(pADI->m_uChannels*pADI->m_uBitsPerSample/8)/pADI->m_uSamplingRate);
			*puOutTS = pADI->m_uComplementTS;	

			iRet = NEXCAL_ERROR_NONE;
		}
		else
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCalBody_AAC.cpp %d] AAC Decoding Error! \n", __LINE__);
			NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			iRet = NEXCAL_ERROR_FAIL;
		}
	}
	else
	{
		if (iRet == NXAACDECERR_NEED_MORE_DATA) 
		{
			pADI->uiBufferUpdate = 1;
			*piWrittenPCMSize = 0;
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			iRet = NEXCAL_ERROR_NONE;
		}
		else if (iRet == NXAACDECERR_CHANNEL_CHANGED)
		{
			NXUINT32 uVal = 0;
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCalBody_AAC.cpp %d] AAC Decoding NXAACDECERR_CHANNEL_CHANGED! Syntax check this frame!\n", __LINE__);

			uVal = NxAACErrCheck(hCodec);
			if (uVal != NXAACDECERR_OK)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[nexCalBody_AAC.cpp %d] This frame syntax error!!!\n", __LINE__);
				NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			}
			else
			{
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_SETTING_CHANGED);
			}
			iRet = NEXCAL_ERROR_FAIL;
		}
		else if (uRet != NXAACDECERR_OK)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCalBody_AAC.cpp %d] AAC Decoding Error! \n", __LINE__);
			NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			iRet = NEXCAL_ERROR_FAIL;
		}
		else
		{
			//*piWrittenPCMSize = hCodec->TotalSamples * sizeof(short); //hCodec->OutputChannels * hCodec->SamplePerChannel * sizeof(short);
			*piWrittenPCMSize = hCodec->OutputChannels * hCodec->SamplePerChannel * sizeof(short);

			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);

			if (pADI->m_uTime == uDTS)
			{
				pADI->m_uComplementTS += (NXUINT32)(((*piWrittenPCMSize)*1000)/(pADI->m_uNumOfSamplesPerChannel*pADI->m_uBitsPerSample/8)/pADI->m_uSamplingRate);
                // rooney
                //				*puOutTS = pADI->m_uTime + pADI->m_uComplementTS;
			}
			else
			{
                // rooney
//				*puOutTS = pADI->m_uTime = uDTS;
				pADI->m_uComplementTS = 0;
			}	
	
			iRet = NEXCAL_ERROR_NONE;
		}
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME) == TRUE)
		nexCALBody_DumpDecode(NEXCALBODY_AUDIO_INPUT, pSource, iLen, uPTS);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	if (NEXCAL_CHECK_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST) == TRUE)
		nexCALBody_DumpDecode(NEXCALBODY_AUDIO_OUTPUT, (NXUINT8 *)pDest, *piWrittenPCMSize , uDTS);
#endif

	if (uMaxPCMSize < *piWrittenPCMSize)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_AUDIO, 0, "[nexCalBody_AAC.cpp %d] AAC Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);	
		*piWrittenPCMSize = uMaxPCMSize;
	}

	return iRet;
}

NXINT32 nexCALBody_Audio_AAC_Reset(NXVOID *pUserData)
{
	AudioDecoderInfo* pADI = (AudioDecoderInfo*)pUserData;
	PNXAACDECHANDLE	hCodec;

	if (pADI == NULL)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[nexCalBody_AAC.cpp, %d] Audio Decoder Info. pointer is NULL\n", __LINE__);
		return NEXCAL_ERROR_NONE;
	}	

	if (pADI->m_bReset == TRUE)
	{
		return 0;
	}
	
	pADI->m_bReset = TRUE;
	if (pADI->m_bInit)
	{
		if (pADI->m_hDecoder)
		{
			NxAACDecReset(pADI->m_hDecoder);
		}
	}
	pADI->m_bReset = FALSE;
	pADI->uiBufferUpdate = 1;
	pADI->uiOffset = 0;
	pADI->uiSize   = 0;
	pADI->m_uTime = INVALID_VALUE;
	pADI->m_uComplementTS = 0;

	return 0;
}


/* AAC */
NXINT32 nexCALBody_Audio_AAC_GetProperty(NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData)
{
	switch (uProperty)
	{
	case NEXCAL_PROPERTY_CODEC_IMPLEMENT:
		*pqValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW;
		break;
	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*pqValue = NEXCAL_PROPERTY_ANSWERIS_YES;
		break;
	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME :
		*pqValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO;
		break;
	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER :
		*pqValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_YES;
		break;
	case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
		if(g_uiSupportES_AAC)
			*pqValue  = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_YES;
		else
			*pqValue = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO;
		break;
	case NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE :
		*pqValue = NEXCAL_PROPERTY_AUDIO_BUFFER_PCM;
		break;		
	case NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL:
		*pqValue = NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL_YES;
		break;		
	default:
		*pqValue = 0;
		break;
	}
	return 0;
}

NXINT32 nexCALBody_Audio_AAC_SetProperty(NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData)
{
	switch(uProperty)
	{
	case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
		g_uiSupportES_AAC = qValue;
		break;

	default:
		break;
	}

	return 0;
}
