//
//  NexEffectRendererTests.m
//  NexEffectRendererTests
//
//  Created by Simon Kim on 8/16/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "NexThemeRenderer.h"
//#import "NexThemeRenderer_Internal.h"
#import "NexThemeRenderer_Platform_iOS.h"

@interface NexEffectRendererTests : XCTestCase

@end

@implementation NexEffectRendererTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {

    NXT_ThemeRenderer_Platform plat = NXT_ThemeRenderer_Platform_CreateiOS();
    NXT_HThemeRenderer renderer = NXT_ThemeRenderer_CreateWithPlatform(NXT_RendererType_InternalPreviewContext, (1024*1024*16), NULL, plat);
    NXT_ThemeRenderer_Destroy(renderer, FALSE);
}

@end
