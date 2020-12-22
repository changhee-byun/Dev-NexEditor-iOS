/**
 * File Name   : AssetResourceDecryptionDelegate.h
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
@protocol AssetResourceDecryptionDelegate <NSObject>
/// Set this first
@property (nonatomic, strong) NSString *packagePath;
/// And this secondly
@property (nonatomic, strong) NSString *subpathToEncryptionManifest;
/// any many times of this
- (NSData *) decryptData:(NSData *) encrypted atSubpath:(NSString *) subpathFromPackage;
- (void) releaseResource;
@end

