/******************************************************************************
 * File Name   :	NexVideoCodecUsingVTX.h
 * Description :	The definition of Video Encoder CAL Body using VTX.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/


#ifndef NexVideoCodecUsingVTX_hpp
#define NexVideoCodecUsingVTX_hpp

#include <pthread.h>
#include "NexCalBodyTool.h"
#include "IProperty.h"
#include "IQueueComponent.h"
#include <videotoolBox/VTVideoEncoderList.h>
#include <videoToolBox/VTCompressionSession.h>

class NexSyncNamedQueue;
class NexMediaBufferFormat;

class VideoEncoderUsingVTX : public IProperty, public IQueueComponent, public UserValueHolder
{
public:
    enum {
        ERROR_NONE = 0,
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

    enum {
        PROPERTY_ID_DUMP_PATH,
        PROPERTY_ID_DEBUGLEVEL,
    };
    
    VideoEncoderUsingVTX();
    ~VideoEncoderUsingVTX();
    
    int init( int w = 1280, int h = 720, int bitrates = 3*1024*1024, int fps = 30, bool baseline = true, int outputBufferCount = OUTPUT_BUFFER_DEFAULT_COUNT );
    int deinit();
    
    static void printVideoEncoders();

    // IQueueComponent
public:
    int enqueue( void* pBuff, int nLen, long long timeInUs, long long bufferSpecificData );
    int setInputEOS();
    int dequeue( void* pBuff, int* pnLen, long long *pTimeInUs, bool *isConfig, long long * pBufferSpecificData );
    bool checkOutputAvailable();
    
    // IProperty
public:
    virtual bool getProperty( int propertyIndex, void** value );
    virtual bool setProperty( int propertyIndex, void* value );
private:
    static const int OUTPUT_BUFFER_DEFAULT_COUNT = 4;
    static const int FLAG_CONFIG_STREAM = 1;
    static const int FLAG_NORMAL_STREAM = 0;
    
    // storages for properties.
    char mDumfilePath[256];
    void* hDump = nullptr;

private:
    int mWidth;
    int mHeight;
    int mBitrate; // bits/sec
    int mFps;
    bool mBaseline;
    void* mEncodingSession;
    int mDebugLevel = 0;
    int mEnqueuedCount = 0;
    int mOutputFilledCount = 0;
    int mDequeuedCount = 0;
    int pixelBufferCount = 0;
    pthread_t mThread;
    bool mThreadExist = false;
    int parameterSetGeneratedCount = 0;
    bool mStopRequest = false;

    NexSyncNamedQueue* mOutputSyncQueue;
    int mOutputBufferCount;
    NexMediaBufferFormat** mOutputBuffers;
    
    void _internalInit();
    void _internalDeinit();
    
    int _internalVTXOpen();
    void _internalVTXClose();
    
    static void * waitCallThreadProc( void* pUser );
    static void didCompress(void *outputCallbackRefCon,
                            void *sourceFrameRefCon,
                            OSStatus status,
                            VTEncodeInfoFlags infoFlags,
                            CMSampleBufferRef sampleBuffer );
};

#endif /* NexVideoCodecUsingVTX_hpp */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 12, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
