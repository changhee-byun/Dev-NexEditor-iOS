/******************************************************************************
 * File Name   : UserField.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "UserField.h"

#pragma mark - UserFieldOption

@interface UserFieldOption ()

- (instancetype _Nullable) initFieldOptionWithAttributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict;

@end

@implementation UserFieldOption

- (void)dealloc
{
    [self.optionValue release];
    [self.optionIcon release];
    [super dealloc];
}

- (instancetype _Nullable)initFieldOptionWithAttributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict
{
    self = [super init];
    if(self) {
        self.optionValue = attributeDict[@"value"];
        self.optionIcon = attributeDict[@"icon"];
    }
    return self;
}

- (instancetype)copyWithZone:(NSZone *)zone
{
    UserFieldOption *destination = [[self class] allocWithZone:zone];
    destination.optionValue = self.optionValue;
    destination.optionIcon = self.optionIcon;
    return destination;
}

@end

#pragma mark - UserField

@implementation UserField

- (void)dealloc
{
    [self.userfieldID release];
    [self.userfieldType release];
    [self.userfieldDefault release];
    [self.userfieldEditor release];
    [self.userfieldOptions release];
    //
    [super dealloc];
}

- (instancetype _Nullable)initUserFieldWithAttributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict
{
    self = [super init];
    if(self) {
        self.userfieldType = attributeDict[@"type"];
        self.userfieldID = [NSString stringWithFormat:@"%@:%@", _userfieldType, attributeDict[@"id"]];
        self.userfieldDefault = attributeDict[@"default"];
        self.userfieldMaxLines = (attributeDict[@"maxlines"]==nil) ? 1 : (int)[attributeDict[@"maxlines"] integerValue];
        self.userfieldEditor = attributeDict[@"editor"];
        self.userfieldOptions = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)setFieldOption:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict
{
    UserFieldOption *userFieldOption = [[UserFieldOption alloc] initFieldOptionWithAttributes:attributeDict];
    [self.userfieldOptions addObject:userFieldOption];
    [userFieldOption release];
}

- (instancetype)copyWithZone:(NSZone *)zone
{
    UserField *destination = [[UserField alloc] init];
    
    destination.userfieldMaxLines = self.userfieldMaxLines;
    destination.userfieldType = self.userfieldType;
    destination.userfieldEditor = self.userfieldEditor;
    destination.userfieldDefault = self.userfieldDefault;
    destination.userfieldID = self.userfieldID;
    destination.userfieldOptions = [[NSMutableArray alloc] initWithArray:self.userfieldOptions copyItems:YES];
    
    // userfieldOptions 배열에 들어가는 Element의 retainCount값을 1로 유지하기 위해서.
    //
    [destination.userfieldOptions release];
    
    return destination;
}

@end

