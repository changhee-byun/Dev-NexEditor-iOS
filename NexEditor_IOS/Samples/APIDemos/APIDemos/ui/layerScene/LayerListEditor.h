/**
 * File Name   : LayerListEditor.h
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

@protocol LayerListEditor;

typedef void (^LayerListEditorDismissed)(id<LayerListEditor> editor, BOOL done);

@protocol LayerListEditor <NSObject>
@property (nonatomic, copy) NSArray<NXELayer *> *layers;
@property (nonatomic, strong) LayerListEditorDismissed onDismiss;
@end

