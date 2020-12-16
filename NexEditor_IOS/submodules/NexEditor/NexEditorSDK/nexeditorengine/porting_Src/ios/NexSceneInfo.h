/**
 * File Name   : NexSceneInfo.h
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

#import <Foundation/Foundation.h>

@interface NexSceneInfo : NSObject

@property (nonatomic) int sceneId;
@property (nonatomic) int subEffectId;
@property (nonatomic) BOOL isTransition;
@property (nonatomic, retain) NSString *effectId;
@property (nonatomic, retain) NSString *titleValue;
@property (nonatomic) int startTimeMS;
@property (nonatomic) int endTimeMS;

@end
