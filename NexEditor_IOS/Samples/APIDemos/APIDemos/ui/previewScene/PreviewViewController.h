/******************************************************************************
 * File Name   :	PreviewViewController.h
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
#import "CERangeSlider.h"

@import NexEditorFramework;

@interface PreviewViewController : UIViewController

@property (retain, nonatomic) IBOutlet NXELayerEditorView *engineView;
@property (retain, nonatomic) IBOutlet UILabel *progressLabel;
@property (retain, nonatomic) IBOutlet UILabel *totalLabel;
@property (retain, nonatomic) IBOutlet CERangeSlider *trimSlider;
@property (retain, nonatomic) IBOutlet UIButton *playButton;
@property (retain, nonatomic) IBOutlet UIButton *layerButton;
@property (retain, nonatomic) IBOutlet UIButton *exportButton;
@property (retain, nonatomic) IBOutlet UIButton *audioTrackButton;
@property (strong, nonatomic) IBOutlet UIButton *projectEditButton;

//- (BOOL)onlyPortrait;

- (void) setClipSources:(NSArray <NXEClipSource *> *) clipSources;

/**
 * \brief audioTrackList 배열에 들어있는 clip을 source project에 추가해준다.
 */
- (void)setAudioTrack:(NSMutableArray *)audioTrackList;

/**
 * \brief project 들어있는 visual clip들을 source project에 새로이 추가해준다.
 */
- (void)setClips:(NSArray<NXEClip *> *)clips;

@end
