/******************************************************************************
 * File Name   : MatrixUtil.h
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
#import <UIKit/UIKit.h>
#import "KMLayer.h"

typedef struct {
    float mat[16];
} KMMatrix4;

@interface MatrixUtil : KMLayer

/** 단위행렬을 반환한다.
    1 0 0 0
    0 1 0 0
    0 0 1 0
    0 0 0 1
 */
+ (KMMatrix4)identity;

/** 단위 행렬에서 오른쪽 열 값을 x, y, z로 대입한 행렬을 반환한다.
    1 0 0 x
    0 1 0 y
    0 0 1 z
    0 0 0 1
 */
+ (KMMatrix4)translateMatrixByX:(float)x Y:(float)y Z:(float)z;

/** 단위 행렬에서 행과 열이 같은 요소의 값을 x, y, z로 대체 한다.
    x 0 0 0
    0 y 0 0
    0 0 z 0
    0 0 0 1
 */
+ (KMMatrix4)scaleMatrixByX:(float)x Y:(float)y Z:(float)z;

/** 두개의 행렬을 곱한다.
    1 0 0 0     1 0 0 0
    0 1 0 0  x  0 1 0 0
    0 0 1 0     0 0 1 0
    0 0 0 1     0 0 0 1
 */
+ (KMMatrix4)multiplyMatrix4Source1:(KMMatrix4*)mat1 Source2:(KMMatrix4*)mat2;

/** 안드로이드 키네마스터에서 사용하는 4*5 형태의 Color Matrix를 계산한다.
 */
+ (void)getColorMatrix:(float[20])colorMatrix WithBrightness:(float)brightness Contrast:(float)contrast Saturation:(float)saturation;

/** 안드로이드 키네마스터에서 사용하는 4*5 형태의 Color Matrix를 계산한다.
 */
+ (void)getColorMatrix:(float[20])colorMatrix WithBrightness:(float)brightness Contrast:(float)contrast Saturation:(float)saturation TintColor:(UIColor*)color;
@end
