/**
 * File Name   : AssetItemPathInfo.h
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

@interface AssetItemPathInfo: NSObject
@property (nonatomic, readonly) NSString *packageId;
@property (nonatomic, readonly) NSString *category;
@property (nonatomic, readonly) NSString *path;
@property (nonatomic, readonly) NSString *infoPath;

+(instancetype) pathInfoWithPath:(NSString *)path packageId:(NSString *)packageId category:(NSString *)category;
- (NSString *) subpathInPackagePath:(NSString *) packagePath;

+ (NSString *) subpathOf:(NSString *) path under:(NSString *) parentPath;
@end
