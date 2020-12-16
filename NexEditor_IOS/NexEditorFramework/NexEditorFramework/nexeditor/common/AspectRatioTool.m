/**
 * File Name   : AspectRatioTool.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "AspectRatioTool.h"

@implementation AspectRatioTool
+ (NXESizeInt) ratioWithType: (NXEAspectType) type
{
    static const NXESizeInt sizes[NXEAspectTypeCustom] = {
        { 16,  9 },
        {  9, 16 },
        {  1,  1 },
        {  1,  2 },
        {  2,  1 },
    };
    
    NXESizeInt result = NXESizeIntMake(0, 0);
    if ( type != NXEAspectTypeCustom ) {
        result = sizes[type];
    } // else, not valid
    return result;
}

+ (CGRect) rectFills:(CGRect) rect ratio:(float) ratio fit:(BOOL) fit
{
    float dratio = ratio;
    float sratio = rect.size.width / rect.size.height;
    
    CGRect roi = rect;
    BOOL horz = (sratio >= dratio);
    if (fit) {
        horz = !horz;
    }
    
    if (horz) {
        roi.size.width = floor(roi.size.height * dratio);
        roi.origin.x = floor((rect.size.width - roi.size.width) / 2);
    } else {
        roi.size.height = floor(roi.size.width / dratio);
        roi.origin.y = floor((rect.size.height - roi.size.height) / 2);
    }
    return roi;
}

+ (CGRect) rectFills:(CGRect) rect inSize:(CGSize) size fit:(BOOL) fit
{
    float dratio = size.width / size.height;
    return [self rectFills:rect ratio:dratio fit:fit];
}

+ (CGRect) aspectFit:(CGSize) size ratiof:(float) ratio
{
    CGRect result = CGRectMake(0, 0, size.width, size.height);
    float windowRatio = size.width / size.height;
    
    if (windowRatio < ratio) {
        result.size.height = floor(size.width / ratio);
        result.origin.y = floor((size.height - result.size.height) / 2);
    } else {
        result.size.width = floor(size.height * ratio);
        result.origin.x = floor( (size.width - result.size.width) / 2);
    }
    return result;
}

+ (CGRect) aspectFit:(CGSize) size ratio:(NXESizeInt) ratioi
{
    float ratio = ((float)ratioi.width) / ((float)ratioi.height);
    return [self aspectFit:size ratiof:ratio];
}

+ (NXESizeInt) frameSizeWithRatio:(NXESizeInt) ratioi minSize:(int32_t) minSize
{
    NXESizeInt frameSize;
    
    if (ratioi.width == 0 || ratioi.height == 0 || minSize == 0) {
        frameSize.width = 0;
        frameSize.height = 0;
        return frameSize;
    }
    
    float ratio = ((float)ratioi.width) / ((float) ratioi.height);
    frameSize.width = floor(ratio * minSize);
    frameSize.height = minSize;
    
    // promote the shorter side to become minSize
    if ( frameSize.width < frameSize.height ) {
        frameSize.height = floor( frameSize.height * ((float)minSize) / ((float)frameSize.width));
        frameSize.width = minSize;
    }
    return frameSize;
}

@end

NXESizeInt NXESizeIntMake(int32_t width, int32_t height)
{
    NXESizeInt result = { width, height };
    return result;
}
