/*
 * File Name   : NXEAssetItem.h
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
#import <UIKit/UIKit.h>
#import "NXEEditorType.h"


@protocol NXEAssetItem

@property (nonatomic, readonly) NSString *itemId;
@property (nonatomic, readonly) NSString *category;
@property (nonatomic, readonly) NXEAspectType aspectType;
@property (nonatomic, readonly) BOOL hasIcon;
- (void) loadIconImageData:( void (^)(NSData * data, NSString *extension)) block;

@end


/**
 * \class NXEAssetItem
 * \brief Base class of asset item classes such as NXETemplateAssetItem, NXETransitionAssetItem and NXEEffectAssetItem.
 * \since version 1.2
 */
@interface NXEAssetItem: NSObject<NXEAssetItem>
/**
 * \brief Unique item id among all the asset items installed.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *itemId;
/**
 * \brief Internal name of the item.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *name;
/**
 * \brief Category string of the item.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *category;
/**
 * \brief Display string of the item in the default language, for example, English.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *label;

/**
 * \brief Description of the asset item.
 * \note The receiver may return nil if the asset item does not have description.
 * \since version 1.2
 */
@property (nonatomic, readonly) NSString *desc;

/**
 * \brief Aspect type the asset item should work on.
 * \note NXEAssetTypeUnknown will be returned if aspect type is not applicable for the asset item the receiver represents.
 * \since version 1.2
 */
@property (nonatomic, readonly) NXEAspectType aspectType;

/**
 * \brief Indicates the item has an icon image.
 * \see loadIconImageData:
 * \since version 1.2
 */
@property (nonatomic, readonly) BOOL hasIcon;
/**
 * \brief Asynchronously loads the icon image data if hasIcon is YES
 * \param block Invoked when icon image loading finishes. data is not nil if the image data was successfully loaded. extension can be @"png", @"jpg", or @"svg". If extension is @"png" or @"jpg", the image data can be used with UIImage.
 * \since version 1.2
 */
- (void) loadIconImageData:( void (^)(NSData * data, NSString *extension)) block;
@end
