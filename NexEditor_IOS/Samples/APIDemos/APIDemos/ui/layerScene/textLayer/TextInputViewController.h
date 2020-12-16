/******************************************************************************
 * File Name   :	TextInputViewController.h
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

typedef void (^refreshTextInputWidget)(NSString *);

@interface TextInputViewController : UIViewController

@property (retain, nonatomic) IBOutlet UITextView *textEditView;

- (void)setupTextLayer4InputCB:(refreshTextInputWidget)refreshTableView textLayerTitle:(NSString *)textLayerTitle;

@end
