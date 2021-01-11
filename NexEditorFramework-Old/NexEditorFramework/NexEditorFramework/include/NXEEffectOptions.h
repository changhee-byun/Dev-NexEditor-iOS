/******************************************************************************
 * File Name   :	NXEEffectOptions.h
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
#import "NXEEditorType.h"

@class NXEClip;

/** \class NXETextOption
 *  \brief NXETextOption class defines all text-related options available to set to an effect.
 *  \since version 1.0.5
 */
@interface NXETextOption : NSObject

/*! A key property of the text option included in a Effect 
 *  \since version 1.0.5
 */
@property (nonatomic, retain) NSString *key4textField;

/*! A text value property of the text option included in a Effect 
 *  \since version 1.0.5
 */
@property (nonatomic, retain) NSString *value4textField;

/*! A maxline property of the text option included in a Effect 
 *  \since version 1.0.5
 */
@property (nonatomic) int maxLine;

@end

/** \class NXEColorOption
 *  \brief NXEColorOption class defines all color-related options available to set to an effect.
 *  \since version 1.0.5
 */
@interface NXEColorOption : NSObject

/*! A key property of the color option included in a Effect 
 *  \since version 1.0.5
 */
@property (nonatomic, retain) NSString *key4colorField;

/*! A color value property of the color option included in a Effect, as an ARGB 32-bit value 
 *  \since version 1.0.5
 */
@property (nonatomic, retain) NSString *value4colorField;

@end

/** \class NXEEffectOptions
 *  \brief NXEEffectOptions class defines all available options of an effect.
 *  This class defines and sets available options for an effect on a clip.<p>
 *  Each effect in the NexEditor&trade;SDK has different characteristics and therefore different possible settings options with default values, 
 *  but setting values can be adjusted in detail using this class to customize the effects.
 *  \since version 1.0.5
 */
@interface NXEEffectOptions : NSObject

/*! \brief An arrary of NXETextOption objects for a clip effect or transition effect. 
 *  \since version 1.1.0
 */
@property (nonatomic, copy, readonly) NSArray *textOptions;

/*! \brief An array of NXEColorOption objects for a clip effect or transition effect.
 *  \since version 1.1.0
 */
@property (nonatomic, copy, readonly) NSArray *colorOptions;

/** \brief Updates textOptions and colorOptions properties of the receiver from the clip or transition effect speficied by effectId.
 *  \param effectId Id of a clip or transition effect to read options from
 *	\param type Effect type
 *  \return YES if successful, NO otherwise.
 *  \since version 1.1.0
 */
- (BOOL)setupWithEffectId:(NSString *) effectId type:(NXEEffectType) type;
@end
