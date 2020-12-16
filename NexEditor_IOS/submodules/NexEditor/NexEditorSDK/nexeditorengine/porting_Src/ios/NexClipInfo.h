/******************************************************************************
 * File Name   : NexClipInfo.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#ifndef NexClipInfo_h
#define NexClipInfo_h

#import <Foundation/Foundation.h>
#import "NexMediaDef.h"

@interface NexClipInfo : NSObject

/** If the media include video, this is 1; otherwise it is 0. */
@property (nonatomic, assign) int existVideo;

/** If the media include audio, this is 1; otherwise it is 0. */
@property (nonatomic, assign) int existAudio;

/** Width of the video, in pixels */
@property (nonatomic, assign) int videoWidth;

/** Height of the video, in pixels */
@property (nonatomic, assign) int videoHeight;

/** Duration of the clip, in second */
@property (nonatomic, assign) int audioDuration;

/** Duration of the clip, in second */
@property (nonatomic, assign) int videoDuration;

/** Seek point count of the clip */
@property (nonatomic, assign) int seekPointCount;

/** Seek point count of the clip */
@property (nonatomic, readonly) NSArray<NSNumber *>* seekTable;

/** Frame per second of the clip */
@property (nonatomic, assign) int fps;

/** Video codec(H264) profile of the clip
 *  Unknown : 0
 *  Baseline : 66
 *  Main : 77
 *  Extended : 88
 *  High : 100
 *  High10 : 110
 *  High422 : 122
 *  High444 : 244 */
@property (nonatomic, assign) int videoH264Profile;

/** Video codec(H264) Level of the clip */
@property (nonatomic, assign) int videoH264Level;

/** Video Bitrateof the clip */
@property (nonatomic, assign) int videoBitRate;

/** Video Orientation the clip */
@property (nonatomic, assign) int videoOrientation;

/** Audio SampleRate the clip */
@property (nonatomic, assign) int audioSampleRate;

/** Audio channels the clip */
@property (nonatomic, assign) int audioChannels;

/** Audio Bitrateof the clip */
@property (nonatomic, assign) int audioBitRate;

/** File path of thumbnail, if available thumbnail. */
@property (nonatomic, readonly) NSString* thumbnailPath;

/** Video Codec Type */
@property (nonatomic) NEX_CODEC_TYPE videoCodecType;

/** Audio Codec Type */
@property (nonatomic) NEX_CODEC_TYPE audioCodecType;

@end


#endif /* NexClipInfo_h */
