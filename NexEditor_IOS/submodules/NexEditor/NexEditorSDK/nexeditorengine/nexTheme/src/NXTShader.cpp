#include "NXTShader.h"


// ==== Shader Inputs ==================================================

static const NXTShaderInput fin_solid_rgb(R"(

    highp vec4 get_color() {
        return vec4(0.0,0.0,0.0,0.0);
    }

)");

static const NXTShaderInput fin_textured_rgba(R"(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;

    highp vec4 get_color() {
        return (texture2D(u_textureSampler, v_texCoord)).bgra;
    }

)");

static const NXTShaderInput fin_textured_yuv(R"(
                                             
    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerU;
    uniform sampler2D u_textureSamplerV;

    highp vec4 get_color() {
        const highp mat4 yuvrgb = mat4(	1.000,  0.000,  1.402, -0.701,
                                       1.000, -0.334, -0.714,  0.529,
                                       1.000,  1.772,  0.000, -0.886,
                                       0.000,  0.000,  0.000,  1.000);
        return = vec4(texture2D(u_textureSamplerY,v_texCoord).r,
                      texture2D(u_textureSamplerU,v_texCoord).r,
                      texture2D(u_textureSamplerV,v_texCoord).r,
                      1.0)*yuvrgb;
    }
 
)");

static const NXTShaderInput fin_textured_nv12( NXTMatrix::biased_yuv_to_rgb(), R"(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;

    highp vec4 get_color() {
        highp vec4 color;
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        color.r = texture2D(u_textureSamplerY,v_texCoord).r * s;
        color.g = texture2D(u_textureSamplerUV,v_texCoord).r * s;
        color.b = texture2D(u_textureSamplerUV,v_texCoord).a * s;
        color.a = 1.0;
        return color;
    }

)");


static const NXTShaderInput fin_textured_external( R"(

    uniform samplerExternalOES u_textureSampler;

    highp vec4 get_color() {
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
        highp vec4 color = texture2D(u_textureSampler, v_texCoord);
        return vec4(color.a*s, color.a*s, color.a*s, 1.0);
    }

)");

// ==== Shader Outputs ==================================================

static const std::string fout_rgb = R"(

    void out_color( highp vec4 color ) {
        gl_FragColor = color;
    }

)";

static const std::string fout_yuva8888_y = R"(

    void out_color( highp vec4 color ) {
        const highp mat4 rgbyuv = mat4(
                                       0.300,  0.589,  0.111, -0.003,
                                       -0.169, -0.332,  0.502,  0.502,
                                       0.499, -0.420, -0.079,  0.502,
                                       0.000,  0.000,  0.000,  1.000);
        gl_FragColor = (color * rgbyuv).rrra;
    }

)";

static const std::string fout_yuva8888_u = R"(

    void out_color( highp vec4 color ) {
        const highp mat4 rgbyuv = mat4(
                                       0.300,  0.589,  0.111, -0.003,
                                       -0.169, -0.332,  0.502,  0.502,
                                       0.499, -0.420, -0.079,  0.502,
                                       0.000,  0.000,  0.000,  1.000);
        gl_FragColor = (color * rgbyuv).ggga;
    }

)";


static const std::string fout_yuva8888_v = R"(

    void out_color( highp vec4 color ) {
        const highp mat4 rgbyuv = mat4(
                                       0.300,  0.589,  0.111, -0.003,
                                       -0.169, -0.332,  0.502,  0.502,
                                       0.499, -0.420, -0.079,  0.502,
                                       0.000,  0.000,  0.000,  1.000);
        gl_FragColor = (color * rgbyuv).bbba;
    }

)";

static const std::string fout_yuva8888_all = R"(

void out_color( highp vec4 color ) {
    const highp mat4 rgbyuv = mat4(
                                   0.300,  0.589,  0.111, -0.003,
                                   -0.169, -0.332,  0.502,  0.502,
                                   0.499, -0.420, -0.079,  0.502,
                                   0.000,  0.000,  0.000,  1.000);
    gl_FragColor = (color * rgbyuv);
}

)";


static const std::string fout_stencil = R"(
    void out_color( highp vec4 color ) {
        if( color.a < 0.5 ) {
            discard;
        } else {
            gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    }
)";

// ==== Shader Body ==================================================

static const std::string fbody_normal = R"(

    uniform highp float 		u_alpha;
    uniform highp mat4			u_colorconv_matrix;
    uniform sampler2D           u_maskSampler;
    uniform highp mat4          u_maskxfm_matrix;
    varying highp vec2          v_maskCoord;
    varying highp vec4          v_color;
    uniform highp vec4          u_color;

    // ---- Lighting ----
    uniform highp vec4          u_diffuse_light;
    uniform highp vec4          u_ambient_light;
    uniform highp vec4          u_specular_light;
    varying highp float         v_diffuse_intensity;
    varying highp float         v_specular_intensity;

    highp vec4 applyLighting( highp vec4 color ) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }

    void main() {
        
        highp float mask = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm_matrix).r;
        
        out_color( mask * u_alpha * applyLighting(clamp( (get_color()+v_color+u_color) * u_colorconv_matrix , 0.0, 1.0)));
        
    }


)";

// ==== Vertex Shader ==================================================

static const std::string vbody_normal = R"(

    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_normal;
    attribute vec2 a_maskCoord;
    attribute vec2 a_color;
    uniform mat4 u_mvp_matrix;
    uniform mat4 u_tex_matrix;
    uniform mat4 u_mask_matrix;
    varying highp vec2 v_texCoord;
    varying highp vec4 v_color;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {
        
        highp vec3 light_direction;
        highp vec3 half_plane;
        
        v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;
        v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;
        v_color = a_color;
        gl_Position = a_position * u_mvp_matrix;
        
        // Lighting
        highp vec3 normal = (a_normal*u_mvp_matrix).xyz;
        light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
        half_plane = normalize(light_direction + vec3(0.0,0.0,1.0));
        highp float ndl = dot(normal,light_direction);
        v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
        v_specular_intensity = pow(max(0.0, dot(normal,half_plane)),u_specexp);
    }


)";

// ======================================================================

static bool check_gl_error(std::string& error_log, const char* location) {
    bool isError = false;
    GLint error;
    for (error = glGetError(); error; error = glGetError()) {
        error_log += "[";
        error_log += location;
        error_log += "] OpenGL Error #";
        error_log += error;
        switch( error ) {
            case GL_NO_ERROR:
                error_log += " (GL_NO_ERROR)\n";
                break;
            case GL_INVALID_ENUM:
                error_log += " (GL_INVALID_ENUM)\n";
                break;
            case GL_INVALID_VALUE:
                error_log += " (GL_INVALID_VALUE)\n";
                break;
            case GL_INVALID_OPERATION:
                error_log += " (GL_INVALID_OPERATION)\n";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error_log += " (GL_INVALID_FRAMEBUFFER_OPERATION)\n";
                break;
            case GL_OUT_OF_MEMORY:
                error_log += " (GL_OUT_OF_MEMORY)\n";
                break;
            default:
                error_log += " (?)\n";
                break;
        }
        isError = true;
    }
    return isError;
}

static void getShaderInfoLog( GLuint hShader, std::string& out_infoLog ) {
    GLint infoLogLength = 0;
    glGetShaderiv(hShader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if( infoLogLength < 1 ) {
        infoLogLength = 4096;
    }
    char infoLogString[infoLogLength];
    glGetShaderInfoLog(hShader, infoLogLength, NULL, infoLogString);
    out_infoLog = infoLogString;
}

static void getProgramInfoLog( GLuint hProgram, std::string& out_infoLog ) {
    GLint infoLogLength = 0;
    glGetProgramiv(hProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
    if( infoLogLength < 1 ) {
        infoLogLength = 4096;
    }
    char infoLogString[infoLogLength];
    glGetProgramInfoLog(hProgram, infoLogLength, NULL, infoLogString);
    out_infoLog = infoLogString;
}

static unsigned int shaderCompileStatus( GLuint hShader ) {
    GLint compileStatus = 0;
    glGetShaderiv(hShader, GL_COMPILE_STATUS, &compileStatus);
    return compileStatus;
}

static unsigned int shaderProgramLinkStatus( GLuint hProgram ) {
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(hProgram, GL_LINK_STATUS, &linkStatus);
    return linkStatus;
}


template<int NUM_ELEMENTS>
static GLuint loadShader(GLenum shaderType,
                         std::string& error_log,
                         const char* (&pCode)[NUM_ELEMENTS])
{
    
    check_gl_error(error_log,"loadShader:in");
    
    if( shaderType!=GL_FRAGMENT_SHADER && shaderType!=GL_VERTEX_SHADER ) {
        error_log += "Unknown shader type: ";
        error_log += shaderType;
        error_log += "\n";
        return 0;
    }
    
    GLuint hShader = glCreateShader(shaderType);
    if( check_gl_error(error_log,"loadShader:glCreateShader")) {
        return 0;
    } else if(!hShader) {
        error_log += "Invalid handle from glCreateShader\n";
        return 0;
    }
    
    glShaderSource(hShader, NUM_ELEMENTS, pCode, NULL);
    check_gl_error(error_log,"loadShader:glShaderSource");
    glCompileShader(hShader);
    check_gl_error(error_log,"loadShader:glCompileShader");
    if( !shaderCompileStatus(hShader) )
    {
        std::string compileErrors;
        getShaderInfoLog(hShader, compileErrors);
        if( compileErrors.length()>0 ) {
            glDeleteShader(hShader);
            
            error_log += "------------ Error(s) detected compiling ";
            
            
            if( shaderType==GL_VERTEX_SHADER ) {
                error_log += "VERTEX ";
            } else if( shaderType==GL_FRAGMENT_SHADER ) {
                error_log =+ "FRAGMENT ";
            }
            
            error_log += " shader ------------\n";
            error_log += compileErrors;
            error_log += "\n-------------------------------------------------------------------\n\n";
            return 0;
        }
    }
    return hShader;
}

template <typename ...Args>
static GLuint loadShader(GLenum shaderType,
                         std::string& error_log,
                         Args... args)
{
    const char* shader_source[] = {args...};
    return loadShader(shaderType, error_log, shader_source);
}

class NXTShaderParam {
    
public:
    const char* name_;
    NXTShaderUniform uniform_;
    NXTShaderAttrib attrib_;
    NXTShaderParam(NXTShaderUniform uniform, const char* name) {
        uniform_ = uniform;
        attrib_ = NXTShaderAttrib::NONE;
        name_ = name;
    }
    NXTShaderParam(NXTShaderAttrib attrib, const char* name) {
        uniform_ = NXTShaderUniform::NONE;
        attrib_ = attrib;
        name_ = name;
    }
};

static NXTShaderParam shader_param_table[] = {
    {NXTShaderUniform::u_mvp_matrix,        "u_mvp_matrix"},
    {NXTShaderUniform::u_tex_matrix,        "u_tex_matrix"},
    {NXTShaderUniform::u_mask_matrix,       "u_mask_matrix"},
    {NXTShaderUniform::u_colorconv_matrix,  "u_colorconv_matrix"},
    {NXTShaderUniform::u_maskxfm_matrix,    "u_maskxfm_matrix"},
    {NXTShaderUniform::u_textureSampler,    "u_textureSampler"},
    {NXTShaderUniform::u_textureSamplerY,   "u_textureSamplerY"},
    {NXTShaderUniform::u_textureSamplerUV,  "u_textureSamplerUV"},
    {NXTShaderUniform::u_textureSamplerU,   "u_textureSamplerU"},
    {NXTShaderUniform::u_textureSamplerV,   "u_textureSamplerV"},
    {NXTShaderUniform::u_alpha,             "u_alpha"},
    {NXTShaderUniform::u_maskSampler,       "u_maskSampler"},
    {NXTShaderUniform::u_color,             "u_color"},
    {NXTShaderUniform::u_diffuse_light,     "u_diffuse_light"},
    {NXTShaderUniform::u_ambient_light,     "u_ambient_light"},
    {NXTShaderUniform::u_specular_light,    "u_specular_light"},
    {NXTShaderUniform::u_lightpos,          "u_lightpos"},
    {NXTShaderUniform::u_light_f0,          "u_light_f0"},
    {NXTShaderUniform::u_light_f1,          "u_light_f1"},
    {NXTShaderUniform::u_specexp,           "u_specexp"},
    {NXTShaderAttrib::a_position,           "a_position"},
    {NXTShaderAttrib::a_texCoord,           "a_texCoord"},
    {NXTShaderAttrib::a_normal,             "a_normal"},
    {NXTShaderAttrib::a_maskCoord,          "a_maskCoord"},
    {NXTShaderAttrib::a_color,              "a_color"},
};



NXTShader::NXTShader(const std::string& programName,
                     const std::string& vbody,
                     const NXTShaderInput& fin,
                     const std::string& fbody,
                     const std::string& fout)
{
    error_log_ = "";
    GLuint hVertexShader = loadShader(GL_VERTEX_SHADER, error_log_, vbody.c_str());
    GLuint hFragmentShader = loadShader(GL_FRAGMENT_SHADER, error_log_,
                                        fin.source_, fout.c_str(), fbody.c_str());
    
    if( !hVertexShader ) {
        error_log_ += "Failed to load vertex shader: ";
        error_log_ += programName;
        error_log_ += "\n";
        if( hFragmentShader ) {
            glDeleteShader(hFragmentShader);
        }
        valid_ = false;
        return;
    }
    if( !hFragmentShader ) {
        error_log_ += "Failed to load fragment shader: ";
        error_log_ += programName;
        error_log_ += "\n";
        if( hVertexShader ) {
            glDeleteShader(hVertexShader);
        }
        valid_ = false;
        return;
    }
    
    GLuint hShaderProgram = glCreateProgram();
    check_gl_error(error_log_,"NXTShader:glCreateProgram");
    if( !hShaderProgram ) {
        error_log_ += "Failed to create shader program: ";
        error_log_ += programName;
        error_log_ += "\n";
        glDeleteShader(hFragmentShader);
        glDeleteShader(hVertexShader);
        valid_ = false;
        return;
    }
    
    glAttachShader(hShaderProgram, hVertexShader);
    check_gl_error(error_log_,"NXTShader:glAttachShader/vertex");
    glAttachShader(hShaderProgram, hFragmentShader);
    check_gl_error(error_log_,"NXTShader:glAttachShader/fragment");
    
    glLinkProgram(hShaderProgram);
    check_gl_error(error_log_,"NXTShader:glLinkProgram");

    if( shaderProgramLinkStatus(hShaderProgram)!=GL_TRUE ) {
        std::string linkErrors;
        getProgramInfoLog(hShaderProgram, linkErrors);
        if( linkErrors.length()>0 ) {
            error_log_ += "------------ Error(s) detected linking shader: ";
            error_log_ += programName;
            error_log_ += " ------------\n";
            error_log_ += linkErrors;
            error_log_ += "\n-------------------------------------------------------------------\n\n";
        }
        valid_ = false;
    } else {
    
        for( NXTShaderParam& shader_param: shader_param_table ) {
            if( shader_param.attrib_ != NXTShaderAttrib::NONE ) {
                attributes_[static_cast<int>(shader_param.attrib_)]
                    = glGetAttribLocation(hShaderProgram,shader_param.name_);
            }
            if( shader_param.uniform_ != NXTShaderUniform::NONE ) {
                uniforms_[static_cast<int>(shader_param.uniform_)]
                    = glGetUniformLocation(hShaderProgram,shader_param.name_);
            }
        }
        valid_ = true;
    }
    
    glDetachShader(hShaderProgram, hVertexShader);
    check_gl_error(error_log_,"NXTShader:glDetachShader/vertex");
    glDetachShader(hShaderProgram, hFragmentShader);
    check_gl_error(error_log_,"NXTShader:glDetachShader/fragment");
    
    glDeleteShader(hVertexShader);
    glDeleteShader(hFragmentShader);
    
    if( valid_ ) {
        shader_program_ = hShaderProgram;
    } else {
        glDeleteProgram(hShaderProgram);
    }
}

NXTShader::~NXTShader() {
    if( valid_ ) {
        glDeleteProgram(shader_program_);
    }
}

NXTShaderInstances::NXTShaderInstances() :

    ext_to_rgba(            "ext_to_rgba",        vbody_normal, fin_textured_external, fbody_normal,fout_rgb),
    rgba_to_rgba(           "rgba_to_rgba",       vbody_normal, fin_textured_rgba,     fbody_normal,fout_rgb),
    rgba_to_stencil(        "rgba_to_stencil",    vbody_normal, fin_textured_rgba,     fbody_normal,fout_stencil),
    yuv_to_rgba(            "yuv_to_rgba",        vbody_normal, fin_textured_yuv,      fbody_normal,fout_rgb),
    nv12_to_rgba(           "nv12_to_rgba",       vbody_normal, fin_textured_nv12,     fbody_normal,fout_rgb),
    nv12_to_yuva8888_y(     "nv12_to_yuva8888_y", vbody_normal, fin_textured_nv12,     fbody_normal,fout_yuva8888_y),
    nv12_to_yuva8888_u(     "nv12_to_yuva8888_u", vbody_normal, fin_textured_nv12,     fbody_normal,fout_yuva8888_u),
    nv12_to_yuva8888_v(     "nv12_to_yuva8888_v", vbody_normal, fin_textured_nv12,     fbody_normal,fout_yuva8888_v),
    nv12_to_yuva8888_all(   "nv12_to_yuva8888_all", vbody_normal, fin_textured_nv12,     fbody_normal,fout_yuva8888_all),
    rgb_to_yuva8888_y(      "rgb_to_yuva8888_y",  vbody_normal, fin_textured_rgba,     fbody_normal,fout_yuva8888_y),
    rgb_to_yuva8888_u(      "rgb_to_yuva8888_u",  vbody_normal, fin_textured_rgba,     fbody_normal,fout_yuva8888_u),
    rgb_to_yuva8888_v(      "rgb_to_yuva8888_v",  vbody_normal, fin_textured_rgba,     fbody_normal,fout_yuva8888_v),
    rgb_to_yuva8888_all(    "rgb_to_yuva8888_all",  vbody_normal, fin_textured_rgba,     fbody_normal,fout_yuva8888_all),
    ext_to_yuva8888_y(      "ext_to_yuva8888_y",  vbody_normal, fin_textured_external, fbody_normal,fout_yuva8888_y),
    ext_to_yuva8888_u(      "ext_to_yuva8888_u",  vbody_normal, fin_textured_external, fbody_normal,fout_yuva8888_u),
    ext_to_yuva8888_v(      "ext_to_yuva8888_v",  vbody_normal, fin_textured_external, fbody_normal,fout_yuva8888_v),
    ext_to_yuva8888_all(    "ext_to_yuva8888_all",  vbody_normal, fin_textured_external, fbody_normal,fout_yuva8888_all)

{

}


NXTVertexAttr& NXTShader::vertexLocations() {
    // TODO
}
NXTVertexAttr& NXTShader::vertexColors() {
    // TODO
}
NXTVertexAttr& NXTShader::vertexTexCoords() {
    // TODO
}
NXTVertexAttr& NXTShader::vertexMaskCoords() {
    // TODO
}
NXTVertexAttr& NXTShader::vertexNormals() {
    // TODO
}

//void NXTShader::uniform1f(NXTShaderUniform uniform, GLfloat v0) {
//    GLint u = uniforms_[static_cast<GLint>(uniform)];
//    if( u != -1 ) {
//        glUniform1f(u, v0);
//    }
//}


