/******************************************************************************
 * File Name   : EditorUtil.h
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
#import <CoreMedia/CoreMedia.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreText/CoreText.h>
#import <UIKit/UIKit.h>

#import "NXEEditorType.h"

#define limit(value, min, max) (value <= min ? min : (value >= max ? max : value))

@interface EditorUtil : NSObject

/**
 */
+ (bool)isSupportedImage:(NSString* _Nonnull) path;
/**
 */
+ (bool)isVideo:(NSString* _Nonnull) path;

+ (bool)isIPodMusic:(NSString* _Nonnull)path;

+ (NSString* _Nullable)removeSubString:(NSString* _Nullable)substring from:(NSString* _Nullable)string;
/**
 */
+ (NSData * _Nullable)dataWithContentsOfStream:(NSInputStream * _Nonnull)input;

/**
 */
+ (void)encryptEffect:(NSString * _Nonnull)effect
           encryptResult:(NSMutableString * _Nonnull)destination;

@end

