//
//  NexThemeRenderer_ios_private.h
//  ThemeTest
//
//  Created by YooSunghyun on 3/3/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#ifndef NexThemeRenderer_Internal_ios_h
#define NexThemeRenderer_Internal_ios_h

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <videoToolBox/VTDecompressionSession.h>
#import "NexThemeRenderer_Platform_iOS.h"

@class EAGLContext;
typedef EAGLContext * EAGLContextPtr;
@class CAEAGLLayer;
typedef CAEAGLLayer * CAEAGLLayerPtr;
@class NSMutableArray;
typedef NSMutableArray * NSMutableArrayPtr;

#define MAX_PIXELBUFF_FOR_ENCODER 4
typedef struct _PixelBufferTextureInfo_ {
    CVPixelBufferRef pixelBuffer;
    CVOpenGLESTextureRef cvTexture;
} PixelBufferTextureInfo;

typedef struct _ThemeRenderer_Platform_Context_iOS {
    EAGLContextPtr eaglContext;
    CAEAGLLayerPtr eaglLayer;
    CVOpenGLESTextureCacheRef videoTextureCache;
    GLuint colorRenderBuffer;
    GLuint depthStencilRenderBuffer;
    GLuint frameBuffer;
    
    PixelBufferTextureInfo pixelBufferTextureInfo;
    
    bool isRenderBufferSet;
    NSMutableArrayPtr imageBufferQueue;
    
    id<NTRPixelBufferProviding> pixelBufferProvider;
    void* stateMonitor;
} ThemeRenderer_Platform_Context_iOS;

#endif /* NexThemeRenderer_Internal_ios_h */
