/******************************************************************************
 * File Name   :	NexAudioCodecUsingAudioToolbox.cpp
 * Description :	The definition of Audio De/EnCoder CAL Body using VTX.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#include "NexAudioCodecUsingAudioToolbox.h"
#include "NexSyncQueue.h"
#include "NexSAL_Internal.h"

static int nInstanceCount = 0;

NexAudioCodecUsingAudioToolboxConverter::NexAudioCodecUsingAudioToolboxConverter()
: mCodeMode(CODE_MODE_NONE)
, mObjectType(OBJECT_NONE)
, mSrcSamplingRateInSec(0)
, mSrcChannels(0)
, mSrcBitsPerChannel(0)
, mSrcFramesPerPacket(0)
, mDstSamplingRateInSec(0)
, mDstChannels(0)
, mDstBitsPerChannel(0)
, mOutputBufferCount(0)
, mOutputBuffers(NULL)
, mInputBufferCount(0)
, mInputBuffers(NULL)
, mThreadExist(false)
, mStopRequest(false)
, mInputEOSFlag(false)
, mOutputEOSFlag(false)
, mLastInputBufferFormat2AudioConverter(NULL)
, mInputSyncQueue(NULL)
, mOutputSyncQueue(NULL)
, mOutputDataPacketSize(0)
{
    myNumber = nInstanceCount;
    snprintf(myName,128,"-%d",myNumber);
    nInstanceCount++;
}

NexAudioCodecUsingAudioToolboxConverter::~NexAudioCodecUsingAudioToolboxConverter()
{
}

void NexAudioCodecUsingAudioToolboxConverter::_internalInit()
{
    mInputBuffers = (BufferFormamt**)malloc( sizeof(BufferFormamt*) * mInputBufferCount );
    for ( int i = 0 ; i < mInputBufferCount ; i++ ) {
        mInputBuffers[i] = (BufferFormamt*)malloc(sizeof(BufferFormamt));
        mInputBuffers[i]->pBuffer = (void*)malloc( _BUFFER_SIZE );
        mInputBuffers[i]->nMaxLen = _BUFFER_SIZE;
        mInputBuffers[i]->nLen = 0;
    }
    
    mOutputBuffers = (BufferFormamt**)malloc( sizeof(BufferFormamt*) * mOutputBufferCount );
    for ( int i = 0 ; i < mOutputBufferCount ; i++ ) {
        mOutputBuffers[i] = (BufferFormamt*)malloc(sizeof(BufferFormamt));
        mOutputBuffers[i]->pBuffer = (void*)malloc( _BUFFER_SIZE );
        mOutputBuffers[i]->nMaxLen = _BUFFER_SIZE;
        mOutputBuffers[i]->nLen = 0;
    }
}


void NexAudioCodecUsingAudioToolboxConverter::_internalDeinit()
{
    if ( mInputBuffers ) {
        for ( int i = 0 ; i < mInputBufferCount ; i++ ) {
            if ( mInputBuffers[i] ) {
                if ( mInputBuffers[i]->pBuffer ) {
                    free( mInputBuffers[i]->pBuffer );
                    //mInputBuffers[i]->pBuffer = NULL;
                }
                free(mInputBuffers[i]);
            }
        }
        
        free( mInputBuffers );
        mInputBuffers = NULL;
    }
    
    
    if ( mOutputBuffers ) {
        for ( int i = 0 ; i < mOutputBufferCount ; i++ ) {
            if ( mOutputBuffers ) {
                if ( mOutputBuffers[i]->pBuffer ) {
                    free( mOutputBuffers[i]->pBuffer );
                    //mOutputBuffers[i].pBuffer = NULL;
                }
                free(mOutputBuffers[i]);
            }
        }
        
        free( mOutputBuffers );
        mOutputBuffers = NULL;
    }
}

void NexAudioCodecUsingAudioToolboxConverter::fillTargetParams( bool bSrc, int* pSamplingRate, int* pBitsPerChannels, int* pCannels)
{
    if ( bSrc ) {
        *pSamplingRate = mSrcSamplingRateInSec;
        *pBitsPerChannels = mSrcBitsPerChannel;
        *pCannels = mSrcChannels;
    }
    else {
        *pSamplingRate = mDstSamplingRateInSec;
        *pBitsPerChannels = mDstBitsPerChannel;
        *pCannels = mDstChannels;
    }
}

int NexAudioCodecUsingAudioToolboxConverter::fillDescription4PCM( AudioStreamBasicDescription* target2fill, bool bSrc )
{
    int sr, bpc, channels;
    fillTargetParams( bSrc, &sr, &bpc, &channels );
    int bytesPerSample = bpc/8*channels;
    
    target2fill->mSampleRate = (Float64)sr;
    target2fill->mFormatID = kAudioFormatLinearPCM;
    target2fill->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    target2fill->mBytesPerPacket = bytesPerSample;
    target2fill->mFramesPerPacket = 1;
    target2fill->mBytesPerFrame = bytesPerSample;
    target2fill->mChannelsPerFrame = channels;
    target2fill->mBitsPerChannel = bpc;
    target2fill->mReserved = 0;
    
    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::fillDescription4AAC( AudioStreamBasicDescription* target2fill, bool bSrc )
{
    int sr, bpc, channels;
    fillTargetParams( bSrc, &sr, &bpc, &channels );

    target2fill->mSampleRate = sr;
    target2fill->mFormatID = kAudioFormatMPEG4AAC;
    target2fill->mFormatFlags = kMPEG4Object_AAC_LC;//kMPEG4Object_AAC_SBR;
    target2fill->mBytesPerPacket = 0;
    target2fill->mFramesPerPacket = 1024;
    target2fill->mBytesPerFrame = 0;
    target2fill->mChannelsPerFrame = channels;
    target2fill->mBitsPerChannel = 0;
    target2fill->mReserved = 0;
    
    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::fillDescription4MP3( AudioStreamBasicDescription* target2fill, bool bSrc )
{
    int sr, bpc, channels;
    fillTargetParams( bSrc, &sr, &bpc, &channels );
    
    target2fill->mSampleRate = sr;
    target2fill->mFormatID = kAudioFormatMPEGLayer3; // has no flag
    target2fill->mFormatFlags = 0;
    target2fill->mBytesPerPacket = 0;
    target2fill->mFramesPerPacket = mSrcFramesPerPacket;
    target2fill->mBytesPerFrame = 0;
    target2fill->mChannelsPerFrame = channels;
    target2fill->mBitsPerChannel = 0;
    target2fill->mReserved = 0;

    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::fillInputDescription( AudioStreamBasicDescription* target2fill )
{
    int ret_value = ERROR_NOT_SUPPORTED;
    
    if ( mCodeMode == CODE_MODE_ENCODE )
        ret_value = fillDescription4PCM( target2fill, true );
    else if ( mCodeMode == CODE_MODE_DECODE ) {
        if ( OBJECT_AAC == mObjectType ) {
            ret_value = fillDescription4AAC( target2fill, true );
        }
        else if ( OBJECT_MP3 == mObjectType ) {
            ret_value = fillDescription4MP3( target2fill, true );
        }
    }
    
    mASPD.mStartOffset = 0;
    mASPD.mVariableFramesInPacket = 0;
    mASPD.mDataByteSize = 4;//;

    return ret_value;
}

int NexAudioCodecUsingAudioToolboxConverter::fillOutputDescription( AudioStreamBasicDescription* target2fill )
{
    int ret_value = ERROR_NOT_SUPPORTED;

    if ( mCodeMode == CODE_MODE_ENCODE ) {
        ret_value = fillDescription4AAC( target2fill, false );
        mOutputDataPacketSize = 1;
    }
    else if ( mCodeMode == CODE_MODE_DECODE ) {
        ret_value = fillDescription4PCM( target2fill, false );
        if ( mObjectType == OBJECT_AAC )
            mOutputDataPacketSize = mSrcFramesPerPacket;
        else if ( mObjectType == OBJECT_MP3 )
            mOutputDataPacketSize = mSrcFramesPerPacket;
    }

    
    return ERROR_NOT_SUPPORTED;
}

int NexAudioCodecUsingAudioToolboxConverter::fillClassDescription( AudioClassDescription* target2fill, bool bSWCodec )
{
    int i, count, ith = 0;
    UInt32 size;
    AudioFormatID formatId = 0;
    AudioFormatPropertyID propertyId = 0;
    unsigned int manufacturer;
    
    if ( CODE_MODE_ENCODE == mCodeMode ) {
        propertyId = kAudioFormatProperty_Encoders;
        formatId = kAudioFormatMPEG4AAC;
        manufacturer = bSWCodec ? kAppleSoftwareAudioCodecManufacturer : kAppleHardwareAudioCodecManufacturer;
    }
    else if ( CODE_MODE_DECODE == mCodeMode ) {
        propertyId = kAudioFormatProperty_Decoders;
        if ( OBJECT_AAC == mObjectType ) {
            formatId = kAudioFormatMPEG4AAC; //kAudioFormatMPEG4AAC_ELD_SBR
        }
        if ( OBJECT_MP3 == mObjectType ) {
            formatId = kAudioFormatMPEGLayer3;
        }
    }
    
    AudioFormatGetPropertyInfo( propertyId, sizeof(formatId), &formatId, &size );
    count = size / sizeof(AudioClassDescription);
    
    AudioClassDescription descriptions[count];
    AudioFormatGetProperty( propertyId, sizeof(formatId), &formatId, &size, &descriptions[0] );
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodec: class descriptions [%d]. mode=%d,obj=%d", count, mCodeMode, mObjectType);
    
    ith = count;
    for ( i = 0 ; i < count ; i++ ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodec: %d th class description. subtype=%x, manufacturer=%c%c%c%c", i,
               descriptions[i].mSubType,
               (descriptions[i].mManufacturer & 0xff000000)>>24,
               (descriptions[i].mManufacturer & 0x00ff0000)>>16,
               (descriptions[i].mManufacturer & 0x0000ff00)>>8,
               (descriptions[i].mManufacturer & 0x000000ff));
        
        if ( formatId == descriptions[i].mSubType ) { //&& (manufacturer == descriptions[i].mManufacturer) ) {
            memcpy( target2fill, &descriptions[i], sizeof(AudioClassDescription) );
            ith = i;
            break; // iPhone 5c, there are 2 descriptions. but the second one is not available. 
        }
    }
    
    
    if ( ith == count )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioDecoder: Filling ClassDescription fails.");
        return ERROR_NOT_SUPPORTED;
    }
    
    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::init
        ( CODE_MODE mode
         ,OBJECT_TYPE objectType
         ,int srcSamplingRateInSec
         ,int srcChannels
         ,int srcBitsPerChannel
         ,int srcFramesPerPacket
         ,int dstSamplingRateInSec
         ,int dstChannels
         ,int dstBitsPerChannel
         ,int inputBufferCount /*= INPUT_BUFFER_DEFAULT_COUNT*/
         ,int outputBufferCount /*= OUTPUT_BUFFER_DEFAULT_COUNT*/ )
{
    DESC_BLOCK("CHECK STATE.")
    {
        if ( mThreadExist ){
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "NexAudioCodecATX: init fails. invalid state. threadexist=%d,",mThreadExist);
            return ERROR_INVALID_STATE;
        }
    }
    
    DESC_BLOCK("CHECK PARAMETERS.")
    {
        if ( ( srcSamplingRateInSec % 8000 ) != 0 && ( srcSamplingRateInSec % 11025 ) ) {
            return ERROR_INVALID_PARAMS;
        }
        if ( ( dstSamplingRateInSec % 8000 ) != 0 && ( dstSamplingRateInSec % 11025 ) ) {
            return ERROR_INVALID_PARAMS;
        }
        if ( srcChannels <= 0 || srcChannels > 8 || dstChannels <= 0 || dstChannels > 8 ) {
            return ERROR_INVALID_PARAMS;
        }
        if ( srcBitsPerChannel % 8 != 0 || dstBitsPerChannel % 8 != 0 ) {
            return ERROR_INVALID_PARAMS;
        }
        
        mSrcSamplingRateInSec = srcSamplingRateInSec;
        mSrcChannels = srcChannels;
        mSrcBitsPerChannel = srcBitsPerChannel;
        mSrcFramesPerPacket = srcFramesPerPacket;
        
        mDstSamplingRateInSec = dstSamplingRateInSec;
        mDstChannels = dstChannels;
        mDstBitsPerChannel = dstBitsPerChannel;
    }
    
    DESC_BLOCK("Internal heap memory allocation.")
    {
        _internalDeinit();
        
        mCodeMode = mode;
        mObjectType = objectType;
        
        mOutputBufferCount = outputBufferCount;
        mInputBufferCount = inputBufferCount;
        
        mThreadExist = false;
        mStopRequest = false;
        mInputEOSFlag = false;
        mOutputEOSFlag = false;
        mLastInputBufferFormat2AudioConverter = NULL;
        mInputSyncQueue = NULL;
        mOutputSyncQueue = NULL;
        
        _internalInit();
        
        if ( mode == CODE_MODE_DECODE )
            snprintf( myName,128,"D%d",myNumber );
        else if ( mode == CODE_MODE_ENCODE )
            snprintf( myName,128,"E%d",myNumber );
    }
    
    
    AudioStreamBasicDescription inDesc = {0}, outDesc = {0};
    AudioClassDescription audioClassDescription;

    fillInputDescription( &inDesc );
    fillOutputDescription( &outDesc );
    fillClassDescription( &audioClassDescription, true );
    
    OSStatus result;
    result = AudioConverterNewSpecific( &inDesc, &outDesc, 1, &audioClassDescription, &mhConverter );
    if ( result != noErr ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "NexAudioCodecUsingAudioToolboxConverter::init, AudioConverterNewSpecific error %d", result);
        return ERROR_INVALID_PLATFORM_PARAM;
    }
    
    char bufferName[128];
    
    snprintf( bufferName, 128, "[%s]InputQ", myName );
    mInputSyncQueue = new NexSyncNamedQueue();
    mInputSyncQueue->init( (void**)mInputBuffers, mInputBufferCount, bufferName );
    
    snprintf( bufferName, 128, "[%s]OutputQ", myName );
    mOutputSyncQueue = new NexSyncNamedQueue();
    mOutputSyncQueue->init( (void**)mOutputBuffers, mOutputBufferCount, "OutputQ" );
    
    mLastInputBufferFormat2AudioConverter = NULL;
    
    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::deinit()
{
    if ( mInputSyncQueue ) {
        mInputSyncQueue->deinit();
        delete mInputSyncQueue;
    }
    
    if ( mOutputSyncQueue ) {
        mOutputSyncQueue->deinit();
        delete mOutputSyncQueue;
    }
    
    _internalDeinit();

    return ERROR_NONE;
}


int NexAudioCodecUsingAudioToolboxConverter::setInputEOS()
{
    mInputEOSFlag = true;
    int nRet = ERROR_NONE;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioCodecUsingAudioToolboxConverter::setInputEOS(): enter.");
    
    for ( int i = 0 ; i < 100 && ERROR_NONE != ( nRet = enqueue( NULL, MINUS_END_OF_STREAM ) ) ; i++ )
        ;
    
    
    if ( nRet != ERROR_NONE ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioCodecUsingAudioToolboxConverter::setInputEOS(): what happened!!! line %d", __LINE__);
        return ERROR_ABNORMAL;
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::setInputEOS(): end.");
    
    return ERROR_NONE;
}

int NexAudioCodecUsingAudioToolboxConverter::enqueue( void* pBuff, int nLen )
{
    if ( mCodeMode == CODE_MODE_ENCODE )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[%s] enqueue was called. size is %d", myName, nLen);
    }
    BufferFormamt* pBufferFormat = (BufferFormamt*)mInputSyncQueue->getEmpty();
    if ( pBufferFormat ) {
        if ( !pBufferFormat->pBuffer )
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::enqueue(): what happened!!! line %d", __LINE__);
        
        if ( pBuff && nLen > 0 )
            memcpy( pBufferFormat->pBuffer, pBuff, nLen );
        pBufferFormat->nLen = nLen;
        
        mInputSyncQueue->markFilled();
        
        return ERROR_NONE;
    }
    else {
        return ERROR_NO_EMPTY_BUFFER;
    }
}

bool NexAudioCodecUsingAudioToolboxConverter::checkOutputAvailable()
{
    bool return_value = false;
    if ( mOutputSyncQueue )
        return_value = mOutputSyncQueue->getFilledCount() > 0 ? true : false;
    
    return return_value;
}

int NexAudioCodecUsingAudioToolboxConverter::dequeue( void* pBuff, int* pnLen )
{
    if ( !checkOutputAvailable() )
        return ERROR_NO_OUTPUT_BUFFER;
    
    BufferFormamt* pBufferFormat = (BufferFormamt*)mOutputSyncQueue->getFilled();
    if ( !pBufferFormat ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioCodecUsingAudioToolboxConverter::dequeue(): what happened!!! line %d", __LINE__);
        return ERROR_NO_OUTPUT_BUFFER;
    }
    
    if ( pBufferFormat->nLen < 0 ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioCodecUsingAudioToolboxConverter::dequeue(): EOS. line %d", __LINE__);
        return ERROR_NONE_EOS;
    }
    
    if ( *pnLen < pBufferFormat->nLen ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioCodecUsingAudioToolboxConverter::dequeue(): memory is too small. line %d", __LINE__);
        return ERROR_OUTPUT_BUUFER_TOO_SMALL;
    }
    
    memcpy( pBuff, pBufferFormat->pBuffer, pBufferFormat->nLen );
    *pnLen = pBufferFormat->nLen;
    
    mOutputSyncQueue->markEmpty();
    
    return ERROR_NONE;
}

OSStatus NexAudioCodecUsingAudioToolboxConverter::converterInputDataProc(AudioConverterRef inAudioConverter,
                                                                      UInt32* ioNumberDataPackets,
                                                                      AudioBufferList* ioData,
                                                                      AudioStreamPacketDescription** ioDataPacketDescription,
                                                                      void* inUserData)
{
    OSStatus return_value = noErr;
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "converterInputDataProc enter. ioDataPacketDescription=%p",ioDataPacketDescription);
    NexAudioCodecUsingAudioToolboxConverter *pThis = (NexAudioCodecUsingAudioToolboxConverter*)inUserData;

    
    if ( pThis->mLastInputBufferFormat2AudioConverter ) {
        pThis->mInputSyncQueue->markEmpty();
        pThis->mLastInputBufferFormat2AudioConverter = NULL;
    }
    
    BufferFormamt* pBufferFormat = NULL;
    while ( pThis->mStopRequest == false && pBufferFormat == NULL ) //&& pThis->mInputEOSFlag )
        pBufferFormat = (BufferFormamt*)pThis->mInputSyncQueue->getFilled();
    
    if ( pBufferFormat && pBufferFormat->nLen > 0 ) {
        if(ioDataPacketDescription) {
            pThis->mASPD.mDataByteSize = pBufferFormat->nLen;
            (*ioDataPacketDescription) = &pThis->mASPD;
        }
        
        ioData->mNumberBuffers = 1;
        ioData->mBuffers[0].mNumberChannels = pThis->mSrcChannels;
        ioData->mBuffers[0].mData = pBufferFormat->pBuffer;
        ioData->mBuffers[0].mDataByteSize = pBufferFormat->nLen;
        
        *ioNumberDataPackets = 1;//ioData->mBuffers[0].mDataByteSize / self->_converter_currentInputDescription.mBytesPerPacket;
        pThis->mLastInputBufferFormat2AudioConverter = pBufferFormat;
        return_value = noErr;
    }
    else {
        (*ioDataPacketDescription) = NULL;
        
        ioData->mNumberBuffers = 1;
        ioData->mBuffers[0].mNumberChannels = pThis->mSrcChannels;
        ioData->mBuffers[0].mData = NULL;
        ioData->mBuffers[0].mDataByteSize = 0;
        *ioNumberDataPackets = 0;
        
        return_value = MINUS_END_OF_STREAM;
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "converterInputDataProc end.");

    return return_value;
}


void * NexAudioCodecUsingAudioToolboxConverter::thread_proc( void* pUser )
{
    OSStatus result;
    UInt32 outputDataPacketSize;
    bool bDone = false;
    AudioBufferList outAudioBufferList;
    BufferFormamt* pBufferFormat = NULL;
    NexAudioCodecUsingAudioToolboxConverter *pThis = (NexAudioCodecUsingAudioToolboxConverter*)pUser;
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "NexAudioCodecUsingAudioToolboxConverter thread_proc started.");
    
    while( pThis->mStopRequest == false && !bDone ) {
        
        pBufferFormat = (BufferFormamt*)pThis->mOutputSyncQueue->getEmpty();
        outputDataPacketSize = pThis->mOutputDataPacketSize;
        
        if ( pBufferFormat ) {
            
            //if ( pBufferFormat->nLen > 0 ) {
                
            outAudioBufferList.mNumberBuffers = 1;
            outAudioBufferList.mBuffers[0].mNumberChannels = 2;
            outAudioBufferList.mBuffers[0].mDataByteSize = pBufferFormat->nMaxLen;// _BUFFER_SIZE;
            outAudioBufferList.mBuffers[0].mData = pBufferFormat->pBuffer;
            
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "AudioConverterFillComplexBuffer call before.");
            
            result = AudioConverterFillComplexBuffer(pThis->mhConverter,
                                                     NexAudioCodecUsingAudioToolboxConverter::converterInputDataProc,
                                                     pThis,
                                                     &outputDataPacketSize, /* UInt32 *ioOutputDataPacketSize */
                                                     &outAudioBufferList, /* AudioBufferList *outOutputData */
                                                     NULL /* AudioStreamPacketDescription *outPacketDescription */
                                                     );
            
            //nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioConverterFillComplexBuffer call after. result = %d", result);
            
            if ( result == noErr ) {
                pBufferFormat->nLen = outAudioBufferList.mBuffers[0].mDataByteSize;
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[%s]ATX: output's length = %d", pThis->myName, outAudioBufferList.mBuffers[0].mDataByteSize);
                pThis->mOutputSyncQueue->markFilled();
            }
            else {
                //?
                if ( result == MINUS_END_OF_STREAM ) {
                    pBufferFormat->nLen = MINUS_END_OF_STREAM;
                }
                else {
                    pBufferFormat->nLen = MINUS_SYSTEM_ERROR;
                }
                
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::thread_proc(): AudioConverterFillComplexBuffer's return code = %d", result);
                bDone = true;
                
                pThis->mOutputSyncQueue->markFilled();
                
            }

            
            //}
            //else {
            //    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::thread_proc(): eos %d", pBufferFormat->nLen);
            //    bDone = true;
            //}
        }
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "NexAudioCodecUsingAudioToolboxConverter thread_proc stoped.");
    return NULL;
}

int NexAudioCodecUsingAudioToolboxConverter::start()
{
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    
    int result = pthread_create( &mThread, &attr, NexAudioCodecUsingAudioToolboxConverter::thread_proc, this );
    if ( result )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::start(): error in pthread_create. error code = %d.", result);
    }
    
    mThreadExist = true;
    pthread_attr_destroy( &attr );
    
    return 0;
}

int NexAudioCodecUsingAudioToolboxConverter::stop()
{
    if ( mThreadExist ) {
        mStopRequest = true;
        void* status;
        pthread_join( mThread, &status );
        mThreadExist = false;
    }
    
    return 0;
}


/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 5, 2016                 Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
