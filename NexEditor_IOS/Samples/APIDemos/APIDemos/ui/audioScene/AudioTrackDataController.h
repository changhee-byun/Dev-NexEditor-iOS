/******************************************************************************
 * File Name   :	AudioTrackDataController.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>
@import NexEditorFramework;

@class NXEClip;

/**
 *  AudioTrackInfo 클래스
 *  ---------------------
 *  title        : 음원의 제목이 보여진다.
 *  filePath     : AudioTrack Path 정보, 해당 정보를 이용해 파일명을 표시한다.
 *  background   : AudioTrack이 background로 사용할지 혹은 단순히 audioTrack 사용할지 결정
 *  loop         : AudioTrack정보가 Loop재생을 할지 안 할지 결정
 *  volumeLevel  : audioTrack 사용할 경우, 0~200 으로 설정이 되지만, background 사용할 경우, 0~1.0으로 scaling값으로 설정해야함.
 *  trimStartTime   : trimStart Time
 *  trimEndTime     : trimEnd Time
 *  duration     : Audio Track duration, unit seconds.
 *  audioChanger    : audio changer (0:none, 1:chipmunk, 2:robert, 3:deep, 4:modulation)
 *  envelopList  : AudioTrack대한 envelop List정보
 */
@interface AudioTrackInfo : NSObject

@property (nonatomic, retain) NSString *title;
@property (nonatomic, strong) NXEClipSource *clipSource;
@property (nonatomic, assign) BOOL background;
@property (nonatomic, assign) BOOL loop;
@property (nonatomic, assign) int volumeLevel;
@property (nonatomic, assign) int trimStartTime;
@property (nonatomic, assign) int trimEndTime;
@property (nonatomic, assign) int startTime;
@property (nonatomic, assign) int duration;
@property (nonatomic, assign) BOOL compressor;
@property (nonatomic, assign) int panLeft;
@property (nonatomic, assign) int panRight;
@property (nonatomic, assign) int pitch;
@property (nonatomic, assign) int audioChanger;
@property (nonatomic, retain) NSMutableArray* envelopList;

/**
 * Creates an audio NXEClip from the receiver
 */
- (NXEClip *)createClipOrError:(NSError **) error;

@end

@interface AudioTrackDataController : NSObject

@property (nonatomic, retain) NSMutableArray *audioTrackList;

/**
 * \brief AudioFileList 에서 해당 API를 통해 AudioTrackInfo 업뎃을 한 뒤, 리스트를 갱신 해야하며, 
 *  audioTrackList property를 기준으로 그려줘야 한다.
 */
- (void)setAudioTrack:(AudioTrackInfo *)audioTrackInfo;

- (void)replaceAudioTrack:(int)trackId audioTrackInfo:(AudioTrackInfo *)audioTrackInfo;

@end
