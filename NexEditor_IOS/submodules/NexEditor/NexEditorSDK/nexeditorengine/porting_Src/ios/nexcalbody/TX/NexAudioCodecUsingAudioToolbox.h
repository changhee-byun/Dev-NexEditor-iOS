/******************************************************************************
 * File Name   :	NexAudioCodecUsingAudioToolbox.h
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

#ifndef NexAudioCodecUsingAudioToolbox_hpp
#define NexAudioCodecUsingAudioToolbox_hpp

#include <pthread.h>
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/AudioConverter.h>
#include <CoreMedia/CMBlockBuffer.h>

#include "NexCalBodyTool.h"

#define DESC_BLOCK(a)

class NexSyncNamedQueue;

class NexAudioCodec
{
    
};

class NexAudioCodecUsingAudioToolboxConverter : public NexAudioCodec, public UserValueHolder
{
public:
    static const int INPUT_BUFFER_DEFAULT_COUNT = 4;
    static const int OUTPUT_BUFFER_DEFAULT_COUNT = 4;
    static const int AAC_SAMPLES_PER_CHANNEL = 1024;
    enum {
        ERROR_NONE = 0,
        ERROR_NONE_EOS,
        ERROR_NO_EMPTY_BUFFER,
        ERROR_NO_OUTPUT_BUFFER,
        ERROR_OUTPUT_BUUFER_TOO_SMALL,
        ERROR_ABNORMAL,
        ERROR_INVALID_PLATFORM_PARAM,
        ERROR_INVALID_STATE,
        ERROR_INVALID_PARAMS,
        ERROR_NOT_SUPPORTED,
        ERROR_NOTIMPLEMENTED,
        ERROR_MAKE_INTEGER_TYPE = 0x7fffffff
    };
    
    typedef enum {
        CODE_MODE_NONE = 0,
        CODE_MODE_DECODE = 1,
        CODE_MODE_ENCODE = 2
    } CODE_MODE;
    
    typedef enum {
        OBJECT_NONE = 0,
        OBJECT_AAC,
        OBJECT_MP3
    } OBJECT_TYPE;
    
    typedef struct _BufferFormat_ {
        void* pBuffer;
        int nLen;
        int nMaxLen;
    } BufferFormamt;
    
    NexAudioCodecUsingAudioToolboxConverter();
    ~NexAudioCodecUsingAudioToolboxConverter();
    
    int init( CODE_MODE mode
             ,OBJECT_TYPE objectType
             ,int srcSamplingRateInSec
             ,int srcChannels
             ,int srcBitsPerChannel
             ,int srcFramesPerPacket
             ,int dstSamplingRateInSec
             ,int dstChannels
             ,int dstBitsPerChannel
             ,int inputBufferCount = INPUT_BUFFER_DEFAULT_COUNT
             ,int outputBufferCount = OUTPUT_BUFFER_DEFAULT_COUNT );
    int deinit();
    int start();
    int stop();
    int enqueue( void* pBuff, int nLen );
    int setInputEOS();
    int dequeue( void* pBuff, int* pnLen );
    bool checkOutputAvailable();
    
    
    static OSStatus converterInputDataProc(AudioConverterRef inAudioConverter,
                                           UInt32* ioNumberDataPackets,
                                           AudioBufferList* ioData,
                                           AudioStreamPacketDescription** ioDataPacketDescription,
                                           void* inUserData);
private:
    static const int MINUS_END_OF_STREAM = -21;
    static const int _BUFFER_SIZE = 1024*24;
    static const int MINUS_SYSTEM_ERROR = -1;
    
    static void* thread_proc( void* pUser );
    
    
    CODE_MODE mCodeMode; // 0 : decode, 1 : encode
    OBJECT_TYPE mObjectType;
    
    int myNumber;
    char myName[128];

    int mSrcSamplingRateInSec;
    int mSrcChannels;
    int mSrcBitsPerChannel;
    int mSrcFramesPerPacket; // only meaningful when decoder mode

    int mDstSamplingRateInSec;
    int mDstChannels;
    int mDstBitsPerChannel;

    AudioConverterRef mhConverter = NULL;
    pthread_t mThread;
    bool mThreadExist;
    bool mStopRequest;
    
    int mOutputBufferCount;
    BufferFormamt** mOutputBuffers;
    //int mIndexForOutputBuffer;
    
    int mInputBufferCount;
    BufferFormamt** mInputBuffers;
    
    BufferFormamt* mLastInputBufferFormat2AudioConverter;
    
    bool mInputEOSFlag;
    bool mOutputEOSFlag;
    
    NexSyncNamedQueue* mInputSyncQueue;
    NexSyncNamedQueue* mOutputSyncQueue;
    
    AudioStreamPacketDescription mASPD;
    unsigned int mOutputDataPacketSize;
    
    void _internalInit();
    void _internalDeinit();

    int fillDescription4PCM( AudioStreamBasicDescription* target2fill, bool bSrc );
    int fillDescription4AAC( AudioStreamBasicDescription* target2fill, bool bSrc );
    int fillDescription4MP3( AudioStreamBasicDescription* target2fill, bool bSrc );
    
    int fillInputDescription( AudioStreamBasicDescription* target2fill );
    int fillOutputDescription( AudioStreamBasicDescription* target2fill );
    int fillClassDescription( AudioClassDescription* target2fill, bool bSWCodec );
    
    void fillTargetParams( bool bSrc, int* pSamplingRate, int* pBitsPerChannels, int* pCannels);

};

#endif /* NexAudioCodecUsingAudioToolbox_hpp */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 5, 2016                 Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/

