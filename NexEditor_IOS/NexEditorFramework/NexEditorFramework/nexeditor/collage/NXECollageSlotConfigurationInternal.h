/**
 * File Name   : NXECollageSlotConfigurationInternal.h
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

#import "NXECollageSlotConfiguration.h"
#import <NexEditorEngine/NexDrawInfo.h>
#import "CollageData.h"
#import "CollageType.h"

@interface NXECollageSlotConfiguration (Internal)

@property (nonatomic) BOOL isClipChanged;
@property (nonatomic) BOOL isUpdating;
@property (nonatomic, retain) CollageSlotData* slotData;
@property (nonatomic, retain) NexDrawInfo *drawInfo;

- (instancetype) initWithCollageSlotData:(CollageSlotData *)slotData slotRect:(CGRect)slotRect collageAspect:(CollageSizeInt)collageAspect;

@end
