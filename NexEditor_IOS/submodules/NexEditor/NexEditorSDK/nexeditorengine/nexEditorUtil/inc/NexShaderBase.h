/******************************************************************************
 * File Name   : NexShaderBase.h
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

#ifndef NexShaderBase_hpp
#define NexShaderBase_hpp

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

#define INVALID_GL_OBJECT 0
//#define LOGGING_GLRESOURCE

class NexBaseShader {
public:
    NexBaseShader();
    virtual ~NexBaseShader();
    
    void registerCodes(const char*, const char*);

    virtual void prepare();
    virtual void draw();
    virtual void clearLocations() {};

    static bool checkGLError(int line, const char *func);
    static GLfloat matrix4f_identity[16];
    static GLfloat matrix3f_vertex_coords[12];
    static GLfloat matrix2f_tex_coords[8];
    
protected:
    char *mProgramName;
    GLuint mVertexShader;
    GLuint mFragmentShader;
    GLuint mProgram;
    bool mLoaded;
    
    virtual void destroy();
    
    virtual void prepareProgram(const char* pVertexShaderCode, const char* pFragmentShaderCode);
    virtual void prepareUniforms() = 0;
    virtual void prepareAttributes() = 0;
    virtual void setAttributes() = 0;
    virtual void setUniforms() = 0;
    virtual void drawInternal() = 0;
    
private:
    const char* mVertexShaderCode;
    const char* mFragmentShaderCode;
    
    static GLuint loadShader(GLenum shaderType, const char* source);
    static GLuint createProgram(GLuint vs, GLuint fs);
};

class NexShaderForTextureBase : public NexBaseShader {
public:
    static const char* mVertexShaderForTextureCode;

    void registerFragmentCodeAndUseDefaultVertexCode(const char*);

    virtual void prepareAttributes();
    virtual void setAttributes();
    virtual void drawInternal();
    
    virtual void clearLocations() {
        NexBaseShader::clearLocations();
        location_a_position = -1;
        location_a_texCoord = -1;
    };

private:
    int location_a_position;
    int location_a_texCoord;

    //static GLuint mVertexShaderForCommon;
    //static int mVertexShaderF
};

class NexShaderDrawTex : public NexBaseShader {
public:
    NexShaderDrawTex();
    virtual ~NexShaderDrawTex();
    
    void setTex(int tex) { mTexId = tex; };
    
    virtual void prepareUniforms();
    virtual void prepareAttributes();
    virtual void setAttributes();
    virtual void setUniforms();
    virtual void drawInternal();
    
private:
    static const char* mVertexShaderCode;
    static const char* mFragmentShaderCode;
    
    int location_u_sTexture0;
    int location_a_position;
    int location_a_texCoord;
    
    int mTexId;
    
    virtual void clearLocations();
};

class NexShaderBlendingTexes : public NexBaseShader {
public:
    NexShaderBlendingTexes();
    virtual ~NexShaderBlendingTexes();
        
    void setTex0(int tex) { mTexId0 = tex; };
    void setTex1(int tex) { mTexId1 = tex; };

    virtual void prepareUniforms();
    virtual void prepareAttributes();
    virtual void setAttributes();
    virtual void setUniforms();
    virtual void drawInternal();
    
private:
    static const char* mVertexShaderCode;
    static const char* mFragmentShaderCode;
    
    int location_u_sTexture0;
    int location_u_sTexture1;
    int location_a_position;
    int location_a_texCoord;
    
    int mTexId0;
    int mTexId1;

    virtual void clearLocations();
};

#endif /* NexShaderBase_hpp */

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/09/02    Draft.
 -----------------------------------------------------------------------------*/
