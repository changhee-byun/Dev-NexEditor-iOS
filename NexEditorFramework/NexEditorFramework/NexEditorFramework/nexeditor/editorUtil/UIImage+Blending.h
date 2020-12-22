/******************************************************************************
 * File Name   : UIImage+Blending.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <UIKit/UIKit.h>

@interface UIImage (Blending)

- (UIImage *)blendingImageWithColor:(UIColor *)tintColor blendingMode:(CGBlendMode)blendMode;

@end
