/******************************************************************************
 * File Name   : KMLut.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMLut.h"
#import "ImageUtil.h"
#import "LUTMap.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"KMLut"

@interface KMLut ()

@property (nonatomic, retain) NXEFileLutSource *source;
@end

@implementation KMLut
{
    KMLutType lutType;
    int textureId;
    int textureId2;
}

- (id)initWithType:(KMLutType)type textureDisposeBag:(id<NXETextureDisposeBagProtocol>) textureDisposeBag
{
    if(self = [super init]) {
        self.source = [LUTMap.instance entryWith:type];
        if( self.source == nil) {
            [self release];
            return nil;
        }
        lutType = type;
        textureId = LAYER_GL_INVALID_TEXTURE_ID;
        textureId2 = LAYER_GL_INVALID_TEXTURE_ID;
        self.textureDisposeBag = textureDisposeBag;
    }
    
    return self;
}

- (void) dealloc
{
    self.source = nil;
    
    for(int i = 0; i < NUM_TEXTURE_CONTEXTS; i++) {
        GLuint texture = [self mainTextureForContextIndex:i];
        if (texture != LAYER_GL_INVALID_TEXTURE_ID) {
            NexLogW(LOG_TAG, @"context:%d Leaked textures:%d %p", i, texture, self);
        }
    }
    self.textureDisposeBag = nil;
    [super dealloc];
}

- (KMLutType)getType
{
    return lutType;
}

- (GLuint)getTextureId:(LayerGLContextIndex) contextIndex
{
    GLint texture = [self mainTextureForContextIndex:contextIndex];
    
    if(texture == LAYER_GL_INVALID_TEXTURE_ID) {
        texture = (GLint)[self.source createTexture];
        if (texture == GL_NONE ) {
            texture = LAYER_GL_INVALID_TEXTURE_ID;
        }
        [self setMainTexture:texture contextIndex:contextIndex];
    }
    
    return texture;
}

- (void)disposeTextures
{
    for(int i = 0; i < NUM_TEXTURE_CONTEXTS; i++) {
        [self disposeMainTextureForContextIndex:(LayerGLContextIndex) i];
    }
}

#pragma mark - Main Textures
- (GLuint) mainTextureForContextIndex:(LayerGLContextIndex) index
{
    return index == LayerGLContextIndexPreview ? textureId : textureId2;
}

- (void) setMainTexture:(GLuint) texture contextIndex:(LayerGLContextIndex) index
{
    NexLogD(LOG_TAG, @"context:%d texture:%d %p", index, texture, self);
    if (index == LayerGLContextIndexPreview) {
        textureId = texture;
    } else {
        textureId2 = texture;
    }
}

- (void) disposeMainTextureForContextIndex:(LayerGLContextIndex) index
{
    GLuint texture = [self mainTextureForContextIndex:index];
    if (texture != LAYER_GL_INVALID_TEXTURE_ID) {
        if (self.textureDisposeBag) {
            [self.textureDisposeBag disposeTexture:texture contextIndex:index];
        } else {
            NexLogW(LOG_TAG, @"Texture dispose bag not set");
        }
        [self setMainTexture:LAYER_GL_INVALID_TEXTURE_ID contextIndex:index];
    }
}
@end
