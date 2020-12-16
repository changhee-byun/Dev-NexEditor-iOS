/******************************************************************************
 * File Name   : EditorFrameworkTestTests-Bridging-Header.h
 * Description :
 *******************************************************************************
 
 NexStreaming Corp. Confidential & Proprietary
 Copyright (C) 2017 NexStreaming Corp. All rights are reserved.
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "EditorUtil.h"
#import "ImageUtil.h"
#import "AssetLibrary.h"
// Private
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"
#import "NXETransitionAssetItemPrivate.h"
#import "NXEEffectAssetItemPrivate.h"
#import "LUTMap.h"
#import "TemplateHelper.h"

#import "KMLayer.h"
#import "KMWaterMark.h"
#import "NexImage.h"

#import "AssetLibraryProxy.h"
#import "AspectRatioTool.h"
#import "LayerManager.h"
#import "InternalAssetLibrary.h"

#import "NXEProject+TemplateInternal.h"
#import "KMImageLayer.h"
#import "BundleResource.h"
#import "NXELutRegistry.h"
#import "NXELutSource.h"
#import "NXELutSourcePrivate.h"
#import "NXELayer.h"
#import "NXEVideoLayer.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import <NexEditorEngine/NXEVisualClipInternal.h>
#import <NexEditorEngine/NexMediaDef.h>
#import "NXEEnginePrivate.h"
#import "FontTools.h"

#import "NXEClipInternal.h"

// CollageLoader
#import "CollageData.h"

@interface VignetteLayers(Private)
@property (nonatomic, strong) NSArray<KMImageLayer *> *imageLayers;
@end
