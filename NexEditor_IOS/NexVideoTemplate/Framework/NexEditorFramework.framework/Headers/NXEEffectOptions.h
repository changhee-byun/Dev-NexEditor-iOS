/******************************************************************************
 * File Name   :	NXEEffectOptions.h
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
#import "NXEEditorType.h"

@class NXEClip;

/**
 * \brief A class that defines the text-related option included in a clip effect.
 * \since version 1.0.5
 */
@interface NXETextOption : NSObject
/**
 * \brief A key property of the text option included in a clip effect.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *key4textField;
/**
 * \brief A text value property of the text option included in a clip effect.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *value4textField;
/**
 * \brief A maxline property of the text option included in a clip effect. 
 * \since version 1.0.5
 */
@property (nonatomic) int maxLine;

@end

/**
 * \brief A class that defines the color-related option included in a clip effect.
 * \since version 1.0.5
 */
@interface NXEColorOption : NSObject
/**
 * \brief A key property of the color option included in a clip effect.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *key4colorField;
/**
 * \brief A color value property of the color option included in a clip effect, as an ARGB 32-bit value
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *value4colorField;

@end

/**
 * \brief This class defines and sets the options available for an effect on a clip
 *			<p>Each effect in the NexEditor&trade;SDK has different characteristics and therefore different possible 
 * 			settings options with default values, but the default settings can be adjusted in detail using this class to customize the effects. 
 *			<p>Instances of this class cannot be created as stand-alone instances but must be used with <b>initWithClip</b>
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [engine setProject:project];
 *
 *      BOOL retValue_e = [effectOption setupWithEffectId:clip.vinfo.titleEffectID];
 *      if(retValue_e != NO) {
 *          // edit text field value via interface, TextOptionInClipEffect
 *          for(NXETextOption *textOption in effectOption.textOptionsInClipEffect) {
 *           NSLog(@"predefined text, maxLine in clipEffect:[%@, %d]", textOption.value4textField, textOption.maxLine);
 *          }
 *
 *          // edit color field value via interface, ColorOptionInClipEffect
 *          for(NXEColorOption *colorOption in effectOption.colorOptionsInClipEffect) {
 *              NSLog(@"predefined color in clipEffect:[%@]", colorOption.value4colorField);
 *          }
 *      }
 *
 *      BOOL retValue_t = [effectOption setupWithTransitionEffectId:clip.vinfo.clipEffectID];
 *      if(retValue_t != NO) {
 *          // edit text field via interface, TextOptionInTransitionEffect
 *          for(NXETextOption *textOption in effectOption.textOptionsInTransitionEffect) {
 *              NSLog(@"predefined text, maxLine in transitionEffect:[%@, %d]", textOption.value4textField, textOption.maxLine);
 *          }
 *
 *          // edit color field via interface, ColorOptionInTransitionEffect
 *          for(NXEColorOption *colorOption in effectOption.colorOptionsInTransitionEffect) {
 *              NSLog(@"predefined color in transitionEffect:[%@]", colorOption.value4colorField);
 *          }
 *      }
 *  }
 * \endcode
 * \since version 1.0.5
 */
@interface NXEEffectOptions : NSObject
/**
 * \brief A property of the multiple text options in a clip effect as an \c array. 
 *      Composed of a TextOption interface. 
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray *textOptionsInClipEffect;
/**
 * \brief A property of the multiple text options in a transition effect as an \c array.
 *      Composed of a TextOption interface.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray *textOptionsInTransitionEffect;
/**
 * \brief  A property of the multiple color options in a clip effect as an \c array.
 *     Composed of a ColorOption interface.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray *colorOptionsInClipEffect;
/**
 * \brief A property of the multiple color options in a transition effect as an \c array.
 *      Composed of a ColorOption interface.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray *colorOptionsInTransitionEffect;

/**
 * \brief Some of the ClipEffects has user fields which are editable and they are:
 *      text, color, and selection.
 *      text field can edit a text in a ClipEffect.
 *      color field can edit the color of a text in a ClipEffect.
 *      text, color values can be multiple, so they are saved in the arrays of textOptionsInClipEffect, colorOptionsInClipEffect.
 */
- (BOOL)setupWithEffectId:(NSString *)effectId;

/**
 * \brief Some of the TransitionEffects has user fields which are editable and they are:
 *      text, color, and selection.
 *      text field can edit a text in a TransitionEffect.
 *      color field can edit the color of a text in a TransitionEffect.
 *      selecion field can edit predefined things such as the position of a text in a TransitionEffect.
 *      text, color values can be multiple, so they are saved in the arrays of textOptionsInTransitionEffect, colorOptionsInTransitionEffect.
 */
- (BOOL)setupWithTransitionId:(NSString *)transitionId;

@end
