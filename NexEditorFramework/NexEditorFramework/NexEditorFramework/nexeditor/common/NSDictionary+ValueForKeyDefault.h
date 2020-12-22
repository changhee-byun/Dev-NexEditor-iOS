/**
 * File Name   : NSDictionary+ValueForKeyDefault.h
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

#import <Foundation/Foundation.h>

@interface NSDictionary (ValueForKeyDefault)

- (int)intValueWithKey:(NSString * _Nonnull)key defaultValue:(int)defaultValue;

- (float)floatValueWithKey:(NSString * _Nonnull)key defaultValue:(float)defaultValue;

- (NSString * _Nullable)stringValueWithKey:(NSString * _Nonnull)key defaultValue:(NSString * _Nullable)defaultValue;

@end
