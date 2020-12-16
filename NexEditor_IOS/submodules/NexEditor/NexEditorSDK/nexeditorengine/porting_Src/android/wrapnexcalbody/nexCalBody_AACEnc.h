#ifndef _CALBODY_AAC_ENC_
#define _CALBODY_AAC_ENC_

#ifdef __cplusplus
extern "C" {
#endif

unsigned int nexCALBody_Audio_AACEnc_Init ( unsigned int uCodecObjectTypeIndication
											, unsigned char** ppConfig
											, int* piConfigLen
											, unsigned int uSamplingRate
											, unsigned int uNumOfChannels
											, int          iBitrate
											, void** puUserData );	

unsigned int nexCALBody_Audio_AACEnc_Deinit ( void* uUserData );



unsigned int nexCALBody_Audio_AACEnc_Encode ( unsigned char* pData
											, int iLen
											, unsigned char** ppOutData
											, unsigned int* piOutLen
											, unsigned int* puEncodeResult
											, void* uUserData );

unsigned int nexCALBody_Audio_AACEnc_Reset ( void* uUserData );

unsigned int nexCALBody_Audio_AACEnc_GetProperty(unsigned int uProperty, NXINT64 *puValue, void* uUserData );
unsigned int nexCALBody_Audio_AACEnc_SetProperty(unsigned int uProperty, NXINT64 uValue, void* uUserData );

#ifdef __cplusplus
}
#endif

#endif		// _CALBODY_AAC_ENC_

