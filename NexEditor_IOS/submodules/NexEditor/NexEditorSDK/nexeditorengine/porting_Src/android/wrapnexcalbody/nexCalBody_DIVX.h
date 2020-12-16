#ifndef _CALBODY_DIVX_
#define _CALBODY_DIVX_

#if ! defined(NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE)

#	if ! defined(NEXCAL_PROPERTY_USER_START)
#	include "nexCAL.h"
#	endif

#define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE									(NEXCAL_PROPERTY_USER_START+0x700)
#		define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE_NORMAL						0
#		define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE_THUMBNAIL					1

#endif


#ifdef __cplusplus
extern "C" {
#endif

unsigned int nexCALBody_Video_DIVX_Init(	unsigned int uCodecObjectTypeIndication,
												unsigned char* pConfig,
												int iLen,
												unsigned char* pConfigEnhance,
												int iEnhLen,
												void *pInitInfo,
												void *pExtraInfo,
												int iNALHeaderLengthSize,
												int* piWidth,
												int* piHeight,
												int* piPitch,
												int a_uMode, 
												int a_uUserDataType, 
												void** puUserData );

unsigned int nexCALBody_Video_DIVX_Close( void* uUserData );

unsigned int nexCALBody_Video_DIVX_Dec(	unsigned char* pData,
												int iLen,
												void *pExtraInfo,
												unsigned int uDTS,
												unsigned int uPTS,
												int iEnhancement, 
												unsigned int* puDecodeResult,
												void* uUserData );
									  
unsigned int nexCALBody_Video_DIVX_GetOutput( unsigned char** ppBits1, unsigned char** ppBits2, unsigned char** ppBits3, unsigned int *pDTS, void* uUserData );

unsigned int nexCALBody_Video_DIVX_Reset( void* uUserData );

unsigned int nexCALBody_Video_DIVX_GetProperty( unsigned int uProperty, NXINT64* puValue, void* uUserData );

unsigned int nexCALBody_Video_DIVX_SetProperty( unsigned int uProperty, NXINT64 uValue, void* uUserData );

#ifdef __cplusplus
}
#endif


#endif

