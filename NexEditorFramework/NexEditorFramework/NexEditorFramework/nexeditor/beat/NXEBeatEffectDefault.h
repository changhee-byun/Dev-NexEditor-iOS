/**
 * File Name   : NXEBeatEffectDefault.h
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

@interface NXEBeatEffectDefault : NSObject

@property (nonatomic, readonly) NSString *colorFilterId;
@property (nonatomic, readonly) NSString *clipEffectId;
@property (nonatomic, readonly) NSString *scColorFilterId;
@property (nonatomic, readonly) NSString *scClipEffectId;

- (instancetype) initWithSource:(NSDictionary *)source;

@end
