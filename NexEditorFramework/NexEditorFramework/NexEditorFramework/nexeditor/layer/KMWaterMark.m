/******************************************************************************
 * File Name   : KMWaterMark.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMWaterMark.h"
#import "BundleResource.h"

@implementation KMWaterMark

- (instancetype)init
{
    NSString *path = [BundleResource pathFor:BundleResourceKeyWM];
    self = [super initWithImage:[UIImage imageWithContentsOfFile:path]];
    if(self) {
        self.y = 0;
        // x should be adjusted through delegate which will be invoked once setNeedsGeometryUpdate is called. 
        self.x = 0;
        self.isSelectable = NO;
        self.isHit = NO;
    }
    return self;
}

@end
