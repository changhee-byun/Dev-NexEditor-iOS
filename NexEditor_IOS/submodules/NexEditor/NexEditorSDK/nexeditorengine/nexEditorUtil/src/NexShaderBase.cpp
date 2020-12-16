/******************************************************************************
 * File Name   : NexShaderBase.cpp
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
#include <stdlib.h>
#include "NexShaderBase.h"

#define LOGE printf
#define CHECK_GL_ERROR() NexBaseShader::checkGLError(__LINE__,__func__)
#define GLSL(shader) #shader

GLfloat NexBaseShader::matrix4f_identity[] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

GLfloat NexBaseShader::matrix3f_vertex_coords[] = {
    -1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    -1.0, -1.0, 0.0,
    1.0, -1.0, 0.0
};

GLfloat NexBaseShader::matrix2f_tex_coords[] = {
//    0.0f, 0.0f,
//    1.0f, 0.0f,
//    0.0f, 1.0f,
//    1.0f, 1.0f
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f
};

NexBaseShader::NexBaseShader()
: mProgramName((char*)0)
, mVertexShader(INVALID_GL_OBJECT)
, mFragmentShader(INVALID_GL_OBJECT)
, mProgram(INVALID_GL_OBJECT)
, mLoaded(false)
, mVertexShaderCode((char*)0)
, mFragmentShaderCode((char*)0)
{
}

NexBaseShader::~NexBaseShader()
{
    destroy();
}

void NexBaseShader::registerCodes(const char* vc, const char* fc)
{
    mVertexShaderCode = vc;
    mFragmentShaderCode = fc;
}

//static
bool NexBaseShader::checkGLError(int line, const char *func) {
    GLint error;
    int ret = 1;
    for (error = glGetError(); error; error = glGetError()) {
        LOGE("[%s %i] glError (0x%x : %s)\n", func, line, error, "none");
        ret = 0;
    }
    return ret;
}

GLuint NexBaseShader::loadShader(GLenum shaderType, const char* source)
{
    GLuint shader;
    GLint compiled;
    
    shader = glCreateShader(shaderType);
    if ( INVALID_GL_OBJECT == shader)
    {
        CHECK_GL_ERROR();
        LOGE("[%s %d] glCreateShader error", __func__, __LINE__);
        return 0;
    }
#ifdef LOGGING_GLRESOURCE
    LOGE("LAYERSTYLE: cre SHADER %d\n", shader);
#endif
    
    glShaderSource(shader, 1, &source, NULL);
    CHECK_GL_ERROR();
    
    glCompileShader(shader);
    CHECK_GL_ERROR();
    
    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLint infoLen = 0;
        
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("Error compiling shader:\n%s\n", infoLog);
            
            free(infoLog);
        }
        
        glDeleteShader(shader);
#ifdef LOGGING_GLRESOURCE
        LOGE("LAYERSTYLE: del SHADER %d\n", shader);
#endif

        return 0;
    }
    
    return shader;
}

//static
GLuint NexBaseShader::createProgram(GLuint vs, GLuint fs)
{
    if ( INVALID_GL_OBJECT == vs || INVALID_GL_OBJECT == fs )
        return INVALID_GL_OBJECT;
    
    GLuint program = glCreateProgram();
    if ( INVALID_GL_OBJECT == program ) {
        return program;
    }

#ifdef LOGGING_GLRESOURCE
    LOGE("LAYERSTYLE: cre PROGRAM %d\n", program);
#endif

    glAttachShader(program, vs);        CHECK_GL_ERROR();
    glAttachShader(program, fs);        CHECK_GL_ERROR();
    
    glLinkProgram(program);
    
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    
    if (!linkStatus) {
        GLint infoLen = 0;
        
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        
        if (infoLen > 1)
        {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            LOGE("Error linking program:\n%s\n", infoLog);
            
            free(infoLog);
        }
        glDetachShader(program, vs);
        glDetachShader(program, fs);
        
        glDeleteProgram(program);
        return INVALID_GL_OBJECT;
    }
    
    return program;
}

void NexBaseShader::destroy()
{
    if ( INVALID_GL_OBJECT != mProgram ) {
        if ( INVALID_GL_OBJECT != mVertexShader )
            glDetachShader(mProgram, mVertexShader);
        
        if ( INVALID_GL_OBJECT != mFragmentShader )
            glDetachShader(mProgram, mFragmentShader);
    }
    
    if ( INVALID_GL_OBJECT != mVertexShader ) {
        glDeleteShader(mVertexShader);
#ifdef LOGGING_GLRESOURCE
        LOGE("LAYERSTYLE: del SHADER %d\n", mVertexShader);
#endif
        mVertexShader = INVALID_GL_OBJECT;
    }
    
    if ( INVALID_GL_OBJECT != mFragmentShader ) {
        glDeleteShader(mFragmentShader);
#ifdef LOGGING_GLRESOURCE
        LOGE("LAYERSTYLE: del SHADER %d\n", mFragmentShader);
#endif
        mFragmentShader = INVALID_GL_OBJECT;
    }
    
    if ( INVALID_GL_OBJECT != mProgram ) {
        glDeleteProgram(mProgram);
#ifdef LOGGING_GLRESOURCE
        LOGE("LAYERSTYLE: del PROGRAM %d\n", mProgram);
#endif
        mProgram = INVALID_GL_OBJECT;
    }
    
    clearLocations();
}

void NexBaseShader::prepareProgram(const char* pVertexShaderCode, const char* pFragmentShaderCode)
{
    destroy();

    mVertexShader = loadShader(GL_VERTEX_SHADER, pVertexShaderCode);
    mFragmentShader = loadShader(GL_FRAGMENT_SHADER, pFragmentShaderCode);
    
    if ( INVALID_GL_OBJECT == mVertexShader || INVALID_GL_OBJECT == mFragmentShader ) {
        destroy();
        return;
    }
    
    mProgram = createProgram(mVertexShader,mFragmentShader);
    if ( INVALID_GL_OBJECT == mProgram ) {
        LOGE("[%s %d] Failed to create shader program (%s)", __func__, __LINE__, this->mProgramName);
        destroy();
        return;
    }
}


void NexBaseShader::prepare()
{
    prepareProgram(mVertexShaderCode, mFragmentShaderCode);
    
    prepareUniforms();
    prepareAttributes();
}


void NexBaseShader::draw()
{
    CHECK_GL_ERROR();
    glUseProgram(mProgram);
    CHECK_GL_ERROR();
    
    setAttributes();
    CHECK_GL_ERROR();
    setUniforms();
    CHECK_GL_ERROR();

    drawInternal();
    CHECK_GL_ERROR();
    glFlush();
}

//////

//static
const char* NexShaderForTextureBase::mVertexShaderForTextureCode = GLSL(
attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform highp mat4 u_tex_matrix;
uniform highp mat4 u_mvp_matrix;
varying vec2 v_v2TexCoord;

void main()
{
    gl_Position =  vec4(a_position.xyz, 1) * u_mvp_matrix;
    v_v2TexCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
}
);

void NexShaderForTextureBase::registerFragmentCodeAndUseDefaultVertexCode(const char* fragmentCode)
{
    registerCodes(mVertexShaderForTextureCode,fragmentCode);
}

void NexShaderForTextureBase::prepareAttributes()
{
    //process Attributes
    location_a_position = glGetAttribLocation( mProgram, "a_position");
    location_a_texCoord = glGetAttribLocation( mProgram, "a_texCoord");
}

void NexShaderForTextureBase::setAttributes()
{
    glEnableVertexAttribArray(location_a_position);
    glVertexAttribPointer(location_a_position, 3, GL_FLOAT, 0, 0, matrix3f_vertex_coords);
    
    glEnableVertexAttribArray(location_a_texCoord);
    glVertexAttribPointer(location_a_texCoord, 2, GL_FLOAT, 0, 0, matrix2f_tex_coords);
}

void NexShaderForTextureBase::drawInternal()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}











const char* NexShaderDrawTex::mVertexShaderCode = GLSL(
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_v2TexCoord0;
void main()
{
    gl_Position =  a_position;
    v_v2TexCoord0 = (vec4(a_texCoord, 1.0, 1.0)).st;
}
);

const char* NexShaderDrawTex::mFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
varying highp vec2 v_v2TexCoord0;

void main()
{
    gl_FragColor =  texture2D(u_sTexture0, v_v2TexCoord0);
}
);


NexShaderDrawTex::NexShaderDrawTex()
: mTexId(-1)
{
    this->registerCodes(mVertexShaderCode, mFragmentShaderCode);
    clearLocations();
}

NexShaderDrawTex::~NexShaderDrawTex()
{
}

void NexShaderDrawTex::clearLocations()
{
    NexBaseShader::clearLocations();
    location_u_sTexture0 = -1;
    location_a_position = -1;
    location_a_texCoord = -1;
}


void NexShaderDrawTex::prepareUniforms()
{
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
}

void NexShaderDrawTex::prepareAttributes()
{
    location_a_position = glGetAttribLocation( mProgram, "a_position");
    location_a_texCoord = glGetAttribLocation( mProgram, "a_texCoord");
}

void NexShaderDrawTex::setAttributes()
{
    glEnableVertexAttribArray(location_a_position);
    glVertexAttribPointer(location_a_position, 3, GL_FLOAT, 0, 0, matrix3f_vertex_coords);
    
    glEnableVertexAttribArray(location_a_texCoord);
    glVertexAttribPointer(location_a_texCoord, 2, GL_FLOAT, 0, 0, matrix2f_tex_coords);
}

void NexShaderDrawTex::setUniforms()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexId);
    glUniform1i(location_u_sTexture0, 0);
}

void NexShaderDrawTex::drawInternal()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}











const char* NexShaderBlendingTexes::mVertexShaderCode = GLSL(
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_v2TexCoord0;
void main()
{
    gl_Position =  a_position;
    v_v2TexCoord0 = (vec4(a_texCoord, 1.0, 1.0)).st;
}
);

const char* NexShaderBlendingTexes::mFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;
varying highp vec2 v_v2TexCoord0;

void main()
{
    highp vec4 color0 = texture2D(u_sTexture0, v_v2TexCoord0);
    highp vec4 color1 = texture2D(u_sTexture1, v_v2TexCoord0);
    
    gl_FragColor = color1;//smoothstep(color0, color1, color1.a);
        
}
);


NexShaderBlendingTexes::NexShaderBlendingTexes()
: mTexId0(-1)
, mTexId1(-1)
{
    this->registerCodes(mVertexShaderCode, mFragmentShaderCode);
    clearLocations();
}

NexShaderBlendingTexes::~NexShaderBlendingTexes()
{
}

void NexShaderBlendingTexes::clearLocations()
{
    NexBaseShader::clearLocations();
    location_u_sTexture0 = -1;
    location_u_sTexture1 = -1;
    location_a_position = -1;
    location_a_texCoord = -1;
}


void NexShaderBlendingTexes::prepareUniforms()
{
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
    location_u_sTexture1 = glGetUniformLocation( mProgram, "u_sTexture1");
}

void NexShaderBlendingTexes::prepareAttributes()
{
    location_a_position = glGetAttribLocation( mProgram, "a_position");
    location_a_texCoord = glGetAttribLocation( mProgram, "a_texCoord");
}

void NexShaderBlendingTexes::setAttributes()
{
    glEnableVertexAttribArray(location_a_position);
    glVertexAttribPointer(location_a_position, 3, GL_FLOAT, 0, 0, matrix3f_vertex_coords);
    
    glEnableVertexAttribArray(location_a_texCoord);
    glVertexAttribPointer(location_a_texCoord, 2, GL_FLOAT, 0, 0, matrix2f_tex_coords);
}

void NexShaderBlendingTexes::setUniforms()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexId0);
    glUniform1i(location_u_sTexture0, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTexId1);
    glUniform1i(location_u_sTexture1, 1);
}

void NexShaderBlendingTexes::drawInternal()
{
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/09/02    Draft.
 -----------------------------------------------------------------------------*/
