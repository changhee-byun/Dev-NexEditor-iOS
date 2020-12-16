#ifndef _CALBODY_MPEG2_HEADER_
#define _CALBODY_MPEG2_HEADER_

#include <stdlib.h>
NXINT32 nexCALBody_Video_MPEG2_Init( NEX_CODEC_TYPE eCodecType, NXUINT8* pConfig, NXINT32 iLen, 
									   NXUINT8* pConfigEnhance, NXINT32 iEnhLen, NXVOID *pInitInfo, NXVOID *pExtraInfo, NXINT32 iNALHeaderLengthSize, NXINT32* piWidth, 
									   NXINT32* piHeight, NXINT32* piPitch, NXUINT32 uMode, NXUINT32 uUserDataType,  NXVOID **ppUserData );
NXINT32 nexCALBody_Video_MPEG2_Close( NXVOID *pUserData );
NXINT32 nexCALBody_Video_MPEG2_Dec( NXUINT8* pData, NXINT32 iLen, NXVOID *pExtraInfo, NXUINT32 uDTS, NXUINT32  uPTS, NXINT32 iFlag, 
									  NXUINT32* puDecodeResult, NXVOID *pUserData );
NXINT32 nexCALBody_Video_MPEG2_GetOutput( NXUINT8** ppBits1, NXUINT8** ppBits2, NXUINT8** ppBits3, NXUINT32 *puPTS, NXVOID *pUserData );
NXINT32 nexCALBody_Video_MPEG2_Reset( NXVOID *pUserData );

#endif	// _CALBODY_MPEG2_HEADER_