/******************************************************************************
 * File Name   :	NXETheme.h
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

@class NXEProject;
@class Theme;

/**
 * \brief This class contains effects, for basic video editing, sorted in themes.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @property(nonatomic, retain) NXETheme *theme;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
 *      NSArray *themeArray = [effectLibrary getThemeNames];
 *      self.theme = [[NXETheme alloc] initWithTheme:[themeArray objectAtIndex:0]];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [theme applyThemeToProject:project applyPredefinedThemeBGM:true]
 *      [engine setProject:project];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5 
 */
@interface NXETheme : NSObject

/**
 * \brief This gets the opening text in a theme.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *opening;
/**
 * \brief This gets the middle text in a theme.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *middle;
/**
 * \brief This gets the ending text in a theme.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *ending;
/**
 * \brief This gets the name information in a theme.
 * \since version 1.0.5
 */
@property (nonatomic, retain, readonly) NSString *themeName;
/**
 * \brief This gets the description information in a theme. 
 * \since version 1.0.5
 */
@property (nonatomic, retain, readonly) NSString *themeDescription;

/**
 * \brief
 * \since version 1.0.5
 */
- (instancetype)initWithTheme:(NSString *)theme;

/**
 * \brief This sets a theme to a project.
 * \project The project instance to apply a theme.
 * \applyThemeBGM <tt>YES</tt> to set the background music of the theme as a project background music; others to not set anything.
 * \since version 1.0.5
 */
- (void)applyThemeToProject:(NXEProject *)project applyPredefinedThemeBGM:(BOOL)applyThemeBGM;

@end
