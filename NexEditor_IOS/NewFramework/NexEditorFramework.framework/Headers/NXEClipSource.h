/*
 * File Name   : NXEClipSource.h
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
#import <Photos/Photos.h>
#import <MediaPlayer/MediaPlayer.h>
/**
 * \class NXEClipSource
 * \brief NXEClipSource represents a source media can be used for creating a NXEClip instance using \c +clipWithSource:error: method.
 * \since version 1.2
 */
@interface NXEClipSource : NSObject

/**
 * \brief Short text describes the receiver for debugging purpose only.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *shortDisplayText;

/**
 * \brief Creates a clip source with a \c PHAsset object fetched from Photos Library through Photos framework
 * \param phasset Image or Video asset from Photo Library
 * \return An NXEClipSource object.
 * \since version 1.2
 */
+ (instancetype) sourceWithPHAsset:(PHAsset *) phasset;
/**
 * \brief Creates a clip source with a \c AVURLAsset object either constructed from a file URL or fetched from a PHAsset
 * \param avasset AVURLAsset represents a file video media
 * \return An NXEClipSource object.
 * \since version 1.2
 */
+ (instancetype) sourceWithAVURLAsset:(AVURLAsset *) avasset;
/**
 * \brief Creates a clip source with a path to a file that is either \c UIImage compatible image or \c AVURLAsset compatible video.
 * \param path Path to a image or video file
 * \return An NXEClipSource object.
 * \since version 1.2
 */
+ (instancetype) sourceWithPath:(NSString *) path;

/**
 * \brief Creates a clip source with a MPMediaItem fetched from \c MPMediaQuery API such as MPMediaQuery
 * \param mediaItem MPMediaItem audio clip source
 * \return An NXEClipSource object.
 * \since version 1.2
 */
+ (instancetype) sourceWithMediaItem:(MPMediaItem *) mediaItem;
@end
