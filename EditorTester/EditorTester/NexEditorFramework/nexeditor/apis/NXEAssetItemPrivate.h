/**
 * File Name   : NXEAssetItemPrivate.h
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

#import "NXEAssetItem.h"
#import "AssetItem.h"
#import "NXEEditorType.h"

@interface NXEAssetItem(Private)
@property (nonatomic) AssetItem *raw;
@property (nonatomic, readonly) NSString *fileInfoResourcePath;
@property (nonatomic, readonly) NSData *fileInfoResourceData;

- (id) initWithRawItem:(AssetItem *) rawItem;
- (BOOL) shouldDecryptResource:(AssetItemInfoResourceType) resourceType;
/// Loads and decrypts resource file at subpath of the asset item
- (NSData *) dataForResourceFile:(NSString *) subpath;

@end

@interface AssetItem(Resource)
@end

@interface NSString(AspectType)
/// Converts to NXEAspectType if the receiver has string representation of aspect type suc as "16v9", or "9v16". NXEAspectTypeUnknown will be returned if not.
- (NXEAspectType) aspectType;
/// Scans string representation of aspect type if the receiver has '.' separated string format. For example, "com.nexstreaming.template.16v9.happy" would be detected as NXEAspectTypeRatio16v9
- (NXEAspectType) detectAspectType;
/// Returns [text detectAspectType] if text is non-nil. NXEAspectTypeUnknown will be returned if text is nil.
+ (NXEAspectType) detectAspectTypeFrom:(NSString *) text;
@end
