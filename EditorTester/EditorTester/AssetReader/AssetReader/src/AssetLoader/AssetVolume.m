/**
 * File Name   : AssetVolume.m
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

#import "AssetVolume.h"

NSString *const kURLSchemeAssetVolume = @"asset-volume";

@interface AssetVolume()
@property (nonatomic) NSString *subpath;
@end


@implementation AssetVolume
- (id) initWithType: (AssetVolumeType) volumeType subpath:(NSString *) subpath
{
    self = [super init];
    if ( self ) {
        self.volumeType = volumeType;
        self.subpath = subpath;
    }
    return self;
}

/// #. <kURLSchemeAssetVolume>://<bundle|documents|library>/path
/// #. file://...
/// #. path
- (id) initWithVolumePath:(NSString *) volumePath
{
    self = [super init];
    if ( self ) {
        AssetVolumeType volumeType = AssetVolumeTypeUnknown;
        NSString *subpath = volumePath;
        
        NSURL *url = [NSURL URLWithString:[volumePath stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
        if ( url) {
            if ([url.scheme isEqualToString:kURLSchemeAssetVolume] && url.host) {
                volumeType = [self.class volumeTypeFromString:url.host];
                if ( volumeType != AssetVolumeTypeUnknown ) {
                    subpath = url.path;
                    if (subpath.length > 0 && [subpath.pathComponents[0] isEqualToString:@"/"] ) {
                        subpath = [subpath substringFromIndex:1];
                    }
                }
            } else if (url.isFileURL) {
                static const AssetVolumeType types[] = {
                    AssetVolumeTypeDocuments,
                    AssetVolumeTypeLibrary,
                    AssetVolumeTypeMainBundle
                };
                int i;
                
                NSString *path = url.path;

                
                for( i = 0; i < sizeof(types)/sizeof(AssetVolumeType); i++) {
                    AssetVolumeType type = types[i];
                    NSString *rpath = [self.class relativePathOf:path to:type];
                    if (rpath) {
                        volumeType = type;
                        subpath = rpath;
                        break;
                    }
                }
                
                // fallback to filesystem path
                if (volumeType == AssetVolumeTypeUnknown) {
                    subpath = url.path;
                }
            }
        }
        self.volumeType = volumeType;
        self.subpath = subpath;
    }
    return self;
}

+ (NSString *) relativePathOf:(NSString *) path to:(AssetVolumeType) type
{
    NSString *result = nil;
    NSURL *url = nil;
    switch(type) {
        case AssetVolumeTypeDocuments:
            url = [self.class urlForDirectory:NSDocumentDirectory];
            break;
        case AssetVolumeTypeLibrary:
            url = [self.class urlForDirectory:NSLibraryDirectory];
            break;
        case AssetVolumeTypeMainBundle:
            url = [[NSBundle mainBundle] resourceURL];
            break;
        default:
            break;
    }
    
    if (url) {
        NSRange range = [path rangeOfString:url.path];
        if ( range.location == 0 ) {
            result = [path substringFromIndex:range.length + 1];
        }
    }
    return result;
}

- (NSString *) assetVolumeURLString
{
    NSString *volumeTypeString = [self.class stringFromVolumeType:self.volumeType];
    NSString *result = nil;
    if (volumeTypeString) {
        result = [NSString stringWithFormat:@"%@://%@/%@",
         kURLSchemeAssetVolume,
         volumeTypeString,
         self.subpath];
    } else {
        result = [NSURL fileURLWithPath:self.subpath].absoluteString;
    }
    return result;
}

- (NSURL *) resolvedURL
{
    return [self.class resolvedFileURLForVolumeType:self.volumeType subpath:self.subpath];
}

- (NSString *) resolvedPath
{
    return self.resolvedURL.path;
}

+ (NSURL *) urlForDirectory:(NSSearchPathDirectory) directory
{
    NSFileManager *fm = [NSFileManager defaultManager];
    return[fm URLsForDirectory:directory
                     inDomains:NSUserDomainMask][0];
}

+ (NSURL *) resolvedFileURLForVolumeType:(AssetVolumeType) type subpath:(NSString *) subpath
{
    NSURL *resultURL = nil;
    
    if ( type == AssetVolumeTypeBundle ) {
        NSArray<NSString *> *components = [subpath pathComponents];
        if ( components.count == 2 ) {
            NSString *bundleName = components[0];
            NSString *resourceName = components[1];
            NSURL *bundleURL = [[NSBundle mainBundle] URLForResource:bundleName withExtension:@"bundle"];
            if (bundleURL) {
                NSBundle *assetsBundle = [NSBundle bundleWithURL:bundleURL];
                resultURL = [assetsBundle URLForResource:resourceName withExtension:nil];
            }
        }
    } else if ( type == AssetVolumeTypeDocuments || type == AssetVolumeTypeLibrary ) {
        NSSearchPathDirectory searchDirectory = NSDocumentDirectory;
        if ( type == AssetVolumeTypeLibrary ) {
            searchDirectory = NSLibraryDirectory;
        }
        NSURL *resourceRootURL = [self urlForDirectory: searchDirectory];
        resultURL = [resourceRootURL URLByAppendingPathComponent:subpath];
    } else if ( type == AssetVolumeTypeMainBundle ) {
        resultURL = [[[NSBundle mainBundle] resourceURL] URLByAppendingPathComponent:subpath];
    }
    
    if ( resultURL == nil ) {
        // fallback to normal filesystem path
        resultURL = [NSURL fileURLWithPath:subpath];
    }
    
    return resultURL;
}

+ (AssetVolumeType) volumeTypeFromString:(NSString *) volumeTypeString
{
    AssetVolumeType result = AssetVolumeTypeUnknown;
    if ( [volumeTypeString isEqualToString:@"bundle"] ) {
        result = AssetVolumeTypeBundle;
    } else if ([volumeTypeString isEqualToString:@"documents"]) {
        result = AssetVolumeTypeDocuments;
    } else if ([volumeTypeString isEqualToString:@"library"]) {
        result = AssetVolumeTypeLibrary;
    } else if ([volumeTypeString isEqualToString:@"mainBundle"]) {
        result = AssetVolumeTypeMainBundle;
    }
    return result;
}

+ (NSString *) stringFromVolumeType:(AssetVolumeType) type
{
    NSString *result = nil;
    switch(type) {
        case AssetVolumeTypeBundle:
            result = @"bundle";
            break;
        case AssetVolumeTypeDocuments:
            result = @"documents";
            break;
        case AssetVolumeTypeLibrary:
            result = @"library";
            break;
        case AssetVolumeTypeMainBundle:
            result = @"mainBundle";
            break;
        default:
            break;
    }
    return result;
}
@end
