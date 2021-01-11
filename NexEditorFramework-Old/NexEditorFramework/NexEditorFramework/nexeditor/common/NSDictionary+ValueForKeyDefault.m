/**
 * File Name   : NSDictionary+ValueForKeyDefault.m
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

#import "NSDictionary+ValueForKeyDefault.h"

@implementation NSDictionary (ValueForKeyDefault)

- (BOOL)applyDefaultValueWithKey:(NSString *)key
{
    if(self[key] == nil
       || [self[key] isEqualToString:@"default"] || [self[key] isEqualToString:@"none"] || [self[key] isEqualToString:@"null"]
       || [self[key] isEqualToString:@"type"]/* overlayTitle */) {
        return YES;
    }
    return NO;
}

- (int)intValueWithKey:(NSString * _Nonnull)key defaultValue:(int)defaultValue
{
    if([self applyDefaultValueWithKey:key]) {
        return defaultValue;
    }
    return [self[key] intValue];
}

- (float)floatValueWithKey:(NSString * _Nonnull)key defaultValue:(float)defaultValue
{
    if([self applyDefaultValueWithKey:key]) {
        return defaultValue;
    }
    return [self[key] floatValue];
}

- (NSString * _Nullable)stringValueWithKey:(NSString * _Nonnull)key defaultValue:(NSString * _Nullable)defaultValue
{
    if([self applyDefaultValueWithKey:key]) {
        return defaultValue;
    }
    return self[key];
}

@end
