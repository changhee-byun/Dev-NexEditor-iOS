/******************************************************************************
 * File Name   : TemplateParser.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXETemplateParser.h"
#import "TemplateV1Handler.h"
#import "TemplateV2Handler.h"
#import "TemplateV3Handler.h"
#import "TemplateHelper.h"
#import "NXEAssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"
#import "NXETemplateProject+Internal.h"

@interface NXETemplateInfo ()

- (instancetype)initWithName:(NSString *)name version:(NSString *)version description:(NSString *)description mode:(NSString *)mode;

@end

@implementation NXETemplateInfo

- (void) dealloc
{
    [_templateName release];
    [_templateVersion release];
    [_templateDescription release];
    [_templateMode release];

    [super dealloc];
}

- (instancetype)initWithName:(NSString *)name version:(NSString *)version description:(NSString *)description mode:(NSString *)mode
{
    self = [super init];
    if(self) {
        _templateName = [NSString stringWithString:name];
        _templateVersion = (version == nil) ? @"1.0" : [NSString stringWithString:version];
        _templateDescription = [NSString stringWithString:description];
        _templateMode = [NSString stringWithString:mode];
        return self;
    }
    return nil;
}

@end

@implementation NXETemplateParser

#pragma mark - for sdk

- (void)configureProject:(NXETemplateProject *)project withData:(NSData *)data
{
    NXEError *error = nil;
    TemplateHelper *helper = [[[TemplateHelper alloc] initWithJSONData:data deepScan:YES nxeError:&error] autorelease];
    if(helper == nil) {
        @throw [NSException exceptionWithName:@"JSONException"
                                       reason:error.localizedDescription
                                     userInfo:nil];
    }
    
    NSString *version = [NSString stringWithString:helper.version];
    
    id templateHandler = nil;
    
    if([version hasPrefix:@"3."]) {
        project.version = TemplateVersion3x;
        templateHandler = (TemplateV3Handler *)[[TemplateV3Handler alloc] initWithTemplateHelper:helper];
        
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"clipType == %@", @(NXE_CLIPTYPE_VIDEO)];
        if ([project.visualClips filteredArrayUsingPredicate:predicate].count > 0) {
            [templateHandler release];
            return; // skip if the project has video clip.
        }
    }
    else if([version hasPrefix:@"2."]) {
        project.version = TemplateVersion2x;
        templateHandler = (TemplateV2Handler *)[[TemplateV2Handler alloc] initWithTemplateHelper:helper];
    }
    else {
        project.version = TemplateVersion1x;
        templateHandler = (TemplateV1Handler *)[[TemplateV1Handler alloc] initWithTemplateHelper:helper];
    }

    [templateHandler setTemplateDataToProject:project];
    [templateHandler release];
}

/** \brief Applies the speficied template to the project.
 *  \param project An NXEProject object.
 *  \param item NXETemplateAssetItem object.
 *  \since version 1.1.0
 */
- (void)configureProject:(NXETemplateProject *)project withAssetItem:(NXETemplateAssetItem *)item
{
    NSData *data;
    if((data = item.fileInfoResourceData) == nil) {
        @throw [NSException exceptionWithName:@"GeneralException"
                                       reason:@"Not get template data from the directory."
                                     userInfo:nil];
    }
    
    [self configureProject:project withData:data];
    
    return;
}

@end
