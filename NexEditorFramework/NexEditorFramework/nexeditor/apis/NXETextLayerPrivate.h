/**
 * File Name   : NXETextLayerPrivate.h
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

#import "NXETextlayer.h"

@interface NXETextLayer(Private)
/** \brief A rectangle information of the text layer.
 *  \note It is recommended not to set manually, because the clip properties are set automatically by calling the initWithText:font:point:.
 *  \since version 1.0.19
 */
@property (nonatomic) CGPoint layerPoint;
@end
