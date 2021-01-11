/**
 * File Name   : NSError+ErrorCode.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import "NSError+ErrorCode.h"

@implementation NSError (ErrorCode)

static NSString * const kNexEditorErrorDomain = @"com.nexstreaming.nexeditor";

+ (NSString *)getDescriptionWithErrorCode:(ERRORCODE)errorCode
{
    NSArray *descriptions = @[@"none",
                              @"general",
                              @"unknown",
                              @"no action",
                              @"invalid info",
                              @"invalid state",
                              @"version mismatch",
                              @"create failed",
                              @"memalloc failed",
                              @"argument failed",
                              @"not enough memory",                    // 10
                              @"eventhandler failed",
                              @"file io failed",
                              @"file invalid syntax",
                              @"file reader create fail",
                              @"file writer create fail",
                              @"audio resampler create fail",
                              @"unsupport format",
                              @"file reader fail",
                              @"play start failed",
                              @"play stop failed",                    // 20
                              @"project not created",
                              @"project not opened",
                              @"codec init failed",
                              @"renderer init failed",
                              @"themeset create failed",
                              @"add clip failed",
                              @"encode video failed",
                              @"inprogress getclipinfo error",
                              @"thumbnail busy",
                              @"unsupport min duration",                // 30
                              @"unsupport max resolution",
                              @"unsupport min resolution",
                              @"unsupport video profile",
                              @"unsupport video level",
                              @"unsupport video fps",
                              @"transcoding busy",
                              @"transcoding not supported format",
                              @"transcoding user cancel",
                              @"transcoding not enough disk space",
                              @"transcoding codec failed",            // 40
                              @"export writer invailed handle",
                              @"export writer init fail",
                              @"export writer start fail",
                              @"export audio dec init fail",
                              @"export video dec init fail",
                              @"export video enc fail",
                              @"export video render init fail",
                              @"export not enough disk space",
                              @"unsupport audio profile",
                              @"thumbnail init failed",                // 50
                              @"unsupport audio codec",
                              @"unsupport video codec",
                              @"highlight file reader init error",
                              @"highlight too short contents",
                              @"highlight codec init error",
                              @"highlight codec decode error",
                              @"highlight render init error",
                              @"highlight writer init error",
                              @"highlight writer write error",
                              @"highlight get index error",            // 60
                              @"highlight user cancel",
                              @"background user cancel",
                              @"directexport cliplist error",
                              @"directexport check error",
                              @"directexport filereader init error",
                              @"directexport filewriter init error",
                              @"directexport dec init error",
                              @"directexport dec init surface error",
                              @"directexport dec decode error",
                              @"directexport enc init error",          // 70
                              @"directexport enc encode error",
                              @"directexport enc input surface error",
                              @"directexport enc function error",
                              @"directexport enc dis diff error",
                              @"directexport enc frame convert error",
                              @"directexport render init error",
                              @"directexport writer write error",
                              @"directexport writer unknown error",
                              @"fastpreview user cancel",
                              @"fastpreview cliplist error",           // 80
                              @"fastpreview find clip error",
                              @"fastpreview find reader error",
                              @"fastpreview video renderer error",
                              @"fastpreview dec init surface error",
                              @"hw not enough memory",
                              @"export user cancel",
                              @"fastpreview dec init error",
                              @"fastpreview filereader init error",
                              @"fastpreview time error",
                              @"fastpreview render init error",        // 90
                              @"fastpreview outputsurface init error",
                              @"fastpreview busy",
                              @"codec decode failed",
                              @"renderer audio failed",
                              @"unsupported audio samplingrate",       // 95
                              @"image process error",                  // start error index for SDK
                              @"settime ignored error",
                              @"settime canceled error",
                              @"capture failed error",
                              @"asset format error",
                              @"template configure error",
                              @"template asset error, unsupported video clip"
                              ];
    
    int endEngineErrorCode = (int)ERROR_UNSUPPORT_AUDIO_SAMPLINGRATE;
    int startSDKErrorCode = (int)ERROR_IMAGE_PROCESS;
    int startSDKErrorIndex = endEngineErrorCode + 1;
    if ( (int)errorCode > endEngineErrorCode ) {
        errorCode = (ERRORCODE)(startSDKErrorIndex + (int)errorCode - startSDKErrorCode);
    }
    
    if ( errorCode > descriptions.count ) {
        errorCode = ERROR_UNKNOWN;
    }
    return [descriptions objectAtIndex:errorCode];
}

- (instancetype)initWithErrorCode:(ERRORCODE)errorCode
{
    NSString *description = [self.class getDescriptionWithErrorCode:errorCode];
    NSDictionary *userInfo = @{NSLocalizedDescriptionKey: description};
    return [self initWithDomain:kNexEditorErrorDomain code:errorCode userInfo:userInfo];
}


@end
