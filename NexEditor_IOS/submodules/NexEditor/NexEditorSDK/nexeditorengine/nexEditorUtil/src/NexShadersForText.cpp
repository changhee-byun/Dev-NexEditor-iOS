/******************************************************************************
 * File Name   : NexShadersForText.cpp
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
#include "NexShadersForText.h"

#define LOGE printf
#define CHECK_GL_ERROR() NexBaseShader::checkGLError(__LINE__,__func__)
#define GLSL(shader) #shader


//////

const char* NexShaderDropShadowFirst::mDropShadow_1stFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
uniform highp float u_alpha;
uniform highp float u_size;
//uniform highp float u_distance;
uniform highp mat4 u_color_matrix;
uniform highp vec2 u_texture_size;
varying highp vec2 v_v2TexCoord;

highp vec2 encodeScreenCoordinate(highp float value) {
  return vec2(
    floor(value / 100.0),
    mod(value, 100.0)
  );
}
                                                             
void main()
{
    
    highp vec4 color;
    highp float size = floor(u_size);
    //highp float distance = u_distance;
    highp float texelX = (1.0 / u_texture_size.x);
    
    highp float minDistOut = 255.0;
    highp float xdistOut;
    highp float i;
    highp vec4 encCol;
    
    for (i = -size; i<size; i+=1.0) {
        encCol = texture2D(u_sTexture0, vec2(v_v2TexCoord.x + i*texelX, v_v2TexCoord.y));
        xdistOut = encCol.a > 0.0 ? abs(i) : size;
        minDistOut = min(minDistOut, xdistOut);
    }
    color = vec4(encodeScreenCoordinate(minDistOut / 255.0), 0.0,0.0);
    color = color * u_color_matrix;
    //color = texture2D(u_sTexture0, v_v2TexCoord)* u_color_matrix;
    gl_FragColor =  color * u_alpha;
}
);

NexShaderDropShadowFirst::NexShaderDropShadowFirst()
: mTexId(-1)
, mTexWidth(0)
, mTexHeight(0)
, mAlpha(1.0)
, mSize(15.0)
{
    this->registerFragmentCodeAndUseDefaultVertexCode(mDropShadow_1stFragmentShaderCode);
    clearLocations();
}

NexShaderDropShadowFirst::~NexShaderDropShadowFirst()
{
    
}

void NexShaderDropShadowFirst::clearLocations()
{
    NexShaderForTextureBase::clearLocations();
    
    location_u_tex_matrix = -1;
    location_u_mvp_matrix = -1;
    location_u_sTexture0 = -1;
    location_u_alpha = -1;
    location_u_size = -1;
    location_u_color_matrix = -1;
    location_u_texture_size = -1;
}

void NexShaderDropShadowFirst::prepareUniforms()
{
    // process Uniforms
    location_u_tex_matrix = glGetUniformLocation( mProgram, "u_tex_matrix");
    location_u_mvp_matrix = glGetUniformLocation( mProgram, "u_mvp_matrix");
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
    location_u_alpha = glGetUniformLocation( mProgram, "u_alpha");
    location_u_size = glGetUniformLocation( mProgram, "u_size");
    location_u_color_matrix = glGetUniformLocation( mProgram, "u_color_matrix");
    location_u_texture_size = glGetUniformLocation( mProgram, "u_texture_size");
}


void NexShaderDropShadowFirst::setUniforms()
{
    glUniformMatrix4fv(location_u_tex_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_mvp_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_color_matrix, 1, GL_FALSE, &matrix4f_identity[0]);

    glUniform1f(location_u_alpha, mAlpha );
    glUniform1f(location_u_size, (GLfloat)mSize );

    glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId);
    glUniform1i(location_u_sTexture0, 0);

    GLfloat size[2] = { (GLfloat)mTexWidth, (GLfloat)mTexHeight };
    glUniform2fv(location_u_texture_size, 1, &size[0]);
}




///////////////////


const char* NexShaderDropShadowSecond::mDropShadow_2ndFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;
uniform highp float u_alpha;
uniform highp mat4 u_color_matrix;
uniform highp vec2 u_v2Resolution;

uniform highp float u_size;
uniform highp float u_softness;
uniform highp float u_glowColAlpha;
uniform highp vec4  u_glowTone;

varying highp vec2 v_v2TexCoord;

//#define MAX_DISTANCE 255.0
                                                                                
highp float decodeScreenCoordinate(highp vec2 channels) {
    return channels.x * 100. + channels.y;
}

void main(void)
{
    highp vec4 color;
    
    highp float size = floor(u_size);
    highp float softness;
    highp float startFading;
    
    //if (u_softness == 0.0)
    //    u_softness = 1.0;
    highp float div;
    div = -0.3 * u_softness + 50.0;
    softness = size * u_softness / div;
    startFading = size - softness;
    
    
    highp float texelY = (1.0 / u_v2Resolution.y);
    
    highp float minDistOut = 255.0;
    highp float xdistOut;
    highp float i;
    highp float fLength;
    highp vec4 decCol;
    for (i = -size; i<size; i+=1.0) {
        decCol = texture2D(u_sTexture0, vec2(v_v2TexCoord.x, v_v2TexCoord.y + i*texelY));
        
        xdistOut = decodeScreenCoordinate(decCol.rg) * 255.0;
        fLength = length(vec2(xdistOut, abs(i)));
        minDistOut = min(minDistOut, fLength);
    }
    
    highp float mixValue = clamp((minDistOut - startFading) / softness, 0.0, 1.0);
    
    
    highp vec4 glowTone = vec4(u_glowTone.rgb,1.0);
    highp vec4 glowColor = glowTone * u_glowTone.a * u_glowColAlpha;
    
#if 0
    highp vec4 oriCol = texture2D(u_sTexture1, v_v2TexCoord);
    glowColor = mix(glowColor, oriCol, mixValue);
#else
    glowColor = mix(glowColor, vec4(0.0), mixValue);
#endif
    
    //highp vec4 oriCol = texture2D(u_sTexture1, v_v2TexCoord) ;
    color = glowColor;// mix(glowColor, vec4(0.0), oriCol.a);
    
    
    color = color * u_color_matrix;
    gl_FragColor = color * u_alpha;
    
}

);


NexShaderDropShadowSecond::NexShaderDropShadowSecond()
: mTexId0(-1)
, mTexId1(-1)
, mTexWidth(0)
, mTexHeight(0)
, mAlpha(1.0)
, mSize(15.)
, mSoftness(30.)
, mGlowColAlpha(1.0)
{
    mColor[0] = (GLfloat)0x3A/255.0;
    mColor[1] = (GLfloat)0x3F/255.0;
    mColor[2] = (GLfloat)0xD3/255.0;
    mColor[3] = 1.0;
             
    this->registerFragmentCodeAndUseDefaultVertexCode(mDropShadow_2ndFragmentShaderCode);
    clearLocations();
}

NexShaderDropShadowSecond::~NexShaderDropShadowSecond()
{
    
}

void NexShaderDropShadowSecond::clearLocations()
{
    NexShaderForTextureBase::clearLocations();

    location_u_tex_matrix = -1;
    location_u_mvp_matrix = -1;
    location_u_sTexture0 = -1;
    location_u_sTexture1 = -1;
    location_u_alpha = -1;
    location_u_size = -1;
    location_u_softness = -1;
    location_u_glowColAlpha = -1;
    location_u_glowTone = -1;
    location_u_color_matrix = -1;
    location_u_v2Resolution = -1;
}

void NexShaderDropShadowSecond::prepareUniforms()
{
    // process Uniforms
    location_u_tex_matrix = glGetUniformLocation( mProgram, "u_tex_matrix");
    location_u_mvp_matrix = glGetUniformLocation( mProgram, "u_mvp_matrix");
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
    location_u_sTexture1 = glGetUniformLocation( mProgram, "u_sTexture1");
    location_u_alpha = glGetUniformLocation( mProgram, "u_alpha");
    location_u_size = glGetUniformLocation( mProgram, "u_size");
    location_u_softness = glGetUniformLocation( mProgram, "u_softness");
    location_u_glowColAlpha = glGetUniformLocation( mProgram, "u_glowColAlpha");
    location_u_glowTone = glGetUniformLocation( mProgram, "u_glowTone");
    location_u_color_matrix = glGetUniformLocation( mProgram, "u_color_matrix");
    location_u_v2Resolution = glGetUniformLocation( mProgram, "u_v2Resolution");
}


void NexShaderDropShadowSecond::setUniforms()
{
    glUniformMatrix4fv(location_u_tex_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_mvp_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_color_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    
    glUniform1f(location_u_alpha, mAlpha);
    glUniform1f(location_u_size, (GLfloat)mSize);
    glUniform1f(location_u_softness, (GLfloat)mSoftness);
    glUniform1f(location_u_glowColAlpha, (GLfloat)mGlowColAlpha);

    glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId0);
    glUniform1i(location_u_sTexture0, 0);

    glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId1);
    glUniform1i(location_u_sTexture1, 1);

    //GLfloat toneColor[4] = { (GLfloat)0x3A/255.0, (GLfloat)0x3F/255.0, (GLfloat)0xD3/255.0, 1.0 };
    glUniform4f(location_u_glowTone, mColor[0], mColor[1], mColor[2], mColor[3]);
    
    glUniform2f(location_u_v2Resolution, (GLfloat)mTexWidth, (GLfloat)mTexHeight );
//    GLfloat size[2] = { (GLfloat)mTexWidth, (GLfloat)mTexHeight };
//    glUniform2fv(location_u_v2Resolution, 1, &size[0]);
}



//////////////////////




const char* NexShaderDropShadowThird::mDropShadow_3rdFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;
uniform highp float u_alpha;
uniform highp float u_dist;
uniform highp float u_angle;

uniform highp mat4 u_color_matrix;
uniform highp vec2 u_v2Resolution;
varying highp vec2 v_v2TexCoord;

const highp float M_PI = 3.1415926535897932384626433832795;

void main()
{
    highp vec4 color;
    highp float texelX = (1.0 / u_v2Resolution.x);
    
    highp float alphaX;
    highp float alphaY;
    highp float sign = 1.0;
    if (u_angle == 0.0 || u_angle == 180.0)
    {
        alphaX = 1.0;
        alphaY = 0.0;
    }
    else if (u_angle == 90.0 || u_angle == 270.0)
    {
        alphaX = 0.0;
        alphaY = 1.0;
    }
    else
    {
        if (((u_angle >45.0) && (u_angle <= 135.0)) || ((u_angle >225.0) && (u_angle <= 315.0)))
        {
            alphaX = 1.0 / tan(u_angle*M_PI / 180.0);
            alphaY = 1.0;
        }
        else
        {
            alphaX = 1.0;
            alphaY = tan(u_angle*M_PI / 180.0);
        }
    }
    if ((u_angle > 135.0) && (u_angle <= 315.0))
        sign = -1.0;
    
    highp float off = u_dist*sign;
    highp vec2 coord = v_v2TexCoord + vec2(off / u_v2Resolution.x * alphaX, off / u_v2Resolution.y*alphaY);
    
    coord = clamp(coord,0.0, 1.0 );
    color = texture2D(u_sTexture0, coord);
    highp vec4 oriCol = texture2D(u_sTexture1, v_v2TexCoord);
    
    color = mix(color, oriCol, oriCol.a);
    
    color = color * u_color_matrix;
    
    gl_FragColor =  color * u_alpha;
    
}

);

NexShaderDropShadowThird::NexShaderDropShadowThird()
: mTexId0(-1)
, mTexId1(-1)
, mTexWidth(0)
, mTexHeight(0)
, mAlpha(1.0)
, mDistance(30.)
, mAngle(30.)
{
    this->registerFragmentCodeAndUseDefaultVertexCode(mDropShadow_3rdFragmentShaderCode);
    clearLocations();
}

NexShaderDropShadowThird::~NexShaderDropShadowThird()
{
    
}

void NexShaderDropShadowThird::clearLocations()
{
    NexShaderForTextureBase::clearLocations();
    
    location_u_tex_matrix = -1;
    location_u_mvp_matrix = -1;
    location_u_sTexture0 = -1;
    location_u_sTexture1 = -1;
    location_u_alpha = -1;
    location_u_dist = -1;
    location_u_angle = -1;
    location_u_color_matrix = -1;
    location_u_v2Resolution = -1;
    
}

void NexShaderDropShadowThird::prepareUniforms()
{
    // process Uniforms
    location_u_tex_matrix = glGetUniformLocation( mProgram, "u_tex_matrix");
    location_u_mvp_matrix = glGetUniformLocation( mProgram, "u_mvp_matrix");
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
    location_u_sTexture1 = glGetUniformLocation( mProgram, "u_sTexture1");
    location_u_alpha = glGetUniformLocation( mProgram, "u_alpha");
    location_u_dist = glGetUniformLocation( mProgram, "u_dist");
    location_u_angle = glGetUniformLocation( mProgram, "u_angle");
    location_u_color_matrix = glGetUniformLocation( mProgram, "u_color_matrix");
    location_u_v2Resolution = glGetUniformLocation( mProgram, "u_v2Resolution");

}


void NexShaderDropShadowThird::setUniforms()
{
    glUniformMatrix4fv(location_u_tex_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_mvp_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_color_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    
    glUniform1f(location_u_alpha, mAlpha);
    glUniform1f(location_u_dist, (GLfloat)mDistance);
    glUniform1f(location_u_angle, (GLfloat)mAngle);
    
    glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId0);
    glUniform1i(location_u_sTexture0, 0);
    
    glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId1);
    glUniform1i(location_u_sTexture1, 1);
    
    glUniform2f(location_u_v2Resolution, (GLfloat)mTexWidth, (GLfloat)mTexHeight );
}

const char* NexShaderAntiAlias::mAntiAliasFragmentShaderCode = GLSL(
uniform sampler2D u_sTexture0;
uniform highp float u_alpha;
uniform highp mat4 u_color_matrix;
uniform highp vec2 u_v2Resolution;
varying highp vec2 v_v2TexCoord;


highp vec4 antiAlias_fn(highp vec2 Coord) {
    highp vec4 color;
    highp vec2 vTexel = vec2(1.0 / u_v2Resolution.x, 1.0 / u_v2Resolution.y);

    highp vec2 vNW = (Coord + vec2(-1.0, -1.0)) * vTexel;
    highp vec2 vNE = (Coord + vec2(1.0, -1.0)) * vTexel;
    highp vec2 vSW = (Coord + vec2(-1.0, 1.0)) * vTexel;
    highp vec2 vSE = (Coord + vec2(1.0, 1.0)) * vTexel;
    highp vec2 vM = vec2(Coord * vTexel);

    highp vec3 colNW = texture2D(u_sTexture0, vNW).rgb;
    highp vec3 colNE = texture2D(u_sTexture0, vNE).rgb;
    highp vec3 colSW = texture2D(u_sTexture0, vSW).rgb;
    highp vec3 colSE = texture2D(u_sTexture0, vSE).rgb;
    highp vec4 oriColor = texture2D(u_sTexture0, vM);
    highp vec3 colM = oriColor.rgb;
    highp vec3 luma = vec3(0.299, 0.587, 0.114);
    highp float lumaNW = dot(colNW, luma);
    highp float lumaNE = dot(colNE, luma);
    highp float lumaSW = dot(colSW, luma);
    highp float lumaSE = dot(colSE, luma);
    highp float lumaM = dot(colM, luma);
    highp float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    highp float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    highp vec2 dir;
    highp float dirReduce;
    highp float rcpDirMin;
    highp float SPAN_MAX = 8.0;
    highp float REDUCE_MIN = (1.0/ 128.0);
    highp float REDUCE_MUL = (1.0 / 8.0);

    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *(0.25 * REDUCE_MUL), REDUCE_MIN);

    rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(SPAN_MAX, SPAN_MAX),max(vec2(-SPAN_MAX, -SPAN_MAX),dir * rcpDirMin)) * vTexel;

    highp vec4 colA = 0.5 * (
        texture2D(u_sTexture0, Coord * vTexel + dir * (1.0 / 3.0 - 0.5)) +
        texture2D(u_sTexture0, Coord * vTexel + dir * (2.0 / 3.0 - 0.5)));
    highp vec4 colB = colA * 0.5 + 0.25 * (
        texture2D(u_sTexture0, Coord * vTexel + dir * -0.5) +
        texture2D(u_sTexture0, Coord * vTexel + dir * 0.5));

    highp float lumaB = dot(colB.rgb, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = colA;
    else
        color = colB;

    return color;
}

void main(void)
{
    highp vec4 color;

    color = antiAlias_fn(u_v2Resolution * v_v2TexCoord);
    
    color = color * u_color_matrix;
    gl_FragColor = color * u_alpha;

}
);

NexShaderAntiAlias::NexShaderAntiAlias()
: mTexId(-1)
, mTexWidth(0)
, mTexHeight(0)
, mAlpha(1.0)
{
    this->registerFragmentCodeAndUseDefaultVertexCode(mAntiAliasFragmentShaderCode);
    clearLocations();
}

NexShaderAntiAlias::~NexShaderAntiAlias()
{
}

void NexShaderAntiAlias::clearLocations()
{
    NexShaderForTextureBase::clearLocations();
    
    location_u_tex_matrix = -1;
    location_u_mvp_matrix = -1;
    
    location_u_sTexture0 = -1;
    location_u_alpha = -1;
    location_u_color_matrix = -1;
    location_u_v2Resolution = -1;
}

void NexShaderAntiAlias::prepareUniforms()
{
    // process Uniforms
    location_u_tex_matrix = glGetUniformLocation( mProgram, "u_tex_matrix");
    location_u_mvp_matrix = glGetUniformLocation( mProgram, "u_mvp_matrix");
    location_u_sTexture0 = glGetUniformLocation( mProgram, "u_sTexture0");
    location_u_alpha = glGetUniformLocation( mProgram, "u_alpha");
    location_u_color_matrix = glGetUniformLocation( mProgram, "u_color_matrix");
    location_u_v2Resolution = glGetUniformLocation( mProgram, "u_v2Resolution");

}


void NexShaderAntiAlias::setUniforms()
{
    glUniformMatrix4fv(location_u_tex_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_mvp_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    glUniformMatrix4fv(location_u_color_matrix, 1, GL_FALSE, &matrix4f_identity[0]);
    
    glUniform1f(location_u_alpha, mAlpha);
    
    glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, mTexId);
    glUniform1i(location_u_sTexture0, 0);
    
    glUniform2f(location_u_v2Resolution, (GLfloat)mTexWidth, (GLfloat)mTexHeight );
}


/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/08/23    Draft.
 -----------------------------------------------------------------------------*/
