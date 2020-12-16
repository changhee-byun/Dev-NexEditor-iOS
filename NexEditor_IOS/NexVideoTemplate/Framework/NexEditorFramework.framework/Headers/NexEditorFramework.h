/******************************************************************************
 * File Name   : NexEditorFramework.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <UIKit/UIKit.h>

//! Project version number for NexEditorFramework.
FOUNDATION_EXPORT double NexEditorFrameworkVersionNumber;

//! Project version string for NexEditorFramework.
FOUNDATION_EXPORT const unsigned char NexEditorFrameworkVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <NexEditorFramework/PublicHeader.h>
//
//// for Kinemaster
//#import <NexEditorFramework/NexLog.h>
//#import <NexEditorFramework/NexClipInfo.h>
//#import <NexEditorFramework/NexEditor.h>
//#import <NexEditorFramework/EditorEventListener.h>
//#import <NexEditorFramework/ImageLoaderProtocol.h>
//#import <NexEditorFramework/CustomLayerProtocol.h>
//#import <NexEditorFramework/LUTLoaderProtocol.h>
//#import <NexEditorFramework/RenderItem.h>
//#import <NexEdiotrFramework/NEXVIDEOEDITOR_Types.h>
//
//// for NXELayer
//#import <NexEditorFramework/NXELayer.h>
//#import <NexEditorFramework/NXEColorMatrix.h>

// for Kinemaster and NexEditorSDK
#import <NexEditorFramework/NXEVisualClip.h>
#import <NexEditorFramework/NXEAudioClip.h>

// for NexEditorSDK
#pragma mark - nexeditor/apis
#import <NexEditorFramework/NXEVideoLayer.h>
#import <NexEditorFramework/NXETextLayer.h>
#import <NexEditorFramework/NXEImageLayer.h>
#import <NexEditorFramework/NXEEngine.h>
#import <NexEditorFramework/NXEVisualEdit.h>
#import <NexEditorFramework/NXEClip.h>
#import <NexEditorFramework/NXETheme.h>
#import <NexEditorFramework/NXETemplateParser.h>
#import <NexEditorFramework/NXEProject.h>
#import <NexEditorFramework/NXEColorEffect.h>
#import <NexEditorFramework/NXEAudioEdit.h>
#import <NexEditorFramework/NXEEffectOptions.h>
#import <NexEditorFramework/NXEEffectLibrary.h>
#import <NexEditorFramework/NXEEditorType.h>
#import <NexEditorFramework/NXEEngineUIEventListener.h>
#import <NexEditorFramework/UIListenerProtocol.h>

#pragma mark - nexeditor/videoeditor
#import <NexEditorFramework/NXEEditorErrorCode.h>

#pragma mark - mediainfo
#import <NexEditorFramework/NXEThumbnailParser.h>
#import <NexEditorFramework/NXEThumbnailUICallbackBlock.h>
