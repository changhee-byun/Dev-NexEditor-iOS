/**
 * File Name   : NXEClip+Extensions.m
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

#import "NXEClip+Extensions.h"

@implementation NXEClip (Throws)

/// If error occured, error with be set and an empty array will be returned
+ (NSArray<NXEClip *> *) clipsWithSources: (NSArray<NXEClipSource *> *) clipSources error:(NSError **) error
{
    NSMutableArray *clips = [NSMutableArray array];
    for ( NXEClipSource *clipSource in clipSources ) {
        NXEClip *clip = [NXEClip clipWithSource:clipSource error:error];
        if ( clip ) {
            [clips addObject: clip];
        } else {
            break;
        }
    }
    
    if (clips.count == 0 && error && *error == nil) {
        *error = [[NXEError alloc] initWithErrorCode:ERROR_INVALID_INFO];
    }
    return [clips copy];
}

@end

@implementation NXEClip (DisplayName)

+ (NSString *)pathFromPHAssetURLString:(NSString *)urlString
{
    __block NSString *filePath = nil;
    
    NSString *filePath_ = nil;
    NSRange range = [urlString rangeOfString:@"phasset-image://"];
    if(range.location != NSNotFound) {
        filePath_ = [urlString stringByReplacingCharactersInRange:range withString:@""];
    }
    PHFetchResult *fetchResult = [PHAsset fetchAssetsWithLocalIdentifiers:@[filePath_] options:nil];
    PHAsset *asset = fetchResult.firstObject;
    
    PHImageRequestOptions * imageRequestOptions = [[PHImageRequestOptions alloc] init];
    imageRequestOptions.synchronous = YES;
    [[PHImageManager defaultManager] requestImageDataForAsset:asset
                                                      options:imageRequestOptions
                                                resultHandler:^(NSData *imageData, NSString *dataUTI, UIImageOrientation orientation, NSDictionary *info) {
                                                    filePath = [[info objectForKey:@"PHImageFileURLKey"] absoluteString];
                                                }];
    imageRequestOptions = nil;
    return filePath;
}

- (NSString *) displayName
{
    return self.clipSource.shortDisplayText;
}
@end
