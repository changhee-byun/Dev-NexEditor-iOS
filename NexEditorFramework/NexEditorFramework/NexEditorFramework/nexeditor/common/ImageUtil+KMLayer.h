/*
 * File Name   :  ImageUtil+KMLayer.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

#import "ImageUtil.h"
#import "KMTextLayer.h"

@interface ImageUtil (KMLayer)

+ (UIImage *)imageFromText:(NSString*)text withFont:(UIFont*)font withProperty:(KMTextLayerProperty*) property withSize:(CGSize)size;

/** 텍스트 정보로 OpenGL 텍스쳐를 생성하고 id 값을 반환한다.
 */
+(GLuint) glTextureIDfromText:(NSArray*)textArray withFont:(UIFont*)font withProperty:(KMTextLayerProperty*) property;

@end
