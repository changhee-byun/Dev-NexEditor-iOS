/******************************************************************************
 * File Name   :	NXELayersManager.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>

@class NXELayer;

@interface NXELayersManager : NSObject

+ (instancetype)sharedInstance;

- (void)cleanup;

/**
 * \brief Adds the input object to layerArray.
 * \param layer The layer input object is one of NXETextLayer, NXEImageLayer, or NXEVideoLayer.
 * \param layerType
 */
- (void)addLayer:(NXELayer *)layer;

/**
 * \brief 등록 된 Layer 중, 하나의 항목을 삭제한다.
 * \param layer The layer input object is one of NXETextLayer, NXEImageLayer, or NXEVideoLayer.
 * \param layerType
 */
- (void)removeEntryForIndex:(long)layerId;

/**
 * \brief 등록 한 Layer를 모두 삭제한다.
 */
- (void)removeAllEntry;

/**
 * \brief 등록 한 Layer에 대한 정보를 받아올 수 있다.
 * \return NSArray
 */
- (NSArray *)getLayerList;

@end
