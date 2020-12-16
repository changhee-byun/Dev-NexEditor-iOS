/*
 * File Name   : NXELutSource.mm
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

#import <Foundation/Foundation.h>
#import "NXELutSource.h"
#import "NXELutSourcePrivate.h"
#import "ImageUtil.h"
#import <NexEditorEngine/NexCubeLUTLoader.h>

@interface NXEFileLutSource()
@property (nonatomic, strong) NSString *path;
@property (nonatomic, strong) NSString *uniqueKey;
@end

@implementation NXEFileLutSource

- (instancetype) initWithPath:(NSString*)path
{
    self = [super init];
    if(self) {
        self.path = path;
        self.uniqueKey = path;
    }
    return self;
}

- (instancetype) initWithPath:(NSString*)path uniqueKey:(NSString*)key
{
    self = [super init];
    if(self) {
        self.path = path;
        self.uniqueKey = key;
    }
    return self;
}

- (GLuint) createTexture
{
    return GL_NONE;
}

- (ERRORCODE) checkValidity
{
    return ERROR_UNSUPPORT_FORMAT;
}

-(BOOL)isEqual:(id)object
{
    NXEFileLutSource* source = object;
    if( self.uniqueKey == source.uniqueKey ) {
        return YES;
    }
    return NO;
}

- (NSUInteger)hash
{
    return [_path hash]^[self.uniqueKey hash];
}

@end


@implementation NXECubeLutSource

- (GLuint) createTexture
{
    GLuint textureId = GL_NONE;
    
    CubeLutData data = [self loadData];
    
    if(data.bytes != nil) {
        textureId = [ImageUtil glTextureIDfromData:(GLubyte*)data.bytes Width:(int)data.imageWidth Height:(int)data.imageHeight];
        
        [NexCubeLUTLoader unloadCubeLUTBytes:(NexCubeLUTBytes) data.bytes];
        data.bytes = NULL;
    }
    return textureId;
}

- (ERRORCODE) checkValidity
{
    ERRORCODE result = ERROR_UNSUPPORT_FORMAT;
    NSFileManager* fm = [NSFileManager defaultManager];
    
    if( [fm fileExistsAtPath:self.path] ) {
        CubeLutData data = [self loadData];
        if(data.bytes != nil) {
            [NexCubeLUTLoader unloadCubeLUTBytes:(NexCubeLUTBytes) data.bytes];
            data.bytes = NULL;
            result = ERROR_NONE;
        }
    }
    else {
        result = ERROR_FILE_IO_FAILED;
    }
    return result;
}

- (CubeLutData) loadData
{
    
#define IMG_WIDTH   64
#define IMG_HEIGTH  4096
        
    CubeLutData lutData;
    lutData.bytes = NULL;
    
    uint32_t* bytes = NULL;
    
    if(self.path) {
        NSData *data = [[NSFileManager defaultManager] contentsAtPath:self.path];
        bytes = (uint32_t *) [NexCubeLUTLoader loadCubeLUTBytesFromData:data];
    }
        
    if(bytes) {
        lutData.bytes = bytes;
        lutData.imageWidth = IMG_WIDTH;
        lutData.imageHeight = IMG_HEIGTH;
    }
    return lutData;
}

@end


@implementation NXEPNGLutSource

- (GLuint) createTexture
{
    GLuint textureId = GL_NONE;
    
    if(self.path) {
        UIImage *image = [UIImage imageNamed:self.path];
        if(image != nil) {
            textureId = [ImageUtil glTextureIDfromImage:image];
        }
    }
    return textureId;
}

- (ERRORCODE) checkValidity
{
    ERRORCODE result = ERROR_UNSUPPORT_FORMAT;
    NSFileManager* fm = [NSFileManager defaultManager];
    
    if( [fm fileExistsAtPath:self.path] ) {
        UIImage *image = [UIImage imageNamed:self.path];
        if( image != nil ) {
            result = ERROR_NONE;
        }
    }
    else {
        result = ERROR_FILE_IO_FAILED;
    }
    return result;
}

@end


