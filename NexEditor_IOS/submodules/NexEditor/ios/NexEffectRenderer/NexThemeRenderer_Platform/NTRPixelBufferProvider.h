//
//  NTRPixelBufferProvider.h
//  NexEditorFramework
//
//  Created by Simon Kim on 7/10/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NexThemeRenderer_Platform_iOS.h"

@interface NTRPixelBufferProvider : NSObject<NTRPixelBufferProviding>
@property (nonatomic) CGSize requiredSize;
@end
