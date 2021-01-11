/**
 * File Name   : AssetDecrypter.mm
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

#import "AssetDecrypter.h"
//#import <keyczar/keyczar.h>
//#import <keyczar/rw/keyset_reader.h>
//#import <keyczar/base/json_value_serializer.h>
#import "assetkey_demangler.h"
#import "NXEError.h"
/*
class KeysetDataReader: public keyczar::rw::KeysetReader {
    
    AssetDecrypterKeyset *keyset;
    
    virtual Value* ReadMetadata() const {
        std::string metadata = (const char *)this->keyset[0].bytes;
        keyczar::base::JSONStringValueSerializer serializer(&metadata);
        return serializer.Deserialize(NULL);
    }
    
    virtual Value* ReadKey(int version) const {
        Value *result = NULL;
        if (version < this->keyset.count) {
            std::string key = (const char *) this->keyset[version].bytes;
            keyczar::base::JSONStringValueSerializer serializer(&key);
            result = serializer.Deserialize(NULL);
        }
        return result;
    }
    
public:
    KeysetDataReader(AssetDecrypterKeyset *keyset) {
        this->keyset = keyset;
    }
};
*/

@interface AssetDecrypter()
{
//    KeysetDataReader *_reader;
//    keyczar::Crypter *_crypter;
}
@property (nonatomic, strong) AssetDecrypterKeyset *keyset;
@end

@implementation AssetDecrypter
- (instancetype) initWithKeyset:(AssetDecrypterKeyset *) keyset
{
    self = [super init];
    if (self) {
        if (keyset && keyset.count > 1) {
            // valid keyset has at least two elements: metadata, and version 1
            self.keyset = keyset;
        }
//        _crypter = nullptr;
    }
    return self;
}

- (void) dealloc
{
    self.keyset = nil;
//    if (_crypter != nullptr) {
//        delete _crypter;
//        delete _reader;
//    }
    [super dealloc];
}

//- (keyczar::Crypter *) crypter
//{
//    if (_crypter == nullptr && self.keyset) {
//        _reader = new KeysetDataReader(self.keyset);
//
//        _crypter = keyczar::Crypter::Read(*_reader);
//        _crypter->set_encoding(keyczar::Keyczar::NO_ENCODING);
//    }
//    return _crypter;
//}

- (NSData *) decrypt:(NSData *)encrypted
{
    NSData *result = nil;
//    if(self.crypter != nullptr)
//    {
//        std::string ciphertext((const char *) encrypted.bytes, encrypted.length);
//        std::string decrypted = "";
//        if (self.crypter->Decrypt(ciphertext, &decrypted)) {
//            result = [NSData dataWithBytes:decrypted.c_str() length:decrypted.length()];
//        }
//    }
    return result;
}

/// "12940175c5..." => "{\"encr..."
+ (AssetDecrypterKeyset *) demangleKeyset:(AssetDecrypterKeyset *) mangledKeyset
{
    NSMutableArray<NSData *> *result = [NSMutableArray array];
    for( NSData *mangled in mangledKeyset) {
        NSMutableData *data = [NSMutableData dataWithLength:mangled.length /2];
        uint8_t *demangled = (uint8_t *)data.mutableBytes;
        akd_scan_hex((const char *)mangled.bytes, mangled.length, demangled);
        akd_demangle_keybytes(demangled, data.length);
        demangled[data.length-1] = 0;
        [result addObject:data];
    }
    return [[result copy] autorelease];
}

@end
