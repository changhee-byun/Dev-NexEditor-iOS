/******************************************************************************
 * File Name   : NexEditorUtil.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NexEditorUtil.h"
#import "NEXVIDEOEDITOR_BaseRect.h"
#import "KenBurnsRectanglesGenerator.h"
#import "CapacityChecker.h"


#import <stdlib.h>
#import <string.h>
#import <stdio.h>
#import <unistd.h>


#define LOG_TAG @"NexEditorUtil"

@implementation NexEditorUtil

+ (KenburnsRects) getKenBurnsRectsInSize:(CGSize)resolution withTargetRatio:(CGSize)ratio duration:(int)duration faceRects:(NSArray <NSValue*>*)faceRects
{
    int iArrayCount = (int)faceRects.count;
    int iAspectWidth = ratio.width;
    int iAspectHeight = ratio.height;
    int iDuration = duration;

    RECT *prFace = NULL;
    if (faceRects != nil && faceRects.count > 0) {
        prFace = new RECT[iArrayCount];
        for (int index = 0; index < iArrayCount; index++) {
            NSValue *faceRect = faceRects[index];
            CGRect rect = faceRect.CGRectValue;
            prFace[index].left = rect.origin.x;
            prFace[index].top = rect.origin.y;
            prFace[index].right = rect.size.width+rect.origin.x;
            prFace[index].bottom = rect.size.height+rect.origin.y;
        }
    }

    RECT src = RECT{0, 0, (int)resolution.width, (int)resolution.height};
    RECT output[2];
    
    KenBurnsRectanglesGenerator *generartor = new KenBurnsRectanglesGenerator();
    generartor->generate(&src, prFace, iArrayCount, iAspectWidth, iAspectHeight, iDuration, output);
    
    KenburnsRects result = {
        CGRectMake(output[0].left, output[0].top, output[0].right-output[0].left, output[0].bottom-output[0].top),
        CGRectMake(output[1].left, output[1].top, output[1].right-output[1].left, output[1].bottom-output[1].top),
    };
    
    delete []prFace;
    delete generartor;
    
    return result;
}

+ (int) checkAvailabilityOfHardwareResourceWithIdentifier:(NSString*)identifier videoInfos:(NSArray*)videoInfo
{
    int iVideoinfoCount = (int)videoInfo.count;
    VideoInfo* info = new VideoInfo[iVideoinfoCount];
    
    for (int i=0; i<videoInfo.count; i++) {
        NSValue* value = videoInfo[i];
        VideoInfo vInfo;
        [value getValue:&vInfo];
        info[i] = vInfo;
    }
    int result = CapacityChecker::isAvailableMemoryCapacity((char*)[identifier UTF8String], (int)videoInfo.count, info);

    delete []info;
    return result;
}

+ (CapacityInfo) getAvailabilityOfHardwareResourceWithIdentifier:(NSString*)identifier
{
	CapacityInfo sInfo;
	memset(&sInfo, 0x00, sizeof(CapacityInfo));
	CapacityChecker::getAvailableMemoryCapacity((char*)[identifier UTF8String], &sInfo.uMaxMemorySize, &sInfo.uMaxDecoderCount);
	return sInfo;
}

@end
