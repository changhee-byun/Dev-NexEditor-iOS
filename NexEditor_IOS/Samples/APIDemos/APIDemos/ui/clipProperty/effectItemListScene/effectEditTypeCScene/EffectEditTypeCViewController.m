/******************************************************************************
 * File Name   :	EffectEditTypeCViewController.m
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

#import "EffectEditTypeCViewController.h"
//#import "NEEffectOptions.h"
@import NexEditorFramework;

@interface EffectEditTypeCViewController ()

@property (copy, nonatomic) EffectOptionConfiguredBlock configuredBlock;
@property (assign, nonatomic) NXEEffectType effectType;
@property (assign, nonatomic) int selectedIdx;
@property (retain, nonatomic) NXEEffectOptions *effectOption;

@end

@implementation EffectEditTypeCViewController

#pragma mark - EffectOptionEditor
- (void)setupWithEffectOption:(NXEEffectOptions *)effectOption
                  selectedIdx:(int)selectedIdx
                   effectType:(NXEEffectType)effectType
                   configured:(EffectOptionConfiguredBlock)configured
{
    self.effectOption = effectOption;
    self.selectedIdx = selectedIdx;
    self.effectType = effectType; // 0: clip effect, 1: transition effect
    self.configuredBlock = configured;
}

#pragma mark -
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.textEditFiled.text = ((NXETextOption *)self.effectOption.textOptions[self.selectedIdx]).value4textField;
}

- (IBAction)actionRefreshList:(id)sender
{
    ((NXETextOption *)self.effectOption.textOptions[self.selectedIdx]).value4textField = self.textEditFiled.text == nil ? @"" : self.textEditFiled.text;
    [self.navigationController popViewControllerAnimated:YES];
    self.configuredBlock();
}

- (void)dealloc {
    _textEditFiled = nil;
}
@end
