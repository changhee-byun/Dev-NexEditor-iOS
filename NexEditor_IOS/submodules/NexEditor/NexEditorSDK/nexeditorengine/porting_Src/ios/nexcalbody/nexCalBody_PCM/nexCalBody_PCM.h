#ifndef _CALBODY_PCM_
#define _CALBODY_PCM_

#ifdef __cplusplus
extern "C" {
#endif
 
NXINT32 nexCALBody_Audio_PCM_Init (	NEX_CODEC_TYPE uCodecObjectTypeIndication,
											NXUINT8* pConfig, NXINT32 iConfigLen,
											NXUINT8* pFrame, NXINT32 iFrameLen,
											NXVOID *pInitInfo,
											NXVOID *pExtraInfo,
											NXUINT32* piSamplingRate,
											NXUINT32* piNumOfChannels,
											NXUINT32* puBitsPerSample,
											NXUINT32* piNumOfSamplesPerChannel,
											NXUINT32 uMode,
											NXUINT32 uUserDataType,
											NXVOID** ppUserData );

NXINT32 nexCALBody_Audio_PCM_Deinit( NXVOID* pUserData);

NXINT32 nexCALBody_Audio_PCM_Dec( NXUINT8* pSource,
											NXINT32 iLen,
											NXVOID* pExtraInfo,
											NXVOID* pDest,
											NXINT32* piWrittenPCMSize,
											NXUINT32 uDTS,
											NXUINT32* puOutputTime,
											NXINT32 nFlag,
											NXUINT32* puDecodeResult,
											NXVOID* pUserData );

NXINT32 nexCALBody_Audio_PCM_Reset( NXVOID* pUserData );

NXINT32 nexCALBody_Audio_PCM_GetInfo	( NXUINT32 uIndex
												, NXUINT32* puResult
												, void* pUserData );


NXINT32 nexCALBody_Audio_PCM_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* pUserData );
NXINT32 nexCALBody_Audio_PCM_SetProperty( unsigned int uProperty, NXINT64 uValue, void* pUserData );

#ifdef __cplusplus
}
#endif

#endif		// _CALBODY_PCM_

