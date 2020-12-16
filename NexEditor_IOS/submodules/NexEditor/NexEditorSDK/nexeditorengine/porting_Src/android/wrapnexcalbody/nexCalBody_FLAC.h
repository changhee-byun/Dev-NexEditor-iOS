#ifndef _CALBODY_FLAC_
#define _CALBODY_FLAC_

#ifdef __cplusplus
extern "C" {
#endif
 
unsigned int nexCALBody_Audio_FLAC_Init (	unsigned int uCodecObjectTypeIndication,
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
											void** ppUserData );

unsigned int nexCALBody_Audio_FLAC_Deinit( void* pUserData);

unsigned int nexCALBody_Audio_FLAC_Dec( unsigned char* pSource, 
											int iLen, 
											void* pExtraInfo,
											void* pDest, 
											int* piWrittenPCMSize,
											unsigned int uDTS, 
											unsigned int *puOutputTime,
											int nFlag,
											unsigned int* puDecodeResult, 
											void* pUserData );

unsigned int nexCALBody_Audio_FLAC_Reset( void* pUserData );

unsigned int nexCALBody_Audio_FLAC_GetInfo	( unsigned int uIndex
												, unsigned int* puResult
												, void* pUserData );


unsigned int nexCALBody_Audio_FLAC_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* pUserData );
unsigned int nexCALBody_Audio_FLAC_SetProperty( unsigned int uProperty, NXINT64 uValue, void* pUserData );

#ifdef __cplusplus
}
#endif

#endif		// _CALBODY_FLAC_
