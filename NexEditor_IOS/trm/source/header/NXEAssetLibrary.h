/*
 * File Name   : NXEAssetLibrary.h
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
#import "NXEAssetItem.h"
#import "NXETemplateAssetItem.h"
#import "NXEEffectAssetItem.h"
#import "NXETransitionAssetItem.h"
#import "NXETextEffectAssetItem.h"

/**
 * \class NXEAssetItemCategory
 * \brief Provides asset item category constants as class properties.
 * \see NXEAssetLibrary's \c -itemsInCategory: method
 * \since version 1.2
 */
@interface NXEAssetItemCategory: NSObject
/**
 * \brief Asset category 'Template'
 * \since version 1.2
 */
@property (class, readonly) NSString * _Nonnull Template;
/**
 * \brief Asset category 'Transition'
 * \since version 1.2
 */
@property (class, readonly) NSString * _Nonnull transition;
/**
 * \brief Asset category 'Effect'
 * \since version 1.2
 */
@property (class, readonly) NSString * _Nonnull effect;

/**
 * \brief Asset category 'TextEffect'
 * \since version 1.2
 */
@property (class, readonly) NSString * _Nonnull textEffect;
@end

/**
 * \brief Defines methods to retrieve Assets
 * \since version 1.2
 */
@protocol NXEAssetLibrary <NSObject>

/**
 * \brief Retrieves asset items in the specified category
 * \since version 1.2
 */
- (NSArray<NXEAssetItem *> * _Nonnull) itemsInCategory:(NSString * _Nonnull) category;
/**
 * \brief Retrieves asset item for the specified itemId
 * \return nil if item with itemId not found
 * \since version 1.2
 */
- (NXEAssetItem * _Nullable) itemForId:(NSString * _Nonnull) itemId;

@end

/**
 * \class NXEAssetLibrary
 * \brief Class provides methods to configure asset package location and retrieve asset items by category or item id.
 * \since version 1.2
 */
@interface NXEAssetLibrary : NSObject <NXEAssetLibrary>
/**
 * \brief Adds file URL to root directory of asset packages to be accessed through a NXEAssetLibrary instance.
 *
 *        If asset packages are bundled with the App and located under, for example, 'AssetPackages' directory,
 *        the following code will let NXEAssetLibrary to search for assets when -itemForId: is called.
 * \code
 *     NSURL *url = [[NSBundle mainBundle] URLForResource:@"AssetPackages" withExtension:nil];
 *     [NXEAssetLibrary addAssetSourceDirectoryURL:url];
 * \endcode
 *
 * \since version 1.2
 */
+ (void) addAssetSourceDirectoryURL:(NSURL * _Nonnull) fileURL;

/**
 * \brief Lets NXEAssetLibrary rescan directories added as source file URLs.
 *
 *        This is useful when asset packages are added to one of asset package source directories added by \c -addAssetSourceDirectoryURL:
 *
 * \note Directory scan will occur when items are queried next time, by \c -itemForId: or \c -itemsInCategory:.
 * \since version 1.2
 */
+ (void) reloadSources;

/**
 * \brief Asynchronously retrieves asset items in the specified category and completion block is invoked with list of asset items
 * \since version 1.2
 */
- (void) itemsInCategory:(NSString * _Nonnull) category completion:(void (^ _Nonnull)(NSArray<NXEAssetItem *> * _Nonnull)) completion;

/**
 * \brief Returns NXEAssetLibrary instance. 
 * \note Use this method to create NXEAssetLibrary instance. Do not call -init
 * \since version 1.2
 */
+ (instancetype _Nonnull) instance;

@end
