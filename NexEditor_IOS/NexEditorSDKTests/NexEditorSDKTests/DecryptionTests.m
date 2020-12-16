//
//  DecryptionTests.m
//  NexEditorFramework
//
//  Created by Simon Kim on 4/12/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import <XCTest/XCTest.h>

@import NexEditorFramework;
#import "AssetDecrypter.h"
#import "assetkey_demangler.h"
#import "AssetKeysetRegistry.h"

static const char *mangledKeys[] = {
    // NEX
    "12940175c51504b4246475650180214535e51475e1210144750414b5d5c514018021969401146435647414018021012606b7f73706b601e1210144750414b5d5c5c774f505750401802130e1210175a424d504643505e5750180214535e51475f4e121940114643564741401802101371766b7467701e1210144750414b5d5c5c774f505750401802100e1210175a424d504643505e5750180214535e51475f4e121940114643564741401802101371766b7467701e1210144750414b5d5c5c774f505750401802110e1210175a424d504643505e5750180214535e51475f4e121940114643564741401802101371766b7467701e1210144750414b5d5c5c774f505750401802160e1210175a424d504643505e5750180214535e51475f4e121940114643564741401802101371766b7467701e1210144750414b5d5c5c774f505750401802170e1210175a424d504643505e5750180214535e51475f4e121940114643564741401802101371766b7467701e1210144750414b5d5c5c774f505750401802140e1210175a424d504643505e5750180214535e51475f4f6e1210164b424750180210137771601e12101c535f5750180210135141475641504b4246401e1210124740424d514750180210167771706b62666d637c767d677c71706b6266601f4",
    "d6d84de9b97959db39f84dcc6dd84de9b97959db39f85a2848f989194dcc6d4dfcf958d94bfc7a3c48789bf80a2a489938bd0b2adb895cdbabb939ec9bfbeb68c81b6828c87b4aabe9b9994dad6d4d58f9c8394dcc6d4c3c0cb8ad6d4d793958db39f85a2848f989194dcc6d4d584a48d9ebfb5cc8e8a97b3b086a8b2a4b7c3c39ab7a4dad6d4db99929394dcc6d4d5b4b5b4dad6d4d58f9c8394dcc6d7c4cecb8",
    "57c055f1a16141c321e055d475c055f1a16141c321e0423050e1910155d47555c374f1311034f1f3c3f4d12474510003542470114203c17253636253636323913224e344a1e0f462b170a355b5755540e1d02155d475542414a0b57555612140c321e0423050e1910155d47555d1b1f111e40040a1305060e4d2f4e174441244f4f06355b57555a181312155d4755543534355b5755540e1d02155d4756454f4a0",
    "6833a6025292b230d213a6278633a6025292b230d213b1c3a31262f2a62786a6a2e3e78762c7c1f20723d16087d10321f201c20762b79110b1b7b030f2d2e191f7026211b772e7e032e3b2a64686a6b31223d2a62786a7d7e7534686a692d2b330d213b1c3a31262f2a62786a611b30222e7e0a14010300271a2d2b0f100a3d73060f2a64686a65272c2d2a62786a6b0a0b0a64686a6b31223d2a6278697a70753",
    "ad6ffa5e0eceee6c8e4ffa7bda6ffa5e0eceee6c8e4fed9fff4e3eaefa7bdafa4ceb1c5c8bfb9d8fae2eae4daebf3e5fdb0c7cfc4ddfcbdded0ebecebd7f8c5bbd8e0c9e0a2d7eae7dbb6efa1adafaef4e7f8efa7bdafb8bbb0f1adaface8eef6c8e4fed9fff4e3eaefa7bdafa8ddf2c6ccf7ecc0e6c8ccfae2c2eeb2ddf4def7eaeaffa1adafa0e2e9e8efa7bdafaecfcecfa1adafaef4e7f8efa7bdacbfb5b0f",
    "cf49dc7828e8c84aa869dc5dfc49dc7828e8c84aa869cbb9d9681888dc5dfcdcc9a89a4acbca0ab9bb0a98fbeded8898c9e86d6d697be8cb1a6acaeb9dedadf918bb2a58ed89f9db69fbbddc3cfcdcc96859a8dc5dfcddad9d293cfcdce8a8c94aa869cbb9d9681888dc5dfcdc982c589948e80b5aad0acaebf9d988fdcb0af9edebebdc3cfcdc2808b8a8dc5dfcdccadacadc3cfcdcc96859a8dc5dfceddd7d29",
    "a5ef7ade8e4e6eec0ecf7afb5aef7ade8e4e6eec0ecf6d1f7fcebe2e7afb5a7a5d4dcd7edd1d1dcb4b2c1fdc3c3f7e6f1c1fffcc1e1d1c4bdbfc0baddcfe5bfcec6d8cbcdecedc4edbfcee7a9a5a7a6fceff0e7afb5a7b0b3b8f9a5a7a4e0e6fec0ecf6d1f7fcebe2e7afb5a7a4fdf0bfe4e1b6e6eec9e4e2e1fbc2cfdaebe3bbcfd2e7a9a5a7a8eae1e0e7afb5a7a6c7c6c7a9a5a7a6fceff0e7afb5a4b7bdb8f"
    
    // LG
//    "D6D84D8979B9394DCC4D79585839285948F868284DAD4D683848689958394DCC4D2B3B5B4AFA6A2A9A7B8B2B9A3B8B5B4AFA6A2A4DAD4D28F868394DCC4D7B3B5A4DAD4D08394858F99989584DCCDAD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D6A4AFBBB7B4AFA4DAD4D08394858F999898B38B94939484DCC7CB8ADD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D7B5B2AFB0A3B4DAD4D08394858F999898B38B94939484DCC4CB8ADD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D7B5B2AFB0A3B4DAD4D08394858F999898B38B94939484DCC5CB8ADD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D7B5B2AFB0A3B4DAD4D08394858F999898B38B94939484DCC2CB8ADD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D7B5B2AFB0A3B4DAD4D08394858F999898B38B94939484DCC3CB8ADD84D39E8689948287949A9394DCC0979A95839AD4D5828792838584DCC4D7B5B2AFB0A3B4DAD4D08394858F999898B38B94939484DCC0CB8BAAD4D39895948F8682839294DCC0979A95839B8",
//    "B83EAB9FDFBE3DDF1EBCCEAE1F6FFFAB2AABCA8ECA0EBABE9A9DEE1E9A4DFFDDBCDCBD8CCADEAE9CAB4BAB0F5F9FBF3DDF1EAB2A3EAB0F5F9FBF3DDF1EBCCEAE1F6FFFAB2AABFFCABD2FBF2D0F5B8C2FAAFEDD2C9E1FEE0DAE2C7CCE3FEDAE6F5BCC4DDCBE7F1EECCECD0CCE9D0AEA9D5DAB4BABBE1F2EDFAB2AAADAEA5E4BAB5F7FCFDFAB2AABBDADBDAB4BABBE1F2EDFAB2A9AAA0A5E",
//    "6293063272139072B3116303B2C25206870602576203B237018150028182D0F2013321737731123106E606A2F232129072B306879306A2F232129072B3116303B2C252068706A34062218327F6D1123312A7D1A1B272A3C21200F62337E03213511767F037A2F0506751D2B7309082537006E60613B283720687077747F3E606F2D2627206870610001006E60613B2837206873707A7F3",
//    "CE59CCF8B8D95AB879DBA9C9780898CC4DCC7B481A49C96B3C6A6BA98D6828BAEBA93888CA1A8AFACC2CCC6838F8D85AB879CC4D59CC6838F8D85AB879DBA9C9780898CC4DCCC9CBDAE9781A98A8D95A6AA83A58489A890818B90ACD7D7899CAF97A696A988D6B69AB186BC8586DCDD8FBCC2CCCD97849B8CC4DCDBD8D392CCC3818A8B8CC4DCCDACADACC2CCCD97849B8CC4DFDCD6D39",
//    "BF4EDBEFAFCE4DAF6ECCBEDE6F1F8FDB5ADBEDBF6CEA5F1DCC7A5F6D6A7D6CED9FEAED0C8F3F9FEDDB3BDB7F2FEFCF4DAF6EDB5A4EDB7F2FEFCF4DAF6ECCBEDE6F1F8FDB5ADBDDED6F7DBE7CACCE7AAFDA0FDF5E4F7FFC3FCDCE7DAD5FEFCFDC8D3DAF4DAEAD8CBD5FCD6F6C8A1FFE6D6CDB3BDBCE6F5EAFDB5ADAAA9A2E3BDB2F0FBFAFDB5ADBCDDDCDDB3BDBCE6F5EAFDB5AEADA7A2E",
//    "F7CA5F6B2B4AC92BEA483A5AEB9B0B5FDE5FB99B096B38D959D9DA28EEDB289909EADB1EAFFE7A695FBF5FFBAB6B4BC92BEA5FDECA5FFBAB6B4BC92BEA483A5AEB9B0B5FDE5F1E4A6EEB38EB490808382899ABE838893B1AFA7A2E1B2869A95B6EDB5A3B3E2B59F86E2AFB783E4A89FE8B5FBF5F4AEBDA2B5FDE5E2E1EAABF5FAB8B3B2B5FDE5F4959495FBF5F4AEBDA2B5FDE6E5EFEAA",
//    "86DD487C3C5DDE3CFD5F2D4DFC8C1C48C948FD7C7C7D793C1D8C2FB8FFAE1EACEF5C1C7C1F5F1E1C48A848ECBC7C5CDE3CFD48C9DD48ECBC7C5CDE3CFD5F2D4DFC8C1C48C9481EF96F2E0D3EFFFF7ECE7F8E3F2CCF2F7E5E1E1EBEEC592E094E2DB8DCED4F2C9C4F8E9E1C5C3F4CFE493E48A8485DFCCD3C48C9493909BDA848BC9C2C3C48C9485E4E5E48A8485DFCCD3C48C97949E9BD"
    
};

@interface DecryptionTests : XCTestCase

@end

@implementation DecryptionTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testDescrambleDigits {

    // nex.bep
    const int providerIDDigits[] = {52, 63, 34, 116, 56, 63, 42};
    
    int length = sizeof(providerIDDigits) / sizeof(int);
    NSMutableData *data = [NSMutableData dataWithLength:length + 1];
    char *providerID = ((char *)data.mutableBytes);
    akd_demangle_digits(&providerIDDigits[0], length, providerID);
    providerID[length] = 0;
    XCTAssert(strncmp(providerID, "nex.bep", length) == 0, @"demangled '%s' != 'nex.bep", providerID);
    
}

- (void) testScanHexString {
    
    size_t length = strlen(mangledKeys[0]);
    NSMutableData *data = [NSMutableData dataWithLength:(length /2)];
    uint8_t *demangled = (uint8_t *) data.mutableBytes;
    
    akd_scan_hex(mangledKeys[0], length, demangled);
    
    XCTAssert(demangled[0] == 0x12, "demangled[0] %02x != 0x12", demangled[0]);
    XCTAssert(demangled[1] == 0x94, "demangled[0] %02x != 0x94", demangled[1]);
    XCTAssert(demangled[length/2 - 2] == 0x01, "demangled[0] %02x != 0x01", demangled[length/2 -2]);
    XCTAssert(demangled[length/2 - 1] == 0xf4, "demangled[0] %02x != 0xf4", demangled[length/2 -1]);
}

- (void) testDemangleKeyBytes {
   
    int count = sizeof(mangledKeys) / sizeof(char *);
    
    const char *expected[] = {
        // NEX
        " [{\"status",
        "cKeyString",
        "cKeyString",
        "cKeyString",
        "cKeyString",
        "cKeyString",
        "cKeyString",
        // LG
//        "DECRYPT_AN",
//        "4p4x3s1Avy",
//        "b7dri1RZGb",
//        "YjOzrX-HXt",
//        "AdY1jNS8jI",
//        "LnGaTJBJzU",
//        "yaaq1ewnT-",
    };
    const size_t indices[] = {
        32,
        17,
        17,
        17,
        17,
        17,
        17,
    };
    
    for( int i = 0; i < count; i++) {
        size_t length = strlen(mangledKeys[i]);
        
        NSMutableData *data = [NSMutableData dataWithLength:(length /2)];
        uint8_t *keybytes = (uint8_t *) data.mutableBytes;
        akd_scan_hex(mangledKeys[i], length, keybytes);
        
        // in-place conversion: (0..length-1)
        akd_demangle_keybytes(keybytes, data.length);
        keybytes[data.length-1] = 0;
        NSLog(@"demangled[%d]: '%s'", i, (const char *)keybytes);

        XCTAssert(strncmp((const char *)keybytes + indices[i], expected[i], 10) == 0, @"!= [%d]'%s'", i, expected[i]);
    }
}

- (void) testDecrypt
{
    AssetDecrypterKeyset *keyset = [AssetKeysetRegistry keysetFor:@"nex.bep"];
    keyset = [AssetDecrypter demangleKeyset:keyset];
    AssetDecrypter *decrypter = [[AssetDecrypter alloc] initWithKeyset:keyset];
    
    NSURL *url = [[NSBundle mainBundle] URLForResource:@"encrypted" withExtension:@"bin"];
    NSData *encrypted = [NSData dataWithContentsOfURL:url];
    
    NSData *decrypted = [decrypter decrypt:encrypted];
    XCTAssert(decrypted != nil, @"Decryption failed");
    if (decrypted) {
        NSString *string = [[NSString alloc] initWithData:decrypted encoding:NSUTF8StringEncoding];
        NSRange range = [string rangeOfString:@"com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.audio.bokeh_bgm"];
        XCTAssert(range.location == 68, @"location %d != 68", (int) range.location);
    }
}

@end