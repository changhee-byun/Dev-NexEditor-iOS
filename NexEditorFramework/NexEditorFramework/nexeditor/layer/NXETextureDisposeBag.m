/*
 * File Name   : NXETextureDisposeBag.m
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

#import "NXETextureDisposeBag.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "ImageUtil.h"

#define LOG_TAG @"NXETextureDisposeBag"

@interface NXETextureDisposeBag()
@property (nonatomic) NSInteger refId;
// Contains textures that should be deleted when the associated GL contex becomes current
@property (nonatomic, strong) NSArray <NSMutableArray <NSNumber *> *> *disposedTextures;
@end

@implementation NXETextureDisposeBag
- (instancetype) initWithReferenceId:(NSInteger) refId
{
    self = [super init];
    if (self) {
        self.refId = refId;
        self.disposedTextures = @[ [NSMutableArray array], [NSMutableArray array]];
    }
    return self;
}

+ (instancetype) disposeBagWithReferenceId:(NSInteger)refId
{
    return [[[self.class alloc] initWithReferenceId:refId] autorelease];
}

- (void) dealloc
{
    int i = 0;
    for(NSArray *textures in self.disposedTextures) {
        if ( textures.count > 0)  {
            NexLogE(LOG_TAG, @"context:%d Leaked %d textures:%@",
                    i,
                    textures.count,
                    [textures.description stringByReplacingOccurrencesOfString:@"\n" withString:@" "]);
        }
        i++;
    }
    self.disposedTextures = nil;
    [super dealloc];
}

- (BOOL) hasTextures
{
    BOOL result = NO;
    for(int i = LayerGLContextIndexPreview; i < NUM_TEXTURE_CONTEXTS; i++) {
        if (self.disposedTextures[i].count > 0) {
            result = YES;
            break;
        }
    }
    return result;
}

- (void) disposeTexture:(GLuint) tex contextIndex:(LayerGLContextIndex) index
{
    @synchronized(self.disposedTextures) {
        [self.disposedTextures[index] addObject:@(tex)];
    }
}

- (void) deleteTextuersForContextIndex:(LayerGLContextIndex) index
{
    NSArray *textures = nil;
    @synchronized(self.disposedTextures) {
        textures = [[self.disposedTextures[index] copy] autorelease];
        [self.disposedTextures[index] removeAllObjects];
    }
    if ( textures.count > 0) {
        NexLogD(LOG_TAG, @"context:%d deleting %d textures:%@", index, textures.count, [textures.description stringByReplacingOccurrencesOfString:@"\n" withString:@" "]);
        
        for(NSNumber *number in textures) {
            [ImageUtil deleteTexture:(GLuint) number.unsignedIntegerValue];
        }
    }
}
@end
