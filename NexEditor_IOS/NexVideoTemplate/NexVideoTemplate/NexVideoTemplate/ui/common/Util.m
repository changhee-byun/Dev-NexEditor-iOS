/******************************************************************************
 * File Name   :	Util.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/
#import <UIKit/UIKit.h>
#import "Util.h"

@implementation Util

+ (NSString*)getCurrentTime
{
    NSDateFormatter* dataFormatter = [[NSDateFormatter alloc] init];
    [dataFormatter setDateFormat:@"yyyy_MM_dd_HH_mm_ss"];
    
    
    NSString* currentTime = [dataFormatter stringFromDate:[NSDate date]];
    NSLog(@"Current Time  %@", currentTime);
    return currentTime;
}

+ (NSString *)getStringTime:(int)timeMs
{
    int totalSeconds = timeMs / 1000;
    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours = totalSeconds / 3600;
    
    if(hours > 0) {
        return [NSString stringWithFormat:@"%02d:%02d:%02d", hours, minutes, seconds];
    } else {
        return [NSString stringWithFormat:@"%02d:%02d", minutes, seconds];
    }
}

+ (void)toastMessage:(UIViewController*)viewController title:(NSString*)title message:(NSString*)message duration:(int)duration
{
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:title
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    
    [viewController presentViewController:alert animated:YES completion:nil];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, duration * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        [alert dismissViewControllerAnimated:YES completion:nil];
    });
}

+ (void)removeAllExportFileInDocument
{
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSArray *directoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentsDirectory error:&error];
    if(error != nil) {
        return;
    }
    for(NSString *fileName in directoryContents) {
        if([fileName containsString:@"Export_"]) {
            [self removeFileInDocument:[documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"%@", fileName]]];
        }
    }
}

+ (void)removeFileInDocument:(NSString*)path
{
    NSError *error;
    BOOL success = [[NSFileManager defaultManager] removeItemAtPath:path error:&error];
    if(success) {
        
    } else {
        NSLog(@"could not delete file: %@", [error localizedDescription]);
    }
}

+ (UIImage *)imageFromVideoAsset:(AVAsset*)asset
{
    
    //  Get thumbnail at the very start of the video
    CMTime thumbnailTime = [asset duration];
    thumbnailTime.value = 25;
    
    //  Get image from the video at the given time
    AVAssetImageGenerator *imageGenerator = [[AVAssetImageGenerator alloc] initWithAsset:asset];
    imageGenerator.appliesPreferredTrackTransform = YES;
    imageGenerator.maximumSize = CGSizeMake(200, 200);
    
    CGImageRef imageRef = [imageGenerator copyCGImageAtTime:thumbnailTime actualTime:NULL error:NULL];
    UIImage *thumbnail = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    
    return thumbnail;
}

+ (NSString*)removeSubString:(NSString*)substring from:(NSString*)string
{
    NSString *result = nil;
    NSRange replaceRange = [string rangeOfString:substring];
    if (replaceRange.location != NSNotFound){
        result = [string stringByReplacingCharactersInRange:replaceRange withString:@""];
    }
    
    return result;
}

@end
