/**
 * File Name   : AssetPackage.h
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

@interface AssetPackage : NSObject
@property (nonatomic, readonly) NSString *path;
@property (nonatomic, readonly) NSString *mergePath;
@property (nonatomic, readonly) NSString *packageId;
@property (nonatomic, readonly) NSString *cryptrecp;
/// Per-locale asset names
@property (nonatomic, readonly) NSDictionary <NSString *, NSString *>* assetNames;

- (instancetype) initWithPath:(NSString *) path packageId:(NSString *)packageId;
- (BOOL) isEncrypted:(NSString *) subpath;
@end
