//
//  VideoToolboxManager.h
//  test_codec
//
//  Created by YooSunghyun on 2/17/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <videoToolBox/VTDecompressionSession.h>
#import <videoToolBox/VTCompressionSession.h>
#import "NexThemeRenderer_Platform_iOS-API.h"

typedef enum {
    VTDErrorNone                = 0,
    VTDErrorBadParameterSets,
    VTDErrorSessionCreation,
    kVTM_Error_None             = VTDErrorNone,
} VTDError;

typedef VTDError VTMErrorCode;

typedef enum {
    VTDCodecTypeH264,
    VTDCodecTypeHEVC
} VTDCodecType;

@class NXEVideoToolboxManager;

@interface NXEDecoderOutputImageBuffer: NSObject <NexThemeRendererImageBuffer>
@property (nonatomic) NSUInteger pts;
@property (nonatomic) CVImageBufferRef image;
@end

@interface NXEPTSQObject : NSObject
@property(nonatomic,assign) unsigned int pts;
@end

#define MAX_USER_VALUES 10

@interface NXEVideoToolboxManager : NSObject

-(VTDError)createDecoderWithOTI:(VTDCodecType)uOTI dsi:(unsigned char*)pDSI dsiLen:(int)nDSILen;
-(VTDError)deleteDecoder;
-(VTDError)decodeWithFrame:(unsigned char*)pData length:(int)len dts:(unsigned int)dts pts:(unsigned int)pts;
-(BOOL)checkOutputAvailable;
-(BOOL)isOutputPtsLastPts;

//getFirstOutputBuffer is just for client. internal usage is forbidden.
-(NXEDecoderOutputImageBuffer*)getFirstOutputBuffer;

-(void)reset;
//-(void)releaseImageBufferWithPointer:(CVImageBufferRef)image;
-(void)setUserIntValueWithIndex:(int)index value:(int)v;
-(int)getUserIntValueWithIndex:(int)index;

-(void)completeAllAsynchronousFrames;

@end
