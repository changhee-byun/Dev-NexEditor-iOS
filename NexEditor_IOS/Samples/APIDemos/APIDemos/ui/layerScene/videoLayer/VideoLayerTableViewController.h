/******************************************************************************
 * File Name   :	VideoLayerTableViewController.h
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

#import <UIKit/UIKit.h>
#import "LayerListEditor.h"

@interface VideoLayerTableViewController : UITableViewController <LayerListEditor>
@property (nonatomic, copy) NSArray<NXELayer *> *layers;
@property (nonatomic, strong) LayerListEditorDismissed onDismiss;
@end
