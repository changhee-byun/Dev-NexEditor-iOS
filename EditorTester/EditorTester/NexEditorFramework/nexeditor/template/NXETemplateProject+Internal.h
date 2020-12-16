/**
 * File Name   : NXETemplateProject+Internal.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

#import "NXETemplateProject.h"

@class NexSceneInfo;
@class NexDrawInfo;

typedef NS_ENUM(NSUInteger, TemplateVersion) {
    TemplateVersion1x = 0,
    TemplateVersion2x,
    TemplateVersion3x
};

@interface NXETemplateProject (Internal)
@property (nonatomic) TemplateVersion version;
@property (nonatomic, readonly) NXEAspectType aspectType;
@property (nonatomic, retain) NSMutableArray <NexSceneInfo *> *sceneInfos;
@property (nonatomic, retain) NSMutableArray <NexDrawInfo *> *drawInfos;
@end
