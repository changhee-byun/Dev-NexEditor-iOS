/******************************************************************************
 * File Name   : NexShadersForText.h
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

#ifndef NexSimpleShader_hpp
#define NexSimpleShader_hpp

#include "NexShaderBase.h"


class NexShaderDropShadowFirst : public NexShaderForTextureBase {
public:
    NexShaderDropShadowFirst();
    virtual ~NexShaderDropShadowFirst();

    void setTex(int tex, int w, int h) { mTexId = tex; mTexWidth = w; mTexHeight = h; }
    void setSize(float size) { mSize = size; };
    
    void setAlpha(float alpha) { mAlpha = alpha; };

//    virtual void prepare();
    
protected:
    static const char* mDropShadow_1stFragmentShaderCode;
private:
    int location_u_tex_matrix;
    int location_u_mvp_matrix;
    int location_u_sTexture0;
    int location_u_alpha;
    int location_u_size;
    int location_u_color_matrix;
    int location_u_texture_size;
//    int location_a_position;
//    int location_a_texCoord;

    int mTexId;
    int mTexWidth;
    int mTexHeight;
    
    float mSize;
    float mAlpha;
    
    
    virtual void clearLocations();

    virtual void prepareUniforms();
    virtual void setUniforms();

};

class NexShaderDropShadowSecond : public NexShaderForTextureBase {
public:
    NexShaderDropShadowSecond();
    virtual ~NexShaderDropShadowSecond();
    
    void setTex(int tex, int w, int h) { mTexId0 = tex; mTexWidth = w; mTexHeight = h; };
    void setOriginalTex(int tex) { mTexId1 = tex; };
    
    void setSize(float size) { mSize = size; };
    void setAlpha(float alpha) { mAlpha = alpha; };
    void setColor(float r, float g, float b, float a) { mColor[0] = r; mColor[1] = g; mColor[2] = b; mColor[3] = a; };
    void setSoftness(float softness) { mSoftness = softness; };
    
protected:
    static const char* mDropShadow_2ndFragmentShaderCode;
private:
    int location_u_tex_matrix;
    int location_u_mvp_matrix;
    int location_u_sTexture0;
    int location_u_sTexture1;
    int location_u_alpha;
    int location_u_size;
    int location_u_softness;
    int location_u_glowColAlpha;
    int location_u_glowTone;
    int location_u_color_matrix;
    int location_u_v2Resolution;
    
    int mTexId0;
    int mTexId1;
    int mTexWidth;
    int mTexHeight;
    
    float mSize;
    float mAlpha;
    float mSoftness;
    float mGlowColAlpha;
    float mColor[4];
    
    virtual void clearLocations();
    
    virtual void prepareUniforms();
    virtual void setUniforms();
};


class NexShaderDropShadowThird : public NexShaderForTextureBase {
public:
    NexShaderDropShadowThird();
    virtual ~NexShaderDropShadowThird();
    
    void setTex(int tex, int w, int h) { mTexId0 = tex; mTexWidth = w; mTexHeight = h; }
    void setOriginalTex(int tex) { mTexId1 = tex; }
    
    void setAlpha(float alpha) { mAlpha = alpha; };
    void setDistance(float distance) { mDistance = distance; };
    void setAngle(float angle) { mAngle = angle; };

protected:
    static const char* mDropShadow_3rdFragmentShaderCode;
    
private:
    int location_u_tex_matrix;
    int location_u_mvp_matrix;
    int location_u_sTexture0;
    int location_u_sTexture1;
    int location_u_alpha;
    int location_u_dist;
    int location_u_angle;
    int location_u_color_matrix;
    int location_u_v2Resolution;
    
    
    int mTexId0;
    int mTexId1;
    int mTexWidth;
    int mTexHeight;
    float mAlpha;
    float mDistance;
    float mAngle;

    virtual void clearLocations();
    
    virtual void prepareUniforms();
    virtual void setUniforms();
};


class NexShaderAntiAlias : public NexShaderForTextureBase {
public:
    NexShaderAntiAlias();
    virtual ~NexShaderAntiAlias();
    void setTex(int tex, int w, int h) { mTexId = tex; mTexWidth = w; mTexHeight = h; }
    void setAlpha(float alpha) { mAlpha = alpha; };

protected:
    static const char* mAntiAliasFragmentShaderCode;
        
private:
    int location_u_tex_matrix;
    int location_u_mvp_matrix;

    int location_u_sTexture0;
    int location_u_alpha;
    int location_u_color_matrix;
    int location_u_v2Resolution;

    int mTexId;
    int mTexWidth;
    int mTexHeight;
    float mAlpha;
    
    virtual void clearLocations();
    
    virtual void prepareUniforms();
    virtual void setUniforms();
};

#endif /* NexSimpleShader_hpp */

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/08/23    Draft.
 -----------------------------------------------------------------------------*/
