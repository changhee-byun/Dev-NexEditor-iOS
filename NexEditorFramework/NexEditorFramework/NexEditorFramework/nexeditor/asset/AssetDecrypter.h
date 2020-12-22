/**
 * File Name   : AssetDecrypter.h
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

typedef NSArray<NSData *> AssetDecrypterKeyset;

/// Low-level data decryper with keyset demaling interface
@interface AssetDecrypter: NSObject
- (instancetype) initWithKeyset:(AssetDecrypterKeyset *) keyset;
- (NSData *) decrypt:(NSData *)encrypted;

+ (AssetDecrypterKeyset *) demangleKeyset:(AssetDecrypterKeyset *) mangledKeyset;
@end

