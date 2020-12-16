/**
 * File Name   : NXEEnginePrivate.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "NXEEngine.h"
#import <NexEditorEngine/NexEditor.h>

@class NexDrawInfo;

@interface NXEEngine(Private)
@property (class, nonatomic) NXEAspectType aspectType;
@property (class, nonatomic) NXESizeInt aspectRatio;
@property (class, readonly) NSString *productName;
@property (nonatomic, readonly) NexEditor *videoEditor;

+ (void) setAspectType:(NXEAspectType) type withRatio:(NXESizeInt) ratio;
- (void) setLayers:(NSArray<NXELayer *> *)layers;
- (void) adjustProjectColorWith:(NXEColorAdjustments)colorAdjustments;
- (void) updateDrawInfo:(NexDrawInfo *)drawInfo;
- (void) resolveProject:(BOOL)flag; // flag > 1: preview, 0: export

@end
