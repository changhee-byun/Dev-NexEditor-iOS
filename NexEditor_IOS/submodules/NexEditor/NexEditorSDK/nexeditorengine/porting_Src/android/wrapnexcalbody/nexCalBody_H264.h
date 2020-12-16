#ifndef _CALBODY_H264_
#define _CALBODY_H264_

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

//namespace android {
unsigned int nexCALBody_Video_AVC_Init(	unsigned int uCodecObjectTypeIndication, 
												unsigned char* pConfig, 
												int iLen, 
												unsigned char* pConfigEnhance, 
												int iEnhLen, 
												void *pInitInfo,
												void *pExtraInfo,
												int uNALHeaderLen4DSI, 
												int* piWidth,
												int* piHeight, 
												int* piPitch, 
												int a_uMode, 
												int a_uUserDataType, 
												void** puUserData );

unsigned int nexCALBody_Video_AVC_Close( void* uUserData );

unsigned int nexCALBody_Video_AVC_Dec(	unsigned char* pData, 
												int iLen, 
												void *pExtraInfo,
												unsigned int uDTS, 
												unsigned int uPTS, 
												int nFlag, 
												unsigned int* puDecodeResult, 
												void* uUserData );
									  
unsigned int nexCALBody_Video_AVC_GetOutput( 	unsigned char** ppY, 
														unsigned char** ppU, 
														unsigned char** ppV, 
														unsigned int *puPTS, 
														void* uUserData );

unsigned int nexCALBody_Video_AVC_Reset( void* uUserData );

unsigned int nexCALBody_Video_AVC_GetProperty( unsigned int uProperty, NXINT64 *puValue, void* uUserData );
unsigned int nexCALBody_Video_AVC_SetProperty( unsigned int uProperty, NXINT64 uValue, void* uUserData );
//};

#ifdef __cplusplus
}
#endif

#endif

