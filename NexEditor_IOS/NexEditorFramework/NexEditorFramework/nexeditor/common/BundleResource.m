/**
 * File Name   : BundleResource.m
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

#import "BundleResource.h"

@implementation BundleResource

+ (NSString *) pathFor:(BundleResourceKey) resourceKey
{
    static NSDictionary *resources = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        resources = @{
                      @(BundleResourceKeyWM): @{
                              @"name": @"keepfileFG2HJ6D4",
                              @"inDirectory": @"resource/image"
                              },
                      @(BundleResourceKeyVignetteHorizontal): @{
                              @"name": @"vignette",
                              @"type": @"png",
                              @"inDirectory": @"resource/image"
                              },
                      @(BundleResourceKeyVignetteVertical): @{
                              @"name": @"vignette_9v16",
                              @"type": @"png",
                              @"inDirectory": @"resource/image"
                              },
                      @(BundleResourceKeyTemplateVignetteHorizontal): @{
                              @"name": @"template_vignette",
                              @"type": @"png",
                              @"inDirectory": @"resource/image"
                              },
                      @(BundleResourceKeyTemplateVignetteVertical): @{
                              @"name": @"template_vignette_9v16",
                              @"type": @"png",
                              @"inDirectory": @"resource/image"
                              },                      };
        [resources retain];
    });
    
    NSString *result = nil;
    NSDictionary *entry = resources[@(resourceKey)];
    if ( entry ) {
        NSBundle *bundle = [NSBundle bundleForClass:self.class];
        result = [bundle pathForResource:entry[@"name"]
                                  ofType:entry[@"type"]
                             inDirectory:entry[@"inDirectory"]];
    }
    return result;
}

@end
