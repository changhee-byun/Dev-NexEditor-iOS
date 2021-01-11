/**
 * File Name   : NXEClipSource.m
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

#import "NXEClipSource.h"
#import "NXEClipSourcePrivate.h"

@protocol NXEClipSource
- (NSString *) path;
- (NSString *) shortDisplayText;
@end

@interface PHAsset(ClipSource) <NXEClipSource>
@end

@interface NSString(ClipSource) <NXEClipSource>
@end

@interface AVURLAsset(ClipSource) <NXEClipSource>
@end

@interface MPMediaItem(ClipSource) <NXEClipSource>
@end

@interface AVComposition(Path)
@property (nonatomic, readonly) AVCompositionTrackSegment *firstSegment;
@end

@interface NXEPHAssetClipSource: NXEClipSource
@property (nonatomic, strong) AVURLAsset *avurlAsset;
@property (nonatomic, strong) AVComposition *avComposition;
@end


@interface NXEClipSource()
@property (nonatomic, strong) id<NXEClipSource> source;
- (NSString *) path;
- (instancetype) initWithClipSource:(id<NXEClipSource>) source;
@end


@implementation NXEClipSource

- (instancetype) initWithClipSource:(id<NXEClipSource>) source
{
    self = [super init];
    if (self ) {
        self.source = source;
    }
    return self;
}

- (void) dealloc {
    self.source = nil;
    [super dealloc];
}

- (NSString *) path
{
    return [self.source path];
}

- (NSString *) shortDisplayText
{
    return [self.source shortDisplayText];
}

+ (instancetype) sourceWithPath:(NSString *) path
{
    return [[[self alloc] initWithClipSource:path] autorelease];
}

+ (instancetype) sourceWithPHAsset:(PHAsset *) phasset
{
    return [[[NXEPHAssetClipSource alloc] initWithClipSource:phasset] autorelease];
}

+ (instancetype) sourceWithAVURLAsset:(AVURLAsset *) avasset
{
    return [[[self alloc] initWithClipSource:avasset] autorelease];
}

+ (instancetype) sourceWithMediaItem:(MPMediaItem *) mediaItem
{
    return [[[self alloc] initWithClipSource:mediaItem] autorelease];
}

@end

#pragma mark -

@implementation NSString(ClipSource)

- (NSString *) path
{
    return self;
}

- (NSString *) shortDisplayText
{
    return self.lastPathComponent;
}

@end

@implementation AVComposition(Path)
- (AVCompositionTrackSegment *) firstSegment
{
    AVCompositionTrackSegment *result = nil;
    NSArray *tracks = self.tracks;
    for ( AVCompositionTrack *track in tracks ) {
        result = track.segments.firstObject;
        if (result) {
            break;
        }
    }
    return result;
}
@end

@implementation PHAsset(ClipSource)

- (NSString *) path
{
    NSString *result = nil;
    if (self.mediaType == PHAssetMediaTypeImage) {
        result = [NSString stringWithFormat:@"phasset-image://%@", self.localIdentifier];
    } else if (self.mediaType == PHAssetMediaTypeVideo) {
        result = self._avurlAsset.path;
        if (result == nil) {
            AVComposition *composition = self._avComposition;
            result = composition.firstSegment.sourceURL.path;
        }
    }
    return result;
}

- (NSString *) shortDisplayText
{
    NSString *result = @"PHAsset";
    if (self.mediaType == PHAssetMediaTypeImage) {
        result = [NSString stringWithFormat:@"%@ Image:%@", result, self.localIdentifier];
    } else if (self.mediaType == PHAssetMediaTypeVideo) {
        result = [NSString stringWithFormat:@"%@ Video:%@", result, self.path.lastPathComponent];
    }
    return result;
}


- (AVAsset *) AVAsset
{
#define AVASSETQUERY_TIMEOUT    (NSEC_PER_SEC / 2)
    AVAsset __block *result = nil;
    
    dispatch_group_t dg = dispatch_group_create();
    dispatch_group_enter(dg);
    [[PHImageManager defaultManager] requestAVAssetForVideo:self options:nil resultHandler:^(AVAsset * _Nullable asset, AVAudioMix * _Nullable audioMix, NSDictionary * _Nullable info) {
        result = asset;
        [result retain];
        dispatch_group_leave(dg);
    }];
    dispatch_group_wait(dg, dispatch_time(DISPATCH_TIME_NOW, AVASSETQUERY_TIMEOUT));
    dispatch_release(dg);
    [result autorelease];
    return result;
}

- (AVURLAsset *) _avurlAsset
{
    AVAsset *avasset = [self AVAsset];
    if ( [avasset isKindOfClass:AVURLAsset.class] ) {
        return (AVURLAsset *) avasset;
    }
    return nil;
}

- (AVComposition *) _avComposition
{
    AVAsset *avasset = [self AVAsset];
    if ( [avasset isKindOfClass:AVComposition.class] ) {
        return (AVComposition *) avasset;
    }
    return nil;
}

@end

@implementation AVURLAsset(ClipSource)

- (NSString *) path
{
    NSString *result = nil;
    if ( self.URL.isFileURL ) {
        result = self.URL.path;
    }
    return result;
}

- (NSString *) shortDisplayText
{
    NSString *name = self.path.lastPathComponent;
    if (name == nil) {
        name = @"unknown";
    }
    return [NSString stringWithFormat:@"AVURLAsset:%@", name];
}
@end

@implementation MPMediaItem(ClipSource)

- (NSString *) path
{
    return self.assetURL.absoluteString;
}

- (NSString *) shortDisplayText
{
    NSString *name = self.path.lastPathComponent;
    if (name == nil) {
        name = @"unknown";
    }
    return [NSString stringWithFormat:@"MPMediaItem:%@", name];
}
@end

@implementation NXEPHAssetClipSource
- (instancetype) initWithClipSource:(id<NXEClipSource>) source
{
    self = [super initWithClipSource:source];
    if ( self ) {
    }
    return self;
}

- (void) dealloc
{
    self.avurlAsset = nil;
    self.avComposition = nil;
    [super dealloc];
}

- (AVURLAsset *) avurlAsset {
    if ( _avurlAsset == nil) {
        _avurlAsset = [[(PHAsset *)self.source _avurlAsset] retain];
    }
    return _avurlAsset;
}

- (AVComposition *) avComposition {
    if ( _avComposition == nil ) {
        _avComposition = [[(PHAsset *)self.source _avComposition] retain];
    }
    return _avComposition;
}

- (NSString *) path
{
    NSString *result = nil;
    PHAsset *phasset = (PHAsset *)self.source;
    if (phasset.mediaType == PHAssetMediaTypeVideo) {
        result = self.avurlAsset.path;
        if ( result == nil ) {
            result = self.avComposition.firstSegment.sourceURL.path;
        }
    } else {
        result = phasset.path;
    }
    return result;
}
@end
