/**
 * File Name   : AssetResourceDecrypter.m
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

#import "AssetResourceDecrypter.h"
#import "AssetDecrypter.h"
#import "AssetKeysetRegistry.h"
#import "AssetEncryptionManifest.h"

@interface AssetResourceDecrypter()
/// full filesystem path to e.json
@property (nonatomic, strong) NSString *pathToManifest;
/// provider and list of encrypted file subpaths
@property (nonatomic, strong) AssetEncryptionManifest *manifest;
/// lazy initialized decrypter with keyset for manifest.provider
@property (nonatomic, strong) AssetDecrypter *decrypter;
@end

@implementation AssetResourceDecrypter
@synthesize packagePath = _packagePath;
@synthesize subpathToEncryptionManifest = _subpathToEncryptionManifest;

- (void) dealloc
{
    self.decrypter = nil;
    self.packagePath = nil;
    self.subpathToEncryptionManifest = nil;
    self.manifest = nil;
    [super dealloc];
}

- (AssetDecrypter *) decrypter
{
    if ( _decrypter == nil ) {
        
        if (self.manifest.provider) {
            AssetDecrypterKeyset *keyset = [AssetKeysetRegistry keysetFor:self.manifest.provider];
            keyset = [AssetDecrypter demangleKeyset:keyset];
            _decrypter = [[AssetDecrypter alloc] initWithKeyset:keyset];
        }
    }
    return _decrypter;
}

#pragma mark - AssetResourceDecryptionDelegate

- (void) setPackagePath:(NSString *)path
{
    [_packagePath release];
    _packagePath = [path retain];
}

- (void) setSubpathToEncryptionManifest:(NSString *)subpath
{
    [_subpathToEncryptionManifest release];
    _subpathToEncryptionManifest = [subpath retain];
    
    if (self.packagePath == nil || subpath == nil) {
        return;
    }
    
    NSString *pathToManifest = [self.packagePath stringByAppendingPathComponent:subpath];
    if ([pathToManifest isEqualToString:self.pathToManifest]) {
        return;
    }
    
    self.pathToManifest = pathToManifest;
    self.manifest = [[[AssetEncryptionManifest alloc] initWithPath:pathToManifest] autorelease];
    self.decrypter = nil;
}

- (NSData *) decryptData:(NSData *) encrypted atSubpath:(NSString *) subpath
{
    NSData *result = encrypted;
    if ( [self.manifest hasSubpath:subpath]) {
        NSData *decrypted = [self.decrypter decrypt: encrypted];
        if (decrypted) {
            result = decrypted;
        }
    }
    return result;
}

- (void) releaseResource
{
    self.manifest = nil;
    self.decrypter = nil;
}

@end
