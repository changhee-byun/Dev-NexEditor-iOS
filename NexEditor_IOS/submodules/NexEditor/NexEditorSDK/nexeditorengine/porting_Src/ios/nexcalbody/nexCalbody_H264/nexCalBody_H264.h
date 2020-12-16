#ifndef _CALBODY_H264_HEADER_
#define _CALBODY_H264_HEADER_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

NXINT32 nexCALBody_Video_AVC_Init( NEX_CODEC_TYPE eCodecType, NXUINT8* pConfig, NXINT32 iLen, 
									   NXUINT8* pConfigEnhance, NXINT32 iEnhLen, NXVOID* pInitInfo, NXVOID* pExtraInfo, NXINT32 iNALHeaderLengthSize, NXINT32* piWidth,
									   NXINT32* piHeight, NXINT32* piPitch, NXUINT32 uMode, NXUINT32 uUserDataType,  NXVOID **ppUserData );							   
NXINT32 nexCALBody_Video_AVC_Close( NXVOID *pUserData );
NXINT32 nexCALBody_Video_AVC_Dec( NXUINT8* pData, NXINT32 iLen, NXVOID* pExtraInfo, NXUINT32 uDTS, NXUINT32  uPTS, NXINT32 iFlag, 
									  NXUINT32* puDecodeResult, NXVOID *pUserData );
NXINT32 nexCALBody_Video_AVC_GetOutput( NXUINT8** ppBits1, NXUINT8** ppBits2, NXUINT8** ppBits3, NXUINT32 *puPTS, NXVOID *pUserData );
NXINT32 nexCALBody_Video_AVC_Reset( NXVOID *pUserData );		
NXINT32 nexCALBody_Video_AVC_GetInfo(NXUINT32 uIndex, NXUINT32* puResult, NXVOID *pUserData);

NXINT32 nexCALBody_Video_AVC_GetProperty( NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData );
NXINT32 nexCALBody_Video_AVC_SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData );

#ifdef __cplusplus
}
#endif

#endif	// _CALBODY_H264_HEADER_
