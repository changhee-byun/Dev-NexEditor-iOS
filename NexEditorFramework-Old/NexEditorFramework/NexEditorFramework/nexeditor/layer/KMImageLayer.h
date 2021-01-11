/******************************************************************************
 * File Name   : KMImageLayer.h
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
/** 아이콘, 스티커 같이 이미지를 레이어로 표현한다.
 리소스, 파일, UIImage 를 활용하여 만들 수 있다.
 */
@interface KMImageLayer : KMLayer<ChromaKeyLayer>

/** 리소스 이름으로 레이어를 만든다.
 @param name - 리소스 이름.(without extension)
 @return KMImageLayer 객체
 */
- (id)initWithResourceName:(NSString*)name;

/** local 파일의 경로 정보로 레이어를 만든다.
 @param path - 파일의 경로.
 @return KMImageLayer 객체
 */
- (id)initWithLocalPath:(NSString*)path;

/** UIImage 객체로 레이어를 만든다.
 @param image - UIImage 객체.
 @return KMImageLayer 객체
 */
- (id)initWithImage:(UIImage*)image;

/** UIColor 객체로 레이어를 만든다.
 @param color - UIColor 객체.
 @return KMImageLayer 객체
 */
- (id)initWithColor:(UIColor*)color;

@end
