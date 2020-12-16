//
//  NexThemeRenderer_Internal.h
//  NexThemeTest
//
//  Created by Matthew Feinberg on 9/8/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NexThemeTest_NexThemeRenderer_Internal_h
#define NexThemeTest_NexThemeRenderer_Internal_h

#include "NexTheme_Config.h"
#include "NexThemeRenderer_Platform.h"
#define NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme.h"
#undef NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme_Nodes.h"
#include "NexTheme_Math.h"
#include "NexSAL_Internal.h"

#if defined(ANDROID)
#include <android/log.h>
#include <android/native_window.h>
#elif defined(__APPLE__)
// FIXME: Include these headers at more appropirate place that here.
// Not including NexThemeRenderer_Internal_ios.h causes side effect that following headers are not included for where strcasecmp/malloc are used.
#include <strings.h>
#include <stdlib.h>
#endif

#include <pthread.h>

#ifdef NEXTHEME_SUPPORT_OPENGL

typedef struct NXT_ShaderField_ {
    GLuint *fieldId;
    const char   *fieldName;
} NXT_ShaderField;

typedef struct NXT_Shader_LightingInfo_ {
	GLuint u_diffuse_light;		// Diffuse light color
    GLuint u_ambient_light;		// Ambient light color
    GLuint u_specular_light;	// Specular light color
    GLuint u_lightpos;			// Position of light source (eye coordinates)
    GLuint u_specexp;			// Specular exponent
    GLuint u_light_f0;			// Lighting surface normal multilication factor 0; Should be 1, 0 or -1 (and should not be the same as u_light_f1)
    GLuint u_light_f1;			// Lighting surface normal multilication factor 1; Should be 1, 0 or -1 (and should not be the same as u_light_f0)
    GLuint a_normal;			// Vertex normals

    NXT_Shader_LightingInfo_():u_diffuse_light(0), u_ambient_light(0), u_specular_light(0), u_lightpos(0), u_specexp(0), u_light_f0(0), u_light_f1(0), a_normal(0){

    }
} NXT_Shader_LightingInfo;

typedef struct NXT_ShaderProgram_Base_ {
    GLuint shaderProgram;
    GLuint shaderVertex;
    GLuint shaderFragment;
    int loaded;
    char *programName;
    char *vertexSource;
    char *fragmentSource;
    NXT_Shader_LightingInfo *lightingInfo;
    char *macroDefs;
    NXT_ShaderField* uniforms;
    NXT_ShaderField* attributes;

    NXT_ShaderProgram_Base_()
    :shaderProgram(0), shaderVertex(0), shaderFragment(0), 
    loaded(0), programName(NULL), vertexSource(NULL), fragmentSource(NULL), lightingInfo(NULL), macroDefs(NULL), uniforms(NULL), attributes(NULL){

    }
} NXT_ShaderProgram_Base;

typedef struct NXT_ShaderProgram_Solid_ {
    // Shader Program
    NXT_ShaderProgram_Base base;
    // Uniforms
    GLuint u_mvp_matrix;
    GLuint u_alpha;
    GLuint u_colorconv;
    // Attributes
    GLuint a_position;
    GLuint a_color;
    // Lighting
    NXT_Shader_LightingInfo lightingInfo;
    
    //Screen Masking
    GLuint u_textureSampler_for_mask;
    GLuint u_inverse_Mask;
    
    GLuint u_hue;
    NXT_ShaderProgram_Solid_() : u_mvp_matrix(0), u_alpha(0), u_colorconv(0), a_position(0), a_color(0), u_textureSampler_for_mask(0), u_inverse_Mask(0), u_hue(0){
    }
} NXT_ShaderProgram_Solid;

typedef struct NXT_ShaderProgram_Textured_ {
    // Shader Program
    NXT_ShaderProgram_Base base;

    // Uniforms
    GLuint u_mvp_matrix;
    GLuint u_homo_matrix;
    GLuint u_tex_matrix;
    GLuint u_back_tex_matrix;//for rendering background image - etc)blur, mosaic...
    GLuint u_alpha;
    GLuint u_colorconv;
	GLuint u_realY;
	GLuint u_realX;
    GLuint u_moveX_value;
    GLuint u_moveY_value;
    // Uniform - for HDR10
    GLuint u_MaximumContentLightLevel;   //Maximum Content Light Level for HDR10
    GLuint u_DeviceLightLevel;  //Maximum Frame Average Light Level for HDR10
    GLuint u_Gamma;
    // Uniform - Sampler for RGB program
    GLuint u_textureSampler;
    GLuint u_back_textureSampler;//for rendering background image - etc)blur, mosaic...
    // Uniforms - Samplers for YUV/NV12 program
    GLuint u_textureSamplerY;   // Y in YUV and NV12
    GLuint u_textureSamplerU;   // U in YUV
    GLuint u_textureSamplerV;   // V in YUV
    GLuint u_textureSamplerUV;  // Interleaved UV in NV12
    // Attributes
    GLuint a_position;
    GLuint a_texCoord;
    // Lighting
    NXT_Shader_LightingInfo lightingInfo;
    //Screen Masking
    GLuint u_textureSampler_for_mask;
    GLuint u_inverse_Mask;

    GLuint u_sharpness;
    GLuint u_vignette;
    GLuint u_vignetteRange;
    GLuint u_resolution;
    GLuint u_hue;
    
    NXT_ShaderProgram_Textured_():u_mvp_matrix(0), u_homo_matrix(0), 
        u_tex_matrix(0), u_back_tex_matrix(0), u_alpha(0), u_colorconv(0), u_realX(0), u_realY(0), u_moveX_value(0), u_moveY_value(0),  u_MaximumContentLightLevel(0), u_DeviceLightLevel(0), u_Gamma(0),
        u_textureSampler(0), u_back_textureSampler(0), u_textureSamplerY(0), u_textureSamplerU(0), u_textureSamplerV(0), u_textureSamplerUV(0), 
        a_position(0), a_texCoord(0),
        u_textureSampler_for_mask(0), u_inverse_Mask(0), u_hue(0),
        u_sharpness(0), u_vignette(0), u_vignetteRange(0), u_resolution(0)
        {

        }
} NXT_ShaderProgram_Textured;

typedef struct NXT_ShaderProgram_Masked_ {
    // Shader Program
    NXT_ShaderProgram_Base base;

    // Uniforms
    GLuint u_mvp_matrix;
    GLuint u_homo_matrix;
    GLuint u_mask_homo_matrix;
    GLuint u_tex_matrix;
    GLuint u_back_tex_matrix;//for rendering background image - etc)blur, mosaic...
    GLuint u_mask_matrix;
    GLuint u_maskSampler;
    GLuint u_alpha;
    GLuint u_color;
    GLuint u_maskxfm;
    GLuint u_colorconv;
	GLuint u_realY;
	GLuint u_realX;
    // Uniform - Sampler for RGB program
    GLuint u_textureSampler;
    GLuint u_back_textureSampler;//for rendering background image - etc)blur, mosaic...
    // Uniforms - Samplers for YUV/NV12 program
    GLuint u_textureSamplerY;   // Y in YUV and NV12
    GLuint u_textureSamplerU;   // U in YUV
    GLuint u_textureSamplerV;   // V in YUV
    GLuint u_textureSamplerUV;  // Interleaved UV in NV12
    // Attributes
    GLuint a_position;
    GLuint a_texCoord;
    GLuint a_maskCoord;
    //ScreenMasking
    GLuint u_textureSampler_for_mask;
    GLuint u_inverse_Mask;

    GLuint u_hue;
    NXT_ShaderProgram_Masked_():u_mvp_matrix(0), u_homo_matrix(0), u_mask_homo_matrix(0), u_tex_matrix(0), u_mask_matrix(0), u_maskSampler(0), 
        u_alpha(0), u_color(0), u_realX(0), u_realY(0),  
        u_textureSampler(0), u_textureSamplerY(0), u_textureSamplerU(0), u_textureSamplerV(0), u_textureSamplerUV(0), 
        a_position(0), a_texCoord(0), a_maskCoord(0), u_textureSampler_for_mask(0), u_inverse_Mask(0), u_hue(0){

        }
} NXT_ShaderProgram_Masked;
#endif //NEXTHEME_SUPPORT_OPENGL

typedef enum NXT_ChannelFilter_ {
    NXT_ChannelFilter_ALL   = 0,
    NXT_ChannelFilter_Y     = 1,
    NXT_ChannelFilter_U     = 2,
    NXT_ChannelFilter_V     = 3
} NXT_ChannelFilter;

#define NXT_ChannelFilter_MAX 4
    
#define INVALID_UNIFORM 0xFFFFFFFFU
#define IS_VALID_UNIFORM(X) ((X)!=INVALID_UNIFORM)

#define INVALID_ATTRIBUTE 0xFFFFFFFFU
#define IS_VALID_ATTRIBUTE(X) ((X)!=INVALID_ATTRIBUTE)


#ifdef ANDROID
#define NXT_SURFACE_TEXTURE_MAX 16

typedef struct NXT_SurfaceTextureInfo_ {
    jobject objSurfaceTexture;
    jobject objSurface;
    jobject objWSTL;
    GLuint surfaceTextureName;
    ANativeWindow* pNativeWindow;
    unsigned int bUsed;
    NXT_Matrix4f matrix;
} NXT_SurfaceTextureInfo;
#endif
    


    /*
     void NXT_ThemeRenderer_SetClipEffect(NXT_HThemeRenderer renderer,
     const char *effectId,  // ID of clip effect, or NULL to clear current effect
     const char *pEffectOptions,
     int clipIndex,         // Index of this clip, from 0
     int totalClipCount,    // Total number of clips in the project
     int clipStartTime,     // Start time for this clip
     int clipEndTime,       // End time for this clip
     int effectStartTime,   // Start time for this effect (must be >= clipStartTime)
     int effectEndTime )    // End time for this effect (must be <= clipEndTime)
     {
     
     }
     
     void NXT_ThemeRenderer_SetTransitionEffect(NXT_HThemeRenderer renderer,
     const char *effectId,  // ID of transition effect, or NULL to clear current effect
     const char *pEffectOptions,
     int clipIndex,         // Index of the transition's clip (the clip before the transition), from 0
     int totalClipCount,    // Total number of clips in the project
     int effectStartTime,   // Start time for this effect
     int effectEndTime )    // End time for this effect
     {
     
     }
     
     
     */

#include <map>
#include <vector>
#include <list>
#include <string>

struct ValueMapObj{

    ValueMapObj(){

    }

    virtual ~ValueMapObj(){

    }
};

typedef std::map<std::string,ValueMapObj*> KeyValueMap_t;

struct NXT_AppliedEffect{
    char effectId[256];
	NXT_EffectType effectType;
    int clip_index;
    int clip_count;
	int bEffectChanged;
    int clipStartTime;
    int clipEndTime;
    int effectStartTime;
    int effectEndTime;
    int effectUserStartTime;
    int effectUserEndTime;
    int textSerial;
    NXT_HEffect nexEDLItem;
    int render_item_id;
    int render_item_encoded;
    KeyValueMap_t keyvaluemap_;

    // Effect options
    char *effectOptionsEncoded;
    int effectOptionCount;
    int transitionOptionCount;
    int clipEffectOptionCount;
    char *effectOptionsBuffer;
    char **effectOptionKeys;
    char **effectOptionValues;   

    NXT_AppliedEffect(){

        effectId[0] = 0;
        effectType = NXT_EffectType_NONE;
        clip_index = 0;
        clip_count = 0;
        bEffectChanged = 0;
        clipStartTime = 0;
        clipEndTime = 0;
        effectStartTime = 0;
        effectEndTime = 0;
        effectUserStartTime = 0;
        effectUserEndTime = 0;
        textSerial = 0;
        nexEDLItem = NULL;
        render_item_id = 0;
        render_item_encoded = 0;
        keyvaluemap_.clear();

        // Effect options
        effectOptionsEncoded = NULL;
        effectOptionCount = 0;
        transitionOptionCount = 0;
        clipEffectOptionCount = 0;
        effectOptionsBuffer = NULL;
        effectOptionKeys = NULL;
        effectOptionValues = NULL;
    }

    ~NXT_AppliedEffect();
};

enum
{
	MAX_DOWNSCALE_STEP = 5,
	MAX_RENDER_TARGET_RESOURCE = 15,
    MAX_LUT_TEXGROUP = 100,
};

typedef struct NXT_Downscale_{
	int width_;
	int height_;
    NXT_Downscale_():width_(0), height_(0){

    }
}NXT_Downscale;

struct NXT_RenderTarget
{   
    GLuint fbo_;
    GLuint depth_buffer_;
    GLuint target_texture_;
    int width_;
    int height_;
    int alpha_;
    int depth_;
    int stencil_;
    int inited_;

    NXT_RenderTarget():fbo_(0), depth_buffer_(0), target_texture_(0), width_(0), height_(0), alpha_(0), depth_(0), stencil_(0), inited_(0){

    }

    ~NXT_RenderTarget(){

    }

    bool init(int w, int h, int alpha_request, int depth_request, int stencil_request);

    bool init_depthonly(int w, int h, int stencil_request);

    void destroy(unsigned int isDetachedConext);
};

struct RenderTargetResource{

    RenderTargetResource():prender_target_(NULL), unused_counter_(0){

    }

    ~RenderTargetResource(){

    }

    NXT_RenderTarget* prender_target_;
    int unused_counter_;
};

class RenderTargetManager{

public:
    RenderTargetManager(){

    }
    ~RenderTargetManager(){};
    void destroy();
    NXT_RenderTarget* getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request);
    NXT_RenderTarget* getDepthonly(int width, int height, int stencil_request);
    void releaseRenderTarget(NXT_RenderTarget* prendertarget);
    void update();
private:
    typedef std::list<RenderTargetResource> RenderTargetResourceList_t;
    RenderTargetResourceList_t resource_;
};

struct LUTTexture{

    int lut_resource_id_;
    int tex_id_for_lut_;

    LUTTexture():lut_resource_id_(0), tex_id_for_lut_(0){

    }
};

typedef struct NXT_RenderFastPreviewBuffer_{
    GLuint fbo_;
    GLuint depth_buffer_;
    GLuint target_texture_;
    int width_;
    int height_;
    int inited_;
    int time_;

	struct NXT_RenderFastPreviewBuffer_* next;
	struct NXT_RenderFastPreviewBuffer_* prev;
}NXT_RenderFastPreviewBuffer;

struct EffectMaster{

    struct EffectInfo{
        typedef std::vector<unsigned int> childlist_t;
        typedef childlist_t::iterator childiter;

        unsigned int uid_;
        unsigned int pending_;
        NXT_AppliedEffect effect_info_;
        int start_track_id_;
        childlist_t childlist_;
    };

    typedef std::vector<unsigned int> effectqueue_t;
    typedef std::map<unsigned int, EffectInfo*> effectmap_t;

    effectmap_t     effectmap_;
    effectqueue_t   effectqueue_;
    effectqueue_t   transitionqueue_;

    unsigned int edl_serial_;

    void initEDLSerial();
    unsigned int issueEDLSerial();
    void releaseEDLSerial(unsigned int serial);

    unsigned int text_serial_;
    void initTextSerial();
    unsigned int issueTextSerial();
    void releaseTextSerial(unsigned int serial);

    bool playEDL(NXT_HThemeRenderer renderer, int currentTime);
    void endEDL(unsigned int uid);

    bool setEffectOption(unsigned int uid, const char* pEffectOptions);

    void removePending();

    unsigned int startTitleEDL(const char* effectId,
                               const char* pEffectOptions,
                               int startTrackId,
                               int effectIndex,
                               int totalEffectCount,
                               int effectStartTime,
                               int effectEndTime);
    unsigned int startTransitionEDL(
                            const char* effectId,
                            const char* pEffectOptions,
                            int effectIndex,
                            int totalEffectCount,
                            int effectStartTime,
                            int effectEndTime);

    void clear();

    EffectMaster(){

        initEDLSerial();
        initTextSerial();
    }

    ~EffectMaster(){

        clear();
    }
};

struct NXT_ThemeRenderer_Context{

private:
#ifdef ANDROID
    EGLContext eglContext;
    EGLSurface dummy_surface_;
#endif
    void* ptex_manager_;
    void* prenderitem_manager_;
    void* pkedlitem_manager_;
    bool shader_init_flag_;
    //Jeff RTT------------------------------------------------------------------------------------------
	RenderTargetManager render_target_manager_;
public:
    unsigned int contextIsAcquired;
    pthread_t contextOwner;
    pthread_mutex_t contextLock;
#ifdef ANDROID
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
#endif
    unsigned int bRequestMultisample;
	unsigned int requestDepthBufferSize;
    GLuint emptyTexture;
	GLuint whiteTexture;

    NXT_ThemeRenderer_Context(int max_texture_cache_size, unsigned int bIsInternalContext, unsigned int bIsMI5C);

    ~NXT_ThemeRenderer_Context();

#ifdef ANDROID
    EGLContext getContext(){

        return eglContext;
    }

    EGLSurface getDummySurface() {

        return dummy_surface_;
    }

    bool getShaderInitFlag() const{

        return shader_init_flag_;
    }

    void setShaderInitFlag(bool flag){

        shader_init_flag_ = flag;
    }

    void setContext(EGLContext context){

        eglContext = context;
    }
#endif
    
    void initDefaultTextures();

	void setTexManager(void* pmanager){

        ptex_manager_ = pmanager;
    }

    void* getTexManager(){

        return ptex_manager_;
    }

    void setRenderItemManager(void* pmanager){

        prenderitem_manager_ = pmanager;
    }

    void* getRenderItemManager(){

        return prenderitem_manager_;
    }

    void setKedlItemManager(void* pmanager){

        pkedlitem_manager_ = pmanager;
    }

    void* getKedlItemManager(){

        return pkedlitem_manager_;
    }

    void* getRendertargetManager(){

        return &render_target_manager_;
    }


    NXT_ShaderProgram_Textured  texturedShaderExternal;
    NXT_ShaderProgram_Textured  texturedShaderExternal_Combined;
    NXT_ShaderProgram_Textured  texturedShaderExternal360;
    NXT_ShaderProgram_Textured  texturedShaderExternal_vignette;
    NXT_ShaderProgram_Textured  texturedShaderExternal_Combined_vignette;
	NXT_ShaderProgram_Textured  texturedShaderPassthroughExternal;
    NXT_ShaderProgram_Textured  texturedShaderPassthroughExternalHDR10;
    NXT_ShaderProgram_Masked    maskedShaderExternal;
    NXT_ShaderProgram_Masked    maskedShaderExternal_Combined;
    NXT_ShaderProgram_Masked    maskedShaderExternal_vignette;

    NXT_ShaderProgram_Solid     solidShaderRGB;
    NXT_ShaderProgram_Textured  texturedShaderRGB;
    NXT_ShaderProgram_Textured  texturedShaderRGB_Homo;
    NXT_ShaderProgram_Textured  texturedShaderRGB_Combined_Homo;
    NXT_ShaderProgram_Textured  texturedShaderRGB_vignette;
    NXT_ShaderProgram_Textured  texturedShaderRGB_vignette_Homo;
    NXT_ShaderProgram_Textured  texturedShaderRGB_Combined_vignette_Homo;
    NXT_ShaderProgram_Textured  texturedShaderRGB_For_Videos;
    NXT_ShaderProgram_Textured  texturedShaderRGB_Combined_For_Videos;
    NXT_ShaderProgram_Textured  texturedShaderRGB_For_Videos360;
    NXT_ShaderProgram_Textured  texturedShaderRGB_For_Videos_vignette;
    NXT_ShaderProgram_Textured  texturedShaderRGB_Combined_For_Videos_vignette;
	NXT_ShaderProgram_Textured  texturedShaderRGB_for_layers;
	NXT_ShaderProgram_Textured  texturedShaderPassthroughRGB;
    NXT_ShaderProgram_Textured  texturedShaderPassthroughRGBWithColorconv;
	NXT_ShaderProgram_Textured  texturedShaderLUT;
	NXT_ShaderProgram_Textured  texturedShaderFilterTest;
    NXT_ShaderProgram_Textured  texturedShaderUserVignette;
    NXT_ShaderProgram_Textured  texturedShaderSharpness;
    NXT_ShaderProgram_Masked    maskedShaderRGB;
    NXT_ShaderProgram_Masked    maskedShaderRGB_Combined;
    NXT_ShaderProgram_Masked    maskedShaderRGB_Homo;
    NXT_ShaderProgram_Masked    maskedShaderRGB_Combined_Homo;

    NXT_ShaderProgram_Textured  texturedShaderStencil;
    NXT_ShaderProgram_Textured  texturedShaderStencil_Homo;
    NXT_ShaderProgram_Masked    maskedShaderStencil;
    NXT_ShaderProgram_Masked    maskedShaderStencil_Homo;
    
    NXT_ShaderProgram_Textured  texturedShaderYUV;
    NXT_ShaderProgram_Textured  texturedShaderYUV_vignette;
	NXT_ShaderProgram_Textured  texturedShaderPassthroughYUV;
    NXT_ShaderProgram_Masked    maskedShaderYUV;
    NXT_ShaderProgram_Masked    maskedShaderYUV_vignette;
    
    NXT_ShaderProgram_Textured  texturedShaderNV12;
    NXT_ShaderProgram_Textured  texturedShaderNV12_Combined;
    NXT_ShaderProgram_Textured  texturedShaderNV12_vignette;
    NXT_ShaderProgram_Textured  texturedShaderNV12_Combined_vignette;
	NXT_ShaderProgram_Textured  texturedShaderPassthroughNV12;
    NXT_ShaderProgram_Masked    maskedShaderNV12;
    NXT_ShaderProgram_Masked    maskedShaderNV12_Combined;
    NXT_ShaderProgram_Masked    maskedShaderNV12_vignette;
    
    NXT_ShaderProgram_Solid     solidShaderNV12toYUVA8888[NXT_ChannelFilter_MAX];
    NXT_ShaderProgram_Textured  texturedShaderNV12toYUVA8888[NXT_ChannelFilter_MAX];
    NXT_ShaderProgram_Masked    maskedShaderNV12toYUVA8888[NXT_ChannelFilter_MAX];    
    
    NXT_ShaderProgram_Textured  texturedShaderRGBAtoYUVA8888[NXT_ChannelFilter_MAX];
	NXT_ShaderProgram_Textured  texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_MAX];
    NXT_ShaderProgram_Masked    maskedShaderRGBAtoYUVA8888[NXT_ChannelFilter_MAX];
    
    NXT_ShaderProgram_Textured  texturedShaderEXTtoYUVA8888[NXT_ChannelFilter_MAX];
    NXT_ShaderProgram_Masked    maskedShaderEXTtoYUVA8888[NXT_ChannelFilter_MAX];

    //Screen Masking
    NXT_ShaderProgram_Solid screenMaskingType_solidShaderRGB;

    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_Homo;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_vignette_Homo;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_For_Videos;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_For_Videos360;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_For_Videos_vignette;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_vignette;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_Combined_For_Videos;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_Combined_Homo;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_Combined_vignette_Homo;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette;
    
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderStencil;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderStencil_Homo;

    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderExternal;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderExternal_Combined;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderExternal360;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderExternal_vignette;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderExternal_Combined_vignette;

    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderYUV;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderYUV_vignette;

    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderNV12;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderNV12_vignette;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderNV12_Combined_vignette;
    NXT_ShaderProgram_Textured screenMaskingType_texturedShaderNV12_Combined;

    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderExternal;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderExternal_Combined;

    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderStencil;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderStencil_Homo;

    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderRGB;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderRGB_Homo;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderRGB_Combined;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderRGB_Combined_Homo;

    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderYUV;
    
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderNV12;
    NXT_ShaderProgram_Masked screenMaskingType_maskedShaderNV12_Combined;

    NXT_ShaderProgram_Textured  texturedShaderY2UVA;
    NXT_ShaderProgram_Textured  texturedShaderPassthroughY2UVA;

    struct TexShader{

        GLuint program_;
        GLuint mvp_matrix_;
        GLuint tex_sampler_;
        GLuint mask_sampler_;
        GLuint reverse_;
        GLuint alpha_;
        GLuint a_pos_;
        GLuint a_tex_;

        TexShader():program_(0), mvp_matrix_(0), tex_sampler_(0), mask_sampler_(0), alpha_(0), a_pos_(0), a_tex_(0), reverse_(0){

        }

        ~TexShader();

        void init();
    };

    TexShader tex_;
};

struct NXT_ImageInfo_Cache{

    NXT_ImageInfo info_;
    unsigned int live_start_;
    unsigned int live_end_;
};

typedef std::map<std::string, NXT_ImageInfo_Cache> CachedBitmapMapper_t;

struct NXT_ThemeRenderer_ {

    NXT_ThemeRenderer_Context* pcontext_;
    std::map<int, int> uidmapper_;
public:
    NXT_ThemeRenderer_Context* getContextManager(){

        return pcontext_;
    }

    void setMapper(std::map<int,int>* pmapper) {

        if(pmapper == nullptr)
            uidmapper_.clear();
        else
            uidmapper_ = *pmapper;
    }

    int getRealUID(int id) {

        const auto& it = uidmapper_.find(id);
        if(it == uidmapper_.end())
            return id;
        return it->second;
    }

    void setContextManager(NXT_ThemeRenderer_Context* pctx){

        pcontext_ = pctx;
    }
#ifdef ANDROID
    EGLContext getContext(){

        if(pcontext_ != NULL)
            return pcontext_->getContext();
        return EGL_NO_CONTEXT;
    }

    bool setContext(EGLContext ctx){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return false;
        }
        pcontext_->setContext(ctx);
        return true;
    }
#endif
    void* getTexManager(){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return NULL;
        }

        return pcontext_->getTexManager();
    }

    bool setTexManager(void* pmanager){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return false;
        }

        pcontext_->setTexManager(pmanager);
        return true;
    }

    void* getRenderItemManager(){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return NULL;
        }

        return pcontext_->getRenderItemManager();
    }

    bool setRenderItemManager(void* pmanager){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return false;
        }

        pcontext_->setRenderItemManager(pmanager);
        return true;
    }

    void* getKedlItemManager(){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return NULL;
        }

        return pcontext_->getKedlItemManager();
    }

    bool setKedlItemManager(void* pmanager){

        if(pcontext_ == NULL){

            LOGE("%s pcontext_ is null", __func__);
            return false;
        }

        pcontext_->setKedlItemManager(pmanager);
        return true;
    }

    void drawTexBox(int tex, bool mask_enabled, float* pmatrix, float left, float top, float right, float bottom, float alpha, int no_filter = 0, int reverse = 0);

    public:
        unsigned int getTrackIssueID();
        void arrangeSurfaceResource();
        void keepSurfaceResource(unsigned int tex_id);
    private:
        void resetTrackIssueID();
        unsigned int track_issue_id_;
    CachedBitmapMapper_t cachedbitmap_mapper_;
public:
	pthread_mutex_t cachedbitmaplock_;
    void lockCachedBitmap();
    void unlockCachedBitmap();
    int getCachedBitmap(const char* key, NXT_ImageInfo* pinfo, unsigned int live_start = 0, unsigned int live_end = 0);
    int addCachedBitmap(const char* key, NXT_ImageInfo* pinfo, unsigned int live_start, unsigned int live_end);
    void clearCachedBitmap(unsigned int live_start, unsigned int live_end);
private:    
    typedef std::vector<unsigned int> surfaceresource_t;
    typedef std::vector<NXT_RenderTarget*> rendertargetstack_t;
    surfaceresource_t surfaceresource_;
    EffectMaster effectmaster_;

    std::vector<NXT_TextureInfo> videoTexture;
    struct VideoResource{

        int slot_id_;
        int ref_count_;
        int state_;

        VideoResource():slot_id_(-1), state_(0), ref_count_(0){

        }

        VideoResource(int slot_id, int state):slot_id_(slot_id), state_(state), ref_count_(0){

        }

        int addRef(){

            return ++ref_count_;
        }

        int decRef(){

            return --ref_count_;
        }

        int getRef(){

            return ref_count_;
        }
    };

    typedef std::vector<int> videoresource_t;
    typedef std::map<unsigned int, VideoResource> videoresourcemap_t;
    videoresource_t left_videotexture_;

    videoresourcemap_t inuse_videotexture_;
    videoresourcemap_t deactive_videotexture_;

    videoresourcemap_t inuse_decotexture_;
    videoresourcemap_t deactive_decotexture_;

    NXT_TextureInfo* video_[NXT_TextureID_COUNT];

    int current_track_id_;

    typedef std::map<int, int> LutResourceMapper_t;
    LutResourceMapper_t lut_resource_;
    int force_rtt_;
public:

    void setForceRTT(int flag);
    int getForceRTT();
    int getLut(int id);
    int releaseLut(int id);
    int setLutTexture(int id, int tex_id);
    void setCustomLutA(unsigned int id, int tex_id);
    void setCustomLutB(unsigned int id, int tex_id);
    void setCustomLutPower(unsigned int id, int power);

    float moveX_value;
    float moveY_value;

    int export_mode_;
    int only_rgb_possible_surface_;
    int mask_init_flag_;
    GLuint tex_id_for_vignette_;
    
    int (*getLutTextureWithID)(int, int);
    int (*getEffectImagePath)(const char*, char*);
    int (*getVignetteTexID)(int);
    
    NXT_RendererType rendererType;
    unsigned int bIsInternalContext;
    
    const char *pFastPreviewOptions;
    
    pthread_mutex_t themesetLock;
	NXT_RenderTarget* pmain_render_target_;
	NXT_RenderTarget* pmain_mask_target_;
    NXT_RenderTarget* pmain_blend_target_;
	
    typedef std::vector<NXT_Downscale> DownScaleList_t;
	DownScaleList_t down_scale_info_;

    int setRenderToDefault();

    int setRenderToMask();

    void setGLStateForRGBAOutput();

    void setGLRenderStart();

    int setTextureNameForLUT(unsigned int uid, float x, float y);

    int createVideoTexture(unsigned int uid);
    int releaseVideoTexture(unsigned int uid);
    void manageVideoTexture();

    int createDecoTexture(unsigned int uid);
    int releaseDecoTexture(unsigned int uid);
    void manageDecoTexture();

    int getTexName(unsigned int uid);
    int getTexName(unsigned int uid, const float** ppmatrix);
    int getRawVideoSrcForUID(unsigned int uid);
    int getDecoVideoSrcForUID(unsigned int uid);
    void linkTrack(unsigned int uid, unsigned int base_uid);
    void setLUT(unsigned int uid, int lut);
    void initVideoTextureResource();
    void releaseAllVideoTexture();

    bool setEffectOption(unsigned int uid, const char* effectOptions);

    unsigned int startTitleEDL(
                       const char* effectId,
                       const char* pEffectOptions,
                       int startTrackId,
                       int effectIndex,
                       int totalEffectCount,
                       int effectStartTime,
                       int effectEndTime);

    unsigned int startTransitionEDL(
                            const char* effectId,
                            const char* pEffectOptions,
                            int effectIndex,
                            int totalEffectCount,
                            int effectStartTime,
                            int effectEndTime);

    void endEDL(unsigned int effect_uid);

    bool playEDL(int currentTime);

    void setVideoSrc(int target, NXT_RenderTarget* psrc);

    void setVideoSrc(int target, NXT_TextureInfo* ptex);

    void resetVideoSrc();

    void resetVideoSrc(int target);

    void applyLUT(NXT_TextureInfo& tex);

    NXT_TextureInfo& getVideoSrc(int target);

    NXT_TextureInfo& getRawVideoSrc(int target);

    void findTexIDAndClear(int tex_id);

    void renderEDL(NXT_AppliedEffect* peffect, unsigned int uid, int start_track_id);

    int getCurrentTrackID();

    int addCurrentTrackID();

    void resetCurrentTrackID();

    void resetCurrentTrackID(int track_id);

    NXT_RenderTarget* getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request);
    NXT_RenderTarget* getDepthonly(int width, int height, int stencil_request);

    void releaseRenderTarget(NXT_RenderTarget* ptarget);

    void clearEffect();

private:

    struct VideoTrackInfo{

        std::vector<int> item_;

        VideoTrackInfo(){

            item_.resize(NXT_TextureID_COUNT);
            reset();
        }

        bool checkVideoTrack(NXT_TextureID track_id){

            if(item_[track_id] < 0)
                return false;
            return true;
        }

        void setVideoTrack(NXT_TextureID track_id, int track_no){

            LOGI("TM30 getVideoSrc setVideoTrack:%d %d", track_id, track_no);
            item_[track_id] = track_no;
        }

        int getVideoTrack(NXT_TextureID track_id){

            return item_[track_id];
        }

        void reset(){

            for(int i = 0; i < NXT_TextureID_COUNT; ++i){

                item_[i] = -1;
            }
        }

        void recover(NXT_ThemeRenderer_* renderer){

            for(int i = 0; i < NXT_TextureID_COUNT; ++i){

                if(item_[i] > 0){

                    int src = renderer->getDecoVideoSrcForUID(item_[i]);
                    renderer->setVideoSrc((NXT_TextureID)i, &renderer->getRawVideoSrc(src));
                }
            }
        }
    };

    std::vector<VideoTrackInfo> videotrackinfo_stack_;

    VideoTrackInfo videotrackinfo_;
public:

    bool checkVideoTrack(NXT_TextureID track_id){

        return videotrackinfo_.checkVideoTrack(track_id);
    }

    void setVideoTrack(NXT_TextureID track_id, int track_no){

        videotrackinfo_.setVideoTrack(track_id, track_no);
    }

    int getVideoTrack(NXT_TextureID track_id){

        return videotrackinfo_.getVideoTrack(track_id);
    }

    void pushVideoTrack(){

        videotrackinfo_stack_.push_back(videotrackinfo_);
        videotrackinfo_.reset();
        LOGI("TM30 getVideoSrc pushVideoTrack:%d", videotrackinfo_stack_.size());
    }

    void popVideoTrack(){

        videotrackinfo_ = videotrackinfo_stack_.back();
        videotrackinfo_stack_.pop_back();
        videotrackinfo_.recover(this);
        LOGI("TM30 getVideoSrc popVideoTrack:%d", videotrackinfo_stack_.size());
    }

    void resetVideoTrack(){

        videotrackinfo_.reset();
    }

    NXT_ThemeRenderer_();

	//--------------------------------------------------------------------------------------------------
    
    //Case FastPreview ---------------------------------------------------------------------------------
    int fastpreview_on;
    int fastpreview_total;
    int fastpreview_index;
    int fastpreview_width;
    int fastpreview_height;

    int fastpreview_maxCount;
    NXT_RenderFastPreviewBuffer* pFastPreviewBuffer;
	NXT_RenderFastPreviewBuffer* pFastPreviewBufferFirst;
	NXT_RenderFastPreviewBuffer* pFastPreviewBufferLast;
	NXT_RenderFastPreviewBuffer* pFastPreviewBufferCur;
    NXT_RendererOutputType fastpreview_renderOutputType;
    //Case FastPreview ---------------------------------------------------------------------------------
	
    unsigned int bInitialized;
    unsigned int bCalledGetPixels;
    unsigned int bDidRender;
	unsigned int bAvailableMultisample;
    unsigned int bEnableCustomRendering;

    NXT_LoadFileCallback    loadFileCallback;    
    NXT_LoadImageCallback loadImageCallback;
    NXT_CustomRenderCallback customRenderCallback;
    NXT_FreeImageCallback freeImageCallback;
    void *imageCallbackPvtData;
    
    NXT_RendererOutputType outputType;
    NXT_ChannelFilter channelFilter;
    
#ifdef NEXTHEME_SUPPORT_OPENGL
    NXT_ShaderProgram_Solid     *pSolidShaderActive;
    NXT_ShaderProgram_Textured  *pTexturedShaderActive;
    NXT_ShaderProgram_Masked    *pMaskedShaderActive;
#endif //NEXTHEME_SUPPORT_OPENGL

    NXT_Vector4f color;
    NXT_Matrix4f basetransform;
    NXT_Matrix4f transform;
    NXT_Matrix4f proj;
    NXT_Matrix4f texture_transform;
    NXT_Matrix4f mask_transform;
    NXT_Matrix4f mask_sample_transform;
    float alpha;
    float animframe;
	float y_coord;
    NXT_Node_Texture *texture;
    NXT_Node_Texture *mask;
    NXT_RenderDest renderDest;
    NXT_RenderTest renderTest;
    GLboolean colorMask[4];
	unsigned int bAntiAlias;
	unsigned int bDepthTest;
	unsigned int bDepthMask;
    NXT_CompMode  compMode;

    unsigned int bExportMode;

#ifdef ANDROID
    ANativeWindow *nativeWindow;
    EGLSurface nativeWindowSurface;

    EGLSurface nativeExportSurface;
    
    int nativeWindowWidth;
    int nativeWindowHeight;
    int exportSurfaceWidth;
    int exportSurfaceHeight;
    
    JavaVM* java_vm;
    jclass	jclass_wrapperforSurfaceTextureListener;

    // The following 3 variables are used in NexThemeRenderer_SurfaceTexture. by Eric.
	jclass      classSurfaceTexture;
   	jmethodID   methodSurfaceTexture_updateTexImage;
	jmethodID   methodSurfaceTexture_getTransformMatrix;
    jmethodID   methodWSTL_waitFrameAvailable;

    
    NXT_SurfaceTextureInfo surfaceTextures[NXT_SURFACE_TEXTURE_MAX];
    NXT_SurfaceTextureInfo outputSurfaceTexture;

#endif
    // Placeholder for platform dependent context
    void* platCtx;
    
    NXT_RandSeed seed;
    
    int cur_effect_overlap;
    int last_upload_tex_id;
    
	NXT_HThemeSet themeSet;
	
    int bNoFx;
    int bSwapV;
    
    NXT_AppliedEffect watermarkEffect;
    NXT_AppliedEffect transitionEffect;
    NXT_AppliedEffect clipEffect;
    NXT_AppliedEffect* pActiveEffect;

    int overlayEffectMark[100];
    NXT_AppliedEffect overlayEffect[100];

    unsigned int bActiveEffectChanged;
	NXT_EffectType effectType;
    int clip_index;
    int clip_count;
    
    unsigned int original_cts;
    unsigned int cts;
    unsigned int cur_time;
    unsigned int max_set_time;
    unsigned int max_time;
    unsigned int bSkipChildren; // Set to 1 in beginRenderFunc to skip children of current node
    unsigned int inTime;
    unsigned int outTime;
    unsigned int cycleTime;
    NXT_RepeatType repeatType;
    NXT_PartType part;
    
    float settime;
    float time;
    
    unsigned int view_width;
    unsigned int view_height;
    NXT_CullFace cullFace;
    
    char *videoPlaceholder1;
    char *videoPlaceholder2;
    char *titleText;
    int masterTextSerial;         // Serial number that is incremented every time the titleText OR EFFECT OPTIONS changes (used for caching rendered text)
    
    int effectStartTime;
    int effectEndTime;
    
    float adj_brightness;
    float adj_contrast;
    float adj_saturation;
    NXT_Vector4f adj_tintColor;

//For Support Xiaomi CD//////////////////////////
    int final_brightness_;
    int final_contrast_;
    int final_saturation_;
/////////////////////////////////////////////////
    
    float final_vignetteRange_;
    float final_vignette_;
    float final_sharpness_;
    
    void setBrightness(int value);
    int getBrightness();
    void setContrast(int value);
    int getContrast();
    void setSaturation(int value);
    int getSaturation();
    
    void setVignette(int value);
    int getVignette();
    void setVignetteRange(int value);
    int getVignetteRange();
    void setSharpness(int value);
    int getSharpness();
    
    unsigned char *pReadPixelData;
    unsigned int readPixelDataSize;

	
    NXT_Vector4f        ambientColor;
    NXT_Vector4f        diffuseColor;
    NXT_Vector4f        specularColor;
    NXT_Vector4f        lightDirection;
    float				specularExponent;
    NXT_ShadingMode     shadingMode;
    NXT_LightingNormals lightingNormals;
    
    int bFlags;
    
	// --- Watermark texture -------------------------------------------------------
    unsigned int bLoadedWatermark;
    unsigned int bUseWatermark;
    NXT_Node_Texture watermarkTexture;
    
    int disable_watermark;

	//Jeff
	unsigned int lut_texture_;
	float lut_strength_;

    GLuint defaultFrameBufferNum;

    int currentTime;
    int actualEffectStartCTS;
    int actualEffectEndCTS;    

    void setRenderTarget(NXT_RenderTarget* prender_target);
    void setRenderTarget(NXT_RenderTarget* prender_target, NXT_RenderTarget* pdepth_target);
    void unbindDepth(NXT_RenderTarget* prender_target);
    void setHQScale(int src_w, int src_h, int dest_w, int dest_h);
    void scaleDownRender();
    void copySurface(NXT_RenderTarget* pdest, NXT_RenderTarget* psrc);
    int basefilter_renderitem_id_;
    char basefilter_renderitem_uid_[256];
    NXT_RenderItem_KeyValues basefilter_keyvalues_;

private:
    int deviceLightLevel;
    float deviceGamma;
public:
    void setDeviceLightLevel(int lightLevel);
    void setDeviceGamma(float gamma);
    int getDeviceLightLevel();
    float getDeviceGamma();
};

char *NXT_ThemeRenderer_GetEffectOption( NXT_HThemeRenderer renderer, const char *pOptionKey );

int NexThemeRenderer_PrepStateForRender(NXT_HThemeRenderer renderer, 
                                         NXT_Node_Texture *texture,
                                         NXT_Node_Texture *mask,
                                         NXT_TextureInfo **useTexture,
                                         NXT_TextureInfo **useMask,
                                         unsigned int fullmode,
                                         const float* homography = NULL,
                                         const float* homography_mask = NULL
                                         );

void NexThemeRenderer_PrepStateForRender2(NXT_HThemeRenderer renderer, 
                                         NXT_Node_Texture *texture,
                                         NXT_Node_Texture *mask,
                                         NXT_TextureInfo **useTexture,
                                         NXT_TextureInfo **useMask,
                                         NXT_Matrix4f* pmatrix,
                                         NXT_Matrix4f* pmatrix_for_colorconv,
                                         int bgr_flag);

void NXT_ThemeRenderer_SetRenderTarget(NXT_HThemeRenderer renderer,NXT_RenderTarget* prender_target);
void NXT_ThemeRenderer_SetRenderTargetCombined(NXT_HThemeRenderer renderer, NXT_RenderTarget* prenertarget, NXT_RenderTarget* pdepthonly);
void NXT_ThemeRenderer_UnbindDepth(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target);
NXT_RenderTarget* NXT_ThemeRenderer_GetRenderTarget(NXT_HThemeRenderer renderer, int width, int height, int alpha_request, int depth_request, int stencil_request);
NXT_RenderTarget* NXT_ThemeRenderer_GetDepthonly(NXT_HThemeRenderer renderer, int width, int height, int stencil_request);
void NXT_ThemeRenderer_ReleaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target);

float NexThemeRenderer_AdjTexCoordX( NXT_TextureInfo *texture, float x );
float NexThemeRenderer_AdjTexCoordY( NXT_TextureInfo *texture, float y );

void NXT_Theme_SetRealXYForRenderItem(void* pmanager, int target, NXT_TextureInfo* texinfo);

#define LOAD_SHADER_ON_TIME
#ifdef LOAD_SHADER_ON_TIME
NXT_Error NexThemeRenderer_PrepareShaders(NXT_ThemeRenderer_Context* pctx);
NXT_Error NexThemeRenderer_UnloadShaders2(NXT_ThemeRenderer_Context* pctx);
NXT_Error NexThemeRenderer_UseShaderProgram(void* programBase, int programSize);
#else
NXT_Error NexThemeRenderer_LoadShaders(NXT_ThemeRenderer_Context* pctx);
NXT_Error NexThemeRenderer_UnloadShaders(NXT_ThemeRenderer_Context* pctx);
#endif

void NXT_ThemeRenderer_FreeAppliedEffect(NXT_AppliedEffect* appliedEffect);
void NXT_ThemeRenderer_SetAppliedEffect(NXT_HThemeRenderer renderer,
                             NXT_AppliedEffect* appliedEffect,
                             NXT_EffectType effectType,
                             const char *effectId,  // ID of clip effect, or NULL to clear current effect
                             const char *effectOptions,
                             int clipIndex,         // Index of this clip, from 0
                             int totalClipCount,    // Total number of clips in the project
                             int clipStartTime,     // Start time for this clip
                             int clipEndTime,       // End time for this clip
                             int effectStartTime,   // Start time for this effect (must be >= clipStartTime)
                             int effectEndTime );
void NXT_ThemeRenderer_TimeClipEffect(NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int actualEffectStartCTS, int actualEffectEndCTS);
void NXT_ThemeRenderer_RenderEffect(NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time);
void NXT_ThemeRenderer_RISetParameterKeyValue(NXT_HThemeRenderer renderer, NXT_RenderItem_KeyValues* pkeyvalues);
    
#define LOGPTR(ptr) ((unsigned int)(size_t)(ptr))

unsigned int NXT_Profile_TickCount( void );
    
#ifdef ANDROID
    #include <jni.h>
    #include <android/log.h>
    #ifdef NEX_THEME_RENDERER_PROFILE_READPIXELS
        #define NXT_BEGIN_PROFILE(xxdesc) { int nxt_dbgprof_st_tm = NXT_Profile_TickCount(); static const char* nxt_dbgprof_desc = (xxdesc);
        #define NXT_PROFILE_CHECKPOINT(xxdesc) { int nxt_dbgprof_elapsed = NXT_Profile_TickCount()-nxt_dbgprof_st_tm; LOGI("[%s %d] PROFILE CHECKPOINT* / %d ms elapsed at: %s / %s ", __func__,__LINE__, nxt_dbgprof_elapsed, (xxdesc), nxt_dbgprof_desc); }
        #define NXT_END_PROFILE() ; int nxt_dbgprof_ed_tm = NXT_Profile_TickCount(); LOGI("[%s %d] PROFILE / %d ms / %s ", __func__,__LINE__, nxt_dbgprof_ed_tm - nxt_dbgprof_st_tm, nxt_dbgprof_desc); }
    #else
        #define  NXT_BEGIN_PROFILE(xxdesc)
        #define  NXT_END_PROFILE(xxdesc)
        #define  NXT_PROFILE_CHECKPOINT(xxdesc)
    #endif
#else //ANDROID
    #define  NXT_BEGIN_PROFILE(xxdesc)
    #define  NXT_END_PROFILE(xxdesc)
    #define  NXT_PROFILE_CHECKPOINT(xxdesc)
#endif //ANDROID

#ifdef EGL_ERROR_CHECKING
    #define  CHECK_EGL_ERROR() NexThemeRenderer_CheckEglError(__LINE__,__func__)
    void NexThemeRenderer_CheckEglError(int line, const char *func);
#else
    #define  CHECK_EGL_ERROR()
    #define  NexThemeRenderer_CheckEglError(line,func)
#endif //EGL_ERROR_CHECKING

#ifdef GL_ERROR_CHECKING
    #define  CHECK_GL_ERROR() NexThemeRenderer_CheckGlError(__LINE__,__func__)
    void NexThemeRenderer_PrintGLString(const char *name, GLenum s);
    void NexThemeRenderer_CheckGlError(int line, const char *func);
#else
    #define  CHECK_GL_ERROR()
    #define  NexThemeRenderer_PrintGLString(n,s)
    #define  NexThemeRenderer_CheckGlError(line,func)
#endif //GL_ERROR_CHECKING    
    
#endif
