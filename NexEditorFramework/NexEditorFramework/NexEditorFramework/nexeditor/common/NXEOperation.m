/******************************************************************************
 * File Name   : NXEOperation.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEOperation.h"

@implementation NXEOperation

- (void)dealloc
{
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        
    }
    return self;
}

- (instancetype)initWithBlocks:(void (^)(void))executionBlock completionBlock:(void (^)(void))completionBlock
{
    self = [super init];
    if(self) {
        [self addExecutionBlock:^{
            executionBlock();
        }];
        
        [self setCompletionBlock:^{
            completionBlock();
        }];
    }
    return self;
}

@end

