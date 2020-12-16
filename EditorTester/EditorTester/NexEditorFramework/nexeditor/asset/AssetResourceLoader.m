/**
 * File Name   : AssetResourceLoader.m
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

#import "AssetResourceLoader.h"
#import "AssetDecrypter.h"
#import "AssetKeysetRegistry.h"

@interface AssetResourceLoader()
@end

@implementation AssetResourceLoader

+ (NSData *)loadFileContentsAtPath:(NSString *) path decrypt:(BOOL) decrypt recipient:(NSString *) recipient
{
    NSData *result = [NSData dataWithContentsOfFile:path];
    if (decrypt) {
        result = [self decryptData:result recipient:recipient];
    }
    return result;
}

+ (NSData *)loadFileContentsAtPath:(NSString *) path
{
    return [self loadFileContentsAtPath:path decrypt:NO recipient:nil];
}

+ (id) loadJSONObjectAtPath:(NSString *) path
{
    NSData *data = [self loadFileContentsAtPath:path];
    return [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
}

+ (NSData *) decryptData:(NSData *) data recipient:(NSString *) recipient
{
    AssetDecrypterKeyset *keyset = [AssetKeysetRegistry keysetFor:recipient];
    if ( keyset && keyset.count > 0) {
        keyset = [AssetDecrypter demangleKeyset:keyset];
        
        // FIXME: reuse decrypter if possible
        AssetDecrypter *decrypter = [[[AssetDecrypter alloc] initWithKeyset:keyset] autorelease];
        data = [decrypter decrypt:data];
    }
    return data;
}
@end
