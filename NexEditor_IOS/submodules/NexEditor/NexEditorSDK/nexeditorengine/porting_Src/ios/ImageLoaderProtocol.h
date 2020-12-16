/******************************************************************************
 * File Name   : ImageLoaderProtocol.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@protocol ImageLoaderProtocol <NSObject>

@required

// general
- (BOOL) openFile:(NSString *)path width:(NSUInteger *)width height:(NSUInteger *)height pixelData:(void **)pixelData imageResource:(void **)imageResource;
- (BOOL) openThemeImage:(NSString *)path width:(NSUInteger *)width height:(NSUInteger *)height pixelData:(void **)pixelData imageResource:(void **)imageResource;
- (BOOL) openThemeTextDesc:(NSString *)desc text:(NSString *)text width:(NSUInteger *)width height:(NSUInteger *)height pixelData:(void **)pixelData imageResource:(void **)imageResource;
- (void)releaseImageResource:(void *)param;

// renderitem
- (BOOL) openThemeFile:(NSString *)path dataLength:(NSUInteger *)dataLength fileData:(void **)fileData;

@end
