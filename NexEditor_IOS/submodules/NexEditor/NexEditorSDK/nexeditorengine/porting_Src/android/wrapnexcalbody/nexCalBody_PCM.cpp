#include "NexCAL.h"
#include "nexCalBody.h"
#include "nexCalBody_PCM.h"

#include <stdlib.h>
#include <unistd.h>
#include <android/log.h>


//namespace android {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PCM
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define NEX_PCM_MONO_TO_STEREO

#define CAL_PCMDEC_MIX_GAIN_3DB_Q15		23170		//Q(15) 0.7079
#define CAL_PCMDEC_MIX_GAIN_6DB_Q15		16384		//Q(15) 0.5
#define CAL_PCMDEC_MIX_GAIN_3DB_Q31		1520203673  //Q(31) 0.7079
#define CAL_PCMDEC_MIX_GAIN_6DB_Q31		1073741823  //Q(31) 0.5

#define NEX_MAX_PCM_SIZE (1024*8)
#define NEX_PCM_NO_MEMCPY (1)

//#define _DUMP_AUDIO_OUTPUT_DUMP_

static const unsigned int BD_LPCM_ChannelAssign[16] =
{
/*  ch       ,        idx       ,   channel Info.						*/
	0,				/* 0 */
	1,				/* 1 */		/* mono */
	0,				/* 2 */
	2,				/* 3 */		/* stereo  : L,R						*/
	3,				/* 4 */		/* 3/0     : L,R,C						*/
	3,				/* 5 */		/* 2/1     : L,R,S						*/
	4,				/* 6 */		/* 3/1     : L,R,C,S					*/
	4,				/* 7 */		/* 2/2     : L,R,Ls,Rs					*/
	5,				/* 8 */		/* 3/2     : L,R,C,Ls,Rs				*/
	6,				/* 9 */		/* 3/2+lfe : L,R,C,Ls,Rs,LFE			*/
	7,				/* 10 */	/* 3/4     : L,R,C,Ls,rLs,rRs,Rs		*/
	8,				/* 11 */	/* 3/4+lfe : L,R,C,Ls,rLs,rRs,Rs,LFE	*/
	0, 0, 0, 0
};

#define NEX_SWAP(a, b)	{ a ^= b; b ^= a; a ^= b; }

typedef enum
{
	NEXCALBODY_AUDIO_MULTICHANNEL_NONE = 0x0,			// Don't Use Multi channel

	NEXCALBODY_AUDIO_MULTICHANNEL_ANY_TO_STEREO = 0x1,			// Don't Use Multi channel
	
	NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1 = 0x2,		// 5.1 channel ���ϴ� Stereo �� ����, 5.1 channel �̻��� 5.1 channel �� ���� //G2 �䱸 ����.
	NEXCALBODY_AUDIO_MULTICHANNEL_ANY_TO_ANY = 0x20,				// Multi Channel Full ���� 
	
}NXAudioMultiChOutputType;

typedef struct
{
	char    sPCMBuffer[NEX_MAX_PCM_SIZE*4];
} NEXPCMInfo, *PNEXPCMInfo;

unsigned int nexCALBody_Audio_PCM_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* pUserData )
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
		default:
			*puValue = 0;
			break;
	}

	return 0;
}

unsigned int nexCALBody_Audio_PCM_SetProperty( unsigned int uProperty, NXINT64 uValue, void* pUserData )
{
	return 0;
}

unsigned int nexCALBody_Audio_PCM_Init (	unsigned int uCodecObjectTypeIndication,
										unsigned char* pConfig, int iConfigLen,
										unsigned char* pFrame, int iFrameLen,
										void *pInitInfo,
										void *pExtraInfo,
										unsigned int* piSamplingRate,
										unsigned int* piNumOfChannels,
										unsigned int* puBitsPerSample,
										unsigned int* piNumOfSamplesPerChannel,
										unsigned int uMode,
										unsigned int uUserDataType,
										void** ppUserData )
{
	PNEXADECUsrData pAudioUsrData = NULL;
	PNEXPCMInfo pPCMInfo;

	pAudioUsrData = (PNEXADECUsrData)nexCAL_MemAlloc( sizeof(NEXADECUsrData) );
	if (!pAudioUsrData)
	{
		nexCAL_DebugPrintf("Error Memory Allocation\n");
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}
	
	pPCMInfo = (PNEXPCMInfo)nexCAL_MemAlloc( sizeof(NEXPCMInfo) );
	if (!pPCMInfo)
	{
		nexCAL_DebugPrintf("Error Memory Allocation\n");
		nexCAL_MemFree( pAudioUsrData );
		*ppUserData = NULL;
		return NEXCAL_ERROR_FAIL;
	}

	memset( pAudioUsrData, 0x00, sizeof(NEXADECUsrData) );
	memset( pPCMInfo, 0x00, sizeof(NEXPCMInfo) );
	pAudioUsrData->m_puExtraData = (void*)pPCMInfo;

	if (uCodecObjectTypeIndication == eNEX_CODEC_A_BLURAYPCM)
	{
		*piNumOfChannels = BD_LPCM_ChannelAssign[ ((pFrame[2] & 0xF0) >> 4) ];
		if (!(*piNumOfChannels))
			return NEXCAL_ERROR_FAIL;

		*piSamplingRate = (pFrame[2] & 0x0F);
		if (*piSamplingRate == 1)
			*piSamplingRate = 48000;
		else if (*piSamplingRate == 4)
			*piSamplingRate = 96000;
		else if (*piSamplingRate == 5)
			*piSamplingRate = 192000;
		else
			return NEXCAL_ERROR_FAIL;

		*puBitsPerSample = (pFrame[3] & 0xC0) >> 6;
		if (*puBitsPerSample == 1)
			*puBitsPerSample = 16;
		else if (*puBitsPerSample == 2)
			*puBitsPerSample = 20;
		else if (*puBitsPerSample == 3)
			*puBitsPerSample = 24;
		else
			return NEXCAL_ERROR_FAIL;

		/* skip config header for samples per channel */
		iFrameLen -= 4;
	}

	if (0 == *puBitsPerSample)
	{
		*puBitsPerSample = 16;
		nexCAL_DebugPrintf("nexCALBody_Audio_PCM_Init.. puBitsPerSample is 0. ");
	}

	if ((0 == *puBitsPerSample) || (0 == *piNumOfChannels))
	{
		nexCAL_DebugPrintf("PCMDEC Init Failed!! *puBitsPerSample:%d, *piNumOfChannels:%d\n", *puBitsPerSample, *piNumOfChannels);
		return NEXCAL_ERROR_FAIL;
	}

	nexCAL_DebugPrintf("PCMDEC Init samplingrate:%d, channel:%d, sampleperchannel:%d\n", *piSamplingRate, *piNumOfChannels, *piNumOfSamplesPerChannel );

	if (*puBitsPerSample==0)
	{
		nexCAL_DebugPrintf("puBitsPerSample value is 0\n");
		*puBitsPerSample = 16;
	}
	else if (*puBitsPerSample < 8)
	{
		nexCAL_DebugPrintf("Multiply *puBitsPerSample(%d) and 8 because *puBitsPerSample is under 8.\n", __LINE__, *puBitsPerSample);
		*puBitsPerSample <<= 3;
	}

	nexCAL_DebugPrintf("PCMDEC Init pFrame:0x%08x, iFrameLen:%d, BitsPerSample:%d\n", pFrame, iFrameLen, *puBitsPerSample );
	*piNumOfSamplesPerChannel = iFrameLen * 8 / (*puBitsPerSample) / (*piNumOfChannels);

	pAudioUsrData->m_uNumOfChannels = *piNumOfChannels;

#ifdef NEX_PCM_MONO_TO_STEREO
	*piNumOfChannels = 2;
#else
	if (*piNumOfChannels > 2)
		*piNumOfChannels = 2;
#endif

	if (((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1) && (pAudioUsrData->m_uNumOfChannels > 4))
		|| ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_ANY_TO_ANY) && (pAudioUsrData->m_uNumOfChannels > 2)))
	{
		if (pAudioUsrData->m_uNumOfChannels == 1)
		{
			*piNumOfChannels = 2;
		}
		else if ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1)
				 && pAudioUsrData->m_uNumOfChannels == 5)
		{
			*piNumOfChannels = 6;
		}
		else
		{
			*piNumOfChannels = pAudioUsrData->m_uNumOfChannels;
		}
	}

	if ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1) && (pAudioUsrData->m_uNumOfChannels == 7 ||pAudioUsrData->m_uNumOfChannels == 8))
	{
		*piNumOfChannels = 6;
	}
//	nexCAL_DebugPrintf("PCMDEC Init *piNumOfChannels:%d, %d, %d\n", *piNumOfChannels, pAudioUsrData->m_uNumOfChannels, *puBitsPerSample );

	pAudioUsrData->m_uSamplingRate = *piSamplingRate;
	pAudioUsrData->m_uBitsPerSample = *puBitsPerSample;

	if (!pAudioUsrData->m_puAudio24bitEnable)
	{
		if (*puBitsPerSample != 16)
			*puBitsPerSample = 16;
	}

	pAudioUsrData->m_uNumOfSamplesPerChannel = *piNumOfSamplesPerChannel;
	pAudioUsrData->m_uAudioOTI = uCodecObjectTypeIndication;

	*ppUserData = (void*)pAudioUsrData;

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_INPUT, pConfig, iConfigLen, uCodecObjectTypeIndication, (unsigned int*)pAudioUsrData);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpInit(NEXCALBODY_AUDIO_OUTPUT, NULL, 0, uCodecObjectTypeIndication, (unsigned int*)pAudioUsrData);
#endif

	return 0;
}

unsigned int nexCALBody_Audio_PCM_Deinit( void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;

	if (pAudioUsrData)
	{
		PNEXPCMInfo pPCMInfo = (PNEXPCMInfo)pAudioUsrData->m_puExtraData;
		unsigned int temp = pAudioUsrData->m_puAudio24bitEnable;
		unsigned int tempChannelSupport = pAudioUsrData->m_puMultiChannelSupport;

		nexCAL_MemFree( pPCMInfo );
		memset(pAudioUsrData, 0x00, sizeof(NEXADECUsrData));
		pAudioUsrData->m_puAudio24bitEnable = temp;
		pAudioUsrData->m_puMultiChannelSupport = tempChannelSupport;

		nexCAL_MemFree( pAudioUsrData );		
	}
	else
	{
		nexCAL_DebugPrintf("PCM Decoder Handle is NULL.\n");
	}

#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_INPUT);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpDeInit(NEXCALBODY_AUDIO_OUTPUT);
#endif

	return 0;
}

unsigned int nexCALBody_Audio_PCM_Dec( unsigned char* pSource, int iLen, void *pExtraInfo, void* pDest, int* piWrittenPCMSize, unsigned int uDTS, unsigned int* puOutputTime, int nFlag, unsigned int* puDecodeResult, void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;
	PNEXPCMInfo pPCMInfo;
	unsigned int uActualSize, uChanIdx;
	unsigned int i;
	int nOutBps;

#ifdef NEX_PCM_NO_MEMCPY
	unsigned int uMaxPCMSize = *piWrittenPCMSize;
#else
	unsigned int uMaxPCMSize = NEX_MAX_PCM_SIZE;
#endif

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

	if (pAudioUsrData->m_uBitsPerSample >= 24)
	{
		if (pAudioUsrData->m_puAudio24bitEnable)
		{
			// 24 bits PCM ����� ���� ��� ����.
			nOutBps = 32;
#ifdef NEX_FLAC_WAV_24BIT_AND_8BIT_PADDING_NOT_USE
			nOutBps = 24;
#endif
		}
		else
		{
			nOutBps = 16;
		}

/*
		if( (bOPT_OUTBPS_S24 == 1) && (bOPT_OUTBPS_S32_24 == 1))
			nOutBps = 32;
		else if((bOPT_OUTBPS_S24 == 1) && (bOPT_OUTBPS_S32_24 == 0))
			nOutBps = 24;
		else
			nOutBps = 16;
*/
	}
	else
	{
		nOutBps = 16;
	}

	pPCMInfo = (PNEXPCMInfo)pAudioUsrData->m_puExtraData;
	if (pPCMInfo == NULL) return 1;

#ifndef NEX_PCM_NO_MEMCPY
	if (pAudioUsrData->m_uWrittenPCMSize)
	{
		if (pAudioUsrData->m_uWrittenPCMSize <= uMaxPCMSize)
		{
			memcpy( pDest, pPCMInfo->sPCMBuffer, pAudioUsrData->m_uWrittenPCMSize );
			*piWrittenPCMSize = pAudioUsrData->m_uWrittenPCMSize;
			pAudioUsrData->m_uWrittenPCMSize = 0;
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
		}
		else
		{
			memcpy( pDest, pPCMInfo->sPCMBuffer, uMaxPCMSize );
			*piWrittenPCMSize = uMaxPCMSize;
			pAudioUsrData->m_uWrittenPCMSize -= uMaxPCMSize;
			memmove ( pPCMInfo->sPCMBuffer, pPCMInfo->sPCMBuffer + uMaxPCMSize, pAudioUsrData->m_uWrittenPCMSize );
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
		}
		return 0;
	}
#endif

	if(nFlag == NEXCAL_ADEC_FLAG_END_OF_STREAM)
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);
		return 0;
	}

	if (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM)
	{
		/* skip config header */
		iLen    -= 4;
		pSource += 4;

	}

	//uActualSize �� ���ü�
	uActualSize = iLen * 8 / pAudioUsrData->m_uBitsPerSample;

	if( (pAudioUsrData->m_uBitsPerSample == 32) || (pAudioUsrData->m_uBitsPerSample == 64) )
	{
		//BigEndian -> littleEndian
		if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_IN32BE) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL32BE) )
		{
			NXUINT16 *pPCM16;
			NXUINT8 *pPCM8 = (NXUINT8*) pSource;
			pPCM16 = (NXUINT16*)pPCM8;

			//0 1 2 3 -> 1 0 3 2
			for(i=0; i<iLen; i+=2)
				NEX_SWAP(pPCM8[i],pPCM8[i+1]);
			//1 0 3 2 -> 3 2 1 0
			for(i=0; i<iLen/2; i+=2)
				NEX_SWAP(pPCM16[i],pPCM16[i+1]);
		}
		else if(pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL64BE)
		{
			NXUINT16 *pPCM16;
			NXUINT32 *pPCM32;
			NXUINT8 *pPCM8 = (NXUINT8*) pSource;
			pPCM16 = (NXUINT16*)pPCM8;
			pPCM32 = (NXUINT32*)pPCM8;

			//0 1 2 3 4 5 6 7 -> 1 0 3 2 5 4 7 6
			for(i=0; i<iLen; i+=2)
				NEX_SWAP(pPCM8[i],pPCM8[i+1]);
			//1 0 3 2 5 4 7 6 -> 3 2 1 0 7 6 5 4
			for(i=0; i<iLen/2; i+=2)
				NEX_SWAP(pPCM16[i],pPCM16[i+1]);
			//3 2 1 0 7 6 5 4 -> 7 6 5 4 3 2 1 0
			for(i=0; i<iLen/4; i+=2)
				NEX_SWAP(pPCM32[i],pPCM32[i+1]);

		}
		//FL32 or FL64 to S32
		if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL32LE) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL32BE) )
		{
			NXINT32 *pPCM32 = (NXINT32*) pSource;
			NXFLOAT *pPCMFL = (NXFLOAT*) pSource;
			NXINT64 iPCM64;
			for(i=0; i<iLen/4; i++)
			{
				iPCM64 = (NXINT64)(pPCMFL[i] * 2147483648LL);
				if(iPCM64 > (NXINT64)0x000000007fffffff)
					pPCM32[i] = 2147483647LL;
				else if(iPCM64 < (NXINT64)0xffffffff80000000)
					pPCM32[i] = -2147483648LL;
				else
					pPCM32[i] = iPCM64;
			}
		}
		else if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL64LE) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_FL64BE))
		{
			NXINT32 j=0;
			NXINT64 iPCM64;
			NXDOUBLE *pPCMFL64 = (NXDOUBLE*)pSource;
			NXINT32 *pPCM32 = (NXINT32*) pSource;

			for(i=0; i<iLen/8; i++)
			{
				iPCM64 = (NXINT64)(pPCMFL64[i] * 2147483648LL);
				if(iPCM64 > (NXINT64)0x000000007fffffff)
					pPCM32[j++] = 2147483647LL;
				else if(iPCM64 < (NXINT64)0xffffffff80000000)
					pPCM32[j++] = -2147483648LL;
				else
					pPCM32[j++] = iPCM64;
			}
			iLen = iLen/2; //8byte->4byte size
		}
		
		switch (nOutBps)
		{
			case 32:
				{
					NXINT32 *pOutPCM32 = (NXINT32*) pDest;
					NXINT32 *pPCM32 = (NXINT32*) pSource;

#ifdef NEX_PCM_MONO_TO_STEREO
					if (pAudioUsrData->m_uNumOfChannels == 1)
					{
						for (i = iLen; i != 0; i -= 4)
						{
							/* little-endian */
							*pOutPCM32++ = *pPCM32;
							*pOutPCM32++ = *pPCM32;
							pPCM32 += 1;
						}
					}
					else
#endif
					{
						for (i = iLen; i != 0; i -= 4)
							*pOutPCM32++ = *pPCM32++; /* little-endian */
					}
				}
				*piWrittenPCMSize = uActualSize * 4;
				break;
			case 24:
				{
					NXUINT8 *pOutPCM24 = (NXUINT8*) pDest;
					NXUINT8 *pPCM8 = (NXUINT8*) pSource;

#ifdef NEX_PCM_MONO_TO_STEREO
					if (pAudioUsrData->m_uNumOfChannels == 1)
					{
						for (i = 0; i<iLen;)
						{
							/* little-endian */
							//Left
							i++;
							*pOutPCM24++ = pPCM8[i++];
							*pOutPCM24++ = pPCM8[i++];
							*pOutPCM24++ = pPCM8[i++];
							//Ritht
							i -=3;
							*pOutPCM24++ = pPCM8[i++];
							*pOutPCM24++ = pPCM8[i++];
							*pOutPCM24++ = pPCM8[i++];
						}
					}
					else
#endif
					{
						for (i = iLen; i != 0; i -= 4)
						{
							pPCM8++;
							*pOutPCM24++ = *pPCM8++; /* little-endian */
							*pOutPCM24++ = *pPCM8++;
							*pOutPCM24++ = *pPCM8++;
						}
					}

				}
				*piWrittenPCMSize = uActualSize * 3;
				break;

			case 16:
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT32 *pPCM32 = (NXINT32*) pSource;

#ifdef NEX_PCM_MONO_TO_STEREO
					if (pAudioUsrData->m_uNumOfChannels == 1)
					{
						for (i = iLen; i != 0; i -= 4)
						{
							/* little-endian */
							*pPCM16++ = (short) ( ((int)(*pPCM32)) >> 16 );
							*pPCM16++ = (short) ( ((int)(*pPCM32)) >> 16 );
							pPCM32 += 1;
						}
					}
					else
#endif
					{
						for (i = iLen; i != 0; i -= 4)
						{
							/* little-endian */
							*pPCM16++ = (short) ( ((int)(*pPCM32)) >> 16 );
							pPCM32 += 1;
						}
					}
				}

				*piWrittenPCMSize = uActualSize * 2;
				break;

		}


		if (*piWrittenPCMSize)
		{
			if (*piWrittenPCMSize <= uMaxPCMSize)
			{
				pAudioUsrData->m_uWrittenPCMSize = 0;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			}
			else
			{
#ifdef NEX_PCM_NO_MEMCPY
				nexCAL_DebugPrintf("[nexCalBody_PCM %d] Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
				pAudioUsrData->m_uWrittenPCMSize = *piWrittenPCMSize - uMaxPCMSize;
				memcpy( pPCMInfo->sPCMBuffer, pDest, pAudioUsrData->m_uWrittenPCMSize );
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#endif
			}
		}
		else
		{
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		}
	}
	else if (pAudioUsrData->m_uBitsPerSample ==24)
	{
		switch (nOutBps)
		{
			case 32:
				{
					NXUINT8 *pOutPCM32 = (NXUINT8*) pDest;
					NXUINT8 *pPCM8 = (NXUINT8*) pSource;

					if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_IN24BE) )
					{
						/* big-endian */
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								//pDest�� integer�� casting ���
								//*pOutPCM32++ = (int) ( 0x00 | (pPCM8[i+2] << 8) | (pPCM8[i+1] << 16) | (pPCM8[i] << 24));
								//*pOutPCM32++ = (int) ( 0x00 | (pPCM8[i+2] << 8) | (pPCM8[i+1] << 16) | (pPCM8[i] << 24));
								//Left
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
								//Right
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
							{
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
							}
						}
					}
					else
					{
						/* little-endian */
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								//pOutPCM32 integer�� casting ���
								//*pOutPCM32++ = (int) ( 0x00 | (pPCM8[i] << 8) | (pPCM8[i+1] << 16) | (pPCM8[i+2] << 24));
								//*pOutPCM32++ = (int) ( 0x00 | (pPCM8[i] << 8) | (pPCM8[i+1] << 16) | (pPCM8[i+2] << 24));
								//Left
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
								//Right
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
							{
								*pOutPCM32++ = 0;
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
							}
						}
					}

					*piWrittenPCMSize = uActualSize * 4;

				}
				break;

			case 24:
				{
					NXUINT8 *pOutPCM32 = (NXUINT8*) pDest;
					NXUINT8 *pPCM8 = (NXUINT8*) pSource;

					if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_IN24BE) )
					{
						/* big-endian to Little-endian output*/
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								//Left
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
								//Right
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
							{
								*pOutPCM32++ = pPCM8[i+2];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i];
							}
						}
					}
					else
					{
						/* little-endian */
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								//Left
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
								//Right
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
							{
								//using memcpy ???
								//memcpy(pOutPCM32, pPCM8, iLen);
								*pOutPCM32++ = pPCM8[i];
								*pOutPCM32++ = pPCM8[i+1];
								*pOutPCM32++ = pPCM8[i+2];
							}
						}
					}
					*piWrittenPCMSize = uActualSize * 3;

				}
				break;

			case 16:
				{
					short *pPCM16 = (short*) pDest;

					if( (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_IN24BE) )
					{
						/* big-endian */
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
								*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
								*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
						}
					}
					else
					{
						/* little-endian */
#ifdef NEX_PCM_MONO_TO_STEREO
						if (pAudioUsrData->m_uNumOfChannels == 1)
						{
							for (i=0; i<iLen; i+=3)
							{
								*pPCM16++ = (short) ( (pSource[i+2]<<8) | pSource[i+1] );
								*pPCM16++ = (short) ( (pSource[i+2]<<8) | pSource[i+1] );
							}
						}
						else
#endif
						{
							for (i=0; i<iLen; i+=3)
								*pPCM16++ = (short) ( (pSource[i+2]<<8) | pSource[i+1] );
						}
					}

					*piWrittenPCMSize = uActualSize * 2;
				}
				break;

		}

		if (*piWrittenPCMSize)
		{
			if (*piWrittenPCMSize <= uMaxPCMSize)
			{
				pAudioUsrData->m_uWrittenPCMSize = 0;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			}
			else
			{
#ifdef NEX_PCM_NO_MEMCPY
				nexCAL_DebugPrintf("[nexCalBody_PCM %d] Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
				pAudioUsrData->m_uWrittenPCMSize = *piWrittenPCMSize - uMaxPCMSize;
				memcpy( pPCMInfo->sPCMBuffer, pDest, pAudioUsrData->m_uWrittenPCMSize );
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#endif
			}
		}
		else
		{
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		}
	}
	else if (pAudioUsrData->m_uBitsPerSample == 20)
	{
		short *pPCM16 = (short*) pDest;

		if (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM)
		{
			/* big-endian, 2 samples (40 bits) are interleaved */
#ifdef NEX_PCM_MONO_TO_STEREO
			if (pAudioUsrData->m_uNumOfChannels == 1)
			{
				for (i=0; i<iLen; i+=5)
				{
					*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
					*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
					*pPCM16++ = (short) ((pSource[i+2] & 0x0F)<<12) | (pSource[i+3] << 4) | ((pSource[i+4] & 0xF0) >> 4);
					*pPCM16++ = (short) ((pSource[i+2] & 0x0F)<<12) | (pSource[i+3] << 4) | ((pSource[i+4] & 0xF0) >> 4);
				}
			}
			else
#endif
			{
				for (i=0; i<iLen; i+=5)
				{
					*pPCM16++ = (short) ( (pSource[i]<<8) | pSource[i+1] );
					*pPCM16++ = (short) ((pSource[i+2] & 0x0F)<<12) | (pSource[i+3] << 4) | ((pSource[i+4] & 0xF0) >> 4);
				}
			}
		}
		else
		{
			return NEXCAL_ERROR_FAIL;
		}

		*piWrittenPCMSize = uActualSize * 2;

		if (*piWrittenPCMSize)
		{
			if (*piWrittenPCMSize <= uMaxPCMSize)
			{
				pAudioUsrData->m_uWrittenPCMSize = 0;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			}
			else
			{
#ifdef NEX_PCM_NO_MEMCPY
				nexCAL_DebugPrintf("[nexCalBody_PCM %d] Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
				pAudioUsrData->m_uWrittenPCMSize = *piWrittenPCMSize - uMaxPCMSize;
				memcpy( pPCMInfo->sPCMBuffer, pDest, pAudioUsrData->m_uWrittenPCMSize );
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#endif
			}
		}
		else
		{
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		}
	}
	else if (pAudioUsrData->m_uBitsPerSample == 16)
	{
		if ((pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_BLURAYPCM) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_S16BE) || (pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_LPCMBE))
		{
			short *pPCM16 = (short*) pDest;

			for (i=0; i<iLen; i+=2)
				NEX_SWAP(pSource[i], pSource[i+1]);
		}

		*piWrittenPCMSize = uActualSize *2;

		if (*piWrittenPCMSize)
		{
			if (*piWrittenPCMSize <= uMaxPCMSize)
			{
#ifdef NEX_PCM_MONO_TO_STEREO
				if (pAudioUsrData->m_uNumOfChannels == 1)
				{
					short *pPCM16 = (short*) pDest;
					short *pSRC = (short*) pSource;

					for (i = (*piWrittenPCMSize); i != 0; i -= 2)
					{
						*pPCM16++ = *pSRC;
						*pPCM16++ = *pSRC++;
					}
				}
				else
#endif
				{
					memcpy( pDest, pSource, *piWrittenPCMSize );
				}
				pAudioUsrData->m_uWrittenPCMSize = 0;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			}
			else
			{
#ifdef NEX_PCM_MONO_TO_STEREO
				if (pAudioUsrData->m_uNumOfChannels == 1)
				{
					short *pPCM16 = (short*) pDest;
					short *pSRC = (short*) pSource;

					for (i = uMaxPCMSize; i != 0; i -= 2)
					{
						*pPCM16++ = *pSRC;
						*pPCM16++ = *pSRC++;
					}
				}
				else
#endif
				{
					memcpy( pDest, pSource, uMaxPCMSize );
				}

#ifdef NEX_PCM_NO_MEMCPY
				nexCAL_DebugPrintf("[nexCalBody_PCM %d] Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
				pAudioUsrData->m_uWrittenPCMSize = *piWrittenPCMSize - uMaxPCMSize;
				memcpy( pPCMInfo->sPCMBuffer, pSource, pAudioUsrData->m_uWrittenPCMSize );
				*piWrittenPCMSize = uMaxPCMSize;
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#endif
			}
		}
		else
		{
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		}
	}
	else if (pAudioUsrData->m_uBitsPerSample ==8)
	{
		if (iLen)
		{
			short *pPCM16 = (short*) pDest;
			int PCM8;

#ifdef NEX_PCM_MONO_TO_STEREO
			if (pAudioUsrData->m_uNumOfChannels == 1)
			{
				for (i=0; i<iLen && i*2<uMaxPCMSize; i++)
				{
					/* 8bit PCM is unsigned 8bit */
					/* (sample8 - 128) << 8 */
					if(pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_RAW)
						PCM8 = (int) (((int)pSource[i]) - 128);
					else
						PCM8 = (int) (((int)pSource[i]));
					*pPCM16++ = (short) (PCM8 << 8);
					*pPCM16++ = (short) (PCM8 << 8);
				}
			}
			else
#endif
			{
				for (i=0; i<iLen && i*2<uMaxPCMSize; i++)
				{
					/* 8bit PCM is unsigned 8bit */
					/* (sample8 - 128) << 8 */
					if(pAudioUsrData->m_uAudioOTI == eNEX_CODEC_A_PCM_RAW)
						PCM8 = (int) (((int)pSource[i]) - 128);
					else
						PCM8 = (int) (((int)pSource[i]));
					*pPCM16++ = (short) (PCM8 << 8);
				}
			}

			*piWrittenPCMSize = uActualSize * 2;
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
			pAudioUsrData->m_uWrittenPCMSize = ( iLen - i ) * 2;

			if (0 == pAudioUsrData->m_uWrittenPCMSize)
			{
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
			}
			else
			{
#ifdef NEX_PCM_NO_MEMCPY
				nexCAL_DebugPrintf("[nexCalBody_PCM %d] Out of MaxPCMSize !!! %d, %d \n", __LINE__, uMaxPCMSize, *piWrittenPCMSize);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
#else
				pPCM16 = (short*) pPCMInfo->sPCMBuffer;
				for (; i<iLen; i++)
				{
					/* 8bit PCM is unsigned 8bit */
					/* (sample8 - 128) << 8 */
					PCM8 = (int) (((int)pSource[i]) - 128);
					*pPCM16++ = (short) (PCM8 << 8);
				}
#endif
			}
		}
		else
		{
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		}
	}
	else
	{
		NEXCAL_DEL_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
	}

	// bit ���� �Ϸ�

	//piWrittenPCMSize ��� ����Ʈ ũ��




	if ((pAudioUsrData->m_puMultiChannelSupport == NEXCALBODY_AUDIO_MULTICHANNEL_NONE)
		|| ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1) && (pAudioUsrData->m_uNumOfChannels < 5))
		|| ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_ANY_TO_ANY) && (pAudioUsrData->m_uNumOfChannels <= 2))
		|| (pAudioUsrData->m_puAudio24bitEnable && (nOutBps == 32 || nOutBps ==24)))
	{

		*piWrittenPCMSize *= 2;
		/* channel down-mix */
		switch (pAudioUsrData->m_uNumOfChannels)
		{
			case 1 :
#ifndef NEX_PCM_MONO_TO_STEREO
				*piWrittenPCMSize /= 2;
#endif
			case 2 :
				break;

			case 3 :
				{
					switch (nOutBps)
					{
						case 32:
							{
								NXINT32     *pPCM32 = (NXINT32*) pDest;
								NXINT32     *pMixedPCM = (NXINT32*) pDest;
								NXINT64     iPCM64 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

									iPCM64 = ( pPCM32[i+0] + CenterGain );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;

									iPCM64 = ( pPCM32[i+1] + CenterGain );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;
								}
							}
							break;
						case 24:
							{
								int j = 0;
								//NXUINT8 tmpPCM8[8192 *4/*int*/ * 3/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
								NXUINT8 *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 3);


								NXUINT8     *pPCM8 = (NXUINT8*) pDest;
								NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
								NXINT64     iPCM64 = 0;
								NXINT32     *pTmpPCM32;
								NXINT32     iPCM32 ;
								NXUINT8     *piPCM8;

								piPCM8      = (NXUINT8 *)&iPCM32;

								for (i=0,j=0; i< uActualSize; i++)
								{
									tmpPCM8[j++] = 0;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
								}

								pTmpPCM32 = (NXINT32 *)&tmpPCM8;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

									iPCM64 = ( pTmpPCM32[i+0] + CenterGain );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];


									iPCM64 = ( pTmpPCM32[i+1] + CenterGain );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];
								}
								nexCAL_MemFree( tmpPCM8 );
							}
							break;

						case 16:
							{
								NXINT16 *pPCM16 = (NXINT16*) pDest;
								NXINT16 *pMixedPCM = (NXINT16*) pDest;
								NXINT32   iPCM32 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15);

									iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;

									iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;
								}
							}
							break;
					}
				}
				break;

			case 4 :
				{
					switch (nOutBps)
					{
						case 32:
							{
								NXINT32     *pPCM32 = (NXINT32*) pDest;
								NXINT32     *pMixedPCM = (NXINT32*) pDest;
								NXINT64     iPCM64 = 0;

								for (i=0; i<uActualSize; i += 4)
								{
									NXINT32 SurroundVal = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

									iPCM64 = ( pPCM32[i+0] + SurroundVal );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;

									SurroundVal = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+3]) >> 31);

									iPCM64 = ( pPCM32[i+1] + SurroundVal );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;
								}
							}
							break;
						case 24:
							{
								int j = 0;
								//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 4/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
								NXUINT8     *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 4);
								NXUINT8     *pPCM8 = (NXUINT8*) pDest;
								NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
								NXINT64     iPCM64 = 0;
								NXINT32     *pTmpPCM32;
								NXINT32     iPCM32 ;
								NXUINT8     *piPCM8;

								piPCM8      = (NXUINT8 *)&iPCM32;

								for (i=0,j=0; i< uActualSize; i++)
								{
									tmpPCM8[j++] = 0;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
								}

								pTmpPCM32 = (NXINT32 *)&tmpPCM8;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 SurroundVal = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

									iPCM64 = ( pTmpPCM32[i+0] + SurroundVal );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];


									SurroundVal = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+3]) >> 31);

									iPCM64 = ( pTmpPCM32[i+1] + SurroundVal );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];
								}
								nexCAL_MemFree( tmpPCM8 );
							}
							break;

						case 16:
							{
								NXINT16 *pPCM16 = (NXINT16*) pDest;
								NXINT16 *pMixedPCM = (NXINT16*) pDest;
								NXINT32   iPCM32 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									iPCM32 = ( ((NXINT32)pPCM16[i+0]) + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;

									iPCM32 = ( ((NXINT32)pPCM16[i+1]) + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+3]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;
								}
							}
							break;
					}
				}
				break;

			case 5 :		// 5 ch : L,R,C,Ls,Rs
				{
					switch (nOutBps)
					{
						case 32:
							{
								NXINT32     *pPCM32 = (NXINT32*) pDest;
								NXINT32     *pMixedPCM = (NXINT32*) pDest;
								NXINT64     iPCM64 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

									iPCM64 = ( ((NXINT64)pPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+3]) ) >> 31) );
									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;

									iPCM64 = ( ((NXINT64)pPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+4]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;
								}
							}
							break;
						case 24:
							{
								int j = 0;
								//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 6/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
								NXUINT8     *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 6);
								NXUINT8     *pPCM8 = (NXUINT8*) pDest;
								NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
								NXINT64     iPCM64 = 0;
								NXINT32     *pTmpPCM32;
								NXINT32     iPCM32 ;
								NXUINT8     *piPCM8;

								piPCM8      = (NXUINT8 *)&iPCM32;

								for (i=0,j=0; i< uActualSize; i++)
								{
									tmpPCM8[j++] = 0;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
								}

								pTmpPCM32 = (NXINT32 *)&tmpPCM8;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

									iPCM64 = ( ((NXINT64)pTmpPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+3]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];

									iPCM64 = ( ((NXINT64)pTmpPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+4]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];
								}
								nexCAL_MemFree( tmpPCM8 );
							}
							break;

						case 16:
							{
								NXINT16 *pPCM16 = (NXINT16*) pDest;
								NXINT16 *pMixedPCM = (NXINT16*) pDest;
								NXINT32   iPCM32 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

									iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+3]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;

									iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+4]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;
								}
							}
							break;
					}
				}
				break;
			case 6 :	/* ignore LFE */ // 5.1 ch �� : L,R,C,LFE,Ls,Rs
				{
					switch( nOutBps )
					{
					case 32:
						{
							NXINT32		*pPCM32 = (NXINT32*) pDest;
							NXINT32		*pMixedPCM = (NXINT32*) pDest;
							NXINT64		iPCM64 = 0;

							for(i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
							{
								NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

								iPCM64 = ( ((NXINT64)pPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+4]) ) >> 31) );
								if(iPCM64 > (NXINT64)0x000000007fffffff)
									*pMixedPCM++ = 2147483647LL;
								else if(iPCM64 < (NXINT64)0xffffffff80000000)
									*pMixedPCM++ = -2147483648LL;
								else
									*pMixedPCM++ = (NXINT32)iPCM64;

								iPCM64 = ( ((NXINT64)pPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+5]) ) >> 31) );

								if(iPCM64 > (NXINT64)0x000000007fffffff)
									*pMixedPCM++ = 2147483647LL;
								else if(iPCM64 < (NXINT64)0xffffffff80000000)
									*pMixedPCM++ = -2147483648LL;
								else
									*pMixedPCM++ = (NXINT32)iPCM64;

								
							}
						}
						break;
					case 24:
						{
							int j = 0;
							//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 6/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
							NXUINT8		*tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 6);
							NXUINT8		*pPCM8 = (NXUINT8*) pDest;
							NXUINT8		*pMixedPCM = (NXUINT8*) pDest;
							NXINT64		iPCM64 = 0;
							NXINT32		*pTmpPCM32; 
							NXINT32		iPCM32 ;
							NXUINT8		*piPCM8;
							
							piPCM8		= (NXUINT8 *)&iPCM32;


							for(i=0,j=0; i< uActualSize; i++ )
							{
								tmpPCM8[j++] = 0;
								tmpPCM8[j++] = *pPCM8++;
								tmpPCM8[j++] = *pPCM8++;
								tmpPCM8[j++] = *pPCM8++;
							}
							pTmpPCM32 = (NXINT32 *)&tmpPCM8;

							for(i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
							{
								NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

								iPCM64 = ( ((NXINT64)pTmpPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+4]) ) >> 31) );

								if(iPCM64 > (NXINT64)0x000000007fffffff)
									iPCM32 = 2147483647LL;
								else if(iPCM64 < (NXINT64)0xffffffff80000000)
									iPCM32 = -2147483648LL;
								else
									iPCM32 = (NXINT32)iPCM64;

								*pMixedPCM++ = piPCM8[1];
								*pMixedPCM++ = piPCM8[2];
								*pMixedPCM++ = piPCM8[3];

								iPCM64 = ( ((NXINT64)pTmpPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+5]) ) >> 31) );

								if(iPCM64 > (NXINT64)0x000000007fffffff)
									iPCM32 = 2147483647LL;
								else if(iPCM64 < (NXINT64)0xffffffff80000000)
									iPCM32 = -2147483648LL;
								else
									iPCM32 = (NXINT32)iPCM64;

								*pMixedPCM++ = piPCM8[1];
								*pMixedPCM++ = piPCM8[2];
								*pMixedPCM++ = piPCM8[3];
							}
							nexCAL_MemFree( tmpPCM8 );
						}
						break;

					case 16:
						{
							NXINT16 *pPCM16 = (NXINT16*) pDest;
							NXINT16 *pMixedPCM = (NXINT16*) pDest;
							NXINT32	  iPCM32 = 0;

							for(i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
							{
								NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

								iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+4]) ) >> 15) );
								if(iPCM32 > 32767)
									*pMixedPCM++ = 32767;
								else if(iPCM32 < -32768)
									*pMixedPCM++ = -32768;
								else
									*pMixedPCM++ = (NXINT16)iPCM32;

								iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+5]) ) >> 15) );
								if(iPCM32 > 32767)
									*pMixedPCM++ = 32767;
								else if(iPCM32 < -32768)
									*pMixedPCM++ = -32768;
								else
									*pMixedPCM++ = (NXINT16)iPCM32;
							}
						}
						break;
					}
				}
				break;

			case 7 :	/* ignore rLs, rRs */			/* 3/4     : L,R,C,Ls,rLs,rRs,Rs		*/
			case 8 :	/* ignore rLs, rRs, LFE */		/* 3/4+lfe : L,R,C,Ls,rLs,rRs,Rs,LFE	*/
				{
					switch (nOutBps)
					{
						case 32:
							{
								NXINT32     *pPCM32 = (NXINT32*) pDest;
								NXINT32     *pMixedPCM = (NXINT32*) pDest;
								NXINT64     iPCM64 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

									iPCM64 = ( ((NXINT64)pPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+3]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;

									iPCM64 = ( ((NXINT64)pPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+6]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										*pMixedPCM++ = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										*pMixedPCM++ = -2147483648LL;
									else
										*pMixedPCM++ = (NXINT32)iPCM64;
								}
							}
							break;
						case 24:
							{
								int j = 0;
								//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 8/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
								NXUINT8     *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 8);
								NXUINT8     *pPCM8 = (NXUINT8*) pDest;
								NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
								NXINT64     iPCM64 = 0;
								NXINT32     *pTmpPCM32;
								NXINT32     iPCM32 ;
								NXUINT8     *piPCM8;

								piPCM8      = (NXUINT8 *)&iPCM32;

								for (i=0,j=0; i< uActualSize; i++)
								{
									tmpPCM8[j++] = 0;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
									tmpPCM8[j++] = *pPCM8++;
								}

								pTmpPCM32 = (NXINT32 *)&tmpPCM8;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

									iPCM64 = ( ((NXINT64)pTmpPCM32[i+0]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+3]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];

									iPCM64 = ( ((NXINT64)pTmpPCM32[i+1]) + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+6]) ) >> 31) );

									if (iPCM64 > (NXINT64)0x000000007fffffff)
										iPCM32 = 2147483647LL;
									else if (iPCM64 < (NXINT64)0xffffffff80000000)
										iPCM32 = -2147483648LL;
									else
										iPCM32 = (NXINT32)iPCM64;

									*pMixedPCM++ = piPCM8[1];
									*pMixedPCM++ = piPCM8[2];
									*pMixedPCM++ = piPCM8[3];
								}
								nexCAL_MemFree( tmpPCM8 );
							}
							break;

						case 16:
							{
								NXINT16 *pPCM16 = (NXINT16*) pDest;
								NXINT16 *pMixedPCM = (NXINT16*) pDest;
								NXINT32   iPCM32 = 0;

								for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
								{
									NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

									iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+3]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;

									iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+6]) ) >> 15) );
									if (iPCM32 > 32767)
										*pMixedPCM++ = 32767;
									else if (iPCM32 < -32768)
										*pMixedPCM++ = -32768;
									else
										*pMixedPCM++ = (NXINT16)iPCM32;
								}
							}
							break;
					}
				}
				break;

			default :
				break;
		}

		*piWrittenPCMSize /= pAudioUsrData->m_uNumOfChannels;
	}
	else //��Ƽä�� ���
	{
		/*
		if(pAudioUsrData->m_uNumOfChannels == 1)
		{
#ifndef NEX_PCM_MONO_TO_STEREO
			*piWrittenPCMSize /= 2;
#endif
		}

		else */

		if (pAudioUsrData->m_uNumOfChannels == 5)
		{
			*piWrittenPCMSize *= 6;
			//*piWrittenPCMSize *= 3;
			switch (nOutBps)
			{
				case 32:
					{
						NXINT32     *pPCM32 = (NXINT32*) pDest;
						NXINT32     *pMixedPCM = (NXINT32*) pDest;
						NXINT32     *iPCM32Temp = (NXINT32 *)nexCAL_MemAlloc(8192 * 4 * 6);
						int count = 0;

						for (i=0; i<uActualSize; i +=  pAudioUsrData->m_uNumOfChannels)
						{
							//L
							iPCM32Temp[count++] = pPCM32[i+0];
							//R
							iPCM32Temp[count++] = pPCM32[i+1];
							//C
							iPCM32Temp[count++] = pPCM32[i+2];
							//Lfe
							iPCM32Temp[count++] = 0;
							//Ls
							iPCM32Temp[count++] = pPCM32[i+3];
							//Rs
							iPCM32Temp[count++] = pPCM32[i+4];
						}

						for (i=0; i<count; i++)
						{
							*pMixedPCM++ = iPCM32Temp[i];
						}
						nexCAL_MemFree( iPCM32Temp );

					}
					break;
#if 1
	case 24:
				{
					NXUINT8		*tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 6); //MaxBufferSize * sizeof(int) * channel 
					NXUINT8		*pPCM8 = (NXUINT8*) pDest;
					NXUINT8		*pMixedPCM = (NXUINT8*) pDest;
					NXINT32		*pTmpPCM32; 
					int count = 0;

					for(i=0 ; i< uActualSize;  i += pAudioUsrData->m_uNumOfChannels )
					{
						//L
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						//R
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						//C
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						//LFE
						tmpPCM8[count++] = 0;
						tmpPCM8[count++] = 0;
						tmpPCM8[count++] = 0;
						//LS
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						//RS
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;
						tmpPCM8[count++] = *pPCM8++;

					}
					
					for(i=0; i<count; i++)
					{
						*pMixedPCM++ = tmpPCM8[i];
					}

					nexCAL_MemFree( tmpPCM8 );
				
				}
				break;
#else
				case 24:
					{
						int j = 0;
						//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 8/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
						NXUINT8     *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 6);
						NXUINT8     *pPCM8 = (NXUINT8*) pDest;
						NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
						NXINT64     iPCM64 = 0;
						NXINT32     *pTmpPCM32;
						NXINT32     iPCM32 ;
						NXUINT8     *piPCM8;
						//NXUINT8		iPCM8Temp[8192 * 4* 8 * 2];
						NXUINT8     *iPCM8Temp = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 6);
						int count = 0;


						piPCM8      = (NXUINT8 *)&iPCM32;


						for (i=0,j=0; i< uActualSize; i++)
						{
							tmpPCM8[j++] = 0;
							tmpPCM8[j++] = *pPCM8++;
							tmpPCM8[j++] = *pPCM8++;
							tmpPCM8[j++] = *pPCM8++;
						}

						pTmpPCM32 = (NXINT32 *)&tmpPCM8;

						for (i=0; i<uActualSize ; i += pAudioUsrData->m_uNumOfChannels)
						{
							//L
							iPCM32 = pTmpPCM32[i+0];
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];

							//R
							iPCM32 = pTmpPCM32[i+1];
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];

							//C
							iPCM32 = pTmpPCM32[i+2];
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];

							//Lfe
							iPCM32 = 0;
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];

							//Ls
							iPCM32 = pTmpPCM32[i+3];
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];

							//Rs
							iPCM32 = pTmpPCM32[i+4];
							iPCM8Temp[count++] = piPCM8[1];
							iPCM8Temp[count++] = piPCM8[2];
							iPCM8Temp[count++] = piPCM8[3];
						}

						for (i=0; i<count; i++)
						{
							*pMixedPCM++ = iPCM8Temp[i];
						}

						nexCAL_MemFree( tmpPCM8 );
						nexCAL_MemFree( iPCM8Temp );

					}
					break;
#endif
				case 16:
					{
						NXINT16 *pPCM16 = (NXINT16*) pDest;
						NXINT16 *pMixedPCM = (NXINT16*) pDest;
						NXINT16   iPCM16 = 0;
						//NXINT16		iPCM16Temp[8192 * 6 * 2];
						NXINT16     *iPCM16Temp = (NXINT16 *)nexCAL_MemAlloc(8192 * 4 * 6 );
						int count = 0;

						for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
						{
							//L
							//iPCM16 = pPCM16[i+0];
							iPCM16Temp[count++] =  pPCM16[i+0];

							//R
							//iPCM16 = pPCM16[i+1];
							iPCM16Temp[count++] = pPCM16[i+1];

							//C
							//iPCM16 = pPCM16[i+2];
							iPCM16Temp[count++] = pPCM16[i+2];

							//Lfe
							//iPCM16 = 0;
							iPCM16Temp[count++] = 0;

							//Ls
							//iPCM16 = pPCM16[i+3];
							iPCM16Temp[count++] = pPCM16[i+3];

							//Rs
							//iPCM16 = pPCM16[i+4];
							iPCM16Temp[count++] = pPCM16[i+4];
						}

						for (i=0; i<count; i++)
						{
							*pMixedPCM++ = iPCM16Temp[i];
						}
						nexCAL_MemFree( iPCM16Temp );
					}
					break;

				default :
					break;
			}
			*piWrittenPCMSize /= pAudioUsrData->m_uNumOfChannels;
		}
		else if ((pAudioUsrData->m_puMultiChannelSupport & NEXCALBODY_AUDIO_MULTICHANNEL_5POINT1_MORE_TO_5POINT1) && (pAudioUsrData->m_uNumOfChannels == 7 ||pAudioUsrData->m_uNumOfChannels == 8))
		{
			//*piWrittenPCMSize *= 3;	//���� output pcm size �� 6ch �̿��� ��, ���� �ڵ忡�� �̹� 2ch output �� ����  pcm size *2 �� �� �����̹Ƿ� *3 = 6 ���� ����
			*piWrittenPCMSize *= 6;	//���� output pcm size �� 6ch �̿��� ��

// 7 /* ignore rLs, rRs */			/* 3/4     : L,R,C,Ls,Rs,rLs,rRs		*/
// 8 /* ignore rLs, rRs, LFE */		/* 3/4+lfe : L,R,C,LFE,Ls,Rs,rLs,rRs,	*/ // 5.1 ch �� : L,R,C,LFE,Ls,Rs
			switch (nOutBps)
			{
				case 32:
					{
						NXINT32     *pPCM32 = (NXINT32*) pDest;
						NXINT32     *pMixedPCM = (NXINT32*) pDest;
						NXINT64     iPCM64 = 0;

						for (i=0; i<uActualSize; i +=  pAudioUsrData->m_uNumOfChannels)
						{
							//NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+2]) >> 31);

							//L
							iPCM64 = (NXINT64)pPCM32[i+0];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+3]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;

							//R
							iPCM64 = (NXINT64)pPCM32[i+1];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+6]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;

							//C
							iPCM64 = (NXINT64)pPCM32[i+2];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pPCM32[i+6]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;
							//Lfe
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = 0;
							else
								iPCM64 = (NXINT64)pPCM32[i+3];

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;

							//Ls
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = (NXINT64)pPCM32[i+3] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+5] ) >> 31);
							else
								iPCM64 = (NXINT64)pPCM32[i+4] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+6] ) >> 31);

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;

							//Rs
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = (NXINT64)pPCM32[i+4] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+6] ) >> 31);
							else
								iPCM64 = (NXINT64)pPCM32[i+5] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pPCM32[i+7] ) >> 31);

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								*pMixedPCM++ = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								*pMixedPCM++ = -2147483648LL;
							else
								*pMixedPCM++ = (NXINT32)iPCM64;
						}
					}
					break;
				case 24:
					{
						int j = 0;
						//NXUINT8		tmpPCM8[8192 *4/*sizeof(int)*/ * 8/*ch*/]; //24bps�ٿ� �ͽ� 32 bit �������� ����,  ���� stack -> heap memory ��....pjb test
						NXUINT8     *tmpPCM8 = (NXUINT8 *)nexCAL_MemAlloc(8192 * 4 * 8);
						NXUINT8     *pPCM8 = (NXUINT8*) pDest;
						NXUINT8     *pMixedPCM = (NXUINT8*) pDest;
						NXINT64     iPCM64 = 0;
						NXINT32     *pTmpPCM32;
						NXINT32     iPCM32 ;
						NXUINT8     *piPCM8;

						piPCM8      = (NXUINT8 *)&iPCM32;

						//for(i=0; i< uActualSize * 3; i++ ) // i �� �Է��� Samples
						//for(j=0; j< uActualSize * 3; ) // j ����� Byte ��
						for (i=0,j=0; i< uActualSize; i++)
						{
							tmpPCM8[j++] = 0;
							tmpPCM8[j++] = *pPCM8++;
							tmpPCM8[j++] = *pPCM8++;
							tmpPCM8[j++] = *pPCM8++;
						}

						pTmpPCM32 = (NXINT32 *)&tmpPCM8;

						for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
						{
							//NXINT32 CenterGain = (NXINT32)( ((NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+2]) >> 31);

							//L
							iPCM64 = (NXINT64)pTmpPCM32[i+0];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+3]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];

							//R
							iPCM64 = (NXINT64)pTmpPCM32[i+1];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+6]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];

							//C
							iPCM64 = (NXINT64)pTmpPCM32[i+2];// + CenterGain + ((( (NXINT64)CAL_PCMDEC_MIX_GAIN_6DB_Q31 * (NXINT64)pTmpPCM32[i+6]) ) >> 31) );

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];

							//Lfe
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = 0;
							else
								iPCM64 = (NXINT64)pTmpPCM32[i+3];

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];

							//Ls
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = (NXINT64)pTmpPCM32[i+3] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+5] ) >> 31);
							else
								iPCM64 = (NXINT64)pTmpPCM32[i+4] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+6] ) >> 31);

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];

							//Rs
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM64 = (NXINT64)pTmpPCM32[i+4] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+6] ) >> 31);
							else
								iPCM64 = (NXINT64)pTmpPCM32[i+5] + (( (NXINT64)CAL_PCMDEC_MIX_GAIN_3DB_Q31 * (NXINT64)pTmpPCM32[i+7] ) >> 31);

							if (iPCM64 > (NXINT64)0x000000007fffffff)
								iPCM32 = 2147483647LL;
							else if (iPCM64 < (NXINT64)0xffffffff80000000)
								iPCM32 = -2147483648LL;
							else
								iPCM32 = (NXINT32)iPCM64;

							*pMixedPCM++ = piPCM8[1];
							*pMixedPCM++ = piPCM8[2];
							*pMixedPCM++ = piPCM8[3];
						}
						nexCAL_MemFree( tmpPCM8 );
					}
					break;

				case 16:
					{
						NXINT16 *pPCM16 = (NXINT16*) pDest;
						NXINT16 *pMixedPCM = (NXINT16*) pDest;
						NXINT32   iPCM32 = 0;

						for (i=0; i<uActualSize; i += pAudioUsrData->m_uNumOfChannels)
						{
							//NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

							//L
							iPCM32 = (NXINT32)pPCM16[i+0];// + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+3]) ) >> 15) );

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32768)
								*pMixedPCM++ = -32768;
							else
								*pMixedPCM++ = (NXINT16)iPCM32;

							//R
							iPCM32 = (NXINT32)pPCM16[i+1];// + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+6]) ) >> 15) );

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32768)
								*pMixedPCM++ = -32768;
							else
								*pMixedPCM++ = (NXINT16)iPCM32;

							//C
							iPCM32 = (NXINT32)pPCM16[i+2];

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32767)
								*pMixedPCM++ = -32767;
							else
								*pMixedPCM++ = (NXINT32)iPCM32;

							//Lfe
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM32 = 0;
							else
								iPCM32 = (NXINT32)pPCM16[i+3];

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32767)
								*pMixedPCM++ = -32767;
							else
								*pMixedPCM++ = (NXINT32)iPCM32;

							//Ls
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM32 = (NXINT32)pPCM16[i+3] + (( (NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * (NXINT32)pPCM16[i+5] ) >> 15);
							else
								iPCM32 = (NXINT32)pPCM16[i+4] + (( (NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * (NXINT32)pPCM16[i+6] ) >> 15);

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32767)
								*pMixedPCM++ = -32767;
							else
								*pMixedPCM++ = (NXINT32)iPCM32;

							//Rs
							if (pAudioUsrData->m_uNumOfChannels == 7)
								iPCM32 = (NXINT32)pPCM16[i+4] + (( (NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * (NXINT32)pPCM16[i+6] ) >> 15);
							else
								iPCM32 = (NXINT32)pPCM16[i+5] + (( (NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * (NXINT32)pPCM16[i+7] ) >> 15);

							if (iPCM32 > 32767)
								*pMixedPCM++ = 32767;
							else if (iPCM32 < -32767)
								*pMixedPCM++ = -32767;
							else
								*pMixedPCM++ = (NXINT32)iPCM32;
						}
					}
					break;

				default :
					break;
			}

			*piWrittenPCMSize /= pAudioUsrData->m_uNumOfChannels;
		}
		/*
		else
		{
			*piWrittenPCMSize /= 2;
		}
		*/
	}
#ifdef _DUMP_AUDIO_INPUT_DUMP_
	nexCALBody_DumpDecode(NEXCALBODY_AUDIO_INPUT, pSource, iLen, uDTS);
#endif

#ifdef _DUMP_AUDIO_OUTPUT_DUMP_
	nexCALBody_DumpDecode(NEXCALBODY_AUDIO_OUTPUT, (unsigned char *)pDest, *piWrittenPCMSize , uDTS);
#endif

	if ((*piWrittenPCMSize%2) != 0)
	{
		*piWrittenPCMSize += 1;
	}

	return NEXCAL_ERROR_NONE;
}

unsigned int nexCALBody_Audio_PCM_Reset( void* pUserData )
{
	PNEXADECUsrData pAudioUsrData = (PNEXADECUsrData)pUserData;
	pAudioUsrData->m_uWrittenPCMSize = 0;
	nexCAL_DebugPrintf("[nexCALBody_Audio_PCM_Reset %d] nexCALBody_Audio_PCM_Reset() %d\n", __LINE__, pAudioUsrData->m_puAudio24bitEnable);

	return 0;
}

unsigned int nexCALBody_Audio_PCM_GetInfo	( unsigned int uIndex
												, unsigned int* puResult
												, void* pUserData )
{

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] nexCALBody_Audio_PCM_GetInfo called!\n", __LINE__);
	return 0;
}

//};
