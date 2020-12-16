/**
 * File Name   : EffectOptionTableViewController.h
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

#import <UIKit/UIKit.h>
@import NexEditorFramework;

typedef void (^EffectOptionsConfiguredBlock)(NXEEffectOptions *);

@class NXEClip;

@interface EffectOptionTableViewController : UITableViewController

/**
 * Call to configure effect option settings before loading the view controller.
 * \param effectId ID for the effect about to configure options for
 * \param effectType Type of the effect require to configure the options UI
 * \param configured Called when user confirms to apply the changes. NXEEffectOptions parameter contains the details of the options changed by the user.
 */
- (void)setupEffectOption:(NSString *)effectId
               effectType:(NXEEffectType)effectType
               configured:(EffectOptionsConfiguredBlock)configured;

@end
