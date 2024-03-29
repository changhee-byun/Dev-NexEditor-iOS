/**
 * File Name   : AssetIndexer.h
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

@protocol AssetIndexer <NSObject>
- (NSDictionary<NSString *, id> *) allEntriesForSection:(NSString *) section;
- (id) entryForKey:(NSString *) key section:(NSString *) section;
- (void) setEntry:(id) entry forKey:(NSString *)key section:(NSString *)section;
- (void) removeEntryForKey:(NSString *) key section:(NSString *) section;
@end
