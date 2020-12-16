/*
 * File Name   : ProjectSource.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
@import NexEditorFramework;

@class AudioTrackInfo;

@interface ProjectSource: NSObject
@property (nonatomic, strong) NXETemplateAssetItem *templateAssetItem;
@property (nonatomic, strong) NSArray <NXEClip *> *clips;
@property (nonatomic, strong) NSArray <AudioTrackInfo *> *audioTracks;
@property (nonatomic, strong) NSArray <NXELayer *> *layers;
@property (nonatomic, strong) NXETextEffect *textEffect;
@property (nonatomic) NXEColorAdjustments colorAdjustments;

@property (nonatomic, readonly) BOOL isTemplate;
@property (nonatomic, readonly) NSArray <NSString *> *audioTrackTitles;
- (NXEProject *) buildProjectWithError:(NSError **) error;

@end
