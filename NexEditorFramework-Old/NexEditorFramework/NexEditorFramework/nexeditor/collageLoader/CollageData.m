/**
 * File Name   : CollageData.m
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

#import "CollageData.h"

static NSString *const kCName = @"collage_name";
static NSString *const kCVersion = @"collage_version";
static NSString *const kCDescription = @"collage_desc";
static NSString *const kCBGM = @"collage_bgm";
static NSString *const kCBMGVolume = @"collage_bgm_volume";
static NSString *const kCType = @"collage_type";
static NSString *const kCDuration = @"collage_duration";
static NSString *const kCEditTime = @"collage_edit_time";
static NSString *const kCRatio = @"collage_ratio";
static NSString *const kCSourceCount = @"collage_source_count";
static NSString *const kCAudioRes = @"audio_res";
static NSString *const kCAudioResPos = @"audio_res_pos";
static NSString *const kCEffect = @"effect";
static NSString *const kCFrameCollage = @"frame_collage";
static NSString *const kCSlotInfos = @"draw_infos";
static NSString *const kCTitleInfos = @"title_infos";
static NSString *const kCFadeInTime = @"collage_project_vol_fade_in_time";
static NSString *const kCFadeOutTime = @"collage_project_vol_fade_out_time";

static NSString *const kSStart = @"start";
static NSString *const kSEnd = @"end";
static NSString *const kSType = @"source_type";
static NSString *const kSDefaultType = @"source_default";
static NSString *const kSPosition = @"position";
static NSString *const kSAudioRes = @"audio_res";
static NSString *const kSAudioResPos = @"audio_res_pos";
static NSString *const kSLUT = @"lut";
static NSString *const kSSlotID = @"draw_id";
static NSString *const kSSlotWidth = @"draw_width";
static NSString *const kSSlotHeight = @"draw_height";

static NSString *const kTStart = @"start";
static NSString *const kTEnd = @"end";
static NSString *const kTPosition = @"position";
static NSString *const kTTitleID = @"title_id";
static NSString *const kTType = @"title_type";
static NSString *const kTSlotID = @"draw_id";
static NSString *const kTMaxLength = @"max_length";
static NSString *const kTRecommendLength = @"recommend_length";
static NSString *const kTMaxLine = @"title_max_lines";
static NSString *const kTDefaultTitle = @"title_default";


@interface CollageSlotData()

@property (nonatomic, strong) NSString *sourceType;
@property (nonatomic, strong) NSString *sourceDefault;
@property (nonatomic, strong) NSString *audioResource;
@property (nonatomic, strong) NSString *lut;
@property (nonatomic, strong) NSString *slotInfoID;
@property (nonatomic, strong) NSString *position;

@end


@implementation CollageSlotData

- (instancetype) initWithRawData:(NSDictionary *)raw
{
    self = [super init];
    
    _sourceType = raw[kSType];
    _sourceDefault = raw[kSDefaultType];
    _audioResource = raw[kSAudioRes];
    _lut = raw[kSLUT];
    _slotInfoID = raw[kSSlotID];
    _position = raw[kSPosition];
    _startTime = [raw[kSStart] intValue];
    _endTime = [raw[kSEnd] intValue];
    _slotWidth = [raw[kSSlotWidth] intValue];
    _slotHeight = [raw[kSSlotHeight] intValue];
    _audioResourcePosition = [raw[kSAudioResPos] floatValue];
    
    return self;
}

@end


@interface CollageTitleData()

@property (nonatomic, strong) NSString *titleInfoID;
@property (nonatomic, strong) NSString *drawInfoID;
@property (nonatomic, strong) NSString *titleInfoType;
@property (nonatomic, strong) NSString *position;
@property (nonatomic, strong) NSDictionary <NSString *, NSString *> *defaultTitle;

@end


@implementation CollageTitleData

- (instancetype) initWithRawData:(NSDictionary *)raw
{
    self = [super init];
    
    _titleInfoID = raw[kTTitleID];
    _drawInfoID = raw[kTSlotID];
    _titleInfoType = raw[kTType];
    _position = raw[kTPosition];
    _startTime = [raw[kTStart] intValue];
    _endTime = [raw[kTEnd] intValue];
    _maxTitleLength = [raw[kTMaxLength] intValue];
    _recommendTitleLength = [raw[kTRecommendLength] intValue];
    _maxLine = [raw[kTMaxLine] intValue];
    
    NSMutableDictionary *list = [[NSMutableDictionary alloc] init];
    NSDictionary *rawtitle = (NSDictionary *)raw[kTDefaultTitle];
    for (NSString *key in [rawtitle allKeys]) {
        [list setObject:rawtitle[key] forKey:key];
    }
    _defaultTitle = list;
    
    return self;
}

@end


@interface CollageData()

@property (nonatomic, strong) NSString *collageName;
@property (nonatomic, strong) NSString *collageVersion;
@property (nonatomic, strong) NSString *collageDescription;
@property (nonatomic, strong) NSString *collageBGM;
@property (nonatomic, strong) NSString *collageRatio;
@property (nonatomic, strong) NSString *collageEffect;
@property (nonatomic, strong) NSString *audioResource;
@property (nonatomic, strong) NSString *collageType;
@property (nonatomic, strong) NSArray <CollageSlotData *> *slots;
@property (nonatomic, strong) NSArray <CollageTitleData *> *titles;

@end


@implementation CollageData

- (NSArray *)configureSlotList:(NSArray *)rawData
{
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    if (rawData.count != 0) {
        for (NSDictionary *raw in rawData) {
            CollageSlotData *slotData = [[CollageSlotData alloc] initWithRawData:raw];
            [list addObject:slotData];
        }
    }
    
    return [list copy];
}

- (NSArray *)configureTitleList:(NSArray *)rawData
{
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    if (rawData.count != 0) {
        for (NSDictionary *raw in rawData) {
            CollageTitleData *titleData = [[CollageTitleData alloc] initWithRawData:raw];
            [list addObject:titleData];
        }
    }
    
    return [list copy];
}

- (instancetype) initWithPath:(NSString *)path error:(NSError **)error
{
    if (path == nil) {
        if (error) {
            *error = [NSError errorWithDomain:NSOSStatusErrorDomain code:-1 userInfo:nil];
        }
        return nil;
    }
    
    NSData *data = [[NSData alloc] initWithContentsOfFile:path options:0 error:error];
    if (data == nil) {
        NSLog(@"Can't load from path:%@ %@", path, error != nil ? [*error localizedDescription] : @"");
        return nil;
    }

    id result = [self initWithData:data error:error];
    if ( result == nil ) {
        NSLog(@"Can't load %@", error != nil ? [*error localizedDescription] : @"");
    }
    return result;
}

- (instancetype) initWithData:(NSData *)data error:(NSError **)error
{
    self = [super init];
    
    NSDictionary *raw = [NSJSONSerialization JSONObjectWithData:data
                                                        options:NSJSONReadingAllowFragments
                                                          error:error];
    if (raw == nil) {
        return nil;
    }
    
    _collageName = raw[kCName];
    _collageVersion = raw[kCVersion];
    _collageDescription = raw[kCDescription];
    _collageBGM = raw[kCBGM];
    _collageRatio = raw[kCRatio];
    _collageEffect = raw[kCEffect];
    _audioResource = raw[kCAudioRes];
    _collageType = raw[kCType];
    _audioResourcePosition = [raw[kCAudioResPos] floatValue];
    _collageBGMVolume = [raw[kCBMGVolume] floatValue];
    _editTime = [raw[kCEditTime] floatValue];
    _fadeInTime = [raw[kCFadeInTime] intValue];
    _fadeOutTime = [raw[kCFadeOutTime] intValue];
    _sourceCount = [raw[kCSourceCount] intValue];
    _isFrameCollage = [raw[kCFrameCollage] boolValue];
    _slots = [self configureSlotList:(NSArray *)raw[kCSlotInfos]];
    _titles = [self configureTitleList:(NSArray *)raw[kCTitleInfos]];
    _durationMs = [raw[kCDuration] intValue];
    return self;
}

@end
