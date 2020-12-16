/******************************************************************************
 * File Name   :	nexCALBody_VTX.h
 * Description :	The definition of Video Decoder/Encoder CAL Body
                    Using AudioToolbox in iOS
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef nexCALBody_VTX_h
#define nexCALBody_VTX_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "nexCAL.h"
    
    //
    // encoder part
    //
    
    NXINT32 nexCALBody_VideoEncoder_VTX_GetProperty( NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData );
    NXINT32 nexCALBody_VideoEncoder_VTX_SetProperty( NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData );
    
    NXINT32 nexCALBody_VideoEncoder_VTX_Init( NEX_CODEC_TYPE a_eCodecType
                                             ,NXUINT8 **a_ppConfig
                                             ,NXINT32 *a_pnConfigLen
                                             ,NXINT32 a_nQuality
                                             ,NXINT32 a_nWidth
                                             ,NXINT32 a_nHeight
                                             ,NXINT32 a_nPitch                                             
                                             ,NXINT32 a_nFPS
                                             ,NXBOOL a_bCBR
                                             ,NXINT32 a_nBitrate
                                             ,NXVOID **a_ppUserData );
    NXINT32 nexCALBody_VideoEncoder_VTX_Deinit(NXVOID *a_pUserData );
    
    NXINT32 nexCALBody_VideoEncoder_VTX_Encode( NXUINT8 *a_pData1
                                               ,NXUINT8 *a_pData2
                                               ,NXUINT8 *a_pData3
                                               ,NXUINT32 a_uPTS
                                               ,NXUINT32 *a_puEncodeResult
                                               ,NXVOID *a_pUserData );
    
    NXINT32 nexCALBody_VideoEncoder_VTX_GetOutput( NXUINT8 **a_ppOutData
                                                  ,NXINT32 *a_pnOutLen
                                                  ,NXUINT32 *a_puPTS
                                                  ,NXVOID *a_pUserData );
    
    NXINT32 nexCALBody_VideoEncoder_VTX_Skip(NXUINT8 **a_ppOutData, NXUINT32 *a_puOutLen, NXVOID *a_pUserData);
    
#ifdef __cplusplus
}
#endif

#endif /* nexCALBody_VTX_h */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 19, 2016                Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
