/*
 * File Name   : NXETextureDisposeBag.h
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

#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/gl.h>
#import "LayerGLContext.h"

@protocol NXETextureDisposeBagProtocol <NSObject>
- (void) disposeTexture:(GLuint) tex contextIndex:(LayerGLContextIndex) index;
@end

@interface NXETextureDisposeBag: NSObject <NXETextureDisposeBagProtocol>
@property (nonatomic) BOOL hasTextures;
@property (nonatomic, readonly) NSInteger refId;

- (instancetype) initWithReferenceId:(NSInteger) refId;
- (void) disposeTexture:(GLuint) tex contextIndex:(LayerGLContextIndex) index;
- (void) deleteTextuersForContextIndex:(LayerGLContextIndex) index;

+ (instancetype) disposeBagWithReferenceId:(NSInteger) refId;
@end

