/******************************************************************************
 * File Name   :	nexCalBody_VideoToolBox.m
 * Description :	The implementation of AVC Decoder CAL Body
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "NexSAL_Com.h"
#include "NexSAL_Internal.h"
#include "NexCAL.h"
#include "VideoToolboxManager.h"

#import	<mach/mach_time.h>
#import	<UIKit/UIKit.h>

typedef struct {
    double vtb_startAbsoluteTime;
    double ticks_to_ns_factor;
} VideoDecoderOutputTimer;

static VideoDecoderOutputTimer timer;

BOOL VideoDecoderOutputTimerIsTimeOut(VideoDecoderOutputTimer *timer) {
    BOOL result = FALSE;
    
    if ( timer->vtb_startAbsoluteTime == 0 ) {
        timer->vtb_startAbsoluteTime = mach_absolute_time();
    }
    if( timer->ticks_to_ns_factor == 0 ) {
        mach_timebase_info_data_t tbi = {0};
        mach_timebase_info(&tbi);
        timer->ticks_to_ns_factor = (double)tbi.numer / tbi.denom;
    }
    
    int  vtb_elapsedTimeMS = (unsigned int)(((double)(mach_absolute_time() - timer->vtb_startAbsoluteTime)*timer->ticks_to_ns_factor)/1000000.0);
    
    if ( vtb_elapsedTimeMS > 300 ) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0,"[%s %d] VTB TimeOut, elapsedTime:%d\n", __func__, __LINE__, vtb_elapsedTimeMS);
        result = TRUE;
    }
    return result;
}

void VideoDecoderOutputTimerReset(VideoDecoderOutputTimer *timer) {
    timer->vtb_startAbsoluteTime = 0;
    timer->ticks_to_ns_factor = 0;
}

NXINT32 nexCALBody_Video_VTB_Init( NEX_CODEC_TYPE eCodecType, NXUINT8* pConfig, NXINT32 iLen,
                                  NXUINT8* pConfigEnhance, NXINT32 iEnhLen, NXVOID* pInitInfo, NXVOID* pExtraInfo, NXINT32 iNALHeaderLengthSize, NXINT32* piWidth,
                                  NXINT32* piHeight, NXINT32* piPitch, NXUINT32 uMode, NXUINT32 uUserDataType,  NXVOID **ppUserData )
{
    NXEVideoToolboxManager* vtm = NULL;
    
    if (!(eCodecType == eNEX_CODEC_V_H264 || eCodecType == eNEX_CODEC_V_HEVC)) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[%s %d] Unsupported codec type 0x%08x", __func__, __LINE__, eCodecType);
        return NEXCAL_ERROR_FAIL;
    }
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[%s %d] Init START!\n", __func__, __LINE__);
    VTDCodecType codecType = VTDCodecTypeH264;
    if (eCodecType == eNEX_CODEC_V_HEVC) {
        codecType = VTDCodecTypeHEVC;
    }
    
    
    vtm = [[NXEVideoToolboxManager alloc]init];
    if ( vtm == nil )
        return NEXCAL_ERROR_FAIL;
    
    [vtm createDecoderWithOTI:codecType dsi:pConfig dsiLen:iLen];
    
    *ppUserData = (void*)vtm;
    
    VideoDecoderOutputTimerReset(&timer);
    
    return NEXCAL_ERROR_NONE;
}


NXINT32 nexCALBody_Video_VTB_Close( NXVOID *pUserData )
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[%s %d]\n", __func__, __LINE__);
    NXEVideoToolboxManager* vtm = (NXEVideoToolboxManager*)pUserData;
    if ( vtm == nil )
        return NEXCAL_ERROR_FAIL;
    
    [vtm deleteDecoder];
    [vtm release];
    vtm = nil;
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_Video_VTB_Dec( NXUINT8* pData, NXINT32 iLen, NXVOID* pExtraInfo, NXUINT32 uDTS, NXUINT32  uPTS, NXINT32 iFlag,
                                 NXUINT32* puDecodeResult, NXVOID *pUserData )
{
    NXEVideoToolboxManager* vtm = (NXEVideoToolboxManager*)pUserData;
    *puDecodeResult = 0;
    
    if ( iFlag & NEXCAL_VDEC_FLAG_OUTPUT_ONLY || iFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM ) {
        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
        
        if ( [vtm getUserIntValueWithIndex:0] == 1 ) {
            NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);
        } else {
            if ( [vtm checkOutputAvailable] == true ) {
                NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
                if ( iFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM ) {
                    if ( [vtm isOutputPtsLastPts] ) {
                        [vtm setUserIntValueWithIndex:0 value:1];
                        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);
                    }
                }
                VideoDecoderOutputTimerReset(&timer);
            } else {
                [vtm completeAllAsynchronousFrames];
                
                if ( VideoDecoderOutputTimerIsTimeOut(&timer) ) {
                    NEXCAL_DEL_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
                    if ( iFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM ) {
                        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);
                    }
                }
            }
        }
        return 0;
    }
    else {
        
        [vtm decodeWithFrame:pData length:iLen dts:uDTS pts:uPTS];
        
        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
        
        if ([vtm checkOutputAvailable])
            NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);
        
    }
    return 0;
    
}

NXINT32 nexCALBody_Video_VTB_GetOutput( NXUINT8** ppBits1, NXUINT8** ppBits2, NXUINT8** ppBits3, NXUINT32 *puPTS, NXVOID *pUserData )
{
    NXEVideoToolboxManager* vtm = (NXEVideoToolboxManager*)pUserData;
    
    if ( [vtm checkOutputAvailable] == true )
    {
        NXEDecoderOutputImageBuffer* mOutputImageBuffer = [vtm getFirstOutputBuffer];
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "[%s,%d] getOutput(%p)'s retainCount=%lu", __func__, __LINE__, mOutputImageBuffer, (unsigned long)mOutputImageBuffer.retainCount);
        
        if(mOutputImageBuffer == NULL)
        {
            return -1;
        }
        
        *ppBits1 = (NXUINT8*)mOutputImageBuffer;//.image;
        *puPTS = (NXUINT32) mOutputImageBuffer.pts;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "[%s,%d] getOutput(%p)'s retainCount=%lu", __func__, __LINE__, mOutputImageBuffer, (unsigned long)mOutputImageBuffer.retainCount);
    }
    
    return 0;
}

NXINT32 nexCALBody_Video_VTB_Reset( NXVOID *pUserData )
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_P_VIDEO, 0,"[%s,%d] reset \n", __func__, __LINE__ );
    
    VideoDecoderOutputTimerReset(&timer);
    
    NXEVideoToolboxManager* vtm = (NXEVideoToolboxManager*)pUserData;
    [vtm reset];
    return 0;
}

NXINT32 nexCALBody_Video_VTB_GetInfo(NXUINT32 uIndex, NXUINT32* puResult, NXVOID *pUserData)
{
    return 0;
}

NXINT32 nexCALBody_Video_VTB_GetProperty( NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData )
{
    switch ( a_uProperty )
    {
		case NEXCAL_PROPERTY_INITPS_FORMAT :
			//*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
			*a_pqValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
			break;
		case NEXCAL_PROPERTY_BYTESTREAM_FORMAT:
			*a_pqValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
			break;
		case NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION:
            *a_pqValue = NEXCAL_PROPERTY_ANSWERIS_NO;
            break;
        case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE:
            *a_pqValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
            break;
        case NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT:
            *a_pqValue = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES;
            break;
        default:
            //result = nexCALBody_Video_GetProperty(a_uProperty, a_pqValue, a_pUserData);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "[%s, %d] getProperty: not implemented property(%ul)",__func__, __LINE__, a_uProperty);
            break;
    }
    return 0;
}

NXINT32 nexCALBody_Video_VTB_SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "[%s, %d] setProperty: not implemented property(%ul)",__func__, __LINE__, a_uProperty);
    return 0;
}
