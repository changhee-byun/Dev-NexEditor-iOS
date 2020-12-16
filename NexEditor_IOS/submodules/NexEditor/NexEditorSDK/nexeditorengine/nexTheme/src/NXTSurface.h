//
//  NXTSurface.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 10/1/14.
//
//

#ifndef __NexVideoEditor__NXTSurface__
#define __NexVideoEditor__NXTSurface__

#include <stdio.h>
#include <GLES2/gl2.h>
#include "ITexture.h"

class NXTSurface : public ITexture {
    
public:
    NXTSurface(const NXTSurface& other) = delete;
    NXTSurface& operator=( const NXTSurface& other) = delete;
    NXTSurface() = default;
    
    void init(int width, int height);
    void deinit();
    void bind();            // Bind surface for output
    void unbind();          // Unbind surface (rendering goes to default surface)
    void bind_texture();    // Bind texture for input
    int get_width() {
        return width_;
    }
    int get_height() {
        return height_;
    }
    
    ~NXTSurface() {
        deinit();
    }
    
    virtual void bindTextures() override {
        bind_texture();
    }
    virtual ITexture::Format getTextureFormat() override {
        return ITexture::Format::RGBA;
    }
    virtual NXTMatrix getMatrix() override {
        return NXTMatrix();
    }
    
private:
    
    bool is_init_ = false;
    int width_;
    int height_;
    GLuint framebuffer_;
    GLuint depth_renderbuffer_;
    GLuint stencil_renderbuffer_;
    GLuint texture_;
    
};

#endif /* defined(__NexVideoEditor__NXTSurface__) */
