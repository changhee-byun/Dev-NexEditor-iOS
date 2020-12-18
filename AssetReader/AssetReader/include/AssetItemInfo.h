/**
 * File Name   : AssetItemInfo.h
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

typedef NS_ENUM(NSInteger, AssetItemInfoField) {
    AssetItemInfoFieldNone          = -1,
    AssetItemInfoFieldId            =  0,        /// NSString, Optional
    AssetItemInfoFieldType,        /// NSString
    AssetItemInfoFieldHidden,      /// BOOL
    AssetItemInfoFieldLabel,       /// NSArray<NSString *> *
    AssetItemInfoFieldIcon,        /// NSString, Optional
    AssetItemInfoFieldFilename,    /// NSString, Optional
    AssetItemInfoFieldSampleText,  /// NSString, Optional
    AssetItemInfoFieldMergePaths,  /// NSArray<NSString *>, Optional
};

@interface AssetItemInfo : NSObject

- (id) valueForInfoField:(AssetItemInfoField) field;
- (id) valueForInfoFieldKey:(NSString *) fieldKey;

@end
