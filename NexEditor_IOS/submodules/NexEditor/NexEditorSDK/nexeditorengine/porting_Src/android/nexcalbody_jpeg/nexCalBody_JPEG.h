#ifndef _CALBODY_JPEG_
#define _CALBODY_JPEG_

#ifdef __cplusplus
extern "C" {
#endif

NXINT32 nexCALBody_Image_JPEG_GetProperty( NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData );
NXINT32 nexCALBody_Image_JPEG_SetProperty( NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData );
NXINT32 nexCALBody_Image_JPEG_getHeader(	NEX_CODEC_TYPE a_eCodecType, 
													NXCHAR *a_strFileName, 
													NXUINT8 *a_pSrc, 
													NXUINT32 a_uSrcLen, 
													NXINT32 *a_pnWidth, 
													NXINT32 *a_pnHeight, 
													NXINT32 *a_pnPitch, 
													NXUINT32 a_uUserDataType, 
													NXVOID **a_ppUserData);
NXINT32 nexCALBody_Image_JPEG_Decoder(		NXCHAR *a_strFileName, 
														NXUINT8 *a_pSrc, 
														NXUINT32 a_uSrcLen, 
														NXINT32 *a_pnWidth, 
														NXINT32 *a_pnHeight, 
														NXINT32 *a_pnPitch, 
														NXUINT8 *a_pBits1, 
														NXUINT8 *a_pBits2, 
														NXUINT8 *a_pBits3, 
														NXUINT32 *a_puDecodeResult, 
														NXVOID *a_pUserData);

#ifdef __cplusplus
}
#endif

#endif // _CALBODY_JPEG_

