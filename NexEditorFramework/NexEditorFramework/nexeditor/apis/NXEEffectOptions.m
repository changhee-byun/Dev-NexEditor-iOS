/******************************************************************************
 * File Name   : NXEEffectOptions.m.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEEffectOptions.h"
#import "NXEClip.h"
#import "UserField.h"
#import "AssetLibraryProxy.h"
#import "NXEEffectAssetItemPrivate.h"
#import "NXETransitionAssetItemPrivate.h"

@implementation NXETextOption

- (void)dealloc
{
    [_key4textField release];
    [_value4textField release];
    [super dealloc];
}

- (NSString *) value4textField
{
    if ( _value4textField == nil ) {
        _value4textField = @"";
    }
    return _value4textField;
}

@end

@implementation NXEColorOption

- (void)dealloc
{
    [_key4colorField release];
    [_value4colorField release];
    [super dealloc];
}

- (NSString *) value4colorField
{
    if ( _value4colorField == nil ) {
        _value4colorField = @"";
    }
    return _value4colorField;
}
@end

// MJ 삭제.
//
//@implementation SelectionOption
//
//- (void)dealloc
//{
//    [_key4selectionField release];
//    [_value4selectionField release];
//    [_menuItems release];
//    [super dealloc];
//}
//
//- (instancetype)init
//{
//    self = [super init];
//    if(self) {
//        _menuItems = [[NSArray alloc] init];
//    }
//    return self;
//}
//
//@end

@interface NXEEffectOptions()
@property (nonatomic) NXEEffectType effectType;
@property (nonatomic, copy) NSArray *textOptions;
@property (nonatomic, copy) NSArray *colorOptions;

@end

@implementation NXEEffectOptions

- (void)dealloc
{
    self.textOptions = nil;
    self.colorOptions = nil;
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.textOptions = [NSArray array];
        self.colorOptions = [NSArray array];
        return self;
    }
    return nil;
}

#pragma mark -
- (BOOL)setupWithEffectId:(NSString *)effectId
{
    id<ConfigurableAssetItem> clipEffect = (id<ConfigurableAssetItem>)[[AssetLibrary proxy] itemForId:effectId];
    if(clipEffect != nil && [clipEffect conformsToProtocol:@protocol(ConfigurableAssetItem)]) {
        if([clipEffect.userFields count] == 0) {
            return NO;
        }
        
        NSMutableArray *textOptions = [NSMutableArray array];
        NSMutableArray *colorOptions = [NSMutableArray array];
        
        for(UserField *userField in clipEffect.userFields) {
            if([userField.userfieldType isEqualToString:@"text"]) {
                NXETextOption *textOption = [[NXETextOption alloc] init];
                textOption.maxLine = userField.userfieldMaxLines;
                textOption.key4textField = [userField.userfieldID copy];
                textOption.value4textField = [userField.userfieldDefault copy];
                [textOptions addObject:textOption];
                [textOption release];
            } else if([userField.userfieldType isEqualToString:@"color"]) {
                NXEColorOption *colorOption = [[NXEColorOption alloc] init];
                colorOption.key4colorField = [userField.userfieldID copy];
                colorOption.value4colorField = [userField.userfieldDefault copy];
                [colorOptions addObject:colorOption];
                [colorOption release];
            } /* else if([userField.userfieldType isEqualToString:@"selection"]) {
                NSMutableArray *tmpOptions = [[NSMutableArray alloc] init];
                for(UserFieldOption *fieldOption in userField.userfieldOptions) {
                    [tmpOptions addObject:fieldOption.optionValue];
                }
                _selectionOptionInClipEffect.key4selectionField = [userField.userfieldID copy];
                _selectionOptionInClipEffect.value4selectionField = [userField.userfieldDefault copy];
                _selectionOptionInClipEffect.menuItems = [tmpOptions copy];
                [tmpOptions release];
            }*/ // MJ 삭제
        }
        
        self.textOptions = [textOptions copy];
        self.colorOptions = [colorOptions copy];
        
        if([self.textOptions count] > 0 || [self.colorOptions count] > 0) {
            return YES;
        } else {
            return NO;
        }
    } else {
        return NO;
    }
}

- (BOOL)setupWithTransitionId:(NSString *)transitionId
{
    id<ConfigurableAssetItem> transitionEffect = (id<ConfigurableAssetItem>)[[AssetLibrary proxy] itemForId:transitionId];
    if(transitionEffect != nil && [transitionEffect conformsToProtocol:@protocol(ConfigurableAssetItem)]) {
        if([transitionEffect.userFields count] == 0) {
            return NO;
        }
        
        NSMutableArray *textOptions = [NSMutableArray array];
        NSMutableArray *colorOptions = [NSMutableArray array];
        
        for(UserField *userField in transitionEffect.userFields) {
            if([userField.userfieldType isEqualToString:@"text"]) {
                NXETextOption *textOption = [[NXETextOption alloc] init];
                textOption.maxLine = userField.userfieldMaxLines;
                textOption.key4textField = [userField.userfieldID copy];
                textOption.value4textField = [userField.userfieldDefault copy];
                [textOptions addObject:textOption];
            } else if([userField.userfieldType isEqualToString:@"color"]) {
                NXEColorOption *colorOption = [[NXEColorOption alloc] init];
                colorOption.key4colorField = [userField.userfieldID copy];
                colorOption.value4colorField = [userField.userfieldDefault copy];
                [colorOptions addObject:colorOption];
                [colorOption release];
            } /* else if([userField.userfieldType isEqualToString:@"selection"]) {
                NSMutableArray *tmpOptions = [[NSMutableArray alloc] init];
                for(UserFieldOption *fieldOption in userField.userfieldOptions) {
                    [tmpOptions addObject:fieldOption.optionValue];
                }
                _selectionOptionInTransitionEffect.key4selectionField = [userField.userfieldID copy];
                _selectionOptionInTransitionEffect.value4selectionField = [userField.userfieldDefault copy];
                _selectionOptionInTransitionEffect.menuItems = [tmpOptions copy];
                [tmpOptions release];
            } */ // MJ 삭제
        }
        
        self.textOptions = [textOptions copy];
        self.colorOptions = [colorOptions copy];
        
        if([self.textOptions count] > 0 || [self.colorOptions count] > 0) {
            return YES;
        } else {
            return NO;
        }
    } else {
        return NO;
    }
}

- (BOOL)setupWithEffectId:(NSString *) effectId type:(NXEEffectType) type
{
    BOOL result = NO;
    if ( type == NXE_CLIP_EFFECT ) {
        result = [self setupWithEffectId:effectId];
    } else if ( type == NXE_CLIP_TRANSITION ) {
        result = [self setupWithTransitionId:effectId];
    }
    self.effectType = type;
    return result;
}

@end
