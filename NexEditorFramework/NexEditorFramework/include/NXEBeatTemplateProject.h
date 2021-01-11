/*
 * File Name   : NXEBeatTemplateProject.h
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

#import "NXEProject.h"
#import "NXEError.h"
#import "NXEBeatAssetItem.h"

@class NXEAssetItem;

/** \class NXEBeatTemplateProject
 *  \since version: 1.5.1
 */
@interface NXEBeatTemplateProject : NXEProject

/** \brief Creates NXEBeatTemplateProject with arry of NXECilp and NXEBeatAssetItem instance
 *  \param assetItem NXEBeatAssetItem instance
 *  \param clips arry of NXECilp
 *  \return An NXEBeatTemplateProject object if succeed, nil if failed.
 *  \since version: 1.5.1
 */
- (instancetype) initWithBeatTemplateAssetItem:(NXEBeatAssetItem *)assetItem  clips: (NSArray<NXEClip *> *)clips error:(NSError **)error;

@end

