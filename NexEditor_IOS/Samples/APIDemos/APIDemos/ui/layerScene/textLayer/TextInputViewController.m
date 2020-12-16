/******************************************************************************
 * File Name   :	TextInputViewController.m
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

#import "TextInputViewController.h"

@interface TextInputViewController ()

@property (copy, nonatomic) void (^refreshTextLayer)(NSString *);
@property (nonatomic, assign) NSString *textLayerTitle;

@end

@implementation TextInputViewController

- (void)setupTextLayer4InputCB:(refreshTextInputWidget)refreshTableView textLayerTitle:(NSString *)textLayerTitle
{
    self.refreshTextLayer = refreshTableView;
    self.textLayerTitle = textLayerTitle;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.textEditView.layer.cornerRadius = 5.0f;
    self.textEditView.layer.borderWidth = .5f;
    self.textEditView.layer.borderColor = [UIColor blackColor].CGColor;
    self.textEditView.editable = YES;
    self.textEditView.text = self.textLayerTitle;
}

- (IBAction)actionRefreshList:(id)sender
{
    [self.navigationController popViewControllerAnimated:YES];
    
    self.refreshTextLayer(self.textEditView.text);
}

@end
