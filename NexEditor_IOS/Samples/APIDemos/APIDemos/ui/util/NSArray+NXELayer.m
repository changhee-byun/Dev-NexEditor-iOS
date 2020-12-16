/**
 * File Name   : NSArray+NXELayer.m
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

#import "NSArray+NXELayer.h"

@implementation NSArray (NXELayer)
- (NSArray *) removingObject:(id) object
{
    NSMutableArray *mutable = [self mutableCopy];
    [mutable removeObject:object];
    return [mutable copy];
}

- (NSArray *) removingObjects:(NSArray *) objects
{
    NSMutableArray *mutable = [self mutableCopy];
    [mutable removeObjectsInArray:objects];
    return [mutable copy];
}

- (NSArray *) layersWithType:(NXELayerType) type
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"layerType == %@", @(type)];
    return [self filteredArrayUsingPredicate:predicate];
}

- (NSArray<NXELayer *> *) mergingLayers:(NSArray<NXELayer *> *) newLayers type:(NXELayerType) type
{
    NSMutableArray *layers = [self mutableCopy];
    [layers removeObjectsInArray:[self layersWithType:type]];
    [layers addObjectsFromArray:newLayers];
    return [layers copy];
}

@end
