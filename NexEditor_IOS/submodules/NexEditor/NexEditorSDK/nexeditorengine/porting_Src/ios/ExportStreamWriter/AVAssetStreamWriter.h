/*
 * File Name   : AVAssetStreamWriter.h
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

#import <Foundation/Foundation.h>
#import "StreamWriter.h"

/// Creates a media file using AVAssetWriter by appending incoming streams through StreamWriter protocol interface.
@interface AVAssetStreamWriter : NSObject <StreamWriter>
@property (nonatomic, readonly) StreamWriterSettings *settings;
@property (nonatomic, readonly) StreamWriterContext *context;

+ (instancetype) writerWithSettings:(StreamWriterSettings *) settings;
@end
