/*
 * File Name   : ProjectSource.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "ProjectSource.h"
#import "AudioTrackDataController.h"

@implementation ProjectSource
- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.colorAdjustments = NXEColorAdjustmentsMake(0, 0, 0);
    }
    return self;
}

- (NXEProject *) buildProjectWithError:(NSError **) error
{
    NXEProject *project = [self buildProjectWithClips:self.clips error:error];
    if ( project == nil ) {
        return nil;
    }
    
    project.layers = self.layers;
    if (self.textEffect) {
        if (![project setTextEffect:self.textEffect error:error]) {
            return nil;
        }
    }
    project.colorAdjustments = self.colorAdjustments;
    
    // Audio tracks
    NSArray <NXEClip *> *audioClips = self.audioClips;
    if (self.isTemplate && audioClips.count > 0) {
        [project setBgmClip:audioClips.firstObject];
    } else {
        project.audioClips = audioClips;
    }
    
    return project;
}

#pragma mark -
- (BOOL) isTemplate
{
    return self.templateAssetItem != nil;
}

- (NXEProject *) buildProjectWithClips:(NSArray<NXEClip *> *) clips error:(NSError **) error
{
    NXEProject *result = nil;
    
    if (self.templateAssetItem) {
        result = [[NXETemplateProject alloc] initWithTemplateAssetItem:self.templateAssetItem clips:clips error:error];
    } else {
        result = [[NXEProject alloc] init];
        result.visualClips = clips;
    }
    [result updateProject];
    
    return result;
}

- (NSArray <NXEClip *> *) audioClips
{
    NSMutableArray<NXEClip *> *audioClips = [NSMutableArray array];
    for( AudioTrackInfo *track in self.audioTracks) {
        NSError *error = nil;
        NXEClip *clip = [track createClipOrError:&error];
        if (clip) {
            [audioClips addObject:clip];
        } else {
            NSLog(@"ERROR: Failed creating audio clip from audio track:%@", error.localizedDescription);
        }
    }
    return [audioClips copy];
}

- (NSArray <NSString *> *) audioTrackTitles
{
    NSMutableArray <NSString *> *titles = [NSMutableArray array];
    for(AudioTrackInfo *trackInfo in self.audioTracks) {
        [titles addObject:trackInfo.title];
    }
    return [titles copy];
}
@end
