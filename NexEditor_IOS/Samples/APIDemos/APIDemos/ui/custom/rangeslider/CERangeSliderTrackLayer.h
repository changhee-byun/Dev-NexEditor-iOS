/******************************************************************************
 * File Name   :	CERangeSliderTrackLayer.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <QuartzCore/QuartzCore.h>

@protocol RangeSliderProtocol;

@interface CERangeSliderTrackLayer : CALayer

@property (nonatomic, weak) id<RangeSliderProtocol> slider;

@end
