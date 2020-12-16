//
//  ItemQuery.m
//  APIDemos
//
//  Created by Simon Kim on 3/30/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import "ItemQuery.h"
@import NexEditorFramework;

@implementation ItemQuery

+ (NSArray<NSString *> *) itemIdsFromAssetLibraryForType:(NXEEffectType) type
{
    NSString *category;

    NXEAssetLibrary *lib = [NXEAssetLibrary instance];

    if(type == NXE_CLIP_EFFECT) {
        category = NXEAssetItemCategory.effect;
    } else {
        category = NXEAssetItemCategory.transition;
    }

    NSArray<NXEAssetItem *> *items = [lib itemsInCategory:category];
    NSMutableArray *itemIds = [[NSMutableArray alloc] init];

    for( NXEAssetItem *item in items) {
        [itemIds addObject:item.itemId];
    }

    return [itemIds sortedArrayUsingComparator:
                    ^NSComparisonResult(id obj1, id obj2) {
                        return [obj1 compare:obj2];
                    }];
}

+ (NSArray<NSString *> *) itemIdsForType:(NXEEffectType) type
{
    return [self itemIdsFromAssetLibraryForType:type];
}

@end
