/**
 * File Name   : CollageData.h
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

@interface CollageSlotData : NSObject

/** related key: source_type
 */
@property (nonatomic, readonly) NSString *sourceType;

/** related key: source_default
 */
@property (nonatomic, readonly) NSString *sourceDefault;

/** related key: audio_res
 */
@property (nonatomic, readonly) NSString *audioResource;

/** related key: lut
 */
@property (nonatomic, readonly) NSString *lut;

/** related key: draw_id
 */
@property (nonatomic, readonly) NSString *slotInfoID;

/** related key: position
 */
@property (nonatomic, readonly) NSString *position;

/** related key: start
 */
@property (nonatomic, readonly) int startTime;

/** related key: end
 */
@property (nonatomic, readonly) int endTime;

/** related key: draw_width
 */
@property (nonatomic, readonly) int slotWidth;

/** related key: draw_height
 */
@property (nonatomic, readonly) int slotHeight;

/** related key: audio_res_pos
 */
@property (nonatomic, readonly) float audioResourcePosition;

- (instancetype) initWithRawData:(NSDictionary *)raw;

@end

@interface CollageTitleData : NSObject

/** related key: title_id
 */
@property (nonatomic, readonly) NSString *titleInfoID;

/** related key: draw_id
 */
@property (nonatomic, readonly) NSString *drawInfoID;

/** related key: title_type
 */
@property (nonatomic, readonly) NSString *titleInfoType;

/** related key: position
 */
@property (nonatomic, readonly) NSString *position;

/** related key: start
 */
@property (nonatomic, readonly) int startTime;

/** related key: end
 */
@property (nonatomic, readonly) int endTime;

/** related key: max_length
 */
@property (nonatomic, readonly) int maxTitleLength;

/** related key: recommen_length
 */
@property (nonatomic, readonly) int recommendTitleLength;

/** related key: title_max_lines
 */
@property (nonatomic, readonly) int maxLine;

/** related key: title_default
 */
@property (nonatomic, readonly) NSDictionary <NSString *, NSString *> *defaultTitle;

- (instancetype) initWithRawData:(NSDictionary *)raw;

@end

@interface CollageData : NSObject

/** related key: collage_name
 */
@property (nonatomic, readonly) NSString *collageName;

/** related key: collage_version
 */
@property (nonatomic, readonly) NSString *collageVersion;

/** related key: collage_desc
 */
@property (nonatomic, readonly) NSString *collageDescription;

/** related key: collage_bgm
 */
@property (nonatomic, readonly) NSString *collageBGM;

/** related key: collage_ratio
 */
@property (nonatomic, readonly) NSString *collageRatio;

/** related key: effect
 */
@property (nonatomic, readonly) NSString *collageEffect;

/** related key: audio_res
 */
@property (nonatomic, readonly) NSString *audioResource;

/** related key: collage_type
 */
@property (nonatomic, readonly) NSString *collageType;

/** related key: audio_res_pos
 */
@property (nonatomic, readonly) float audioResourcePosition;

/** related key: collage_bgm_volume
 */
@property (nonatomic, readonly) float collageBGMVolume;

/** related key: collage_edit_time
 */
@property (nonatomic, readonly) float editTime;

/** related key: collage_project_vol_fade_in_time
 */
@property (nonatomic, readonly) int fadeInTime;

/** related key: collage_project_vol_fade_out_time
 */
@property (nonatomic, readonly) int fadeOutTime;

/** related key: collage_duation
 */
@property (nonatomic, readonly) int durationMs;

/** related key: collage_source_count
 */
@property (nonatomic, readonly) int sourceCount;

/** related key: frame_collage
 */
@property (nonatomic, readonly) BOOL isFrameCollage;

/** related key: draw_infos
 */
@property (nonatomic, readonly) NSArray <CollageSlotData *> *slots;

/** related key: title_infos
 */
@property (nonatomic, readonly) NSArray <CollageTitleData *> *titles;

- (instancetype) initWithPath:(NSString *)path error:(NSError **)error;
- (instancetype) initWithData:(NSData *)data error:(NSError **)error;

@end
