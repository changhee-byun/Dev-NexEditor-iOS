/******************************************************************************
 * File Name   :	CERangeSliderCallbackBlock.h
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


#ifndef CERangeSliderCallbackBlock_h
#define CERangeSliderCallbackBlock_h

@interface CERangeSliderCallbackBlock : NSObject

- (id) initWithCallbackBlock:(void (^)(bool isTouchedCenterKnob))endTrackingWithTouchCallbackBlock ContinueTrackingWithTouchCallbackBlock:(void (^)(bool isTouchedCenterKnob))continueTrackingWithTouchCallbackBlock;

- (void) endTrackingWithTouchCallback:(bool)isTouchedCenterKnob;
- (void) continueTrackingWithTouchCallback:(bool)isTouchedCenterKnob;

@end

#endif /* CERangeSliderCallbackBlock_h */
