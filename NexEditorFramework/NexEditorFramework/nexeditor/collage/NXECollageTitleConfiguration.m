/**
 * File Name   : NXECollageTitleConfiguration.m
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

#import "NXECollageTitleConfiguration.h"
#import "NXECollageTitleConfigurationInternal.h"
#import "RenderDataEncoder.h"

@interface NXECollageTitleConfiguration()

@property (nonatomic, retain) NSDate *date;
@property (nonatomic, retain) CollageTitleData* titleData;
@property (nonatomic, readonly) NSString *textRenderData;

@end

@implementation NXECollageTitleConfiguration

- (NSDate *)date
{
    if (_date == nil) {
        _date = [[NSDate date] retain];
    }
    return _date;
}

- (void) dealloc
{
    self.date = nil;
    self.titleData = nil;

    [super dealloc];
}

- (NSString *)textRenderData
{
    NSLocale *locale = [NSLocale autoupdatingCurrentLocale];
    NSString *languageCode = locale.languageCode;
    if (self.titleData.defaultTitle[locale.languageCode] == NO) {
        languageCode = @"en";
    }
    NSString *defaultText = self.titleData.defaultTitle[languageCode];
    return [RenderDataEncoder textRenderData:self titleID:self.titleData.titleInfoID date:self.date defaultText:defaultText];
}

@end

@implementation NXECollageTitleConfiguration (Internal)

- (instancetype) initWithCollageTitleData:(CollageTitleData *)titleData
{
    self = [super init];
    
    self.titleData = titleData;
    
    return self;
}

@end
