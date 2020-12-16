/******************************************************************************
 * File Name   : MediaInfo.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "MediaInfo.h"
#import <NexEditorEngine/NexEditor.h>
#import "EditorUtil.h"
#import <NexEditorEngine/NexClipInfo.h>
@import Photos;

@implementation MediaInfo

- (instancetype) initWithPath:(NSString *)path error:(NXEError **)error
{
    BOOL succeed = YES;
    self = [super init];
    if(self) {
        NexEditor* editor = [NexEditor sharedInstance];
        
        if([EditorUtil isSupportedImage:path]) {
            if (![self readImageInfo:[path UTF8String]] ) {
                succeed = NO;
                if (error) {
                    *error = [[NXEError alloc] initWithErrorCode:ERROR_FILE_IO_FAILED];
                }
            }
        } else {
            
            NexClipInfo* clipinfo = [[[NexClipInfo alloc] init] autorelease];
            ERRORCODE error_ = [editor getClipInfoSync:path NexClipInfo:clipinfo includeSeekTable:true userTag:0];
            if(error_ == ERROR_NONE) {
                _hasAudio = clipinfo.existAudio;
                if ( _hasAudio ) {
                    if ( [editor isSupportedCodecType:clipinfo.audioCodecType isVideo:NO isDecoder:YES] == NO ) {
                        if ( error ) {
                            *error = [[NXEError alloc] initWithErrorCode:ERROR_UNSUPPORT_FORMAT];
                        }
                        succeed = NO;
                    }
                }
                _hasVideo = clipinfo.existVideo;
                if ( _hasVideo ) {
                    if ( [editor isSupportedCodecType:clipinfo.videoCodecType isVideo:YES isDecoder:YES] == NO ) {
                        if ( error ) {
                            *error = [[NXEError alloc] initWithErrorCode:ERROR_UNSUPPORT_FORMAT];
                        }
                        succeed = NO;
                    }
                    if( succeed && (clipinfo.videoCodecType==eNEX_CODEC_V_HEVC) ) {
                        AVURLAsset* avurlAsset = [AVURLAsset assetWithURL:[NSURL fileURLWithPath:path]];
                        
                        for(int i =0; i<avurlAsset.tracks.count; i++) {
                            AVAssetTrack* track = avurlAsset.tracks[i];
                            if( [track.mediaType isEqualToString:AVMediaTypeVideo] && !track.isPlayable ) {
                                *error = [[NXEError alloc] initWithErrorCode:ERROR_UNSUPPORT_FORMAT];
                                succeed = NO;
                                break;
                            }
                        }
                    }
                }
                _hasImage = false;
                _audioDuration = clipinfo.audioDuration;
                _videoDuration = clipinfo.videoDuration;
                _videoWidth = clipinfo.videoWidth;
                _videoHeight = clipinfo.videoHeight;
                _seekPointCount = clipinfo.seekPointCount;
                _fps = clipinfo.fps;
                _videoH264Profile = clipinfo.videoH264Profile;
                _videoH264Level = clipinfo.videoH264Level;
                _videoOrientation = clipinfo.videoOrientation;
                _videoBitRate = clipinfo.videoBitRate;
                _audioBitRate = clipinfo.audioBitRate;
                _audioSamplingRate = clipinfo.audioSampleRate;
                _audioChannels = clipinfo.audioChannels;
            } else {
                if ( error ) {
                    *error = [[NXEError alloc] initWithErrorCode:error_];
                }
                succeed = NO;
            }
        }
    }
    
    id result = self;
    if ( !succeed ) {
        [self release];
        result = nil;
    }
    return result;
}

- (BOOL)readImageInfo:(const char *)path
{
    BOOL result = NO;
    
    __block UIImage *image = nil;
    if([[NSString stringWithFormat:@"%s", path] hasPrefix:@"phasset-image://"]) {
        NSString *filepath = [EditorUtil removeSubString:@"phasset-image://" from:[NSString stringWithFormat:@"%s", path]];
        PHAsset *asset = [PHAsset fetchAssetsWithLocalIdentifiers:@[filepath] options:nil][0];
        PHImageRequestOptions * imageRequestOptions = [PHImageRequestOptions new];
        imageRequestOptions.synchronous = YES;
        [[PHImageManager defaultManager] requestImageDataForAsset:asset
                                                          options:imageRequestOptions
                                                    resultHandler:^(NSData *imageData, NSString *dataUTI, UIImageOrientation orientation, NSDictionary *info) {
            image = [UIImage imageWithData:imageData];
        }];
        
    } else {
        image = [UIImage imageNamed:[NSString stringWithUTF8String:path]];
    }
    
    if ( image ) {
        _hasAudio = false;
        _hasVideo = false;
        _hasImage = true;
        _audioDuration = 0;
        _videoDuration = 0;
        _videoWidth = (int)image.size.width;
        _videoHeight = (int)image.size.height;
        _seekPointCount = 0;
        _fps = 0;
        _videoH264Profile = 0;
        _videoH264Level = 0;
        _audioBitRate = 0;
        _audioSamplingRate = 0;
        _audioChannels = 0;
        // Discard image orientation and let the image loader handle it
        _rotateDegree = 0;
        result = YES;
    }
    return result;
}

#pragma mark - NSCopying

- (instancetype)copyWithZone:(NSZone *)zone
{
    MediaInfo *destination = [[self class] allocWithZone:zone];
    
    destination.hasAudio = self.hasAudio;
    destination.hasVideo = self.hasVideo;
    destination.hasImage = self.hasImage;
    destination.audioDuration = self.audioDuration;
    destination.videoDuration = self.videoDuration;
    destination.videoWidth = self.videoWidth;
    destination.videoHeight = self.videoHeight;
    destination.rotateDegree = self.rotateDegree;
    destination.seekPointCount = self.seekPointCount;
    destination.fps = self.fps;
    destination.videoH264Profile = self.videoH264Profile;
    destination.videoH264Level = self.videoH264Level;
    destination.videoOrientation = self.videoOrientation;
    destination.videoBitRate = self.videoBitRate;
    destination.audioBitRate = self.audioBitRate;
    destination.audioSamplingRate = self.audioSamplingRate;
    destination.audioChannels = self.audioChannels;
    
    return destination;
}

@end
