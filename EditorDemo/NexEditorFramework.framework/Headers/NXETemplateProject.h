/**
 * File Name   : NXETemplateProject.h
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

@class NXETemplateAssetItem;
@class NXEClipSource;

/** \class NXETemplateProject
 *  \since version: 1.5.0
 */
@interface NXETemplateProject : NXEProject

/** \brief Creates NXETemplateProject with arry of NXECilp and NXETemplateAssetItem instance
 *  \param templateAssetItem NXETemplateAssetItem instance
 *  \param clips arry of NXECilp
 *  \return An NXETemplateProject object if succeed, nil if failed.
 *  \since version: 1.5.0
 */
- (instancetype) initWithTemplateAssetItem:(NXETemplateAssetItem *)templateAssetItem clips: (NSArray<NXEClip *> *)clips error:(NSError **)error;

@end
