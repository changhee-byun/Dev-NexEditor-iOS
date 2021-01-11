/******************************************************************************
 * File Name   : NXEOperation.h
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

@interface NXEOperation : NSBlockOperation

- (instancetype)initWithBlocks:(void (^)(void))block completionBlock:(void (^)(void))completionBlock;
- (instancetype)init;

@end
