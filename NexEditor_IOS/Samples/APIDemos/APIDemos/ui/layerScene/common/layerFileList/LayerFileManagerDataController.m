/******************************************************************************
 * File Name   :	LayerFileManagerDataController.m
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

#import "LayerFileManagerDataController.h"
#import "Util.h"
#import <UIKit/UIKit.h>

@implementation LayerFileListItem

- (id)init
{
    return self;
}

- (void)dealloc
{
    _phAssetIdentifier = nil;
    _filePath = nil;
}

@end

@interface LayerFileManagerDataController ()

@property(nonatomic, retain) NSMutableArray *videoList;
@property(nonatomic, retain) NSMutableArray *imageList;
@property(nonatomic, retain) NSMutableArray *stickerList;

@property(nonatomic, assign) BOOL needVideo;
@property(nonatomic, assign) BOOL needImage;
@property(nonatomic, assign) BOOL needSticker;

@property(nonatomic, assign) int imageCount;

@end

@implementation LayerFileManagerDataController

- (instancetype)initWithVideo:(BOOL)needVideo Image:(BOOL)needImage Sticker:(BOOL)needSticker
{
    self = [super init];
    if(self) {
        _videoList = [[NSMutableArray alloc] init];
        _imageList = [[NSMutableArray alloc] init];
        _stickerList = [[NSMutableArray alloc] init];
        
        _needVideo = needVideo;
        _needImage = needImage;
        _needSticker = needSticker;
        _imageCount = 0;
        
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
    _stickerList = nil;
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
    for(LayerFileListItem *listItem in self.videoList) {
        if([[path lastPathComponent] isEqualToString:[listItem.filePath lastPathComponent]]) {
            return true;
        }
    }
    return false;
}

- (void)setItemsWithAssetCollection:(PHFetchResult*)result
{
    for(PHAsset *asset in result) {
        LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
        listItem.isChecked = false;
        
        if(asset.mediaType == PHAssetMediaTypeVideo) {
            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
            PHVideoRequestOptions *option = [PHVideoRequestOptions new];
            [[PHImageManager defaultManager] requestAVAssetForVideo:asset
                                                            options:option
                                                      resultHandler:^(AVAsset *ass, AVAudioMix *audioMix, NSDictionary *info) {
                  if([ass isKindOfClass:[AVURLAsset class]]) {
                      listItem.avURLAsset = (AVURLAsset*)ass;
                      listItem.filePath = ((AVURLAsset*)ass).URL.path;
                      
                      // 해당 코드에서 메모리 릭 발생
                      // 처음부터 thumbnail을 추출 하여 저장하지 않고, table view load할시에 thumbnail을 필요한 만큼만 추출 하도록 변경
                      // listItem.thumbnail = [Util imageFromVideoAsset:ass];
                  }
                  dispatch_semaphore_signal(semaphore);
              }];
            
            // wait 0.1 seconds
            dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 0.1 * NSEC_PER_SEC));
            
            if(self.needVideo) {
                if([self isSupportedVideoType:[listItem.filePath pathExtension]] && [self isDuplicateVideoPath:listItem.filePath] == NO) {
                    listItem.contentType = VideoType;
                    [self.videoList addObject:listItem];
                }
            }
        } else if(asset.mediaType == PHAssetMediaTypeImage) {
            PHImageRequestOptions *options = [PHImageRequestOptions new];
            options.synchronous = YES;
            options.resizeMode = PHImageRequestOptionsResizeModeExact;
            [[PHImageManager defaultManager] requestImageForAsset:asset
                                                       targetSize:CGSizeMake( 300, 300 )
                                                      contentMode:PHImageContentModeAspectFill
                                                          options:options
                                                    resultHandler:^(UIImage *result, NSDictionary *info) {
                listItem.thumbnail = result;
                listItem.filePath = [NSString stringWithFormat:@"phasset-image://%@", asset.localIdentifier];
                /*if([info objectForKey:@"PHImageFileURLKey"] == nil) {
                 listItem.filePath = [NSString stringWithFormat:@"phasset-image://%@", asset.localIdentifier];
                 } else {
                 listItem.filePath = [[info objectForKey:@"PHImageFileURLKey"] absoluteString];
                 }
                 
                 listItem.phAssetIdentifier = [NSString stringWithFormat:@"phasset-image://%@", asset.localIdentifier];*/
            }];
            
            if(self.needImage) {
                listItem.contentType = ImageType;
                [self.imageList addObject:listItem];
            }
        }
        listItem = nil;
    }
}

- (void)setListFromPHAsset
{
    PHFetchResult *collections = [PHAssetCollection fetchAssetCollectionsWithType:PHAssetCollectionTypeSmartAlbum
                                                                          subtype:PHAssetCollectionSubtypeAny
                                                                          options:nil];
    PHAssetCollection *videoCollection = nil;
    PHAssetCollection *camerarollCollection = nil;
    //    PHAssetCollection *screenshotsCollection = nil;
    
    for(PHAssetCollection *collection in collections) {
        if([collection.localizedTitle isEqualToString:@"Videos"])
            videoCollection = collection;
        else if([collection.localizedTitle isEqualToString:@"Camera Roll"]
                || [collection.localizedTitle isEqualToString:@"All Photos"])
            camerarollCollection = collection;
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
                
                LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
                listItem.contentType = VideoType;
                listItem.filePath = filePath;
                listItem.isChecked = false;
                listItem.phAssetIdentifier = nil;
                listItem.avURLAsset = nil;
                listItem.thumbnail = nil;
                
                [self.videoList addObject:listItem];
                listItem = nil;
            }
        }
        enumerator_video = nil;
    }
    // image add
    if(self.needImage) {
        NSDirectoryEnumerator *enumerator_image = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:BUNDLE_CLIPSOURCES_IMAGE]];
        
        while((fileName = [enumerator_image nextObject]) != nil) {
            if([self isSupportedImageType:[fileName pathExtension]]) {
                filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                           ofType:[fileName pathExtension]
                                                      inDirectory:BUNDLE_CLIPSOURCES_IMAGE];
                LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
                listItem.contentType = ImageType;
                listItem.filePath = filePath;
                listItem.isChecked = false;
                listItem.avURLAsset = nil;
                listItem.phAssetIdentifier = nil;
                listItem.thumbnail = [UIImage imageWithData:[NSData dataWithContentsOfFile:listItem.filePath]];
                
                [self.imageList addObject:listItem];
                listItem = nil;
            }
        }
        enumerator_image = nil;
    }
    // sticker add
    if(self.needSticker) {
        NSDirectoryEnumerator *enumerator_sticker = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:BUNDLE_CLIPSOURCES_STICKER]];
        
        while((fileName = [enumerator_sticker nextObject]) != nil) {
            if([self isSupportedImageType:[fileName pathExtension]]) {
                filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                           ofType:[fileName pathExtension]
                                                      inDirectory:BUNDLE_CLIPSOURCES_STICKER];
                LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
                listItem.contentType = StickerType;
                listItem.filePath = filePath;
                listItem.isChecked = false;
                listItem.avURLAsset = nil;
                listItem.phAssetIdentifier = nil;
                listItem.thumbnail = [UIImage imageWithData:[NSData dataWithContentsOfFile:listItem.filePath]];
                
                [self.stickerList addObject:listItem];
                listItem = nil;
            }
        }
        enumerator_sticker = nil;
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
        if([self isSupportedVideoType:[fileName pathExtension]] && [self isDuplicateVideoPath:fileName] == false) {
            if(self.needVideo) {
                LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
                listItem.contentType = VideoType;
                listItem.filePath = [documentsDirectory stringByAppendingFormat:@"%@%@", @"/", fileName];
                listItem.isChecked = false;
                listItem.avURLAsset = nil;
                listItem.phAssetIdentifier = nil;
                listItem.thumbnail = nil;
                
                [self.videoList addObject:listItem];
                listItem = nil;
            }
        } else if([self isSupportedImageType:[fileName pathExtension]]) {
            if(self.needImage) {
                LayerFileListItem *listItem = [[LayerFileListItem alloc] init];
                listItem.contentType = ImageType;
                listItem.filePath = [documentsDirectory stringByAppendingFormat:@"%@%@", @"/", fileName];
                listItem.isChecked = false;
                listItem.avURLAsset = nil;
                listItem.phAssetIdentifier = nil;//listItem.filePath;
                listItem.thumbnail = [UIImage imageWithData:[NSData dataWithContentsOfFile:listItem.filePath]];
                
                [self.imageList addObject:listItem];
                listItem = nil;
            }
        } else {
            continue;
        }
    }
}

- (UIImage *)getThumbnailWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    UIImage *thumbnail;
    
    if(contentType == VideoType) {
        thumbnail = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).thumbnail;
    } else if(contentType == ImageType) {
        thumbnail = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).thumbnail;
    } else {
        thumbnail = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).thumbnail;
    }
    
    return thumbnail;
}

- (AVAsset *)getAVAssetWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    AVAsset *avasset;
    
    if(contentType == VideoType) {
        avasset = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).avURLAsset;
    } else if(contentType == ImageType) {
        avasset = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).avURLAsset;
    } else {
        avasset = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).avURLAsset;
    }
    
    return avasset;
}

- (NSString *)getFileNameWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    NSString *fullPath;
    
    if(contentType == VideoType) {
        fullPath = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).filePath;
    } else if(contentType == ImageType) {
        fullPath = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).filePath;
    } else {
        fullPath = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).filePath;
    }
    
    if([fullPath hasPrefix:@"phasset-imag"]) {
        return [NSString stringWithFormat:@"IMG_%d", listIndex];
    } else {
        return [fullPath lastPathComponent];
    }
}

- (Boolean)getIsCheckedWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    Boolean isChecked = false;
    
    if(contentType == VideoType) {
        isChecked = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).isChecked;
    } else if(contentType == ImageType) {
        isChecked = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).isChecked;
    } else {
        isChecked = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).isChecked;
    }
    
    return isChecked;
}

- (NSString *)getFilePathWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    NSString *filePath = nil;
    
    if(contentType == VideoType) {
        filePath = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).filePath;
    } else if(contentType == ImageType) {
        filePath = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).filePath;
    } else {
        filePath = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).filePath;
    }
    
    return filePath;
}

- (NSString *)getphAssetIdentifierWithContentType:(ContentType)contentType listIndex:(int)listIndex
{
    NSString *filePath = nil;
    
    if(contentType == VideoType) {
        filePath = ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).phAssetIdentifier;
    } else if(contentType == ImageType) {
        filePath = ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).phAssetIdentifier;
    } else {
        filePath = ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).phAssetIdentifier;
    }
    
    return filePath;
}

- (int)getCountWithContentType:(ContentType)contentType
{
    if(contentType == VideoType) {
        return (int)[self.videoList count];
    } else if(contentType == ImageType) {
        return (int)[self.imageList count];
    } else {
        return (int)[self.stickerList count];
    }
}

- (NSMutableArray *)getFileListWithContentType:(ContentType)contentType
{
    if(contentType == VideoType) {
        return self.videoList;
    } else if(contentType == ImageType) {
        return self.imageList;
    } else {
        return self.stickerList;
    }
}

- (void)setIsCheckedWithContentType:(ContentType)contentType listIndex:(int)listIndex isChecked:(Boolean)isChecked
{
    if(contentType == VideoType) {
        ((LayerFileListItem*)[self.videoList objectAtIndex:listIndex]).isChecked = isChecked;
    } else if(contentType == ImageType) {
        ((LayerFileListItem*)[self.imageList objectAtIndex:listIndex]).isChecked = isChecked;
    } else {
        ((LayerFileListItem*)[self.stickerList objectAtIndex:listIndex]).isChecked = isChecked;
    }
}

@end
