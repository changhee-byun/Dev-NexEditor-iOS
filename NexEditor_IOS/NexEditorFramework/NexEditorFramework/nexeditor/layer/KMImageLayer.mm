/******************************************************************************
 * File Name   : KMImageLayer.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMImageLayer.h"
#import "ImageUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"KMImageLayer"

@interface KMImageLayer ()

@property (nonatomic, retain) UIImage* image;
@property (nonatomic, retain) UIColor* solidColor;

@end

@implementation KMImageLayer {
    BOOL useColor;
//    UIImage* image;
//    UIColor* solidColor;
}

#pragma mark initialize method
- (id)initWithResourceName:(NSString *)name
{
    if(self = [super init]) {
        self.image = [UIImage imageNamed:name];
        useColor = NO;
        [self initSize];
    }
    
    return self;
}

- (id)initWithLocalPath:(NSString *)path
{
    if(self = [super init]) {
        self.image = [UIImage imageWithContentsOfFile:path];
        useColor = NO;
        [self initSize];
    }
    
    return self;
}

- (id)initWithImage:(UIImage *)img
{
    if(self = [super init]) {
        self.image = img;
        useColor = NO;
        [self initSize];
    }
    
    return self;
}

- (id)initWithColor:(UIColor*)color
{
    if(self = [super init]) {
        self.image = nil;
        self.solidColor = color;
        useColor = YES;
        [self initSize];
    }
    
    return self;
}

- (void) dealloc
{
    self.image = nil;
    [super dealloc];
}

- (void)initSize
{
    if(useColor) {
        if(!self.solidColor) return;
        
        self.width = 320;
        self.height = 180;
    }
    else {
        if(!self.image) return;
        
        //OpenGL 최대 텍스쳐 사이즈 넘어갈 경우 resizing 해야 함.
        self.image = [ImageUtil imageOfValidTextureWithImage:self.image];

        self.width = (int)CGImageGetWidth([self.image CGImage]);
        self.height = (int)CGImageGetHeight([self.image CGImage]);
    }
}

#pragma mark rendering
//- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer*) renderer
//{
//}

- (GLuint)getTextureId
{
    //
    GLint texture;
    
    int renderMode = [[NexLayer sharedInstance] getRenderMode];
    if(renderMode == 0)
        texture = textureId;
    else
        texture = textureId2;
    //
    if(useColor) {
        if(!self.solidColor) return -1;
        if(texture == -1) {
            CGFloat red, green, blue, alpha;
            [self.solidColor getRed:&red green:&green blue:&blue alpha:&alpha];
            texture = [ImageUtil glTextureIDfromColorRed:red Green:green Blue:blue Alpha:alpha];
        }
    } else {
        if(!self.image) return -1;
        if(texture == LAYER_GL_INVALID_TEXTURE_ID) {
            texture = [ImageUtil glTextureIDfromImage:self.image];
            NexLogD(LOG_TAG, @"context:%d texture:%d", renderMode, texture);
        }
    }
    //
    if(renderMode == 0)
        textureId = texture;
    else
        textureId2 = texture;
    //
    return texture;
}

@end
