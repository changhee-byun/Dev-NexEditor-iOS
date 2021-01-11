/**
 * File Name   : TemplateHelper+Utility.h
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

#import "TemplateHelper.h"

@interface TemplateHelper (Utility)

+ (int)intValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(int)defaultValue;

+ (float)floatValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(float)defaultValue;

+ (NSString *)stringValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key defaultValue:(NSString *)defaultValue;

+ (NSArray *)arrayValueWithDictionary:(NSDictionary *)dictionary key:(NSString *)key;

@end
