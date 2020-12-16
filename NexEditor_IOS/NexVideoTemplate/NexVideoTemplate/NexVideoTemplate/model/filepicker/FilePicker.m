/******************************************************************************
 * File Name   :	FilePicker.m
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
@import Photos;
#import "FilePicker.h"
#import "Util.h"

@implementation ListItem
@end

@interface FilePicker()

/* for saving video content from ph picker */
@property (nonatomic, retain) NSMutableArray *videoList;

@end

@implementation FilePicker

- (instancetype)init
{
    self = [super init];
    if(self) {
        _videoList = [[NSMutableArray alloc] init];
    }
    return self;
}


- (NSMutableArray*)getVideoList
{
    return self.videoList;
}

- (void)updateListFromPHAsset
{
    PHFetchResult *collections = [PHAssetCollection fetchAssetCollectionsWithType:PHAssetCollectionTypeSmartAlbum subtype:PHAssetCollectionSubtypeAny options:nil];
    
    PHAssetCollection *videoCollection = nil;
//    PHAssetCollection *screenshotsCollection = nil;
//    PHAssetCollection *camerarollCollection = nil;
    
    for(PHAssetCollection *collection in collections) {
        if([collection.localizedTitle isEqualToString:@"Videos"])
            videoCollection = collection;
//        else if([collection.localizedTitle isEqualToString:@"Camera Roll"])
//            camerarollCollection = collection;
//        else if([collection.localizedTitle isEqualToString:@"Screenshots"])
//            screenshotsCollection = collection;
    }
    
    if(!videoCollection)
        return ;
    
//    if(!screenshotsCollection)
//        return ;
    
//    if(!camerarollCollection)
//        return ;
    
    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:videoCollection options:nil]];
//    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:videoCollection options:nil] count:30];
//    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:screenshotsCollection options:nil]];
//    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:camerarollCollection options:nil]];
}

- (void)setItemsWithAssetCollection:(PHFetchResult*)result
{
    for(PHAsset *asset in result) {
        ListItem *listItem = [[ListItem alloc] init];
        
        if(asset.mediaType == PHAssetMediaTypeVideo) {
            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
            PHVideoRequestOptions *option = [PHVideoRequestOptions new];
            [[PHImageManager defaultManager] requestAVAssetForVideo:asset options:option resultHandler:^(AVAsset * ass, AVAudioMix * audioMix, NSDictionary * info) {
                if([ass isKindOfClass:[AVURLAsset class]]) {
                    listItem.avURLAsset = (AVURLAsset*)ass;
                    listItem.duration = (int)ass.duration.value / (int)ass.duration.timescale * 1000;
                    listItem.isChecked = false;
                }

                dispatch_semaphore_signal(semaphore);
            }];
            
            dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);  //작업 종료되기까지 무기한 대기
        }
        
        if([self isSupportedVideoType:[listItem.avURLAsset.URL.path pathExtension]] && [self containsItem:listItem.avURLAsset.URL.path inList:self.videoList] == false) {
            [self.videoList addObject:listItem];
        }
    }
}

- (void)removeAllExportFileInDocument
{
    [Util removeAllExportFileInDocument];
}

- (BOOL)isSupportedVideoType:(NSString *)fileExtension
{
    NSArray *supportedVideoExtensions = @[@"mp4", @"3gp", @"mov", @"m4v"];
    
    if([supportedVideoExtensions containsObject:[fileExtension lowercaseString]]) {
        return TRUE;
    }
    
    return FALSE;
}

- (BOOL)containsItem:(NSString*)path inList:(NSMutableArray*)array
{
   
    for(int i = 0; i < [array count]; i++) {
        if([path isEqualToString:((ListItem*)[array objectAtIndex:i]).avURLAsset.URL.path]) {
            return TRUE;
        }
    }
    return FALSE;
}




@end
