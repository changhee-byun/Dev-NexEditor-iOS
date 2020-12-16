/******************************************************************************
 * File Name   :	nexCALBody_ATX.h
 * Description :	The definition of Audio Decoder/Encoder CAL Body
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

#ifndef nexCALBody_ATX_h
#define nexCALBody_ATX_h

//#define _TEST_WITHOUT_CAL_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_TEST_WITHOUT_CAL_)

#include "nexCAL.h"

    //
    // decoder part
    //
    
    NXINT32 nexCALBody_AudioDecoder_ATX_GetProperty(NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData);
    NXINT32 nexCALBody_AudioDecoder_ATX_SetProperty(NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData);
    
    NXINT32 nexCALBody_AudioDecoder_ATX_Init( NEX_CODEC_TYPE eCodecType
                                             ,NXUINT8* pConfig, NXINT32 iConfigLen
                                             ,NXUINT8* pFrame, NXINT32 iFrameLen
                                             ,NXVOID* pInitInfo
                                             ,NXVOID* pExtraInfo
                                             ,NXUINT32* piSamplingRate
                                             ,NXUINT32* piNumOfChannels
                                             ,NXUINT32* puBitsPerSample
                                             ,NXUINT32* piNumOfSamplesPerChannel
                                             ,NXUINT32 uMode
                                             ,NXUINT32 uUserDataType
                                             ,NXVOID **ppUserData );
    
    NXINT32 nexCALBody_AudioDecoder_ATX_Deinit( NXVOID *pUserData );
    
    NXINT32 nexCALBody_AudioDecoder_ATX_Dec( NXUINT8* pSource, NXINT32 iLen, NXVOID* pExtraInfo
                                            ,NXVOID* pDest, NXINT32* piWrittenPCMSize
                                            ,NXUINT32 uDTS
                                            ,NXUINT32* puOutputTime, NXINT32 nFlag,  NXUINT32* puDecodeResult
                                            ,NXVOID *pUserData );
    
    NXINT32 nexCALBody_AudioDecoder_ATX_Reset( NXVOID *pUserData );

    //
    // encoder part
    //
    
    NXINT32 nexCALBody_AudioEncoder_ATX_GetProperty(NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData);
    NXINT32 nexCALBody_AudioEncoder_ATX_SetProperty(NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData);

    NXINT32 nexCALBody_AudioEncoder_ATX_Init( NEX_CODEC_TYPE eCodecType
                                             ,NXUINT8 **ppConfig
                                             ,NXINT32 *pnConfigLen
                                             ,NXUINT32 uSamplingRate
                                             ,NXUINT32 uNumOfChannels
                                             ,NXINT32 nBitRate
                                             ,NXVOID **ppUserData );

    NXINT32 nexCALBody_AudioEncoder_ATX_Deinit( NXVOID *pUserData );
    
    NXINT32 nexCALBody_AudioEncoder_ATX_Encode( NXUINT8 *pData
                                               ,NXINT32 nLen
                                               ,NXUINT8 **ppOutData
                                               ,NXUINT32 *puOutLen
                                               ,NXUINT32 *puEncodeResult
                                               ,NXVOID *pUserData );
    
    NXINT32 nexCALBody_AudioEncoder_ATX_Reset( NXVOID *pUserData );

    
#endif // if !defined(_TEST_WITHOUT_CAL_)
    
#ifdef __cplusplus
}
#endif
#endif /* nexCALBody_ATX_h */


/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr  8, 2016                Draft.
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 19, 2016                Encoder Part was added.
 ----------------------------------------------------------------------------*/

