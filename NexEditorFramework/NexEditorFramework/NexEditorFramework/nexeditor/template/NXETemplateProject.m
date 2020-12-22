/**
 * File Name   : NXETemplateProject.m
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

#import "NXETemplateProject.h"
#import "NXETemplateParser.h"
#import "NXETemplateAssetItem.h"
#import "NXEClip.h"
#import "NXEError.h"
#import "NXEProjectInternal.h"
#import "NXETemplateProject+Internal.h"
#import "AssetLibrary.h"
#import <NexEditorEngine/NexEditorEngine.h>

@interface NXETemplateProject()
@property (nonatomic) TemplateVersion version;
@property (nonatomic) NXEAspectType aspectType;
@property (nonatomic, retain) NSMutableArray <NexSceneInfo *> *sceneInfos;
@property (nonatomic, retain) NSMutableArray <NexDrawInfo *> *drawInfos;
@end

@implementation NXETemplateProject

- (instancetype) initWithTemplateAssetItem:(NXETemplateAssetItem *)templateAssetItem clips: (NSArray<NXEClip *> *)clips error:(NSError **)error
{
    if (self = [super init]) {
        self.visualClips = clips;
        self.sceneInfos = [[[NSMutableArray alloc] init] autorelease];
        self.drawInfos = [[[NSMutableArray alloc] init] autorelease];
        self.aspectType = templateAssetItem.aspectType;
        NXETemplateParser *templateParser = [[NXETemplateParser alloc] init];
        [templateParser configureProject:self withAssetItem:(NXETemplateAssetItem *)templateAssetItem];
        if (self.version == TemplateVersion3x) {
            NSPredicate *predicate = [NSPredicate predicateWithFormat:@"clipType == %@", @(NXE_CLIPTYPE_VIDEO)];
            if ([clips filteredArrayUsingPredicate:predicate].count > 0) {
                NexLogE(LOG_TAG_PROJECT, @"in current template 3.x, not support video clip");
                if (error != nil) {
                    *error = [[NXEError alloc] initWithErrorCode:ERROR_TEMPLATE_ASSET_UNSUPPORT_VIDEOCLIPS];
                }
                [templateParser release];
                [self release];
                return nil;
            }
        }
        [templateParser release];
    }

    return self;
}

- (void) dealloc
{
    self.sceneInfos = nil;
    self.drawInfos = nil;
    
    [super dealloc];
}

- (int) getTotalTime
{
    if (self.version == TemplateVersion3x) {
        int result = 0;
        for (NXEClip *clip in self.visualClips) {
            result = result < clip.vinfo.endTime ? clip.vinfo.endTime : result;
        }
        return result;
    }
    return [super getTotalTime];
}

- (void) updateProject
{
    if (self.version == TemplateVersion3x) {
        return; // skip in case of template 3.x
    }
    [super updateProject];
}

- (NSArray<NexSceneInfo *> *)sceneList
{
    return self.sceneInfos;
}

- (NSArray<NexDrawInfo *> *)slotList
{
    return self.drawInfos;
}

@end
