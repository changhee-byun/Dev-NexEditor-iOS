/**
 * File Name   : NXEBeatAssetItemInfo.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

#import "NXEBeatAssetItemInfo.h"
#import "NSString+ValueReader.h"

static NSString *const kTitle = @"string_title";
static NSString *const kAudioId = @"string_audio_id";
static NSString *const kVersion = @"int_version";
static NSString *const kDefaultEffect = @"default_effect";
static NSString *const kEffectEntries = @"list_effectEntries";

@interface NXEBeatAssetItemInfo()

@property (nonatomic, strong) NSString *title;
@property (nonatomic, strong) NSString *audioId;
@property (nonatomic) NSInteger version;
@property (nonatomic, strong) NXEBeatEffectDefault *defaultEffect;
@property (nonatomic, strong) NSArray<NXEBeatEffect*> *effects;

@end


@implementation NXEBeatAssetItemInfo

- (instancetype) initWithData:(NSData *)data {
    self = [super init];
    NSDictionary *source = [NSJSONSerialization JSONObjectWithData:data
                                                            options:NSJSONReadingAllowFragments
                                                              error:nil];
    
    if (source && [source isKindOfClass:[NSDictionary class]]) {
        self.title = source[kTitle];
        self.audioId = source[kAudioId];
        self.version = [source[kVersion] intValue];
        self.defaultEffect = [[[NXEBeatEffectDefault alloc] initWithSource:source[kDefaultEffect]] autorelease];
        self.effects = [self configureEffects:source[kEffectEntries]];
    }
    return self;
}

- (NSArray *)configureEffects:(NSArray *)source
{
    NSMutableArray *list = [[[NSMutableArray alloc] init] autorelease];

    if (source.count != 0) {
        for (NSDictionary *item in source) {
            NXEBeatEffect *entry = [[[NXEBeatEffect alloc] initWithSource:item] autorelease];
            [list addObject:entry];
        }
    }
    return list;
}

- (void)dealloc
{
    self.audioId = nil;
    self.title = nil;
    self.defaultEffect = nil;
    self.effects = nil;
    [super dealloc];
}

@end
