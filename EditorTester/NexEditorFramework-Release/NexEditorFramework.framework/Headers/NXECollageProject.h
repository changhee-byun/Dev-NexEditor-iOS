/**
 * File Name   : NXECollageProject.h
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

#import "NXEProject.h"
#import "NXEError.h"
#import "CollageInfo.h"
#import "NXECollageSlotConfiguration.h"
#import "NXECollageTitleConfiguration.h"

@class NXEAssetItem;

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface NXECollageProject : NXEProject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) CollageInfo *info;

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <NXECollageSlotConfiguration *> *slots;

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <NXECollageTitleConfiguration *> *titles;

/** \brief TBD
 *  \since version: 1.5.0
 */
- (instancetype) initWithCollageAssetItem:(NXEAssetItem *)assetItem error:(NSError **)error;

/** \brief TBD
 *  \since version: 1.5.0
 */
- (void) beginUpdates;

/** \brief TBD
 *  \since version: 1.5.0
 */
- (void) endUpdates;
@end

@interface CollageInfo(Convert)
/** \brief TBD
 *  \since version: 1.5.0
 */
- (NSArray<NSValue *>*) convertFromPositions:(NSArray<NSValue *>*)positions toUIView:(UIView *)view;

/** \brief TBD
 *  \since version: 1.5.0
 */
- (CGRect) convertFromRect:(CGRect)rect toUIView:(UIView *)view;
@end

