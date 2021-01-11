/******************************************************************************
 * File Name   : UserField.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@interface UserFieldOption : NSObject <NSCopying>

@property (nonatomic, retain) NSString * _Nullable optionValue;
@property (nonatomic, retain) NSString * _Nullable optionIcon;
//@property (nonatomic, retain) NSMutableArray * _Nullable optionLabels;

@end

@interface UserField : NSObject <NSCopying>

@property (nonatomic, assign) int userfieldMaxLines;
@property (nonatomic, copy) NSString * _Nullable userfieldType;
@property (nonatomic, copy) NSString * _Nullable userfieldDefault;
@property (nonatomic, copy) NSString * _Nullable userfieldEditor;
@property (nonatomic, copy) NSString * _Nullable userfieldID;
@property (nonatomic, retain) NSMutableArray * _Nullable userfieldOptions;
//@property (nonatomic, retain) UserFieldOption * _Nullable userfieldOption;

/**
 */
- (instancetype _Nullable)initUserFieldWithAttributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict;
/**
 */
- (void)setFieldOption:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict;
/**
 */
//- (void)processFieldLabel:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict;

@end
