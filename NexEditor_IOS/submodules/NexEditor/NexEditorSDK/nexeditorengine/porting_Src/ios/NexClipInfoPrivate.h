/*
 * File Name   : NexClipInfo.h
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

#ifndef NexClipInfoPrivate_h
#define NexClipInfoPrivate_h

#import <Foundation/Foundation.h>
#import "NexClipInfo.h"

@interface NexClipInfo(Private)
@property (nonatomic, strong) NSArray<NSNumber *>* seekTable;
@property (nonatomic, strong) NSString* thumbnailPath;
@end

#endif /* NexClipInfoPrivate_h */
