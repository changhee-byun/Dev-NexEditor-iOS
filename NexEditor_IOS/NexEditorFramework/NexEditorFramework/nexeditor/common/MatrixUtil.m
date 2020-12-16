/******************************************************************************
 * File Name   : MatrixUtil.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "MatrixUtil.h"
#import "ImageUtil.h"

@implementation MatrixUtil

+ (KMMatrix4)identity
{
    KMMatrix4 result = {
        1.0,    0.0,    0.0,    0.0,
        0.0,    1.0,    0.0,    0.0,
        0.0,    0.0,    1.0,    0.0,
        0.0,    0.0,    0.0,    1.0
    };
    return result;
}

+ (KMMatrix4)translateMatrixByX:(float)x Y:(float)y Z:(float)z {
    KMMatrix4 result = {
        1.0,    0.0,    0.0,    x,
        0.0,    1.0,    0.0,    y,
        0.0,    0.0,    1.0,    z,
        0.0,    0.0,    0.0,    1.0
    };
    return result;
}

+ (KMMatrix4)scaleMatrixByX:(float)x Y:(float)y Z:(float)z {
    KMMatrix4 result = {
        x,      0.0,    0.0,    0.0,
        0.0,    y,      0.0,    0.0,
        0.0,    0.0,    z,      0.0,
        0.0,    0.0,    0.0,    1.0
    };
    return result;
}

+ (KMMatrix4)multiplyMatrix4Source1:(KMMatrix4*)mat1 Source2:(KMMatrix4*)mat2 {
    KMMatrix4 result;
    float* dest = result.mat;
    
    float* src1 = mat1->mat;
    float* src2 = mat2->mat;
    
    dest[0] = src1[0] * src2[0] + src1[1] * src2[4] + src1[2] * src2[8] + src1[3] * src2[12];
    dest[1] = src1[0] * src2[1] + src1[1] * src2[5] + src1[2] * src2[9] + src1[3] * src2[13];
    dest[2] = src1[0] * src2[2] + src1[1] * src2[6] + src1[2] * src2[10] + src1[3] * src2[14];
    dest[3] = src1[0] * src2[3] + src1[1] * src2[7] + src1[2] * src2[11] + src1[3] * src2[15];
    
    dest[4] = src1[4] * src2[0] + src1[5] * src2[4] + src1[6] * src2[8] + src1[7] * src2[12];
    dest[5] = src1[4] * src2[1] + src1[5] * src2[5] + src1[6] * src2[9] + src1[7] * src2[13];
    dest[6] = src1[4] * src2[2] + src1[5] * src2[6] + src1[6] * src2[10] + src1[7] * src2[14];
    dest[7] = src1[4] * src2[3] + src1[5] * src2[7] + src1[6] * src2[11] + src1[7] * src2[15];
    
    dest[8] = src1[8] * src2[0] + src1[9] * src2[4] + src1[10] * src2[8] + src1[11] * src2[12];
    dest[9] = src1[8] * src2[1] + src1[9] * src2[5] + src1[10] * src2[9] + src1[11] * src2[13];
    dest[10] = src1[8] * src2[2] + src1[9] * src2[6] + src1[10] * src2[10] + src1[11] * src2[14];
    dest[11] = src1[8] * src2[3] + src1[9] * src2[7] + src1[10] * src2[11] + src1[11] * src2[15];
    
    dest[12] = src1[12] * src2[0] + src1[13] * src2[4] + src1[14] * src2[8] + src1[15] * src2[12];
    dest[13] = src1[12] * src2[1] + src1[13] * src2[5] + src1[14] * src2[9] + src1[15] * src2[13];
    dest[14] = src1[12] * src2[2] + src1[13] * src2[6] + src1[14] * src2[10] + src1[15] * src2[14];
    dest[15] = src1[12] * src2[3] + src1[13] * src2[7] + src1[14] * src2[11] + src1[15] * src2[15];
    
    return result;
}

+ (void)multiplyColorMatrixSource:(float[20])srcMatrix PreMatrix:(float[20])preMatrix {
    float tmp[20] = {0,};
    
    float* a = srcMatrix;
    float* b = preMatrix;
    
    int index = 0;
    for (int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            tmp[index++] =  a[j + 0] * b[i + 0] +  a[j + 1] * b[i + 5] +
                            a[j + 2] * b[i + 10] + a[j + 3] * b[i + 15];
        }
        tmp[index++] =  a[j + 0] * b[4] +  a[j + 1] * b[9] +
                        a[j + 2] * b[14] + a[j + 3] * b[19] +
                        a[j + 4];
    }
    
    memcpy(srcMatrix, tmp, sizeof(tmp));
}


+ (KMMatrix4) yuv2rgbWithBrightness:(float)brightness Contrast:(float)contrast Saturation:(float)saturation TintColor:(UIColor*)color
{
    KMMatrix4 unbiased_yuvToRGB = {
        1.164,  0.000,  1.596,  0.000,
        1.164, -0.392, -0.813,  0.000,
        1.164,  2.017,  0.000,  0.000,
        0.000,  0.000,  0.000,  1.000
    };
    
    KMMatrix4 brightnessAdjust = [self translateMatrixByX:brightness Y:0 Z:0];
    KMMatrix4 contrastAdjust = [self translateMatrixByX:0.5 Y:0.5 Z:0.5];
    
    float c1 = 1 + contrast;
    KMMatrix4 temp = [self scaleMatrixByX:c1 Y:c1 Z:c1];
    contrastAdjust = [self multiplyMatrix4Source1:&contrastAdjust Source2:&temp];
    
    temp = [self translateMatrixByX:-0.5 Y:-0.5 Z:-0.5];
    contrastAdjust = [self multiplyMatrix4Source1:&contrastAdjust Source2:&temp];
    
    float s1 = 1+saturation;
    KMMatrix4 satAdjust = [self scaleMatrixByX:1 Y:s1 Z:s1];
    
    KMMatrix4 bias = [self translateMatrixByX:-0.0625 Y:-0.5 Z:-0.5];
    
    CGFloat rc, gc, bc, alpha;
    [color getRed:&rc green:&gc blue:&bc alpha:&alpha];
    float r_weight = 0.241f;
    float g_weight = 0.691f;
    float b_weight = 0.068f;
    
    KMMatrix4 tintAdjust;
    if(saturation < 0) {
        KMMatrix4 temp = {
            rc*r_weight, rc*g_weight, rc*b_weight, 0.0,             // Red      output
            gc*r_weight, gc*g_weight, gc*b_weight, 0.0,             // Green    output
            bc*r_weight, bc*g_weight, bc*b_weight, 0.0,             // Blue     output
            0.0,         0.0,         0.0,         1.0
        };
        tintAdjust = temp;
        
        KMMatrix4 identity = [self identity];
        for(int i=0; i<16; i++) {
            tintAdjust.mat[i] = (tintAdjust.mat[i] * (1-s1)) + identity.mat[i] * s1;
        }
    }
    else
        tintAdjust = [self identity];
    
    KMMatrix4 sa_bias = [self multiplyMatrix4Source1:&satAdjust Source2:&bias];
    KMMatrix4 sa_bias_yuv = [self multiplyMatrix4Source1:&unbiased_yuvToRGB Source2:&sa_bias];
    
    
    KMMatrix4 result = [self multiplyMatrix4Source1:&sa_bias_yuv Source2:&brightnessAdjust];
    result = [self multiplyMatrix4Source1:&result Source2:&contrastAdjust];
    result = [self multiplyMatrix4Source1:&tintAdjust Source2:&result];
    return result;
}

+ (void)getColorMatrix:(float[20])colorMatrix WithBrightness:(float)brightness Contrast:(float)contrast Saturation:(float)saturation TintColor:(UIColor*)color;
{
    KMMatrix4 rgb_biasedyuv = {
        0.257,	0.504,	0.098,	0.0625,
        -0.148,	-0.291,	0.439,	0.500,
        0.439,	-0.368,	-0.071,	0.500,
        0.000,	0.000,	0.000,	1.000
    };
    
    KMMatrix4 yuv2rgb = [self yuv2rgbWithBrightness:brightness Contrast:contrast Saturation:saturation TintColor:color];
    KMMatrix4 colorAdjust = [self multiplyMatrix4Source1:&yuv2rgb Source2:&rgb_biasedyuv];
    
    float* e = colorAdjust.mat;
    float src[20] = {
        e[0],   e[1],   e[2],   0,  e[3],
        e[4], 	e[5], 	e[6], 	0,  e[7],
        e[8],	e[9],	e[10],	0,  e[11],
        e[12], 	e[13],	e[14], 	1,  0
    };
    
    memcpy(colorMatrix, src, sizeof(src));
}

+ (void)getColorMatrix:(float[20])colorMatrix WithBrightness:(float)brightness Contrast:(float)contrast Saturation:(float)saturation
{
    [self getColorMatrix:colorMatrix WithBrightness:brightness Contrast:contrast Saturation:saturation TintColor:RGBA(0, 0, 0, 0)];
}

@end
