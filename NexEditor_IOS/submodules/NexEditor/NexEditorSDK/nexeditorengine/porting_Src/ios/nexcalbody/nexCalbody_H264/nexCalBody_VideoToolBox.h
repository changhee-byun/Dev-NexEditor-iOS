/******************************************************************************
 * File Name   :	nexCalBody_VideoToolBox.h
 * Description :	The definition of AVC Decoder CAL Body
                    Using VideoToolbox in iOS 8.0 above
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef _CALBODY_VIDEOTOOLBOX_HEADER_
#define _CALBODY_VIDEOTOOLBOX_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

NXINT32 nexCALBody_Video_VTB_Init( NEX_CODEC_TYPE eCodecType, NXUINT8* pConfig, NXINT32 iLen,
									   NXUINT8* pConfigEnhance, NXINT32 iEnhLen, NXVOID* pInitInfo, NXVOID* pExtraInfo, NXINT32 iNALHeaderLengthSize, NXINT32* piWidth,
									   NXINT32* piHeight, NXINT32* piPitch, NXUINT32 uMode, NXUINT32 uUserDataType,  NXVOID **ppUserData );							   
NXINT32 nexCALBody_Video_VTB_Close( NXVOID *pUserData );
NXINT32 nexCALBody_Video_VTB_Dec( NXUINT8* pData, NXINT32 iLen, NXVOID* pExtraInfo, NXUINT32 uDTS, NXUINT32  uPTS, NXINT32 iFlag,
									  NXUINT32* puDecodeResult, NXVOID *pUserData );
NXINT32 nexCALBody_Video_VTB_GetOutput( NXUINT8** ppBits1, NXUINT8** ppBits2, NXUINT8** ppBits3, NXUINT32 *puPTS, NXVOID *pUserData );
NXINT32 nexCALBody_Video_VTB_Reset( NXVOID *pUserData );
//NXINT32 nexCALBody_Video_VTB_GetInfo(NXUINT32 uIndex, NXUINT32* puResult, NXVOID *pUserData);

NXINT32 nexCALBody_Video_VTB_GetProperty( NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData );
NXINT32 nexCALBody_Video_VTB_SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData );

#ifdef __cplusplus
}
#endif

#endif	// _CALBODY_H264_HEADER_
