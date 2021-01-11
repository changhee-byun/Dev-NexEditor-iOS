/**
 * File Name   : AssetItemInfoPrivate.h
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

#ifndef AssetItemInfoPrivate_h
#define AssetItemInfoPrivate_h

extern NSString *const kAssetItemInfoFieldNone;

@interface AssetItemInfo(Private)

- (id) initWithData:(NSData *) infoData;

/// kAssetItemInfoFieldNone returned if field == AssetItemInfoFieldNone
+ (NSString *) infoFieldStringFor:(AssetItemInfoField) field;
@end

#endif /* AssetItemInfoPrivate_h */
