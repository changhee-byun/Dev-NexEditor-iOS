/******************************************************************************
 * File Name   :	AudioFileListDataController.m
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

#import "AudioFileListDataController.h"

@import MediaPlayer;

@implementation AudioFilListItem

- (id)init
{
    self = [super init];
    if(self) {
        _filePath = nil;
        _title = nil;
        _url = nil;
        _duration = 0.0f;
        _check = false;
    }
    return self;
}

- (void)dealloc
{
    _filePath = nil;
    _title = nil;
    _url = nil;
}
@end

@implementation AudioFileListDataController

- (void)dealloc
{
	self.audioList = nil;
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        _audioList = [[NSMutableArray alloc] init];
        
        [self setListFromBundle];
        [self setListFromDocument];
        [self setListFromiPod];
        
        return self;
    }
    return nil;
}

- (bool)isSupportedAudioTypes:(NSString *)fileExtension
{
    NSArray *supportedAudioExtensions = @[@"aac", @"mp3", @"m4a"];
    
    if([supportedAudioExtensions containsObject:[fileExtension lowercaseString]]) {
        return true;
    }
    
    return false;
}

- (BOOL)isDuplicateVideoPath:(NSString *)path
{
    for(AudioFilListItem *listItem in self.audioList) {
        if([[path lastPathComponent] isEqualToString:[listItem.filePath lastPathComponent]]) {
            return true;
        }
    }
    return false;
}

/**
 *  Pre-Installed Audio Files 가져온다는 조건이기에 /template 폴더에서 읽어오는 것으로 함.
 */
- (void)setListFromBundle
{
    NSString *fileName = nil;
    NSString *filePath = nil;
    
    NSDirectoryEnumerator *enumerator_audio = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/template"]];
    
    while((fileName = [enumerator_audio nextObject]) != nil) {
        if([self isSupportedAudioTypes:[fileName pathExtension]]) {
            filePath = [[NSBundle mainBundle] pathForResource:[fileName stringByDeletingPathExtension]
                                                       ofType:[fileName pathExtension]
                                                  inDirectory:@"template"];
            
            AudioFilListItem *listItem = [[AudioFilListItem alloc] init];
            listItem.filePath = filePath;
            listItem.title = [[listItem.filePath lastPathComponent] stringByDeletingPathExtension];
            //
            NSURL *audioFileURL = [NSURL fileURLWithPath:listItem.filePath];
            AVURLAsset *audioAsset = [AVURLAsset URLAssetWithURL:audioFileURL options:nil];
            CMTime audioDuration = audioAsset.duration;
            listItem.duration = CMTimeGetSeconds(audioDuration);
            
            [self.audioList addObject:listItem];
			listItem = nil;
        }
    }
	enumerator_audio = nil;
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
        if([self isSupportedAudioTypes:[fileName pathExtension]]) {
            AudioFilListItem *listItem = [[AudioFilListItem alloc] init];
            listItem.filePath = [documentsDirectory stringByAppendingFormat:@"%@%@", @"/", fileName];
            listItem.title = [[listItem.filePath lastPathComponent] stringByDeletingPathExtension];
            //
            NSURL *audioFileURL = [NSURL fileURLWithPath:listItem.filePath];
            AVURLAsset *audioAsset = [AVURLAsset URLAssetWithURL:audioFileURL options:nil];
            CMTime audioDuration = audioAsset.duration;
            listItem.duration = CMTimeGetSeconds(audioDuration);
            
            [self.audioList addObject:listItem];
            listItem = nil;
        }
    }
}

- (void)setListFromiPod
{
    MPMediaQuery *allLocalSong = [MPMediaQuery songsQuery];
    [allLocalSong addFilterPredicate:[MPMediaPropertyPredicate predicateWithValue:[NSNumber numberWithBool:NO]
                                                                      forProperty:MPMediaItemPropertyIsCloudItem]];
    NSArray *songList = [allLocalSong items];
    NSSet *properties = [NSSet setWithObjects:MPMediaItemPropertyIsCloudItem,
                         MPMediaItemPropertyAssetURL,
                         MPMediaItemPropertyTitle,
                         MPMediaItemPropertyPlaybackDuration,
                         nil];
    
    for(MPMediaItem *mediaItem in songList) {
        AudioFilListItem *item = [[AudioFilListItem alloc] init];
        [mediaItem enumerateValuesForProperties:properties usingBlock:^(NSString * property, id value, BOOL *stop) {
            if([property isEqualToString:MPMediaItemPropertyIsCloudItem]) {
                if(value == nil || [(NSNumber *)value boolValue]) {
                    item.url = nil;
                    *stop = YES;
                }
            } else if([property isEqualToString:MPMediaItemPropertyAssetURL]) {
                if(!(item.url = (NSURL *)value)) {
                    *stop = YES;
                }
            } else if([property isEqualToString:MPMediaItemPropertyTitle]) {
                item.title = (NSString *)value;
            } else if([property isEqualToString:MPMediaItemPropertyPlaybackDuration]) {
                NSTimeInterval duration = [(NSNumber *)value doubleValue];
                item.duration = (float)duration;
            }
        }];
        //
        if(item.url) {
            [self.audioList addObject:item];
        }
        item = nil;
    }
}

@end
