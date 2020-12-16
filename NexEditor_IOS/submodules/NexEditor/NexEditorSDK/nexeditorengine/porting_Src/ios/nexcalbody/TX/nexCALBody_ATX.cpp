/******************************************************************************
 * File Name   :	nexCALBody_ATX.cpp
 * Description :	The implementation of Audio Decoder/Encoder CAL Body
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

#include "NexCALBody_ATX.h"
#include "NexCalBodyTool.h"
#include "NexAudioCodecUsingAudioToolbox.h"

// For Decoder and Encoder
#define CHECK_SET_EOS_INDEX 0
#define COMMON_INT_INDEX_END 0
#define COMMON_PNT_INDEX_END 0

// For Decoder only (integer)
#define SAMPLINGRATE_INDEX (COMMON_INT_INDEX_END+1)
#define CHANNELS_INDEX (COMMON_INT_INDEX_END+2)
#define BITPERSAMPLE_INDEX (COMMON_INT_INDEX_END+3)
#define DECODER_OUTPUT_SIZE_INDEX (COMMON_INT_INDEX_END+4)
#define DECODER_OBJECT_TYPE (COMMON_INT_INDEX_END+5)

// For Encoder only (integer)
#define STATIC_OUTPUTBUFFER_SIZE_INDEX (COMMON_INT_INDEX_END+1)
// For Encoder only (pointer)
#define STATIC_OUTPUTBUFFER_INDEX (COMMON_PNT_INDEX_END+1)

#define MAX_INT_USER_DATA 8
#define MAX_PNT_USER_DATA 4

#if !defined(_TEST_WITHOUT_CAL_)

NXINT32 nexCALBody_AudioDecoder_ATX_GetProperty(NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData)
{
    switch (uProperty)
    {
        case NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION :
            *pqValue = NEXCAL_PROPERTY_ANSWERIS_YES;
            break;
        case NEXCAL_PROPERTY_CODEC_IMPLEMENT:
            *pqValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW;
            break;
        case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
            *pqValue = NEXCAL_PROPERTY_ANSWERIS_YES;
            break;
        case NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME :
            *pqValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO;
            break;
        case NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER :
            *pqValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_N0;
            break;
        case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
            *pqValue = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO;
            break;
        case NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE :
            *pqValue = NEXCAL_PROPERTY_AUDIO_BUFFER_PCM;
            break;
        case NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL:
            *pqValue = NEXCAL_PROPERTY_HAVE_SAMPLE_PER_CHANNEL_NO;
            break;
        default:
            *pqValue = 0;
            break;
    }
    return 0;
}

NXINT32 nexCALBody_AudioDecoder_ATX_SetProperty(NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData)
{
    switch(uProperty)
    {
        case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
            break;
            
        default:
            break;
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_CRAL,0,"ATX: SetProperty was called. property(%d)", uProperty);
    return 0;
}


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
                                         ,NXVOID **ppUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. oti : %s", NexCalBodyTools::getNameFromOTI(eCodecType) );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. sampling rate : %d", *piSamplingRate );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. channels : %d", *piNumOfChannels );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. Bits/Sample : %d", *puBitsPerSample );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. Samples/Packet : %d", *piNumOfSamplesPerChannel );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. uUserDataType : %d", uUserDataType );
    
    NexAudioCodecUsingAudioToolboxConverter::OBJECT_TYPE objectType;
    if ( eCodecType == eNEX_CODEC_A_AAC || eCodecType == eNEX_CODEC_A_AACPLUS || eCodecType == eNEX_CODEC_A_AACPLUSV2 )
        objectType = NexAudioCodecUsingAudioToolboxConverter::OBJECT_TYPE::OBJECT_AAC;
    else if ( eCodecType == eNEX_CODEC_A_MP3 )
        objectType = NexAudioCodecUsingAudioToolboxConverter::OBJECT_TYPE::OBJECT_MP3;
    else
        return NEXCAL_ERROR_NOT_SUPPORT_AUDIO_CODEC;
    
  
    
    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = new NexAudioCodecUsingAudioToolboxConverter();
    int ret = pAudioCodec->init( NexAudioCodecUsingAudioToolboxConverter::CODE_MODE::CODE_MODE_DECODE
                                ,objectType
                                ,*piSamplingRate
                                ,*piNumOfChannels
                                ,*puBitsPerSample
                                ,*piNumOfSamplesPerChannel
                                ,*piSamplingRate     // for output pcm
                                ,2//(*piNumOfChannels
                                ,*puBitsPerSample
                                );
    
    if ( ret != NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE ) {
        nexSAL_TraceCat( NEX_TRACE_CATEGORY_ERR, 0, "ATX:DEC: Init error. ret of init = %d", ret );
        delete pAudioCodec;
        return NEXCAL_ERROR_NOT_SUPPORT_AUDIO_CODEC;
    }

    pAudioCodec->setMaxUserValue(MAX_INT_USER_DATA, MAX_PNT_USER_DATA);
    pAudioCodec->setUserValueInt(SAMPLINGRATE_INDEX, *piSamplingRate);
    pAudioCodec->setUserValueInt(CHANNELS_INDEX, *piNumOfChannels);
    pAudioCodec->setUserValueInt(BITPERSAMPLE_INDEX, *puBitsPerSample);
    pAudioCodec->setUserValueInt(DECODER_OUTPUT_SIZE_INDEX, (*piNumOfSamplesPerChannel) * /*(*piNumOfChannels)*/ 2 * (*puBitsPerSample) / 8);
    pAudioCodec->setUserValueInt(CHECK_SET_EOS_INDEX, 0);
    pAudioCodec->setUserValueInt(DECODER_OBJECT_TYPE, objectType);

    pAudioCodec->start();
    
    *ppUserData = (NXVOID*)pAudioCodec;
    *piNumOfChannels = (*piNumOfChannels != 2) ? 2 : *piNumOfChannels; // = 2;
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Init. Success" );

    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioDecoder_ATX_Deinit( NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Deinit." );
    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = (NexAudioCodecUsingAudioToolboxConverter *)pUserData;
    
    pAudioCodec->stop();
    pAudioCodec->deinit();
    delete pAudioCodec;

    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Deinit End." );

    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioDecoder_ATX_Dec( NXUINT8* pSource, NXINT32 iLen, NXVOID* pExtraInfo
                                        ,NXVOID* pDest, NXINT32* piWrittenPCMSize
                                        ,NXUINT32 uDTS, NXUINT32* puOutputTime
                                        ,NXINT32 nFlag
                                        ,NXUINT32* puDecodeResult
                                        ,NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 2, "ATX:DEC: Decode Enter. p=0x%x, len=%d, flag=0x%x", pSource, iLen, nFlag );
    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = (NexAudioCodecUsingAudioToolboxConverter *)pUserData;

    *puDecodeResult = 0 ;
    NEXCAL_INIT_ADEC_RET(*puDecodeResult);
    NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
    
    if ( nFlag == NEXCAL_ADEC_FLAG_END_OF_STREAM ){

        if ( pAudioCodec->getUserValueInt(CHECK_SET_EOS_INDEX) == 0 ) {
            pAudioCodec->setInputEOS();
            pAudioCodec->setUserValueInt(CHECK_SET_EOS_INDEX, 1);
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Codec setEOS." );
        }
    }
    else {
    
        if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE != pAudioCodec->enqueue( pSource, iLen ) ) {
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Dec. No input buffer" );
        }
        else {
            NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
        }
    }
    
    if ( pAudioCodec->checkOutputAvailable() ) {
        int nLen = pAudioCodec->getUserValueInt( DECODER_OUTPUT_SIZE_INDEX );
        if ( nLen > *piWrittenPCMSize ) {
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_ERR, 0, "ATX:DEC: Dec. Too small buffer which from caller.nLen=%d,nLen2=%d",nLen, *piWrittenPCMSize);
        }
        //int nLen = 4096*4;
        int dequeue_ret;
        if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE == (dequeue_ret = pAudioCodec->dequeue( pDest, piWrittenPCMSize ) ) ) {
            if ( nLen != *piWrittenPCMSize ) {
                nexSAL_TraceCat( NEX_TRACE_CATEGORY_ERR, 0, "ATX:DEC: Dec. What happend!!!." );
            }
            NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
        }
        else {//if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE_EOS == dequeue_ret ) {
            NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);
        }
    }
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 2, "ATX:DEC: Decode End. output_flag = 0x%x", *puDecodeResult );

    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioDecoder_ATX_Reset( NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:DEC: Reset was called." );
    return NEXCAL_ERROR_NONE;
}


//
// encoder part
//

NXINT32 nexCALBody_AudioEncoder_ATX_GetProperty(NXUINT32 uProperty, NXINT64 *pqValue, NXVOID *pUserData)
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: GetProperty was called. Did nothing." );
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioEncoder_ATX_SetProperty(NXUINT32 uProperty, NXINT64 qValue, NXVOID *pUserData)
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: SetProperty was called. Did nothing." );
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioEncoder_ATX_Init( NEX_CODEC_TYPE eCodecType
                                         ,NXUINT8 **ppConfig
                                         ,NXINT32 *pnConfigLen
                                         ,NXUINT32 uSamplingRate
                                         ,NXUINT32 uNumOfChannels
                                         ,NXINT32 nBitRate
                                         ,NXVOID **ppUserData)
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Init. oti : %s", NexCalBodyTools::getNameFromOTI(eCodecType) );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Init. sampling rate : %d", uSamplingRate );
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Init. channels : %d", uNumOfChannels );

    NexAudioCodecUsingAudioToolboxConverter::OBJECT_TYPE objectType;
    if ( eCodecType == eNEX_CODEC_A_AAC || eCodecType == eNEX_CODEC_A_AACPLUS || eCodecType == eNEX_CODEC_A_AACPLUSV2 )
        objectType = NexAudioCodecUsingAudioToolboxConverter::OBJECT_TYPE::OBJECT_AAC;
    else
        return NEXCAL_ERROR_NOT_SUPPORT_AUDIO_CODEC;

    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = new NexAudioCodecUsingAudioToolboxConverter();
    int ret = pAudioCodec->init( NexAudioCodecUsingAudioToolboxConverter::CODE_MODE::CODE_MODE_ENCODE
                                ,objectType
                                ,uSamplingRate
                                ,uNumOfChannels
                                ,16
                                ,NexAudioCodecUsingAudioToolboxConverter::AAC_SAMPLES_PER_CHANNEL
                                ,uSamplingRate     // for output pcm
                                ,uNumOfChannels // In appl encoder, for mono input, we cann't set output's channels to 2. we should set mono for output.
                                ,16
                                );

    if ( ret != NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE ) {
        nexSAL_TraceCat( NEX_TRACE_CATEGORY_ERR, 0, "ATX:ENC: Init error. ret of init = %d", ret );
        delete pAudioCodec;
        return NEXCAL_ERROR_NOT_SUPPORT_AUDIO_CODEC;
    }
    
    void* pOutputStaticBuffer = (void*)malloc( 1024*16 );

    pAudioCodec->setMaxUserValue(MAX_INT_USER_DATA, MAX_PNT_USER_DATA);
    pAudioCodec->setUserValueInt(CHECK_SET_EOS_INDEX, 0);
    pAudioCodec->setUserValuePointer(STATIC_OUTPUTBUFFER_INDEX, pOutputStaticBuffer);
    pAudioCodec->setUserValueInt(STATIC_OUTPUTBUFFER_SIZE_INDEX, 1024*16 );
    
    pAudioCodec->start();

    ((unsigned char*)pOutputStaticBuffer)[0] = 0x12;
    ((unsigned char*)pOutputStaticBuffer)[1] = 0x10;
    *ppConfig = &((unsigned char*)pOutputStaticBuffer)[0];
    *pnConfigLen = 2;
    
    *ppUserData = (NXVOID*)pAudioCodec;
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Init. Success" );

    return NEXCAL_ERROR_NONE;
}


NXINT32 nexCALBody_AudioEncoder_ATX_Deinit( NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Deinit." );
    
    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = static_cast<NexAudioCodecUsingAudioToolboxConverter*>(pUserData);
    if ( !pAudioCodec )
        return NEXCAL_ERROR_NONE;
    
    pAudioCodec->stop();
    pAudioCodec->deinit();
    
    void* pOutputStaticBuffer = pAudioCodec->getUserValuePointer(STATIC_OUTPUTBUFFER_INDEX);
    if ( pOutputStaticBuffer ) {
        free( pOutputStaticBuffer );
        pAudioCodec->setUserValuePointer(STATIC_OUTPUTBUFFER_INDEX, nullptr);
    }
    
    delete pAudioCodec;
    
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Deinit End." );
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioEncoder_ATX_Encode( NXUINT8 *pData
                                           ,NXINT32 iLen
                                           ,NXUINT8 **ppOutData
                                           ,NXUINT32 *puOutLen
                                           ,NXUINT32 *puEncodeResult
                                           ,NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 2, "ATX:ENC: Encode Enter. p=0x%x, len=%d", pData, iLen );
    
    NexAudioCodecUsingAudioToolboxConverter * pAudioCodec = (NexAudioCodecUsingAudioToolboxConverter *)pUserData;
    
    *puEncodeResult = 0 ;
    NEXCAL_INIT_AENC_RET(*puEncodeResult);
    NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_ENCODING_SUCCESS);
    
    if ( NULL == pData ) {
        if ( pAudioCodec->getUserValueInt(CHECK_SET_EOS_INDEX) == 0 ) {
            pAudioCodec->setInputEOS();
            pAudioCodec->setUserValueInt(CHECK_SET_EOS_INDEX, 1);
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Encode setEOS." );
        }
    }
    else {
        if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE != pAudioCodec->enqueue( pData, iLen ) ) {
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: No input buffer" );
        }
        else {
            NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_NEXT_INPUT);
        }
    }
    
    if ( pAudioCodec->checkOutputAvailable() ) {
        void* pOutputStaticBuffer = pAudioCodec->getUserValuePointer( STATIC_OUTPUTBUFFER_INDEX );
        int nStaticBufferIndex = pAudioCodec->getUserValueInt( STATIC_OUTPUTBUFFER_SIZE_INDEX );

        if ( !pOutputStaticBuffer || nStaticBufferIndex < 0 ) {
            nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Enc. this pointer is NULL ???." );
        }
        int nBufferFilledInByte = nStaticBufferIndex;
        int dequeue_ret;
        
        if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE == ( dequeue_ret = pAudioCodec->dequeue( (char*)pOutputStaticBuffer+7, &nBufferFilledInByte ) ) ) {
//            if ( 0 > nBufferFilledInByte || nStaticBufferIndex < nBufferFilledInByte ) {
            if ( 0 > nBufferFilledInByte || nStaticBufferIndex < nBufferFilledInByte+7 ) {
                nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Enc. What happend!!!." );
            }
            else {
                NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_OUTPUT_EXIST);
                *ppOutData = static_cast<NXUINT8*>(pOutputStaticBuffer);
//                *puOutLen = nBufferFilledInByte;
                *puOutLen = nBufferFilledInByte+7;
            }
        }
        else {//if ( NexAudioCodecUsingAudioToolboxConverter::ERROR_NONE_EOS == dequeue_ret ) {
            NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_EOS);
        }
    }

    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 2, "ATX:ENC: Encode End. output_flag = 0x%x", *puEncodeResult );
    
    return NEXCAL_ERROR_NONE;
}

NXINT32 nexCALBody_AudioEncoder_ATX_Reset( NXVOID *pUserData )
{
    nexSAL_TraceCat( NEX_TRACE_CATEGORY_CRAL, 0, "ATX:ENC: Reset was called." );
    return NEXCAL_ERROR_NONE;
}

#endif

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr  8, 2016				Draft.
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 19, 2016                Encoder Part was added.
 ----------------------------------------------------------------------------*/
