/******************************************************************************
 * File Name   : NexOffScreenManager.h
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

#ifndef NexOffScreenManager_h
#define NexOffScreenManager_h

#if defined(ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(__APPLE__)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

class NexOffScreenFrameBuffer {
public:
    class NexGLGeneratedObj {
        GLuint obj;
        bool available;
    public:
        NexGLGeneratedObj() { clear(); };
        void set(GLuint o) { obj = o; available = true; };
        bool beAvailable() { return available; };
        void clear() { obj = (GLuint)-1; available = false; };
        operator GLuint() { return (available ? obj : (GLuint)-1); };
    };
    
    NexOffScreenFrameBuffer(GLuint fbo=0);
    virtual ~NexOffScreenFrameBuffer();
    
//    void setDefaultFrameBuffer(int fbo) {mDefaultFrameBuffer = fbo};
    void create(int w, int h, GLuint texid = -1, int colorFormat = GL_RGBA, bool depth = false, bool stencil = false);
    //void createFromTex( GLuint tex, int w, int h )
    void destroy();
    bool created() { return mCreated; }
    
    operator GLuint() { return (GLuint)mFBO; };
    GLuint getTex() { return (GLuint)mTex; };
    bool releaseTexOwnership();
    
private:
    NexGLGeneratedObj mFBO;
    NexGLGeneratedObj mTex;
    //GLuint mFBO;
    //GLuint mTex;
    
    GLuint mReceivedTex;
    bool mCreated;
    int mWidth;
    int mHeight;
    int mColorFormat;
    bool mDepthBuffEnabled;
    bool mStencilBuffEnabled;
};

#endif /* NexOffScreenManager_hpp */

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/08/29    Draft.
 -----------------------------------------------------------------------------*/
