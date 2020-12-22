/******************************************************************************
 * File Name   : KMEffectLayer.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMEffectLayer.h"
#import "ImageUtil.h"
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "LayerManager.h"

@implementation KMEffectLayer {
    KMEffectType effectType;
}

- (id)initWithType:(KMEffectType) type
{
    if(self = [super init]) {
        effectType = type;
    }
    return self;
}

#pragma mark rendering
- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer*) renderer
{
    if(effectType < KM_EFFECT_BLUR || effectType >= KM_EFFECT_MAX) return;
    
    [renderer save];
    
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glColorMask(false, false, false, false);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    
    //renderer.fillRect(Color.WHITE, left, top, right, bottom);
    [renderer save];
    int texture = [ImageUtil glTextureIDfromColorRed:1.0 Green:1.0 Blue:1.0 Alpha:1.0];
    int w = 300, h = 300;
    int l = -w/2;
    int t = -h/2;
    int r = l + w;
    int b = t + h;
    [renderer drawBitmap:texture left:l top:t right:r bottom:b];
    [ImageUtil deleteTexture:texture];
    [renderer restore];

    glColorMask(true, true, true, true);
    glStencilFunc(GL_EQUAL, 1, 0xFF);     // Pass test if stencil value is 1
    glStencilMask(0x00);                  // Don't write anything to stencil buffer
    
    [renderer resetMatrix];
    [renderer setEffectTextureSize:[renderer getScreenDimensionWidth] height:[renderer getScreenDimensionWidth]];
    
    if(effectType == KM_EFFECT_BLUR) {
        [renderer setHBlurEnabled:YES];
        [renderer setVBlurEnabled:NO];
        [renderer setEffectStrength:3];
    }
    else {
        [renderer setHBlurEnabled:NO];
    }
    
    if(effectType == KM_EFFECT_MOSAIC) {
        [renderer setMosaicEnabled:YES];
        [renderer setEffectStrength:30];
    }
    else{
        [renderer setMosaicEnabled:NO];
    }
    
    [self gettextureId:renderer];
    
    //
    GLint texture_;
    
    if([renderer getRenderMode] == 0)
        texture_ = textureId;
    else
        texture_ = textureId2;
    
    CGSize size = CGSizeMake([renderer getOutputWidth], [renderer getOutputHeight]);
    
    [renderer drawDirect:texture_ x:640 y:360 w:size.width h:size.height];
    
    if(effectType == KM_EFFECT_BLUR) {
        [renderer setHBlurEnabled:NO];
        [renderer setVBlurEnabled:YES];
        [self gettextureId:renderer];
        [renderer drawDirect:texture_ x:640 y:360 w:size.width h:size.height];
    }
    glDisable(GL_STENCIL_TEST);
    
    [renderer restore];
    [renderer setVBlurEnabled:NO];
    [renderer setHBlurEnabled:NO];
    [renderer setMosaicEnabled:NO];
}

/** 레이어의 텍스쳐 id 값을 가져온다.
 @return OpenGL texture의 id 값
 */
- (GLuint)getTextureId
{
    int renderMode = [[NexLayer sharedInstance] getRenderMode];
    if(renderMode == 0)
        return textureId;
    else
        return textureId2;
}

- (GLuint)gettextureId:(NexLayer*)renderer
{
    if(effectType < KM_EFFECT_BLUR || effectType >= KM_EFFECT_MAX) return -1;
    //
    GLint texture;
    
    int renderMode = [renderer getRenderMode];
    if(renderMode == 0)
        texture = textureId;
    else
        texture = textureId2;
    //
    if(texture == -1) {
        self.width = (int)[renderer getScreenDimensionWidth];
        self.height = (int)[renderer getScreenDimensionHeight];
        
        GLuint texture_;
        glGenTextures(1, &texture_);
        texture = texture_;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self.width, self.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self.x, self.y, self.width, self.height);
    //
    if(renderMode == 0)
        textureId = texture;
    else
        textureId2 = texture;
    //
    return texture;
}
@end
