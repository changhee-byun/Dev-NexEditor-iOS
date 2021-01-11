/*
 * File Name   : NXEAssetItemGroup.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
#import "NXEAssetItem.h"

/**
 * \class NXEAssetItemGroup
 * \brief Represents a group of asset items belong to the same asset. For example, items are the same kind of template but have different aspect ratios.
 * Create instance using NXEAssetLibrary's -groupsInCategory:.
 * \since version 1.5.0
 */
@interface NXEAssetItemGroup : NSObject
/**
 * \brief Localized name of the group
 * \since version 1.5.0
 */
@property (nonatomic, readonly) NSString *localizedName;
/**
 * \brief Category string of the item.
 * \since version 1.5.0
 */
@property (nonatomic, readonly) NSString *category;
/**
 * \brief Asset items belong to this group.
 * \since version 1.5.0
 */
@property (nonatomic, readonly) NSArray<NXEAssetItem *> *items;
/**
 * \brief Indicates the item group has an icon image.
 * \see loadIconImageData:
 * \since version 1.5.0
 */
@property (nonatomic, readonly) BOOL hasIcon;
/**
 * \brief Asynchronously loads the icon image data if hasIcon is YES
 * \param block Invoked when icon image loading finishes. data is not nil if the image data was successfully loaded. extension can be @"png", @"jpg", or @"svg". If extension is @"png" or @"jpg", the image data can be used with UIImage.
 * \since version 1.5.0
 */
- (void) loadIconImageData:( void (^)(NSData * data, NSString *extension)) block;
@end
