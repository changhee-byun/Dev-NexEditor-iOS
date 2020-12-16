/******************************************************************************
 * File Name   : VignetteLoaderProtocol.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@protocol VignetteLoaderProtocol <NSObject>

@required

- (int)getVignetteTextID:(int)exportFlag;
- (void)deleteTextureForVignette;

@end
