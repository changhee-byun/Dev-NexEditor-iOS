/**
 * File Name   : AssetResourceLoader.h
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

@interface AssetResourceLoader : NSObject

+ (NSData *)loadFileContentsAtPath:(NSString *) path;
+ (NSData *)loadFileContentsAtPath:(NSString *) path decrypt:(BOOL) decrypt recipient:(NSString *) recipient;
+ (id) loadJSONObjectAtPath:(NSString *) path;

@end
