/**
 * File Name   : NXEBeatAssetItemInfo.h
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
#import "NXEBeatEffect.h"
#import "NXEEffectEntry.h"

@interface NXEBeatAssetItemInfo: NSObject

- (instancetype) initWithData:(NSData *)data;

@property (nonatomic, readonly) NSString *title;
@property (nonatomic, readonly) NSString *audioId;
@property (nonatomic, readonly) NSInteger version;
@property (nonatomic, readonly) NXEBeatEffectDefault *defaultEffect;
@property (nonatomic, readonly) NSArray<NXEEffectEntry*> *entries;
@property (nonatomic, readonly) NSArray<NSNumber*> *positions;

@end
