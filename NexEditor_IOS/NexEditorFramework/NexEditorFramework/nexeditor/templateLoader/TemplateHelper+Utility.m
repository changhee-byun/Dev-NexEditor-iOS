/**
 * File Name   : TemplateHelper+Utility.m
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

#import "TemplateHelper+Utility.h"

@implementation TemplateHelper (Utility)

+ (BOOL)isDefaultValueWithKey:(NSString *)key
{
    if(key == nil || [key isEqualToString:@"default"] || [key isEqualToString:@"null"] || [key isEqualToString:@"none"]) {
        return YES;
    }
    return NO;
}

+ (int)intValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(int)defaultValue
{
    int result = defaultValue;
    NSString *value = dictionary[key];
    
    if(![self isDefaultValueWithKey:value]) {
        result = value.intValue;
    }
    
    return result;
}

+ (float)floatValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(float)defaultValue
{
    float result = defaultValue;
    NSString *value = dictionary[key];
    
    if(![self isDefaultValueWithKey:value]) {
        result = value.floatValue;
    }
    
    return result;
}

+ (NSString *)stringValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(NSString *)defaultValue
{
    NSString *value = dictionary[key];
    
    if([self isDefaultValueWithKey:value]) {
        value = defaultValue;
    }
    
    return value;
}

+ (NSArray *)arrayValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key
{
    NSDictionary *dict = dictionary[key];
    
    if(dict == nil) {
        return nil;
    }
    
    NSArray *sortedKeys = [[dict allKeys] sortedArrayUsingComparator:^NSComparisonResult(NSString *first, NSString *second) {
        return [first compare:second];
    }];
    
    NSMutableArray *array = [NSMutableArray array];
    for(NSString *dictkey in sortedKeys) {
        [array addObject:dict[dictkey]];
    }
    
    return [[array copy] autorelease];
}

@end
