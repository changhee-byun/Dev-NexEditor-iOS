/**
 * File Name   : NXEProject+TemplateInteranl.h
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

#import "NXEProject.h"
#import "NXEProjectInternal.h"
#import "NXETemplateProject.h"

@class OverlayItem;
@class KMImageLayer;

@interface VignetteLayers: NSObject
@property (nonatomic, readonly) NSArray<KMImageLayer *> *imageLayers;
- (instancetype) initWithImageLayers:(NSArray<KMImageLayer *> *) imageLayers;
@end

@interface NXEProject (TemplateInteranl)

@property (nonatomic) VignetteLayers *vignetteLayers;
- (NSArray<KMImageLayer *>*)vignetteLayersWithOverlays:(NSArray<OverlayItem *>*)overlays;

@end
