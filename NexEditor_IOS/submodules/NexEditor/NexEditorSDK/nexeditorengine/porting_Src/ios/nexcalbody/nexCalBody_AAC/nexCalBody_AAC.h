#ifndef _CALBODY_AAC_HEADER_
#define _CALBODY_AAC_HEADER_

//#ifdef __cplusplus
//extern "C" {
//#endif

// rooney common api
void* nex_malloc( int iSize );
void nex_free( void *p );

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
										, NXVOID **ppUserData );
NXINT32 nexCALBody_Audio_AAC_Deinit( NXVOID *pUserData );
NXINT32 nexCALBody_Audio_AAC_Dec( NXUINT8* pSource, NXINT32 iLen, NXVOID* pExtraInfo, NXVOID* pDest, NXINT32* piWrittenPCMSize, NXUINT32 uDTS, NXUINT32 *puOutTS, NXINT32 nFlag,  NXUINT32* puDecodeResult, NXVOID *pUserData );
NXINT32 nexCALBody_Audio_AAC_Reset( NXVOID *pUserData );
NXINT32 nexCALBody_Audio_AAC_SetProperty( NXUINT32 uProperty , NXINT64 uValue , NXVOID *pUserData );
NXINT32 nexCALBody_Audio_AAC_GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData );										

//#ifdef __cplusplus
//}
//#endif

#endif	// _CALBODY_AAC_HEADER_