/******************************************************************************
 * File Name   :	NXETemplateParser.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <Foundation/Foundation.h>

@class NXETemplateProject;
@class NXEClip;
@class NXETemplateAssetItem;

/** \class NXETemplateInfo
 *  \brief NXETemplateInfo class defines all properties associated with a template.
 *  \since version 1.0.5
 */
@interface NXETemplateInfo : NSObject

/** \brief Tempalte name information
 *  \since version 1.0.5
 */
@property (nonatomic, retain, readonly) NSString *templateName;

/** \brief Template version information
 *  \since version 1.0.5
 */
@property (nonatomic, retain, readonly) NSString *templateVersion;

/** \brief Template description information
 *  \since version 1.0.5
 */
@property (nonatomic, retain, readonly) NSString *templateDescription;

/** \brief Template mode
 *  \since version 1.1.0
 */
@property (nonatomic, retain, readonly) NSString *templateMode;

@end

/** \class NXETemplateParser
 *  \brief NXETemplateParser class defines all methods to parse a template.
 *  \since version 1.0.5
 */
@interface NXETemplateParser : NSObject

/** \brief Applies the speficied template to the project.
 *  \param project An NXEProject object.
 *  \param item NXETemplateAssetItem object.
 *  \since version 1.2
 */
- (void)configureProject:(NXETemplateProject *)project withAssetItem:(NXETemplateAssetItem *)item;

@end
