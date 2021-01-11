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
#import "NXECollageAssetItem.h"
#import "NXEAssetItemGroup.h"
#import "NXEBeatAssetItem.h"

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

/**
 * \brief Asset category 'Collage'
 * \since version 1.5.0
 */
@property (class, readonly) NSString * _Nonnull collage;

/**
 * \brief Asset category 'BeatTemplate'
 * \since version 1.5.1
 */
@property (class, readonly) NSString * _Nonnull beatTemplate;

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
 * \brief Rescans directories added as source file URLs to clear out package directories no longer exist in the file system.
 * \note Blocks until if finishes rebuilding Asset Library internal database.
 * \since version 1.2
 *  \deprecated To add or remove each asset package directory, use -addAssetPackageDirectoryURLs:, -removeAssetPackageDirectoryURLs:, or removeAllAssetPackageDirectoryURLs:.
 */
+ (void) reloadSources __attribute__((deprecated));

/**
 * \brief Adds file URLs of each asset package to be accessed through a NXEAssetLibrary instance.
 *        Each URL must point to an asset package directory, not parent.
 * \since version 1.5.1
 */
+ (void) addAssetPackageDirectoryURLs:(NSArray <NSURL *> * _Nonnull) fileURLs;

/**
 * \brief Removes asset packages at each of 'fileURLs' from NXEAssetLibrary. Asset items in each asset packages are no longer retrieved by -itemForId: or -itemsInCategory:.
 *        Each URL must point to an asset package directory, not parent.
 * \since version 1.5.1
 */
+ (void) removeAssetPackageDirectoryURLs:(NSArray <NSURL *> * _Nonnull) fileURLs;

/**
 * \brief Removes all asset packages that were added by -addAssetPackageDirectoryURLs: or addAssetSourceDirectoryURL: from NXEAssetLibrary.
 * \since version 1.5.1
 */
+ (void) removeAllAssetPackageDirectoryURLs;

/**
 * \brief Asynchronously retrieves asset items in the specified category and completion block is invoked with list of asset items
 * \since version 1.2
 */
- (void) itemsInCategory:(NSString * _Nonnull) category completion:(void (^ _Nonnull)(NSArray<NXEAssetItem *> * _Nonnull)) completion;

/**
 * \brief Retrieves asset item groups in the specified category
 * \since version 1.5.0
 */
- (NSArray<NXEAssetItemGroup *> * _Nonnull) groupsInCategory:(NSString * _Nonnull) category;

/**
 * \brief Returns NXEAssetLibrary instance. 
 * \note Use this method to create NXEAssetLibrary instance. Do not call -init
 * \since version 1.2
 */
+ (instancetype _Nonnull) instance;

@end
