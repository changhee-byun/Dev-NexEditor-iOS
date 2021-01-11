/**
 * File Name   : CollageInfo.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *          Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
#import <CoreGraphics/CoreGraphics.h>
#import "CollageTextStyle.h"
#import "CollageType.h"

@protocol CollageClip <NSObject>
@end

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageSlotInfo: NSObject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) CGRect rect;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <NSValue *> *position;

@end

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageTitleInfo: NSObject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) CGRect rect;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <NSValue *> *position;

@end

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageSlotConfiguration: NSObject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) id<CollageClip> clip;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic) int angle;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic) FlipType flip;
/** \brief UIPinchGestureRecognizer's scale property
 *  \since version: 1.5.0
 */
@property (nonatomic) CGFloat scale;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) NSString *lutID;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic) CGPoint position;

@end

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageTitleConfiguration: NSObject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) NSString *text;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) CollageTextStyle *textStyle;

@end

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageInfo: NSObject

/** \brief TBD
 *  \since version: 1.5.0
*/
@property (nonatomic, readonly) int durationMs;

/** \brief Timestamp, in milliseconds, where all the editable collage elements are visible such as source image slots and titles
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) int editTimeMs;

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) CollageType type;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) CollageSizeInt aspectRatio;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <CollageSlotInfo *> *slots;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, readonly) NSArray <CollageTitleInfo *> *titles;

@end
