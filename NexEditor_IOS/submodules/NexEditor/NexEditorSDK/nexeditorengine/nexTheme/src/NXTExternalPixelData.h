//
//  NXTExternalPixelData.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/25/14.
//
//

#ifndef __NexVideoEditor__NXTExternalPixelData__
#define __NexVideoEditor__NXTExternalPixelData__

#include <iostream>
#include "NexThemeRenderer.h"
#include "IRenderContext.h"
#include "NXTRenderer.h"

class NXTExternalPixelData {
    
public:
    
    NXTExternalPixelData() = default;
    
    ~NXTExternalPixelData() {
        freePixelData();
    }
    
    bool isEmpty() {
        return is_emtpy_;
    }
    
    bool isPreMultiplied() {
        return is_premultiplied_;
    }
    
    bool loadPixelData( IRenderContext& rendererContext, const std::string& path, bool premultipy_alpha=false ) {
        
        NXTRenderer& renderer = rendererContext.getRenderer();
        
        if( !isEmpty() ) {
            freePixelData();
        }
        
        if( !renderer.load_image_callback_ ) {
            return false;
        }
        
        // Load image callback does not write to the path (it could be const) but for
        // legacy reasons, it's a little hard to change it, so for now we use const_cast
        // to remove const-ness.  But later, it's better to just change loadImageCallback
        // to take a const char*.
        NXT_Error result = renderer.load_image_callback_(&image_info_,
                                                      const_cast<char*>(path.c_str()),
                                                      renderer.image_callback_private_data_);
        
        if( result != NXT_Error_None ) {
            return false;
        }
        
        image_callback_private_data_ = renderer.image_callback_private_data_;
        
        if( premultipy_alpha ) {
            premultAlpha(reinterpret_cast<unsigned int*>(image_info_.pixels),
                         image_info_.width*image_info_.height);
            is_premultiplied_ = true;
        } else {
            is_premultiplied_ = false;
        }
        
        is_emtpy_ = false;
    }
    
    void freePixelData() {
        if( isEmpty() ) {
            return;
        }
        if( image_info_.freeImageCallback ) {
            NXT_Error result = image_info_.freeImageCallback(&image_info_,
                                                             image_callback_private_data_);
            image_info_.freeImageCallback = nullptr;
        }
        
        is_emtpy_ = true;
    }
    
    int getWidth() {
        return is_emtpy_?0:image_info_.width;
    }
    
    int getHeight() {
        return is_emtpy_?0:image_info_.height;
    }
    
    int getPitch() {
        return is_emtpy_?0:image_info_.pitch;
    }
    
    NXT_PixelFormat getPixelFormat() {
        return is_emtpy_?NXT_PixelFormat_NONE:image_info_.pixelFormat;
    }
    
    void* getPixelDataPointer() {
        return is_emtpy_?nullptr:image_info_.pixels;
    }
    
    void cacheInternally() {
        // TODO
    }
    
    bool isCachedInternally() {
        // TODO
        return false;
    }
    
private:
    NXTExternalPixelData( const NXTExternalPixelData& other) = delete;
    NXTExternalPixelData& operator=(NXTExternalPixelData const&) = delete;
    
    bool is_premultiplied_ = false;
    bool is_emtpy_ = true;
    NXT_ImageInfo image_info_ = {0};
    void* image_callback_private_data_;

    void premultAlpha(unsigned int *pixels, unsigned int numPixels) {
        
        if( !pixels )
            return;
        
        unsigned char *p = (unsigned char*)pixels;
        unsigned char *end = p + (numPixels*4);
        
        for( ; p<end; p+=4 ) {
            unsigned char a = *(p+3);
            *(p+0) = *(p+0) * a / 255;
            *(p+1) = *(p+1) * a / 255;
            *(p+2) = *(p+2) * a / 255;
        }
        
    }

};


//struct NXT_ImageInfo_ {
//    int                     width;                  // Width of image (in pixels)
//    int                     height;                 // Height of image (in pixels)
//    int                     pitch;                  // Pitch of image (in pixels)
//    NXT_PixelFormat         pixelFormat;            // One of NXT_PixelFormat_????
//    int                     *pixels;                // Pointer to pixels
//    NXT_FreeImageCallback   freeImageCallback;      // Function to call to free pixel data
//    void                    *cbprivate0;            // Private data for use by freeImageCallback
//    void                    *cbprivate1;            // Private data for use by freeImageCallback
//};

#endif /* defined(__NexVideoEditor__NXTExternalPixelData__) */
