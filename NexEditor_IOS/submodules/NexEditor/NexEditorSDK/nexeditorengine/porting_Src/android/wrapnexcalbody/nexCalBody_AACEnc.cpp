#include "nexCalBody.h"
#include "NexCAL.h"
#include "NexMediaDef.h"
#include "NxAACEncAPI.h"
#include "nexCalBody_AACEnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define		__FILENAME__ "nexCalBody_AACEnc.cpp"

//namespace android {
typedef struct AudioEncoderInfo
{
	unsigned int	m_bInit;
	PNXAACENCHANDLE m_hEncoder;
	unsigned int m_uSamplingRate;
	unsigned int m_uChannels;
	unsigned int m_uNumOfSamplesPerChannel;
} AudioEncoderInfo;



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// AAC
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned int nexCALBody_Audio_AACEnc_Init ( unsigned int uCodecObjectTypeIndication
										, unsigned char** ppConfig
										, int* piConfigLen
										, unsigned int uSamplingRate
										, unsigned int uNumOfChannels
										, int          iBitrate
										, void** puUserData )
{
	nexCAL_DebugPrintf("Start AAC Encoder open");
	
	AudioEncoderInfo *pEncInfo = (AudioEncoderInfo *)nexCAL_MemAlloc(sizeof(AudioEncoderInfo));
	if(pEncInfo == NULL)
	{
		nexCAL_DebugPrintf("[nexCALBody_Audio_AACEnc_Init] AudioEncoderInfo malloc fail. \n");
		return -1;
	}
	
	memset(pEncInfo, 0, sizeof(	AudioEncoderInfo ));
	
	pEncInfo->m_hEncoder = NxAACEncOpen();
	if(pEncInfo->m_hEncoder)
	{
		pEncInfo->m_hEncoder->pOutputBitStream = (unsigned char*)nexCAL_MemAlloc(15000);

		pEncInfo->m_hEncoder->nSampleRate = uSamplingRate;
		pEncInfo->m_hEncoder->nChannels = uNumOfChannels;		
		pEncInfo->m_hEncoder->nBitRate = iBitrate;
		nexCAL_DebugPrintf("Start Init!");
		int error = NxAACEncInit(pEncInfo->m_hEncoder);

		*ppConfig = pEncInfo->m_hEncoder->pConfig;
		*piConfigLen = pEncInfo->m_hEncoder->nConfigLen;

		nexCAL_DebugPrintf("Init complete!, ConfigLen(%d)", pEncInfo->m_hEncoder->nConfigLen);
		nexCAL_MemDump(pEncInfo->m_hEncoder->pConfig, pEncInfo->m_hEncoder->nConfigLen);
		
		*puUserData = (void*)pEncInfo;
		
		return 0;
	}
	else
	{
		nexCAL_DebugPrintf("AAC Encoder create failed!");
	}
	return -1;	
}																					
																			

unsigned int nexCALBody_Audio_AACEnc_Deinit ( void* pUserData )
{
	AudioEncoderInfo* pAEI = (AudioEncoderInfo *)pUserData;

	if(pAEI == NULL)
	{
		nexCAL_DebugPrintf("[CALBody.cpp, %d] AudioInfo is NULL!! nexCALBody_Audio_AACEnc_Deinit !!\n", __LINE__);
		return 0;
	}

	nexCAL_DebugPrintf("[CALBody.cpp, %d] nexCALBody_Audio_AACEnc_Deinit. ", __LINE__ );
	
	NxAACEncClose(pAEI->m_hEncoder);

	if(pAEI->m_hEncoder->pOutputBitStream)
	{
		//nexCAL_MemFree(pAEI->m_hEncoder->pOutputBitStream);
	}
	
	nexCAL_MemFree(pAEI);
	return -1;	
}



unsigned int nexCALBody_Audio_AACEnc_Encode ( unsigned char* pData
											, int iLen
											, unsigned char** ppOutData
											, unsigned int* piOutLen
											, unsigned int* puEncodeResult
											, void* pUserData )
{
	AudioEncoderInfo* pAEI = (AudioEncoderInfo *)pUserData;

	if(pAEI == NULL)
	{
		nexCAL_DebugPrintf("[CALBody.cpp, %d] AudioInfo is NULL!! nexCALBody_Audio_AACEnc_Encode !!\n", __LINE__);
		return 0;
	}

	//nexCAL_DebugPrintf("[CALBody.cpp, %d] nexCALBody_Audio_AACEnc_Encode. ", __LINE__ );
	
	NEXCAL_INIT_AENC_RET(*puEncodeResult);

	NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_ENCODING_SUCCESS);

	if(pAEI->m_hEncoder && iLen)
	{
		int error = 0;

		pAEI->m_hEncoder->pInputPCM = (short *)pData;
		pAEI->m_hEncoder->nInputPCMLength = iLen;

		//nexCAL_DebugPrintf("Encode AAC : 0x%x(%d)", pAEI->m_hEncoder->pInputPCM, pAEI->m_hEncoder->nInputPCMLength);
		error = NxAACEncEncode(pAEI->m_hEncoder);
		//nexCAL_DebugPrintf("Encode AAC : 0x%x(%d)", pAEI->m_hEncoder->pInputPCM, pAEI->m_hEncoder->nInputPCMLength);
		//nexCAL_DebugPrintf("Encode AAC ret : %d(%d))", error, pAEI->m_hEncoder->nEncodedLength);

		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_NEXT_INPUT);
		if(pAEI->m_hEncoder->nEncodedLength)
		{
			//nexCAL_DebugPrintf("[%s %d] AAC Encoding complete! write length is %d", __FILENAME__, __LINE__, pAEI->m_hEncoder->nEncodedLength);
					
			*ppOutData = pAEI->m_hEncoder->pOutputBitStream;
			*piOutLen = pAEI->m_hEncoder->nEncodedLength;	

			NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_OUTPUT_EXIST);
			return 0;		
		}
	}
	else
	{
		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_EOS);
	}
	return -1;	
}																						

unsigned int nexCALBody_Audio_AACEnc_Reset ( void* uUserData )
{
	return 0;	
}

unsigned int nexCALBody_Audio_AACEnc_GetProperty(unsigned int uProperty, NXINT64 *puValue, void* pUserData )
{
	return 0;
}

unsigned int nexCALBody_Audio_AACEnc_SetProperty(unsigned int uProperty, NXINT64 uValue, void* pUserData )
{
	return 0;
}

//};

