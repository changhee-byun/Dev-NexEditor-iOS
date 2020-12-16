//
//  APIDemosTests.m
//  APIDemosTests
//
//  Created by mj on 2016. 6. 28..
//  Copyright © 2016년 NexStreaming. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <stdlib.h>
#include <mach/mach_time.h>

@interface APIDemosTests : XCTestCase

@end

@implementation APIDemosTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testRandom {
    srandom((unsigned) mach_absolute_time());
    
    double step = 1.0 / 16;
    double cap = step;
    for( int i = 0; i < 16; i++) {
        for( int j = 0; j < 4; j++ ) {
            double r = ((double) random() / INT32_MAX) * cap;
            double g = ((double) random() / INT32_MAX) * cap;
            double b = ((double) random() / INT32_MAX) * cap;
            NSLog(@"%1.6f %1.6f %1.6f", r, g, b);
        }
        
        cap += step;
    }
}
@end
