/******************************************************************************
 * File Name   :	nexCALBody_VTX.cpp
 * Description :	The implementation of video Decoder/Encoder CAL Body
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

#include "stdlib.h"
#include "nexCALBody_VTX.h"
#include "NexCalBodyTool.h"
#include "NexVideoCodecUsingVTX.h"

// For Encoder (integer)
#define CHECK_SET_EOS_INDEX 0
#define STATIC_OUTPUTBUFFER_SIZE_INDEX 1
// For Encoder (pointer)
#define STATIC_OUTPUTBUFFER_INDEX 0

#define MAX_INT_USER_DATA 4
#define MAX_PNT_USER_DATA 4

NXINT32 nexCALBody_VideoEncoder_VTX_GetProperty( NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: GetProperty was called. property=0x%x.", uProperty );
    
    switch ( uProperty ) {
        case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
            *pqValue = static_cast<NXINT64>(420);
            break;
        case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
            *pqValue = static_cast<NXINT64>(420);
            break;
        case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
            *pqValue = static_cast<NXINT64>(420);
            break;
        case NEXCAL_PROPERTY_VIDEO_INPUT_BUFFER_TYPE:
            *pqValue = static_cast<NXINT64>(21);
            break;
        default:
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: GetProperty was called. Did nothing." );
            
    }
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_VideoEncoder_VTX_SetProperty( NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: SetProperty was called. property=0x%x. Did nothing.", uProperty );
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_VideoEncoder_VTX_Init( NEX_CODEC_TYPE eCodecType
                                         ,NXUINT8 **ppConfig
                                         ,NXINT32 *pnConfigLen
                                         ,NXINT32 nQuality
                                         ,NXINT32 nWidth
                                         ,NXINT32 nHeight
                                         ,NXINT32 nPitch
                                         ,NXINT32 nFPS
                                         ,NXBOOL bCBR
                                         ,NXINT32 nBitrate
                                         ,NXVOID **ppUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Init was called." );
    
    VideoEncoderUsingVTX *pEncoder = new VideoEncoderUsingVTX();
    pEncoder->setProperty(VideoEncoderUsingVTX::PROPERTY_ID_DEBUGLEVEL,reinterpret_cast<void*>(7));
    if ( VideoEncoderUsingVTX::ERROR_NONE != pEncoder->init( nWidth, nHeight, nBitrate, nFPS ) ) {
        nexSAL_TraceCat( NEX_TRACE_CATEGORY_ERR, 0, "VTX:ENC: Init failed." );
        delete pEncoder;
        return NEXCAL_ERROR_FAIL;
    }
    
    void* pOutputStaticBuffer = (void*)malloc( 1024*1024 );
    
    pEncoder->setMaxUserValue(MAX_INT_USER_DATA, MAX_PNT_USER_DATA);
    pEncoder->setUserValueInt(CHECK_SET_EOS_INDEX, 0);
    pEncoder->setUserValuePointer(STATIC_OUTPUTBUFFER_INDEX, pOutputStaticBuffer);
    pEncoder->setUserValueInt(STATIC_OUTPUTBUFFER_SIZE_INDEX, 1024*1024 );


    *ppUserData = static_cast<NXVOID*>(pEncoder);
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Init end." );
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_VideoEncoder_VTX_Deinit(NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Deinit was called." );
    
    VideoEncoderUsingVTX *pEncoder = static_cast<VideoEncoderUsingVTX *>(pUserData);
    if ( !pEncoder )
        return NEXCAL_ERROR_NONE;
    
    pEncoder->deinit();
    
    void* pOutputStaticBuffer = pEncoder->getUserValuePointer(STATIC_OUTPUTBUFFER_INDEX);
    if ( pOutputStaticBuffer ) {
        free( pOutputStaticBuffer );
        pEncoder->setUserValuePointer(STATIC_OUTPUTBUFFER_INDEX, nullptr);
    }
    
    delete pEncoder;
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Deinit end." );

    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_VideoEncoder_VTX_Encode( NXUINT8 *pData1
                                           ,NXUINT8 *pData2
                                           ,NXUINT8 *pData3
                                           ,NXUINT32 uPTS
                                           ,NXUINT32 *puEncodeResult
                                           ,NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Encode was called." );

    VideoEncoderUsingVTX *pEncoder = static_cast<VideoEncoderUsingVTX *>(pUserData);

    *puEncodeResult = 0 ;
    NEXCAL_INIT_VENC_RET(*puEncodeResult);
    NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS);

    if ( pData1 == nullptr ) {
        if ( pEncoder->getUserValueInt(CHECK_SET_EOS_INDEX) == 0 ) {
            pEncoder->setInputEOS();
            pEncoder->setUserValueInt(CHECK_SET_EOS_INDEX, 1);
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Encode setEOS." );
        }
    }
    else {
        if ( VideoEncoderUsingVTX::ERROR_NONE != pEncoder->enqueue( pData1, 0, ((long long) uPTS) * 1000, 0 ) ) {
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: No input buffer" );
        }
        else {
            NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_NEXT_INPUT);
        }
    }
    
    if ( pEncoder->checkOutputAvailable() ) {
        NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_EXIST);
    }
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 2, "VTX:ENC: Encode End. output_flag = 0x%x", *puEncodeResult );

    return NEXCAL_ERROR_NONE;
}

extern "C" {
NXUINT8* NexCodecUtil_AVC_NAL_GetConfigStream
( NXUINT8 *frame
 , NXINT32 frameLength
 , NXINT32 sizeOfNAL
 , NXINT32 *resultLength );
};

void* FindPictureNAL(unsigned char* pFrame, int len)
{
    int resultLen = 0;
    unsigned char * pRet = NexCodecUtil_AVC_NAL_GetConfigStream( pFrame, len, 4, &resultLen );
    return pRet+resultLen;
}

NXINT32 nexCALBody_VideoEncoder_VTX_GetOutput( NXUINT8 **ppOutData
                                              ,NXINT32 *pnOutLen
                                              ,NXUINT32 *puPTS
                                              ,NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: GetOutput was called." );
    
    VideoEncoderUsingVTX *pEncoder = static_cast<VideoEncoderUsingVTX *>(pUserData);
    
    if ( pEncoder->checkOutputAvailable() ) {
        void* pOutputStaticBuffer = pEncoder->getUserValuePointer( STATIC_OUTPUTBUFFER_INDEX );
        int nStaticBufferSize = pEncoder->getUserValueInt( STATIC_OUTPUTBUFFER_SIZE_INDEX );
        
        int nBufferFilledInByte = nStaticBufferSize;
        int dequeue_ret;
        long long dtsInUs;
        bool isConfig;

#if 0
        if ( VideoEncoderUsingVTX::ERROR_NONE == ( dequeue_ret = pEncoder->dequeue( (char*)pOutputStaticBuffer+4, &nBufferFilledInByte, &dtsInUs, NULL ) ) ) {
            if ( 0 > nBufferFilledInByte || nStaticBufferSize < nBufferFilledInByte + 4 ) {
                nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Enc. What happend!!!." );
                return NEXCAL_ERROR_FAIL;
            }
            else {
                memcpy( pOutputStaticBuffer, &nBufferFilledInByte, sizeof(int) );
                *ppOutData = static_cast<NXUINT8*>(pOutputStaticBuffer);
                *pnOutLen = nBufferFilledInByte+4;
                *puPTS = static_cast<NXUINT32>(dtsInUs / 1000l);
            }
        }
        else {
            return NEXCAL_ERROR_FAIL;
        }
#else
        if ( VideoEncoderUsingVTX::ERROR_NONE == ( dequeue_ret = pEncoder->dequeue( pOutputStaticBuffer, &nBufferFilledInByte, &dtsInUs, &isConfig, NULL ) ) ) {
            if ( 0 > nBufferFilledInByte || nStaticBufferSize < nBufferFilledInByte ) {
                nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Enc. What happend!!!." );
                return NEXCAL_ERROR_FAIL;
            }
            else {
                
                if ( isConfig ) {
                    *ppOutData = static_cast<NXUINT8*>((unsigned char*)pOutputStaticBuffer);
                    *pnOutLen = nBufferFilledInByte;
                }
                else {
                    int resultLen = 0;
                    unsigned char * pRet = NexCodecUtil_AVC_NAL_GetConfigStream( (unsigned char*)pOutputStaticBuffer,nBufferFilledInByte, 4, &resultLen );
                    if ( pRet && resultLen > 0 ) {
                        *ppOutData = pRet + resultLen;
                        *pnOutLen = nBufferFilledInByte - ( (long long)pRet - (long long)pOutputStaticBuffer + resultLen );
                    }
                    else {
                        *ppOutData = static_cast<NXUINT8*>((unsigned char*)pOutputStaticBuffer);
                        *pnOutLen = nBufferFilledInByte;
                    }
                }
                *puPTS = static_cast<NXUINT32>(dtsInUs / 1000l);
            }
        }
        else {
            return NEXCAL_ERROR_FAIL;
        }
#endif
        
    }
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_VideoEncoder_VTX_Skip(NXUINT8 **ppOutData, NXUINT32 *puOutLen, NXVOID *pUserData)
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "VTX:ENC: Skip was called." );
    return NEXCAL_ERROR_NONE;
}

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 19, 2016                Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
