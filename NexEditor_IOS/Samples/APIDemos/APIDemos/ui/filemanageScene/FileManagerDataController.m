/******************************************************************************
 * File Name   :	FileManagerDataController.m
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

#import "FileManagerDataController.h"
#import "Util.h"

@implementation ClipMediaItem

- (id)init
{
    return self;
}

- (void)dealloc
{
    self.phAsset = nil;
    self.filePath = nil;
}
@end

@interface FileManagerDataController ()

@property(nonatomic, retain) NSMutableArray *videoList;
@property(nonatomic, retain) NSMutableArray *audioList;
@property(nonatomic, retain) NSMutableArray *imageList;

@property(nonatomic, assign) BOOL needVideo;
@property(nonatomic, assign) BOOL needImage;
@property(nonatomic, assign) BOOL needAudio;

@end

@implementation FileManagerDataController

- (instancetype)initWithVideo:(BOOL)needVideo Image:(BOOL)needImage Audio:(BOOL)needAudio
{
    self = [super init];
    if(self) {
        _videoList = [[NSMutableArray alloc] init];
        _imageList = [[NSMutableArray alloc] init];
        _audioList = [[NSMutableArray alloc] init];
        
        _needVideo = needVideo;
        _needImage = needImage;
        _needAudio = needAudio;
        
        [self setListFromBundle];
        [self setListFromDocument];
        [self setListFromPHAsset];
        
        return self;
    }
    return nil;
}

- (void)dealloc
{
    _videoList = nil;
    _imageList = nil;
    _audioList = nil;

}
- (bool)isSupportedAudioTypes:(NSString *)fileExtension
{
    NSArray *supportedAudioExtensions = @[@"aac", @"mp3"];
    
    if([supportedAudioExtensions containsObject:[fileExtension lowercaseString]]) {
        return true;
    }
    
    return false;
}

- (bool)isSupportedImageType:(NSString *)fileExtension
{
    NSArray *supportedImageExtensions = @[@"jpg", @"jpeg", @"png", @"bmp", @"gif", @"JPG", @"JPEG", @"PNG", @"BMP", @"GIF"];
    
    for(int i = 0; i < [supportedImageExtensions count]; i++) {
        NSString *extension = [supportedImageExtensions objectAtIndex:i];
        if([fileExtension containsString:extension]) {
            return true;
        }
    }
    
    return false;
}

- (bool)isSupportedVideoType:(NSString *)fileExtension
{
    NSArray *supportedVideoExtensions = @[@"mp4", @"3gp", @"mov", @"m4v"];
    
    if([supportedVideoExtensions containsObject:[fileExtension lowercaseString]]) {
        return true;
    }
    
    return false;
}

- (BOOL)isDuplicateVideoPath:(NSString *)path
{
    for(ClipMediaItem *listItem in self.videoList) {
        if([[path lastPathComponent] isEqualToString:[listItem.filePath lastPathComponent]]) {
            return true;
        }
    }
    return false;
}

- (void)setItemsWithAssetCollection:(PHFetchResult*)result
{
    for(PHAsset *asset in result) {
        
        if(asset.mediaType == PHAssetMediaTypeVideo) {
            if (self.needVideo) {
                ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
                listItem.phAsset = asset;
                listItem.type = ClipMediaTypeVideo;
                __block BOOL duplicate = NO;
                [self.videoList enumerateObjectsUsingBlock:^(ClipMediaItem * _Nonnull item, NSUInteger idx, BOOL * _Nonnull stop) {
                    if ([item.phAsset.localIdentifier isEqualToString:asset.localIdentifier]) {
                        duplicate = YES;
                        *stop = YES;
                    }
                }];
                if (!duplicate) {
                    [self.videoList addObject:listItem];
                }
            }
        } else if(asset.mediaType == PHAssetMediaTypeImage) {
            if(self.needImage) {
                ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
                listItem.phAsset = asset;
                listItem.type = ClipMediaTypeImage;
                [self.imageList addObject:listItem];
            }
        }
    }
}

- (void)setListFromPHAsset
{
    PHFetchResult *collections = [PHAssetCollection fetchAssetCollectionsWithType:PHAssetCollectionTypeSmartAlbum
                                                                          subtype:PHAssetCollectionSubtypeAny
                                                                          options:nil];
    PHAssetCollection *videoCollection = nil;
    PHAssetCollection *camerarollCollection = nil;
    
    for(PHAssetCollection *collection in collections) {
        if([collection.localizedTitle isEqualToString:@"Videos"]) {
            videoCollection = collection;
        } else if([collection.localizedTitle isEqualToString:@"Camera Roll"]
                  || [collection.localizedTitle isEqualToString:@"All Photos"]) {
            camerarollCollection = collection;
        }
    }
    
    if(!videoCollection || !camerarollCollection) {
        return ;
    }

    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:videoCollection options:nil]];
    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:camerarollCollection options:nil]];
//    [self setItemsWithAssetCollection:[PHAsset fetchAssetsInAssetCollection:screenshotsCollection options:nil]];
}

- (void)setListFromBundle
{
    NSString *fileName = nil;
    NSString *filePath = nil;
    
    // video add
    if(self.needVideo) {
        NSDirectoryEnumerator *enumerator_video = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:BUNDLE_CLIPSOURCES_VIDEO]];
        
        while((fileName = [enumerator_video nextObject]) != nil) {
            if([self isSupportedVideoType:[fileName pathExtension]] && [self isDuplicateVideoPath:fileName] == false) {
                filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                           ofType:[fileName pathExtension]
                                                      inDirectory:BUNDLE_CLIPSOURCES_VIDEO];
                
                ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
                listItem.type = ClipMediaTypeVideo;
                listItem.filePath = filePath;
                
                [self.videoList addObject:listItem];
				listItem = nil;
            }
        }
		enumerator_video = nil;
    }
    
    // audio add
    if(self.needAudio) {
        NSDirectoryEnumerator *enumerator_audio = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:BUNDLE_CLIPSOURCES_AUDIO]];
        
        while((fileName = [enumerator_audio nextObject]) != nil) {
            if([self isSupportedAudioTypes:[fileName pathExtension]]) {
                filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                           ofType:[fileName pathExtension]
                                                      inDirectory:BUNDLE_CLIPSOURCES_AUDIO];
                
                ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
                listItem.type = ClipMediaTypeAudio;
                listItem.filePath = filePath;
                
                [self.audioList addObject:listItem];
				listItem = nil;
            }
        }
		enumerator_audio = nil;
    }
    
    // image add
    if(self.needImage) {
        NSDirectoryEnumerator *enumerator_image = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:BUNDLE_CLIPSOURCES_IMAGE]];
        
        while((fileName = [enumerator_image nextObject]) != nil) {
            if([self isSupportedImageType:[fileName pathExtension]]) {
                filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                           ofType:[fileName pathExtension]
                                                      inDirectory:BUNDLE_CLIPSOURCES_IMAGE];
                ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
                listItem.type = ClipMediaTypeImage;
                listItem.filePath = filePath;
                
                [self.imageList addObject:listItem];
				listItem = nil;
            }
        }
		enumerator_image = nil;
    }
}

- (void)setListFromDocument
{
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSArray *directoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentsDirectory error:&error];
    if(error != nil) {
        return;
    }
    
    for(NSString *fileName in directoryContents) {
        NSMutableArray *list = nil;
        ClipMediaType type = ClipMediaTypeVideo;
        NSString *ext = [fileName pathExtension];

        if([self isSupportedVideoType:ext] && [self isDuplicateVideoPath:fileName] == false) {
            if(self.needVideo) {
                type = ClipMediaTypeVideo;
                list = self.videoList;
            }
        } else if([self isSupportedImageType:ext]) {
            if(self.needImage) {
                type = ClipMediaTypeImage;
                list = self.imageList;
            }
        } else if([self isSupportedAudioTypes:ext]) {
            if(self.needAudio) {
                type = ClipMediaTypeAudio;
                list = self.audioList;
            }
        }
        
        if (list) {
            NSString *path = [documentsDirectory stringByAppendingFormat:@"%@%@", @"/", fileName];
            ClipMediaItem *listItem = [[ClipMediaItem alloc] init];
            listItem.type = type;
            listItem.filePath = path;
            
            [list addObject:listItem];
        }
    }
}

- (NSArray<ClipMediaItem *> *)itemsOfType:(ClipMediaType)type
{
    NSArray<ClipMediaItem *> *result = nil;
    switch(type) {
        case ClipMediaTypeVideo:
            result = self.videoList;
            break;
        case ClipMediaTypeImage:
            result = self.imageList;
            break;
        case ClipMediaTypeAudio:
            result = self.audioList;
            break;
        default:
            break;
    }
    return result;
}

- (ClipMediaItem *) itemForType:(ClipMediaType) type atIndex:(NSInteger) index
{
    ClipMediaItem *result = nil;
    NSArray *list = nil;
    if(type == ClipMediaTypeVideo) {
        list = self.videoList;
    } else if(type == ClipMediaTypeAudio) {
        list = self.audioList;
    } else if (type == ClipMediaTypeImage) {
        list = self.imageList;
    }

    if (list) {
        result = list[index];
    }
    return result;
}

@end
