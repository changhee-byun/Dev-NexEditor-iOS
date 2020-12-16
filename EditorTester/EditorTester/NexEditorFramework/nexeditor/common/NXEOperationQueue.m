/******************************************************************************
 * File Name   : NXEOperationQueue.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEOperationQueue.h"

@interface NXEOperationQueue()

@end

@implementation NXEOperationQueue

- (void)dealloc
{
   
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        [self setMaxConcurrentOperationCount:1];
    }
    return self;
}

@end
