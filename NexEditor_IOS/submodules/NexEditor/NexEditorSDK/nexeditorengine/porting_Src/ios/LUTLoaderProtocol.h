/******************************************************************************
 * File Name   : LUTLoaderProtocol.h
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

@protocol LUTLoaderProtocol <NSObject>

@required

- (int)getLUTWithID:(int)lutResourceId exportFlag:(int)exportFlag;
- (void)deleteTextureForLUT;

@end
