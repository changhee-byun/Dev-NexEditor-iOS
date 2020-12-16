/******************************************************************************
 * File Name   : MathUtil.h
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

/** 수학적 계산 기능을 모아놓은 유틸리티 클래스
 */
@interface MathUtil : NSObject

/** 0, 0 을 기준으로 해당 지점까지의 거리를 계산한다.
 피타고라스 정리를 이용하여 계산한다.
 */
+ (double)getDistanceFromWidth:(double)width Height:(double)height;

/** 0, 0 을 기준으로 해당 지점까지의 거리를 계산한다.
 */
+ (double)getDistanceP1:(CGPoint)p1;

/** 두 점 사이의 거리를 계산한다.
 */
+ (double)getDistanceP1:(CGPoint)p1 P2:(CGPoint)p2;

/** 원점을 기준으로 두 점 사이의 각도를 계산한다.
 */
+ (double)getAngleP1:(CGPoint)p1 P2:(CGPoint)p2;

/** 라디안(2PI) 형식의 각도를 디그리(360) 형식의 각도로 변환한다.
 */
+ (double)toDegrees:(double)radian;

/** 디그리(360) 형식의 각도를 라디안(2PI) 형식의 각도로 변환한다.
 */
+ (double)toRadians:(double)degrees;

//0, 0 기준으로 해당 포인트에서 특정 각도값으로 회전된 포인트를 계산한다.
+ (CGPoint)getRotatedPoint:(double)radius Radian:(double)radian;

/** 두 직선이 교차하는 점을 반환한다.
 직선1의 두 점 p1, p2
 직선2의 두 점 p3, p4
 */
+ (CGPoint)getIntersectionWithP1:(CGPoint)p1 P2:(CGPoint)p2 P3:(CGPoint)p3 P4:(CGPoint)p4;
@end
