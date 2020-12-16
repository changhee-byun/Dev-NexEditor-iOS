/******************************************************************************
 * File Name   : KMVideoLayer.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMLayer.h"

@class NXEVisualClip;

/** 비디오 파일로 부터 레이어를 생성한다. 
 */
@interface KMVideoLayer : KMLayer<ChromaKeyLayer>

@property (nonatomic, assign) int layerIndex;

/** 클립 id로 레이어를 만든다.
 @return KMVideoLayer 객체
 */
- (instancetype)initWithWidth:(int)width height:(int)height;

@end
