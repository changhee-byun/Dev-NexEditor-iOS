/**
 * File Name   : NXEVisualClipInternal.h
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

#import "NXEVisualClip.h"

@interface NXEVisualClip(Internal)
@property (nonatomic) BOOL isMotionTrackedVideo;
@property (nonatomic) CGRect cropStartRectInPixels;
@property (nonatomic) CGRect cropEndRectInPixels;
// For video, it's size that regards rotation degree from it's orientation. The returned size contains swapped width and heigh if it's a portrait video, for example.
@property (nonatomic) CGSize orientationRegardedSize;
@end
