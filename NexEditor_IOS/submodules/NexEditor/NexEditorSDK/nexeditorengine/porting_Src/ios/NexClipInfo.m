/******************************************************************************
 * File Name   : NexClipInfo.m
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
#import "NexClipInfo.h"
#import "NexClipInfoPrivate.h"

@interface NexClipInfo()
@property (nonatomic, strong) NSArray<NSNumber *>* seekTable;
@property (nonatomic, strong) NSString* thumbnailPath;
@end

@implementation NexClipInfo

- (id)init
{
    return self;
}

- (void) dealloc
{
    self.seekTable = nil;
    self.thumbnailPath = nil;
    [super dealloc];
}
@end
