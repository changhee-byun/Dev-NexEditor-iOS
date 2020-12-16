/******************************************************************************
 * File Name   : NexOffScreenManager.cpp
 * Description :
 *******************************************************************************
 * Copyright (c) 2002-2019 KineMaster Corporation. All rights reserved.
 * http://www.kinemaster.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

#include <stdio.h>
#include "NexOffScreenFrameBuffer.h"

#define LOGE printf
//#define LOGGING_GLRESOURCE

static int _CheckGLError(int line, const char *func) {
    GLint error;
    int ret = 1;
    for (error = glGetError(); error; error = glGetError()) {
        LOGE("[%s %i] glError (0x%x : %s)\n", func, line, error, "none");
        ret = 0;
    }
    return ret;
}

#define CHECK_GL_ERROR() _CheckGLError(__LINE__,__func__)

NexOffScreenFrameBuffer::NexOffScreenFrameBuffer(GLuint fbo)
: mCreated(false)
, mTex()
, mFBO()
, mReceivedTex(-1)
, mWidth(0)
, mHeight(0)
, mColorFormat(0)
, mDepthBuffEnabled(false)
, mStencilBuffEnabled(false)
{
};

NexOffScreenFrameBuffer::~NexOffScreenFrameBuffer()
{
    destroy();
}

void NexOffScreenFrameBuffer::create(int w, int h, GLuint texid, int colorFormat, bool depth, bool stencil)
{
    if ( mCreated ) {
        if ( mReceivedTex == texid && mWidth == w && mHeight == h && mColorFormat == colorFormat && mDepthBuffEnabled == depth && mStencilBuffEnabled == stencil ) {
            return;
        }
    }
    
    // TODO: let's separate 'delete tex' from 'delete fbo'
    destroy();
    
    // just for useTex == true.
    CHECK_GL_ERROR();

    GLint old_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
    CHECK_GL_ERROR();

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    if ( !CHECK_GL_ERROR() )
        return;
    
    mFBO.set(fbo);
    
#ifdef LOGGING_GLRESOURCE
    LOGE("LAYERSTYLE: gen FBO %d\n", (GLuint)mFBO);
#endif
    
    if ( texid != -1 ) {
        mReceivedTex = texid;
        mTex.set(texid);
    } else {
        GLuint tex;
        glGenTextures(1, &tex);
        
        if ( !CHECK_GL_ERROR() )
            return;

        mTex.set(tex);
        
#ifdef LOGGING_GLRESOURCE
        LOGE("LAYERSTYLE: gen TEX %d\n", (GLuint)mTex);
#endif

        GLint old_ = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_);
        glBindTexture(GL_TEXTURE_2D, mTex);
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, w, h, 0, colorFormat, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, old_);
    }
    

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        if ( texid != -1 ) {
            GLuint tex = mTex;
            glDeleteTextures(1, &tex);
#ifdef LOGGING_GLRESOURCE
            LOGE("LAYERSTYLE: del TEX %d\n", tex);
#endif
            mTex.clear();
        }
        
        {
            GLuint fbo = mFBO;
            glDeleteFramebuffers(1, &fbo);
#ifdef LOGGING_GLRESOURCE
            LOGE("LAYERSTYLE: del FBO %d\n", fbo);
#endif
            mFBO.clear();
        }
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER,old_fbo);
    mWidth = w;
    mHeight = h;
    mColorFormat = colorFormat;
    mDepthBuffEnabled = depth;
    mStencilBuffEnabled = stencil;
    
    mCreated = true;
}

void NexOffScreenFrameBuffer::destroy()
{
    if ( mCreated ) {
        GLint old_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
        CHECK_GL_ERROR();
        
        if ( old_fbo == mFBO ) {
            LOGE("Error. this destroy call delete fbo which are binded currently.\n");
        }
        
        if ( mReceivedTex == mTex ) {
            // must not delete tex.
        } else {
            GLuint tex = mTex;
            glDeleteTextures(1, &tex);
#ifdef LOGGING_GLRESOURCE
            LOGE("LAYERSTYLE: del TEX %d\n", (GLuint)mTex);
#endif
        }
        mTex.clear();
        mReceivedTex = -1;
        
        {
            GLuint fbo = mFBO;
            glDeleteFramebuffers(1, &fbo);
#ifdef LOGGING_GLRESOURCE
            LOGE("LAYERSTYLE: del FBO %d\n", fbo);
#endif
            mFBO.clear();

        }
        mCreated = false;
    }
}

bool NexOffScreenFrameBuffer::releaseTexOwnership()
{
    if ( !mTex.beAvailable() )
        return false;
    
    mReceivedTex = mTex;
    
    return true;
}


/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/08/29    Draft.
 -----------------------------------------------------------------------------*/
