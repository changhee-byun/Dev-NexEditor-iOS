/******************************************************************************
 * File Name   :	TemplateHandler.h
 * Description : SuperClass to handle Template
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Property
 Copyright (C) 2005~2017 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>

#import "NXEEngine.h"
#import "NXEProject.h"
#import "NXEProject+TemplateInternal.h"
#import "NXEClip.h"
#import "NXEClipInternal.h"
#import "NXEEditorType.h"
#import <NexEditorEngine/NXEVisualClip.h>
#import "Math.h"
#import "KMImageLayer.h"
#import "LayerManager.h"
#import "TemplateHelper.h"
#import "LUTMap.h"

#define LOG_TAG_TEMPLATE @"NXETemplateParser"

@class TemplateHelper;
@class NXEProject;

@interface OverlayItem : NSObject

@property (nonatomic) int overlayId;
@property (nonatomic, retain) NSString *displayTime;
@property (nonatomic, retain) NSString *variation;
@property (nonatomic, retain) NSString *animationEnable;
@property (nonatomic, retain) NSString *aniFadeInSTime;
@property (nonatomic, retain) NSString *aniFadeInDuration;
@property (nonatomic, retain) NSString *aniFadeOutSTime;
@property (nonatomic, retain) NSString *aniFadeOutDuration;

@end

@interface TemplateHandler : NSObject

@property (nonatomic, retain) TemplateHelper *templateHelper;
@property (nonatomic, retain) NSMutableArray *overlayArray;

- (instancetype)initWithTemplateHelper:(TemplateHelper *)helper;

/** 인자로 전달 된 project에 template을 반영한다.
 */
- (void)setTemplateDataToProject:(NXETemplateProject *)project;

/** 인자로 전달 된 clip에 component로 정의한 property를 설정해준다.
 */
- (void)configureClip:(NXEClip *)clip withComponent:(TemplateComponent *)component;

- (void) setCrop:(NXEClip *)clip withCropMode:(NSString *)cropMode;

- (void) setBGMToProject:(NXETemplateProject *)project bUsingSingleTemplateBGM:(BOOL)bUsingSingleTemplateBGM;

@end
