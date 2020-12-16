/******************************************************************************
 * File Name   :	NXEEffectLibrary.h
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

@class Effect;
@class ClipEffect;
@class Theme;
@class TransitionEffect;

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

/**
 * \ingroup types
 * \brief A list of Filter Types
 * \since version 1.0.5 
 */
typedef NS_ENUM(NSUInteger, FilterTypes) {
    FILTER_HIDDEN = 0x00000001, // This causes hidden themes or effects to be included (these are themes or effects that are marked hidden internally within their package)
    FILTER_THEMEONLY = 0x00000002, // Causes theme-only effects to be included. Theme-only effects are effects that are intended to be used only as part of a theme, not individually.
    FILTER_NONTHEME = 0x00000004, // Causes non-theme-only effects to be included.These are effects that intended to be individually selectable by the user.
    FILTER_SPECIAL = 0x00000008, // Causes special themes or effects to be included. These are themes and effects that are used for internal implementation purposes and should not be used directly (so this flag should generally be used for testing purposes only, not in production code).    <p> For example, when no transition effect is selected, a special zero-duration effect is actually used internally. <p>This effect should not be applied directly and is subject to change in future versions, and so is included under FILTER_SPECIAL
    FILTER_ALL = 0xFFFFFFFF // Causes all themes or effects to be included.<p> Since this also includes #FILTER_SPECIAL, it should generally not be used in production code.
} _DOC_ENUM(FilterTypes);

/**
 * \ingroup types
 * \brief A list of Effect Types
 * \since version 1.0.5 
 */
typedef NS_ENUM(NSUInteger, EffectTypes) {
    EFFECT_NONE = 0, // This indicates that no effect is set to a clip. 
    EFFECT_CLIP_AUDIO, // This indicates that the clip effect is automatically set matching the theme currently set. 
    EFFECT_CLIP_USER, // This indicates that the clip effect is not part of any theme but can be set by the user.
    EFFECT_TRANSITION_AUTO, // This indicates that the transition effect is automatically set matching the theme currently set.
    EFFECT_TRANSITION_USER // This indicates the transition effect is set by the user.
} _DOC_ENUM(EffectTypes);
/**
 * \brief This class manages all required effects and theme packages in advance. 
 *          After parsing all effects and theme packages, the class sorts and delivers the related resources according to certain conditions.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEEffectLibrary *effectLibrary;
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.effectLibrary = [NXEEffectLibrary sharedInstance:false
 *                                            completeLoading:^(int numError) {
 *      // effect parsing is done, so normal editing task is possible at this point.
 *      //
 *          self.engine = [NXEEngine sharedInstance];
 *          self.project = [[NXEProject alloc] init];
 *
 *          NSString *filePath = @"";
 *          int errorType;
 *          NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *          if(clip == nil) {
 *              NSLog(@"errorType=%d", errorType);
 *              return;
 *          }
 *          [project addClip:clip];
 *          [engine setProject:project];
 *      }];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5     
 */
@interface NXEEffectLibrary : NSObject

/**
 * \brief This method gets the library instance for clip effects, transition effects, and theme packages needed for the editing.
 * \since version 1.0.5
 */
+ (instancetype)sharedInstance;

/**
 * \brief This method gets the library instance for clip effects, transition effects, and theme packages needed for the editing.
 * \param isMinimumLoading <tt>YES</tt> to execute the completeLoading callback after a set effect is parsed; <tt>NO</tt> to execute the completeLoading callback after all effects are parsed.
 * \param completeLoading Registers a method to be executed after an event parsing is finished.
 * \since version 1.0.5
 */
+ (instancetype)sharedInstance:(BOOL)isMinimumLoading
               completeLoading:(void (^)(int numError))completeLoading;

/**
 * \brief NXEEffectLibrary instance를 release해주는 함수
 */
- (void)cleanup;

/**
 * \brief This method adds the theme name information from the installed effects to the destination array.
 * \param[out] destination
 * \return The predefined ThemeIds value as in \c NSArray.
 * \code
 *  NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
 *  NSArray *themeNameArray = [effectLibrary getThemeNames];
 * \endcode
 * \since version 1.0.5
 */
- (NSArray *)getThemeNames;

/**
 * \brief This method adds the clipEffect id information from the installed effects to the destination array. 
 * \param flags Enter one value from the FilterTypes Type.
 * \param[out] destination
 * \return The created ClipEffectIds value as in \c NSArray.
 * \code
 *  NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
 *  NSArray *clipEffectIdArray = [effectLibrary getClipEffectIdsWithFliterFlag:FILTER_ALL];
 * \endcode
 * \see FilterTypes
 * \since version 1.0.5
 */
- (NSArray *)getClipEffectIdsWithFliterFlag:(int)flags;

/**
 * \brief This method adds the transitionEffect id information from the installed effects to the destination array. 
 * \param flags Enter one value from the FilterTypes Type.
 * \param[out] destination
 * \return The created TransitionEffectIds value as in \c NSArray.
 * \code
 *  NXEEffectLibrary *effectLibrary = [NXEEffectLibrary\ sharedInstance];
 *  NSArray *transitionEffectIdArray = [effectLibrary getTransitionEffectIdsWithFliterFlag:FILTER_ALL];
 * \endcode 
 * \see FilterTypes 
 * \since version 1.0.5
 */
- (NSArray *)getTransitionEffectIdsWithFliterFlag:(int)flags;

@end
