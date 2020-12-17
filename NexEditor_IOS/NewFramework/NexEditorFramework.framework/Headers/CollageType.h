/**
 * File Name   : CollageType.h
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

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

/** \brief TBD
 *  \since version: 1.5.0
 */
typedef struct  {
    /*! width */
    int32_t width;
    /*! height */
    int32_t height;
} CollageSizeInt;

/** \brief TBD
 *  \since version: 1.5.0
 */
typedef NS_ENUM(NSUInteger, FlipType)
{
    /*! */
    FlipTypeNone = 0,
    /*! */
    FlipTypeHorizontal,
    /*! */
    FlipTypeVertical
} _DOC_ENUM(FlipType);

/** \brief TBD
 *  \since version: 1.5.0
 */
typedef NS_ENUM(NSUInteger, CollageType)
{
    /*! */
    CollageTypeStatic = 0,
    /*! */
    CollageTypeDynamic
    
} _DOC_ENUM(CollageType);
