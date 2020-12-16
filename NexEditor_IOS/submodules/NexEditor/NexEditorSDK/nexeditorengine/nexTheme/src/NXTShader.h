//
//  NXTShader.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/24/14.
//
//

#ifndef __NexVideoEditor__NXTShader__
#define __NexVideoEditor__NXTShader__

#include <iostream>
#include <vector>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "NXTMatrix.h"
#include "NXTImage.h"
#include "NXTUniform.h"

class NXTVertexAttr;

enum class NXTShaderUniform : int {
    NONE                    = -1,
    u_mvp_matrix            = 0,
    u_tex_matrix            = 1,
    u_mask_matrix           = 2,
    u_colorconv_matrix      = 3,
    u_maskxfm_matrix        = 4,
    u_textureSampler        = 5,
    u_textureSamplerY       = 6,
    u_textureSamplerUV      = 7,
    u_textureSamplerU       = 8,
    u_textureSamplerV       = 9,
    u_alpha                 = 10,
    u_maskSampler           = 11,
    u_color                 = 12,
    u_diffuse_light         = 13,
    u_ambient_light         = 14,
    u_specular_light        = 15,
    u_lightpos              = 16,
    u_light_f0              = 17,
    u_light_f1              = 18,
    u_specexp               = 19,
    COUNT                   = 20
};

enum class NXTShaderAttrib : int {
    NONE                    = -1,
    a_position              = 0,
    a_texCoord              = 1,
    a_normal                = 2,
    a_maskCoord             = 3,
    a_color                 = 4,
    COUNT                   = 5
};

enum class NXTChannelFilter : int {
    Y,U,V,ALL
};

class NXTShaderInput {
    
public:
    const char* source_;
    NXTMatrix matrix_;
    
    NXTShaderInput( const char* source ) {
        source_ = source;
    }
    
    NXTShaderInput( const NXTMatrix& matrix, const char* source ) {
        source_ = source;
        matrix_ = matrix;
    }
    
};
    
class NXTShader {
    
private:
    std::string error_log_;
    GLuint uniforms_[static_cast<int>(NXTShaderUniform::COUNT)];
    GLuint attributes_[static_cast<int>(NXTShaderAttrib::COUNT)];
    bool valid_ = false;
    GLuint shader_program_;
    

    
public:
    
    NXTShader(NXTShader const&) = delete;
    NXTShader& operator=(NXTShader const&) = delete;
    
    NXTShader(const std::string& name,
              const std::string& vbody,
              const NXTShaderInput& fin,
              const std::string& fbody,
              const std::string& fout);
    
    ~NXTShader();
    
    bool isValid() {
        return valid_;
    }
    
    const std::string& getErrorLog() {
        return error_log_;
    }
    
//    void set_uniform(NXTShaderUniform uniform, GLfloat v0);
//    void set_uniform(NXTShaderUniform uniform, GLfloat v0, GLfloat v1);
//    void set_uniform(NXTShaderUniform uniform, GLfloat v0, GLfloat v1, GLfloat v2);
//    void set_uniform(NXTShaderUniform uniform, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v4);
//    void set_uniform(NXTShaderUniform uniform, GLint v0);
//    void set_uniform(NXTShaderUniform uniform, GLint v0, GLint v1);
//    void set_uniform(NXTShaderUniform uniform, GLint v0, GLint v1, GLint v2);
//    void set_uniform(NXTShaderUniform uniform, GLint v0, GLint v1, GLint v2, GLint v4);
//    void set_uniform(NXTShaderUniform uniform, const NXTMatrix& matrix );

    // TODO: VertexAttr class needs to be updated
    NXTVertexAttr& vertexLocations();
    NXTVertexAttr& vertexColors();
    NXTVertexAttr& vertexTexCoords();
    NXTVertexAttr& vertexMaskCoords();
    NXTVertexAttr& vertexNormals();
    
    NXTUniformMatrix mvpMatrix() {
        return NXTUniformMatrix(uniforms_[static_cast<int>(NXTShaderUniform::u_mvp_matrix)]);
    }
    NXTUniformMatrix texMatrix() {
        return NXTUniformMatrix(uniforms_[static_cast<int>(NXTShaderUniform::u_tex_matrix)]);
    }
    NXTUniformMatrix maskMatrix() {
        return NXTUniformMatrix(uniforms_[static_cast<int>(NXTShaderUniform::u_mask_matrix)]);
    }
    NXTUniformMatrix colorConvMatrix() {
        return NXTUniformMatrix(uniforms_[static_cast<int>(NXTShaderUniform::u_colorconv_matrix)]);
    }
    NXTUniformMatrix maskXfmMatrix() {
        return NXTUniformMatrix(uniforms_[static_cast<int>(NXTShaderUniform::u_maskxfm_matrix)]);
    }
    NXTUniformVec<3> lightPos() {
        return NXTUniformVec<3>(uniforms_[static_cast<int>(NXTShaderUniform::u_lightpos)]);
    }
    NXTUniformVec<4> color() {
        return NXTUniformVec<4>(uniforms_[static_cast<int>(NXTShaderUniform::u_color)]);
    }
    NXTUniformVec<4> diffuseLight() {
        return NXTUniformVec<4>(uniforms_[static_cast<int>(NXTShaderUniform::u_diffuse_light)]);
    }
    NXTUniformVec<4> ambientLight() {
        return NXTUniformVec<4>(uniforms_[static_cast<int>(NXTShaderUniform::u_ambient_light)]);
    }
    NXTUniformVec<4> specularLight() {
        return NXTUniformVec<4>(uniforms_[static_cast<int>(NXTShaderUniform::u_specular_light)]);
    }
    NXTUniformSampler2D textureSampler() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_textureSampler)]);
    }
    NXTUniformSampler2D textureSamplerY() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_textureSamplerY)]);
    }
    NXTUniformSampler2D textureSamplerUV() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_textureSamplerUV)]);
    }
    NXTUniformSampler2D textureSamplerU() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_textureSamplerU)]);
    }
    NXTUniformSampler2D textureSamplerV() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_textureSamplerV)]);
    }
    NXTUniformSampler2D maskSampler() {
        return NXTUniformSampler2D(uniforms_[static_cast<int>(NXTShaderUniform::u_maskSampler)]);
    }
    NXTUniformFloat alpha() {
        return NXTUniformFloat(uniforms_[static_cast<int>(NXTShaderUniform::u_alpha)]);
    }
    NXTUniformFloat lightF0() {
        return NXTUniformFloat(uniforms_[static_cast<int>(NXTShaderUniform::u_light_f0)]);
    }
    NXTUniformFloat lightF1() {
        return NXTUniformFloat(uniforms_[static_cast<int>(NXTShaderUniform::u_light_f1)]);
    }
    NXTUniformFloat lightSpecExp() {
        return NXTUniformFloat(uniforms_[static_cast<int>(NXTShaderUniform::u_specexp)]);
    }
};

class NXTShaderInstances {
    
public:
    
    NXTShaderInstances();
    
    NXTShader ext_to_rgba;
    NXTShader rgba_to_rgba;
    NXTShader rgba_to_stencil;
    NXTShader yuv_to_rgba;
    NXTShader nv12_to_rgba;
    NXTShader nv12_to_yuva8888_y;
    NXTShader nv12_to_yuva8888_u;
    NXTShader nv12_to_yuva8888_v;
    NXTShader nv12_to_yuva8888_all;
    NXTShader rgb_to_yuva8888_y;
    NXTShader rgb_to_yuva8888_u;
    NXTShader rgb_to_yuva8888_v;
    NXTShader rgb_to_yuva8888_all;
    NXTShader ext_to_yuva8888_y;
    NXTShader ext_to_yuva8888_u;
    NXTShader ext_to_yuva8888_v;
    NXTShader ext_to_yuva8888_all;
    
    NXTShader& nv12_to_yuva8888(NXTChannelFilter cf) {
        switch( cf ) {
            case NXTChannelFilter::Y:   return nv12_to_yuva8888_y;
            case NXTChannelFilter::U:   return nv12_to_yuva8888_u;
            case NXTChannelFilter::V:   return nv12_to_yuva8888_v;
            case NXTChannelFilter::ALL: return nv12_to_yuva8888_all;
            default: throw "invalid channel filter";
        }
    }
    
    NXTShader& rgb_to_yuva8888(NXTChannelFilter cf) {
        switch( cf ) {
            case NXTChannelFilter::Y:   return rgb_to_yuva8888_y;
            case NXTChannelFilter::U:   return rgb_to_yuva8888_u;
            case NXTChannelFilter::V:   return rgb_to_yuva8888_v;
            case NXTChannelFilter::ALL: return rgb_to_yuva8888_all;
            default: throw "invalid channel filter";
        }
    }
    
    NXTShader& ext_to_yuva8888(NXTChannelFilter cf) {
        switch( cf ) {
            case NXTChannelFilter::Y:   return ext_to_yuva8888_y;
            case NXTChannelFilter::U:   return ext_to_yuva8888_u;
            case NXTChannelFilter::V:   return ext_to_yuva8888_v;
            case NXTChannelFilter::ALL: return ext_to_yuva8888_all;
            default: throw "invalid channel filter";
        }
    }
    
    
//    NXTShader const *test[4] = {&ext_to_rgba,&rgba_to_rgba};
    
};

#endif /* defined(__NexVideoEditor__NXTShader__) */

