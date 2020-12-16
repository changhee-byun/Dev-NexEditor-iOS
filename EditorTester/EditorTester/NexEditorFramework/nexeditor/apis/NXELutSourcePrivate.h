/*
 * File Name   : NXELutRegistryPrivate.h
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
#import "NXELutSource.h"
#import "NXELutRegistry.h"
#import "NXEEditorErrorCode.h"
#import <OpenGLES/ES2/gl.h>

typedef struct CubeLutData {
    uint32_t* bytes;
    NSUInteger imageWidth;
    NSUInteger imageHeight;
} CubeLutData;


@interface NXEPNGLutSource : NXEFileLutSource

@end


@interface NXECubeLutSource (Private)
- (CubeLutData) loadData;
@end


@interface NXEFileLutSource (Private)

@property (nonatomic, readonly) NSString *uniqueKey;

- (instancetype) initWithPath:(NSString*)path uniqueKey:(NSString*)key;
- (GLuint) createTexture;
- (ERRORCODE) checkValidity;

@end
