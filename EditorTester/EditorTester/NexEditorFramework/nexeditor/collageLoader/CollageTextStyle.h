/**
 * File Name   : CollageTextStyle.h
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
#import <UIKit/UIKit.h>

/** \brief TBD
 *  \since version: 1.5.0
 */
@interface CollageTextStyle : NSObject

/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) UIFont *font;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) UIColor *fillColor;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) UIColor *strokeColor;
/** \brief TBD
 *  \since version: 1.5.0
 */
@property (nonatomic, strong) UIColor *shadowColor;

@end
