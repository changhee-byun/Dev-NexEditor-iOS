/**
 * File Name   : AssetEncryptionManifest.m
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

#import "AssetEncryptionManifest.h"

static NSString *const kProvider = @"provider";
static NSString *const kSubpaths = @"subpaths";
static NSString *const kPSD = @"psd";
static NSString *const kF = @"f";

@interface AssetEncryptionManifest()
@property (nonatomic, strong) NSString *provider;
@property (nonatomic, strong) NSArray<NSString *> *subpaths;
@end

@implementation AssetEncryptionManifest

- (instancetype) initWithPath:(NSString *) path
{
    self = [super init];
    if (self) {
        NSDictionary *info = [self.class loadManifestAtPath:path];
        self.provider = info[kProvider];
        self.subpaths = info[kSubpaths];
    }
    return self;
}

- (BOOL) hasSubpath:(NSString *) subpath
{
    return self.subpaths && [self.subpaths indexOfObject:subpath] != NSNotFound;
}

+ (NSDictionary *) loadManifestAtPath:(NSString *) path
{
    NSDictionary *info = nil;
    NSData *data = [NSData dataWithContentsOfFile:path];
    if ( data ) {
        info = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        if ( info ) {
            NSData *psd = [info[kPSD] dataUsingEncoding:NSUTF8StringEncoding];
            if ( psd ) {
                NSDictionary *psdInfo = [NSJSONSerialization JSONObjectWithData:psd options:0 error:nil];
                if ( psdInfo && psdInfo[kF] && info[kProvider]) {
                    info = @{ kProvider: info[kProvider], kSubpaths: psdInfo[kF] };
                }
            }
        }
    }
    return info;
}
@end
