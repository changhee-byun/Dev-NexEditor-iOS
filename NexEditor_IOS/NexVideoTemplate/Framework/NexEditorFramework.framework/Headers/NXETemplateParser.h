/******************************************************************************
 * File Name   :	NXETemplateParser.h
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

/**
 * \brief A class that can get name, version, version, and description information from a template.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @property(nonatomic, retain) NXETemplateParser *templateParser;
 *  @property(nonatomic, retain) NXETemplateInfo *templateInfo;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *      self.templateParser = [[NXETemplateParser alloc] init];
 *      self.templateInfo = [templateParser getTemplateInfo:@"template_family"
 *                                              inDirectory:@"resource/asset/template"];
 *      NSLog(@"template info(name, description) = (%@, %@)", templateInfo.templateName, templateInfo.templateDescription);
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
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5
 */
@interface NXETemplateInfo : NSObject

/**
 * \brief This can get the name information from a template.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *templateName;
/**
 * \brief This can get the version information from a template.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *templateVersion;

/**
 * \brief This can get the description information from a template.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSString *templateDescription;

@end

/**
 * \brief This class newly composes clips of a project that has been created from a preregistered template file.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @property(nonatomic, retain) NXETemplateParser *templateParser;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *      self.templateParser = [[NXETemplateParser alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [templateParser applyTemplateToProject:project
 *                            templateFileName:@"template_family"
 *                                 inDirectory:@"resource/asset/template"];
 *      [engine setProject:project];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5
 */
@interface NXETemplateParser : NSObject

/**
 * \brief 인자로 받은 templatePath에 있는 파일을 기반으로 project를 재구성해준다.
 * \param templateFileName 확장자를 가진 파일이름 가령, template_love.txt
 * \param subpath The bundle subdirectory, can be nil.
 * \return <b>NXETemplateInfo</b> class.
 * \since version 1.0.5
 */
- (NXETemplateInfo *)getTemplateInfo:(NSString *)templateFileName inDirectory:(NSString *)subpath;

/**
 * \brief 인자로 받은 templatePath에 있는 파일을 기반으로 project를 재구성해준다.
 * \param project The project instance to which a template effect will be applied.
 * \param templateFileName 확장자를 가진 파일이름 가령, template_love.txt
 * \param subpath The bundle subdirectory, can be nil
 * \since version 1.0.5
 */
- (void)applyTemplateToProject:(NXEProject *)project templateFileName:(NSString *)templateFileName inDirectory:(NSString *)subpath;

/**
 * \brief Template 배경음원을 설정 해 준다.
 * \param project The project instance to which a template effect will be applied.
 * \param filePath 배경음원에 대한 전체 경로를 입력 해 준다.
 * \param volumeScale 음원에 대한 Scale값을 입력 해 준다. 0.0 ~1.0 입력 해 준다.
 */
- (void)setTemplateCustomBGM:(NXEProject *)project filePath:(NSString *)filePath volumeScale:(float)volumeScale;

@end
