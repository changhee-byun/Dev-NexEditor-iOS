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

#import "EditorUtil.h"
#import "ImageUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"EditorUtil"
#define SIMULATOR_DUMB_STILLCAPTURE_WIDTH   4096
#define SIMULATOR_DUMB_STILLCAPTURE_HEIGHT   4096

@implementation EditorUtil

+ (bool)isSupportedImage:(NSString*) path
{
    bool ret = false;
    
    NSArray* supportedAssetPrefix = @[@"phasset-image://"];
    
    NSArray* supportedImageExtensions = @[@"ext=jpg", @"ext=jpeg", @"ext=png", @"ext=bmp", @"ext=gif", @".jpg", @".jpeg", @".png", @".bmp", @".gif"/*, @".webp"*/];
    
    if([path isKindOfClass:[NSURL class]]) {
        path = [(NSURL*)path absoluteString];
    }
    
    NSString* filepath = [path lowercaseString];
    for(NSString *prefix in supportedAssetPrefix) {
        if([filepath hasPrefix:prefix]) {
            ret = true;
        }
    } 
    
    for(NSString* extension in supportedImageExtensions) {
        if([filepath hasSuffix:extension]) {
            ret = true;
        }
    }

    return ret;
}

+ (CMVideoDimensions) getCameraMaxStillImageResolution
{
    CMVideoDimensions max_resolution;
    max_resolution.width = 0;
    max_resolution.height = 0;
    
    AVCaptureDevice *captureDevice = nil;
    
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == AVCaptureDevicePositionBack) {
            captureDevice = device;
            break;
        }
    }
    
    if (captureDevice == nil) {
        return max_resolution;
    }
    
    NSArray* availFormats = captureDevice.formats;
    
    for (AVCaptureDeviceFormat* format in availFormats) {
        CMVideoDimensions resolution = format.highResolutionStillImageDimensions;
        int w = resolution.width;
        int h = resolution.height;
        if ((w * h) > (max_resolution.width * max_resolution.height)) {
            max_resolution.width = w;
            max_resolution.height = h;
        }
    }
    
    return max_resolution;
}



+ (bool)isIPodMusic:(NSString*)path
{
    NSArray *supportedIPodPrefix = @[@"ipod-library://"];
    
    NSString *filepath = [path lowercaseString];
    for(NSString *prefix in supportedIPodPrefix) {
        if([filepath hasPrefix:prefix]) {
            return true;
        }
    }
    
    return false;
}

+ (bool)isVideo:(NSString*) path
{
    NSArray* supportedVideoExtensions = @[@".mp4", @".3gp", @".3gpp", @".mov", @".k3g", @".acc", @".avi", @".wmv"];
    
    NSString* filepath = [path lowercaseString];
    for(NSString* extension in supportedVideoExtensions) {
        if([filepath hasSuffix:extension]) {
            return true;
        }
    }
    
    return false;
}

+ (NSString*)removeSubString:(NSString*)substring from:(NSString*)string
{
    NSString *result = string;
    NSRange replaceRange = [string rangeOfString:substring];
    if (replaceRange.location != NSNotFound) {
        result = [string stringByReplacingCharactersInRange:replaceRange withString:@""];
    }
    
    return result;
}

+ (NSData *)dataWithContentsOfStream:(NSInputStream *)stream
{
    uint8_t buf[8192];
    
    NSMutableData* result = [NSMutableData data];
    
    [stream open];
    
    while(1) {
        if([stream hasBytesAvailable]) {
            NSInteger n = [stream read:buf maxLength:8192];
            if(n<0) {
                result = nil;
                break;
            } else if(n==0) {
                break;
            } else {
                [result appendBytes:buf length:n];
            }
        } else {
            break;
        }
    }
    
    [stream close];
    return result;
}

+ (void)encryptEffect:(NSString *)effect encryptResult:(NSMutableString *)destination
{
    [self executeEncrypt:[NSString stringWithFormat:@"%@%@", @"com.nexstreaming.kinemaster.builtin.watermark.", effect]
           encryptResult:destination];
}

+ (void)executeEncrypt:(NSString *)effect encryptResult:(NSMutableString *)destination
{
    if(effect == nil) {
        return;
    }
    
    int offset_a = 0, offset_b = 0;
    int mask_a = 0, mask_b = 0;
    char encryptChar_a, encryptChar_b, tmpChar;
    
    NSString *mask = @"Ax/VXn_zsAiwFi[CITPC;y2c}*0B'S0-7&QznQlMa6U9gmSoighZeC&@$-hAaXiN";
    
    for(int i=0; i<8; i++) {
        encryptChar_a = (char)((arc4random() & 0xFFFFFFFF) % 94 + ' ');
        if(i == 0) {
            offset_a = encryptChar_a ^ 0x3F;
        } else if(i == 1) {
            offset_b = encryptChar_a ^ 0x81;
        }
        [destination appendString:[NSString stringWithFormat:@"%c", encryptChar_a]];
    }
    
    for(int j=0; j<[effect length]; j++) {
        mask_a = [mask characterAtIndex:((offset_a + j) % 64)];
        mask_b = [destination characterAtIndex:((offset_b + j) % 8)];
        tmpChar = [effect characterAtIndex:j];
        encryptChar_b = (char)((tmpChar - 32 + mask_a - mask_b + 188) % 94 + 32);
        [destination appendString:[NSString stringWithFormat:@"%c", encryptChar_b]];
    }
}

@end

