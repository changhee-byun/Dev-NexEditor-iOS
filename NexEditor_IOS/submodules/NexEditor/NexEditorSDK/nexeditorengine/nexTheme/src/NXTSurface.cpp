//
//  NXTSurface.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 10/1/14.
//
//

#include "NXTSurface.h"
#include "NXTRenderer.h"

void NXTSurface::init(int width, int height) {
    
    if( is_init_ ) {
        deinit();
    }
    
    width_ = width;
    height_ = height;
    
    // OPTIMIZE?:   Since we render to only one texture at a time, and always clear
    //              the texture before rendering it, we could use shared renderbuffers
    //              and save on memory usage.
    
    glGenFramebuffers(1,&framebuffer_);                                     CHECK_GL_ERROR();
    glGenRenderbuffers(1,&depth_renderbuffer_);                             CHECK_GL_ERROR();
    glGenRenderbuffers(1,&stencil_renderbuffer_);                           CHECK_GL_ERROR();
    glGenTextures(1,&texture_);                                             CHECK_GL_ERROR();
    
    glBindTexture(GL_TEXTURE_2D,texture_);                                  CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);      CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);      CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);         CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);         CHECK_GL_ERROR();
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
    CHECK_GL_ERROR();
    
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    CHECK_GL_ERROR();
    
    glBindRenderbuffer(GL_RENDERBUFFER, stencil_renderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
    CHECK_GL_ERROR();
    
    is_init_ = true;
    
}

void NXTSurface::deinit() {
    if( !is_init_ )
        return;
    
    glDeleteFramebuffers(1,&framebuffer_);                                  CHECK_GL_ERROR();
    glDeleteRenderbuffers(1,&depth_renderbuffer_);                          CHECK_GL_ERROR();
    glDeleteRenderbuffers(1,&stencil_renderbuffer_);                        CHECK_GL_ERROR();
    
    is_init_ = false;
    
}

void NXTSurface::bind_texture() {
    glBindTexture(GL_TEXTURE_2D,texture_);
    CHECK_GL_ERROR();
}

void NXTSurface::bind() {
    if( !is_init_ )
        return;
    
    glDisable(GL_SCISSOR_TEST);                                                                             CHECK_GL_ERROR();
    glViewport(0,0,width_,height_);                                                                         CHECK_GL_ERROR();
    glScissor(0,0,width_,height_);                                                                          CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER,framebuffer_);                                                         CHECK_GL_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture_,0);                   CHECK_GL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth_renderbuffer_);      CHECK_GL_ERROR();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,stencil_renderbuffer_);  CHECK_GL_ERROR();
    
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        // TODO: Error handling
    }
    
}

void NXTSurface::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    CHECK_GL_ERROR();
}

