/******************************************************************************
 * File Name   :	NexVideoCodecUsingVTX.cpp
 * Description :	The Implementation of Video Encoder CAL Body using VTX.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/


#include <string.h>
#include <videotoolBox/VTVideoEncoderList.h>
#include <videoToolBox/VTCompressionSession.h>

#include "NexSAL_Internal.h"
#include "NexSyncQueue.h"
#include "NexMediaBufferFormat.h"
#include "NexVideoCodecUsingVTX.h"
#include <deque>
#include <map>
#include <string>

#define DESC_BLOCK(a)
#define STRINGIFY(v) #v
#define VTERROR_ENTRY(error) std::pair<int, std::string>(error, STRINGIFY(error))

static std::string getVTErrorString(int error) {
    
    std::map<int, std::string> vtErrors = {
        VTERROR_ENTRY(kVTPropertyNotSupportedErr),
        VTERROR_ENTRY(kVTPropertyReadOnlyErr),
        VTERROR_ENTRY(kVTParameterErr),
        VTERROR_ENTRY(kVTInvalidSessionErr),
        VTERROR_ENTRY(kVTAllocationFailedErr),
        VTERROR_ENTRY(kVTPixelTransferNotSupportedErr),
        VTERROR_ENTRY(kVTCouldNotFindVideoDecoderErr),
        VTERROR_ENTRY(kVTCouldNotCreateInstanceErr),
        VTERROR_ENTRY(kVTCouldNotFindVideoEncoderErr),
        VTERROR_ENTRY(kVTVideoDecoderBadDataErr),
        VTERROR_ENTRY(kVTVideoDecoderUnsupportedDataFormatErr),
        VTERROR_ENTRY(kVTVideoDecoderMalfunctionErr),
        VTERROR_ENTRY(kVTVideoEncoderMalfunctionErr),
        VTERROR_ENTRY(kVTVideoDecoderNotAvailableNowErr),
        VTERROR_ENTRY(kVTImageRotationNotSupportedErr),
        VTERROR_ENTRY(kVTVideoEncoderNotAvailableNowErr),
        VTERROR_ENTRY(kVTFormatDescriptionChangeNotSupportedErr),
        VTERROR_ENTRY(kVTInsufficientSourceColorDataErr),
        VTERROR_ENTRY(kVTCouldNotCreateColorCorrectionDataErr),
        VTERROR_ENTRY(kVTColorSyncTransformConvertFailedErr),
        VTERROR_ENTRY(kVTVideoDecoderAuthorizationErr),
        VTERROR_ENTRY(kVTVideoEncoderAuthorizationErr),
        VTERROR_ENTRY(kVTColorCorrectionPixelTransferFailedErr),
        VTERROR_ENTRY(kVTMultiPassStorageIdentifierMismatchErr),
        VTERROR_ENTRY(kVTMultiPassStorageInvalidErr),
        VTERROR_ENTRY(kVTFrameSiloInvalidTimeStampErr),
        VTERROR_ENTRY(kVTFrameSiloInvalidTimeRangeErr),
        VTERROR_ENTRY(kVTCouldNotFindTemporalFilterErr),
        VTERROR_ENTRY(kVTPixelTransferNotPermittedErr),
        VTERROR_ENTRY(kVTColorCorrectionImageRotationFailedErr),
    };
    return vtErrors[error];
}

VideoEncoderUsingVTX::VideoEncoderUsingVTX()
: mEncodingSession(NULL)
, mWidth(0)
, mHeight(0)
, mBitrate(0)
, mFps(0)
, mBaseline(true)
, hDump(nullptr)
, mEnqueuedCount(0)
, mOutputFilledCount(0)
, mDequeuedCount(0)
, mThreadExist(false)
, parameterSetGeneratedCount(0)
, mOutputSyncQueue(NULL)
, mOutputBufferCount(0)
, mOutputBuffers(NULL)
, mStopRequest(false)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX : Created.");
    //printVideoEncoders();
}


VideoEncoderUsingVTX::~VideoEncoderUsingVTX()
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: delete     session=%p pbc=%d", mEncodingSession, pixelBufferCount );
    _internalDeinit();
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX : Deleted.");
}

void VideoEncoderUsingVTX::_internalInit()
{
    mOutputBuffers = (NexMediaBufferFormat**)malloc(sizeof(NexMediaBufferFormat*)*mOutputBufferCount);
    
    for ( int i = 0 ; i < mOutputBufferCount ; i++ ) {
        mOutputBuffers[i] = new NexMediaBufferFormat();
    }
    
}

void VideoEncoderUsingVTX::_internalDeinit()
{
    if ( mOutputBuffers && mOutputBufferCount > 0 ) {
        for ( int i = 0 ; i < mOutputBufferCount ; i++ ) {
            delete mOutputBuffers[i];
        }
        free(mOutputBuffers);
        mOutputBuffers = NULL;
        mOutputBufferCount = 0;
    }
}

int VideoEncoderUsingVTX::init( int w, int h, int bitrates, int fps, bool baseline, int outputBufferCount )
{
    mWidth = w;
    mHeight = h;
    mBitrate = bitrates;
    mFps = fps;
    mBaseline = baseline;
    parameterSetGeneratedCount = 0;
    mStopRequest = false;
    
    mEnqueuedCount = 0;
    mOutputFilledCount = 0;
    mDequeuedCount = 0;

    mOutputBufferCount = outputBufferCount;
    
    _internalInit();

    mOutputSyncQueue = new NexSyncNamedQueue();
    mOutputSyncQueue->init( (void**)mOutputBuffers, mOutputBufferCount, "VE_OutputQ" );
    
    int vtxRet = _internalVTXOpen();
    if(vtxRet != 0) {
        return vtxRet;
    }
    
    if ( strlen(mDumfilePath) > 0 )
        hDump = fopen( &mDumfilePath[0], "wb" );

    return 0;
}

int VideoEncoderUsingVTX::deinit()
{
    mStopRequest = true;
    
    if ( mThreadExist ) { // setInputEOS was called.
        void* status;
        pthread_join( mThread, &status );
        mThreadExist = false;
    }

    _internalVTXClose();
    
    if ( mOutputSyncQueue ) {
        mOutputSyncQueue->deinit();
        delete mOutputSyncQueue;
    }

    _internalDeinit();
    
    if ( hDump ) {
        fclose( (FILE*)hDump );
        hDump = nullptr;
    }
    
    return 0;
}

void VideoEncoderUsingVTX::_internalVTXClose()
{
    if(mEncodingSession) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: close      session=%p pbc=%d", mEncodingSession, pixelBufferCount );
        VTCompressionSessionInvalidate((VTCompressionSessionRef)mEncodingSession);
        CFRelease((VTCompressionSessionRef)mEncodingSession);
        mEncodingSession = NULL;
    }
}

void VideoEncoderUsingVTX::didCompress(void *outputCallbackRefCon,
                                       void *sourceFrameRefCon,
                                       OSStatus status,
                                       VTEncodeInfoFlags infoFlags,
                                       CMSampleBufferRef sampleBuffer )
{
    VideoEncoderUsingVTX* pThis = (VideoEncoderUsingVTX*)outputCallbackRefCon;
    NexMediaBufferFormat *pBufferFormat = NULL;

    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) sourceFrameRefCon;
    CVPixelBufferRelease(pixelBuffer);
    pThis->mEnqueuedCount--;
    pThis->pixelBufferCount--;

    CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    CMTime dts = CMSampleBufferGetDecodeTimeStamp(sampleBuffer);

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX:didCompress, enqueued: %d, pts=(%ld,%d,%d,%ld), dts=(%ld,%d,%d,%ld)", pThis->mEnqueuedCount, pts.value, pts.timescale, pts.flags, pts.epoch, dts.value, dts.timescale, dts.flags, dts.epoch);
    if ( status != noErr ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX:didCompress, status=%d, what happened!", status );
        
    }
    
    bool isKeyframe = false;
    
    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);

    if ( attachments != NULL ) {
        CFDictionaryRef attachment;
        CFBooleanRef dependsOnOthers;
        attachment = (CFDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
        if ( attachment != NULL ) {
            dependsOnOthers = (CFBooleanRef)CFDictionaryGetValue(attachment, kCMSampleAttachmentKey_DependsOnOthers);
            if ( dependsOnOthers ) {
                isKeyframe = (dependsOnOthers == kCFBooleanFalse);
                //CFRelease( dependsOnOthers );
            }
            //CFRelease( attachment );
        }
        //CFRelease( attachments );
    }
    
    if(isKeyframe) {
        
        if ( 0 < pThis->parameterSetGeneratedCount ) {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX:parameterSetGeneratedCount is %d", pThis->parameterSetGeneratedCount);
        }
        else {
            // Send the SPS and PPS.
            CMFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
            size_t spsSize, ppsSize;
            size_t parmCount;
            const uint8_t* sps, *pps;
            
            CMVideoFormatDescriptionGetH264ParameterSetAtIndex( format, 0, &sps, &spsSize, &parmCount, nullptr );
            CMVideoFormatDescriptionGetH264ParameterSetAtIndex( format, 1, &pps, &ppsSize, &parmCount, nullptr );
            
            unsigned char pss[128];
            int index = 0;
            pss[index++] = 0x01;
            pss[index++] = spsSize>>8;
            pss[index++] = spsSize&0x000000ff;
            memcpy( pss+index, sps, spsSize );
            index += spsSize;
            pss[index++] = 0x01;
            pss[index++] = ppsSize>>8;
            pss[index++] = ppsSize&0x000000ff;
            memcpy( pss+index, pps, ppsSize );
            index += ppsSize;
            
            if ( pThis->hDump ) {
                fwrite( &pss[0], 1, index, (FILE *)pThis->hDump );
            }
            
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX:didCompress keyframe");
            
            bool bDone = false;
            while ( false == pThis->mStopRequest && false == bDone ) {
                pBufferFormat = (NexMediaBufferFormat*)pThis->mOutputSyncQueue->getEmpty();
                if ( pBufferFormat ) {
                    pBufferFormat->smartCopy( pss, index );
                    pBufferFormat->mPTS = 0;
                    pBufferFormat->mDTS = 0;
                    pBufferFormat->mUserData = reinterpret_cast<void*>(FLAG_CONFIG_STREAM);
                    pThis->mOutputSyncQueue->markFilled();
                    bDone = true;
                    
                    pThis->mOutputFilledCount++;
                }
            }
        }

        pThis->parameterSetGeneratedCount++;
        
    }
    
    size_t size;
    CMBlockBufferRef block = CMSampleBufferGetDataBuffer(sampleBuffer);
    
    if ( block ) {
        char* bufferData;
        CMBlockBufferGetDataPointer( block, 0, NULL, &size, &bufferData );
        
        if ( pThis->hDump && size>0 ) {
            fwrite( &size, 1, 4, (FILE *)pThis->hDump );
            fwrite( bufferData, 1, size, (FILE *)pThis->hDump );
        }
        
        bool bDone = false;
        while ( false == pThis->mStopRequest && false == bDone ) {
            pBufferFormat = (NexMediaBufferFormat*)pThis->mOutputSyncQueue->getEmpty();
            if ( pBufferFormat ) {
                pBufferFormat->smartCopy( bufferData, static_cast<int>(size) );
                if ( pts.timescale != 0 )
                    pBufferFormat->mPTS = pts.value * 1000 * 1000 / pts.timescale;
                else
                    pBufferFormat->mPTS = 0;
                
                if ( dts.timescale != 0 )
                    pBufferFormat->mDTS = dts.value * 1000 * 1000 / dts.timescale;
                else
                    pBufferFormat->mDTS = 0;

                if ( pThis->mOutputFilledCount > 1 && pBufferFormat->mDTS == 0 && pBufferFormat->mPTS > 0 )
                    pBufferFormat->mDTS = pBufferFormat->mPTS;

                pBufferFormat->mUserData = reinterpret_cast<void*>(FLAG_NORMAL_STREAM);
                pThis->mOutputSyncQueue->markFilled();
                bDone = true;
                pThis->mOutputFilledCount++;
            }
        }
        
        //CFRelease( block );
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: didCompress stoprequest=%d, mOutputFilledCount=%d size=%d", pThis->mStopRequest, pThis->mOutputFilledCount, size );
}


int VideoEncoderUsingVTX::_internalVTXOpen()
{
    CFMutableDictionaryRef encoderSpecifications = nullptr;

    DESC_BLOCK("Fill Encoder Specification")
    {
//        CFStringRef key = kVTVideoEncoderSpecification_EncoderID;
//        CFStringRef value = CFSTR("com.apple.videotoolbox.videoencoder.h264.gva");
//        
//        CFStringRef bkey = CFSTR("EnableHardwareAcceleratedVideoEncoder");
//        CFBooleanRef bvalue = kCFBooleanTrue;
//        
//        CFStringRef ckey = CFSTR("RequireHardwareAcceleratedVideoEncoder");
//        CFBooleanRef cvalue = kCFBooleanTrue;
//        
//        //encoder specification
//        encoderSpecifications = CFDictionaryCreateMutable(
//                                                          kCFAllocatorDefault,
//                                                          3,
//                                                          &kCFTypeDictionaryKeyCallBacks,
//                                                          &kCFTypeDictionaryValueCallBacks);
//        
//        CFDictionaryAddValue(encoderSpecifications, bkey, bvalue);
//        CFDictionaryAddValue(encoderSpecifications, ckey, cvalue);
//        CFDictionaryAddValue(encoderSpecifications, key, value);
    }
    
    //source image Buffer Attributes.
    SInt32 cvPixelFormatTypeValue = ::kCVPixelFormatType_32BGRA;
    SInt8  boolYESValue = 0xFF;
    
    CFDictionaryRef emptyDict = ::CFDictionaryCreate(kCFAllocatorDefault, nil, nil, 0, nil, nil);
    CFNumberRef cvPixelFormatType = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void*)(&(cvPixelFormatTypeValue)));
    CFNumberRef frameW = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void*)(&(mWidth)));
    CFNumberRef frameH = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void*)(&(mHeight)));
    CFNumberRef boolYES = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt8Type, (const void*)(&(boolYESValue)));
    
    const void *imageBufferOptionsDictKeys[] = {
        kCVPixelBufferPixelFormatTypeKey,
        kCVPixelBufferWidthKey,
        kCVPixelBufferHeightKey,
        kCVPixelBufferOpenGLESCompatibilityKey,
        kCVPixelBufferIOSurfacePropertiesKey };
    
    const void *imageBufferOptionsDictValues[] = {
        cvPixelFormatType,
        frameW,
        frameH,
        boolYES,
        emptyDict };
    
    CFDictionaryRef imageBufferAttribs = ::CFDictionaryCreate(kCFAllocatorDefault,
                                                              imageBufferOptionsDictKeys,
                                                              imageBufferOptionsDictValues,
                                                              5, nil, nil);

    OSStatus status = VTCompressionSessionCreate(NULL, mWidth, mHeight,
                                                 kCMVideoCodecType_H264,
                                                 encoderSpecifications,
                                                 imageBufferAttribs, // sourceImageBufferAttributes
                                                 NULL,
                                                 didCompress, this,
                                                 (VTCompressionSessionRef*)&mEncodingSession);
    
    CFRelease(imageBufferAttribs);
    CFRelease(boolYES);
    CFRelease(frameH);
    CFRelease(frameW);
    CFRelease(cvPixelFormatType);
    CFRelease(emptyDict);
    
    if ( status == noErr ) {
        const int32_t v = mFps * 1; // 2-second kfi
        
        CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, ref);
        CFRelease(ref);
    }
    
    if ( status == noErr ) {
        const int v = mFps;
        CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_ExpectedFrameRate, ref);
        CFRelease(ref);
    }
    
    if ( status == noErr ) {
        CFBooleanRef allowFrameReodering = mBaseline ? kCFBooleanFalse : kCFBooleanTrue;
        status = VTSessionSetProperty(mEncodingSession , kVTCompressionPropertyKey_AllowFrameReordering, allowFrameReodering);
    }
    
    if ( status == noErr ) {
        const int v = mBitrate; //bits per second
        CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_AverageBitRate, ref);
        CFRelease(ref);
    }
    
    if ( status == noErr ) {
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_RealTime, kCFBooleanFalse);
    }
    
    if ( status == noErr ) {
        CFStringRef profileLevel = mBaseline ? kVTProfileLevel_H264_Baseline_AutoLevel : kVTProfileLevel_H264_Main_AutoLevel;
        
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_ProfileLevel, profileLevel);
    }
    
    if ( !mBaseline ) {
        status = VTSessionSetProperty(mEncodingSession, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CABAC);
    }
    
    if( status == noErr ) {
        status = VTCompressionSessionPrepareToEncodeFrames((VTCompressionSessionRef)mEncodingSession);
    }
    
    if( status != noErr ) {
        std::string error_value = getVTErrorString(status);

        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Error status = %d, %s", status, error_value.c_str());
    }
    
    return status;
    
}

int VideoEncoderUsingVTX::enqueue( void* pBuff, int nLen, long long timeInUs, long long bufferSpecificData )
{
    int result = ERROR_NONE;
    CVPixelBufferRef pixelBuff = (CVPixelBufferRef) pBuff;

    if(!mEncodingSession)
        result = ERROR_INVALID_STATE;
    
    if ( mStopRequest )
        result = ERROR_INVALID_STATE;

    if ( pixelBuff == NULL ) {
        result = ERROR_INVALID_PARAMS;
    }

    if (result != ERROR_NONE) {
        return result;
    }
    CVPixelBufferRetain(pixelBuff);
    this->pixelBufferCount++;
    
    VTCompressionSessionRef session = (VTCompressionSessionRef)mEncodingSession;

    CMTime pts = CMTimeMake(timeInUs, 1000 * 1000); // timestamp is in us.
    CMTime dur = CMTimeMake(1, mFps);
    VTEncodeInfoFlags flags;

    //memcpy( &pixelBuff, pBufferInfo->pixelBufferForExport, sizeof(CVPixelBufferRef) );
    
    
    if ( mDebugLevel > 2 ) nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: %d th enqueue + session=%p, pixelBuff=%p, pts=(%ld,%ld)", mEnqueuedCount, session, pixelBuff, pts.value, pts.timescale );
    if ( pixelBuff == nullptr)
        mDebugLevel++;
    OSStatus status = VTCompressionSessionEncodeFrame(session, pixelBuff, pts, dur, NULL, pBuff, &flags);
    if ( mDebugLevel > 2 ) nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: %d th enqueue - flags=0x%x, ret=%d", mEnqueuedCount, flags, status);
    
    if ( status == noErr )
        mEnqueuedCount++;
    else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX: %d th enqueue - fail. flags=0x%x, ret=%d", mEnqueuedCount, flags, status);
        result = ERROR_INVALID_PLATFORM_PARAM;
    }

    return result;
}

void* VideoEncoderUsingVTX::waitCallThreadProc( void* pUser )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX VTCompressionSessionCompleteFrames was called");
    VideoEncoderUsingVTX* pThis = static_cast<VideoEncoderUsingVTX*>(pUser);
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "VTX: flush      session=%p pbc=%d", pThis->mEncodingSession, pThis->pixelBufferCount );

    OSStatus status = VTCompressionSessionCompleteFrames((VTCompressionSessionRef)pThis->mEncodingSession, kCMTimeInvalid);
    //if ( pThis->mDebugLevel > 2 )
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX VTCompressionSessionCompleteFrames's ret = %d", status);

    return NULL;
}

int VideoEncoderUsingVTX::setInputEOS()
{
    if(!mEncodingSession)
        return ERROR_INVALID_STATE;
    
    DESC_BLOCK("CHECK STATE.")
    {
        if ( mThreadExist ){
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VTX: setInputEOS fails. invalid state. threadexist=%d,",mThreadExist);
            return ERROR_INVALID_STATE;
        }
    }

    pthread_attr_t attr;
    pthread_attr_init( &attr );
    
    int result = pthread_create( &mThread, &attr, waitCallThreadProc, this );
    if ( result )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodecUsingAudioToolboxConverter::start(): error in pthread_create. error code = %d.", result);
    }
    
    mThreadExist = true;
    pthread_attr_destroy( &attr );
  
    
    return ERROR_NONE;
}

bool VideoEncoderUsingVTX::checkOutputAvailable()
{
    bool return_value = false;
    if ( mOutputSyncQueue )
        return_value = mOutputSyncQueue->getFilledCount() > 0 ? true : false;
    
    return return_value;
}

int VideoEncoderUsingVTX::dequeue( void* pBuff, int* pnLen, long long *pTimeInUs, bool *isConfig, long long * pBufferSpecificData )
{
    if ( !checkOutputAvailable() )
        return ERROR_NO_OUTPUT_BUFFER;
    
    NexMediaBufferFormat* pBufferFormat = (NexMediaBufferFormat*)mOutputSyncQueue->getFilled();
    if ( !pBufferFormat ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoEncoderUsingVTX::dequeue(): what happened!!! line %d", __LINE__);
        return ERROR_NO_OUTPUT_BUFFER;
    }
    
    if ( *pnLen < pBufferFormat->getBufferLen() ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoEncoderUsingVTX::dequeue(): memory is too small. line %d", __LINE__);
        return ERROR_OUTPUT_BUUFER_TOO_SMALL;
    }
    
    *isConfig = false;
    if ( pBufferFormat->mUserData == reinterpret_cast<void*>(FLAG_CONFIG_STREAM) )
    {
        *isConfig = true;
    }
    
    memcpy( pBuff, pBufferFormat->getBufferPtr(), pBufferFormat->getBufferLen() );
    *pnLen = pBufferFormat->getBufferLen();
    *pTimeInUs = pBufferFormat->mDTS;
    
    mOutputSyncQueue->markEmpty();
    
    return ERROR_NONE;
	
}

bool VideoEncoderUsingVTX::getProperty( int propertyIndex, void** value )
{
    if ( propertyIndex == PROPERTY_ID_DEBUGLEVEL ) {
        *((int*)value) = mDebugLevel;
    }
    
    return ERROR_NOTIMPLEMENTED;
}


bool VideoEncoderUsingVTX::setProperty( int propertyIndex, void* value )
{
    if ( propertyIndex == PROPERTY_ID_DUMP_PATH ) {
        if ( (char*)value == nullptr )
            mDumfilePath[0] = '\0';
        else
            strlcpy( &mDumfilePath[0], (char*)value, sizeof(mDumfilePath) );
    }
    else if ( propertyIndex == PROPERTY_ID_DEBUGLEVEL ) {
        mDebugLevel = static_cast<int>( reinterpret_cast<long>(value) );
    }

    return ERROR_NONE;
}

void VideoEncoderUsingVTX::printVideoEncoders()
{
    CFArrayRef codeclist;
    VTCopyVideoEncoderList( NULL, &codeclist );
    int nEncoderCnt = static_cast<int>(CFArrayGetCount(codeclist));
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Video Encoders: total %d", nEncoderCnt );
    
    for ( int i = 0 ; i < nEncoderCnt ; i++ ) {
        CFDictionaryRef encDic = (CFDictionaryRef)CFArrayGetValueAtIndex(codeclist,i);
        int nCount = static_cast<int>(CFDictionaryGetCount(encDic));
        if ( nCount > 0 ) {
            const void ** keys = (const void**)malloc(nCount*sizeof(void*));
            
            CFDictionaryGetKeysAndValues(encDic, keys, NULL);
            for (int i = 0; i < nCount; i++) {
                const char * keyStr = CFStringGetCStringPtr((CFStringRef)keys[i], CFStringGetSystemEncoding());
                CFTypeRef keyType = CFDictionaryGetValue( encDic, keys[i] );
                if ( CFGetTypeID(keyType) == CFStringGetTypeID() ) {
                    const char * valStr = CFStringGetCStringPtr((CFStringRef)keyType, CFStringGetSystemEncoding());
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "- %s/%s", keyStr,valStr);
                }
                else if ( CFGetTypeID(keyType) == CFNumberGetTypeID() ) {
                    CFNumberType ntype = CFNumberGetType((CFNumberRef)keyType);
                    int usagePage;
                    CFNumberGetValue( ( CFNumberRef )keyType, kCFNumberSInt32Type, &usagePage );
                    
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "- %s/[%d]0x%x", keyStr,ntype,usagePage);
                }
                else {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "- %s/[not pritable type]", keyStr);
                }
                CFRelease(keyType);
            }
            
            if ( keys )
                free( keys );
        }
        CFRelease(encDic);
    }
    CFRelease(codeclist);
}

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 12, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
