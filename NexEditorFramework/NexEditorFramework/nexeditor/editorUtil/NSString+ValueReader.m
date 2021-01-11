/**
 * File Name   : NSString+ValueReader.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
#import "NSString+ValueReader.h"

@implementation NSString (ValueReader)

- (unsigned long long) scanHexLongLong {
    unsigned long long value;
    [[NSScanner scannerWithString:self] scanHexLongLong:&value];
    return value;
}

- (int) intValueOrDefault:(int) value {
    if(self == nil) {
        return value;
    }
    return [self intValue];
}

- (float) floatValueOrDefault:(float) value {
    if(self == nil) {
        return value;
    }
    return [self floatValue];
}

- (float) boolValueOrDefault:(BOOL) value {
    if(self == nil) {
        return value;
    }
    return [self boolValue];
}

@end

