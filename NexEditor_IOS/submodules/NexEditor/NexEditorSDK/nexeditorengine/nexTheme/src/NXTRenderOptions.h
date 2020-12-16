//
//  NXTRenderOptions.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 12/1/14.
//
//

#ifndef __NexVideoEditor__NXTRenderOptions__
#define __NexVideoEditor__NXTRenderOptions__

#include <stdio.h>

class NXTRenderOptions {
public:
    void parse(const char* options);
    
    float brightness_;
    float contrast_;
    float saturation_;
    int tint_color_;
    bool no_fx_;
    
private:
    void handleFastFloatArrayPreviewOption( const char* optionName, int paramCount, float* params );
    void handleFastPreviewOption( const char* optionName, int value );
    
};

#endif /* defined(__NexVideoEditor__NXTRenderOptions__) */
