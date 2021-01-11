/******************************************************************************
 * File Name   : MediaInfo.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>
#import "NXEError.h"

@interface MediaInfo : NSObject <NSCopying>

//@property (nonatomic, assign) int version;
//@property (getter=getPath, nonatomic, assign) NSString *path;
@property (getter=getFileSize, nonatomic, assign) long fileSize;
@property (nonatomic, assign) BOOL hasAudio;
@property (nonatomic, assign) BOOL hasVideo;
@property (nonatomic, assign) BOOL hasImage;
@property (nonatomic, assign) int videoWidth;
@property (nonatomic, assign) int videoHeight;
@property (nonatomic, assign) int audioDuration;
@property (nonatomic, assign) int videoDuration;
@property (nonatomic, assign) int seekPointCount;
@property (nonatomic, assign) int fps;
@property (nonatomic, assign) int videoH264Profile;
@property (nonatomic, assign) int videoH264Level;
@property (nonatomic, assign) int audioBitrate;
@property (nonatomic, assign) int audioSamplingRate;
@property (nonatomic, assign) int audioChannels;
@property (nonatomic, assign) int videoOrientation;
@property (nonatomic, assign) int videoBitRate;
@property (nonatomic, assign) int audioBitRate;
@property (nonatomic, assign) int rotateDegree;

- (instancetype) initWithPath:(NSString *)path error:(NXEError **)error;

@end
