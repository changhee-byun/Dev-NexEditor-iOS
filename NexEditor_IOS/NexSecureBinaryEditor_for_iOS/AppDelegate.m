//
//  AppDelegate.m
//  NexSecureBinaryEditor
//
//  Created by Matthew Feinberg on 8/13/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "AppDelegate.h"
#import "NexSecureWare.h"


@implementation AppDelegate

- (id) init {
    self = [super init];
    if (self) {
		
    }
    return self;
}

- (BOOL) applicationShouldOpenUntitledFile:(NSApplication *)sender {
	return NO;
}

@end
