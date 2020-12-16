/******************************************************************************
 * File Name   : MathUtil.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "MathUtil.h"

@implementation MathUtil

+ (double)getDistanceFromWidth:(double)width Height:(double)height {
    return sqrt(pow(width, 2) + pow(height, 2));
}

+ (double)getDistanceP1:(CGPoint)p1 {
    return [self getDistanceFromWidth:p1.x Height:p1.y];
}

+ (double)getDistanceP1:(CGPoint)p1 P2:(CGPoint)p2 {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

+ (double)getAngleP1:(CGPoint)p1 P2:(CGPoint)p2
{
    double theta = atan2(p2.y - p1.y, p2.x - p1.x);
    double angle = [self toDegrees:(theta + M_PI / 2)];
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}

+ (double)toDegrees:(double)radian {
    return radian * 180.0 / M_PI;
}

+ (double)toRadians:(double)degrees {
    return degrees / 180.0 * M_PI;
}

+ (CGPoint)getRotatedPoint:(double)radius Radian:(double)radian {
    double x = radius * sin(radian);
    double y = radius * cos(radian);
    return CGPointMake(x, y);
}

+ (CGPoint)getIntersectionWithP1:(CGPoint)p1 P2:(CGPoint)p2 P3:(CGPoint)p3 P4:(CGPoint)p4 {
    float increase1=0, constant1=0, sameValue1=0;
    float increase2=0, constant2=0, sameValue2=0;
    
    //직선1 계산
    if(p1.x == p2.x) {      //y축과 평행한 경우
        sameValue1 = p1.x;
    }
    else {
        increase1 = (p2.y-p1.y)/(p2.x-p1.x);    //기울기 계산
        constant1 = p1.y - increase1 * p1.x;    //상수 계산
    }
    
    //직선2 계산
    if(p3.x == p4.x) {      //y축과 평행한 경우
        sameValue2 = p3.x;
    }
    else {
        increase2 = (p4.y-p3.y)/(p4.x-p3.x);    //기울기 계산
        constant2 = p3.y - increase2 * p3.x;    //상수 계산
    }
    
    //교차점
    if(p1.x == p2.x && p3.x == p4.x) return CGPointMake(FLT_MIN, FLT_MIN);  //평행한 경우. 교차점 없음.
    
    if(p1.x == p2.x)
        return CGPointMake(sameValue1, increase2 * sameValue1 + constant2); //직선1이 y축에 평행할 경우
    else if(p3.x == p4.x)
        return CGPointMake(sameValue2, increase1 * sameValue2 + constant1); //직선2가 y축에 평행할 경우
    else {
        float x = -(constant1-constant2) / (increase1-increase2);           //x좌표
        float y = increase1 * x + constant1;                                //y좌표
        return CGPointMake(x, y);
    }
}
@end
