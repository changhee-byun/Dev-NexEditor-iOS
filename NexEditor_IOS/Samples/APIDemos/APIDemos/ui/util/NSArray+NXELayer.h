/**
 * File Name   : NSArray+NXELayer.h
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
@import NexEditorFramework;

@interface NSArray (NXELayer)
- (NSArray *) removingObject:(id) object;
- (NSArray *) removingObjects:(NSArray *) objects;
- (NSArray *) layersWithType:(NXELayerType) type;
/// Creates a new array by replacing layers of 'type' in self with newLayers
- (NSArray<NXELayer *> *) mergingLayers:(NSArray<NXELayer *> *) newLayers type:(NXELayerType) type;

@end
