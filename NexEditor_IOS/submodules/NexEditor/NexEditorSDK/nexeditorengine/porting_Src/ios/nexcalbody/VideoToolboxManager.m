//
//  VideoToolboxManager.m
//  test_codec
//
//  Created by YooSunghyun on 2/17/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#import "VideoToolboxManager.h"
#import "NexSAL_Internal.h"

#import "NexCodecUtil.h"

#define VTDPARAMETERSETS_MAX_ENTRIES    10
#define IMAGEBUFFER_MANAGEMENT_STRICT
#define CFSafeRelease(x) if (x) { CFRelease(x); }

#pragma mark - Parameter Sets Scanner
typedef struct {
    const uint8_t *pParameterSets[VTDPARAMETERSETS_MAX_ENTRIES];
    size_t lengths[VTDPARAMETERSETS_MAX_ENTRIES];
    size_t count;
} VTDParameterSets;

typedef enum {
    NALUnitType_VPS = 32,
    NALUnitType_SPS = 33,
    NALUnitType_PPS = 34
} NALUnitType;

typedef void (*NTRReleaseImageBuffer)(NXEDecoderOutputImageBuffer *owner);

static VTDParameterSets scanH264ParameterSets(const uint8_t *pDSI, size_t dsiLength);
static VTDParameterSets scanHEVCParameterSets(const uint8_t *pDSI, size_t dsiLength);
static void callbackReleaseImageBuffer(NXEDecoderOutputImageBuffer *owner);

@interface NXEDecoderOutputImageBuffer()
@property(nonatomic,retain) NXEVideoToolboxManager* vtm;
@property (nonatomic) NTRReleaseImageBuffer releaseImageBuffer;
@end


#pragma mark -
@implementation NXEDecoderOutputImageBuffer

+ (NSMutableSet*) entries
{
    static NSMutableSet* _entries = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _entries = [[NSMutableSet alloc] init];
    });
    return _entries;
}

+ (void) addEntryWithPointer:(void*)addr {
	@autoreleasepool {
		@synchronized(self) {
			[self.entries addObject:[NSValue valueWithPointer:addr]];
		}
	}
}

+ (void) removeEntryWithPointer:(void*)addr {
	@autoreleasepool {
		@synchronized(self) {
			[self.entries removeObject:[NSValue valueWithPointer:addr]];
		}
	}
}

+ (BOOL) isManagedImageBufferPointer:(void*)addr {
	BOOL result = NO;
	@autoreleasepool {
        @synchronized(self) {
            result = [self.entries containsObject:[NSValue valueWithPointer:addr]];
        }
	}
	return result;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
		[self.class addEntryWithPointer:self];
    }
    return self;
}

- (void) setImage:(CVImageBufferRef)image
{
    if ( _image != nil ) {
        CVPixelBufferRelease(_image);
        _image = nil;
    }
    if ( image != NULL) {
        CVPixelBufferRetain(image);
    }
    _image = image;
}

- (void) dealloc
{
	[self.class removeEntryWithPointer:self];
    self.vtm = nil;
    self.image = NULL;
    [super dealloc];
}

- (void) releaseResource
{
    if (self.releaseImageBuffer) {
        self.releaseImageBuffer(self);
    }
}
@end

@implementation NXEPTSQObject
- (instancetype) initWithPts:(unsigned int) pts
{
    self = [super init];
    if (self) {
        self.pts = pts;
    }
    return self;
}

@end

@interface NXEVideoToolboxManager()
@property (nonatomic,retain) NSMutableArray<NXEDecoderOutputImageBuffer *>* outputQueue;
#ifdef  IMAGEBUFFER_MANAGEMENT_STRICT
@property (nonatomic,retain) NSMutableArray* imageBufferQueueForRelease;
#endif

@property (nonatomic,retain) NSMutableArray<NXEPTSQObject *>* ptsQueue;
@property (nonatomic, strong) NSData *dsiData;
@property (nonatomic) VTDCodecType codecType;
@end

#define MAX_USER_VALUES 10

@implementation NXEVideoToolboxManager {
    CMFormatDescriptionRef formatDesc;
    VTDecompressionSessionRef decodeSession;
    unsigned int lastInputPTS;
    int userValues[10];
}

const static char* _FormatError(char *str, OSStatus error)
{
    /*
     VTError.h
     ----------
     */
    static const char *_VTErrorString129XX[] = {
        "kVTPropertyNotSupportedErr", //				= -12900,
        "kVTPropertyReadOnlyErr", //					= -12901,
        "kVTParameterErr", //							= -12902,
        "kVTInvalidSessionErr", //					= -12903,
        "kVTAllocationFailedErr", //					= -12904,
        "kVTPixelTransferNotSupportedErr", //			= -12905, // c.f. -8961
        "kVTCouldNotFindVideoDecoderErr", //			= -12906,
        "kVTCouldNotCreateInstanceErr", //			= -12907,
        "kVTCouldNotFindVideoEncoderErr", //			= -12908,
        "kVTVideoDecoderBadDataErr", //				= -12909, // c.f. -8969
        "kVTVideoDecoderUnsupportedDataFormatErr", //	= -12910, // c.f. -8970
        "kVTVideoDecoderMalfunctionErr", //			= -12911, // c.f. -8960
        "kVTVideoEncoderMalfunctionErr", //			= -12912,
        "kVTVideoDecoderNotAvailableNowErr", //		= -12913,
        "kVTImageRotationNotSupportedErr", //			= -12914,
        "kVTVideoEncoderNotAvailableNowErr", //		= -12915,
        "kVTFormatDescriptionChangeNotSupportedErr", //	= -12916,
        "kVTInsufficientSourceColorDataErr", //		= -12917,
        "kVTCouldNotCreateColorCorrectionDataErr", //	= -12918,
        "kVTColorSyncTransformConvertFailedErr", //	= -12919,
    };
    static const char *_VTErrorString122XX[] = {
        "kVTVideoDecoderAuthorizationErr", //			= -12210,
        "kVTVideoEncoderAuthorizationErr", //			= -12211,
        "kVTColorCorrectionPixelTransferFailedErr", //	= -12212,
        "kVTMultiPassStorageIdentifierMismatchErr", //	= -12213,
        "kVTMultiPassStorageInvalidErr", //			= -12214,
        "kVTFrameSiloInvalidTimeStampErr", //			= -12215,
        "kVTFrameSiloInvalidTimeRangeErr", //			= -12216,
        "kVTCouldNotFindTemporalFilterErr", //		= -12217,
        "kVTPixelTransferNotPermittedErr", //			= -12218,
    };
    const char *ret = NULL;
    if ( error <= -12900 && error >= -12919) {
        ret = _VTErrorString129XX[-(error + 12900)];
    } else if ( error <= -12210 && error >= -12218) {
        ret = _VTErrorString122XX[-(error + 12210)];
    }
    
    if ( ret == NULL ) {
        if ( str != NULL ) {
            // see if it appears to be a 4-char-code
            *(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
            if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
                str[0] = str[5] = '\'';
                str[6] = '\0';
            } else {
                // no, format it as an integer
                sprintf(str, "%d", (int)error);
            }
            ret = str;
        }
    }
    
    if ( ret == NULL ) {
        ret = "Unknown";
    }
    return ret;
}

- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.ptsQueue = [[[NSMutableArray alloc] init] autorelease];
        self.outputQueue = [[[NSMutableArray alloc] init] autorelease];
        NSMutableArray *outputImages = [[NSMutableArray alloc] init];
        self.imageBufferQueueForRelease = outputImages;
        [outputImages release];
        [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
    }
    return self;
}

- (void) dealloc
{
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
    
    self.dsiData = nil;
    self.ptsQueue = nil;
    self.outputQueue = nil;
    self.imageBufferQueueForRelease = nil;

    [super dealloc];
}

void decodeOutputCB( void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime presentationDuration )
{
    NXEVideoToolboxManager* decoder = (__bridge NXEVideoToolboxManager*)decompressionOutputRefCon;
    
    // This pts is for removing ptsValue from ptsQueue. In case error(drop), presentationTimeStamp comes with kCMTimeInvalid.
    CMTime pts = [(NSValue*)sourceFrameRefCon CMTimeValue];
    CFSafeRelease((CFTypeRef)sourceFrameRefCon);
    
    if ( status == noErr) {
        NXEDecoderOutputImageBuffer * output = [[[NXEDecoderOutputImageBuffer alloc] init] autorelease];
        output.vtm = decoder;
        output.pts = (unsigned int)presentationTimeStamp.value;
        output.image = imageBuffer;
        output.releaseImageBuffer = callbackReleaseImageBuffer;
        
        @synchronized(decoder) {
            int i = 0;
            for ( i = 0 ; decoder.outputQueue.count > i; i++ ) {
                if(decoder.outputQueue[i].pts > output.pts)
                    break;
            }
            [decoder.outputQueue insertObject:output atIndex:i];
        }
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "VTB Decoded frame(pts:%d, %p)", output.pts, output.image);
        [decoder logStatsWithTag:[NSString stringWithFormat:@"%s(%d)", __FUNCTION__, __LINE__]];

    } else {
        char convErr[6] = {0,};
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "DecodeOutput Error:%s, pts:%d, frameDropped:%s", _FormatError(convErr, status),
                        (unsigned int)pts.value, ((infoFlags & kVTDecodeInfo_FrameDropped) ? "true" : "false") );
        
        @synchronized(decoder) {
            for ( NXEPTSQObject *ptsQ in decoder.ptsQueue ) {
                if ( ptsQ.pts == (unsigned int)pts.value ) {
                    [decoder.ptsQueue removeObject:ptsQ];
                    break;
                }
            }
        }
    }
}

- (void)completeAllAsynchronousFrames
{
    if ( decodeSession ) {
        VTDecompressionSessionWaitForAsynchronousFrames(decodeSession);
    }
}

void releaseImageBuffer(void* imageBuffer)
{
    callbackReleaseImageBuffer((NXEDecoderOutputImageBuffer *) imageBuffer);
}

static void callbackReleaseImageBuffer(NXEDecoderOutputImageBuffer *owner)
{
    NXEDecoderOutputImageBuffer * doib = (NXEDecoderOutputImageBuffer *)owner;
    if ([NXEDecoderOutputImageBuffer isManagedImageBufferPointer:(void *)doib]) {
         nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "releaseImageBuffer pts=%d retainCount=%lu", doib.pts, (unsigned long)doib.retainCount);
        [doib.vtm releaseImageBufferWithPointer:doib.image];
    }
}

unsigned int getPTSFromImageBuffer(void* imageBuffer)
{
    unsigned int result = 0;
    NXEDecoderOutputImageBuffer * doib = (NXEDecoderOutputImageBuffer *)imageBuffer;
    if ([NXEDecoderOutputImageBuffer isManagedImageBufferPointer:(void*)doib]) {
        result = (unsigned int)doib.pts;
    }
    return result;
}

-(void)releaseImageBufferWithPointer:(CVImageBufferRef)image
{
    @synchronized (self.imageBufferQueueForRelease) {

        int i;
        NXEDecoderOutputImageBuffer* output = nil;
        
        for ( i = 0 ; i < _imageBufferQueueForRelease.count ; i++ )
        {
            output = self.imageBufferQueueForRelease[i];
            if ( output.image == image )
                break;
            else {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 1, "releaseImageBufferWithPointer. first entry is not target. %d th, pts = %d",i,output.pts);
            }
        }
        
        if ( i == _imageBufferQueueForRelease.count )
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "releaseImageBufferWithPointer error. what happened!. no buffer pointer(%p)",image);
        else {
            if ( i != 0 ) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "releaseImageBufferWithPointer. first entry is not deleted(%p), pts=%d",image,output.pts);
            } else {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "releaseImageBufferWithPointer. first entry was deleted, pts=%d", output.pts);
            }
            
            [self.imageBufferQueueForRelease removeObjectAtIndex:i];
        }
    }
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
}


-(VTDError)createDecoderWithOTI:(VTDCodecType)uOTI dsi:(unsigned char*)pDSI dsiLen:(int)nDSILen
{
    self.dsiData = [NSData dataWithBytes:pDSI length:nDSILen];
    self.codecType = uOTI;
    return [self createDecoderWithDSIData:self.dsiData codecType:uOTI];
}


- (CMFormatDescriptionRef) createParameterSetsFor:(VTDCodecType)codecType pDSI:(unsigned char *)pDSI dsiLength:(size_t) dsiLength
{
    CMFormatDescriptionRef result = NULL;
    
    if (codecType == VTDCodecTypeH264) {
        VTDParameterSets parameterSets = scanH264ParameterSets(pDSI, dsiLength);
        CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                            parameterSets.count,
                                                            parameterSets.pParameterSets,
                                                            parameterSets.lengths,
                                                            4,
                                                            &result );
    } else if ( codecType == VTDCodecTypeHEVC) {
        BOOL supported = NO;
        
#if defined(__IPHONE_11_0) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0)
        if (@available(iOS 11.0, *)) {
            VTDParameterSets parameterSets = scanHEVCParameterSets(pDSI, dsiLength);
            CMVideoFormatDescriptionCreateFromHEVCParameterSets(kCFAllocatorDefault,
                                                                parameterSets.count,
                                                                parameterSets.pParameterSets,
                                                                parameterSets.lengths, 4, NULL, &result);
            supported = YES;
        }
#endif
        if (!supported) {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "VTB HEVC not supported on this version of iOS");
        }
    }
    return result;
}

-(VTDError)createDecoderWithDSIData:(NSData *) dsiData codecType: (VTDCodecType) codecType
{
    VTDError result = VTDErrorNone;
    
    // only for AVC
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];

    unsigned char *pDSI = (unsigned char *) dsiData.bytes;
    formatDesc = NULL;
    decodeSession = NULL;
    
    for ( int i = 0 ; i < MAX_USER_VALUES ; i++ )
    {
        userValues[i] = 0;
    }

    formatDesc = [self createParameterSetsFor:codecType pDSI:pDSI dsiLength:dsiData.length];
    
    if (formatDesc == NULL) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "VTB Failed creating parameter sets for codecType:%d", codecType);
        result = VTDErrorBadParameterSets;
    }
    
    if (result == VTDErrorNone)
    {
        //const void* videoDecoderSpecificationKeys[] = { kVTDecompressionPropertyKey_RealTime };
        //const void* videoDecoderSpecificationValues[] = { kCFBooleanTrue };
        CFDictionaryRef videoDecoderSpecification = (__bridge CFDictionaryRef) @{
                (NSString*)kVTDecompressionPropertyKey_RealTime : (NSNumber *)kCFBooleanTrue};
        
        CFDictionaryRef outputBufferAttributes = (__bridge CFDictionaryRef) @{
                                                                              (NSString *) kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange),
                                                                              //(NSString *) kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32RGBA),
                
                (NSString *) kCVPixelBufferOpenGLCompatibilityKey: (NSNumber *)kCFBooleanTrue};

        VTDecompressionOutputCallbackRecord callback = { decodeOutputCB, (__bridge void*)self };
        //callback.decompressionOutputCallback = didDecompress;
        //callback.decompressionOutputRefCon = (__bridge void*)self;
        
        OSStatus ret = VTDecompressionSessionCreate(kCFAllocatorDefault, formatDesc, videoDecoderSpecification, outputBufferAttributes, &callback, &decodeSession);
        if (ret != noErr) {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "VTB Failed creating decompression session for codecType:%d", codecType);
            result = VTDErrorSessionCreation;
        }
    }
    return result;
}

- (void) logStatsWithTag:(NSString *) tag
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "VTB vtm: %p output: %d release: %d %s", self, (int) self.outputQueue.count, self.imageBufferQueueForRelease.count, tag.UTF8String);
}

-(void)releaseRemainingOutputBuffers
{
    @synchronized(self) {
        NSInteger count = self.outputQueue.count;
        [self.outputQueue removeAllObjects];
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "Decoder(%p)'s releaseRemainingOutputBuffers count=%d.",self, (int)count);
        [self.ptsQueue removeAllObjects];
    }

    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
}

-(void)releaseRemainingOutputBuffersForRelease
{
    @synchronized (self.imageBufferQueueForRelease) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 0, "Decoder(%p)'s releaseRemainingOutputBuffersForRelease count=%d",self, self.imageBufferQueueForRelease.count);
        [self.imageBufferQueueForRelease removeAllObjects];
    }
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
}

-(VTDError)deleteDecoder
{
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];

    if ( decodeSession ) {
        // NESI-504 Not waiting async frames causes a hang in VTDecompressionSessionInvalidate()
        VTDecompressionSessionWaitForAsynchronousFrames(decodeSession);
        VTDecompressionSessionInvalidate(decodeSession);
        
        CFRelease(decodeSession);
        decodeSession = nil;
    }
    [self releaseRemainingOutputBuffers];
    [self releaseRemainingOutputBuffersForRelease];
    
    if (formatDesc != NULL) {
        CFRelease(formatDesc);
        formatDesc = NULL;
    }

    return kVTM_Error_None;
}

-(VTDError)decodeWithFrame:(unsigned char*)pData length:(int)len dts:(unsigned int)dts pts:(unsigned int)pts
{
    if(len <= 0) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "ignored this Frame length:%d, dts:%d, pts:%d",len, dts, pts);
        return kVTM_Error_None;
    }
    CMBlockBufferRef blockBufferRef = NULL;
    CMSampleBufferRef sampleBuffer = NULL;
    size_t sampleSize = len;
    CMSampleTimingInfo timingInfo = {0,};
    VTDecodeInfoFlags outFlags = 0UL;

    timingInfo.duration = CMTimeMake(30, 1000);
    timingInfo.decodeTimeStamp = CMTimeMake(dts, 1000);
    timingInfo.presentationTimeStamp = CMTimeMake(pts, 1000);
    
    CMBlockBufferCreateWithMemoryBlock( kCFAllocatorDefault, pData, len, kCFAllocatorNull, NULL, 0, len, 0, &blockBufferRef);
    CMSampleBufferCreate(kCFAllocatorDefault, blockBufferRef, true, NULL, NULL, formatDesc, 1, 1, &timingInfo, 1, &sampleSize, &sampleBuffer);
    @autoreleasepool {
        NSValue* ptsValue = [NSValue valueWithCMTime:timingInfo.presentationTimeStamp];
        VTDecompressionSessionDecodeFrame(decodeSession, sampleBuffer, kVTDecodeFrame_EnableAsynchronousDecompression, (void*)CFBridgingRetain(ptsValue), &outFlags);
    }

    CFSafeRelease(sampleBuffer);
    CFSafeRelease(blockBufferRef);
    
    @synchronized(self) {
        int i = 0;
        for ( i = 0 ; self.ptsQueue.count > i ; i++ ) {
            if( self.ptsQueue[i].pts > pts)
                break;
        }
        NXEPTSQObject* ptsQObject = [[[NXEPTSQObject alloc] initWithPts:pts] autorelease];
        [self.ptsQueue insertObject:ptsQObject atIndex:i];
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "decodeWithFrame length=%d, dts=%d, pts=%d",len, dts,pts);

    return kVTM_Error_None;
}

-(BOOL)checkOutputAvailable
{
    BOOL retValue = NO;
    @synchronized(self) {
        if ( self.outputQueue.count > 0 && self.ptsQueue.count > 0 ) {
            NXEPTSQObject *ptso = self.ptsQueue[0];
            NXEDecoderOutputImageBuffer *outo = self.outputQueue[0];
            retValue = ( ptso.pts == outo.pts );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "c=%d",retValue);
		}
    }
    return retValue;
}

-(BOOL)isOutputPtsLastPts
{
    BOOL retValue = FALSE;

    if( self.ptsQueue.count == 1 ) {
        retValue = [self checkOutputAvailable];
    }
    
    return retValue;
}

-(NXEDecoderOutputImageBuffer*)getFirstOutputBuffer
{
    if ( FALSE == [self checkOutputAvailable] )
        return nil;
    
    NXEDecoderOutputImageBuffer * output;
    
    @synchronized(self) {
        output = [self.outputQueue[0] retain];
        [self.outputQueue removeObjectAtIndex:0];
        [self.ptsQueue removeObjectAtIndex:0];
    }

    @synchronized (self.imageBufferQueueForRelease) {
        [_imageBufferQueueForRelease addObject:output];
        [output release];
    }

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_VIDEO, 3, "[VTM] getFirstOutputBuffer. pts=%d", output.pts);
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];
    return output;
}

-(void)reset
{
    [self logStatsWithTag:[NSString stringWithFormat:@"%@(%d)", NSStringFromSelector(_cmd), __LINE__]];

    [self releaseRemainingOutputBuffers];
    [self releaseRemainingOutputBuffersForRelease];
    
    [self deleteDecoder];
    [self createDecoderWithDSIData:self.dsiData codecType:self.codecType];
}

-(void)setUserIntValueWithIndex:(int)index value:(int)v
{
    if ( index > 0 && index < MAX_USER_VALUES )
        userValues[index] = v;
}

-(int)getUserIntValueWithIndex:(int)index
{
    if ( index > 0 && index < MAX_USER_VALUES )
        return userValues[index];
    
    return 0;
}

@end

void configureVTDPrameterSetsForH264(VTDParameterSets *result, int8_t nalType, const uint8_t *pDSI, int dsiSize) {
	int xpsSize = 0;
	uint8_t* xpsAddr = NexCodecUtil_FindNAL((NXUINT8*)pDSI, dsiSize, NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, 0, nalType, &xpsSize);
	if(xpsSize > 0)    {
		result->pParameterSets[result->count] = xpsAddr;
		result->lengths[result->count] = xpsSize;
		result->count += 1;
	}
}

static VTDParameterSets scanH264ParameterSets(const uint8_t *pDSI, size_t dsiLength)
{
	//ANNEXB로 받은 configdata에서 SPS, PPS 만 추출해 넘겨주자.
	VTDParameterSets set2ret = {0};
	
	configureVTDPrameterSetsForH264(&set2ret, eNEX_NAL_SPS, pDSI, (int)dsiLength);
	configureVTDPrameterSetsForH264(&set2ret, eNEX_NAL_PPS, pDSI, (int)dsiLength);
	
	return set2ret;
}

void configureVTDPrameterSetsForHEVC(VTDParameterSets *result, int8_t nalType, const uint8_t *pDSI, int dsiSize) {
	int xpsSize = 0;
	
	uint8_t *pTemp = (uint8_t*)pDSI;
	int iTempSize = dsiSize;
	
	while(iTempSize > 4) {
		uint8_t* xpsAddr = NexCodecUtil_HEVC_FindNAL((NXUINT8*)pTemp, iTempSize, 0, nalType, &xpsSize);
		if(xpsSize > 0)    {
			result->pParameterSets[result->count] = xpsAddr;
			result->lengths[result->count] = xpsSize;
			result->count += 1;
			
			iTempSize -= (unsigned int)xpsAddr + xpsSize - (unsigned int)pTemp;
			pTemp = xpsAddr + xpsSize;
		} else {
			break;
		}
	}
}

static VTDParameterSets scanHEVCParameterSets(const uint8_t *pDSI, size_t dsiLength)
{
	VTDParameterSets set2ret = {0};
	
	//ANNEXB로 받은 configdata에서 VPS, SPS, PPS 추출해 넘겨주자.
	configureVTDPrameterSetsForHEVC(&set2ret, eNEX_HEVC_NALU_TYPE_VPS, pDSI, (int)dsiLength);
	configureVTDPrameterSetsForHEVC(&set2ret, eNEX_HEVC_NALU_TYPE_SPS, pDSI, (int)dsiLength);
	configureVTDPrameterSetsForHEVC(&set2ret, eNEX_HEVC_NALU_TYPE_PPS, pDSI, (int)dsiLength);
	
	return set2ret;
}
