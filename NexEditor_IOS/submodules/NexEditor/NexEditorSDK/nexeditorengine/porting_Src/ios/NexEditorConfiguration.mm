/**
 * File Name   : NexEditorConfiguration.mm
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

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <CoreText/CoreText.h>
#import "NexEditorConfiguration.h"
#import "NexThemeRenderer.h"
#import "NexEditorLog.h"

static CGFloat maxTextureSize = 0.0;

@implementation NexEditorConfiguration

/// Capture parameters/constants from the current GL Context
/// \note The caller must ensure a current GL Context is set before calling this method
+ (void) captureGLContextParams
{
    GLfloat size = 0.0;
    glGetFloatv(GL_MAX_TEXTURE_SIZE, &size);
    
    GLenum glerror = glGetError();
    if (glerror != GL_NO_ERROR) {
        NexLogW(NSStringFromClass(self), @"Failed to get GL_MAX_TEXTURE_SIZE error: %d value: %2.3f", (int) glerror, size);
    }
    maxTextureSize = size;
}

- (CGFloat)maxClipImageSize
{
    return maxTextureSize;
}

- (NSString *)valueOf:(NexEditorConfigParam)param
{
    NSString *result = nil;
    
    switch(param) {
        case NexEditorConfigParamMaxClipImageSize:
            result = [NSString stringWithFormat:@"%f", [self maxClipImageSize]];
            break;
            
        default:
            break;
    }
    return result;
}

@end
