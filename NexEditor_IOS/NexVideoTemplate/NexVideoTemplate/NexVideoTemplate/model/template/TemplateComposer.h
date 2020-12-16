/******************************************************************************
 * File Name   :	TemplateComposer.h
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
#import <Foundation/Foundation.h>
#import "NexEditorFramework/NexEditorFramework.h"

@interface TemplateComposer : NSObject

/**
 */
- (NSUInteger)listCount;
/**
 */
- (UIImage*)getTemplateImageAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio;
/**
 */
- (NSString *)getTemplateNameAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio;
/**
 */
- (NSArray *)getTemplateItemAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio;

/**
 */
- (void)applyTemplateToProject:(NXEProject*)project templateFileName:(NSString *)templateFileName inDirectory:(NSString *)subpath;
@end
