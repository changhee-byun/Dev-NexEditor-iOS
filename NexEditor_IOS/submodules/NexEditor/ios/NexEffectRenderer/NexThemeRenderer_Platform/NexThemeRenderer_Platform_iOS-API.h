/**
 * File Name   : NexThemeRendererInternalAPI-iOS.h
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

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CVPixelBuffer.h>
#import "NexThemeRenderer.h"

@protocol NexThemeRendererImageBuffer<NSObject>
@property (nonatomic) NSUInteger pts;
@property (nonatomic) CVImageBufferRef image;
- (void) releaseResource;
@end

@protocol NTRPixelBufferProviding<NSObject>
@property (nonatomic) CGSize requiredSize;
- (CVPixelBufferRef) newPixelBuffer;
- (void) disposePixelBuffer:(CVPixelBufferRef) pixelBuffer;
@end

#ifdef __cplusplus
extern "C" {
#endif
    /// Returns the EAGLContext, internally created for the current instance of the renderer.
    EAGLContext *NXT_ThemeRenderer_GetEAGLContext(NXT_HThemeRenderer renderer);
    
    /// Replaces the default pixel buffer supply. Setting pixelBufferSupply parameter to nil will restore the default supply.
    void NXT_ThemeRenderer_SetExternalPixelBufferSupply(NXT_HThemeRenderer renderer, id<NTRPixelBufferProviding> pixelBufferSupply);
#ifdef __cplusplus
};
#endif
