//
//  NexThemeRenderer_ios.m
//  NexThemeRenderer
//
//  Created by YooSunghyun on 2/11/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <videoToolBox/VTDecompressionSession.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "NexThemeRenderer_Internal.h"
#include "NexThemeRenderer_ios.h"
#include "NexThemeRenderer_Platform_iOS.h"
#include "NexThemeRenderer_ios_private.h"

#include "NexSAL_Internal.h"
#import "NexThemeRenderer_ios.h"
#import "NTRPixelBufferProvider.h"

/// Returns ios context from the renderer
#define RENDERER_IOSCTX(renderer)   ((ThemeRenderer_Platform_Context_iOS *)renderer->platCtx)

/// Define these macros to designated malloc/free functions
#define __RENDERER_MALLOC(size)     malloc(size)
#define __RENDERER_FREE(p)          free(p)

typedef struct NEXTHEMERENDERER_TARGET_ADDITIONAL_INFO {
    int scale;
    int dummy;
} NEXTHEMERENDERER_TARGET_ADDITIONAL_INFO;

@interface NexThemeRendererEAGLLayer: CAEAGLLayer
@end
@implementation NexThemeRendererEAGLLayer
@end

@interface AppStateMonitor: NSObject
@property (nonatomic) BOOL isActive;
@end

@interface NXERenderResouces : NSObject
//@property(nonatomic,assign) unsigned int pts;
@property(nonatomic,retain) id<NexThemeRendererImageBuffer> imageBuff;
@property(nonatomic,assign) CVOpenGLESTextureRef ytex;
@property(nonatomic,assign) CVOpenGLESTextureRef uvtex;
@end

@implementation NXERenderResouces

/*
-(oneway void)release
{
    NSLog(@"NXERenderResouces's release (%p)",self);
}
 */
- (void) dealloc
{
    self.imageBuff = nil;
    [super dealloc];
}
@end

static NXT_Error setupLayer( NXT_HThemeRenderer renderer )
{
    //_eaglLayer = (CAEAGLLayer*) self.layer;
    //CAEAGLLayer* _eaglLayer;

    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    ctx->eaglLayer = [[NexThemeRendererEAGLLayer alloc] init];
    //NSLog(@"layer.rc=%lu",(unsigned long)ctx->eaglLayer.retainCount);
    ctx->eaglLayer.opaque = YES;


    ctx->eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                nil];
    //NSLog(@"layer.rc=%lu",ctx->eaglLayer.retainCount);
  
    
    
    //_eaglLayer.frame = CGRectMake(0,0,300,300); //self.bounds;
    //_eaglLayer.frame = self.bounds;//self.layer.frame;
    //[self.layer addSublayer:_eaglLayer];

    return NXT_Error_None;
}

static NXT_Error setupContext( NXT_HThemeRenderer renderer, NXT_HThemeRenderer hAnotherRendererForShareGroup )
{
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    
    //static EAGLSharegroup* sg = nil;
    if (hAnotherRendererForShareGroup == NULL) {
        ctx->eaglContext = [[EAGLContext alloc] initWithAPI:api];
    } else {
        ctx->eaglContext = [[EAGLContext alloc] initWithAPI:api sharegroup:[RENDERER_IOSCTX(hAnotherRendererForShareGroup)->eaglContext sharegroup]];
    }
    
    //ctx->eaglContext = [[EAGLContext alloc] initWithAPI:api];
    if (!ctx->eaglContext) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed to initialize OpenGLES 3.0 context");
        return NXT_Error_GLContextErr;
    }
    if (![EAGLContext setCurrentContext:ctx->eaglContext]) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed to set current OpenGL context");
        return NXT_Error_GLContextErr;
    }
    
    //NSLog(@"c.rc=%lu",ctx->eaglContext.retainCount);

    return NXT_Error_None;
}

static NXT_Error setupTextureCache( NXT_HThemeRenderer renderer )
{
    //NSLog(@"c.rc=%lu",RENDERER_IOSCTX(renderer)->eaglContext.retainCount);
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, RENDERER_IOSCTX(renderer)->eaglContext, NULL, &RENDERER_IOSCTX(renderer)->videoTextureCache);
    //NSLog(@"c.rc=%lu",RENDERER_IOSCTX(renderer)->eaglContext.retainCount);

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "CVOpenGLESTextureCacheCreate's return value : %d",err);
    return NXT_Error_None;
}


#pragma mark -
void NXT_ThemeRenderer_SetDefaultFrameBuffer(NXT_HThemeRenderer renderer,unsigned int frameBufferNum);

NXT_Error NXT_Platform_CreateInternalContext(NXT_HThemeRenderer renderer, NXT_HThemeRenderer hAnotherRendererForShareGroup)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: createInternalContext enter. type(%d)", renderer->rendererType);
    NXT_Error ret = NXT_Error_None;
    
    renderer->platCtx = __RENDERER_MALLOC(sizeof(ThemeRenderer_Platform_Context_iOS));
    memset(renderer->platCtx, 0x00, sizeof(ThemeRenderer_Platform_Context_iOS));

    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    ctx->pixelBufferProvider = [[NTRPixelBufferProvider alloc] init];
    
    ret = setupLayer( renderer );
    ret = setupContext( renderer, hAnotherRendererForShareGroup);
    ret = setupTextureCache( renderer );
    
    
    glGenFramebuffers( 1, &ctx->frameBuffer );
    glGenRenderbuffers( 1, &ctx->depthStencilRenderBuffer );

    if ( NXT_RendererType_InternalPreviewContext == renderer->rendererType ) {
        glGenRenderbuffers( 1, &ctx->colorRenderBuffer );
    }

    NXT_ThemeRenderer_SetDefaultFrameBuffer( renderer, ctx->frameBuffer );
    
    ctx->imageBufferQueue = [[NSMutableArray alloc] init];
    //[ctx->imageBufferQueue release];
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: createInternalContext end");
    
    ctx->stateMonitor = (void *) [[AppStateMonitor alloc] init];
    return ret;
}


// The following texPool_* functions will be modified when revision job will be merged.
static void* texPool_Get(NXT_HThemeRenderer renderer)
{
    return NULL;
}

static void texPool_Put(NXT_HThemeRenderer renderer,void *a)
{
    
}

void OnPixelBufferSentToEncoder(void*);

static CVReturn createTextureForPixelBuffer( CVOpenGLESTextureCacheRef videoTextureCache, CVPixelBufferRef pixelBuffer, CVOpenGLESTextureRef *texref)
{
    GLsizei width = (GLsizei) CVPixelBufferGetWidth(pixelBuffer);
    GLsizei height = (GLsizei) CVPixelBufferGetHeight(pixelBuffer);
    
    return CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                 videoTextureCache,
                                                 pixelBuffer,
                                                 NULL, // texture attributes
                                                 GL_TEXTURE_2D,
                                                 GL_RGBA, // opengl format
                                                 width,
                                                 height,
                                                 //GL_RGBA,
                                                 GL_BGRA, // native iOS format
                                                 GL_UNSIGNED_BYTE,
                                                 0,
                                                 texref);
    
}

static void _clearPixelBuffer( NXT_HThemeRenderer renderer, id<NTRPixelBufferProviding> pixelBufferSupply )
{
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    
    PixelBufferTextureInfo *info = &ctx->pixelBufferTextureInfo;
    
    if ( info->cvTexture ) {
        CFRelease(info->cvTexture);
        info->cvTexture = NULL;
    }
    
    if ( info->pixelBuffer ) {
        [pixelBufferSupply disposePixelBuffer:info->pixelBuffer];
        info->pixelBuffer = NULL;
    }
}

/// NULL value not allowed for pixelBuffer
static NXT_Error _setPixelBuffer( NXT_HThemeRenderer renderer, CVPixelBufferRef pixelBuffer )
{
    NXT_Error result = NXT_Error_Internal;
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    
    if (EAGLContext.currentContext != ctx->eaglContext) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Current EAGLContext %p is not for us %p ", EAGLContext.currentContext, ctx->eaglContext);
        return NXT_Error_GLContextErr;
    }

    CVOpenGLESTextureRef cvTexture;
    CVReturn ret = createTextureForPixelBuffer(ctx->videoTextureCache, pixelBuffer, &cvTexture);
    if (ret == kCVReturnSuccess ) {
        
        GLenum tex_target = CVOpenGLESTextureGetTarget(cvTexture);
        GLuint tex_name = CVOpenGLESTextureGetName(cvTexture);
        glBindFramebuffer(GL_FRAMEBUFFER, ctx->frameBuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_target,tex_name);
        glTexParameterf(tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_target, tex_name, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
        if(status == GL_FRAMEBUFFER_COMPLETE) {
            ctx->pixelBufferTextureInfo.pixelBuffer = pixelBuffer;
            ctx->pixelBufferTextureInfo.cvTexture = cvTexture;
            result = NXT_Error_None;
        } else {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed to make complete framebuffer object %x", status);
            result = NXT_Error_GLContextErr;
            CFRelease(cvTexture);
        }
        
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed creating texture with a new pixelBuffer, CVReturn:0x%08x", ret);
    }
    return result;
}

static NXT_Error setRenderTargetInformation( NXT_HThemeRenderer renderer, int x, int y, int reqWidth, int reqHeight, NEXTHEMERENDERER_TARGET_ADDITIONAL_INFO* pInfo )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: setRenderTargetInformation enter (type,x,y,w,h,s)=(%d,%d,%d,%d,%d,%d)",renderer->rendererType,x,y,reqWidth,reqHeight,pInfo?pInfo->scale:0);

    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    NXT_Error result = NXT_Error_None;
    if ( renderer->rendererType == NXT_RendererType_InternalExportContext ) {
        if ( reqWidth > 0 && reqHeight > 0 ) {
            if (![EAGLContext setCurrentContext:ctx->eaglContext]) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed to set current OpenGL context");
                return NXT_Error_GLContextErr;
            }
            
            glBindFramebuffer(GL_FRAMEBUFFER, ctx->frameBuffer);
            
            glBindRenderbuffer(GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, reqWidth, reqHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);

            id<NTRPixelBufferProviding> pixelBufferSupply = ctx->pixelBufferProvider;
            pixelBufferSupply.requiredSize = CGSizeMake(reqWidth, reqHeight);
            
            CVPixelBufferRef pixelBuffer = [pixelBufferSupply newPixelBuffer];
            _clearPixelBuffer(renderer, ctx->pixelBufferProvider);
            result = _setPixelBuffer(renderer, pixelBuffer);
            if (result != NXT_Error_None ) {
                [pixelBufferSupply disposePixelBuffer:pixelBuffer];
            }
        }
    }
    else if (renderer->rendererType == NXT_RendererType_InternalPreviewContext) {
        if ( !pInfo )
            return NXT_Error_MissingParam;
        
        int pxWidth = reqWidth*pInfo->scale;
        int pxHeight = reqHeight*pInfo->scale;
        CGRect layerFrame = CGRectMake(x, y, reqWidth, reqHeight);
        result = NXT_Error_None;

        // reconfigure GL layer only if layout changed (frame and scale)
        if ( ( pxWidth > 0 && pxHeight > 0 )
            && (!CGRectEqualToRect(ctx->eaglLayer.frame, layerFrame) ||
                (ctx->eaglLayer.contentsScale != pInfo->scale )) ) {
                
            if (![EAGLContext setCurrentContext:ctx->eaglContext]) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Failed to set current OpenGL context");
                return NXT_Error_GLContextErr;
            }

            ctx->eaglLayer.frame = layerFrame;
            ctx->eaglLayer.contentsScale = (CGFloat)pInfo->scale;

            glBindRenderbuffer(GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, pxWidth, pxHeight);
            
            glBindRenderbuffer(GL_RENDERBUFFER, ctx->colorRenderBuffer);
            [ctx->eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:ctx->eaglLayer];
            //NSLog(@"layer.rc=%lu",ctx->eaglLayer.retainCount);

            glBindFramebuffer(GL_FRAMEBUFFER, ctx->frameBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ctx->colorRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ctx->depthStencilRenderBuffer);
            
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
            if(status != GL_FRAMEBUFFER_COMPLETE) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "failed to make complete framebuffer object %x", status);
                result = NXT_Error_GLContextErr;
            } else {
                ctx->isRenderBufferSet = true;
            }
        }
    } else {
        result = NXT_Error_WrongRendererType;
    }

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: setRenderTargetInformation end");
    return result;
}

// ToDo:width/height are not needed.
static NXT_Error getPixelBuffer(NXT_HThemeRenderer renderer, void **ppData, int *pSize )
{
    NXT_Error return_value = NXT_Error_WrongRendererType;
    
    if ( renderer->rendererType == NXT_RendererType_InternalExportContext ) {
        {
            CVPixelBufferRef pixelBuffer = RENDERER_IOSCTX(renderer)->pixelBufferTextureInfo.pixelBuffer;
            // The receiving side, e.g. encoder, should release when no longer necessary
            CVPixelBufferRetain(pixelBuffer);
            *ppData = (void *) pixelBuffer;
            *pSize = sizeof(pixelBuffer);
            return_value = NXT_ThemeRenderer_SwitchToNextOutputPixelBuffer(renderer);
        }
        
        glFinish();
    }
    
    return return_value;
}

static NXT_Error getPixelBufferFormat(NXT_HThemeRenderer renderer,NXT_PixelFormat *pFormat)
{
    if ( renderer->rendererType == NXT_RendererType_InternalExportContext ) {
        *pFormat = NXT_PixelFormat_Opaque;
    }
    
    return NXT_Error_None;
}

/*
NXT_Error reconfigRenderBuffers( NXT_HThemeRenderer renderer, int x, int y, int w, int h, int scale )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: reconfigRenderBuffers enter (x,y,w,h,s)=(%d,%d,%d,%d,%d)",x,y,w,h,scale);
    
    int pxWidth = w*scale;
    int pxHeight = h*scale;
    
    if ( renderer->view_width != pxWidth || renderer->view_height != pxHeight ) {
        if ( pxWidth > 0 && pxHeight > 0 ) {
            
            if (![EAGLContext setCurrentContext:renderer->ios.eaglContext]) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Failed to set current OpenGL context");
                return NXT_Error_GLContextErr;
            }
            renderer->ios.eaglLayer.frame = CGRectMake(x, y, w, h);
            renderer->ios.eaglLayer.bounds = CGRectMake(x, y, w, h);
            renderer->ios.eaglLayer.contentsScale = (CGFloat)scale;
            
            glBindRenderbuffer(GL_RENDERBUFFER, renderer->ios.depthStencilRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, pxWidth, pxHeight);


            glBindRenderbuffer(GL_RENDERBUFFER, renderer->ios.colorRenderBuffer);
            [renderer->ios.eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:renderer->ios.eaglLayer];

            glBindFramebuffer(GL_FRAMEBUFFER, renderer->ios.frameBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderer->ios.colorRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderer->ios.depthStencilRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderer->ios.depthStencilRenderBuffer);
            
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
            if(status != GL_FRAMEBUFFER_COMPLETE) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "failed to make complete framebuffer object %x", status);
                return NXT_Error_GLContextErr;
            }
            //GL_FRAMEBUFFER_UNSUPPORTED
            
            renderer->ios.isRenderBufferSet = true;
        }
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: reconfigRenderBuffers end");
    
    return NXT_Error_None;
}
*/

NXT_Error NXT_Platform_DeleteInternalContext( NXT_HThemeRenderer renderer )
{
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: deleteInternalContext enter");
    //setCurrentInternalContext( renderer, SET_CONTEXT_MODE_PREVIEW);
    [EAGLContext setCurrentContext:ctx->eaglContext];
    
    //ToDO:delete texturecache.
    
    _clearPixelBuffer(renderer, ctx->pixelBufferProvider);
    
    if ( ctx->eaglLayer != nil ) {
        glBindRenderbuffer(GL_RENDERBUFFER, ctx->colorRenderBuffer);
        [ctx->eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:nil];
        //NSLog(@"layer.rc=%lu",ctx->eaglLayer.retainCount);
        //ctx->eaglLayer.drawableProperties = nil;
        //NSLog(@"layer.rc=%lu",ctx->eaglLayer.retainCount);
        [ctx->eaglLayer release];
        ctx->eaglLayer = nil;
    }
    NXT_Platform_AfterDraw(renderer);
    //NSLog(@"ibq.rc=%lu",ctx->imageBufferQueue.retainCount);
    [ctx->imageBufferQueue release];
    ctx->imageBufferQueue = nil;
    
    AppStateMonitor *monitor = (AppStateMonitor *) ctx->stateMonitor;
    [monitor release];
    ctx->stateMonitor = NULL;
    
    if ( NXT_RendererType_InternalPreviewContext == renderer->rendererType ) {
        glDeleteRenderbuffers( 1, &ctx->colorRenderBuffer );
        ctx->colorRenderBuffer = 0;
    }

    glDeleteRenderbuffers( 1, &ctx->depthStencilRenderBuffer );
    ctx->depthStencilRenderBuffer = 0;
    
    glDeleteFramebuffers( 1, &ctx->frameBuffer );
    ctx->frameBuffer = 0;

    if ( ctx->videoTextureCache ) {
        CFRelease(ctx->videoTextureCache);
        ctx->videoTextureCache = NULL;
    }
    
    [EAGLContext setCurrentContext:nil];
    if ( ctx->eaglContext ) {
        //NSLog(@"c.rc=%lu",ctx->eaglContext.retainCount);
        
        [ctx->eaglContext release];
        //NSLog(@"c.rc=%lu",ctx->eaglContext.retainCount);
        ctx->eaglContext = nil;
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "Theme Porting: deleteInternalContext end");

    [ctx->pixelBufferProvider release];
    ctx->pixelBufferProvider = nil;
    
    __RENDERER_FREE(renderer->platCtx);
    renderer->platCtx = NULL;
    return NXT_Error_None;
}

static NXT_Error checkInternalContextAvailable()
{
    EAGLContext* cc = [EAGLContext currentContext];
    if( cc == nil ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NO EGL CONTEXT - Upload texture failed", __func__, __LINE__ );
        return NXT_Error_NoContext;
    }
    cc = nil;
    return NXT_Error_None;
}
/// @return NXT_Error_GLContextErr if failed, NXT_Error_BadParam if wrong argument, NXT_Error_None otherwise.
NXT_Error NXT_Platform_SetCurrentInternalContext( NXT_HThemeRenderer renderer, NEXTHEMERENDERER_SET_CONEXTEXT_MODE mode )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 3, "Theme Porting: setCurrentInternalContext enter");
    
    NXT_Error ret = NXT_Error_None;
    
    if ( mode == SET_CONTEXT_MODE_EXPORT || mode == SET_CONTEXT_MODE_PREVIEW ) {
        ret = ( [EAGLContext setCurrentContext:RENDERER_IOSCTX(renderer)->eaglContext] ) ? NXT_Error_None : NXT_Error_GLContextErr;
    } else if ( mode == SET_CONTEXT_MODE_RELEASE ) {
        ret = ( [EAGLContext setCurrentContext:nil] ) ? NXT_Error_None : NXT_Error_GLContextErr;
    } else {
        ret = NXT_Error_BadParam;
    }

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 3, "Theme Porting: setCurrentInternalContext end");
    
    return ret;
}

NXT_Error NXT_Platform_PresentBuffer( NXT_HThemeRenderer renderer, NEXTHEMERENDERER_PRESENT_MODE mode )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 3, "Theme Porting: presentBuffer enter");
    NXT_Error ret = NXT_Error_None;
    
    //glClearColor(0.0f, 0.0f, 0.9f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if ( mode == PRESENT_MODE_PREVIEW ) {
        ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
        // NESI-551 fastOptionPreview in background mode causes a crash at this point
        AppStateMonitor *monitor = (AppStateMonitor *) ctx->stateMonitor;
        if ([monitor isActive]) {
            glBindRenderbuffer(GL_RENDERBUFFER, ctx->colorRenderBuffer);
            ret = ( [ctx->eaglContext presentRenderbuffer:GL_RENDERBUFFER] ) ? NXT_Error_None : NXT_Error_OpenGLFailure;
        } else {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "Skiping presentRenderbuffer: due to inactive application state");
            ret = NXT_Error_InvalidState;
        }
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 3, "Theme Porting: presentBuffer end");
    return ret;
}


void NXT_Platform_AfterDraw(NXT_HThemeRenderer renderer)
{
    BOOL bDone = FALSE;
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    if ( ctx->imageBufferQueue.count > 0 ) {
        // NESI-516
        glFlush();
    }
    while ( ctx->imageBufferQueue.count && bDone == FALSE ) {
    
        NXERenderResouces * output = [ctx->imageBufferQueue objectAtIndex:0];

        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 1, "imageBuffer delete %p", output.imageBuff);
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 1, "imageBuffer tex delete %p, %p", output.ytex, output.uvtex );
        
        
        if ( output.ytex )
            CFRelease(output.ytex);
        
        if ( output.uvtex )
            CFRelease(output.uvtex);

        if ( output.imageBuff )
            [output.imageBuff releaseResource];
        
        output.imageBuff = nil;
        //[output release];
        output = nil;
        
        [ctx->imageBufferQueue removeObjectAtIndex:0];
    }
}


static NXT_Error uploadTextureForHWDecoder(NXT_HThemeRenderer renderer,
                                    NXT_TextureInfo *tex,
                                    unsigned char* data0,
                                    unsigned char* data1,
                                    unsigned char* data2,
                                    void** ppInfoFree)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 1, "Theme Porting: uploadTextureForHWDecoder enter");
    NXT_Error ret = NXT_Error_None;


    if ( !data0 ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] CVImageBufferRef is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    CVOpenGLESTextureCacheFlush(RENDERER_IOSCTX(renderer)->videoTextureCache, 0);

    //NXT_TextureInfo *tex = &renderer->videoTexture[texture_id];

    id<NexThemeRendererImageBuffer> doib = (id<NexThemeRendererImageBuffer>)data0;
    CVImageBufferRef imageBuffer = doib.image;
    
    //nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Debug [%s,%d] output(%p)'s pts=%d, retainCount=%lu", __func__,__LINE__, doib, doib.pts, (unsigned long)doib.retainCount);

    int frameWidth = (int)CVPixelBufferGetWidth(imageBuffer);
    int frameHeight = (int)CVPixelBufferGetHeight(imageBuffer);
    CVReturn cvRet;
    CVOpenGLESTextureRef yTex = NULL, uvTex = NULL;

    cvRet = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       RENDERER_IOSCTX(renderer)->videoTextureCache,
                                                       imageBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RED_EXT,
                                                       frameWidth,
                                                       frameHeight,
                                                       GL_RED_EXT,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &yTex);
    
    if ( cvRet == kCVReturnSuccess ) {
        // UV-plane.
        cvRet = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           RENDERER_IOSCTX(renderer)->videoTextureCache,
                                                           imageBuffer,
                                                           NULL,
                                                           GL_TEXTURE_2D,
                                                           GL_RG_EXT,
                                                           frameWidth / 2,
                                                           frameHeight / 2,
                                                           GL_RG_EXT,
                                                           GL_UNSIGNED_BYTE,
                                                           1,
                                                           &uvTex);
    }
    
    if ( cvRet != kCVReturnSuccess ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "CV Texture Creation failed. ret:%x", cvRet);
        ret = NXT_Error_BadParam;
        if (yTex != NULL) {
            CFRelease(yTex);
        }
        return ret;
    }

    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(CVOpenGLESTextureGetTarget(yTex), CVOpenGLESTextureGetName(yTex));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    tex->texName[0] = CVOpenGLESTextureGetName(yTex);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(CVOpenGLESTextureGetTarget(uvTex), CVOpenGLESTextureGetName(uvTex));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    tex->texName[1] = CVOpenGLESTextureGetName(uvTex);

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 3, "Theme Porting: texName (%d,%d),(%d,%d) - pts:%d",CVOpenGLESTextureGetTarget(yTex),tex->texName[0],CVOpenGLESTextureGetTarget(uvTex),tex->texName[1],doib.pts);
    //CFRelease(yTex);
    //CFRelease(uvTex);
    //CFRelease(imageBuffer);
    //CFRelease(imageBuffer);
    
    //unsigned int tw = NXT_NextHighestPowerOfTwo(frameWidth);
    //unsigned int th = NXT_NextHighestPowerOfTwo(frameHeight);
    unsigned int tw = frameWidth;//NXT_NextHighestPowerOfTwo(frameWidth);
    unsigned int th = frameHeight;//NXT_NextHighestPowerOfTwo(frameHeight);
    
    //th = tw;
    
    tex->texNameInitCount = 2;
    tex->textureFormat = NXT_PixelFormat_NV12;

    tex->left = 0;
    tex->bottom = 0;
    tex->right = frameWidth-1;
    tex->top = frameHeight-1;

    tex->textureWidth = tw;
    tex->textureHeight = th;
    tex->textureUVWidth = tw/2;
    tex->textureUVHeight = th/2;
    
    tex->srcWidth = frameWidth;
    tex->srcHeight = frameHeight;
    tex->srcUVWidth = frameWidth/2;
    tex->srcUVHeight = frameHeight/2;
    tex->bValidTexture = 1;
    tex->bTextureSetByPlatform = 1;
    
    NXERenderResouces * res = [[NXERenderResouces alloc] init];
    res.imageBuff = doib;
    res.ytex = yTex;
    res.uvtex = uvTex;

    *ppInfoFree = res;
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 1, "Theme Porting: uploadTextureForHWDecoder end");
    return ret;
}

static NXT_Error freeUploadTexture4HWDecoder(NXT_HThemeRenderer renderer,void*pInfo, bool now)
{
    NXERenderResouces * res = (NXERenderResouces*)pInfo;
    if ( now ) {
        // NESI-516
        glFlush();
        CFRelease(res.ytex);
        CFRelease(res.uvtex);
        //CFRelease(imageBuffer);
        
        if ( res.imageBuff ) {
            [res.imageBuff releaseResource];
        }

        [res release];
        res = nil;
    }
    else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 2, "Debug [%s,%d] output(%p)'s retainCount=%lu", __func__,__LINE__, res.imageBuff, (unsigned long)res.imageBuff.retainCount);
        
        [RENDERER_IOSCTX(renderer)->imageBufferQueue addObject:res];
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 2, "Debug [%s,%d] output(%p)'s retainCount=%lu", __func__,__LINE__, res.imageBuff, (unsigned long)res.imageBuff.retainCount);
        
        [res release];

    }
    return NXT_Error_None;
}

EAGLContext *NXT_ThemeRenderer_GetEAGLContext(NXT_HThemeRenderer renderer)
{
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    
    return ctx->eaglContext;
}

void NXT_ThemeRenderer_SetExternalPixelBufferSupply(NXT_HThemeRenderer renderer, id<NTRPixelBufferProviding> pixelBufferSupply)
{
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    if (ctx->pixelBufferProvider) {
        [ctx->pixelBufferProvider release];
    }
    if (pixelBufferSupply == NULL) {
        // Return to default provider
        pixelBufferSupply = [[NTRPixelBufferProvider alloc] init];
    } else {
        [pixelBufferSupply retain];
    }
    ctx->pixelBufferProvider = pixelBufferSupply;
}

NXT_Error NXT_ThemeRenderer_SwitchToNextOutputPixelBuffer(NXT_HThemeRenderer renderer)
{
    NXT_Error result = NXT_Error_None;
    ThemeRenderer_Platform_Context_iOS *ctx = RENDERER_IOSCTX(renderer);
    id<NTRPixelBufferProviding> pixelBufferSupply = ctx->pixelBufferProvider;
    
    CVPixelBufferRef pixelBuffer = [pixelBufferSupply newPixelBuffer];
    if (pixelBuffer != NULL) {
        _clearPixelBuffer(renderer, pixelBufferSupply);
        NXT_Error ret = _setPixelBuffer(renderer, pixelBuffer);
        if (ret != NXT_Error_None ) {
            [pixelBufferSupply disposePixelBuffer:pixelBuffer];
            // Oops! this will crash.
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed configuring a new render buffer", __func__,__LINE__);
            result = ret;
        }
    } else {
        result = NXT_Error_Malloc;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Failed allocating a new render buffer", __func__,__LINE__);
    }
    
    return result;
    
}
#pragma mark - Migrated from NexThemeRenderer.cpp
NXT_Error NXT_ThemeRenderer_SetRenderTargetInformation( NXT_HThemeRenderer renderer, int reqWidth, int reqHeight)
{
    // in android, just same as NXT_ThemeRenderer_SetNativeWindow

    NXT_Error result = NXT_ThemeRenderer_AquireContext(renderer);
    if( result == NXT_Error_None ) {
        float scale = 1.0;
        
        NXT_Platform_RenderTargetInfo info = { 0, 0, { (uint32_t)reqWidth, (uint32_t) reqHeight}, scale };
        result = NXT_Platform_SetRenderTargetInfo(renderer, info);
        
        if (result == NXT_Error_None) {
            glViewport( 0, 0, reqWidth, reqHeight );
            
            renderer->view_width = reqWidth;
            renderer->view_height = reqHeight;
        }
        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    }

    return result;
}

void NXT_ThemeRenderer_GetRendererViewSize(NXT_HThemeRenderer renderer, unsigned int *width, unsigned int *height)
{
    *width = renderer->view_width;
    *height = renderer->view_height;
}

NXT_Error NXT_ThemeRenderer_GetPixelBuffer(NXT_HThemeRenderer renderer,
                                           void **ppData,
                                           int *pSize)
{
    return getPixelBuffer( renderer, ppData, pSize );
}

NXT_Error NXT_ThemeRenderer_ReturnPixelBufferForReuse(NXT_HThemeRenderer renderer, void *pPixelBuffer)
{
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) pPixelBuffer;
    CVPixelBufferRelease(pixelBuffer);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_UploadTextureForHWDecoder(NXT_HThemeRenderer renderer,
                                                      NXT_TextureID texture_id,
                                                      unsigned char* data0,
                                                      unsigned char* data1,
                                                      unsigned char* data2,
                                                      int convert_to_rgb_flag,
                                                      int lut_id)
{
    if ( !renderer ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if ( !renderer->bInitialized ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Renderer is not initialized. error. Eric want to know when this log can appear.", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if ( texture_id >= NXT_TextureID_COUNT || texture_id < 0 /* NESI-535 */ ) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    if( renderer->bIsInternalContext ) {
        NXT_Error ret;
        if ( NXT_Error_None != ( ret = checkInternalContextAvailable() ) ) {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NO GL CONTEXT - Upload texture failed", __func__, __LINE__ );
            return ret;
        }
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    NXT_ThemeRenderer_ResetTexture(renderer, texture_id,0,0); // width and height info will be set in uploadTextureForHWDecoder
    
    void* pInfo4Free = NULL;
    NXT_Error ret = uploadTextureForHWDecoder(renderer,tex,data0,data1,data2,&pInfo4Free);
    if ( NXT_Error_None != ret )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_Internal;
    }
    
    bool bConverted = false;
    if ( convert_to_rgb_flag || lut_id ) {
        int tex_id_for_lut = 0;
        if (lut_id != 0) {
            tex_id_for_lut = NXT_ThemeRenderer_GetLUTTexture(renderer, lut_id);
            if ( tex_id_for_lut == 0 ) {
                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, lut_id);
                if ( tex_id != 0 ) {
                    tex_id_for_lut = NXT_ThemeRenderer_SetLUTTexture(renderer, lut_id, tex_id);
                }
            }
        }
        NXT_ThemeRenderer_ConvertTextureToRGB(renderer, &renderer->getVideoSrc(texture_id), 0, 0, 0, tex_id_for_lut, NULL, 0, 10000.0);
        bConverted = true;
        // FIXME: workaround to avoid green render result somehow when textName[0,1] are unset while textures will be freed and shouldn't be used.
        tex->texName[0] = 0;
        tex->texName[1] = 0;
    }
    
    freeUploadTexture4HWDecoder(renderer,pInfo4Free,bConverted);
    
    return ret;
}

#pragma mark - NXT_Platform extension

NXT_Error NXT_Platform_GetProperty(NXT_HThemeRenderer renderer, int nPropertyName, void**ppRetValue)
{
    if ( nPropertyName == TR_PROPERTY_NAME_IOS_LAYER )
    {
        *ppRetValue = (void*)RENDERER_IOSCTX(renderer)->eaglLayer;
    }
    return NXT_Error_None;
}

NXT_Error NXT_Platform_CheckInternalContextAvailable(NXT_HThemeRenderer renderer)
{
    return checkInternalContextAvailable();
}

NXT_Error NXT_Platform_SetRenderTargetInfo(struct NXT_ThemeRenderer_ * hRenderer, NXT_Platform_RenderTargetInfo info)
{
    NEXTHEMERENDERER_TARGET_ADDITIONAL_INFO additinalInfo;
    additinalInfo.scale = (int) info.scale;
    
    return setRenderTargetInformation( hRenderer, info.x, info.y, info.size.width, info.size.height, &additinalInfo );
}

NXT_Error NXT_Platform_UnloadTexture(struct NXT_ThemeRenderer_ * hRenderer, NXT_TextureID texture_id)
{
    NXT_Error error = NXT_Error_None;
    LOGI("[%s %d] start", __func__, __LINE__);
    
    // Confirm renderer is valid and initialized
    if( !hRenderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !hRenderer->bInitialized ) {
        LOGE("[%s %d] Renderer is not initialized", __func__, __LINE__);
        return NXT_Error_NotInit;
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &hRenderer->getRawVideoSrc(texture_id);
    
    if( tex->bTextureSetByPlatform ) {
        tex->texNameInitCount = 0;
        tex->bTextureSetByPlatform = 0;
    }
    
    return error;
}

#include "nexTexturesLogger.h"

void Logging_glGenTextures(int n, unsigned int* textures, const char* where, int line)
{
    glGenTextures(n, textures);
    NSLog(@"glGenTextures(%d) - %s(%d)", n, where, line);
    for(int i = 0; i < n; i++) {
        NSLog(@"  %04d", textures[i]);
    }
}

void Logging_glDeleteTextures (int n, unsigned int* textures, const char* where, int line)
{
    BOOL printLog = YES;
    if ( n == 0 ) {
        printLog = NO;
    }
    if (n == 2 && line == 10382 && textures[0] == 0 && textures[1] == 0) {
        printLog = NO;
    }
    
    if (printLog) {
        NSLog(@"glDeleteTextures(%d) - %s(%d)", n, where, line);
        for(int i = 0; i < n; i++) {
            NSLog(@"  %04d", textures[i]);
        }
    }
    glDeleteTextures(n, textures);
}

@implementation AppStateMonitor
- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.isActive = YES;
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidChangeActiveState:) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidChangeActiveState:) name:UIApplicationDidBecomeActiveNotification object:nil];
    }
    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    [super dealloc];
}

- (void)applicationDidChangeActiveState:(NSNotification *)notif {
    BOOL active = NO;
    
    if ([notif.name isEqualToString:UIApplicationWillResignActiveNotification]) {
        active = NO;
    } else if ([notif.name isEqualToString:UIApplicationDidBecomeActiveNotification]) {
        active = YES;
    } else {
        return;
    }
    self.isActive = active;
}

@end
