/******************************************************************************
* File Name   :	NexThemeRenderer.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include "NexTheme_Config.h"

#if defined(ANDROID)
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define EGL_SUPPORT
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IOS > 0
#define NEXTHEME_SUPPORT_OPENGL
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#ifdef EGL_SUPPORT
#undef EGL_SUPPORT
#endif
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#include "NexTheme.h"
#include "NexThemeRenderer.h"
#include "NexThemeRenderer_Internal.h"
#include "NexThemeRenderer_Platform.h"
#if defined(ANDROID)
#include "NexThemeRenderer_SurfaceTexture.h"
#endif
#include "NexTheme_Nodes.h"
#include "NexTheme_Internal.h"
#include "nexTexturesLogger.h"


#include "nxtRenderItem.h"

// #define KEEP_CONTEXT_DURING_EXPORT   
//#define USE_IMAGE_LOADER_FOR_LAYERS

#define USE_ASSET_MANAGER //yoon
#define LOAD_SHADER_ON_TIME
//#if defined(__APPLE__)
#define FORCE_RGB_CONVERTER
//#endif
#ifdef NEX_GL_ASYNC
#include <errno.h>
#endif

#ifdef NEX_ENABLE_GLES_IMAGE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#define PI_OVER_180	 0.017453292519943295769236907684886

#define  LOG_TAG    "NexThemeRenderer"

#define HALF_TEX_BORDER_SIZE 0
#define TEX_BORDER_SIZE (HALF_TEX_BORDER_SIZE*2)
#define TWICE_TEX_BORDER_SIZE (TEX_BORDER_SIZE*2)

// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_TEXCOORD,
    NUM_ATTRIBUTES
};

// ----

static NXT_Error initRenderer( NXT_HThemeRenderer renderer );
static void render_internal( NXT_HThemeRenderer renderer );
static void handleFastFloatArrayPreviewOption( NXT_HThemeRenderer renderer, char* optionName, int paramCount, float* params );
static void handleFastPreviewOption( NXT_HThemeRenderer renderer, char* optionName, int value, int& customlut_clip, int& customlut_power);
static void freeDecodedEffectOptions( NXT_AppliedEffect *appliedEffect );
static void freeAppliedEffect( NXT_AppliedEffect *appliedEffect );
static void configureShaderForLighting( NXT_HThemeRenderer renderer, NXT_Shader_LightingInfo *lightingInfo );
static void renderWatermark_effect(NXT_HThemeRenderer renderer, NXT_HThemeSet themeSet);
static void renderLayers(NXT_HThemeRenderer renderer);
static void renderWatermark_image(NXT_HThemeRenderer renderer);
static void autoSelectEffectFromCurrentCTS(NXT_HThemeRenderer renderer);
static void calcLocalTimeFromCurrentEffectAndCTS(NXT_HThemeRenderer renderer);
//Jeff
static void releaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target, unsigned int isDetachedConext);
static int checkRenderTargetInitilized(NXT_RenderTarget* prender_target);
static int getScaleStep(int src, int dest, int* pstart);
static void copyTexToTex(NXT_HThemeRenderer renderer, NXT_RenderTarget* pdest, NXT_RenderTarget* psrc);

static NXT_Error NXT_ThemeRenderer_GLDraw_Main(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType);
static void setRenderTargetForRTT(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int forced);
static NXT_Error beginRenderer(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType);
static void freeReadpixels(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType);
static void convertColorspace(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType);
static void renderSimplePlane(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse);
static void setViewportForRGB(NXT_HThemeRenderer renderer);
static void NXT_ThemeRenderer_UpdateRenderTargetManager(NXT_HThemeRenderer renderer);
static void renderSimplePlaneForTestFilter(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, NXT_RenderTarget* psrc, int reverse);
static void setRealXY(int x_index, int y_index, NXT_TextureInfo* texinfo, int bEGLI);
static void luminance_alpha_bugfix(NXT_HThemeRenderer renderer);
static void timeClipEffect( NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int actualEffectStartCTS, int actualEffectEndCTS);
static void render_effect( NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int reset_flag = 0);
static void renderSimplePlaneForLUT(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, GLuint tex_id_for_lut, int reverse, float realX, float realY, float half_pixel_size_x, float half_pixel_size_y, float alpha = 1.0f);
static void parseEffectOptions( NXT_AppliedEffect *appliedEffect );
static void applyRenderItemOption(NXT_HThemeRenderer renderer, NXT_AppliedEffect *appliedEffect);
// Case
int init_fastpreviewbuffer(NXT_HThemeRenderer renderer, NXT_RenderFastPreviewBuffer *prender_fastpreview, int w, int h);
void deinit_fastpreviewbuffer(NXT_RenderFastPreviewBuffer* prender_fastpreview, unsigned int isDetachedConext);
void settarget_fastpreviewbuffer(NXT_RenderFastPreviewBuffer* prender_fastpreview);
int check_fastpreviewbuffer_init(NXT_RenderFastPreviewBuffer* prender_fastpreview);
void setrendertarget_For_AddFastPreviewFrame(NXT_HThemeRenderer renderer, int iTime);
void setrendertarget_For_FastPreview(NXT_HThemeRenderer renderer, int iTime);
NXT_Error convertColorspace_For_FastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType);

NXT_Error NXT_ThemeRenderer_Add_FastPreviewFrame(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int iTime);
NXT_Error NXT_ThemeRenderer_GLDraw_Main_For_FastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int iTime);

bool NXT_RenderTarget::init(int w, int h, int alpha_request, int depth_request, int stencil_request){

    width_ = w;
    height_ = h;
    alpha_ = alpha_request;
    depth_ = depth_request;
    stencil_ = stencil_request;

    glGenFramebuffers(1, &fbo_);
    if (depth_request){
        glGenRenderbuffers(1, &depth_buffer_);
    }
    GL_GenTextures(1, &target_texture_);

    int color_format = alpha_request ? GL_RGBA : GL_RGB;

    glBindTexture(GL_TEXTURE_2D, target_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, color_format, w, h, 0, color_format, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (depth_request){
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
        if (stencil_request){
#if defined(ANDROID)
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
#elif defined(__APPLE__)
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
#else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
#endif
            
        }
        else{
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture_, 0);
    if (depth_request){
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
    }
    if (stencil_request){
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    inited_ = 1;
    return true;
}

bool NXT_RenderTarget::init_depthonly(int w, int h, int stencil_request){

    fbo_ = 0;
    width_ = w;
    height_ = h;
    alpha_ = 0;
    depth_ = 1;
    stencil_ = stencil_request;

    glGenRenderbuffers(1, &depth_buffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
    if (stencil_request){
#if defined(ANDROID)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
#elif defined(__APPLE__)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
#else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
#endif
        
    }
    else{
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
    }

    inited_ = 1;
    return true;
}

void NXT_RenderTarget::destroy(unsigned int isDetachedConext){

    if (inited_){
        if (0 == isDetachedConext){
            if (depth_buffer_ > 0) glDeleteRenderbuffers(1, &depth_buffer_);
            if (fbo_ > 0) glDeleteFramebuffers(1, &fbo_);
            if (target_texture_ > 0) GL_DeleteTextures(1, &target_texture_);
        }
        target_texture_ = 0;
        depth_buffer_ = 0;
        fbo_ = 0;
        width_ = 0;
        height_ = 0;
        inited_ = 0;
    }
}

NXT_RenderTarget* RenderTargetManager::getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request){

    for(RenderTargetResourceList_t::iterator itor = resource_.begin(); itor != resource_.end(); ++itor){

        RenderTargetResource& res = *itor;
        if( (res.prender_target_->fbo_ > 0)
            && (res.prender_target_->width_ == width)
            && (res.prender_target_->height_ == height)
            && (res.prender_target_->alpha_ == alpha_request)
            && (res.prender_target_->depth_ == depth_request)
            && (res.prender_target_->stencil_ == stencil_request)){

            NXT_RenderTarget* pret = res.prender_target_;
            resource_.erase(itor);

            return pret;
        }
    }

    NXT_RenderTarget* prender_target = new NXT_RenderTarget();
    prender_target->init(width, height, alpha_request, depth_request, stencil_request);
    return prender_target;
}

NXT_RenderTarget* RenderTargetManager::getDepthonly(int width, int height, int stencil_request){

    for(RenderTargetResourceList_t::iterator itor = resource_.begin(); itor != resource_.end(); ++itor){

        RenderTargetResource& res = *itor;
        if( (res.prender_target_->fbo_ == 0)
            && (res.prender_target_->width_ == width)
            && (res.prender_target_->height_ == height)
            && (res.prender_target_->depth_ == 1)
            && (res.prender_target_->stencil_ == stencil_request)){

            NXT_RenderTarget* pret = res.prender_target_;
            resource_.erase(itor);

            return pret;
        }
    }

    NXT_RenderTarget* prender_target = new NXT_RenderTarget();
    prender_target->init_depthonly(width, height, stencil_request);
    return prender_target;
}

void RenderTargetManager::releaseRenderTarget(NXT_RenderTarget* prendertarget){

    if (NULL == prendertarget)
        return;

    while(resource_.size() >= MAX_RENDER_TARGET_RESOURCE){

        RenderTargetResource* pres = &resource_.front();
        pres->prender_target_->destroy(0);
        delete pres->prender_target_;
        resource_.pop_front();
    }
    RenderTargetResource res;
    res.prender_target_ = prendertarget;
    res.unused_counter_ = 0;
    resource_.push_back(res);
}

void RenderTargetManager::update(){

    for(RenderTargetResourceList_t::iterator itor = resource_.begin(); itor != resource_.end();){

        RenderTargetResource& res = *itor;
        if(res.unused_counter_ > 1){

            res.prender_target_->destroy(0);
            delete res.prender_target_;
            resource_.erase(itor++);
        }
        else{
            res.unused_counter_++;
            ++itor;
        }
    }
}

void RenderTargetManager::destroy(){

    for(RenderTargetResourceList_t::iterator itor = resource_.begin(); itor != resource_.end(); ++itor){

        RenderTargetResource& res = *itor;
        res.prender_target_->destroy(0);
        delete res.prender_target_;
    }

    resource_.clear();
}

void NXT_ThemeRenderer_::setRenderTarget(NXT_RenderTarget* prendertarget){

    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (NULL == prendertarget){

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBufferNum);
#ifdef ANDROID
        view_width = nativeExportSurface?exportSurfaceWidth:nativeWindowWidth;
        view_height = nativeExportSurface?exportSurfaceHeight:nativeWindowHeight;
#endif
        glViewport(0, 0, view_width, view_height);
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, prendertarget->fbo_);
#ifdef ANDROID
    view_width = prendertarget->width_;
    view_height = prendertarget->height_;
#endif
    glViewport(0, 0, prendertarget->width_, prendertarget->height_);
}

void NXT_ThemeRenderer_::setRenderTarget(NXT_RenderTarget* prendertarget, NXT_RenderTarget* pdepthtarget){

    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (NULL == prendertarget){

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBufferNum);
#ifdef ANDROID
        view_width = nativeExportSurface?exportSurfaceWidth:nativeWindowWidth;
        view_height = nativeExportSurface?exportSurfaceHeight:nativeWindowHeight;
#endif
        glViewport(0, 0, view_width, view_height);
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, prendertarget->fbo_);
#ifdef ANDROID
    view_width = prendertarget->width_;
    view_height = prendertarget->height_;
#endif
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pdepthtarget->depth_buffer_);
    if(pdepthtarget->stencil_)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pdepthtarget->depth_buffer_);
    else
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ){

        CHECK_GL_ERROR();
        LOGE("setRenderTarget combined mode failed");
    }
    glViewport(0, 0, prendertarget->width_, prendertarget->height_);
}

void NXT_ThemeRenderer_::unbindDepth(NXT_RenderTarget* prendertarget){

    glBindFramebuffer(GL_FRAMEBUFFER, prendertarget->fbo_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ){

        CHECK_GL_ERROR();
        LOGE("unbindDepth failed");
    }
}


void NXT_ThemeRenderer_SetRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prendertarget){
	
    renderer->setRenderTarget(prendertarget);
}

void NXT_ThemeRenderer_SetRenderTargetCombined(NXT_HThemeRenderer renderer, NXT_RenderTarget* prendertarget, NXT_RenderTarget* pdepthonly){

    renderer->setRenderTarget(prendertarget, pdepthonly);
}

void NXT_ThemeRenderer_UnbindDepth(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target){

    renderer->unbindDepth(prender_target);
}

static int getScaleStep(int src, int dest, int* pstart){

	int start = *pstart = NXT_NextHighestPowerOfTwo(src);
	int end = NXT_NextHighestPowerOfTwo(dest);
	int step = 0;
	int i = 0;
	for (i = 0; i < MAX_DOWNSCALE_STEP; ++i){
		start /= 2;
		if (start < end)
			break;
		step++;
	}
	return step;
}

void NXT_ThemeRenderer_::copySurface(NXT_RenderTarget* pdest, NXT_RenderTarget* psrc){

    setRenderTarget(pdest);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	CHECK_GL_ERROR();
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR();

	NXT_ShaderProgram_Textured *sp = NULL;
	sp = &getContextManager()->texturedShaderPassthroughRGB;
#ifdef LOAD_SHADER_ON_TIME
	NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, psrc->target_texture_);              CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};
	
	glVertexAttribPointer(pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}


void NXT_ThemeRenderer_::scaleDownRender(){

    int last_step = down_scale_info_.size() - 1;

    RenderTargetManager* prender_target_manager = (RenderTargetManager*)getContextManager()->getRendertargetManager();
    
    for (int i = 0; i < last_step; ++i)
    {
        NXT_RenderTarget* prender_target = prender_target_manager->getRenderTarget(down_scale_info_[i + 1].width_, down_scale_info_[i + 1].height_, 0, 0, 0);
        copySurface(prender_target, pmain_render_target_);
        prender_target_manager->releaseRenderTarget(pmain_render_target_);
        pmain_render_target_ = prender_target;
        view_width = pmain_render_target_->width_;
        view_height = pmain_render_target_->height_;
    }
}

void NXT_ThemeRenderer_::setHQScale(int src_w, int src_h, int dest_w, int dest_h){
    
    float scale_factor_w = (float)dest_w / (float)src_w;
    float scale_factor_h = (float)dest_h / (float)src_h;
    
    if (scale_factor_w > 0.5f && scale_factor_h > 0.5f){

        down_scale_info_.clear();
        return;
    }

    int start_w = 0;
    int start_h = 0;
    int step_down_w = getScaleStep(src_w, dest_w, &start_w);
    int step_down_h = getScaleStep(src_h, dest_h, &start_h);

    down_scale_info_.clear();

    int step = (step_down_w > step_down_h) ? step_down_w : step_down_h;
    int i = 0;
    for (i = 0; i < step; ++i){
        start_w = (step_down_w-- > 0) ? start_w / 2 : start_w;
        start_h = (step_down_h-- > 0) ? start_h / 2 : start_h;
        NXT_Downscale scale;
        scale.width_ = start_w;
        scale.height_ = start_h;
        down_scale_info_.push_back(scale);
    }
}

void NXT_ThemeRenderer_SetHQScale(NXT_HThemeRenderer renderer, int src_w, int src_h, int dest_w, int dest_h){
	
    renderer->setHQScale(src_w, src_h, dest_w, dest_h);
}

void NXT_ThemeRenderer_::initVideoTextureResource(){

    NXT_TextureInfo tmp;
    memset(&tmp, 0x0, sizeof(tmp));

    videoTexture.reserve(NXT_TextureID_COUNT);
    for(int i = 0; i < NXT_TextureID_COUNT; ++i){

        videoTexture.push_back(tmp);
    }

    for(int i = NXT_TextureID_Video_Layer_START; i < NXT_TextureID_COUNT; ++i)
        left_videotexture_.push_back(i);
    
    inuse_videotexture_.clear();
    deactive_videotexture_.clear();

    inuse_decotexture_.clear();
    deactive_decotexture_.clear();

}

void NXT_ThemeRenderer_::releaseAllVideoTexture(){

    for(videoresourcemap_t::iterator itor = deactive_videotexture_.begin(); itor != deactive_videotexture_.end(); ++itor){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        NXT_TextureInfo* ptex = &videoTexture[slot_id];
        if(ptex->texNameInitCount > 0){

            GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
            ptex->texNameInitCount = 0;
        }
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
    }

    for(videoresourcemap_t::iterator itor = inuse_videotexture_.begin(); itor != inuse_videotexture_.end(); ++itor){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        NXT_TextureInfo* ptex = &videoTexture[slot_id];
        if(ptex->texNameInitCount > 0){

            GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
            ptex->texNameInitCount = 0;
        }
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
    }

    for(videoresourcemap_t::iterator itor = deactive_decotexture_.begin(); itor != deactive_decotexture_.end(); ++itor){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        NXT_TextureInfo* ptex = &videoTexture[slot_id];
        if(ptex->texNameInitCount > 0){

            GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
            ptex->texNameInitCount = 0;
        }
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
    }

    for(videoresourcemap_t::iterator itor = inuse_decotexture_.begin(); itor != inuse_decotexture_.end(); ++itor){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        NXT_TextureInfo* ptex = &videoTexture[slot_id];
        if(ptex->texNameInitCount > 0){

            GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
            ptex->texNameInitCount = 0;
        }
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
    }

    for(std::vector<NXT_TextureInfo>::iterator itor = videoTexture.begin(); itor != videoTexture.end(); ++itor){

        NXT_TextureInfo* ptex = &(*itor);
        if(ptex->texNameInitCount > 0){

            GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
            ptex->texNameInitCount = 0;
        }
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
    }
}

int NXT_ThemeRenderer_::createVideoTexture(unsigned int uid){

	videoresourcemap_t::iterator itor = inuse_videotexture_.find(uid);
	
	if(itor != inuse_videotexture_.end()){

        VideoResource& res = itor->second;
        res.addRef();
        LOGI("TM30 createVideoTexture uid:%d ref:%d ret:%d", uid, res.getRef(), res.slot_id_);
		return res.slot_id_;
	}

	itor = deactive_videotexture_.find(uid);
	if(itor != deactive_videotexture_.end()){

        VideoResource res = itor->second;
        res.addRef();
		int ret = res.slot_id_;
		deactive_videotexture_.erase(itor);
		inuse_videotexture_.insert(std::make_pair(uid, res));
        LOGI("TM30 createVideoTexture uid:%d ref:%d ret:%d", uid, res.getRef(), ret);
		return ret;
	}

	if(left_videotexture_.size() > 0){

		int ret = left_videotexture_.back();
		left_videotexture_.pop_back();
        VideoResource res(ret, 0);
        res.addRef();
		inuse_videotexture_.insert(std::make_pair(uid, res));
        LOGI("TM30 createVideoTexture uid:%d ref:%d ret:%d", uid, res.getRef(), ret);
		return ret;
	}

	itor = deactive_videotexture_.begin();
	if(itor != deactive_videotexture_.end()){

		VideoResource res = itor->second;
        int ret = res.slot_id_;
		deactive_videotexture_.erase(itor);
        res.addRef();
		inuse_videotexture_.insert(std::make_pair(uid, res));
		NXT_TextureInfo* ptex = &videoTexture[ret];
		ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
        LOGI("TM30 createVideoTexture uid:%d ref:%d ret:%d", uid, res.getRef(), ret);
		return ret;
	}

    LOGI("TM30 createVideoTexture uid:%d FAIL", uid);

	return -1;
}

int NXT_ThemeRenderer_::releaseVideoTexture(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_videotexture_.find(uid);
    
    if(itor != inuse_videotexture_.end()){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        if(res.decRef() <= 0){

            res.state_ = 0;
            deactive_videotexture_.insert(std::make_pair(uid, res));
            inuse_videotexture_.erase(itor);
            LOGI("TM30 releaseVideoTexture uid:%d ref:%d", uid, res.getRef());
        }
        else{

            LOGI("TM30 releaseVideoTexture uid:%d ref:%d", uid, res.getRef());
        }
        return slot_id;
    }
    else if(uid == 0xFFFFFFFF){

        releaseDecoTexture(uid);
        for(itor = inuse_videotexture_.begin(); itor != inuse_videotexture_.end(); ){

            VideoResource& res = itor->second;
            int slot_id = res.slot_id_;
            unsigned int uid = itor->first;
            if(res.decRef() <= 0){

                res.state_ = 0;
                deactive_videotexture_.insert(std::make_pair(uid, res));
                inuse_videotexture_.erase(itor++);
                LOGI("TM30 releaseVideoTexture uid:%d ref:%d", slot_id, res.getRef());
            }
            else{
                ++itor;
                LOGI("TM30 releaseVideoTexture uid:%d ref:%d", slot_id, res.getRef());
            }
        }
    }
    else{

        LOGI("TM30 releaseVideoTexture uid:%d not found", uid);
    }

    return -1;
}

void NXT_ThemeRenderer_::manageVideoTexture(){

    for(videoresourcemap_t::iterator itor = deactive_videotexture_.begin(); itor != deactive_videotexture_.end(); ){

        VideoResource& res = itor->second;
        if(res.state_++ > 1){

            int slot_id = res.slot_id_;
            deactive_videotexture_.erase(itor++);
            left_videotexture_.push_back(slot_id);
            NXT_TextureInfo* ptex = &videoTexture[slot_id];
            if(ptex->texNameInitCount > 0){

#ifdef ANDROID
				if(ptex->texNameInitCount == 1){

                    for(int i = 0; i < NXT_SURFACE_TEXTURE_MAX; i++) {

                        NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;
                        if(surfaceTextures[i].bUsed) {
                            pSurfTexInfo = &surfaceTextures[i];
                            if(pSurfTexInfo->surfaceTextureName == ptex->texName[0]){

                                ptex->texNameInitCount = 0;
                                break;
                            }
                        }
                    }
                }
#endif
				
                if(ptex->texNameInitCount > 0){

                    GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
                    ptex->texNameInitCount = 0;
                }
            }
            ptex->texName_for_rgb = 0;
            NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
            NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
            ptex->prendertarget_ = NULL;
            ptex->filtered_.prender_target_ = NULL;
            memset(ptex, 0x0, sizeof(*ptex));
        }
        else{

            ++itor;
        }
    }
}

int NXT_ThemeRenderer_::createDecoTexture(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_decotexture_.find(uid);
    
    if(itor != inuse_decotexture_.end()){

        VideoResource& res = itor->second;
        res.addRef();
        LOGI("TM30 createDecoTexture uid:%d ref:%d", uid, res.getRef());
        return res.slot_id_;
    }

    itor = deactive_decotexture_.find(uid);
    if(itor != deactive_decotexture_.end()){

        VideoResource res = itor->second;
        res.addRef();
        int ret = res.slot_id_;
        deactive_decotexture_.erase(itor);
        inuse_decotexture_.insert(std::make_pair(uid, res));
        LOGI("TM30 createDecoTexture uid:%d ref:%d", uid, res.getRef());
        return ret;
    }

    if(left_videotexture_.size() > 0){

        int ret = left_videotexture_.back();
        left_videotexture_.pop_back();
        VideoResource res(ret, 0);
        res.addRef();
        inuse_decotexture_.insert(std::make_pair(uid, res));
        LOGI("TM30 createDecoTexture uid:%d ref:%d", uid, res.getRef());
        return ret;
    }

    itor = deactive_decotexture_.begin();
    if(itor != deactive_decotexture_.end()){

        VideoResource res = itor->second;
        int ret = res.slot_id_;
        deactive_decotexture_.erase(itor);
        res.addRef();
        inuse_decotexture_.insert(std::make_pair(uid, res));
        NXT_TextureInfo* ptex = &videoTexture[ret];
        ptex->texName_for_rgb = 0;
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
        NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
        ptex->prendertarget_ = NULL;
        ptex->filtered_.prender_target_ = NULL;
        LOGI("TM30 createDecoTexture uid:%d ref:%d", uid, res.getRef());
        return ret;
    }

    LOGI("TM30 createDecoTexture uid:%d FAIL", uid);

    return -1;
}

int NXT_ThemeRenderer_::releaseDecoTexture(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_decotexture_.find(uid);
    
    if(itor != inuse_decotexture_.end()){

        VideoResource& res = itor->second;
        int slot_id = res.slot_id_;
        if(res.decRef() <= 0){
            
            res.state_ = 0;            
            deactive_decotexture_.insert(std::make_pair(uid, res));
            inuse_decotexture_.erase(itor);
            LOGI("TM30 releaseDecoTexture uid:%d ref:%d", uid, res.getRef());

            if(videoTexture[slot_id].base_track_id_ > 0){

                releaseVideoTexture(videoTexture[slot_id].base_track_id_);
            }
        }
        else{

            LOGI("TM30 releaseDecoTexture uid:%d ref:%d", uid, res.getRef());
        }
        return slot_id;
    }
    else if(uid == 0xFFFFFFFF){

        for(itor = inuse_decotexture_.begin(); itor != inuse_decotexture_.end(); ){

            VideoResource& res = itor->second;
            int slot_id = res.slot_id_;
            unsigned int uid = itor->first;
            if(res.decRef() <= 0){
                
                res.state_ = 0;            
                deactive_decotexture_.insert(std::make_pair(uid, res));
                inuse_decotexture_.erase(itor++);
                LOGI("TM30 releaseDecoTexture uid:%d ref:%d", slot_id, res.getRef());

                if(videoTexture[slot_id].base_track_id_ > 0){

                    releaseVideoTexture(videoTexture[slot_id].base_track_id_);
                }
            }
            else{

                ++itor;
                LOGI("TM30 releaseDecoTexture uid:%d ref:%d", slot_id, res.getRef());
            }
        }
    }

    return -1;
}

void NXT_ThemeRenderer_::manageDecoTexture(){

    for(videoresourcemap_t::iterator itor = deactive_decotexture_.begin(); itor != deactive_decotexture_.end(); ){

        VideoResource& res = itor->second;
        if(res.state_++ > 1){

            int slot_id = res.slot_id_;
            deactive_decotexture_.erase(itor++);
            left_videotexture_.push_back(slot_id);
            NXT_TextureInfo* ptex = &videoTexture[slot_id];
            if(ptex->texNameInitCount > 0){

                GL_DeleteTextures(ptex->texNameInitCount, ptex->texName);
                ptex->texNameInitCount = 0;
            }
            ptex->texName_for_rgb = 0;
            NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->prendertarget_);
            NXT_ThemeRenderer_ReleaseRenderTarget(this, ptex->filtered_.prender_target_);
            ptex->prendertarget_ = NULL;
            ptex->filtered_.prender_target_ = NULL;
            memset(ptex, 0x0, sizeof(*ptex));
        }
        else{

            ++itor;
        }
    }
}
int NXT_ThemeRenderer_::getTexName(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_videotexture_.find(uid);
    
    if(itor != inuse_videotexture_.end()){

        VideoResource& res = itor->second;
        return videoTexture[res.slot_id_].texName_for_rgb;
    }

    itor = deactive_videotexture_.find(uid);
    if(itor != deactive_videotexture_.end()){

        VideoResource& res = itor->second;
        res.state_ = 0;
        return videoTexture[res.slot_id_].texName_for_rgb;
    }
    return 0;
}

int NXT_ThemeRenderer_::getTexName(unsigned int uid, const float** ppmatrix){

    videoresourcemap_t::iterator itor = inuse_videotexture_.find(uid);
    
    if(itor != inuse_videotexture_.end()){

        VideoResource& res = itor->second;
        int simpleFormat = 0;
        NXT_TextureInfo& texinfo = videoTexture[res.slot_id_];
        *ppmatrix = texinfo.surfaceMatrix.e;
        if(texinfo.texName_for_rgb > 0)
            return texinfo.texName_for_rgb;
                
        simpleFormat = texinfo.textureFormat;

        if(simpleFormat == NXT_PixelFormat_RGBA8888)
            simpleFormat = 0;
        else
            simpleFormat = 1;

        return (simpleFormat << 24)|videoTexture[res.slot_id_].texName[0];
    }

    itor = deactive_videotexture_.find(uid);
    if(itor != deactive_videotexture_.end()){

        VideoResource& res = itor->second;
        res.state_ = 0;
        int simpleFormat = 0;
        NXT_TextureInfo& texinfo = videoTexture[res.slot_id_];
        *ppmatrix = texinfo.surfaceMatrix.e;
        if(texinfo.texName_for_rgb > 0)
            return texinfo.texName_for_rgb;
                
        simpleFormat = texinfo.textureFormat;

        if(simpleFormat == NXT_PixelFormat_RGBA8888)
            simpleFormat = 0;
        else
            simpleFormat = 1;

        return (simpleFormat << 24)|videoTexture[res.slot_id_].texName[0];
    }
    return 0;
}

int NXT_ThemeRenderer_::getRawVideoSrcForUID(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_videotexture_.find(uid);
    
    if(itor != inuse_videotexture_.end()){

        VideoResource& res = itor->second;
        return res.slot_id_;
    }

    itor = deactive_videotexture_.find(uid);
    if(itor != deactive_videotexture_.end()){

        VideoResource& res = itor->second;
        res.state_ = 0;
        return res.slot_id_;
    }
    return 0;
}

int NXT_ThemeRenderer_::getDecoVideoSrcForUID(unsigned int uid){

    videoresourcemap_t::iterator itor = inuse_decotexture_.find(uid);
    
    if(itor != inuse_decotexture_.end()){

        VideoResource& res = itor->second;
        return res.slot_id_;
    }

    itor = deactive_decotexture_.find(uid);
    if(itor != deactive_decotexture_.end()){

        VideoResource& res = itor->second;
        res.state_ = 0;
        return res.slot_id_;
    }
    return 0;
}


void NXT_ThemeRenderer_::linkTrack(unsigned int uid, unsigned int base_uid){

    videoresourcemap_t::iterator itor = inuse_decotexture_.find(uid);
    
    if(itor != inuse_decotexture_.end()){

        VideoResource& target = itor->second;
        itor = inuse_videotexture_.find(base_uid);
        if(itor != inuse_videotexture_.end()){

            VideoResource& base = itor->second;
            base.addRef();
            videoTexture[target.slot_id_].base_track_id_ = base_uid;
        }
        else{

            LOGI("TM30 base_uid:%d not found", base_uid);            
        }
    }
    else{

        LOGI("TM30 uid:%d not found", uid);            
    }
}

void NXT_ThemeRenderer_::setLUT(unsigned int uid, int lut){

    NXT_TextureInfo& target = getRawVideoSrc(getDecoVideoSrcForUID(uid));
    if(target.lut != lut){

        target.lut = lut;
        target.track_update_id_ = 0;        
    }
    
}

void NXT_ThemeRenderer_::setCustomLutA(unsigned int uid, int lut){

    NXT_TextureInfo& target = getRawVideoSrc(getDecoVideoSrcForUID(uid));
    if(target.custom_lut_a != lut){

        target.custom_lut_a = lut;
        target.track_update_id_ = 0;        
    }
    
}

void NXT_ThemeRenderer_::setCustomLutB(unsigned int uid, int lut){

    NXT_TextureInfo& target = getRawVideoSrc(getDecoVideoSrcForUID(uid));
    if(target.custom_lut_b != lut){

        target.custom_lut_b = lut;
        target.track_update_id_ = 0;        
    }
    
}

void NXT_ThemeRenderer_::setCustomLutPower(unsigned int uid, int power){

    NXT_TextureInfo& target = getRawVideoSrc(getDecoVideoSrcForUID(uid));
    if(target.custom_lut_power != power){

        target.custom_lut_power = power;
        target.track_update_id_ = 0;        
    }
    
}

void NXT_ThemeRenderer_SetLUT(NXT_HThemeRenderer renderer, unsigned int uid, int lut){

    renderer->setLUT(uid, lut);
}

void NXT_ThemeRenderer_SetCustomLUTA(NXT_HThemeRenderer renderer, unsigned int uid, int lut){

    renderer->setCustomLutA(uid, lut);
}

void NXT_ThemeRenderer_SetCustomLUTB(NXT_HThemeRenderer renderer, unsigned int uid, int lut){

    renderer->setCustomLutB(uid, lut);
}

void NXT_ThemeRenderer_SetCustomLUTPower(NXT_HThemeRenderer renderer, unsigned int uid, int power){

    renderer->setCustomLutPower(uid, power);
}

int NXT_ThemeRenderer_CreateTextureForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid){
	
    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->createVideoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_ReleaseTextureForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image){

    NXT_ThemeRenderer_AquireContext(renderer);
	int ret = renderer->releaseVideoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_CreateTextureForTrackID(NXT_HThemeRenderer renderer, unsigned int uid){
    
    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->createVideoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_ReleaseTextureForTrackID(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image){

    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->releaseVideoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_CreateDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid){

    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->createDecoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_GetDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid){

    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->getDecoVideoSrcForUID(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_ReleaseDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image){

    NXT_ThemeRenderer_AquireContext(renderer);
    int ret = renderer->releaseDecoTexture(uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_GetTextureNameForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid){

	NXT_ThemeRenderer_AquireContext(renderer);
    int real_uid = renderer->getRealUID(uid);
    int ret = renderer->getTexName(real_uid);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_GetTextureNameForVideoLayerExternal(NXT_HThemeRenderer renderer, unsigned int uid, const float** ppmatrix){

	NXT_ThemeRenderer_AquireContext(renderer);
    int real_uid = renderer->getRealUID(uid);
    int ret = renderer->getTexName(real_uid, ppmatrix);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    return ret;
}

int NXT_ThemeRenderer_GetTextureNameForMask(NXT_HThemeRenderer renderer){
	if (renderer->pmain_mask_target_)
		return renderer->pmain_mask_target_->target_texture_;
	return 0;
}

int NXT_ThemeRenderer_GetTextureNameForBlend(NXT_HThemeRenderer renderer){
    if (renderer->pmain_blend_target_)
        return renderer->pmain_blend_target_->target_texture_;
    return 0;
}

int NXT_ThemeRenderer_SwapBlendMain(NXT_HThemeRenderer renderer){

    NXT_RenderTarget* prender_target = renderer->pmain_blend_target_;
    renderer->pmain_blend_target_ = renderer->pmain_render_target_;
    renderer->pmain_render_target_ = prender_target;
    renderer->setRenderTarget(renderer->pmain_render_target_);
    return 0;
}

int NXT_ThemeRenderer_GetTextureNameForWhite(NXT_HThemeRenderer renderer){
	if (renderer->getContextManager()->whiteTexture)
		return renderer->getContextManager()->whiteTexture;
	return 0;
}

#include <algorithm>

struct EffectMasterLess{

    EffectMasterLess(EffectMaster::effectmap_t& effectmap):effectmap_(effectmap){

    }

    bool operator()(unsigned int a, unsigned int b) const{

        EffectMaster::effectmap_t::iterator itor = effectmap_.find(a);
        EffectMaster::EffectInfo* peffect_a = itor->second;
        itor = effectmap_.find(b);
        EffectMaster::EffectInfo* peffect_b = itor->second;

        return peffect_a->effect_info_.effectStartTime < peffect_b->effect_info_.effectStartTime;
    }

private:
    EffectMaster::effectmap_t& effectmap_;
};

void EffectMaster::initEDLSerial(){

    edl_serial_ = 0;
}

void EffectMaster::initTextSerial(){

    text_serial_ = 0;
}

unsigned int EffectMaster::issueEDLSerial(){

    edl_serial_++;
    if(edl_serial_ == 0)
        edl_serial_ = 1;
    return edl_serial_;
}

unsigned int EffectMaster::issueTextSerial(){

    return 1;

    // text_serial_++;
    // if(text_serial_ == 0)
    //     text_serial_ = 1;
    // return text_serial_;
}

void EffectMaster::releaseEDLSerial(unsigned int serial){

    //do something
}

void EffectMaster::releaseTextSerial(unsigned int serial){

    //do something
}

bool EffectMaster::setEffectOption(unsigned int effect_uid, const char* effectOptions){

    EffectInfo* peffectinfo = NULL;
    effectmap_t::iterator itor = effectmap_.find(effect_uid);
    if(itor == effectmap_.end()){

        LOGI("TM30 there's no effect for uid:%d", effect_uid);
        return false;
    }

    peffectinfo = itor->second;

    NXT_AppliedEffect* appliedEffect = &peffectinfo->effect_info_;

    if( effectOptions == NULL) {

        if( appliedEffect->effectOptionsEncoded ) {
            
            freeAppliedEffect(appliedEffect);
        }
    } else {
        if( appliedEffect->effectOptionsEncoded==NULL || strcmp(appliedEffect->effectOptionsEncoded, effectOptions)!=0 ) {

            freeAppliedEffect(appliedEffect);

            appliedEffect->effectOptionsEncoded = (char*)malloc(strlen(effectOptions)+1);
            if( !appliedEffect->effectOptionsEncoded ) {
                LOGE("[%s %d] malloc failure", __func__, __LINE__);
            }
            strcpy(appliedEffect->effectOptionsEncoded, effectOptions);
            
            parseEffectOptions(appliedEffect);
            appliedEffect->render_item_encoded = 0;
            appliedEffect->bEffectChanged = 1;
        }
    }

    return true;
}

void EffectMaster::clear(){

    effectqueue_.clear();
    transitionqueue_.clear();

    for(effectmap_t::iterator itor = effectmap_.begin(); itor != effectmap_.end(); ++itor){

        delete itor->second;
    }
    effectmap_.clear();
}

void EffectMaster::removePending(){

    for(effectmap_t::iterator itor = effectmap_.begin(); itor != effectmap_.end(); ){

        EffectInfo* peffectinfo = itor->second;
        unsigned int effect_uid = itor->first;

        if(peffectinfo->pending_){

            effectmap_.erase(itor++);
            releaseTextSerial(peffectinfo->effect_info_.textSerial);

            bool sort_required(false);

            for(effectqueue_t::iterator itor = effectqueue_.begin(); itor != effectqueue_.end(); ++itor){

                if(*itor == effect_uid){

                    effectqueue_.erase(itor);
                    releaseEDLSerial(effect_uid);
                    break;
                }
            }

            for(effectqueue_t::iterator itor = transitionqueue_.begin(); itor != transitionqueue_.end(); ++itor){

                if(*itor == effect_uid){

                    transitionqueue_.erase(itor);
                    releaseEDLSerial(effect_uid);
                    break;
                }
            }
            delete peffectinfo;
        }
        else
            ++itor;
    }
}

unsigned int EffectMaster::startTitleEDL(
                                       const char* effectId,
                                       const char* effectOptions,
                                       int startTrackId,
                                       int effectIndex,
                                       int totalEffectCount,
                                       int effectStartTime,
                                       int effectEndTime){

    
    removePending();
    unsigned int effect_uid = issueEDLSerial();
    EffectInfo* peffectinfo = NULL;
    effectmap_t::iterator itor = effectmap_.find(effect_uid);
    if(itor != effectmap_.end()){

        //Something wrong happen

        peffectinfo = itor->second;
        peffectinfo->start_track_id_ = startTrackId;
        return 0x0;
    }
    else{

        peffectinfo = new EffectInfo();        
        peffectinfo->start_track_id_ = startTrackId;
        peffectinfo->uid_ = effect_uid;
        peffectinfo->pending_ = 0;
        peffectinfo->effect_info_.effectType = NXT_EffectType_Title;
        strcpy(peffectinfo->effect_info_.effectId, effectId);
        peffectinfo->effect_info_.effectStartTime = effectStartTime;
        peffectinfo->effect_info_.effectEndTime = effectEndTime;
        peffectinfo->effect_info_.clip_count = totalEffectCount;
        peffectinfo->effect_info_.clip_index = effectIndex;
        peffectinfo->effect_info_.textSerial = issueTextSerial();
        effectmap_.insert(std::make_pair(effect_uid, peffectinfo));

        NXT_AppliedEffect* appliedEffect = &peffectinfo->effect_info_;

        if( effectOptions == NULL) {
            
            freeAppliedEffect(appliedEffect);
        } else {
            if( appliedEffect->effectOptionsEncoded==NULL || strcmp(appliedEffect->effectOptionsEncoded, effectOptions)!=0 ) {
                
                freeAppliedEffect(appliedEffect);
                appliedEffect->effectOptionsEncoded = (char*)malloc(strlen(effectOptions)+1);
                if( !appliedEffect->effectOptionsEncoded ) {
                    LOGE("[%s %d] malloc failure", __func__, __LINE__);
                }
                strcpy(appliedEffect->effectOptionsEncoded, effectOptions);
                
                parseEffectOptions(appliedEffect);
                appliedEffect->render_item_encoded = 0;
                appliedEffect->bEffectChanged = 1;

            }
        }
    }
    effectqueue_.push_back(effect_uid);

    std::sort(effectqueue_.begin(), effectqueue_.end(), EffectMasterLess(effectmap_));

    return effect_uid;
}

unsigned int EffectMaster::startTransitionEDL(
                                            const char* effectId,
                                            const char* effectOptions,
                                            int effectIndex,
                                            int totalEffectCount,
                                            int effectStartTime,
                                            int effectEndTime){

    removePending();
    unsigned int effect_uid = issueEDLSerial();
    EffectInfo* peffectinfo = NULL;
    effectmap_t::iterator itor = effectmap_.find(effect_uid);
    if(itor != effectmap_.end()){

        //Something wrong happen

        peffectinfo = itor->second;
        return 0x0;
    }
    else{

        peffectinfo = new EffectInfo();
        peffectinfo->start_track_id_ = -1;
        peffectinfo->uid_ = effect_uid;
        peffectinfo->pending_ = 0;
        peffectinfo->effect_info_.effectType = NXT_EffectType_Transition;
        strcpy(peffectinfo->effect_info_.effectId, effectId);
        peffectinfo->effect_info_.effectStartTime = effectStartTime;
        peffectinfo->effect_info_.effectEndTime = effectEndTime;
        peffectinfo->effect_info_.clip_count = totalEffectCount;
        peffectinfo->effect_info_.clip_index = effectIndex;
        peffectinfo->effect_info_.textSerial = issueTextSerial();
        effectmap_.insert(std::make_pair(effect_uid, peffectinfo));

        NXT_AppliedEffect* appliedEffect = &peffectinfo->effect_info_;

        if( effectOptions == NULL) {

            freeAppliedEffect(appliedEffect);
        } else {
            if( appliedEffect->effectOptionsEncoded==NULL || strcmp(appliedEffect->effectOptionsEncoded, effectOptions)!=0 ) {
                
                freeAppliedEffect(appliedEffect);
                appliedEffect->effectOptionsEncoded = (char*)malloc(strlen(effectOptions)+1);
                if( !appliedEffect->effectOptionsEncoded ) {
                    LOGE("[%s %d] malloc failure", __func__, __LINE__);
                }
                strcpy(appliedEffect->effectOptionsEncoded, effectOptions);
                
                parseEffectOptions(appliedEffect);
                appliedEffect->render_item_encoded = 0;
                appliedEffect->bEffectChanged = 1;

            }
        }
    }
    peffectinfo->childlist_.clear();
    peffectinfo->childlist_.push_back(effect_uid - 1);
    peffectinfo->childlist_.push_back(effect_uid + 1);

    transitionqueue_.push_back(effect_uid);
    std::sort(transitionqueue_.begin(), transitionqueue_.end(), EffectMasterLess(effectmap_));

    return effect_uid;
}

void EffectMaster::endEDL(unsigned int effect_uid){

    effectmap_t::iterator itor = effectmap_.find(effect_uid);
    if(itor == effectmap_.end()){

        LOGI("TM30 endEDL find fail uid:%d", effect_uid);
        return;
    }
    EffectInfo* peffectinfo = itor->second;
    peffectinfo->pending_ = 1;
    
    // effectmap_.erase(itor);
    // releaseTextSerial(peffectinfo->effect_info_.textSerial);

    // bool sort_required(false);

    // for(effectqueue_t::iterator itor = effectqueue_.begin(); itor != effectqueue_.end(); ++itor){

    //     if(*itor == effect_uid){

    //         effectqueue_.erase(itor);
    //         releaseEDLSerial(effect_uid);
    //         break;
    //     }
    // }

    // for(effectqueue_t::iterator itor = transitionqueue_.begin(); itor != transitionqueue_.end(); ++itor){

    //     if(*itor == effect_uid){

    //         transitionqueue_.erase(itor);
    //         releaseEDLSerial(effect_uid);
    //         break;
    //     }
    // }
    // delete peffectinfo;
}


bool EffectMaster::playEDL(NXT_HThemeRenderer renderer, int currentTime){

    if(transitionqueue_.size() <= 0 && effectqueue_.size() <= 0){


        return false;
    }

    // resetVideoSrc();

    typedef std::map<unsigned int, NXT_RenderTarget*> RenderTargetMapper_t;
    RenderTargetMapper_t rendertargetmapper;

    int texinfo_pointer = 0;
    NXT_TextureInfo texinfo[NXT_TextureID_COUNT];

    for(effectqueue_t::iterator itor = transitionqueue_.begin(); itor != transitionqueue_.end(); ++itor){

        unsigned int uid = *itor;
        effectmap_t::iterator itor_effect = effectmap_.find(uid);
        if(itor_effect == effectmap_.end()){

            continue;
        }

        EffectInfo* peffectinfo = itor_effect->second;
        NXT_AppliedEffect& effectinfo = peffectinfo->effect_info_;
        if(effectinfo.effectStartTime > currentTime || effectinfo.effectEndTime < currentTime){

            //out of playtime
            continue;
        }

        EffectInfo::childlist_t& childlist = peffectinfo->childlist_;

        for(EffectInfo::childiter itor = childlist.begin(); itor != childlist.end(); ++itor){

            NXT_RenderTarget* prendertarget = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 1, 1, 1);
            rendertargetmapper.insert(std::make_pair(*itor, prendertarget));
        }
    }

    for(effectqueue_t::iterator itor = effectqueue_.begin(); itor != effectqueue_.end(); ++itor){

        unsigned int uid = *itor;
        effectmap_t::iterator itor_effect = effectmap_.find(uid);
        if(itor_effect == effectmap_.end()){

            continue;
        }

        EffectInfo* peffectinfo = itor_effect->second;
        NXT_AppliedEffect& effectinfo = peffectinfo->effect_info_;
        if(effectinfo.effectStartTime > currentTime || effectinfo.effectEndTime < currentTime){

            //out of playtime
            LOGI("TM30 effect uid:%d out of playtime effectStartTime:%d effectEndTime:%d currentTime:%d", *itor, effectinfo.effectStartTime, effectinfo.effectEndTime, currentTime);
            continue;
        }

        bool check_return_to_default = false;
        NXT_RenderTarget* backup_for_mainrendertarget = NULL;        
        RenderTargetMapper_t::iterator itor_rt = rendertargetmapper.find(*itor);
        if(itor_rt != rendertargetmapper.end()){

            backup_for_mainrendertarget = renderer->pmain_render_target_;
            renderer->pmain_render_target_ = itor_rt->second;
            NXT_ThemeRenderer_SetRenderTarget(renderer, itor_rt->second);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
            check_return_to_default = true;
        }
        renderer->renderEDL(&effectinfo, peffectinfo->uid_, peffectinfo->start_track_id_);
        if(check_return_to_default){

            renderer->pmain_render_target_ = backup_for_mainrendertarget;
            NXT_ThemeRenderer_SetRenderTarget(renderer, renderer->pmain_render_target_);
        }
    }

    NXT_ThemeRenderer_SetRenderToDefault(renderer);

    for(effectqueue_t::iterator itor = transitionqueue_.begin(); itor != transitionqueue_.end(); ++itor){

        unsigned int uid = *itor;
        effectmap_t::iterator itor_effect = effectmap_.find(uid);
        if(itor_effect == effectmap_.end()){

            continue;
        }

        EffectInfo* peffectinfo = itor_effect->second;
        NXT_AppliedEffect& effectinfo = peffectinfo->effect_info_;
        if(effectinfo.effectStartTime > currentTime || effectinfo.effectEndTime < currentTime){

            //out of playtime
            continue;
        }
        //Do Render with rendertargetmapper?
        EffectInfo::childlist_t& childlist = peffectinfo->childlist_;
        int video_target = NXT_TextureID_Video_1;
        for(EffectInfo::childiter itor = childlist.begin(); itor != childlist.end(); ++itor){

            RenderTargetMapper_t::iterator iter_rt = rendertargetmapper.find(*itor);
            if(iter_rt != rendertargetmapper.end()){

                NXT_TextureInfo* ptexinfo = &texinfo[texinfo_pointer++];
                memset(ptexinfo, 0x0, sizeof(*ptexinfo));
                NXT_RenderTarget* prendertarget = iter_rt->second;
                
                ptexinfo->left = 0;
                ptexinfo->right = 100000;
                ptexinfo->top = 0;
                ptexinfo->bottom = 100000;
                ptexinfo->bHasInputRect = 1;
                ptexinfo->srcWidth = prendertarget->width_;
                ptexinfo->srcHeight = prendertarget->height_;
                ptexinfo->textureWidth = prendertarget->width_;
                ptexinfo->textureHeight = prendertarget->height_;
                ptexinfo->bValidTexture = 1;
                ptexinfo->textureFormat = NXT_PixelFormat_RGBA8888;
                ptexinfo->texName_for_rgb = prendertarget->target_texture_;
                ptexinfo->prendertarget_ = prendertarget;

                renderer->setVideoSrc(video_target++, ptexinfo);
            }
        }
        renderer->renderEDL(&effectinfo, peffectinfo->uid_, peffectinfo->start_track_id_);
    }

    for(RenderTargetMapper_t::iterator itor = rendertargetmapper.begin(); itor != rendertargetmapper.end(); ++itor){

        NXT_ThemeRenderer_ReleaseRenderTarget(renderer, itor->second);
    }
    // renderer->resetVideoSrc();

    return true;
}

bool NXT_ThemeRenderer_::setEffectOption(unsigned int uid, const char* effectOptions){

    return effectmaster_.setEffectOption(uid, effectOptions);
}

void NXT_ThemeRenderer_::clearEffect(){

    effectmaster_.clear();
}

unsigned int NXT_ThemeRenderer_::startTitleEDL(
                                       const char* effectId,
                                       const char* pEffectOptions,
                                       int startTrackId,
                                       int effectIndex,
                                       int totalEffectCount,
                                       int effectStartTime,
                                       int effectEndTime){

    return effectmaster_.startTitleEDL(effectId, pEffectOptions, startTrackId, effectIndex, totalEffectCount, effectStartTime, effectEndTime);
}

unsigned int NXT_ThemeRenderer_::startTransitionEDL(
                                            const char* effectId,
                                            const char* pEffectOptions,
                                            int effectIndex,
                                            int totalEffectCount,
                                            int effectStartTime,
                                            int effectEndTime){

    return effectmaster_.startTransitionEDL(effectId, pEffectOptions, effectIndex, totalEffectCount, effectStartTime, effectEndTime);
}

void NXT_ThemeRenderer_::endEDL(unsigned int effect_uid){

    effectmaster_.endEDL(effect_uid);
}


bool NXT_ThemeRenderer_::playEDL(int currentTime){

    resetVideoSrc();
    if(effectmaster_.playEDL(this, currentTime)){

        resetVideoSrc();
        return true;
    }
    return false;
}

void NXT_ThemeRenderer_::setVideoSrc(int target, NXT_TextureInfo* psrc){

    video_[target] = psrc;
}

void NXT_ThemeRenderer_::applyLUT(NXT_TextureInfo& tex){

    int lut_id = tex.lut;
    int customlut_a_id = tex.custom_lut_a;
    int customlut_b_id = tex.custom_lut_b;
    int customlut_power = tex.custom_lut_power;
    int tex_id_for_lut = 0;

    int texid_for_customlut_a = 0;
    int texid_for_customlut_b = 0;

    if(customlut_a_id == 0 && customlut_b_id == 0 && lut_id == 0){

        return;
    }

    GLboolean cull_face = 0;
    glGetBooleanv(GL_CULL_FACE, &cull_face);
    if(cull_face)
        glDisable(GL_CULL_FACE);

    releaseRenderTarget(tex.prendertarget_);
    tex.prendertarget_ = NULL;

    if(customlut_a_id > 0){

        texid_for_customlut_a = NXT_ThemeRenderer_GetLUTTexture(this, customlut_a_id);
        if(texid_for_customlut_a == 0){

            int tex_id = NXT_ThemeRenderer_GetLutTexWithID(this, customlut_a_id);
            if(tex_id != 0)
                texid_for_customlut_a = NXT_ThemeRenderer_SetLUTTexture(this, customlut_a_id, tex_id);
        }
    }

    if(customlut_b_id > 0){

        texid_for_customlut_b = NXT_ThemeRenderer_GetLUTTexture(this, customlut_b_id);
        if(texid_for_customlut_b == 0){

            int tex_id = NXT_ThemeRenderer_GetLutTexWithID(this, customlut_b_id);
            if(tex_id != 0)
                texid_for_customlut_b = NXT_ThemeRenderer_SetLUTTexture(this, customlut_b_id, tex_id);
        }
    }

    if(lut_id != 0){
        
       tex_id_for_lut = NXT_ThemeRenderer_GetLUTTexture(this, lut_id);
       if(tex_id_for_lut == 0){

            int tex_id = NXT_ThemeRenderer_GetLutTexWithID(this, lut_id);
            if(tex_id != 0)
                tex_id_for_lut = NXT_ThemeRenderer_SetLUTTexture(this, lut_id, tex_id);
       }
    }

    if(tex.base_track_id_ == 0){

        tex.textureFormat = tex.origin_textureformat_;
        NXT_ThemeRenderer_ConvertTextureToRGB(this, &tex, texid_for_customlut_a, texid_for_customlut_b, customlut_power, tex_id_for_lut, NULL, tex.color_transfer_function_id_, tex.max_cll_);
        if(cull_face)
            glEnable(GL_CULL_FACE);
        return;
    }
    
    if(texid_for_customlut_a > 0 || texid_for_customlut_b > 0){

        glEnable(GL_BLEND);
        CHECK_GL_ERROR();
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL_ERROR();   

        float boarder_size = 0.0f;

        if(customlut_power >= 100000){

            //draw by lutA only
            if(texid_for_customlut_a > 0){

                NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
                setRenderTarget(prender_target);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_a, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f);
                releaseRenderTarget(tex.prendertarget_);
            
                tex.textureFormat = NXT_PixelFormat_RGBA8888;
                tex.texName_for_rgb = prender_target->target_texture_;
                tex.prendertarget_ = prender_target;
                setRenderToDefault();
            }
        }
        else if(customlut_power <= 0){

            //draw by lutB only
            if(texid_for_customlut_b > 0){

                NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
                setRenderTarget(prender_target);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_b, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f);
                releaseRenderTarget(tex.prendertarget_);
            
                tex.textureFormat = NXT_PixelFormat_RGBA8888;
                tex.texName_for_rgb = prender_target->target_texture_;
                tex.prendertarget_ = prender_target;
                setRenderToDefault();
            }
        }
        else{

            if(texid_for_customlut_a > 0 && texid_for_customlut_b > 0){

                NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
                setRenderTarget(prender_target);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_a, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_b, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f, 1.0f - (float)customlut_power / 100000);
                releaseRenderTarget(tex.prendertarget_);
            
                tex.textureFormat = NXT_PixelFormat_RGBA8888;
                tex.texName_for_rgb = prender_target->target_texture_;
                tex.prendertarget_ = prender_target;
                setRenderToDefault();
            }
            else if(texid_for_customlut_a > 0){

                NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
                setRenderTarget(prender_target);
                renderSimplePlane(this, &getContextManager()->texturedShaderPassthroughRGB, tex.texName_for_rgb, 1);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_a, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f, (float)customlut_power / 100000);
                releaseRenderTarget(tex.prendertarget_);
            
                tex.textureFormat = NXT_PixelFormat_RGBA8888;
                tex.texName_for_rgb = prender_target->target_texture_;
                tex.prendertarget_ = prender_target;
                setRenderToDefault();
            }
            else if(texid_for_customlut_b > 0){

                NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
                setRenderTarget(prender_target);
                renderSimplePlane(this, &getContextManager()->texturedShaderPassthroughRGB, tex.texName_for_rgb, 1);
                renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, texid_for_customlut_b, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f, 1.0f - (float)customlut_power / 100000);
                releaseRenderTarget(tex.prendertarget_);
            
                tex.textureFormat = NXT_PixelFormat_RGBA8888;
                tex.texName_for_rgb = prender_target->target_texture_;
                tex.prendertarget_ = prender_target;
                setRenderToDefault();
            }
        }
    }

    if(tex_id_for_lut){

        NXT_RenderTarget* prender_target = getRenderTarget(tex.textureWidth, tex.textureHeight, 0, 0, 0);
        setRenderTarget(prender_target);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderSimplePlaneForLUT(this, &getContextManager()->texturedShaderLUT, tex.texName_for_rgb, tex_id_for_lut, 1, (float)tex.srcWidth / (float)tex.textureWidth, (float)tex.srcHeight / (float)tex.textureHeight, 0.0f, 0.0f);
        releaseRenderTarget(tex.prendertarget_);
    
        tex.textureFormat = NXT_PixelFormat_RGBA8888;
        tex.texName_for_rgb = prender_target->target_texture_;
        tex.prendertarget_ = prender_target;
        setRenderToDefault();
    }
}

NXT_TextureInfo& NXT_ThemeRenderer_::getVideoSrc(int target){

    if(video_[target]){

        if(video_[target]->base_track_id_){

            int slot = getRawVideoSrcForUID(video_[target]->base_track_id_);

            NXT_TextureInfo& target_info = *video_[target];
            NXT_TextureInfo& src_info = getRawVideoSrc(slot);

            if(target_info.track_update_id_ != src_info.track_update_id_){

                target_info.bValidTexture = 1;
                target_info.track_update_id_ = src_info.track_update_id_;
                target_info.textureFormat = NXT_PixelFormat_RGBA8888;
                target_info.texName_for_rgb = src_info.texName_for_rgb;
                target_info.srcWidth = src_info.srcWidth;
                target_info.srcHeight = src_info.srcHeight;
                target_info.textureWidth = src_info.textureWidth;
                target_info.textureHeight = src_info.textureHeight;
                target_info.serial_++;
                applyLUT(target_info);
            }

            
            return target_info;
        }
        else{

            LOGI("TM30 getVideoSrc(%d) -> real slot:%d", target, target);
        }
    }

    return *video_[target];
}

NXT_TextureInfo& NXT_ThemeRenderer_::getRawVideoSrc(int target){

    return videoTexture[target];
}

void NXT_ThemeRenderer_::findTexIDAndClear(int tex_id){

    for(int i = 0; i < NXT_TextureID_COUNT; ++i){

        if(videoTexture[i].texName[0] == tex_id){

            videoTexture[i].texNameInitCount = 0;
            videoTexture[i].texName[0] =  0;
        }
    }
}

void NXT_ThemeRenderer_::resetVideoSrc(){

    for(int i = 0; i < NXT_TextureID_COUNT; ++i){

        video_[i] = &videoTexture[i];
    }
}

void NXT_ThemeRenderer_::resetVideoSrc(int target){

    video_[target] = &videoTexture[target];
}

void NXT_ThemeRenderer_::renderEDL(NXT_AppliedEffect* peffect, unsigned int uid, int start_track_id){

    current_track_id_ = start_track_id;
    resetVideoTrack();



    int cts = this->cts;

    if( peffect!=NULL ) {
        int duration = peffect->effectEndTime - peffect->effectStartTime;
        int currenttime = cts - peffect->effectStartTime;
        this->cur_time = currenttime;
        this->max_set_time = duration;
        this->max_time = duration;
        this->settime = ((float)currenttime) / ((float)duration);
        this->time = this->settime;
        this->effectStartTime = peffect->effectUserStartTime;
        this->effectEndTime = peffect->effectUserEndTime;

        pthread_mutex_lock(&this->themesetLock);
        
        NXT_HThemeSet themeset = this->themeSet;

        if( themeset!=NULL ) {
            NXT_ThemeSet_Retain(themeset);
        }
        
        pthread_mutex_unlock(&this->themesetLock);

        NXT_HEffect cur_effect = NULL;
        
        cur_effect = NXT_ThemeSet_GetEffectById(themeset, peffect->effectId);
        if(cur_effect == NULL){

            LOGI("TM30 renderEDL can't find out effect %s on theme", peffect->effectId);
        }
        this->effectType = NXT_Effect_GetType(cur_effect);
        this->clip_count = peffect->clip_count;
        this->clip_index = peffect->clip_index;
        this->cur_effect_overlap = NXT_Effect_GetIntProperty(cur_effect, NXT_EffectProperty_VideoOverlap);

        unsigned int cur_time = this->cur_time;

        this->texture = NULL;
        this->mask = NULL;
        this->alpha  = 1.0;
        this->renderDest = NXT_RenderDest_Screen;
        this->renderTest = NXT_RenderTest_None;
        
        this->max_time = this->max_set_time;
        
        glColorMask(this->colorMask[0], this->colorMask[1], this->colorMask[2], this->colorMask[3]);
        
        unsigned int actualEffectStartCTS = 0;
        unsigned int actualEffectEndCTS = 0;
        
        unsigned int bUseEffect = 0;
        if( cur_effect && NXT_Node_ClassCheck((NXT_NodeHeader*)cur_effect,&NXT_NodeClass_Effect)) {
            NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)cur_effect);
            if( pEffect->effectType == NXT_EffectType_Title ) {
                int endTime = this->effectEndTime < this->max_time ? this->effectEndTime : this->max_time;
                if( this->effectStartTime >= 0 && endTime > 0 && endTime > this->effectStartTime && pEffect->bUserDuration ) {
                    if( cur_time >= this->effectStartTime && cur_time <= endTime ) {
                        bUseEffect = 1;
                        cur_time -= this->effectStartTime;
                        this->max_time = endTime - this->effectStartTime;
                        actualEffectStartCTS = peffect->effectStartTime + this->effectStartTime;
                        actualEffectEndCTS = peffect->effectStartTime + endTime;
                    }
                    else
                        LOGI("TM30 renderEDL out here %d", __LINE__);
                //} else if( pEffect->maxTitleDuration > 0 && this->max_time > pEffect->maxTitleDuration) {
                //    this->max_time = pEffect->maxTitleDuration;
                } else if( cur_time <= this->max_time ) {
                    actualEffectStartCTS = peffect->effectStartTime;
                    actualEffectEndCTS = peffect->effectEndTime;
                    bUseEffect = 1;
                }
                else
                    LOGI("TM30 renderEDL out here %d", __LINE__);
            } else {
                actualEffectStartCTS = peffect->effectStartTime;
                actualEffectEndCTS = peffect->effectEndTime;
                bUseEffect = 1;
            }
        }
        else{

            LOGI("TM30 renderEDL out here %d", __LINE__);
        }
        
        if( bUseEffect ) {


            NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)cur_effect);

            pActiveEffect = peffect;

            if(peffect->bEffectChanged) {
                
                peffect->bEffectChanged = 0;
                LOGI("[%s %d] Effect changed: Updating user fields", __func__, __LINE__ );
                NXT_Effect_UpdateUserFields(cur_effect, this);
            }
            
            luminance_alpha_bugfix(this);
            
            if( pEffect->effectType == NXT_EffectType_Title ) {
                timeClipEffect( this, cur_effect, cur_time, actualEffectStartCTS, actualEffectEndCTS );
            } else {
                this->time = this->settime;
                this->part = NXT_PartType_Mid;
            }

            this->currentTime = cur_time;
            this->actualEffectStartCTS = actualEffectStartCTS;
            this->actualEffectEndCTS = actualEffectEndCTS;        
            render_effect(this, cur_effect, cur_time, 1);
        }
        else
        {
            NXT_Node_Texture textureNode1 = {0};
            NXT_Node_Texture textureNode2 = {0};
            textureNode1.textureType = NXT_TextureType_Video;
            textureNode1.videoSource = 1;
            textureNode2.textureType = NXT_TextureType_Video;
            textureNode2.videoSource = 2;
            NXT_TextureInfo *texture = NULL;
            NXT_TextureInfo *mask = NULL;

            if (peffect)
            {

                int render_item_id = NXT_Theme_GetEffectID(getRenderItemManager(), peffect->effectId);
                cur_effect_overlap = NXT_Theme_GetEffectOverlap(getRenderItemManager(), render_item_id);
                NXT_Theme_ClearSrcInfo(getRenderItemManager());

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glEnable(GL_BLEND);
                CHECK_GL_ERROR();
                glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);

                NXT_Theme_BeginRenderItem(getRenderItemManager(), render_item_id, view_width, view_height, pmain_render_target_);
                applyRenderItemOption(this, peffect);

                NXT_Matrix4f texmat, colormat;
                NexThemeRenderer_PrepStateForRender2(this, &textureNode2, NULL, &texture, &mask, &texmat, &colormat, 0);

                NXT_Theme_SetTextureInfoTargetRenderItem(getRenderItemManager(),
                                                            texture->texName_for_rgb,
                                                            texture->textureWidth,
                                                            texture->textureHeight,
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_RIGHT);

                NXT_Theme_SetTexMatrix(getRenderItemManager(), &texmat, E_TARGET_VIDEO_RIGHT);
                NXT_Theme_SetColorconvMatrix(getRenderItemManager(), &colormat, E_TARGET_VIDEO_RIGHT);
                NXT_Theme_SetRealXYForRenderItem(getRenderItemManager(), E_TARGET_VIDEO_RIGHT, texture);

                NexThemeRenderer_PrepStateForRender2(this, &textureNode1, NULL, &texture, &mask, &texmat, &colormat, 0);

                NXT_Theme_SetTextureInfoTargetRenderItem(getRenderItemManager(),
                                                            texture->texName_for_rgb,
                                                            texture->textureWidth,
                                                            texture->textureHeight,
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_SRC);

                NXT_Theme_SetTextureInfoTargetRenderItem(getRenderItemManager(),
                                                            texture->texName_for_rgb,
                                                            texture->textureWidth,
                                                            texture->textureHeight,
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_LEFT);

                NXT_Theme_SetTexMatrix(getRenderItemManager(), &texmat, E_TARGET_VIDEO_SRC);
                NXT_Theme_SetTexMatrix(getRenderItemManager(), &texmat, E_TARGET_VIDEO_LEFT);
                NXT_Theme_SetColorconvMatrix(getRenderItemManager(), &colormat, E_TARGET_VIDEO_SRC);
                NXT_Theme_SetRealXYForRenderItem(getRenderItemManager(), E_TARGET_VIDEO_SRC, texture);
                NXT_Theme_SetColorconvMatrix(getRenderItemManager(), &colormat, E_TARGET_VIDEO_LEFT);
                NXT_Theme_SetRealXYForRenderItem(getRenderItemManager(), E_TARGET_VIDEO_LEFT, texture);

                NXT_Theme_DoEffect(
                    getRenderItemManager(),
                    cts,
                    cur_time,
                    peffect->clipStartTime,
                    peffect->clipEndTime,
                    max_set_time,
                    actualEffectStartCTS,
                    actualEffectEndCTS,
                    peffect->clip_index,
                    peffect->clip_count);

                NXT_Theme_ApplyRenderItem(getRenderItemManager(), time);
                NXT_Theme_EndRenderItem(getRenderItemManager());
            }
        }
        if( themeset!=NULL ) {
            NXT_ThemeSet_Release(this->getContextManager(), themeset, 0);
        }
    }
}

int NXT_ThemeRenderer_::getCurrentTrackID(){

    return current_track_id_;
}

int NXT_ThemeRenderer_::addCurrentTrackID(){

    return current_track_id_++;
}

void NXT_ThemeRenderer_::resetCurrentTrackID(){

    current_track_id_ = -1;
}

void NXT_ThemeRenderer_::resetCurrentTrackID(int track_id){

    current_track_id_ = track_id;
}

int NXT_ThemeRenderer_::setRenderToMask(){

    if (pmain_mask_target_){
        setRenderTarget(pmain_mask_target_);
        if(mask_init_flag_ == 0){

            setGLStateForRGBAOutput();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            mask_init_flag_ = 1;
        }
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        return 0;
    }
    return -1;
}

int NXT_ThemeRenderer_SetRenderToMask(NXT_HThemeRenderer renderer){

	return renderer->setRenderToMask();
}

int NXT_ThemeRenderer_::setRenderToDefault(){

    setRenderTarget(pmain_render_target_);
    return 0;
}

int NXT_ThemeRenderer_SetRenderToDefault(NXT_HThemeRenderer renderer){

	return renderer->setRenderToDefault();
}

int NXT_ThemeRenderer_::setTextureNameForLUT(unsigned int uid, float x, float y){

    lut_texture_ = uid;
    lut_strength_ = 1.0f;// - fabsf((float)renderer->view_width * 0.5f - x) / ((float)renderer->view_width * 0.5f);
    return -1;
}

int NXT_ThemeRenderer_SetTextureNameForLUT(NXT_HThemeRenderer renderer, unsigned int uid, float x, float y){

    return renderer->setTextureNameForLUT(uid, x, y);
}

int init_fastpreviewbuffer(NXT_HThemeRenderer renderer, NXT_RenderFastPreviewBuffer* prender_fastpreview, int w, int h){

    switch(renderer->fastpreview_renderOutputType )
    {
        case NXT_RendererOutputType_RGBA_8888:
            prender_fastpreview->width_ = w;
            prender_fastpreview->height_ = h;
            break;
        case NXT_RendererOutputType_Y2CrA_8888:
            prender_fastpreview->width_ = w;
            prender_fastpreview->height_ = h / 2;
            break;
        default:
            LOGI("[NexThemeRenderer.cpp %d] init_fastpreviewbuffer failed with not support output type(%d)", __LINE__, renderer->fastpreview_renderOutputType);
            return 0;
    }

    glGenFramebuffers(1, &prender_fastpreview->fbo_);
    glGenRenderbuffers(1, &prender_fastpreview->depth_buffer_);
    GL_GenTextures(1, &prender_fastpreview->target_texture_);
    
    glBindTexture(GL_TEXTURE_2D, prender_fastpreview->target_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, prender_fastpreview->width_, prender_fastpreview->height_, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindRenderbuffer(GL_RENDERBUFFER, prender_fastpreview->depth_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, prender_fastpreview->width_, prender_fastpreview->height_);
    
    glBindFramebuffer(GL_FRAMEBUFFER, prender_fastpreview->fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prender_fastpreview->target_texture_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, prender_fastpreview->depth_buffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, prender_fastpreview->depth_buffer_);
    
    if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
        return 0;

	prender_fastpreview->next = NULL;
	prender_fastpreview->prev = NULL;

	prender_fastpreview->inited_ = 1;
	prender_fastpreview->time_ = -1;
    LOGI("[NexThemeRenderer.cpp %d] prender_fastpreview(prender_target->width_ = (%d) prender_target->height_=(%d))", __LINE__, prender_fastpreview->width_, prender_fastpreview->height_);
    return 1;
}

void deinit_fastpreviewbuffer(NXT_RenderFastPreviewBuffer* prender_fastpreview, unsigned int isDetachedConext){
    if (prender_fastpreview->inited_){
        if (0 == isDetachedConext){
            if (prender_fastpreview->depth_buffer_ > 0) glDeleteRenderbuffers(1, &prender_fastpreview->depth_buffer_);
            if (prender_fastpreview->fbo_ > 0) glDeleteFramebuffers(1, &prender_fastpreview->fbo_);
            if (prender_fastpreview->target_texture_ > 0) GL_DeleteTextures(1, &prender_fastpreview->target_texture_);
        }
        prender_fastpreview->target_texture_ = 0;
        prender_fastpreview->depth_buffer_ = 0;
        prender_fastpreview->fbo_ = 0;
        prender_fastpreview->width_ = 0;
        prender_fastpreview->height_ = 0;
        prender_fastpreview->time_ = 0;
        prender_fastpreview->inited_ = 0;
    }
}

void settarget_fastpreviewbuffer(NXT_RenderFastPreviewBuffer* prender_fastpreview){
    if (NULL == prender_fastpreview){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, prender_fastpreview->fbo_);
    glViewport(0, 0, prender_fastpreview->width_, prender_fastpreview->height_);
}

int check_fastpreviewbuffer_init(NXT_RenderFastPreviewBuffer* prender_fastpreview){
    return prender_fastpreview->inited_;
}

void checkFastPreviewBuffer(NXT_RenderFastPreviewBuffer* root) {
	int i = 0;
	while( root != NULL ) {
		LOGE("[NexThemeRenderer.cpp %d] checkFastPreviewBuffer(%d %d %d)", __LINE__, i, root->time_, root->inited_);
		root = root->next;
		i++;
	}
}

NXT_RenderFastPreviewBuffer* getFastPreviewBuffer(NXT_HThemeRenderer renderer, int iTime) {

	NXT_RenderFastPreviewBuffer* first = renderer->pFastPreviewBufferFirst;
	NXT_RenderFastPreviewBuffer* last = renderer->pFastPreviewBufferLast;

	if( first != NULL && first->time_ == -1 ) {
		return first;
	}

	if( last != NULL && last->time_ == -1 ) {
		renderer->pFastPreviewBufferLast = last->prev;
		last->prev->next = NULL;
		last->next = NULL;
		last->prev = NULL;
	}
	else if( last != NULL && last->time_ < iTime ) {
		renderer->pFastPreviewBufferFirst = first->next;

		last = first;
		last->next->prev = NULL;
		last->next = NULL;
		last->prev = NULL;
	}
	else if( last != NULL && last->time_ > iTime ) {
		renderer->pFastPreviewBufferLast = last->prev;
		renderer->pFastPreviewBufferLast->next = NULL;

		last->next = NULL;
		last->prev = NULL;
	}
	else {
		LOGE("[NexThemeRenderer.cpp %d] getFastPreviewBuffer failed(%d)", __LINE__, iTime);
		checkFastPreviewBuffer(renderer->pFastPreviewBufferFirst);
		return  NULL;
	}

	first = renderer->pFastPreviewBufferFirst;
	while( first != NULL ) {
		if( first->time_ == -1 || first->time_ > iTime ) {
			last->prev = first->prev;
			last->next = first;

			if( last->prev == NULL ) {
				renderer->pFastPreviewBufferFirst = last;
			}
			else {
				last->prev->next = last;
			}

			first->prev = last;

			return last;
		}
		first = first->next;
	}

	renderer->pFastPreviewBufferLast->next = last;
	last->prev = renderer->pFastPreviewBufferLast;
	renderer->pFastPreviewBufferLast = last;
	return last;
}

void setrendertarget_For_AddFastPreviewFrame(NXT_HThemeRenderer renderer, int iTime){

#if 0 // fast preview
    renderer->pFastPreviewBuffer[renderer->fastpreview_total].time_ = iTime;
    renderer->view_width = renderer->pFastPreviewBuffer[renderer->fastpreview_total].width_;
    renderer->view_height = renderer->pFastPreviewBuffer[renderer->fastpreview_total].height_;

    settarget_fastpreviewbuffer(&renderer->pFastPreviewBuffer[renderer->fastpreview_total]);
#endif

	// checkFastPreviewBuffer(renderer->pFastPreviewBufferFirst);
	NXT_RenderFastPreviewBuffer* pBuffer = getFastPreviewBuffer(renderer, iTime);
	if( pBuffer == NULL ) return;

	pBuffer->time_ = iTime;
	renderer->view_width = pBuffer->width_;
	renderer->view_height = pBuffer->height_;

	settarget_fastpreviewbuffer(pBuffer);

	checkFastPreviewBuffer(renderer->pFastPreviewBufferFirst);

    LOGE("[NexThemeRenderer.cpp %d] setrendertarget_For_AddFastPreviewFrame(%d)", __LINE__, renderer->fastpreview_total);
}

void setrendertarget_For_FastPreview(NXT_HThemeRenderer renderer, int iTime){
#if 0 // fast preview
    int i = 0;
    for(i = 0; i < renderer->fastpreview_total; i++)
    {
        if( renderer->pFastPreviewBuffer[i].time_ < iTime )
            continue;
        renderer->fastpreview_index = i;
        return;
    }
    renderer->fastpreview_index = -1;
#endif

	renderer->pFastPreviewBufferCur = NULL;
	NXT_RenderFastPreviewBuffer* node = renderer->pFastPreviewBufferFirst;
	while(node != NULL ) {
		if( node->time_ < iTime )
		{
			node = node->next;
			continue;
		}
		break;
	}

	renderer->pFastPreviewBufferCur = node;
}

NXT_Error convertColorspace_For_FastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
    if( renderer == NULL )
        return NXT_Error_NotInit;

    if( renderer->pFastPreviewBuffer == NULL )
        return NXT_Error_NotInit;

    renderer->outputType = outputType;
    settarget_fastpreviewbuffer(NULL);
    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();

#if 0 // fast preview
    LOGI("[NexThemeRenderer.cpp %d] convertColorspace_For_FastPreview(%d)", __LINE__, renderer->fastpreview_index);

    GLuint tex_id = renderer->pFastPreviewBuffer[renderer->fastpreview_index].target_texture_;
#else
	LOGI("[NexThemeRenderer.cpp %d] convertColorspace_For_FastPreview(0x%p)", __LINE__, renderer->pFastPreviewBufferCur);

	GLuint tex_id = renderer->pFastPreviewBufferCur->target_texture_;
#endif

    switch(renderer->fastpreview_renderOutputType)
    {
        case NXT_RendererOutputType_RGBA_8888:
            setViewportForRGB(renderer);
            renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassthroughRGB, tex_id, 0);
            break;
        case NXT_RendererOutputType_Y2CrA_8888:
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            setViewportForRGB(renderer);
            renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderY2UVA, tex_id, 0);
            break;
        default:
            break;
    }

    return NXT_Error_None;
}

static void resetMainRenderTarget(NXT_HThemeRenderer renderer){

    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_render_target_);
    renderer->pmain_render_target_ = NULL;
}

NXT_Error NXT_ThemeRenderer_Add_FastPreviewFrame(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int iTime)
{
    unsigned int renderer_view_width = 0;
    unsigned int renderer_view_height = 0;

    if( renderer->fastpreview_total >= renderer->fastpreview_maxCount )
    {
        return NXT_Error_None;
    }

    NXT_Error err = beginRenderer(renderer, renderer->fastpreview_renderOutputType);
    if (NXT_Error_None != err)
        return err;

    renderer_view_width = renderer->view_width;
    renderer_view_height = renderer->view_height;

    setrendertarget_For_AddFastPreviewFrame(renderer, iTime);
    renderer->setGLRenderStart();

    //For simplifying the solution, I will render scene with only RGBA output.
    //Color space conversion will be done with result RGBA Render Target Texture.

    switch(renderer->fastpreview_renderOutputType)
    {
        case NXT_RendererOutputType_RGBA_8888:
            NXT_BEGIN_PROFILE("Render for output (RGBA_8888)");
            setViewportForRGB(renderer);
            renderer->setGLStateForRGBAOutput();
            render_internal(renderer);
            NXT_END_PROFILE();
            break;
        case NXT_RendererOutputType_Y2CrA_8888:
            LOGI("[%s %d] GLRenderAndGetPixels/Y2CrA8888 (%d x %d)", __func__, __LINE__, renderer->view_width, renderer->view_height);

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Prep frame render");
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClearColor(0.0,0.0,0.5,1.0);
            CHECK_GL_ERROR();
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
            CHECK_GL_ERROR();
            glEnable(GL_SCISSOR_TEST);
            NXT_END_PROFILE();

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 1");
            // glScissor(0,0,renderer->view_width,renderer->view_height);
            glViewport(0,0,renderer->view_width,renderer->view_height*2);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
            renderer->colorMask[0] = GL_TRUE;
            renderer->colorMask[1] = GL_FALSE;
            renderer->colorMask[2] = GL_FALSE;
            renderer->colorMask[3] = GL_FALSE;
            renderer->outputType = NXT_RendererOutputType_YUVA_8888;
            renderer->channelFilter = NXT_ChannelFilter_Y;
            render_internal(renderer);
            NXT_END_PROFILE();

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 2");
            // glScissor(0,0,renderer->view_width,renderer->view_height);
            glViewport(0,-renderer->view_height,renderer->view_width,renderer->view_height*2);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //            glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE);
            renderer->colorMask[0] = GL_FALSE;
            renderer->colorMask[1] = GL_TRUE;
            renderer->colorMask[2] = GL_FALSE;
            renderer->colorMask[3] = GL_FALSE;
            renderer->outputType = NXT_RendererOutputType_YUVA_8888;
            renderer->channelFilter = NXT_ChannelFilter_Y;
            render_internal(renderer);
            NXT_END_PROFILE();

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : U Pass");
            glScissor(0,0,renderer->view_width/2,renderer->view_height);
            glViewport(0,0,renderer->view_width/2,renderer->view_height);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //            glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_FALSE);
            renderer->colorMask[0] = GL_FALSE;
            renderer->colorMask[1] = GL_FALSE;
            renderer->colorMask[2] = GL_TRUE;
            renderer->colorMask[3] = GL_FALSE;
            renderer->outputType = NXT_RendererOutputType_YUVA_8888;
            renderer->channelFilter = NXT_ChannelFilter_U;
            render_internal(renderer);
            NXT_END_PROFILE();

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : V Pass");
            glScissor(0+(renderer->view_width/2),0,renderer->view_width/2,renderer->view_height);
            glViewport(0+(renderer->view_width/2),0,renderer->view_width/2,renderer->view_height);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            //            glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_FALSE);
            renderer->colorMask[0] = GL_FALSE;
            renderer->colorMask[1] = GL_FALSE;
            renderer->colorMask[2] = GL_TRUE;
            renderer->colorMask[3] = GL_FALSE;
            renderer->outputType = NXT_RendererOutputType_YUVA_8888;
            renderer->channelFilter = NXT_ChannelFilter_V;
            render_internal(renderer);
            NXT_END_PROFILE();

            //            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            renderer->colorMask[0] = GL_TRUE;
            renderer->colorMask[1] = GL_TRUE;
            renderer->colorMask[2] = GL_TRUE;
            renderer->colorMask[3] = GL_TRUE;
            renderer->channelFilter = NXT_ChannelFilter_ALL;
            renderer->outputType = NXT_RendererOutputType_Y2CrA_8888;
            glViewport(0,0,renderer->view_width,renderer->view_height);
            glScissor(0,0,renderer->view_width,renderer->view_height);
            glDisable(GL_SCISSOR_TEST);

            break;
        default:
            break;
    }

    renderer->view_width = renderer_view_width;
    renderer->view_height = renderer_view_height;

    // renderer->fastpreview_index = renderer->fastpreview_total;
    // convertColorspace(renderer, outputType);
    freeReadpixels(renderer, outputType);
    resetMainRenderTarget(renderer);
#if 0 // fast preview
    renderer->fastpreview_total++;
#endif
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_GLDraw_Main_For_FastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int iTime){
    NXT_Error err = beginRenderer(renderer, outputType);
    if (NXT_Error_None != err)
        return err;

    setrendertarget_For_FastPreview(renderer, iTime);

#if 0 // fast preview
    if( renderer->fastpreview_index < 0 )
    {
        LOGI("[NexThemeRenderer.cpp %d] setrendertarget_For_FastPreview failed(%d)", __LINE__, renderer->fastpreview_index);
        return NXT_Error_Internal;
    }
#else
	if( renderer->pFastPreviewBufferCur == NULL )
	{
		LOGI("[NexThemeRenderer.cpp %d] setrendertarget_For_FastPreview failed(%d)", __LINE__, renderer->fastpreview_index);
		return NXT_Error_Internal;
	}
#endif

    renderer->setGLRenderStart();

    convertColorspace_For_FastPreview(renderer, outputType);

    freeReadpixels(renderer, outputType);
    resetMainRenderTarget(renderer);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_InitFastPreview(NXT_HThemeRenderer renderer, int w, int h, int maxCount, NXT_RendererOutputType outputType)
{
    int i = 0;
    if( renderer == NULL )
        return NXT_Error_NotInit;

    NXT_ThemeRenderer_DeinitFastPreview(renderer);

    renderer->pFastPreviewBuffer = (NXT_RenderFastPreviewBuffer*)malloc(sizeof(NXT_RenderFastPreviewBuffer)*maxCount);
    if( renderer->pFastPreviewBuffer == NULL )
    {
        return NXT_Error_Malloc;
    }

    renderer->fastpreview_maxCount = maxCount;
    renderer->fastpreview_renderOutputType = outputType;

    NXT_RenderFastPreviewBuffer* pFastpreviewbuffer = renderer->pFastPreviewBuffer;

    for(i = 0; i < renderer->fastpreview_maxCount; i++ )
    {
        init_fastpreviewbuffer(renderer, pFastpreviewbuffer + i, w, h);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

	NXT_RenderFastPreviewBuffer* pFastpreviewbufferLink = renderer->pFastPreviewBufferFirst;
	for(i = 0; i < renderer->fastpreview_maxCount; i++ )
	{
		if( pFastpreviewbufferLink == NULL )
		{
			pFastpreviewbufferLink = pFastpreviewbuffer+i;
			renderer->pFastPreviewBufferFirst = pFastpreviewbufferLink;
			renderer->pFastPreviewBufferLast = pFastpreviewbufferLink;
			pFastpreviewbufferLink->prev = NULL;
			pFastpreviewbufferLink->next = NULL;
			continue;
		}

		pFastpreviewbufferLink->next = pFastpreviewbuffer+i;
		pFastpreviewbufferLink->next->prev = pFastpreviewbufferLink;

		pFastpreviewbufferLink = pFastpreviewbuffer+i;
		renderer->pFastPreviewBufferLast = pFastpreviewbufferLink;
	}

	checkFastPreviewBuffer(renderer->pFastPreviewBufferFirst);

    renderer->fastpreview_index = 0;
    renderer->fastpreview_total = 0;
    renderer->fastpreview_index = 0;
    renderer->fastpreview_width = w;
    renderer->fastpreview_height = h;
    renderer->fastpreview_on = 1;

    LOGI("[NexThemeRenderer.cpp %d] NXT_ThemeRenderer_InitFastPreview", __LINE__);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_DeinitFastPreview(NXT_HThemeRenderer renderer)
{
    if( renderer == NULL )
        return NXT_Error_NotInit;

    if( renderer->pFastPreviewBuffer != NULL ) {
        NXT_RenderFastPreviewBuffer* pfastpreviewbuffer = renderer->pFastPreviewBuffer;
        int i = 0;
        for(i = 0; i < renderer->fastpreview_maxCount; i++ )
        {
            if (check_fastpreviewbuffer_init(&pfastpreviewbuffer[i])){
                deinit_fastpreviewbuffer(&pfastpreviewbuffer[i], 0);
            }
        }

        free(pfastpreviewbuffer);
        renderer->pFastPreviewBuffer = NULL;
		renderer->pFastPreviewBufferFirst = NULL;
		renderer->pFastPreviewBufferLast = NULL;
    }

    renderer->fastpreview_maxCount = 0;
    renderer->fastpreview_index = 0;
    renderer->fastpreview_total = 0;
    renderer->fastpreview_index = 0;
    renderer->fastpreview_width = 0;
    renderer->fastpreview_height = 0;
    renderer->fastpreview_on = 0;

    renderer->fastpreview_renderOutputType = NXT_RendererOutputType_RGBA_8888;

    LOGI("[NexThemeRenderer.cpp %d] NXT_ThemeRenderer_DeinitFastPreview", __LINE__);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_GetFastPreviewBufferTime(NXT_HThemeRenderer renderer, unsigned int* pStart, unsigned int* pEnd)
{
	if( renderer == NULL || pStart == NULL || pEnd == NULL )
		return NXT_Error_NotInit;

	NXT_RenderFastPreviewBuffer* first = renderer->pFastPreviewBufferFirst;
	NXT_RenderFastPreviewBuffer* last = renderer->pFastPreviewBufferLast;

	while( last != NULL && first != last && last->time_ == -1 ) {
		last = last->prev;
	}

	*pStart = first->time_;
	*pEnd = last->time_;
	return NXT_Error_None;
}


//static void dumpSurfaceInfo( EGLDisplay dpy, EGLSurface surface, const char* tag );

static int g_loglevel = 4;

//void NXT_ThemeRenderer_PrecacheImages(NXT_HThemeRenderer renderer, 
//                                      NXT_HTheme hTheme,    // Any theme in the set (doesn't matter which one)
//                                      NXT_PrecacheMode precacheMode);


#define LAYERRENDER_FLAG_EXPORT 0x00000001

void NXT_ThemeRenderer_SetLogLevel( int level ) {
    // if( level < -1 )
    //     g_loglevel = -1;
    // else if( level > 7 )
    //     g_loglevel = 7;
    // else
    //     g_loglevel = level;
}


void NXT_ThemeRenderer_SetTextureTransform(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, float* startMatrix, unsigned int startTime, float* endMatrix, unsigned int endTime)
{
}

unsigned int __check_nexthemerenderer_loglevel( int level ) {
    return ( level <= g_loglevel || level < 0 );
}

#ifdef EGL_ERROR_CHECKING

static const char* getEGLErrorString( EGLint error ) {
    switch (error) {
        case EGL_SUCCESS: return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
        case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
        default: return "?";
    }
}

void NexThemeRenderer_CheckEglError(int line, const char *func) {
    EGLint error;
    for (error = eglGetError(); error!=EGL_SUCCESS; error = eglGetError()) {
        LOGE("[%s %i] eglError (0x%x : %s)\n", func, line, error, getEGLErrorString(error));
    }
}

#endif //EGL_ERROR_CHECKING

#ifdef GL_ERROR_CHECKING

static const char* getGLErrorString( GLint error ) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "?";
    }
}

void NexThemeRenderer_CheckGlError(int line, const char *func) {
    GLint error;
    for (error = glGetError(); error; error = glGetError()) {
        LOGE("[%s %i] glError (0x%x : %s)\n", func, line, error, getGLErrorString(error));
    }
}

void NexThemeRenderer_PrintGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGD("GL %s = %s\n", name, v);
}

#endif // GL_ERROR_CHECKING


NXT_Error NXT_ThemeRenderer_ClearScreen(NXT_HThemeRenderer renderer, int iTag) {
    
    LOGI("[%s %d] Clear screen (%d)", __func__, __LINE__, iTag);
    NXT_Error result = NXT_ThemeRenderer_AquireContext(renderer);
    if( result != NXT_Error_None ) {
        return result;
    }
    
    unsigned int bSwapBuffers = 0;
    
    if( iTag==1 ) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        CHECK_GL_ERROR();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        CHECK_GL_ERROR();
        glViewport(0,0,renderer->view_width,renderer->view_height);
        CHECK_GL_ERROR();
        glScissor(0,0,renderer->view_width,renderer->view_height);
        CHECK_GL_ERROR();
        glDisable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR();
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        CHECK_GL_ERROR();
        bSwapBuffers = 1;
    } else if( iTag==2 ) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        CHECK_GL_ERROR();
        glClearColor(0.0, 0.0, 0.0, 0.0);
        CHECK_GL_ERROR();
        glViewport(0,0,renderer->view_width,renderer->view_height);
        CHECK_GL_ERROR();
        glScissor(0,0,renderer->view_width,renderer->view_height);
        CHECK_GL_ERROR();
        glDisable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR();
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        CHECK_GL_ERROR();
        bSwapBuffers = 1;
    }

    
    NXT_ThemeRenderer_ReleaseContext(renderer, bSwapBuffers);
    return NXT_Error_None;
}


static unsigned int getTickCount()
{
	static unsigned int uPreTick = 0;
	unsigned int uRet;
	struct timeval tm;
    
	gettimeofday( &tm, NULL );
	uRet = (unsigned int)( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
	if ( uPreTick > uRet && (uPreTick-uRet) < 100000 )
		return uPreTick;
    
	uPreTick = uRet;
	return uRet;
}

static unsigned int nextHighestPowerOfTwo( unsigned int v ) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}


#ifdef EGL_SUPPORT

static struct {
    const char *name;
    EGLint attr;
} *p_dumpattr, dumpattr[] = {
    {"EGL_CONFIG_ID",               EGL_CONFIG_ID},
    {"EGL_RED_SIZE",                EGL_RED_SIZE},
    {"EGL_GREEN_SIZE",              EGL_GREEN_SIZE},
    {"EGL_BLUE_SIZE",               EGL_BLUE_SIZE},
    {"EGL_ALPHA_SIZE",              EGL_ALPHA_SIZE},
    {"EGL_ALPHA_MASK_SIZE",         EGL_ALPHA_MASK_SIZE},
    {"EGL_BIND_TO_TEXTURE_RGB",     EGL_BIND_TO_TEXTURE_RGB},
    {"EGL_BIND_TO_TEXTURE_RGBA",    EGL_BIND_TO_TEXTURE_RGBA},
    {"EGL_BUFFER_SIZE",             EGL_BUFFER_SIZE},
    {"EGL_COLOR_BUFFER_TYPE",       EGL_COLOR_BUFFER_TYPE},
    {"EGL_CONFIG_CAVEAT",           EGL_CONFIG_CAVEAT},
    {"EGL_CONFORMANT",              EGL_CONFORMANT},
    {"EGL_DEPTH_SIZE",              EGL_DEPTH_SIZE},
    {"EGL_LEVEL",                   EGL_LEVEL},
    {"EGL_LUMINANCE_SIZE",          EGL_LUMINANCE_SIZE},
    {"EGL_MAX_PBUFFER_WIDTH",       EGL_MAX_PBUFFER_WIDTH},
    {"EGL_MAX_PBUFFER_HEIGHT",      EGL_MAX_PBUFFER_HEIGHT},
    {"EGL_MAX_PBUFFER_PIXELS",      EGL_MAX_PBUFFER_PIXELS},
    {"EGL_MAX_SWAP_INTERVAL",       EGL_MAX_SWAP_INTERVAL},
    {"EGL_MIN_SWAP_INTERVAL",       EGL_MIN_SWAP_INTERVAL},
    {"EGL_NATIVE_RENDERABLE",       EGL_NATIVE_RENDERABLE},
    {"EGL_NATIVE_VISUAL_ID",        EGL_NATIVE_VISUAL_ID},
    {"EGL_NATIVE_VISUAL_TYPE",      EGL_NATIVE_VISUAL_TYPE},
    {"EGL_RENDERABLE_TYPE",         EGL_RENDERABLE_TYPE},
    {"EGL_SAMPLE_BUFFERS",          EGL_SAMPLE_BUFFERS},
    {"EGL_SAMPLES",                 EGL_SAMPLES},
    {"EGL_STENCIL_SIZE",            EGL_STENCIL_SIZE},
    {"EGL_SURFACE_TYPE",            EGL_SURFACE_TYPE},
    {"EGL_TRANSPARENT_TYPE",        EGL_TRANSPARENT_TYPE},
    {"EGL_TRANSPARENT_RED_VALUE",   EGL_TRANSPARENT_RED_VALUE},
    {"EGL_TRANSPARENT_GREEN_VALUE", EGL_TRANSPARENT_GREEN_VALUE},
    {"EGL_TRANSPARENT_BLUE_VALUE",  EGL_TRANSPARENT_BLUE_VALUE},
    {NULL,0}
};

#endif

#ifdef ANDROID

NXT_Error NXT_ThemeRenderer_SetNativeWindow( NXT_HThemeRenderer renderer, ANativeWindow *pNativeWindow, int reqWidth, int reqHeight ) {
    
    int nativeWindowWidth = 0;
    int nativeWindowHeight = 0;
    int nativeWindowFormat = 0;
	int bReqSize = 0;
	
	if( reqWidth>0 && reqHeight>0 )
		bReqSize = 1;

    if( !renderer )
        return NXT_Error_BadParam;
    
    if( renderer->rendererType == NXT_RendererType_ExternalContext ) {
        LOGE("[%s %d] Attempt to set native window on incompatible renderer type.  required(%d) actual(%d)", __func__, __LINE__, NXT_RendererType_InternalPreviewContext, renderer->rendererType );
        return NXT_Error_WrongRendererType;
    }
    
    if( pNativeWindow && renderer->nativeWindow == pNativeWindow ) {

        nativeWindowWidth = ANativeWindow_getWidth(pNativeWindow);
        nativeWindowHeight = ANativeWindow_getHeight(pNativeWindow);
        nativeWindowFormat = ANativeWindow_getFormat(pNativeWindow);
		
        if( nativeWindowWidth == renderer->nativeWindowWidth && nativeWindowHeight == renderer->nativeWindowHeight && (!bReqSize || (nativeWindowWidth==reqWidth && nativeWindowHeight==reqHeight) ) ) {
            LOGI("[%s %d] Same native window AND width and height match; nothing to do pNativeWindow(0x%X) iWidth(%d) iHeight(%d)", __func__, __LINE__, LOGPTR(pNativeWindow), nativeWindowWidth, nativeWindowHeight);
            NXT_ThemeRenderer_ClearScreen(renderer, 1);
            return NXT_Error_None;
        }
    }
        
    if( renderer->nativeWindow ) {
        if( renderer->nativeWindowSurface != EGL_NO_SURFACE ) {
            LOGI("[%s %d] Destroying native window surface(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeWindowSurface));
            eglDestroySurface(renderer->getContextManager()->eglDisplay,renderer->nativeWindowSurface);
            CHECK_EGL_ERROR();
            renderer->nativeWindowSurface = EGL_NO_SURFACE;
        }
        if( renderer->nativeWindow != pNativeWindow) {
            LOGI("[%s %d] Releasing native window(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeWindow));
            ANativeWindow_release(renderer->nativeWindow);
            renderer->nativeWindow = NULL;
        }
    }
    
    if( pNativeWindow ) {

        if( renderer->nativeWindow != pNativeWindow ) {
            LOGI("[%s %d] Acquiring native window(0x%X)", __func__, __LINE__, LOGPTR(pNativeWindow));
            ANativeWindow_acquire(pNativeWindow);
            renderer->nativeWindow = pNativeWindow;
        }
        
        EGLint attr_list[] = {
            EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
            EGL_NONE
        };
        
        nativeWindowWidth = ANativeWindow_getWidth(pNativeWindow);
        nativeWindowHeight = ANativeWindow_getHeight(pNativeWindow);
        nativeWindowFormat = ANativeWindow_getFormat(pNativeWindow);
/*
        WINDOW_FORMAT_RGBA_8888          = 1,
        WINDOW_FORMAT_RGBX_8888          = 2,
        WINDOW_FORMAT_RGB_565            = 4,
*/
        LOGN("[%s %d] native window (before setBuffersGeometry) : %d x %d (format=%d) req %d x %d", __func__, __LINE__, nativeWindowWidth, nativeWindowHeight, nativeWindowFormat, reqWidth, reqHeight);
        
	EGLint format;
	eglGetConfigAttrib(renderer->getContextManager()->eglDisplay, renderer->getContextManager()->eglConfig, EGL_NATIVE_VISUAL_ID, &format);

	LOGN("[%s %d] native window (before setBuffersGeometry) : %d x %d (format=%d) req %d x %d format(%d)", __func__, __LINE__, nativeWindowWidth, nativeWindowHeight, nativeWindowFormat, reqWidth, reqHeight, format);
#if 0	
        int32_t sbgresult;
		if( bReqSize ) {
		sbgresult = ANativeWindow_setBuffersGeometry(pNativeWindow, reqWidth, reqHeight, format);
		} else {
		sbgresult = ANativeWindow_setBuffersGeometry(pNativeWindow, nativeWindowWidth, nativeWindowHeight, format);
		}
#else
        int32_t sbgresult;
	if( bReqSize ) {
		sbgresult = ANativeWindow_setBuffersGeometry(pNativeWindow, reqWidth, reqHeight, WINDOW_FORMAT_RGBX_8888);
	} else {
		sbgresult = ANativeWindow_setBuffersGeometry(pNativeWindow, nativeWindowWidth, nativeWindowHeight, WINDOW_FORMAT_RGBX_8888);
	}
#endif	

        // int32_t sbgresult = ANativeWindow_setBuffersGeometry(pNativeWindow, 1280, 720, WINDOW_FORMAT_RGBX_8888);
        // int32_t sbgresult = 1;
        
        int iWidthB = ANativeWindow_getWidth(pNativeWindow);
        int iHeightB = ANativeWindow_getHeight(pNativeWindow);
        int iFormatB = ANativeWindow_getFormat(pNativeWindow);
        
        LOGN("[%s %d] native window (after setBuffersGeometry) : %d x %d (format=%d) result(0x%x)", __func__, __LINE__, iWidthB, iHeightB, iFormatB, sbgresult);
        
        renderer->nativeWindowSurface = eglCreateWindowSurface(renderer->getContextManager()->eglDisplay, renderer->getContextManager()->eglConfig, (EGLNativeWindowType)pNativeWindow, attr_list );
        CHECK_EGL_ERROR();
        if( renderer->nativeWindowSurface == EGL_NO_SURFACE ) {
            LOGE("[%s %d] eglCreateWindowSurface failed", __func__, __LINE__);
            ANativeWindow_release(renderer->nativeWindow);
            renderer->nativeWindow = NULL;
            return NXT_Error_GLSurfaceErr;
        }
        
		//ANativeWindow_release(renderer->nativeWindow);
		//ANativeWindow_acquire(renderer->nativeWindow);
		
        EGLint surfaceWidth = 0;
        EGLint surfaceHeight = 0;
        
        struct {
            EGLint attr;
            EGLint *val;
        } getattrs[] = {
            {EGL_HEIGHT,                &surfaceHeight},
            {EGL_WIDTH,                 &surfaceWidth},
            {0}
        };
        
        int i;
        EGLint surfaceParamValue;
        unsigned int bQueryOK;
        
		for( i=0; getattrs[i].val; i++ ) {
            surfaceParamValue = 0;
            bQueryOK = ( eglQuerySurface(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, getattrs[i].attr, &surfaceParamValue ) == EGL_TRUE );
            CHECK_EGL_ERROR();
            
            if( !bQueryOK )
            {
                LOGI("[%s %d] eglQuerySurface(%d) failed : Destroying native window surface(0x%X)", __func__, __LINE__, i, LOGPTR(renderer->nativeWindowSurface));
                eglDestroySurface(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface);
                renderer->nativeWindowSurface = EGL_NO_SURFACE;
                ANativeWindow_release(renderer->nativeWindow);
                renderer->nativeWindow = NULL;
                return NXT_Error_GLSurfaceErr;
            }
            
            *(getattrs[i].val) = surfaceParamValue;
        }
        CHECK_EGL_ERROR();
        
        LOGN("[%s %d] got surface(0x%x) for display(0x%x) size(%d x %d) geometry(%d)", __func__, __LINE__, LOGPTR(renderer->nativeWindowSurface), LOGPTR(renderer->getContextManager()->eglDisplay), surfaceWidth, surfaceHeight, sbgresult);
        
		surfaceWidth = surfaceWidth == 1 ? reqWidth : surfaceWidth;
		surfaceHeight = surfaceHeight == 1 ? reqHeight : surfaceHeight;
        
        renderer->nativeWindowWidth = surfaceWidth;
        renderer->nativeWindowHeight = surfaceHeight;
        
    }

	if( !renderer->bInitialized && renderer->rendererType==NXT_RendererType_InternalPreviewContext )
	{
        if( NXT_Error_None == NXT_ThemeRenderer_AquireContext(renderer) ) {
            NXT_Error result = initRenderer( renderer );
            if( result != NXT_Error_None && !renderer->bInitialized ) {
                LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                return result;
            }
            NXT_ThemeRenderer_ReleaseContext(renderer, 1);
        }
        else{
            LOGE("[%s %d] error NXT_ThemeRenderer_AquireContext", __func__, __LINE__);
        }
    }
    return NXT_Error_None;
}

/*static void dumpSurfaceInfo( EGLDisplay dpy, EGLSurface surface, const char* tag ) {
    
    struct {
        EGLint attr;
        char* name;
    } getattrs[] = {
        {EGL_HEIGHT,                "EGL_HEIGHT"                    },
        {EGL_WIDTH,                 "EGL_WIDTH"                     },
        {EGL_CONFIG_ID,             "EGL_CONFIG_ID"                 },
        {EGL_HORIZONTAL_RESOLUTION, "EGL_HORIZONTAL_RESOLUTION"     },
        {EGL_VERTICAL_RESOLUTION,   "EGL_VERTICAL_RESOLUTION"       },
        {EGL_LARGEST_PBUFFER,       "EGL_LARGEST_PBUFFER"           },
        {EGL_MIPMAP_TEXTURE,        "EGL_MIPMAP_TEXTURE"            },
        {EGL_MULTISAMPLE_RESOLVE,   "EGL_MULTISAMPLE_RESOLVE"       },
        {EGL_PIXEL_ASPECT_RATIO,    "EGL_PIXEL_ASPECT_RATIO"        },
        {EGL_RENDER_BUFFER,         "EGL_RENDER_BUFFER"             },
        {EGL_MIPMAP_LEVEL,          "EGL_MIPMAP_LEVEL"              },
        {EGL_SWAP_BEHAVIOR,         "EGL_SWAP_BEHAVIOR"             },
        {EGL_TEXTURE_FORMAT,        "EGL_TEXTURE_FORMAT"            },
        {EGL_TEXTURE_TARGET,        "EGL_TEXTURE_TARGET"            },
        {EGL_VG_COLORSPACE,         "EGL_VG_COLORSPACE"             },
        {EGL_VG_ALPHA_FORMAT,       "EGL_VG_ALPHA_FORMAT"           },
        {0}
    };

    int i;
    EGLint surfaceParamValue;
    unsigned int bQueryOK;
    const char* hint;
    
    LOGI( "[%s %d] %s surface(0x%08X) display(0x%08X):", __func__, __LINE__, tag, surface, dpy );
    
    for( i=0; getattrs[i].name; i++ ) {
        surfaceParamValue = 0;
        bQueryOK = ( eglQuerySurface( dpy, surface, getattrs[i].attr, &surfaceParamValue ) == EGL_TRUE );
        CHECK_EGL_ERROR();
        switch (surfaceParamValue) {
            case EGL_VG_COLORSPACE_sRGB:            hint="EGL_VG_COLORSPACE_sRGB"; break;
            case EGL_VG_COLORSPACE_LINEAR:          hint="EGL_VG_COLORSPACE_LINEAR"; break;
            case EGL_BACK_BUFFER:                   hint="EGL_BACK_BUFFER"; break;
            case EGL_SINGLE_BUFFER:                 hint="EGL_SINGLE_BUFFER"; break;
            case EGL_VG_ALPHA_FORMAT_NONPRE:        hint="EGL_VG_ALPHA_FORMAT_NONPRE"; break;
            case EGL_VG_ALPHA_FORMAT_PRE:           hint="EGL_VG_ALPHA_FORMAT_PRE"; break;
            case EGL_BUFFER_PRESERVED:              hint="EGL_BUFFER_PRESERVED"; break;
            case EGL_BUFFER_DESTROYED:              hint="EGL_BUFFER_DESTROYED"; break;
            case EGL_NO_TEXTURE:                    hint="EGL_NO_TEXTURE"; break;
            case EGL_TEXTURE_RGB:                   hint="EGL_TEXTURE_RGB"; break;
            case EGL_TEXTURE_RGBA:                  hint="EGL_TEXTURE_RGBA"; break;
            case EGL_TEXTURE_2D:                    hint="EGL_TEXTURE_2D"; break;
            case EGL_MULTISAMPLE_RESOLVE_DEFAULT:   hint="EGL_MULTISAMPLE_RESOLVE_DEFAULT"; break;
            case EGL_MULTISAMPLE_RESOLVE_BOX:       hint="EGL_MULTISAMPLE_RESOLVE_BOX"; break;
            default:                                hint=""; break;
        }
        if( bQueryOK ) {
            LOGI( "[%s %d]     %s surface->%s = %d %s", __func__, __LINE__, tag, getattrs[i].name, surfaceParamValue, hint );
        } else {
            LOGI( "[%s %d]     %s surface->%s = <ERROR>", __func__, __LINE__, tag, getattrs[i].name);
        }
    }
    
}*/

NXT_Error NXT_ThemeRenderer_BeginExport( NXT_HThemeRenderer renderer, int width, int height, ANativeWindow *pNativeWindow ) {
    
    if( !renderer )
        return NXT_Error_BadParam;

    if( renderer->bExportMode ) {
        LOGI("[%s %d] Request to begin export while already in export mode; ending export", __func__, __LINE__);
        NXT_ThemeRenderer_EndExport(renderer);
    }
    
    if( renderer->bExportMode ) {
        LOGE("[%s %d] Request to begin export while already in export mode (and unable to stop current export)", __func__, __LINE__);
        return NXT_Error_InvalidState;
    }

    LOGI("[%s %d] RENDERER BEGIN EXPORT", __func__, __LINE__);
    
    //createExportContext( renderer );
    
    /*EGLint attr_list[] = {
        EGL_HEIGHT, height,
        EGL_WIDTH, width,
        EGL_TEXTURE_TARGET, EGL_NO_TEXTURE, // EGL_NO_TEXTURE, EGL_TEXTURE_2D
        EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE, // EGL_NO_TEXTURE, EGL_TEXTURE_RGB, EGL_TEXTURE_RGBA
        EGL_NONE
    };
    renderer->nativeExportSurface = eglCreatePbufferSurface(renderer->getContextManager()->eglDisplay, renderer->getContextManager()->eglConfig, attr_list);
    CHECK_EGL_ERROR();
    if( renderer->nativeExportSurface == EGL_NO_SURFACE ) {
        LOGE("[%s %d] eglCreatePbufferSurface failed", __func__, __LINE__);
        return NXT_Error_GLSurfaceErr;
    }*/
    
    LOGI("[%s %d] eglCreateWindowSurface eglDisplay(0x%08X) eglConfig(0x%08X) pNativeWindow(0x%08X)", __func__, __LINE__, LOGPTR(renderer->getContextManager()->eglDisplay), LOGPTR(renderer->getContextManager()->eglConfig), LOGPTR(pNativeWindow));
    renderer->nativeExportSurface = eglCreateWindowSurface(renderer->getContextManager()->eglDisplay, renderer->getContextManager()->eglConfig, (EGLNativeWindowType)pNativeWindow, NULL );
    if( renderer->nativeExportSurface == EGL_NO_SURFACE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglCreateWindowSurface failed", __func__, __LINE__);
        if( renderer->nativeWindow != NULL )
        {
                ANativeWindow_release(renderer->nativeWindow);
                renderer->nativeWindow = NULL;
        }
        return NXT_Error_GLSurfaceErr;
    }
    
    EGLint surfaceWidth = 0;
    EGLint surfaceHeight = 0;
    
    if( eglQuerySurface(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface, EGL_WIDTH, &surfaceWidth ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglQuerySurface failed", __func__, __LINE__);
        eglDestroySurface(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface);
        return NXT_Error_GLSurfaceErr;
    }
    
    if( eglQuerySurface(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface, EGL_HEIGHT, &surfaceHeight ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglQuerySurface failed", __func__, __LINE__);
        eglDestroySurface(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface);
        return NXT_Error_GLInitFail;
    }
    
    CHECK_EGL_ERROR();
    LOGI("[%s %d] got surface size(%d x %d)", __func__, __LINE__, surfaceWidth, surfaceHeight);
    
#ifdef KEEP_CONTEXT_DURING_EXPORT
#ifdef ANDROID
    if( eglMakeCurrent(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface, renderer->nativeExportSurface, renderer->getContext()) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglMakeCurrent failed", __func__, __LINE__);
#elif defined(__APPLE__)
    if( NXT_Error_None != setCurrentInternalContext ( renderer, SET_CONTEXT_MODE_EXPORT ) ) {
        LOGE("[%s %d] setCurrentInternalContext failed", __func__, __LINE__);
#endif //ANDROID
        return NXT_Error_GLContextErr;
    }
#endif

    renderer->exportSurfaceWidth = surfaceWidth;
    renderer->exportSurfaceHeight = surfaceHeight;

    renderer->view_width = surfaceWidth;
    renderer->view_height = surfaceHeight;
	
    renderer->bExportMode = 1;
    return NXT_Error_None;	
}

NXT_Error NXT_ThemeRenderer_EndExport( NXT_HThemeRenderer renderer ) {
    
    if( !renderer )
        return NXT_Error_BadParam;
    
    if( !renderer->bExportMode ) {
        LOGI("[%s %d] Request to end export, but not in export mode", __func__, __LINE__);
        return NXT_Error_None;
    }

#ifdef KEEP_CONTEXT_DURING_EXPORT
#ifdef ANDROID
    eglMakeCurrent(renderer->getContextManager()->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    CHECK_EGL_ERROR();
#elif defined(__APPLE__)
    setCurrentInternalContext( renderer, SET_CONTEXT_MODE_RELEASE );
#endif
    

#endif //KEEP_CONTEXT_DURING_EXPORT

    LOGI("[%s %d] RENDERER END EXPORT", __func__, __LINE__);

    //destroyExportContext(renderer);

    LOGI("[%s %d] RENDERER END EXPORT", __func__, __LINE__);
    eglDestroySurface(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface);
    CHECK_EGL_ERROR();
    
    renderer->bExportMode = 0;
    renderer->nativeExportSurface = EGL_NO_SURFACE;

    return NXT_Error_None;
}


static NXT_Error ensureInitEGL(NXT_HThemeRenderer_Context context) {
    
    if(!context)
        return NXT_Error_Internal;
    
    if(context->eglDisplay == EGL_NO_DISPLAY) {
        
        // EGL : Get the display
        context->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        CHECK_EGL_ERROR();
        
        if(context->eglDisplay == EGL_NO_DISPLAY) {
            LOGE("[%s %d] eglGetDisplay failed", __func__, __LINE__);
            return NXT_Error_GLInitFail;
        }
        
        // EGL : Initialize for this display (and get version number)
        EGLint egl_major = 0;
        EGLint egl_minor = 0;
        if( eglInitialize(context->eglDisplay, &egl_major, &egl_minor) != EGL_TRUE ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglInitialize failed", __func__, __LINE__);
            return NXT_Error_GLInitFail;
        }
        CHECK_EGL_ERROR();
        
        LOGI("[%s %d] EGL %d.%d  display(0x%X)", __func__, __LINE__, egl_major, egl_minor, LOGPTR(context->eglDisplay));
        
    }

    return NXT_Error_None;
}

static NXT_Error createInternalContext(NXT_HThemeRenderer_Context context, unsigned int bIsMI5C) {

    NXT_Error result = NXT_Error_None;

    result = ensureInitEGL(context);
    if( result!=NXT_Error_None )
        return result;
	
    // EGL : Choose the configuration
    
    EGLint attrib_list_mi5c[] = {
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_RECORDABLE_ANDROID,     EGL_TRUE,
        //EGL_ALPHA_SIZE,             8,
        //EGL_STENCIL_SIZE,           8,
        //EGL_DEPTH_SIZE,             (EGLint)context->requestDepthBufferSize,
        EGL_NONE
    };

    EGLint attrib_list_preview[] = {
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_ALPHA_SIZE,             8,
        EGL_STENCIL_SIZE,           8,
        EGL_DEPTH_SIZE,             (EGLint)context->requestDepthBufferSize,
        EGL_NONE
    };
    
    EGLint numConfig = 0;
	EGLint* attrib_list = bIsMI5C?attrib_list_mi5c:attrib_list_preview;
    if(eglChooseConfig(context->eglDisplay, attrib_list, NULL, 0, &numConfig) != EGL_TRUE) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglChooseConfig failed (returned false getting count)", __func__, __LINE__);
        return NXT_Error_GLInitFail;
    }

	if(numConfig < 1) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned true but no configs)", __func__, __LINE__);
		return NXT_Error_GLInitFail;
	}
	
	EGLConfig configs[numConfig];
	EGLint actualNumConfig = 0;
	CHECK_EGL_ERROR();
	if( eglChooseConfig(context->eglDisplay, attrib_list, configs, numConfig, &actualNumConfig) != EGL_TRUE ) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned false getting list)", __func__, __LINE__);
		return NXT_Error_GLNoConfig;
	} else if(actualNumConfig < 1) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned true but no configs)", __func__, __LINE__);
		return NXT_Error_GLNoConfig;
	} else if(actualNumConfig > numConfig) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned true but actualNumConfig > numConfig)", __func__, __LINE__);
		return NXT_Error_InvalidState;
	} else {
		CHECK_EGL_ERROR();
		
		LOGD("[%s %d] eglChooseConfig returned %d configs", __func__, __LINE__, actualNumConfig);
		int i;
		int j;
		EGLint value;
		for( i=0; i<actualNumConfig; i++ ) {
			LOGD("[%s %d]   Config %d:", __func__, __LINE__, i);
			for( p_dumpattr = dumpattr; p_dumpattr->name; p_dumpattr++ ) {
				value = 0;
				if( eglGetConfigAttrib(context->eglDisplay, configs[i], p_dumpattr->attr, &value ) != EGL_TRUE ) {
					LOGD("[%s %d]       %s: ERR", __func__, __LINE__, p_dumpattr->name );
				} else {
					LOGD("[%s %d]       %s: %d", __func__, __LINE__, p_dumpattr->name, value);
				}
				CHECK_EGL_ERROR();
			}
		}
		
		int useConfig = 0;
		if( useConfig > actualNumConfig-1 ) {
			useConfig = actualNumConfig-1;
		}
		
		LOGD("[%s %d] using config(%d)", __func__, __LINE__, useConfig);
		
		context->eglConfig = configs[useConfig];
		
		EGLint context_attr_list[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		EGLContext ctx = eglCreateContext(context->eglDisplay, context->eglConfig, EGL_NO_CONTEXT, context_attr_list);
		CHECK_EGL_ERROR();
		if(ctx == EGL_NO_CONTEXT) {
			LOGE("[%s %d] eglCreateContext failed", __func__, __LINE__);
			return NXT_Error_GLInitFail;
		}

        context->setContext(ctx);	
		LOGD("[%s %d] eglCreateContext(0x%x)", __func__, __LINE__, LOGPTR(context->getContext()));
		
		return NXT_Error_None;
	}
}

NXT_Error NXT_ThemeRenderer_SetJavaVM( NXT_HThemeRenderer renderer, JavaVM * java_vm ) {
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !java_vm ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    renderer->java_vm = java_vm;
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_SetWrapperForSurfaceTextureListener( NXT_HThemeRenderer renderer, jclass wstl )
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !wstl ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }

    renderer->jclass_wrapperforSurfaceTextureListener = wstl;

    NXT_ThemeRenderer_InitForSurfaceTexture( renderer );
    return NXT_Error_None;
}

#endif //#ifdef ANDROID


NXT_Error NXT_ThemeRenderer_GetProperty(NXT_HThemeRenderer hRenderer,int nPropertyName,void**ppRetValue)
{
    NXT_Error result = NXT_Error_BadParam;
#ifdef USE_PLATFORM_SUPPORT
    result = NXT_Platform_GetProperty(hRenderer, nPropertyName, ppRetValue);
#endif
    return result;
}

#include <map>
#include <string>

class KEDLManager{

	typedef std::map<std::string, NXT_HThemeSet> ThemeMapper_t;

	ThemeMapper_t theme_mapper_;
    NXT_HThemeRenderer_Context context_;
    pthread_mutex_t kedl_lock_;
    
    NXT_HThemeSet get_internal(const char* uid){

		ThemeMapper_t::iterator itor = theme_mapper_.find(std::string(uid));
		if(itor == theme_mapper_.end()){

			return NULL;
        }
        NXT_HThemeSet htheme = itor->second;
        NXT_ThemeSet_Retain(htheme);
		return htheme;
	}

public:
	KEDLManager(NXT_HThemeRenderer_Context context):context_(context){

        memset(&kedl_lock_, 0x0, sizeof(kedl_lock_));
        pthread_mutex_init(&kedl_lock_,NULL);
	}

	~KEDLManager(){

        pthread_mutex_destroy(&kedl_lock_);
	}

	void destroy(int isdetached){

        pthread_mutex_lock(&kedl_lock_);

        for(ThemeMapper_t::iterator itor = theme_mapper_.begin(); itor != theme_mapper_.end(); ++itor){

			NXT_HThemeSet themeset = itor->second;
			NXT_ThemeSet_Release(context_, themeset, isdetached);
		}
		theme_mapper_.clear();
        pthread_mutex_unlock(&kedl_lock_);
	}

	NXT_HThemeSet get(const char* uid){

        pthread_mutex_lock(&kedl_lock_);
        NXT_HThemeSet htheme = get_internal(uid);
        pthread_mutex_unlock(&kedl_lock_);
		return htheme;
	}

    void release(const char* uid, int isdetached){

        pthread_mutex_lock(&kedl_lock_);
        ThemeMapper_t::iterator itor = theme_mapper_.find(std::string(uid));
		if(itor == theme_mapper_.end()){

            pthread_mutex_unlock(&kedl_lock_);
			return;
        }
        NXT_HThemeSet htheme = itor->second;        
        if(NXT_ThemeSet_Release(context_, htheme, isdetached)){

            theme_mapper_.erase(itor);
        }
        pthread_mutex_unlock(&kedl_lock_);
    }

	NXT_HThemeSet put(const char* uid, const char* src){

        pthread_mutex_lock(&kedl_lock_);
		NXT_HThemeSet themeset = get_internal(uid);
		if(NULL == themeset){

			themeset = NXT_ThemeSet_CreateFromString(src);
            NXT_ThemeSet_Retain(themeset);
			theme_mapper_.insert(make_pair(std::string(uid), themeset));
		}
        pthread_mutex_unlock(&kedl_lock_);
		return themeset;
	}
};

unsigned int NXT_ThemeRenderer_::getTrackIssueID(){

    return track_issue_id_++;
}

void NXT_ThemeRenderer_::arrangeSurfaceResource(){

    for(surfaceresource_t::iterator itor = surfaceresource_.begin(); itor != surfaceresource_.end(); ++itor){

        unsigned int texid = *itor;
        findTexIDAndClear(texid);
        glDeleteTextures(1, &texid);
    }

    surfaceresource_.clear();
}

void NXT_ThemeRenderer_::keepSurfaceResource(unsigned int tex_id){

    if(export_mode_){

        findTexIDAndClear(tex_id);
        glDeleteTextures(1, &tex_id);
    }
    else
        surfaceresource_.push_back(tex_id);
}

void NXT_ThemeRenderer_::resetTrackIssueID(){

    track_issue_id_ = 0;
}

void NXT_ThemeRenderer_::setForceRTT(int flag){

    force_rtt_ = flag;
}

int NXT_ThemeRenderer_::getForceRTT(){

    return force_rtt_;
}

NXT_ThemeRenderer_::NXT_ThemeRenderer_(){

    resetTrackIssueID();
    resetCurrentTrackID();

    moveX_value = 0.0f;
    moveY_value = 0.0f;

    force_rtt_ = 0;

    export_mode_ = 0;
    only_rgb_possible_surface_ = 0;
    mask_init_flag_ = 0;
    tex_id_for_vignette_ = 0;
    
    getLutTextureWithID = NULL;
    getEffectImagePath = NULL;
    getVignetteTexID = NULL;
    
    rendererType = NXT_RendererType_None;
    bIsInternalContext = 0;
    
    pFastPreviewOptions = NULL;

    memset(&themesetLock, 0x0, sizeof(themesetLock));
    memset(&cachedbitmaplock_, 0x0, sizeof(cachedbitmaplock_));
    //Jeff RTT------------------------------------------------------------------------------------------
    pmain_render_target_ = NULL;
    pmain_mask_target_ = NULL;
    pmain_blend_target_ = NULL;

	initVideoTextureResource();

    fastpreview_on = 0;
    fastpreview_total = 0;
    fastpreview_index = 0;
    fastpreview_width = 0;
    fastpreview_height = 0;

    fastpreview_maxCount = 0;
    pFastPreviewBuffer = NULL;
    pFastPreviewBufferFirst = NULL;
    pFastPreviewBufferLast = NULL;
    pFastPreviewBufferCur = NULL;
    fastpreview_renderOutputType = NXT_RendererOutputType_JUST_CLEAR;
    
    bInitialized = 0;
    bCalledGetPixels = 0;
    bDidRender = 0;
    bAvailableMultisample = 0;
    bEnableCustomRendering = 0;

    loadFileCallback = NULL;
    loadImageCallback = NULL;
    customRenderCallback = NULL;
    freeImageCallback = NULL;
    imageCallbackPvtData = NULL;
    
    outputType = NXT_RendererOutputType_JUST_CLEAR;
    channelFilter = NXT_ChannelFilter_ALL;
    
#ifdef NEXTHEME_SUPPORT_OPENGL
    pSolidShaderActive = NULL;
    pTexturedShaderActive = NULL;
    pMaskedShaderActive = NULL;
#endif //NEXTHEME_SUPPORT_OPENGL

    alpha = 0.0f;
    animframe = 0.0f;
    y_coord = 0.0f;
    texture = NULL;
    mask = NULL;
    renderDest = NXT_RenderDest_Screen;
    renderTest = NXT_RenderTest_None;
    colorMask[0] = 0;
    colorMask[1] = 0;
    colorMask[2] = 0;
    colorMask[3] = 0;
    bAntiAlias = 0;
    bDepthTest = 0;
    bDepthMask = 0;
    compMode = NXT_CompMode_Normal;

    bExportMode = 0;

#ifdef ANDROID
    nativeWindow = NULL;
    nativeWindowSurface = EGL_NO_SURFACE;;
    nativeExportSurface = EGL_NO_SURFACE;;
    
    nativeWindowWidth = 0;
    nativeWindowHeight = 0;
    exportSurfaceWidth = 0;
    exportSurfaceHeight = 0;
    
    java_vm = NULL;
    memset(&jclass_wrapperforSurfaceTextureListener, 0x0, sizeof(jclass_wrapperforSurfaceTextureListener));
    memset(&classSurfaceTexture, 0x0, sizeof(classSurfaceTexture));
    memset(&methodSurfaceTexture_updateTexImage, 0x0, sizeof(methodSurfaceTexture_updateTexImage));
    memset(&methodSurfaceTexture_getTransformMatrix, 0x0, sizeof(methodSurfaceTexture_getTransformMatrix));
    memset(&methodWSTL_waitFrameAvailable, 0x0, sizeof(methodWSTL_waitFrameAvailable));
    memset(surfaceTextures, 0x0, sizeof(surfaceTextures));
    memset(&outputSurfaceTexture, 0x0, sizeof(outputSurfaceTexture));
#endif
    platCtx = NULL;
    
    resetVideoSrc();
    cur_effect_overlap = 0;
    last_upload_tex_id = 0;
    
    themeSet = NULL;
    
    bNoFx = 0;
    bSwapV = 0;

//    memset(&watermarkEffect, 0x0, sizeof(watermarkEffect));
//    memset(&transitionEffect, 0x0, sizeof(transitionEffect));
//    memset(&clipEffect, 0x0, sizeof(clipEffect));

    pActiveEffect = NULL;

    memset(overlayEffectMark, 0x0, sizeof(overlayEffectMark));
  //  memset(overlayEffect, 0x0, sizeof(overlayEffect));

    bActiveEffectChanged = 0;
    effectType = NXT_EffectType_NONE;
    clip_index = 0;
    clip_count = 0;
    
    original_cts = 0;
    cts = 0;
    cur_time = 0;
    max_set_time = 0;
    max_time = 0;
    bSkipChildren = 0; // Set to 1 in beginRenderFunc to skip children of current node
    inTime = 0;
    outTime = 0;
    cycleTime = 0;
    repeatType = NXT_RepeatType_None;
    part = NXT_PartType_All;
    
    settime = 0.0f;
    time = 0.0f;
    
    view_width = 0;
    view_height = 0;
    cullFace = NXT_CullFace_None;
    
    videoPlaceholder1 = NULL;
    videoPlaceholder2 = NULL;
    titleText = NULL;
    masterTextSerial = 0;         // Serial number that is incremented every time the titleText OR EFFECT OPTIONS changes (used for caching rendered text)
    
    effectStartTime = 0;
    effectEndTime = 0;
    
    adj_brightness = 0.0f;
    adj_contrast = 0.0f;
    adj_saturation = 0.0f;
    
    pReadPixelData = NULL;
    readPixelDataSize = 0;

    specularExponent = 0.0f;
    shadingMode = NXT_ShadingMode_Off;
    lightingNormals = NXT_LightingNormals_Front;
    
    bFlags = 0;
    
    // --- Watermark texture -------------------------------------------------------
    bLoadedWatermark = 0;
    bUseWatermark = 0;
    memset(&watermarkTexture, 0x0, sizeof(watermarkTexture));
    
    disable_watermark = 0;

    //Jeff
    lut_texture_ = 0;
    lut_strength_ = 0.0f;
    defaultFrameBufferNum = 0;

    currentTime = 0;
    actualEffectStartCTS = 0;
    actualEffectEndCTS = 0; 

    final_brightness_ = 0;
    final_contrast_ = 0;
    final_saturation_ = 0;

    final_vignette_ = 0;
    final_vignetteRange_ = 0;
    final_sharpness_ = 0;
    
    deviceLightLevel = 0;
    deviceGamma = 0.0;
}

NXT_HThemeRenderer_Context NXT_ThemeRendererContext_Create(int max_texture_cache_mem_size, unsigned int bIsInternalContext, unsigned int bIsMI5C){

    return new NXT_ThemeRenderer_Context(max_texture_cache_mem_size, bIsInternalContext, bIsMI5C);
};

void NXT_ThemeRendererContext_Destroy(NXT_HThemeRenderer_Context ctx){

    if(ctx)
        delete ctx;
};

NXT_HThemeRenderer NXT_ThemeRenderer_Create(NXT_RendererType rendererType, int* properties, NXT_HThemeRenderer_Context ctx, NXT_HThemeRenderer anotherRendererForShareGroup)
{
    
    LOGN("[%s %d] Nexstreaming Theme Renderer : Version " NEXTHEME_VERSION, __func__, __LINE__);
    NXT_HThemeRenderer renderer = new NXT_ThemeRenderer();
    if( !renderer ) {
        LOGF("[%s %d] Memory allocation failed", __func__, __LINE__);
        return NULL;
    }

    renderer->pcontext_ = ctx;
    renderer->basefilter_renderitem_id_ = -1;
    renderer->basefilter_renderitem_uid_[0] = 0;

    
    switch(rendererType) {
        case NXT_RendererType_ExternalContext:
            renderer->bIsInternalContext = 0;
            break;
        case NXT_RendererType_InternalPreviewContext:
            renderer->bIsInternalContext = 1;
            break;
        case NXT_RendererType_InternalExportContext:
            renderer->bIsInternalContext = 1;
            break;
        default:
            LOGF("[%s %d] Bad renderer type (%d)", __func__, __LINE__, rendererType);
            delete renderer;
            return NULL;
    }
	
	renderer->bDepthMask = 1;
    renderer->rendererType = rendererType;
    renderer->bEnableCustomRendering = 1;
#ifdef ANDROID
    renderer->nativeWindowSurface = EGL_NO_SURFACE;
    renderer->nativeExportSurface = EGL_NO_SURFACE;
    renderer->bExportMode = 0;
#endif // ANDROID
	renderer->defaultFrameBufferNum = 0;
	
	if( renderer->bIsInternalContext ) {
#ifdef USE_PLATFORM_SUPPORT
		NXT_Error result = NXT_Platform_CreateInternalContext(renderer, anotherRendererForShareGroup);
		if (result != NXT_Error_None) {
			LOGF("[%s %d] Create internal context failed (%d)", __func__, __LINE__, result);
			delete renderer;
			return NULL;
		}
#endif
	}

    pthread_mutex_init(&renderer->themesetLock,NULL);
    pthread_mutex_init(&renderer->cachedbitmaplock_, NULL);
	
    return renderer;
}

static NXT_Error initRenderer( NXT_HThemeRenderer renderer )
{
    //int i;
    
    if( !renderer ) {
        return NXT_Error_BadParam;
    } else if( renderer->bInitialized ) {
        return NXT_Error_NoEffect;
    }
              
    // Check that we have an OpenGL context
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            LOGE("[%s %d] NO EGL CONTEXT - Upload texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO GL CONTEXT - Upload texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }

    renderer->getContextManager()->initDefaultTextures();
    GLint maxTextureSize = 2048;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    NXT_Theme_SetTextureMaxSize(renderer->getRenderItemManager(), maxTextureSize);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	CHECK_GL_ERROR();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	CHECK_GL_ERROR();

    NexThemeRenderer_PrintGLString("Version", GL_VERSION);
    NexThemeRenderer_PrintGLString("Vendor", GL_VENDOR);
    NexThemeRenderer_PrintGLString("Renderer", GL_RENDERER);
    NexThemeRenderer_PrintGLString("Extensions", GL_EXTENSIONS);	
    
#ifdef LOAD_SHADER_ON_TIME
    NXT_Error err = NexThemeRenderer_PrepareShaders(renderer->getContextManager());
#else
    NXT_Error err = NexThemeRenderer_LoadShaders(renderer->getContextManager());
#endif
    if( err ) {
        LOGF("[%s %d] FAILED loading one or more shaders", __func__, __LINE__ );
        NXT_ThemeRenderer_Destroy(renderer, 0);
        return err;
    }

    NXT_Theme_InitContextOnManager(renderer->getRenderItemManager());
    //NXT_ThemeRenderer_ReleaseRenderItem(renderer, -1);
    renderer->tex_id_for_vignette_ = 0;
    NXT_Theme_UpdateRenderItemManager(renderer->getRenderItemManager(), 0, 0, 0);


    renderer->color     = (NXT_Vector4f){0.5,0.5,0.5,1.0};
    renderer->alpha     = 1.0;
    renderer->texture   = NULL;
    renderer->mask      = NULL;
    renderer->titleText = NULL;
    renderer->masterTextSerial= 1;
	renderer->y_coord 	= 1.0;
    renderer->ambientColor	= (NXT_Vector4f){1.0,1.0,1.0,1.0};
    renderer->diffuseColor	= (NXT_Vector4f){0.0,0.0,0.0,0.0};
    renderer->specularColor	= (NXT_Vector4f){0.0,0.0,0.0,0.0};
    renderer->specularExponent = 10;
    renderer->lightDirection = (NXT_Vector4f){10000.0,10000.0,10000.0,0.0};
    
    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();
    
    glDisable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
    glDisable(GL_CULL_FACE);
    CHECK_GL_ERROR();
    
    renderer->bInitialized = 1;
    LOGI("[%s %d] Initialize OK", __func__, __LINE__);

    
    unsigned char *tmpbuf = (unsigned char *)malloc((720+360)*1280);
    if( tmpbuf ) {
        memset(tmpbuf+(720*1280),0x80,360*1280);
        NXT_ThemeRenderer_UploadTexture(renderer, 0, NXT_TextureID_Video_1, 1280, 720, 1280, NXT_PixelFormat_NV12, tmpbuf, tmpbuf+(720*1280), NULL, 0, 0, 0, 0, NULL);
        NXT_ThemeRenderer_UploadTexture(renderer, 0, NXT_TextureID_Video_2, 1280, 720, 1280, NXT_PixelFormat_NV12, tmpbuf, tmpbuf+(720*1280), NULL, 0, 0, 0, 0, NULL);
        LOGI("[%s %d] Uploaded initial textures", __func__, __LINE__);
        free(tmpbuf);
        tmpbuf=0;
    }
    
    return NXT_Error_None;
}

void NXT_ThemeRenderer_Destroy(NXT_HThemeRenderer renderer, unsigned int isDetachedContext)
{
    LOGI("[%s %d]DetachedContext(%d)", __func__, __LINE__, isDetachedContext);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    NXT_ThemeRenderer_AquireContext(renderer);

    NXT_ThemeRenderer_ReleaseRenderItem(renderer, -1);

    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_render_target_);
    renderer->pmain_render_target_ = NULL;
	NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_mask_target_);
	renderer->pmain_mask_target_ = NULL;
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_blend_target_);
    renderer->pmain_blend_target_ = NULL;
    renderer->arrangeSurfaceResource();
    renderer->releaseAllVideoTexture();
    // for memory leak in theme renderer 7813
    if( renderer->themeSet ) {
        NXT_ThemeSet_Release(renderer->getContextManager(), renderer->themeSet, isDetachedContext);
        renderer->themeSet = NULL;
    }

    //delete basefilter_keyvalues_
    NXT_FreeRenderitemKeyValues(&renderer->basefilter_keyvalues_);

    if( renderer->watermarkTexture.header.isa ) {
        if( renderer->watermarkTexture.header.isa->freeTextureFunc && !isDetachedContext )
        {
            renderer->watermarkTexture.header.isa->freeTextureFunc(renderer->getContextManager(), &renderer->watermarkTexture.header);
        }
    	renderer->watermarkTexture.header.isa->freeNodeFunc(&renderer->watermarkTexture.header);
        renderer->watermarkTexture.header.isa = NULL;
    }

    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    
#ifdef ANDROID    
    switch( renderer->rendererType ) {
        case NXT_RendererType_ExternalContext:
            break;
        case NXT_RendererType_InternalExportContext:
            if( renderer->bExportMode ) {
                NXT_ThemeRenderer_EndExport(renderer);
            }
            break;
        case NXT_RendererType_InternalPreviewContext:
            NXT_ThemeRenderer_SetNativeWindow(renderer,NULL, 0,0);
            break;
        default:
            break;
    }
    
    if(renderer->bIsInternalContext && renderer->getContext()) {
    
        LOGI("[%s %d] Destroying context(0x%X) for display(0x%X)", __func__, __LINE__, LOGPTR(renderer->getContext()), LOGPTR(renderer->getContextManager()->eglDisplay));

        // for add below code for hangup after eglReleaseThread function
        eglMakeCurrent( renderer->getContextManager()->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

        eglDestroyContext(renderer->getContextManager()->eglDisplay, renderer->getContext());
        CHECK_EGL_ERROR();
        renderer->setContext(EGL_NO_CONTEXT);
        LOGI("[%s %d] EGL Terminate and Release Thread", __func__, __LINE__);
    }
#endif

#ifdef USE_PLATFORM_SUPPORT
    if( renderer->bIsInternalContext ) {
        NXT_Platform_SetCurrentInternalContext( renderer, SET_CONTEXT_MODE_RELEASE );
        NXT_Platform_DeleteInternalContext( renderer );
    }
#endif
    /*
     // Free video textuers (disabled because this may be called on a non-GL thread; presumably these are automatically cleaned up with the context -- we hope!)
    int texture_id;
    for( texture_id=0; texture_id<NXT_TextureID_COUNT; texture_id++ ) {
        NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
        if( tex->texNameInitCount>0 ) {
            GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
            tex->texNameInitCount = 0;
        }
        if( tex->eglImageID != EGL_NO_IMAGE_KHR )
        {
            eglDestroyImageKHR( eglGetCurrentDisplay(), tex->eglImageID );
            tex->eglImageID = EGL_NO_IMAGE_KHR;
        }
        tex->bUseNativeBuffer = 0;
        tex->bValidTexture = 0;
    }*/
    
    if( renderer->titleText ) {
        free(renderer->titleText);
        renderer->titleText = NULL;
    }

#ifdef ANDROID
    NXT_ThemeRenderer_DeinitForSurfaceTexture( renderer );
#endif
    freeAppliedEffect(&renderer->clipEffect);
    freeAppliedEffect(&renderer->transitionEffect);
    freeAppliedEffect(&renderer->watermarkEffect);
    
    if( renderer->pReadPixelData ) {
        free(renderer->pReadPixelData);
        renderer->pReadPixelData = NULL;
        renderer->readPixelDataSize = 0;
    }
    
    if( renderer->videoPlaceholder1 ) {
        free( renderer->videoPlaceholder1 );
        renderer->videoPlaceholder1 = NULL;
    }
    
    if( renderer->videoPlaceholder2 ) {
        free( renderer->videoPlaceholder2 );
        renderer->videoPlaceholder2 = NULL;
    }
        
    pthread_mutex_destroy(&renderer->themesetLock);
    pthread_mutex_destroy(&renderer->cachedbitmaplock_);


    delete renderer;
}

NXT_AppliedEffect::~NXT_AppliedEffect(){

    freeAppliedEffect(this);
}

void NXT_ThemeRenderer_FreeAppliedEffect(NXT_AppliedEffect* appliedEffect){

	freeAppliedEffect(appliedEffect);
}

static void freeAppliedEffect( NXT_AppliedEffect *appliedEffect ) {
    
    if( !appliedEffect ) {
        // ERROR: null pointer
        return;
    }
    
    if( appliedEffect->effectOptionsEncoded ) {
        free(appliedEffect->effectOptionsEncoded);
        appliedEffect->effectOptionsEncoded = NULL;
    }

    KeyValueMap_t& valuemap = appliedEffect->keyvaluemap_;
    for(KeyValueMap_t::iterator itor = valuemap.begin(); itor != valuemap.end(); ++itor){

        if(itor->second)
            delete itor->second;
    }
    valuemap.clear();
    
    freeDecodedEffectOptions(appliedEffect);

}

static void freeDecodedEffectOptions( NXT_AppliedEffect *appliedEffect ) {
    
    if( !appliedEffect ) {
        // ERROR: null pointer
        return;
    }
    
    appliedEffect->effectOptionCount = 0;
    appliedEffect->transitionOptionCount = 0;
    appliedEffect->clipEffectOptionCount = 0;
    
    if( appliedEffect->effectOptionsBuffer ) {
        free(appliedEffect->effectOptionsBuffer);
        appliedEffect->effectOptionsBuffer = NULL;
    }
    
    if( appliedEffect->effectOptionKeys ) {
        free(appliedEffect->effectOptionKeys);
        appliedEffect->effectOptionKeys = NULL;
    }
    
    if( appliedEffect->effectOptionValues ) {
        free(appliedEffect->effectOptionValues);
        appliedEffect->effectOptionValues = NULL;
    }
    
}

static int isHexDigit( char c ) {
    if( c>='0' && c<='9' ) {
        return 1;
    } else if( c>='A' && c<='F' ) {
        return 1;
    } else if( c>='a' && c<='f' ) {
        return 1;
    } else {
        return 0;
    }
}

static int hexDigitValue( char c ) {
    if( c>='0' && c<='9' ) {
        return c - '0';
    } else if( c>='A' && c<='F' ) {
        return c - 'A' + 10;
    } else if( c>='a' && c<='f' ) {
        return c - 'a' + 10;
    } else {
        return 0;
    }
}

static int parseint( char* s, char **tail ) {
    int result = 0;
    while( *s >='0' && *s <='9' ) {
        result *= 10;
        result += (*s - '0');
        s++;
    }
    if( tail ) {
        *tail = s;
    }
    return result;
}

void converter(char* pbuf, char** option_keys, char** option_values, int option_count, int* plength){

	int i = 0;

	for(i = 0; i < option_count; ++i) {
    	
    	int option_filter_pos = 0;
    	char option_filter[256] = {};
        char* pcmp = option_keys[i];
        
        int length = strlen(pcmp);

        if(pbuf){

        	option_keys[i] = pbuf;
        	strcpy(pbuf, pcmp);
        	pbuf += length;
        	*pbuf++ = 0;
        }

        if(plength){

        	*plength += (length + 1);
     	}
        

        while(*pcmp && (*pcmp != '.' && *pcmp != ':')){

        	option_filter[option_filter_pos++] = *pcmp++;
        }

        if(*pcmp == ':')
        	*pcmp = '.';

        if(*pcmp== '.'){

        	option_filter[option_filter_pos] = 0;
        	char* option_pointer = pbuf;

        	if(!strcasecmp(option_filter, "color")){

        		if(pbuf){

        			char* pvalue = option_values[i];
        			pvalue++;//skip #
        			const char* member[] = {"x=", "y=", "z=", "w="};
	        		*pbuf++ = '{';
	        		int member_index = 0;
	        		for(member_index = 0; member_index < 4; ++member_index){

	        			strcpy(pbuf, member[member_index]);
	        			pbuf += (strlen(pbuf));
	        			*pbuf++ = '0';
	        			*pbuf++ = 'x';
	        			*pbuf++ = *pvalue++;
	        			*pbuf++ = *pvalue++;
	        			*pbuf++=',';
	        		}
	        		pbuf--;
	        		*pbuf++ = '}';
	        		*pbuf++ = 0;
        		}

        		if(plength){

        			(*plength) += (strlen("{x=0xFF,y=0xFF,z=0xFF,w=0xFF}") + 1);
    			}
        	}
        	else if(!strcasecmp(option_filter, "selection")|| !strcasecmp(option_filter, "choice")){

        		//option value should be converted as RenderItem Color vector table
        		int member_index = -1;
        		int search_next = 1;
        		const char* member[] = {"x=", "y=", "z=", "w="};
        		char* pvalue = option_values[i];

        		if(pbuf) *pbuf++ = '{';
        		if(plength) (*plength)++;
        		while(*pvalue){

        			if(search_next){

        				if(*pvalue > 32){

        					if(plength) (*plength) += 2;//?=
        					search_next = 0;
        					member_index ++;
        					if(member_index > 0){

        						if(pbuf) *pbuf++ = ',';
        						if(plength) (*plength)++;//for comma
        					}
        					if(pbuf) *pbuf++ = member[member_index][0];
        					if(pbuf) *pbuf++ = member[member_index][1];
        				}
        			}
        			else{

        				if(*pvalue <= 32){
        					search_next = 1;
        				}
        			}
        			if(search_next == 0){
        				if(pbuf) *pbuf++ = *pvalue;
        				if(plength) (*plength)++;
        			}
        			pvalue++;
        		}

        		if(pbuf) *pbuf++ = '}';
        		if(pbuf) *pbuf++ = 0;
        		if(plength) (*plength) += 2;
        	}
        	else if(!strcasecmp(option_filter, "rect")){

        		//option value should be converted as RenderItem Color vector table
        		int member_index = -1;
        		int search_next = 1;
        		const char* member[] = {"x0=", "y0=", "x1=", "y1="};
        		char* pvalue = option_values[i];

        		if(pbuf) *pbuf++ = '{';
        		if(plength) (*plength)++;
        		while(*pvalue){

        			if(search_next){

        				if(*pvalue > 32){

        					if(plength) (*plength) += 3;//x0=
        					search_next = 0;
        					member_index ++;
        					if(member_index > 0){

        						if(pbuf) *pbuf++ = ',';
        						if(plength) (*plength)++;//for comma
        					}
        					if(pbuf) *pbuf++ = member[member_index][0];
        					if(pbuf) *pbuf++ = member[member_index][1];
        					if(pbuf) *pbuf++ = member[member_index][2];
        				}
        			}
        			else{

        				if(*pvalue <= 32){
        					search_next = 1;
        				}
        			}
        			if(search_next == 0){
        				if(pbuf) *pbuf++ = *pvalue;
        				if(plength) (*plength)++;
        			}
        			pvalue++;
        		}

        		if(pbuf) *pbuf++ = '}';
        		if(pbuf) *pbuf++ = 0;
        		if(plength) (*plength) += 2;
        	}
        	else if(!strcasecmp(option_filter, "point")){

        		//option value should be converted as RenderItem Color vector table
        		int member_index = -1;
        		int search_next = 1;
        		const char* member[] = {"x=", "y=", "z="};
        		char* pvalue = option_values[i];

        		if(pbuf) *pbuf++ = '{';
        		if(plength) (*plength)++;
        		while(*pvalue){

        			if(search_next){

        				if(*pvalue > 32){

        					if(plength) (*plength) += 2;//x=
        					search_next = 0;
        					member_index ++;
        					if(member_index > 0){

        						if(pbuf) *pbuf++ = ',';
        						if(plength) (*plength)++;//for comma
        					}
        					if(pbuf) *pbuf++ = member[member_index][0];
        					if(pbuf) *pbuf++ = member[member_index][1];
        				}
        			}
        			else{

        				if(*pvalue <= 32){
        					search_next = 1;
        				}
        			}
        			if(search_next == 0){
        				if(pbuf) *pbuf++ = *pvalue;
        				if(plength) (*plength)++;
        			}
        			pvalue++;
        		}

        		if(pbuf) *pbuf++ = '}';
        		if(pbuf) *pbuf++ = 0;
        		if(plength) (*plength) += 2;
        	}
        	else if(!strcasecmp(option_filter, "text")){

        		if(plength) (*plength) += (strlen(option_values[i]) + 4 + 1);//for the double square mark for string value,

        		if(pbuf){

	        		*pbuf++ ='[';
	        		*pbuf++ ='[';
	        		strcpy(pbuf, option_values[i]);
	        		pbuf += strlen(option_values[i]);
	        		*pbuf++ =']';
	        		*pbuf++ =']';
	        		*pbuf++ = 0;	
        		}
        	}
        	else if(!strcasecmp(option_filter, "image")){

        		if(plength) (*plength) += (strlen(option_values[i]) + 2 + 1);//for the quatation mark for string value,

        		if(pbuf){

	        		*pbuf++ ='\'';
	        		strcpy(pbuf, option_values[i]);
	        		pbuf += strlen(option_values[i]);
	        		*pbuf++ ='\'';
	        		*pbuf++ = 0;
        		}
        	}
        	else{

        		if(pbuf){

        			strcpy(pbuf, option_values[i]);
	        		pbuf += strlen(option_values[i]);
	        		*pbuf++ = 0;
        		}
        		if(plength) (*plength) += (strlen(option_values[i]) + 1);
        	}
        	if(option_pointer) option_values[i] = option_pointer;
        }
    }
}

static void convertToRenderItemOption(NXT_HThemeRenderer renderer, NXT_AppliedEffect *appliedEffect) {
    
    if( !appliedEffect ) {
        // ERROR: null pointer
        return;
    }

    if(appliedEffect->render_item_encoded != 0)
    	return;

    appliedEffect->render_item_encoded = 1;

    NXT_EffectType effectType = appliedEffect->effectType; //NXT_Effect_GetType(renderer->effect);
    
    int optionCount;
    char **optionKeys;
    char **optionValues;
    
    if( effectType == NXT_EffectType_Title ) {
        optionCount = appliedEffect->clipEffectOptionCount;
        optionKeys = appliedEffect->effectOptionKeys + appliedEffect->transitionOptionCount;
        optionValues = appliedEffect->effectOptionValues + appliedEffect->transitionOptionCount;
    } else if( effectType == NXT_EffectType_Transition ) {
        optionCount = appliedEffect->transitionOptionCount;
        optionKeys = appliedEffect->effectOptionKeys;
        optionValues = appliedEffect->effectOptionValues;
    } else {
        return;
    }

    int length_needed = 0;

    LOGI("[%s %d] val:%s count:%d", __func__, __LINE__, optionValues, optionCount);

    converter(NULL, optionKeys, optionValues, optionCount, &length_needed);
    char* buf = (char*)malloc(sizeof(char) * length_needed + 1);
    buf[length_needed] = 0;    
    converter(buf, optionKeys, optionValues, optionCount, NULL);

    if(appliedEffect->effectOptionsBuffer) {

        free(appliedEffect->effectOptionsBuffer);
        appliedEffect->effectOptionsBuffer = NULL;
    }
    
    appliedEffect->effectOptionsBuffer = buf;
}

static void applyRenderItemOption(NXT_HThemeRenderer renderer, NXT_AppliedEffect *appliedEffect){

	if( !appliedEffect ) {
        // ERROR: null pointer
        return;
    }

    convertToRenderItemOption(renderer, appliedEffect);

    NXT_EffectType effectType = appliedEffect->effectType; //NXT_Effect_GetType(renderer->effect);
    
    int i;
    int optionCount;
    char **optionKeys;
    char **optionValues;
    char *pCmp;
    
    if( effectType == NXT_EffectType_Title ) {
        optionCount = appliedEffect->clipEffectOptionCount;
        optionKeys = appliedEffect->effectOptionKeys + appliedEffect->transitionOptionCount;
        optionValues = appliedEffect->effectOptionValues + appliedEffect->transitionOptionCount;
    } else if( effectType == NXT_EffectType_Transition ) {
        optionCount = appliedEffect->transitionOptionCount;
        optionKeys = appliedEffect->effectOptionKeys;
        optionValues = appliedEffect->effectOptionValues;
    } else {
        return;
    }

    NXT_Theme_ResetDefaultValuemap(renderer->getRenderItemManager());

    for( i=0; i<optionCount; i++ ) {

        NXT_Theme_SetValue(renderer->getRenderItemManager(), optionKeys[i], optionValues[i]);
    }

    NXT_Theme_ApplyDefaultValues(renderer->getRenderItemManager());
}

static void parseEffectOptions( NXT_AppliedEffect *appliedEffect ) {
    
    if( !appliedEffect ) {
        // ERROR: null pointer
        return;
    }
    
    if( !appliedEffect->effectOptionsEncoded )
        return;
    
    freeDecodedEffectOptions(appliedEffect);
    
    /*
     
     int effectOptionCount;
     int transitionOptionCount;
     int clipEffectOptionCount;
     char *effectOptionsEncoded;
     char *effectOptionsBuffer;
     char **effectOptionKeys;
     char **effectOptionValues;

     
     */
    
    int effectOptionsLen = 0;
    char *p;
    char *effectOptionsEncoded = appliedEffect->effectOptionsEncoded;
    char *tail = 0;
    
    p = effectOptionsEncoded;
    
    while( *p==' ' )
        p++;
    appliedEffect->effectUserStartTime = parseint(p, &tail);
    while( *tail==' ' )
        tail++;
    if( tail!=p && *tail==',' ) {
        p = tail+1;
        while( *p==' ' )
            p++;
        appliedEffect->effectUserEndTime = parseint(p, &tail);
        while( *tail==' ' )
            tail++;
        if( tail != p && *tail=='?' ) {
            p = tail+1;
            while( *p==' ' )
                p++;
        } else {
            appliedEffect->effectUserStartTime = 0;
            appliedEffect->effectUserEndTime = 0;
        }
    } else {
        appliedEffect->effectUserStartTime = 0;
        appliedEffect->effectUserEndTime = 0;
    }
    
    effectOptionsEncoded = p;

    appliedEffect->effectOptionCount = 0;
    appliedEffect->transitionOptionCount = 0;
    appliedEffect->clipEffectOptionCount = 0;
    int bTransition = 1;
    for( p = effectOptionsEncoded; *p; p++ ) {
        effectOptionsLen++;
        if( *p=='?' ) {
            bTransition=0;
        } else if( *p=='=' ) {
            appliedEffect->effectOptionCount++;
            if( bTransition ) {
                appliedEffect->transitionOptionCount++;
            } else {
                appliedEffect->clipEffectOptionCount++;
            }
        }
    }


	if( appliedEffect->effectOptionCount > 0 )
		appliedEffect->effectOptionKeys = (char **)malloc(sizeof(char*)*appliedEffect->effectOptionCount);
	if( appliedEffect->effectOptionCount > 0 )
		appliedEffect->effectOptionValues = (char **)malloc(sizeof(char*)*appliedEffect->effectOptionCount);
    appliedEffect->effectOptionsBuffer = (char*)malloc(effectOptionsLen+1);
    
    if( !appliedEffect->effectOptionKeys || !appliedEffect->effectOptionValues || !appliedEffect->effectOptionsBuffer ) {
        freeDecodedEffectOptions(appliedEffect);
        return;
    }
    
    char* buf = appliedEffect->effectOptionsBuffer;
    char* pkey = NULL;
    char* pvalue = NULL;
    char** pkeylist = appliedEffect->effectOptionKeys;
    char** pvaluelist = appliedEffect->effectOptionValues;
    
    for( p = effectOptionsEncoded; *p; p++ ) {
        if( pkey==NULL && pvalue==NULL ) {
            pkey = buf;
        }
        if( *p == '=' ) {
            *buf++ = 0;
            pvalue = buf;
        } else if( *p == '&' ) {
            if( pvalue && pkey ) {
                *buf++ = 0;
                *pkeylist++ = pkey;
                *pvaluelist++ = pvalue;
            }
            pvalue = NULL;
            pkey = NULL;
        } else if( *p == '?' ) {
            if( pvalue && pkey ) {
                *buf++ = 0;
                *pkeylist++ = pkey;
                *pvaluelist++ = pvalue;
            }
            pvalue = NULL;
            pkey = NULL;
        } else if( *p == '+' ) {
            *buf++ = ' ';
        } else if( *p == '%' ) {
            if( isHexDigit(*(p+1)) && isHexDigit(*(p+2)) ) {
                *buf++ = ((hexDigitValue(*(p+1))<<4) | hexDigitValue(*(p+2)));
                p += 2;
            }
        } else {
            *buf++ = *p;
        }
    }
    if( pvalue && pkey ) {
        *buf++ = 0;
        *pkeylist++ = pkey;
        *pvaluelist++ = pvalue;
    }
}

char *NXT_ThemeRenderer_GetEffectOption( NXT_HThemeRenderer renderer, const char *pOptionKey ) {
    
    if( !renderer ) {
        // ERROR: null pointer
        return NULL;
    }
    
    if( renderer->pActiveEffect==NULL ) {
        return NULL;
    }

    NXT_EffectType effectType = renderer->pActiveEffect->effectType; //NXT_Effect_GetType(renderer->effect);
    
    int i;
    int optionCount;
    char **optionKeys;
    char **optionValues;
    char *pCmp;
    
    if( effectType == NXT_EffectType_Title ) {
        optionCount = renderer->pActiveEffect->clipEffectOptionCount;
        optionKeys = renderer->pActiveEffect->effectOptionKeys + renderer->pActiveEffect->transitionOptionCount;
        optionValues = renderer->pActiveEffect->effectOptionValues + renderer->pActiveEffect->transitionOptionCount;
    } else if( effectType == NXT_EffectType_Transition ) {
        optionCount = renderer->pActiveEffect->transitionOptionCount;
        optionKeys = renderer->pActiveEffect->effectOptionKeys;
        optionValues = renderer->pActiveEffect->effectOptionValues;
    } else {
        return NULL;
    }
    
    for( i=0; i<optionCount; i++ ) {
        pCmp = optionKeys[i];
        while( *pCmp && *pCmp!=':' )
            pCmp++;
        if( *pCmp==':' )
            pCmp++;
        if( !*pCmp )
            pCmp = optionKeys[i];
        if( strcmp(pOptionKey, pCmp)==0 ) {
            return optionValues[i];
        }
    }
    return NULL;
}

/*void NXT_ThemeRenderer_SetEffectOptions( NXT_HThemeRenderer renderer, const char *pEffectOptions )
{
    LOGI("[%s %d] renderer=0x%08X pEffectOptions='%s'", __func__, __LINE__, (unsigned int)renderer, pEffectOptions);
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
//    int effectOptionCount;
//    char *effectOptionsEncoded;
//    char *effectOptionsBuffer;
//    char **effectOptionKeys;
//    char **effectOptionValues;

    
    if( renderer->effectOptionsEncoded == pEffectOptions ||
       (renderer->effectOptionsEncoded && pEffectOptions && strcmp(renderer->effectOptionsEncoded,pEffectOptions)==0))
    {
		LOGI("[%s %d] Effect Option Match - IGNORE", __func__, __LINE__);
        // Already matches existing options
        return;
    }
    
    renderer->textSerial++;
    
    if( renderer->effectOptionsEncoded ) {
        free(renderer->effectOptionsEncoded);
        renderer->effectOptionsEncoded = NULL;
    }
    
    freeDecodedEffectOptions(renderer);
    renderer->effectStartTime = 0;
    renderer->effectEndTime = 0;
    
    if( pEffectOptions ) {
        renderer->effectOptionsEncoded = malloc(strlen(pEffectOptions)+1);
        if( !renderer->effectOptionsEncoded ) {
			LOGE("[%s %d] malloc failure", __func__, __LINE__);
            // ERROR: memory allocation
            return;
        }
        strcpy(renderer->effectOptionsEncoded, pEffectOptions);
        
        parseEffectOptions(renderer);
        
		LOGI("[%s %d] Flag effect changed", __func__, __LINE__);
		renderer->bEffectChanged = 1;
//        if(renderer->effect)
//            NXT_Effect_UpdateUserFields(renderer->effect, renderer);
    }
    
}
*/

/*void NXT_ThemeRenderer_SetTitleText( NXT_HThemeRenderer renderer, const char *titleText )
{
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    if( renderer->titleText == titleText || 
       (renderer->titleText && titleText && strcmp(renderer->titleText,titleText)==0)) 
    {
        // Already matches existing text
        return;
    }
       
    renderer->textSerial++;
    
    if( renderer->titleText ) {
        free(renderer->titleText);
        renderer->titleText = NULL;
    }
    
    if( titleText ) {
        renderer->titleText = malloc(strlen(titleText)+1);
        if( !renderer->titleText ) {
            // ERROR: memory allocation
            return;
        }
        strcpy(renderer->titleText, titleText);
    }
                                            
}*/

void NXT_ThemeRenderer_SetVideoPlaceholders(NXT_HThemeRenderer renderer,
                                            const char *video1,
                                            const char *video2)
{
    //LOGI("[%s %d] renderer=0x%08X video1='%s', video2='%s'", __func__, __LINE__, (unsigned int)renderer, video1, video2);
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }

    if( renderer->videoPlaceholder1 ) {
        free( renderer->videoPlaceholder1 );
        renderer->videoPlaceholder1 = NULL;
    }
    
    if( renderer->videoPlaceholder2 ) {
        free( renderer->videoPlaceholder2 );
        renderer->videoPlaceholder2 = NULL;
    }
    
    if( video1 ) {
        renderer->videoPlaceholder1 = (char*)malloc(strlen(video1)+1);
        strcpy( renderer->videoPlaceholder1, video1 );
    }
    
    if( video2 ) {
        renderer->videoPlaceholder2 = (char*)malloc(strlen(video2)+1);
        strcpy( renderer->videoPlaceholder2, video2 );
    }
    //LOGI("[%s %d] renderer->videoPlaceholder1='%s', renderer->videoPlaceholder2='%s'", __func__, __LINE__, renderer->videoPlaceholder1, renderer->videoPlaceholder2);
    
}

void NXT_ThemeRenderer_RegisterCustomRenderCallback(NXT_HThemeRenderer renderer,
                                                    NXT_CustomRenderCallback customRenderCallback)
{
    LOGD("[%s %d]", __func__, __LINE__);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    renderer->customRenderCallback = customRenderCallback;
}

void NXT_ThemeRenderer_RegisterImageCallbacks(NXT_HThemeRenderer renderer,
                                              NXT_LoadImageCallback loadImageCallback,
                                              NXT_FreeImageCallback freeImageCallback,
                                              NXT_LoadFileCallback loadFileCallback,
                                              void *cbdata)
{
    //LOGD("[%s %d]", __func__, __LINE__);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    renderer->loadImageCallback = loadImageCallback;
    renderer->freeImageCallback = freeImageCallback;
    renderer->loadFileCallback = loadFileCallback;
    renderer->imageCallbackPvtData = cbdata;
}

void NXT_ThemeRenderer_RegisterLoadfileCallback(NXT_HThemeRenderer renderer, NXT_LoadFileCallback loadFileCallback){

	LOGD("[%s %d]", __func__, __LINE__);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }

    renderer->loadFileCallback = loadFileCallback;
}

NXT_Error NXT_ThemeRenderer_LoadRenderItem(NXT_HThemeRenderer renderer, const char* pID, const char* renderitemData, int iFlags){

	if( !renderer ) 
    {
        return NXT_Error_BadParam;
    }

	renderer->bFlags = iFlags;

    NXT_ThemeRenderer_AquireContext(renderer);
	
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
	return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_LoadThemesAndEffects(NXT_HThemeRenderer renderer, const char* themeAndEffectData, int iFlags ) {
	
    if( !renderer ) 
    {
        return NXT_Error_BadParam;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
	LOGI("[%s %d] (acq=%d)", __func__, __LINE__, ctx->contextIsAcquired);

	// RYU 20130723 set flags.
	renderer->bFlags = iFlags;

    NXT_ThemeRenderer_AquireContext(renderer);

    renderer->clearEffect();

    NXT_ThemeRenderer_ClearClipEffect(renderer);
    NXT_ThemeRenderer_ClearTransitionEffect(renderer);

	NXT_HThemeSet themeSet = NULL;
	if( themeAndEffectData!=NULL ) {
		themeSet = NXT_ThemeSet_CreateFromString(themeAndEffectData);
	}
	
    pthread_mutex_lock(&renderer->themesetLock);

	if( renderer->themeSet!=themeSet )
	{
		renderer->bActiveEffectChanged = 1;

		if( renderer->themeSet!=NULL ) 
		{
	 	    // Theme decided that conext is detached or not.  !renderer->contextIsAcquired means not-detached-context
            NXT_ThemeSet_Release(renderer->getContextManager(), renderer->themeSet, !ctx->contextIsAcquired);
            renderer->themeSet = NULL;
		}

		if( themeSet!=NULL ) {
			NXT_ThemeSet_Retain(themeSet);
			renderer->themeSet = themeSet;
		}
        
//        NXT_ThemeRenderer_PrecacheImages(renderer, themeSet->rootNode, NXT_PrecacheMode_Upload);
	}

    pthread_mutex_unlock(&renderer->themesetLock);
	
	if( themeSet !=NULL ) {
		NXT_ThemeSet_Release(renderer->getContextManager(), themeSet,0);
	}
	
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
	return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_ClearThemesAndEffects(NXT_HThemeRenderer renderer, int iFlags ) {

	if( !renderer )
	{
		return NXT_Error_BadParam;
	}
    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
	LOGI("[%s %d] (acq=%d)", __func__, __LINE__, ctx->contextIsAcquired);

	// RYU 20130723 set flags.
	renderer->bFlags = iFlags;

	NXT_ThemeRenderer_AquireContext(renderer);

	pthread_mutex_lock(&renderer->themesetLock);

	if( renderer->themeSet != NULL )
	{
		NXT_ThemeSet_Release(renderer->getContextManager(), renderer->themeSet, !ctx->contextIsAcquired);
		renderer->themeSet = NULL;

		renderer->bActiveEffectChanged = 1;
	}

	pthread_mutex_unlock(&renderer->themesetLock);

	NXT_ThemeRenderer_ReleaseContext(renderer, 0);
	return NXT_Error_None;
}

//
//static void setRendererThemeSet(NXT_HThemeRenderer renderer, NXT_HThemeSet themeSet) {
//	
//	if( !renderer ) {
//        // ERROR: null pointer
//        return;
//    }
//	
//    pthread_mutex_lock(&renderer->themesetLock);
//	
//
//	if( renderer->themeSet!=themeSet )
//	{
//		renderer->bEffectChanged = 1;
//		
//		if( renderer->themeSet!=NULL ) {
//			setEffectInternal(renderer, NULL);
//			NXT_ThemeSet_Release(renderer->getContextManager(), renderer->themeSet);
//			renderer->themeSet = NULL;
//		}
//		
//		if( themeSet!=NULL ) {
//			NXT_ThemeSet_Retain(themeSet);
//			themeSet = renderer->themeSet;
//		}
//	}
//	
//	
//    pthread_mutex_unlock(&renderer->themesetLock);
//	
//	
//}

static void setAppliedEffect(NXT_HThemeRenderer renderer,
                             NXT_AppliedEffect* appliedEffect,
                             NXT_EffectType effectType,
                             const char *effectId,  // ID of clip effect, or NULL to clear current effect
                             const char *effectOptions,
                             int clipIndex,         // Index of this clip, from 0
                             int totalClipCount,    // Total number of clips in the project
                             int clipStartTime,     // Start time for this clip
                             int clipEndTime,       // End time for this clip
                             int effectStartTime,   // Start time for this effect (must be >= clipStartTime)
                             int effectEndTime )
{
    LOGI("[%s %d]setAppliedEffect : %s (clip %d/%d : %d -> %d) (fx %d -> %d)", __func__, __LINE__,effectId, clipIndex, totalClipCount, clipStartTime, clipEndTime, effectStartTime, effectEndTime);
    if( appliedEffect==NULL || renderer==NULL ) {
        return;
    }
    
    pthread_mutex_lock(&renderer->themesetLock);
    
	if( effectId == NULL)
		effectId="";
    
    
	if( strcmp(appliedEffect->effectId, effectId)!=0 ) {
		appliedEffect->bEffectChanged = 1;
		strlcpy(appliedEffect->effectId, effectId, sizeof(appliedEffect->effectId));
		LOGI("[%s %d]change Effect : %s", __func__, __LINE__,effectId);
		freeAppliedEffect(appliedEffect);
	}
    
	if( effectOptions == NULL) {
        if( appliedEffect->effectOptionsEncoded ) {
            free(appliedEffect->effectOptionsEncoded);
            appliedEffect->effectOptionsEncoded = NULL;
            appliedEffect->textSerial = (++renderer->masterTextSerial);
        }
    } else {
        if( appliedEffect->effectOptionsEncoded==NULL || strcmp(appliedEffect->effectOptionsEncoded, effectOptions)!=0 ) {
            freeAppliedEffect(appliedEffect);
            appliedEffect->effectOptionsEncoded = (char*)malloc(strlen(effectOptions)+1);
            if( !appliedEffect->effectOptionsEncoded ) {
                LOGE("[%s %d] malloc failure", __func__, __LINE__);
                return;
            }
            strcpy(appliedEffect->effectOptionsEncoded, effectOptions);
            
            parseEffectOptions(appliedEffect);
            appliedEffect->render_item_encoded = 0;
            appliedEffect->bEffectChanged = 1;
            appliedEffect->textSerial = (++renderer->masterTextSerial);

        }
    }
    
    appliedEffect->effectType = effectType;
    
    appliedEffect->clip_index = clipIndex;
    appliedEffect->clip_count = totalClipCount;
    appliedEffect->clipStartTime = clipStartTime;
    appliedEffect->clipEndTime = clipEndTime;
    appliedEffect->effectStartTime = effectStartTime;
    appliedEffect->effectEndTime = effectEndTime;
    
    pthread_mutex_unlock(&renderer->themesetLock);

//    NXT_ThemeRenderer_SetCTS(renderer, renderer->cts);
    LOGD("[%s %d] OUT", __func__, __LINE__);

}

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
                             int effectEndTime ){

	setAppliedEffect(renderer,
                     appliedEffect,
                     effectType,
                     effectId,  // ID of clip effect, or NULL to clear current effect
                     effectOptions,
                     clipIndex,         // Index of this clip, from 0
                     totalClipCount,    // Total number of clips in the project
                     clipStartTime,     // Start time for this clip
                     clipEndTime,       // End time for this clip
                     effectStartTime,   // Start time for this effect (must be >= clipStartTime)
                     effectEndTime);
}


static void autoSelectEffectFromCurrentCTS(NXT_HThemeRenderer renderer) {
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    NXT_AppliedEffect* pActiveEffect = NULL;
    
    int cts = renderer->cts;
    
    if( renderer->transitionEffect.effectId[0] && cts >= renderer->transitionEffect.effectStartTime && cts <= renderer->transitionEffect.effectEndTime ) {
        pActiveEffect = &renderer->transitionEffect;
        LOGD("[%s %d] Transition : %s StartTime:%d EndTime:%d CurCTS:%d", __func__, __LINE__, pActiveEffect->effectId, pActiveEffect->effectStartTime, pActiveEffect->effectEndTime, cts);
    } else if( renderer->clipEffect.effectId[0] && cts >= renderer->clipEffect.effectStartTime && cts <= renderer->clipEffect.effectEndTime ) {
        pActiveEffect = &renderer->clipEffect;
        LOGD("[%s %d] clipEffect : %s StartTime:%d EndTime:%d CurCTS:%d", __func__, __LINE__, pActiveEffect->effectId, pActiveEffect->effectStartTime, pActiveEffect->effectEndTime, cts);
    } else {
        LOGD("[%s %d] StartTime:%d EndTime:%d ClipStartTime:%d ClipEndTime:%d CurCTS:%d", __func__, __LINE__, renderer->transitionEffect.effectStartTime, renderer->transitionEffect.effectEndTime,
             renderer->clipEffect.effectStartTime, renderer->clipEffect.effectEndTime, cts);
        pActiveEffect = NULL;
    }

    if( pActiveEffect!=renderer->pActiveEffect ) {
        renderer->pActiveEffect = pActiveEffect;
        renderer->bActiveEffectChanged = 1;
    }

}

static void calcLocalTimeFromCurrentEffectAndCTS(NXT_HThemeRenderer renderer) {
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    NXT_AppliedEffect* pActiveEffect = renderer->pActiveEffect;
    int cts = renderer->cts;

    if( pActiveEffect!=NULL ) {
        int duration = pActiveEffect->effectEndTime - pActiveEffect->effectStartTime;
        int currenttime = cts - pActiveEffect->effectStartTime;
        renderer->cur_time = currenttime;
        renderer->max_set_time = duration;
        renderer->max_time = duration;
        renderer->settime = ((float)currenttime) / ((float)duration);
        renderer->time = renderer->settime;
        renderer->effectStartTime = pActiveEffect->effectUserStartTime;
        renderer->effectEndTime = pActiveEffect->effectUserEndTime;
    }
    
}


void NXT_ThemeRenderer_SetCTS(NXT_HThemeRenderer renderer, int cts) {
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    renderer->cts = cts;
    renderer->original_cts = cts;

}

void NXT_ThemeRenderer_SetDefaultFrameBuffer(NXT_HThemeRenderer renderer,unsigned int frameBufferNum)
{
    renderer->defaultFrameBufferNum = (GLuint)frameBufferNum;
    NXT_Theme_SetDefaultFrameBufferNum(renderer->getRenderItemManager(), frameBufferNum);
}

void NXT_ThemeRenderer_ClearWatermarkEffect(NXT_HThemeRenderer renderer) {
    NXT_ThemeRenderer_SetWatermarkEffect(renderer, NULL, 0);
}

void NXT_ThemeRenderer_ClearTransitionEffect(NXT_HThemeRenderer renderer) {
    NXT_ThemeRenderer_SetTransitionEffect(renderer, NULL, NULL, 0, 0, 0, 0);
}

void NXT_ThemeRenderer_ClearClipEffect(NXT_HThemeRenderer renderer) {
    NXT_ThemeRenderer_SetClipEffect(renderer, NULL, NULL, 0, 0, 0, 0, 0, 0);
}


void NXT_ThemeRenderer_SetWatermarkEffect(NXT_HThemeRenderer renderer,
                                          const char *effectId,  // ID of clip effect, or NULL to clear current effect
                                          int projectDuration)
{
    char decoded_id[512] = {0};
    char mask[64+1] = "Ax/VXn_zsAiwFi[CITPC;y2c}*0B'S0-7&QznQlMa6U9gmSoighZeC&@$-hAaXiN";
    
    
    size_t idlen = strlen(effectId);
    if( idlen < 8 || idlen-8 >= sizeof(decoded_id) ) {
        NXT_ThemeRenderer_ClearWatermarkEffect(renderer);
        return;
    }
    
    int offsa = effectId[0]^0x3F;
    int offsb = effectId[1]^0x81;
    int i;

    for( i=0; i<idlen-8; i++ ) {
        decoded_id[i] = (effectId[8+i] - ' ' - mask[(offsa+i)%64] + effectId[(offsb+i)%8] + 188) % 94 + ' ';
    }
    decoded_id[i] = 0;
    
    LOGI("[%s %d]NXT_ThemeRenderer_SetWatermarkEffect : %s : %s ", __func__, __LINE__,decoded_id,effectId);
    setAppliedEffect(renderer, &renderer->watermarkEffect, NXT_EffectType_Title, decoded_id, NULL, 0, 0, 0, projectDuration, 0, projectDuration);
}

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
    LOGI("[%s %d]NXT_ThemeRenderer_SetClipEffect : %s", __func__, __LINE__,effectId);
    setAppliedEffect(renderer, &renderer->clipEffect, NXT_EffectType_Title, effectId, pEffectOptions, clipIndex, totalClipCount, clipStartTime, clipEndTime, effectStartTime, effectEndTime);
}

void NXT_ThemeRenderer_SetTransitionEffect(NXT_HThemeRenderer renderer,
                                           const char *effectId,  // ID of transition effect, or NULL to clear current effect
                                           const char *pEffectOptions,
                                           int clipIndex,         // Index of the transition's clip (the clip before the transition), from 0
                                           int totalClipCount,    // Total number of clips in the project
                                           int effectStartTime,   // Start time for this effect
                                           int effectEndTime )    // End time for this effect
{
    LOGI("[%s %d]NXT_ThemeRenderer_SetTransitionEffect : %s", __func__, __LINE__,effectId);
    setAppliedEffect(renderer, &renderer->transitionEffect, NXT_EffectType_Transition, effectId, pEffectOptions, clipIndex, totalClipCount, 0, 0, effectStartTime, effectEndTime);
}


/*
void NXT_ThemeRenderer_SetEffect(NXT_HThemeRenderer renderer, const char *effectId, int clipIndex, int totalClipCount)
{
    LOGI("[%s %d]EffectID : %s", __func__, __LINE__,effectId);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
	
    renderer->clip_index = clipIndex;
    renderer->clip_count = totalClipCount;
	
    pthread_mutex_lock(&renderer->themesetLock);

	if( effectId == NULL)
		effectId="";

	if( strcmp(renderer->effectId, effectId)!=0 ) {

		renderer->bEffectChanged = 1;
		strlcpy(renderer->effectId, effectId, sizeof(renderer->effectId));
		LOGI("[%s %d]change Effect : %s", __func__, __LINE__,effectId);
	}

	
    pthread_mutex_unlock(&renderer->themesetLock);
	

}
*/
/*
void NXT_ThemeRenderer_SetTime(NXT_HThemeRenderer renderer, float time) {
    if( !renderer ) {
        // ERROR: null pointer
        return;
    } 
    renderer->settime = time;
    renderer->time = time;
    renderer->cur_time = (int)(100000.0 * time);
    renderer->max_set_time = 100000;
    renderer->max_time = 100000;
    LOGI("[%s %d] (THEMECHK) set time: %f", __func__, __LINE__, renderer->time );
}

void NXT_ThemeRenderer_SetTimeMS(NXT_HThemeRenderer renderer, int currenttime, int duration) {
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    renderer->cur_time = currenttime;
    renderer->max_set_time = duration;
    renderer->max_time = duration;
    renderer->settime = ((float)currenttime) / ((float)duration);
    renderer->time = renderer->settime;
    
    LOGI("[%s %d] (THEMECHK) set time MS(%d,%d): %f", __func__, __LINE__, currenttime, duration, renderer->time );
}*/

NXT_Error NXT_ThemeRenderer_GLWaitToFinishRendering(NXT_HThemeRenderer renderer)
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
    }
    
    NXT_BEGIN_PROFILE("glFinish");
    glFinish();
    NXT_END_PROFILE();
    
    CHECK_GL_ERROR();
    
    return NXT_Error_None;    
}

NXT_Error NXT_ThemeRenderer_GetPixels(NXT_HThemeRenderer renderer,
                                      int *pWidth,
                                      int *pHeight,
                                      int *pSize,
                                      unsigned char **ppData,
                                      unsigned int bCallerOwnsMemory)
{
    if( !renderer || !pWidth || !pHeight || !ppData ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    
    if( !renderer->bDidRender ) {
        LOGE("[%s %d] No previous rendering pass; cannot get pixels", __func__, __LINE__);
        return NXT_Error_InvalidState;
    } else if( !renderer->bInitialized ) {
        LOGE("[%s %d] ThemeRenderer not initialized", __func__, __LINE__);
        return NXT_Error_InvalidState;
    }
    
    CHECK_GL_ERROR();
    
    renderer->bCalledGetPixels = 1;
    
    unsigned int reqHeight = renderer->view_height;
    
    if( renderer->outputType == NXT_RendererOutputType_Y2CrA_8888 ) {
        reqHeight /= 2;
    }
    
    unsigned int reqPixelDataSize;
    
    reqPixelDataSize = renderer->view_width * reqHeight * 4;
    
    unsigned char *pReadPixelData;
    
    if( bCallerOwnsMemory ) {
        pReadPixelData = (unsigned char *)malloc(reqPixelDataSize);
        if( !pReadPixelData ) {
            LOGE("[%s %d] malloc failure", __func__, __LINE__);
            return NXT_Error_Malloc;
        }
    } else {
    
        if( renderer->pReadPixelData && reqPixelDataSize!=renderer->readPixelDataSize ) {
            free(renderer->pReadPixelData);
            renderer->pReadPixelData = NULL;
            renderer->readPixelDataSize = 0;
        }

        if( !renderer->pReadPixelData ) {
            renderer->readPixelDataSize = reqPixelDataSize;
            renderer->pReadPixelData = (unsigned char *)malloc(renderer->readPixelDataSize);
        }
        
        if( !renderer->pReadPixelData ) {
            LOGE("[%s %d] malloc failure", __func__, __LINE__);
            return NXT_Error_Malloc;
        }
        
        pReadPixelData = renderer->pReadPixelData;
    }
    
    NXT_BEGIN_PROFILE("glReadPixels");
        glReadPixels(0,renderer->view_height-reqHeight,renderer->view_width,reqHeight,GL_RGBA,GL_UNSIGNED_BYTE, pReadPixelData);
    CHECK_GL_ERROR();
    NXT_END_PROFILE();


    *ppData = pReadPixelData;
    *pWidth = renderer->view_width;
    *pHeight = renderer->view_height;
    *pSize = reqPixelDataSize;
    
    return NXT_Error_None;
}



NXT_Error NXT_ThemeRenderer_GetPixels2(NXT_HThemeRenderer renderer,
                                      int *pWidth,
                                      int *pHeight,
                                      int *pSize,
                                      unsigned char *pData)
{
    if( !renderer || !pWidth || !pHeight || !pData) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    
    if( !renderer->bDidRender ) {
        LOGE("[%s %d] No previous rendering pass; cannot get pixels", __func__, __LINE__);
        return NXT_Error_InvalidState;
    } else if( !renderer->bInitialized ) {
        LOGE("[%s %d] ThemeRenderer not initialized", __func__, __LINE__);
        return NXT_Error_InvalidState;
    }
    
    renderer->bCalledGetPixels = 1;
    
    unsigned int reqHeight = renderer->view_height;
    
    if( renderer->outputType == NXT_RendererOutputType_Y2CrA_8888 ) {
        reqHeight /= 2;
    }
    
    unsigned int reqPixelDataSize;
    
    reqPixelDataSize = renderer->view_width * reqHeight * 4;
   
    NXT_BEGIN_PROFILE("glReadPixels");
        glReadPixels(0,renderer->view_height-reqHeight,renderer->view_width,reqHeight,GL_RGBA,GL_UNSIGNED_BYTE, pData);
    CHECK_GL_ERROR();
    NXT_END_PROFILE();

    *pWidth = renderer->view_width;
    *pHeight = renderer->view_height;
    *pSize = reqPixelDataSize;
    
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_AquireContext(NXT_HThemeRenderer renderer) {
    LOGD("[%s %d] Begin acquire context", __func__, __LINE__);
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    
#ifdef KEEP_CONTEXT_DURING_EXPORT
    if( renderer->bExportMode )
        return NXT_Error_None;
#endif

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    
    if( ctx->contextIsAcquired ) {
        if( pthread_equal(pthread_self(), ctx->contextOwner) ) {
            ctx->contextIsAcquired++;
            LOGE("[%s %d] Attempt to acquire context while already aquired in same thread; would deadlock.(contextIsAcquired:%d)", __func__, __LINE__, ctx->contextIsAcquired);
            return NXT_Error_None;
        }
    }
    pthread_mutex_lock(&ctx->contextLock);
    pthread_t exContextOwner = ctx->contextOwner;
    ctx->contextOwner = pthread_self();
    ctx->contextIsAcquired = 1;
    
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( renderer->nativeExportSurface != EGL_NO_SURFACE ) {
//            dumpSurfaceInfo(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface, "AquireContext/nativeExportSurface");
            renderer->view_width = renderer->exportSurfaceWidth;
            renderer->view_height = renderer->exportSurfaceHeight;
            if(eglMakeCurrent(renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface, renderer->nativeExportSurface, renderer->getContext()) != EGL_TRUE) {
                CHECK_EGL_ERROR();
                LOGE("[%s %d] eglMakeCurrent failed(0x%X)", __func__, __LINE__, LOGPTR(exContextOwner));
                ctx->contextIsAcquired = 0;
                pthread_mutex_unlock(&ctx->contextLock);
                return NXT_Error_GLContextErr;
            }
            CHECK_EGL_ERROR();
        } else if( renderer->nativeWindowSurface != EGL_NO_SURFACE ) {
//            dumpSurfaceInfo(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, "AquireContext/nativeWindowSurface");
            renderer->view_width = renderer->nativeWindowWidth;
            renderer->view_height = renderer->nativeWindowHeight;
            if(eglMakeCurrent( renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, renderer->nativeWindowSurface, renderer->getContext()) != EGL_TRUE ) {
                CHECK_EGL_ERROR();
                LOGE("[%s %d] eglMakeCurrent failed(0x%X)", __func__, __LINE__, LOGPTR(exContextOwner));
                ctx->contextIsAcquired = 0;
                pthread_mutex_unlock(&ctx->contextLock);
                return NXT_Error_GLContextErr;
            }
            CHECK_EGL_ERROR();
//            dumpSurfaceInfo(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, "AquireContext+MakeCurrent/nativeWindowSurface");
        } else {
            LOGD("[%s %d] set DummySurface", __func__, __LINE__);
            ctx->contextIsAcquired = 1;
             if(eglMakeCurrent(ctx->eglDisplay, ctx->getDummySurface(), ctx->getDummySurface(), ctx->getContext()) != EGL_TRUE ) {
                 CHECK_EGL_ERROR();
                 LOGE("[%s %d] eglMakeCurrent failed(0x%X)", __func__, __LINE__, LOGPTR(exContextOwner));
                 ctx->contextIsAcquired = 0;
                 pthread_mutex_unlock(&ctx->contextLock);
                 return NXT_Error_GLContextErr;
            }
            // pthread_mutex_unlock(&ctx->contextLock);
            return NXT_Error_None;
        }
        CHECK_EGL_ERROR();
#endif
#ifdef USE_PLATFORM_SUPPORT
        NEXTHEMERENDERER_SET_CONEXTEXT_MODE contextMode = SET_CONTEXT_MODE_PREVIEW;
        if ( renderer->rendererType == NXT_RendererType_InternalExportContext ) {
            contextMode = SET_CONTEXT_MODE_EXPORT;
        }
        if (NXT_Error_None != NXT_Platform_SetCurrentInternalContext(renderer, contextMode)) {
            LOGE("[%s %d] setCurrentInternalContext failed", __func__, __LINE__);
            ctx->contextIsAcquired = 0;
            pthread_mutex_unlock(&ctx->contextLock);
            return NXT_Error_GLContextErr;
        }
#endif
        glViewport(0,0,renderer->view_width,renderer->view_height);
        CHECK_GL_ERROR();
        ////////////////////////////// MATTHEW 2015-08-11 ////////////////////////////////////
        if( !renderer->bInitialized ) {
            NXT_Error result = initRenderer( renderer );
            if( result!=NXT_Error_None || !renderer->bInitialized ) {
                LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
                return result;
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////
    }
    renderer->original_cts = renderer->cts;
    LOGD("[%s %d] End acquire context", __func__, __LINE__);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_SwapBuffers(NXT_HThemeRenderer renderer)
{
    //LOGI("[%s %d] Begin swap Buffer: rendererType(%d) surface(0x%X) display(0x%X)", __func__, __LINE__, renderer->rendererType, LOGPTR(renderer->nativeExportSurface), LOGPTR(renderer->getContextManager()->eglDisplay));

    if( !renderer->bIsInternalContext ) {
        LOGI("[%s %d] SKIP swap Buffer: not internal context", __func__, __LINE__ );
        return NXT_Error_None;
    }
        
#ifdef ANDROID
    if( renderer->bExportMode ) {
        if( eglSwapBuffers( renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface) != EGL_TRUE ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglSwapBuffers failed; surface(0x%X) display(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeExportSurface), LOGPTR(renderer->getContextManager()->eglDisplay) );
            return NXT_Error_OpenGLFailure;
        }
    } 
    else 
    {
        if( eglSwapBuffers( renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface) != EGL_TRUE ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglSwapBuffers failed; surface(0x%X) display(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeWindowSurface), LOGPTR(renderer->getContextManager()->eglDisplay) );
            return NXT_Error_OpenGLFailure;
        }
    }
    CHECK_EGL_ERROR();
    LOGI("[%s %d] End swap Buffer: rendererType(%d) surface(0x%X) display(0x%X)", __func__, __LINE__, renderer->rendererType, LOGPTR(renderer->nativeExportSurface), LOGPTR(renderer->getContextManager()->eglDisplay));
#endif
    
#ifdef USE_PLATFORM_SUPPORT
    NEXTHEMERENDERER_PRESENT_MODE presentMode = renderer->bExportMode ? PRESENT_MODE_EXPORT : PRESENT_MODE_PREVIEW;
    if ( NXT_Error_None != NXT_Platform_PresentBuffer( renderer, presentMode ) ) {
        LOGE("[%s %d] presentBuffer failed; ", __func__, __LINE__ );
        return NXT_Error_OpenGLFailure;
    }
    CHECK_GL_ERROR();
    LOGI("[%s %d] End swap Buffer: rendererType(%d)", __func__, __LINE__, renderer->rendererType);
#endif
    return NXT_Error_None;
}


NXT_Error NXT_ThemeRenderer_ReleaseContext(NXT_HThemeRenderer renderer, unsigned int bSwapBuffers ) {
    //LOGI("[%s %d] Begin release context : bSwapBuffers(%d) rendererType(%d) surface(0x%X) display(0x%X)", __func__, __LINE__, bSwapBuffers, LOGPTR(renderer->rendererType), LOGPTR(renderer->nativeExportSurface), LOGPTR(renderer->getContextManager()->eglDisplay));

    NXT_Error err = NXT_Error_None;

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
       
#ifdef KEEP_CONTEXT_DURING_EXPORT
    if( renderer->bExportMode )
        return NXT_Error_None;
#endif

    if( !ctx->contextIsAcquired )
        return NXT_Error_None;
#ifdef ANDROID
    if(bSwapBuffers) {

        if(renderer->bIsInternalContext) {
            LOGD("[%s %d] Internal context; surface(0x%X) display(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeExportSurface), LOGPTR(renderer->getContextManager()->eglDisplay) );
            
            if( renderer->bExportMode ) {
                //dumpSurfaceInfo(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, "ReleaseContext/nativeWindowSurface");
                if( eglSwapBuffers( renderer->getContextManager()->eglDisplay, renderer->nativeExportSurface) != EGL_TRUE ) {
                    CHECK_EGL_ERROR();
                    LOGE("[%s %d] eglSwapBuffers failed; surface(0x%X) display(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeWindowSurface), LOGPTR(renderer->getContextManager()->eglDisplay) );
                    err = NXT_Error_OpenGLFailure;
                }
                CHECK_EGL_ERROR();
				
            } else if( renderer->nativeWindowSurface ) {
                //dumpSurfaceInfo(renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface, "ReleaseContext/nativeWindowSurface");
                if( eglSwapBuffers( renderer->getContextManager()->eglDisplay, renderer->nativeWindowSurface) != EGL_TRUE ) {
                    CHECK_EGL_ERROR();
                    LOGE("[%s %d] eglSwapBuffers failed; surface(0x%X) display(0x%X)", __func__, __LINE__, LOGPTR(renderer->nativeWindowSurface), LOGPTR(renderer->getContextManager()->eglDisplay) );
                    err = NXT_Error_OpenGLFailure;
                }
                CHECK_EGL_ERROR();
            }
        }
    }
    
    if(--ctx->contextIsAcquired <= 0){
    	eglMakeCurrent(renderer->getContextManager()->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        CHECK_EGL_ERROR();
    	pthread_mutex_unlock(&ctx->contextLock);
    }    
#endif  // ANDROID
    
#ifdef USE_PLATFORM_SUPPORT
    if( renderer->bIsInternalContext ) {
        if( bSwapBuffers ) {
            NEXTHEMERENDERER_PRESENT_MODE presentMode = renderer->bExportMode ? PRESENT_MODE_EXPORT : PRESENT_MODE_PREVIEW;
            err = NXT_Platform_PresentBuffer( renderer, presentMode );
        }
    }
    if(--ctx->contextIsAcquired <= 0){
        if( renderer->bIsInternalContext ) {
            /* err = */ NXT_Platform_SetCurrentInternalContext(renderer, SET_CONTEXT_MODE_RELEASE);
        }
        pthread_mutex_unlock(&ctx->contextLock);
    }
#endif
    LOGD("[%s %d] End release context", __func__, __LINE__);
	return err;
}

static void parseFastPreviewOptions( NXT_HThemeRenderer renderer, const char *pOptions ) {
    

    int customlut_power = 0;
    int customlut_clip = 0;
    LOGI("[%s %d] parseFastPreviewOptions : '%s'", __func__, __LINE__, pOptions);
    
    const char *pChar = pOptions;
    
    while( *pChar ) {
        
        int value = 1;
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        char tag[32] = {0};
        int taglen = 0;
        
        while( (*pChar >='a' && *pChar <='z') || (*pChar >='A' && *pChar <='Z') || (*pChar >='0' && *pChar <='9') || *pChar=='_' ) {
            if( taglen < sizeof(tag)-1 ) {
                tag[taglen++]=*pChar++;
            }
        }
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        if( *pChar=='=' ) {
            pChar++;
            
            if( *pChar=='{' ) {
                pChar++;
                
                float params[32] = {0};
                int paramCount;
                
                do {
                    
                    const char *st = pChar;
                
                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    float f = strtof(pChar, (char**)&pChar);
                    
                    if( paramCount < 32 ) {
                        params[paramCount++] = f;
                    }
                    
                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    while( *pChar == ',' )
                        pChar++;

                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    if( st==pChar ) {
                        // Malformed
                        break;
                    }
                    
                } while ( *pChar && *pChar!='}' );
                
                if( *pChar == '}' )
                    pChar++;
                
                if( paramCount > 0 ) {
                    handleFastFloatArrayPreviewOption(renderer, tag, paramCount, params);
                }
                continue;
               
            } else {
            
                while( *pChar == ' ' || *pChar == '\t' )
                    pChar++;
                
                value = 0;
                
                int sign = 1;
                
                if( *pChar=='-' ) {
                    pChar++;
                    sign = -1;
                }
                
                while( (*pChar >='0' && *pChar <='9') ) {
                    value *= 10;
                    value += ((*pChar++)-'0');
                }
                
                value *= sign;
                
            }
        }
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        if( taglen < 1 )
            break;
        
        handleFastPreviewOption(renderer, tag, value, customlut_clip, customlut_power);
    }

    if(customlut_clip && customlut_power){

        NXT_TextureInfo* ptex = NULL;

        if(customlut_clip < 0){

            if(customlut_clip == -1){

                ptex = &renderer->getRawVideoSrc(NXT_TextureID_Video_1);
            }
            else if(customlut_clip == -2){

                ptex = &renderer->getRawVideoSrc(NXT_TextureID_Video_2);
            }
            if(ptex->track_id_ <= 0)
                ptex = NULL;
        }
        else{

            int slot = renderer->getRawVideoSrcForUID(customlut_clip);
            ptex = slot?&renderer->getRawVideoSrc(slot):NULL;
            if(ptex == NULL){

                ptex = &renderer->getRawVideoSrc(NXT_TextureID_Video_1);
                if(ptex->track_id_ != customlut_clip){

                    ptex = NULL;
                }
            }
            if(ptex == NULL){

                ptex = &renderer->getRawVideoSrc(NXT_TextureID_Video_2);
                if(ptex->track_id_ != customlut_clip){

                    ptex = NULL;
                }
            }
        }

        if(ptex){

            ptex->custom_lut_power = customlut_power;
            renderer->applyLUT(*ptex);
        }
    }
    LOGI("[%s %d] DONE parseFastPreviewOptions", __func__, __LINE__);
    
}


static void handleFastFloatArrayPreviewOption( NXT_HThemeRenderer renderer, char* optionName, int paramCount, float* params ) {
    LOGI("[%s %d] handleFastFloatArrayPreviewOption('%s',%d)", __func__, __LINE__, optionName, paramCount);
    if( strcasecmp(optionName, "matrix")==0 ) {
//        renderer->getVideoSrc(0).brightness = (float)value/255.0f;
    }
}


static void handleFastPreviewOption( NXT_HThemeRenderer renderer, char* optionName, int value, int& customlut_clip, int& customlut_power) {
    
    LOGI("[%s %d] handleFastPreviewOption('%s',%d)", __func__, __LINE__, optionName, value);

    NXT_TextureInfo& video = renderer->getVideoSrc(0);
    
    if( strcasecmp(optionName, "brightness")==0 ) {
        video.brightness = (float)value/255.0f;
    } else if( strcasecmp(optionName, "contrast")==0 ) {
        video.contrast = (float)value/255.0f;
    } else if( strcasecmp(optionName, "saturation")==0 ) {
        video.saturation = (float)value/255.0f;
    } else if( strcasecmp(optionName, "hue")==0 ) {
        video.hue = (float)value/180.0f;
    } else if( strcasecmp(optionName, "adj_brightness")==0 ) {

        renderer->setBrightness(value);
    } else if( strcasecmp(optionName, "adj_contrast")==0 ) {

        renderer->setContrast(value);
    } else if( strcasecmp(optionName, "adj_saturation")==0 ) {

        renderer->setSaturation(value);
    } else if( strcasecmp(optionName, "tintColor")==0 ) {
        video.tintColor = value;
    } else if( strcasecmp(optionName, "left")==0 ) {
        video.left = value;
        video.bHasInputRect = 1;
    } else if( strcasecmp(optionName, "top")==0 ) {
        video.top = value;
        video.bHasInputRect = 1;
    } else if( strcasecmp(optionName, "right")==0 ) {
        video.right = value;
        video.bHasInputRect = 1;
    } else if( strcasecmp(optionName, "bottom")==0 ) {
        video.bottom = value;
        video.bHasInputRect = 1;
    } else if( strcasecmp(optionName, "nofx")==0 ) {
        renderer->bNoFx = value?1:0;
    } else if( strcasecmp(optionName, "cts")==0 ) {
        renderer->cts = value;
    } else if( strcasecmp(optionName, "swapv")==0 ) {
        renderer->bSwapV = value?1:0;
    } else if( strcasecmp(optionName, "video360flag")==0 ) {

        video.is360video = value;
        LOGI("[%s %d] handleFastPreviewOption is360video=%d", __func__, __LINE__,value);
    } else if( strcasecmp(optionName, "video360_horizontal")==0 ) {

        video.moveX_value = value;
        LOGI("[%s %d] handleFastPreviewOption video360_horizontal=%d", __func__, __LINE__,value);
    } else if( strcasecmp(optionName, "video360_vertical")==0 ) {

        video.moveY_value = value;
        LOGI("[%s %d] handleFastPreviewOption video360_vertical=%d", __func__, __LINE__,value);
	} else if( strcasecmp(optionName, "adj_vignette")==0 ) {
        renderer->setVignette(value);
    } else if( strcasecmp(optionName, "adj_vignetteRange")==0 ) {
        renderer->setVignetteRange(value);
	} else if (strcasecmp(optionName, "adj_sharpness") == 0) {
        renderer->setSharpness(value);
	} else if(strcasecmp(optionName, "customlut_power") == 0){

        customlut_power = value;
    }
    else if (strcasecmp(optionName, "customlut_clip") == 0)
    {

        customlut_clip = value;
    }
    
}

inline NXT_Error NXT_ThemeRenderer_GLDraw_Internal(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark, int block_arrange_surface_resource)
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    
    if( !ctx->contextIsAcquired && renderer->bIsInternalContext ) {
        LOGW("[%s %d] WARNING: Attempt to render with no context; BYPASSING", __func__, __LINE__);
        return NXT_Error_None;
    }

    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO GL CONTEXT", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }

	renderer->lut_texture_ = 0;
    renderer->disable_watermark = disable_watermark;

    if(block_arrange_surface_resource == 0){

        renderer->arrangeSurfaceResource();
    }
    

    CHECK_GL_ERROR();
#ifdef ANDROID	
    CHECK_EGL_ERROR();
#endif
    NXT_Error result = NXT_Error_None;

    if(NXT_RendererOutputType_JUST_CLEAR == outputType)
    	glClear(GL_COLOR_BUFFER_BIT);
	else 
		result = NXT_ThemeRenderer_GLDraw_Main(renderer, outputType);
	
    CHECK_GL_ERROR();
#ifdef ANDROID
    CHECK_EGL_ERROR();
#endif
#ifdef USE_PLATFORM_SUPPORT
    NXT_Platform_AfterDraw(renderer);
#endif
    return result;
    
    
}

NXT_Error NXT_ThemeRenderer_fastOptionPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, const char *pOptions) {

    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    LOGI("[%s %d] fastOptionPreview('%s')", __func__, __LINE__, pOptions);
    
    renderer->pFastPreviewOptions = pOptions;
    parseFastPreviewOptions( renderer, pOptions );
    
    NXT_Error result = NXT_ThemeRenderer_GLDraw_Internal(renderer,outputType, 0, 1);
    renderer->pFastPreviewOptions = NULL;
	renderer->bNoFx = 0;
    renderer->cts = renderer->original_cts;
    renderer->bSwapV = 0;

    return result;
}

NXT_Error NXT_ThemeRenderer_GLDraw(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark){

    return NXT_ThemeRenderer_GLDraw_Internal(renderer, outputType, disable_watermark, 0);
}

NXT_Error NXT_ThemeRenderer_AddFastPreviewFrame(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark, int iTime)
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    
    if( !ctx->contextIsAcquired && renderer->bIsInternalContext ) {
        LOGW("[%s %d] WARNING: Attempt to render with no context; BYPASSING", __func__, __LINE__);
        return NXT_Error_None;
    }
    
    renderer->disable_watermark = disable_watermark;
    
    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();
    // setHQScale(renderer, renderer->scaler_src_w_, renderer->scaler_src_h_, renderer->scaler_dst_w_, renderer->scaler_dst_h_, outputType);

    NXT_Error result = NXT_ThemeRenderer_Add_FastPreviewFrame(renderer, outputType, iTime);
    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();
    return result;
}

NXT_Error NXT_ThemeRenderer_GLDrawForFastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark, int iTime)
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    
    if( !ctx->contextIsAcquired && renderer->bIsInternalContext ) {
        LOGW("[%s %d] WARNING: Attempt to render with no context; BYPASSING", __func__, __LINE__);
        return NXT_Error_None;
    }
    
    renderer->disable_watermark = disable_watermark;
    
    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();
    // setHQScale(renderer, renderer->scaler_src_w_, renderer->scaler_src_h_, renderer->scaler_dst_w_, renderer->scaler_dst_h_, outputType);
    
    NXT_Error result = NXT_ThemeRenderer_GLDraw_Main_For_FastPreview(renderer, outputType, iTime);
    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();
    return result;
}

static void setViewportForRGB(NXT_HThemeRenderer renderer){
	if (renderer->view_height == 1088)
	{
		glViewport(0, 8, renderer->view_width, renderer->view_height);
	}
	else if (renderer->view_height == 736)
	{
		glViewport(0, 16, renderer->view_width, renderer->view_height);
	}
	else if (renderer->view_height == 544)
	{
		glViewport(0, 4, renderer->view_width, renderer->view_height);
	}
	else if (renderer->view_height == 368)
	{
		glViewport(0, 8, renderer->view_width, renderer->view_height);
	}
	else
	{
		glViewport(0, 0, renderer->view_width, renderer->view_height);
	}
}

void NXT_ThemeRenderer_::setGLStateForRGBAOutput(){

    colorMask[0] = GL_TRUE;
    colorMask[1] = GL_TRUE;
    colorMask[2] = GL_TRUE;
    colorMask[3] = GL_TRUE;

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    CHECK_GL_ERROR();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    CHECK_GL_ERROR();

    outputType = NXT_RendererOutputType_RGBA_8888;
    channelFilter = NXT_ChannelFilter_ALL;
}

void NXT_ThemeRenderer_::setGLRenderStart(){

    resetCurrentTrackID();
	transform = NXT_Matrix4f_Identity();
	texture_transform = NXT_Matrix4f_Identity();
	mask_transform = NXT_Matrix4f_Identity();
	mask_sample_transform = NXT_Matrix4f_Identity();
	proj = NXT_Matrix4f_Identity();
	colorMask[0] = GL_TRUE;
	colorMask[1] = GL_TRUE;
	colorMask[2] = GL_TRUE;
	colorMask[3] = GL_TRUE;

	glDisable(GL_DEPTH_TEST);
	CHECK_GL_ERROR();
	glDepthMask(GL_TRUE);
	CHECK_GL_ERROR();
	glDisable(GL_CULL_FACE);
	CHECK_GL_ERROR();
}

static void renderSimpleBoxForDebug(NXT_HThemeRenderer renderer, int dbg_pos){

	NXT_ShaderProgram_Solid* sp = &renderer->getContextManager()->solidShaderRGB;

#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pSolidShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_color);                        CHECK_GL_ERROR();

	GLfloat vertexCoord[4][8] = {
		{
			-1.0, 1.0,
			0.0, 1.0,
			-1.0, 0.0,
			0.0, 0.0,
		},
		{
			0.0, 1.0,
			1.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,
		},
		{
			0.0, 0.0,
			1.0, 0.0,
			0.0, -1.0,
			1.0, -1.0,
		},
		{
			-1.0, 0.0,
			0.0, 0.0,
			-1.0, -1.0,
			0.0, -1.0,
		},
	};

	glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord[dbg_pos % 4]);
	CHECK_GL_ERROR();

	NXT_Vector4f color;

	color.e[0] = 1.0f;
	color.e[1] = 1.0f;
	color.e[2] = 1.0f;
	color.e[3] = 1.0f;
        
    GLfloat vertexColor[16];
    memcpy(vertexColor+0, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+4, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+8, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+12, color.e, sizeof(GLfloat)*4);

    NXT_Matrix4f mvp = NXT_Matrix4f_Identity();
    glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
    glUniform1f(sp->u_alpha, 1.0f);                          			CHECK_GL_ERROR();
    
    glVertexAttribPointer(sp->a_color,4,GL_FLOAT,0,0,vertexColor );
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void renderSimplePlane(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse){
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);							CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0,
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	if (reverse){
		lowy = 1.0f;
		highy = 0.0f;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}


static void renderSimplePlaneUserVignette(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse){
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();
       
	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);							CHECK_GL_ERROR();
    glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
    
    glUniform1f(sp->u_vignette, 1.0f - (renderer->final_vignette_ * 0.01f));                             CHECK_GL_ERROR();
    glUniform1f(sp->u_vignetteRange, renderer->final_vignetteRange_ * 0.01f);
    CHECK_GL_ERROR();

    GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0,
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	if (reverse){
		lowy = 1.0f;
		highy = 0.0f;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void renderSimplePlaneSharpness(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse){
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();
       
	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);							CHECK_GL_ERROR();
    glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
    glUniform1f(sp->u_sharpness, renderer->final_sharpness_ * 0.01f);                   CHECK_GL_ERROR();
    glUniform2f(sp->u_resolution, renderer->view_width, renderer->view_height);       CHECK_GL_ERROR();
        
    GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0,
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	if (reverse){
		lowy = 1.0f;
		highy = 0.0f;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void renderSimplePlaneWithColorconv(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse){
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
    glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
    renderer->pTexturedShaderActive = sp;

    glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
    glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, tex_id);                           CHECK_GL_ERROR();
    glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();

    float brightness = (float)renderer->final_brightness_ / 255.0f;
    float contrast = (float)renderer->final_contrast_ / 255.0f;
    float saturation = (float)renderer->final_saturation_ / 255.0f;
    NXT_Matrix4f colorconv;
    colorconv = NXT_ColorAdjustRGB(brightness, contrast, saturation, NXT_Vector4f(0, 0, 0, 1));
    glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();

    GLfloat vertexCoord[] = {
        -1.0, 1.0,
        1.0, 1.0,
        -1.0, -1.0,
        1.0, -1.0,
    };
    float lowx = 0.0f;
    float lowy = 0.0f;
    float highx = 1.0f;
    float highy = 1.0f;
    if (reverse){
        lowy = 1.0f;
        highy = 0.0f;
    }
    GLfloat texCoords[] = {
        lowx, lowy,
        highx, lowy,
        lowx, highy,
        highx, highy
    };

    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
    CHECK_GL_ERROR();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR();
}

static void renderSimplePlaneForLUT(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, GLuint tex_id_for_lut, int reverse, float realX, float realY, float half_pixel_size_x, float half_pixel_size_y, float alpha){
#ifdef LOAD_SHADER_ON_TIME
	NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);							CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id_for_lut);			CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSamplerY, 1);                           CHECK_GL_ERROR();
	glUniform1f(sp->u_realX, realX);
	glUniform1f(sp->u_realY, realY);
    glUniform1f(sp->u_alpha, alpha);
	GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0,
	};
	float lowx = 0.0f + half_pixel_size_x;
	float lowy = 0.0f + half_pixel_size_y;
	float highx = 1.0f - half_pixel_size_x;
	float highy = 1.0f - half_pixel_size_y;
	if (reverse){
		float tmp = lowy;
		lowy = highy;
		highy = tmp;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void renderSimplePlaneForTestFilter(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, NXT_RenderTarget* psrc, int reverse){
#ifdef LOAD_SHADER_ON_TIME
	NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, psrc->target_texture_);							CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	glUniform2f(sp->u_realX, psrc->width_, psrc->height_);
//	glUniform1f(sp->u_realY, (float)( getTickCount() % 3000 )/1000.0f);
	GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0,
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	if (reverse){
		lowy = 1.0f;
		highy = 0.0f;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void renderSimplePlaneForTestFilter_Rect(NXT_HThemeRenderer renderer, NXT_ShaderProgram_Textured *sp, GLuint tex_id, int reverse, float x, float y, float w, float h){
#ifdef LOAD_SHADER_ON_TIME
	NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else 
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pTexturedShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);                        CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_id);							CHECK_GL_ERROR();
	glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	glUniform2f(sp->u_realX, w, h);
	//	glUniform1f(sp->u_realY, (float)( getTickCount() % 3000 )/1000.0f);
	float left_x = 2.0f * x / (float)renderer->view_width - 1.0f;
	float right_x = 2.0f * (x + w) / (float)renderer->view_width - 1.0f;
	float top_y = 2.0f * y / (float)renderer->view_height - 1.0f;
	float bottom_y = 2.0f * (y + h) / (float)renderer->view_height - 1.0f;
	GLfloat vertexCoord[] = {
		left_x, top_y,
		right_x, top_y,
		left_x, bottom_y,
		right_x, bottom_y,
	};
	float lowx = 0.0f;
	float lowy = 0.0f;
	float highx = 1.0f;
	float highy = 1.0f;
	if (reverse){
		lowy = 1.0f;
		highy = 0.0f;
	}
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};

	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

static void convertColorspace(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
    
	renderer->outputType = outputType;
    if(outputType == NXT_RendererOutputType_RGBA_8888 && renderer->pmain_render_target_ == NULL)
        return;
    CHECK_GL_ERROR();
	NXT_ThemeRenderer_SetRenderTarget(renderer,NULL);
	glEnable(GL_BLEND);
	CHECK_GL_ERROR();
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR();
	int reverse = 0;
	if (renderer->down_scale_info_.size() > 0)
		reverse = renderer->down_scale_info_.size() % 2;
	else
		reverse = 1;

	GLuint tex_id = renderer->pmain_render_target_?renderer->pmain_render_target_->target_texture_:0;
	NXT_RenderTarget* prender_target = NULL;

//If we render scene on to the framebuffer directly,
	if(tex_id == 0){

//if output format is same with the framebuffer, we don't need to something...
		if(outputType == NXT_RendererOutputType_RGBA_8888)
			return;

		prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 0, 0, 0);
		tex_id = prender_target->target_texture_;
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, renderer->view_width, renderer->view_height);
	}

	switch (outputType){
	case NXT_RendererOutputType_RGBA_8888:
	{
		setViewportForRGB(renderer);
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassthroughRGB, tex_id, reverse);
	}
	break;
    case NXT_RendererOutputType_RGBA_8888_yflip:
    {
        setViewportForRGB(renderer);
        renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassthroughRGB, tex_id, 0);
    }
    break;
	case NXT_RendererOutputType_YUVA_8888:
	{
		NXT_BEGIN_PROFILE("Render for output (YUVA_8888)");

		glViewport(0, 0, renderer->view_width, renderer->view_height);
		glScissor(0, 0, renderer->view_width, renderer->view_height);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0, 0.5, 0.5, 1.0);
		CHECK_GL_ERROR();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		CHECK_GL_ERROR();
		
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_ALL], tex_id, reverse);

		NXT_END_PROFILE();
	}
	break;
	case NXT_RendererOutputType_Y2CrA_8888:
	{
		LOGI("[%s %d] GLRenderAndGetPixels/Y2CrA8888 (%d x %d)", __func__, __LINE__, renderer->view_width, renderer->view_height);

		NXT_BEGIN_PROFILE("Render Y2CrA8888 : Prep frame render");
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0, 0.0, 0.5, 1.0);
		CHECK_GL_ERROR();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		CHECK_GL_ERROR();
		glEnable(GL_SCISSOR_TEST);
		NXT_END_PROFILE();

		NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 1");
		glScissor(0, renderer->view_height / 2, renderer->view_width, renderer->view_height / 2);
		glViewport(0, renderer->view_height / 2, renderer->view_width, renderer->view_height);
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_Y], tex_id, reverse);
		NXT_END_PROFILE();

		NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 2");
		glScissor(0, renderer->view_height / 2, renderer->view_width, renderer->view_height / 2);
		glViewport(0, 0, renderer->view_width, renderer->view_height);
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE);
		
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_Y], tex_id, reverse);
		NXT_END_PROFILE();

		NXT_BEGIN_PROFILE("Render Y2CrA8888 : U Pass");
		glScissor(0, renderer->view_height / 2, renderer->view_width / 2, renderer->view_height / 2);
		glViewport(0, renderer->view_height / 2, renderer->view_width / 2, renderer->view_height / 2);
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_FALSE);
		
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_U], tex_id, reverse);
		NXT_END_PROFILE();

		NXT_BEGIN_PROFILE("Render Y2CrA8888 : V Pass");
		glScissor(0 + (renderer->view_width / 2), renderer->view_height / 2, renderer->view_width / 2, renderer->view_height / 2);
		glViewport(0 + (renderer->view_width / 2), renderer->view_height / 2, renderer->view_width / 2, renderer->view_height / 2);
		glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_FALSE);
		renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassThroughRGBAtoYUVA8888[NXT_ChannelFilter_V], tex_id, reverse);
		NXT_END_PROFILE();

		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		renderer->channelFilter = NXT_ChannelFilter_ALL;
		renderer->outputType = NXT_RendererOutputType_Y2CrA_8888;
		glViewport(0, 0, renderer->view_width, renderer->view_height);
		glScissor(0, 0, renderer->view_width, renderer->view_height);
		glDisable(GL_SCISSOR_TEST);
	}
	break;
    default:
    break;
	};


	NXT_ThemeRenderer_ReleaseRenderTarget(renderer, prender_target);

}

static void freeReadpixels(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
	// Free the readPixels buffer if one was allocated (we don't need it
	// when rendering to the screen, and we should conserve memory as
	// much as possible).  Do this only if we didn't call GetPixels last
	// rendering pass.
	if (renderer->pReadPixelData && !renderer->bCalledGetPixels) {
		LOGD("[%s %d] freeing getpixels buffer to conserve memory", __func__, __LINE__);
		free(renderer->pReadPixelData);
		renderer->pReadPixelData = NULL;
		renderer->readPixelDataSize = 0;
	}
	renderer->bCalledGetPixels = 0;
	renderer->bDidRender = 1;
}

static NXT_Error beginRenderer(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
	if (!renderer) {
		LOGE("[%s %d] null pointer", __func__, __LINE__);
		return NXT_Error_MissingParam;
	}

	if (!renderer->bInitialized) {
		NXT_Error result = initRenderer(renderer);
		if (result != NXT_Error_None || !renderer->bInitialized) {
			LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
			return result;
		}
	}
	renderer->mask_init_flag_ = 0;
	return 	NXT_Error_None;
}

static void setRenderTargetForRTT(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int forced){

	if (renderer->down_scale_info_.size() > 0){
		renderer->view_width = renderer->down_scale_info_[0].width_;
		renderer->view_height = renderer->down_scale_info_[0].height_;
	}

    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_render_target_);
    renderer->pmain_render_target_ = NULL;

	if((renderer->down_scale_info_.size() > 0) || forced){
    
		renderer->pmain_render_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 0, 1, 1);
    }

	NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_mask_target_);
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, renderer->pmain_blend_target_);
    int mask_render_target_width = renderer->view_width;;
    int mask_render_target_height = renderer->view_height;

    if(renderer->view_width * renderer->view_height > 1920*1080){

        if(renderer->view_width > renderer->view_height){

            float scale = 1920.0f / float(renderer->view_width);
            mask_render_target_width = 1920;
            mask_render_target_height = int(float(renderer->view_height) * scale);
        }
        else{

            float scale = 1080.0f / float(renderer->view_height);
            mask_render_target_width = int(float(renderer->view_width) * scale);
            mask_render_target_height = 1080;           
        }
    }
	renderer->pmain_mask_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, mask_render_target_width, mask_render_target_height, 0, 0, 0);
    renderer->pmain_blend_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 1, 1, 1);
	NXT_ThemeRenderer_SetRenderTarget(renderer,renderer->pmain_render_target_);
}

void cleanupMaskWithWhite(NXT_HThemeRenderer renderer){
	if (0 == NXT_ThemeRenderer_GetTextureNameForMask(renderer)){
		return;
	}
	if (0 == NXT_ThemeRenderer_GetTextureNameForWhite(renderer)){
		return;
	}
	NXT_ThemeRenderer_SetRenderTarget(renderer,renderer->pmain_mask_target_);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	NXT_ThemeRenderer_SetRenderTarget(renderer,renderer->pmain_render_target_);
}

int NXT_ThemeRenderer_CleanupMaskWithWhite(NXT_HThemeRenderer renderer){
	cleanupMaskWithWhite(renderer);
	return 0;
}

static NXT_Error NXT_ThemeRenderer_GLDraw_Main(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
	NXT_RendererOutputType targetOutputType = outputType;
	
	unsigned int renderer_view_width = 0;
	unsigned int renderer_view_height = 0;
	outputType = NXT_RendererOutputType_RGBA_8888;
	int forced = ((outputType != targetOutputType)||(renderer->only_rgb_possible_surface_))?1:renderer->getForceRTT();
	NXT_Error err = beginRenderer(renderer, outputType);
	if (NXT_Error_None != err)
		return err;

	renderer_view_width = renderer->view_width;
	renderer_view_height = renderer->view_height;
	setRenderTargetForRTT(renderer, outputType, forced);
	renderer->setGLRenderStart();

	NXT_ThemeRenderer_UpdateRenderTargetManager(renderer);
	NXT_Theme_UpdateTextureManager(renderer->getTexManager());
    NXT_Theme_UpdateRenderItemManager(renderer->getRenderItemManager(), renderer->pmain_mask_target_->target_texture_, renderer->pmain_blend_target_->target_texture_, NXT_ThemeRenderer_GetTextureNameForWhite(renderer));

	//For simplifying the solution, I will render scene with only RGBA output.
	//Color space conversion will be done with result RGBA Render Target Texture.

	NXT_BEGIN_PROFILE("Render for output (RGBA_8888)");

	setViewportForRGB(renderer);
	renderer->setGLStateForRGBAOutput();
    
	render_internal(renderer);
#ifdef DEBUG_OUTPUT
	static int dbg_pos = 0;
	renderSimpleBoxForDebug(renderer, dbg_pos);
	dbg_pos = (dbg_pos + 1) % 4;
#endif//DEBUG_OUTPUT
	NXT_END_PROFILE();
	renderer->scaleDownRender();
	renderer->view_width = renderer_view_width;
	renderer->view_height = renderer_view_height;
	outputType = targetOutputType;
	convertColorspace(renderer, outputType);
	freeReadpixels(renderer, outputType);
    resetMainRenderTarget(renderer);
	return NXT_Error_None;
}

static void timeClipEffect( NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int actualEffectStartCTS, int actualEffectEndCTS) {
    NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)effect);
    LOGI("[%s %d] (THEMECHK) timeClipEffect // ThemeID=%s EffectID=%s, T=%f", __func__, __LINE__, NXT_Theme_GetID(NXT_Effect_GetTheme(effect)), NXT_Effect_GetID(effect), renderer->time);
    
    renderer->inTime = pEffect->inTime;
    renderer->outTime = pEffect->outTime;
    renderer->cycleTime = pEffect->cycleTime;
    renderer->repeatType = pEffect->repeatType;
    //            if( renderer->clip_count>0 && renderer->clip_index==0 ) {
    //                LOGI("[%s %d] renderer title time : FIRST CLIP %d -> %d", __func__, __LINE__, renderer->inTime, pEffect->inTimeFirst );
    //                renderer->inTime = pEffect->inTimeFirst;
    //            }
    //            if( renderer->clip_count>0 && renderer->clip_index>=(renderer->clip_count-1)) {
    //                LOGI("[%s %d] renderer title time : LAST CLIP %d -> %d", __func__, __LINE__, renderer->outTime, pEffect->outTimeLast );
    //                renderer->outTime = pEffect->outTimeLast;
    //            }
    LOGD("[%s %d] renderer title : actualEffect(%d,%d) clip(%d,%d) computed(%d,%d)", __func__, __LINE__, actualEffectStartCTS, actualEffectEndCTS, renderer->pActiveEffect->clipStartTime, renderer->pActiveEffect->clipEndTime, (actualEffectStartCTS - renderer->pActiveEffect->clipStartTime ), (renderer->pActiveEffect->clipEndTime - actualEffectEndCTS) );
    if( actualEffectStartCTS < renderer->pActiveEffect->clipStartTime + 33 ) {
        LOGI("[%s %d] renderer title time : FIRST CLIP %d -> %d", __func__, __LINE__, renderer->inTime, pEffect->inTimeFirst );
        renderer->inTime = pEffect->inTimeFirst;
    }
    if( actualEffectEndCTS > renderer->pActiveEffect->clipEndTime - 33 ) {
        LOGI("[%s %d] renderer title time : LAST CLIP %d -> %d", __func__, __LINE__, renderer->outTime, pEffect->outTimeLast );
        renderer->outTime = pEffect->outTimeLast;
    }
    if(renderer->inTime + renderer->outTime + renderer->cycleTime > renderer->max_time ) {
        int divisor = 1;
        int partMaxTime;
        if( renderer->inTime > 0 )
            divisor++;
        if( renderer->outTime > 0 )
            divisor++;
        partMaxTime = renderer->max_time / divisor;
        LOGI("[%s %d] renderer title time (need limit; divisor=%d); cur_time=%d; inTime=%d; outTime=%d; cycleTime=%d; maxTime=%d", __func__, __LINE__, divisor, cur_time, renderer->inTime, renderer->outTime, renderer->cycleTime, renderer->max_time);
        if( renderer->inTime > partMaxTime  )
            renderer->inTime = partMaxTime;
        if( renderer->outTime > partMaxTime )
            renderer->outTime = partMaxTime;
        partMaxTime = renderer->max_time - (renderer->inTime + renderer->outTime);
        if( renderer->cycleTime > partMaxTime )
            renderer->cycleTime = partMaxTime;
    }
    
    LOGI("[%s %d] renderer title time; cur_time=%d; inTime=%d; outTime=%d; cycleTime=%d; maxTime=%d", __func__, __LINE__, cur_time, renderer->inTime, renderer->outTime, renderer->cycleTime, renderer->max_time);
    
    if( cur_time < renderer->inTime ) {
        renderer->part = NXT_PartType_In;
        renderer->time = (float)cur_time / (float)renderer->inTime;
    } else if (cur_time > renderer->max_time - renderer->outTime ) {
        renderer->part = NXT_PartType_Out;
        renderer->time = (float)(cur_time-(renderer->max_time - renderer->outTime)) / (float)renderer->outTime;
    } else if (renderer->repeatType == NXT_RepeatType_None || renderer->cycleTime==0 ) {
        renderer->part = NXT_PartType_Mid;
        int midtime = (renderer->max_time - (renderer->inTime+renderer->outTime));
        renderer->time = (float)(cur_time-renderer->inTime) / (float)(renderer->max_time - (renderer->inTime+renderer->outTime));
        if(midtime < 1){

        	renderer->time = 0;
        }else{

        	renderer->time = (float)(cur_time-renderer->inTime) / (float)midtime;
        }
        
    } else {
        renderer->part = NXT_PartType_Mid;
        int midtime = (renderer->max_time - (renderer->inTime+renderer->outTime));
        int numcycles = floor(float(midtime) / float(renderer->cycleTime));
        int remnant = midtime % renderer->cycleTime;
        
        if(remnant != 0){

            if(numcycles > 0){

                float plus_one_cycle = fabsf(float(remnant - renderer->cycleTime) / float(renderer->cycleTime * (numcycles + 1)));
                float plus_cycle_time = fabsf(float(remnant) / float(renderer->cycleTime * numcycles));

                if(plus_one_cycle < plus_cycle_time){

                    numcycles = numcycles + 1;
                }
            }
        }

        if( numcycles<1 )
            numcycles = 1;        
        int actualCycleTime = midtime / numcycles;
        if(actualCycleTime < 1)
        	actualCycleTime = 1;
        int curCycleTime = (cur_time-renderer->inTime)%actualCycleTime;
        if(floor(float(cur_time-renderer->inTime) / float(actualCycleTime)) >= float(numcycles))
            curCycleTime = actualCycleTime;
        renderer->time = (float)curCycleTime / (float)actualCycleTime;
        switch( renderer->repeatType ) {
            case NXT_RepeatType_Sawtooth:
                if( renderer->time <= 0.5f ) {
                    renderer->time *= 2.0f;
                } else {
                    renderer->time = (1.0f - renderer->time)*2.0f;
                }
                break;
            case NXT_RepeatType_Sine:
                renderer->time = 1.0f - cosf(renderer->time * 3.14159265f * 2.0f);
                break;
            case NXT_RepeatType_Repeat:
            case NXT_RepeatType_None:
                break;
        }
    }
    
}

void NXT_ThemeRenderer_TimeClipEffect(NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int actualEffectStartCTS, int actualEffectEndCTS) {

	timeClipEffect(renderer, effect, cur_time, actualEffectStartCTS, actualEffectEndCTS);
}


static void render_effect( NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time, int reset_flag) {
    NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)effect);
    LOGI("[%s %d] (THEMECHK) render_effect // ThemeID=%s EffectID=%s, T=%f", __func__, __LINE__, NXT_Theme_GetID(NXT_Effect_GetTheme(effect)), NXT_Effect_GetID(effect), renderer->time);

    // int start_track_id = renderer->getCurrentTrackID();

    NXT_NodeHeader *kedlnode = NXT_FindFirstNodeOfClass( (NXT_NodeHeader*)effect, &NXT_NodeClass_KEDL );
    while( kedlnode ) {
        if( kedlnode->isa->beginRenderFunc ) {
            kedlnode->isa->beginRenderFunc( kedlnode, renderer );
        }
        kedlnode = NXT_FindNextNodeOfClass(kedlnode, &NXT_NodeClass_KEDL );
    }

    // if(reset_flag)
    //     renderer->resetCurrentTrackID(start_track_id);
    
    float overallTime = (float)cur_time / (float)renderer->max_time;
    
    NXT_Effect_Seek( effect, renderer->time, /*renderer->settime*/overallTime, renderer->part, renderer->clip_index, renderer->clip_count );
    
    NXT_NodeHeader *pn = (NXT_NodeHeader*)effect;
    int repeat = 0;
    int stop = 0;
    while(pn && !stop) {
        renderer->bSkipChildren = 0;
        if( !repeat )
            pn->isa->beginRenderFunc(pn,renderer);
        
        repeat = 0;
        
        if( pn->child && !renderer->bSkipChildren) {
            pn = pn->child;
        } else if( pn->next ) {
            switch( pn->isa->endRenderFunc(pn,renderer)) {
                case NXT_NodeAction_Repeat:
                    repeat = 1;
                    break;
                case NXT_NodeAction_Stop:
                    stop = 1;
                    break;
                case NXT_NodeAction_Next:
                    break;
            }
            if( repeat || stop )
                continue;
            pn = pn->next;
        } else {
            while( pn->parent && !pn->parent->next ) {
                switch( pn->isa->endRenderFunc(pn,renderer) ) {
                    case NXT_NodeAction_Repeat:
                        repeat = 1;
                        break;
                    case NXT_NodeAction_Stop:
                        stop = 1;
                        break;
                    case NXT_NodeAction_Next:
                        break;
                }
                if( repeat || stop )
                    break;
                pn = pn->parent;
            }
            if( repeat || stop )
                continue;
            switch( pn->isa->endRenderFunc(pn,renderer)) {
                case NXT_NodeAction_Repeat:
                    repeat = 1;
                    break;
                case NXT_NodeAction_Stop:
                    stop = 1;
                    break;
                case NXT_NodeAction_Next:
                    break;
            }
            if( repeat || stop )
                continue;
            if( pn->parent && pn->parent->next ) {
                pn = pn->parent;
                switch( pn->isa->endRenderFunc(pn,renderer)) {
                    case NXT_NodeAction_Repeat:
                        repeat = 1;
                        break;
                    case NXT_NodeAction_Stop:
                        stop = 1;
                        break;
                    default:
                        break;
                }
                if( repeat || stop )
                    continue;
                pn = pn->next;
            } else {
                break;
            }
        }
    }
    
}

void NXT_ThemeRenderer_RenderEffect(NXT_HThemeRenderer renderer, NXT_HEffect effect, int cur_time){

	render_effect(renderer, effect, cur_time);
}

static void luminance_alpha_bugfix(NXT_HThemeRenderer renderer) {
    //////// WORKAROUND ////////////////////////////////////////////////////
    // There seems to be a GPU bug on certain chipsets where luminance-alpha
    // texture used to draw the YUV frame is not read properly in the shader.
    // By selecting a different texture first and then re-selecting the
    // original, this problem seems to be solved.
    
    NXT_Node_Texture textureNode1 = {0};
    NXT_Node_Texture textureNode2 = {0};
    textureNode1.textureType = NXT_TextureType_Video;
    textureNode1.videoSource = 1;
    textureNode2.textureType = NXT_TextureType_Video;
    textureNode2.videoSource = 2;
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    NexThemeRenderer_PrepStateForRender(renderer,&textureNode2,NULL,&texture,&mask, 0);
    GLfloat vertexCoord[] = {
        -1.0,   1.0,
        1.0,   1.0,
        -1.0,  -1.0,
        1.0,  -1.0
    };
    GLfloat tmpCoords[8] = {0};
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,tmpCoords);
    CHECK_GL_ERROR();
    
    NexThemeRenderer_PrepStateForRender(renderer,&textureNode1,NULL,&texture,&mask, 0);
    GLfloat texCoords[] = {
        0,      0,
        1,      0,
        0,      1,
        1,      1
    };
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
    CHECK_GL_ERROR();
    ////////////////////////////////////////////////////////////////////////

}

static void uploadTexture_internal(NXT_HThemeRenderer renderer, char* path, NXT_ImageInfo* imgInfo, NXT_TextureInfo *texinfo) {
    
    texinfo->bValidTexture=1;
    
    texinfo->srcWidth = imgInfo->width;
    texinfo->srcHeight = imgInfo->height;
    texinfo->left = 0;
    texinfo->bottom = 0;
    texinfo->right = texinfo->srcWidth;
    texinfo->top = texinfo->srcHeight;
    texinfo->textureFormat = NXT_PixelFormat_RGBA8888;    
    
    GL_GenTextures( 1, &texinfo->texName[0] );
    texinfo->texNameInitCount = 1;
    texinfo->texName_for_rgb = texinfo->texName[0];
    NXT_Theme_SetTextureInfo(renderer->getTexManager(), path, texinfo);
    
    // Initialize
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texinfo->texName[0]);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    
    // Choose texture format
    GLint texInternalFormat;
    GLenum texDataFormat;
    GLenum texDataType;
    int bytesPerPixel;
    switch( imgInfo->pixelFormat ) {
        case NXT_PixelFormat_RGB565:
            texInternalFormat=GL_RGB;
            texDataFormat=GL_RGB;
            texDataType=GL_UNSIGNED_SHORT_5_6_5;
            bytesPerPixel = 2;
            break;
        case NXT_PixelFormat_RGBA8888:
        default:
            texInternalFormat=GL_RGBA;
            texDataFormat=GL_RGBA;
            texDataType=GL_UNSIGNED_BYTE;
            bytesPerPixel = 4;
            break;
    }

    texinfo->textureWidth = NXT_NextHighestPowerOfTwo(imgInfo->pitch);
    texinfo->textureHeight = NXT_NextHighestPowerOfTwo(imgInfo->height);
    
    // Make the texture
    if(texinfo->textureWidth == imgInfo->pitch &&
       texinfo->textureHeight == imgInfo->height)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat,
                     texinfo->textureWidth, texinfo->textureHeight, 0,
                     texDataFormat, texDataType, 
                     (const GLvoid*)imgInfo->pixels );
        CHECK_GL_ERROR();
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat,
                     texinfo->textureWidth, texinfo->textureHeight, 0,
                     texDataFormat, texDataType, 
                     NULL );
        CHECK_GL_ERROR();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                        imgInfo->pitch, imgInfo->height, 
                        texDataFormat, texDataType, 
                        (const GLvoid*)imgInfo->pixels );
        CHECK_GL_ERROR();
    }
    
    if (renderer->freeImageCallback ) {
        	
        renderer->freeImageCallback(imgInfo, renderer->imageCallbackPvtData);
    }
}

static void premultAlpha_internal(unsigned int *pixels, unsigned int numPixels) {
    
    if( !pixels )
        return;
    
    unsigned char *p = (unsigned char*)pixels;
    unsigned char *end = p + (numPixels*4);
    
    for( ; p<end; p+=4 ) {
        unsigned char a = *(p+3);
        *(p+0) = *(p+0) * a / 255;
        *(p+1) = *(p+1) * a / 255;
        *(p+2) = *(p+2) * a / 255;
    }
    
}

static void getTexture_internal(NXT_HThemeRenderer renderer, char* path, NXT_TextureInfo *texinfo, NXT_Matrix4f* pmatrix){

	int check_for_already_loaded = 0;

	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float scale_x_for_src = 1.0f;
	float scale_y_for_src = 1.0f;

    if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), path, texinfo))
        check_for_already_loaded = 1;

    if(check_for_already_loaded){

        texinfo->bValidTexture=1;

        texinfo->left = 0;
        texinfo->bottom = 0;
        texinfo->right = texinfo->srcWidth;
        texinfo->top = texinfo->srcHeight;
        texinfo->textureFormat = NXT_PixelFormat_RGBA8888;

        texinfo->textureWidth = NXT_NextHighestPowerOfTwo(texinfo->srcWidth);
        texinfo->textureHeight = NXT_NextHighestPowerOfTwo(texinfo->srcHeight);
    }
    else if(renderer->loadImageCallback ) {

        NXT_Error result;
        NXT_ImageInfo imgInfo = {0};
        result = renderer->loadImageCallback(&imgInfo,
                                             path,
                                             0, 
                                             renderer->imageCallbackPvtData);        
        
        premultAlpha_internal((unsigned int*)imgInfo.pixels, imgInfo.width*imgInfo.height);
        uploadTexture_internal(renderer, path, &imgInfo, texinfo);
    }
    scale_x = (float)renderer->view_width / (float)texinfo->srcWidth;
    scale_y = (float)renderer->view_height / (float)texinfo->srcHeight;

    scale_x_for_src = (float)texinfo->srcWidth / (float)texinfo->textureWidth;
	scale_y_for_src = (float)texinfo->srcHeight / (float)texinfo->textureHeight;

    if(scale_x > scale_y)
    	 scale_x_for_src *= ((float)renderer->view_width / ((float)texinfo->srcWidth * scale_y));
    else
    	scale_y_for_src *= ((float)renderer->view_height / ((float)texinfo->srcHeight * scale_x));

    
    *pmatrix = NXT_Matrix4f_Scale(scale_x_for_src, scale_y_for_src, 1.0f);
}

struct FinalAdjustRenderer{

    FinalAdjustRenderer(NXT_HThemeRenderer renderer){

        renderer_ = renderer;
        final_adjust_ = false;
        pmain_render_target_ = NULL;
        pmain_mask_target_ = NULL;
        pmain_blend_target_ = NULL;
        //to remove race condition
        //we can achieve it by locking but it too much heavy for this operation
        //so I decided to use local variables

        final_brightness_ = renderer->final_brightness_;
        final_contrast_ = renderer->final_contrast_;
        final_saturation_ = renderer->final_saturation_;    
        final_vignetteRange_ = renderer->final_vignetteRange_;
        final_vignette_ = renderer->final_vignette_;
        final_sharpness_ = renderer->final_sharpness_;

        if (final_saturation_ != 0 || final_contrast_ != 0 || final_brightness_ != 0 || final_sharpness_ != 0 || final_vignette_ != 0 || final_vignetteRange_ != 0)
        {
            final_adjust_ = true;          
            
            pmain_render_target_ = renderer->pmain_render_target_;
            renderer->pmain_render_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 0, 1, 1);
            pmain_mask_target_ = renderer->pmain_mask_target_;
            renderer->pmain_mask_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 0, 0, 0);
            pmain_blend_target_ = renderer->pmain_blend_target_;
            renderer->pmain_blend_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 1, 1, 1);
            NXT_ThemeRenderer_SetRenderTarget(renderer,renderer->pmain_render_target_);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            CHECK_GL_ERROR();
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            CHECK_GL_ERROR();
            
        }
    }

    void apply(){

        if (final_adjust_){

            int render_pass = 0;
            if(final_saturation_ != 0 || final_contrast_ != 0 || final_brightness_ != 0)
                render_pass++;
            if(final_sharpness_ != 0 )
                render_pass++;
            if(final_vignette_ != 0 || final_vignetteRange_ != 0)
                render_pass++;
            

            NXT_RenderTarget* pbackup_render_target = renderer_->pmain_render_target_;
            NXT_RenderTarget* pbackup_mask_target = renderer_->pmain_mask_target_;
            NXT_RenderTarget* pbackup_blend_target = renderer_->pmain_blend_target_;
            renderer_->pmain_render_target_ = pmain_render_target_;
            renderer_->pmain_mask_target_ = pmain_mask_target_;
            renderer_->pmain_blend_target_ = pmain_blend_target_;
            
            if(final_saturation_ != 0 || final_contrast_ != 0 || final_brightness_ != 0) {

                NXT_RenderTarget* prender_target = pmain_render_target_;
                render_pass--;
                if(render_pass > 0){

                    prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer_, renderer_->view_width, renderer_->view_height, 0, 1, 1);
                }
                NXT_ThemeRenderer_SetRenderTarget(renderer_, prender_target);
                renderSimplePlaneWithColorconv(renderer_, &renderer_->getContextManager()->texturedShaderPassthroughRGBWithColorconv, pbackup_render_target->target_texture_, 1);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer_, pbackup_render_target);
                pbackup_render_target = prender_target;
            }

            if(final_sharpness_ != 0 ) {

                NXT_RenderTarget* prender_target = pmain_render_target_;
                render_pass--;
                if(render_pass > 0){

                    prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer_, renderer_->view_width, renderer_->view_height, 0, 1, 1);
                }
                NXT_ThemeRenderer_SetRenderTarget(renderer_, prender_target);
                renderSimplePlaneSharpness(renderer_, &renderer_->getContextManager()->texturedShaderSharpness, pbackup_render_target->target_texture_, 1);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer_, pbackup_render_target);
                pbackup_render_target = prender_target;
            }

            if(final_vignette_ != 0 || final_vignetteRange_ != 0) {
                
                NXT_RenderTarget* prender_target = pmain_render_target_;
                render_pass--;
                if(render_pass > 0){

                    prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer_, renderer_->view_width, renderer_->view_height, 0, 1, 1);
                }
                NXT_ThemeRenderer_SetRenderTarget(renderer_, prender_target);
                renderSimplePlaneUserVignette(renderer_, &renderer_->getContextManager()->texturedShaderUserVignette, pbackup_render_target->target_texture_, 1);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer_, pbackup_render_target);
                pbackup_render_target = prender_target;

            }
            NXT_ThemeRenderer_ReleaseRenderTarget(renderer_, pbackup_mask_target);
            NXT_ThemeRenderer_ReleaseRenderTarget(renderer_, pbackup_blend_target);
            
            final_adjust_ = false;
            pmain_render_target_ = NULL;
            pmain_mask_target_ = NULL;
            pmain_blend_target_ = NULL;
        }
    }

    ~FinalAdjustRenderer(){

        apply();
    }
private:
    bool final_adjust_;
    int final_brightness_;
    int final_contrast_;
    int final_saturation_;    
    float final_vignetteRange_;
    float final_vignette_;
    float final_sharpness_;

    NXT_HThemeRenderer renderer_;
    NXT_RenderTarget* pmain_render_target_;
    NXT_RenderTarget* pmain_mask_target_;
    NXT_RenderTarget* pmain_blend_target_;
};

static void render_internal( NXT_HThemeRenderer renderer )
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }

    FinalAdjustRenderer finalAdjuster(renderer);    
    
    renderer->basetransform = NXT_Matrix4f_Identity();
    NXT_HThemeSet themeSet = NULL;

    if(!renderer->playEDL(renderer->cts)){

        autoSelectEffectFromCurrentCTS(renderer);
        calcLocalTimeFromCurrentEffectAndCTS(renderer);

        
        pthread_mutex_lock(&renderer->themesetLock);
        
        themeSet = renderer->themeSet;
        if( themeSet!=NULL ) {
            NXT_ThemeSet_Retain(themeSet);
        }
        
        pthread_mutex_unlock(&renderer->themesetLock);
        
        if( renderer->pActiveEffect!=NULL ) {
            if( renderer->pActiveEffect->bEffectChanged ) {
                renderer->pActiveEffect->bEffectChanged = 0;
                renderer->bActiveEffectChanged = 1;
            }
        }

        NXT_HEffect cur_effect = NULL;
        
        if( renderer->pActiveEffect!=NULL ) {
            cur_effect = NXT_ThemeSet_GetEffectById(themeSet, renderer->pActiveEffect->effectId);
            renderer->effectType = NXT_Effect_GetType(cur_effect);
            renderer->clip_count = renderer->pActiveEffect->clip_count;
            renderer->clip_index = renderer->pActiveEffect->clip_index;
            renderer->cur_effect_overlap = NXT_Effect_GetIntProperty(cur_effect, NXT_EffectProperty_VideoOverlap);
            LOGI("[%s %d] Current Effect 0x%x, effectID: %s", __func__, __LINE__, LOGPTR(cur_effect), renderer->pActiveEffect->effectId);
        } else {
            LOGI("[%s %d] Current Effect 0x%x, no active effect", __func__, __LINE__, LOGPTR(cur_effect) );
            renderer->effectType = NXT_EffectType_NONE;
            renderer->clip_count = 0;
            renderer->clip_index = 0;
            renderer->cur_effect_overlap = 0;
        }


        if( renderer->bActiveEffectChanged ) {
            renderer->bActiveEffectChanged = 0;
            LOGI("[%s %d] Effect changed: Updating user fields", __func__, __LINE__ );
            NXT_Effect_UpdateUserFields(cur_effect, renderer);
        }
        
        unsigned int cur_time = renderer->cur_time;

        renderer->texture = NULL;
        renderer->mask = NULL;
        renderer->alpha  = 1.0;
        renderer->renderDest = NXT_RenderDest_Screen;
        renderer->renderTest = NXT_RenderTest_None;
        
        renderer->max_time = renderer->max_set_time;
        
        glColorMask(renderer->colorMask[0], renderer->colorMask[1], renderer->colorMask[2], renderer->colorMask[3]);
        
        unsigned int actualEffectStartCTS = 0;
        unsigned int actualEffectEndCTS = 0;
        
        unsigned int bUseEffect = 0;
        if( cur_effect && NXT_Node_ClassCheck((NXT_NodeHeader*)cur_effect,&NXT_NodeClass_Effect)) {
            NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)cur_effect);
            if( pEffect->effectType == NXT_EffectType_Title ) {
                int endTime = renderer->effectEndTime < renderer->max_time ? renderer->effectEndTime : renderer->max_time;
                if( renderer->effectStartTime >= 0 && endTime > 0 && endTime > renderer->effectStartTime && pEffect->bUserDuration ) {
                    if( cur_time >= renderer->effectStartTime && cur_time <= endTime ) {
                        bUseEffect = 1;
                        cur_time -= renderer->effectStartTime;
                        renderer->max_time = endTime - renderer->effectStartTime;
                        actualEffectStartCTS = renderer->pActiveEffect->effectStartTime + renderer->effectStartTime;
                        actualEffectEndCTS = renderer->pActiveEffect->effectStartTime + endTime;
                    }
                //} else if( pEffect->maxTitleDuration > 0 && renderer->max_time > pEffect->maxTitleDuration) {
                //    renderer->max_time = pEffect->maxTitleDuration;
                } else if( cur_time <= renderer->max_time ) {
                    actualEffectStartCTS = renderer->pActiveEffect->effectStartTime;
                    actualEffectEndCTS = renderer->pActiveEffect->effectEndTime;
                    bUseEffect = 1;
                }
            } else {
                actualEffectStartCTS = renderer->pActiveEffect->effectStartTime;
                actualEffectEndCTS = renderer->pActiveEffect->effectEndTime;
                bUseEffect = 1;
            }
        }
        
        if( renderer->bNoFx )
            bUseEffect = 0;
        
        if( bUseEffect ) {
            

            NXT_Node_Effect *pEffect = ((NXT_Node_Effect*)cur_effect);
            
            LOGI("[%s %d] (THEMECHK) BEGIN RENDER THEMED FRAME // ThemeID=%s EffectID=%s, T=%f", __func__, __LINE__, NXT_Theme_GetID(NXT_Effect_GetTheme(cur_effect)), NXT_Effect_GetID(cur_effect), renderer->time);
            
            luminance_alpha_bugfix(renderer);
            
            if( pEffect->effectType == NXT_EffectType_Title ) {
                timeClipEffect( renderer, cur_effect, cur_time, actualEffectStartCTS, actualEffectEndCTS );
            } else {
                renderer->time = renderer->settime;
                renderer->part = NXT_PartType_Mid;
            }

            renderer->currentTime = cur_time;
            renderer->actualEffectStartCTS = actualEffectStartCTS;
            renderer->actualEffectEndCTS = actualEffectEndCTS;

            if(renderer->videoPlaceholder1 && renderer->videoPlaceholder2){

                NXT_NodeHeader *texNode = NXT_FindFirstNodeOfClass( (NXT_NodeHeader*)cur_effect, &NXT_NodeClass_Texture );
                while( texNode ) {
                    if( texNode->isa->beginRenderFunc ) {
                        texNode->isa->beginRenderFunc( texNode, renderer );
                    }
                    texNode = NXT_FindNextNodeOfClass(texNode, &NXT_NodeClass_Texture );
                }                        
            }            

            render_effect(renderer, cur_effect, cur_time);
            
        }
        else
        {
            // No effect set, use texture 1
            LOGI("[%s %d] Current effect is NULL; just displaying video texture 1 (time %f ignored).", __func__, __LINE__, renderer->time);
            int skip_normal_draw = 0;
            NXT_Node_Texture textureNode1 = {0};
            NXT_Node_Texture textureNode2 = {0};
            textureNode1.textureType = NXT_TextureType_Video;
            textureNode1.videoSource = 1;
            textureNode2.textureType = NXT_TextureType_Video;
            textureNode2.videoSource = 2;
            NXT_TextureInfo *texture = NULL;
            NXT_TextureInfo *mask = NULL;

            if(renderer->pActiveEffect){

                int render_item_id = NXT_Theme_GetEffectID(renderer->getRenderItemManager(), renderer->pActiveEffect->effectId);
                int effect_type = NXT_Theme_GetEffectType(renderer->getRenderItemManager(), render_item_id);
                renderer->cur_effect_overlap = NXT_Theme_GetEffectOverlap(renderer->getRenderItemManager(), render_item_id);
                LOGI("%s %d effect_type:%d", __func__, __LINE__, effect_type);
                NXT_Theme_ClearSrcInfo(renderer->getRenderItemManager());

                if(effect_type == 1){

                    skip_normal_draw = 1;

                    if(renderer->videoPlaceholder1 && renderer->videoPlaceholder2){

                        NXT_Matrix4f texmat_0, texmat_1;

                        getTexture_internal(renderer, renderer->videoPlaceholder1, &textureNode1.texinfo_preview, &texmat_0);
                        getTexture_internal(renderer, renderer->videoPlaceholder2, &textureNode2.texinfo_preview, &texmat_1);

                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_BLEND);
                        CHECK_GL_ERROR();
                        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        CHECK_GL_ERROR();

                        glDisable(GL_CULL_FACE);
                        glDisable(GL_DEPTH_TEST);
                        glDepthMask(GL_FALSE);

                        NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), render_item_id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);
                        applyRenderItemOption(renderer, renderer->pActiveEffect);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            textureNode2.texinfo_preview.texName_for_rgb, 
                                                            textureNode2.texinfo_preview.textureWidth, 
                                                            textureNode2.texinfo_preview.textureHeight, 
                                                            textureNode2.texinfo_preview.srcWidth,
                                                            textureNode2.texinfo_preview.srcHeight,
                                                            E_TARGET_VIDEO_RIGHT);

                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat_1, E_TARGET_VIDEO_RIGHT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_RIGHT, &textureNode2.texinfo_preview);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), NULL, E_TARGET_VIDEO_RIGHT);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            textureNode1.texinfo_preview.texName_for_rgb, 
                                                            textureNode1.texinfo_preview.textureWidth, 
                                                            textureNode1.texinfo_preview.textureHeight, 
                                                            textureNode1.texinfo_preview.srcWidth,
                                                            textureNode1.texinfo_preview.srcHeight,
                                                            E_TARGET_VIDEO_SRC);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            textureNode1.texinfo_preview.texName_for_rgb, 
                                                            textureNode1.texinfo_preview.textureWidth, 
                                                            textureNode1.texinfo_preview.textureHeight, 
                                                            textureNode1.texinfo_preview.srcWidth,
                                                            textureNode1.texinfo_preview.srcHeight,
                                                            E_TARGET_VIDEO_LEFT);


                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat_0, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat_0, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, &textureNode1.texinfo_preview);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, &textureNode1.texinfo_preview);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), NULL, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), NULL, E_TARGET_VIDEO_LEFT);

                        NXT_Theme_DoEffect(
                            renderer->getRenderItemManager(), 
                            renderer->cts, 
                            cur_time, 
                            renderer->pActiveEffect->clipStartTime, 
                            renderer->pActiveEffect->clipEndTime, 
                            renderer->max_set_time, 
                            actualEffectStartCTS, 
                            actualEffectEndCTS, 
                            renderer->pActiveEffect->clip_index, 
                            renderer->pActiveEffect->clip_count);

                        NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
                        NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
                    }
                    else{

                        if(renderer->getVideoSrc(NXT_TextureID_Video_1).textureFormat != NXT_PixelFormat_RGBA8888){

                            NXT_ThemeRenderer_ConvertTextureToRGB(renderer, &renderer->getVideoSrc(NXT_TextureID_Video_1), 0, 0, 0, 0, NULL, 0, 10000.0);
                        }
                        if(renderer->getVideoSrc(NXT_TextureID_Video_2).textureFormat != NXT_PixelFormat_RGBA8888){

                            NXT_ThemeRenderer_ConvertTextureToRGB(renderer, &renderer->getVideoSrc(NXT_TextureID_Video_2), 0, 0, 0, 0, NULL, 0, 10000.0);
                        }

                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_BLEND);
                        CHECK_GL_ERROR();
                        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                        glDisable(GL_CULL_FACE);
                        glDisable(GL_DEPTH_TEST);
                        glDepthMask(GL_FALSE);

                        NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), render_item_id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);
                        applyRenderItemOption(renderer, renderer->pActiveEffect);

                        NXT_Matrix4f texmat, colormat;
                        NexThemeRenderer_PrepStateForRender2(renderer,&textureNode2,NULL,&texture,&mask, &texmat, &colormat, 0);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            texture->texName_for_rgb, 
                                                            texture->textureWidth, 
                                                            texture->textureHeight, 
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_RIGHT);

                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_RIGHT);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_RIGHT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_RIGHT, texture);

                        NexThemeRenderer_PrepStateForRender2(renderer,&textureNode1,NULL,&texture,&mask, &texmat, &colormat, 0);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            texture->texName_for_rgb, 
                                                            texture->textureWidth, 
                                                            texture->textureHeight, 
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_SRC);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            texture->texName_for_rgb, 
                                                            texture->textureWidth, 
                                                            texture->textureHeight, 
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_LEFT);

                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, texture);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, texture);

                        NXT_Theme_DoEffect(
                            renderer->getRenderItemManager(), 
                            renderer->cts, 
                            cur_time, 
                            renderer->pActiveEffect->clipStartTime, 
                            renderer->pActiveEffect->clipEndTime, 
                            renderer->max_set_time, 
                            actualEffectStartCTS, 
                            actualEffectEndCTS, 
                            renderer->pActiveEffect->clip_index, 
                            renderer->pActiveEffect->clip_count);

                        NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
                        NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
                    }
                    

                }
                if(effect_type == 2){

                    skip_normal_draw = 1;

                    if(renderer->videoPlaceholder1){

                        NXT_Matrix4f texmat;
                        getTexture_internal(renderer, renderer->videoPlaceholder1, &textureNode1.texinfo_preview, &texmat);

                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_BLEND);
                        CHECK_GL_ERROR();
                        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                        glDisable(GL_CULL_FACE);
                        glDisable(GL_DEPTH_TEST);
                        glDepthMask(GL_FALSE);

                        NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), render_item_id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);

                        applyRenderItemOption(renderer, renderer->pActiveEffect);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            textureNode1.texinfo_preview.texName[0], 
                                                            textureNode1.texinfo_preview.textureWidth, 
                                                            textureNode1.texinfo_preview.textureHeight, 
                                                            textureNode1.texinfo_preview.srcWidth,
                                                            textureNode1.texinfo_preview.srcHeight,
                                                            E_TARGET_VIDEO_SRC);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            textureNode1.texinfo_preview.texName[0], 
                                                            textureNode1.texinfo_preview.textureWidth, 
                                                            textureNode1.texinfo_preview.textureHeight, 
                                                            textureNode1.texinfo_preview.srcWidth,
                                                            textureNode1.texinfo_preview.srcHeight,
                                                            E_TARGET_VIDEO_LEFT);


                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, &textureNode1.texinfo_preview);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, &textureNode1.texinfo_preview);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), NULL, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), NULL, E_TARGET_VIDEO_LEFT);                    

                        NXT_Theme_DoEffect(
                            renderer->getRenderItemManager(), 
                            renderer->cts, 
                            cur_time, 
                            renderer->pActiveEffect->clipStartTime, 
                            renderer->pActiveEffect->clipEndTime, 
                            renderer->max_set_time, 
                            actualEffectStartCTS, 
                            actualEffectEndCTS, 
                            renderer->pActiveEffect->clip_index, 
                            renderer->pActiveEffect->clip_count);

                        NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
                        NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
                    }
                    else{

                        if(renderer->getVideoSrc(NXT_TextureID_Video_1).textureFormat != NXT_PixelFormat_RGBA8888){

                            NXT_ThemeRenderer_ConvertTextureToRGB(renderer, &renderer->getVideoSrc(NXT_TextureID_Video_1), 0, 0, 0, 0, NULL, 0, 10000.0);
                        }

                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glEnable(GL_BLEND);
                        CHECK_GL_ERROR();
                        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        CHECK_GL_ERROR();
                        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                        glDisable(GL_CULL_FACE);
                        glDisable(GL_DEPTH_TEST);
                        glDepthMask(GL_FALSE);

                        NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), render_item_id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);
                        applyRenderItemOption(renderer, renderer->pActiveEffect);

                        NXT_Matrix4f texmat, colormat;

                        NexThemeRenderer_PrepStateForRender2(renderer,&textureNode1,NULL,&texture,&mask, &texmat, &colormat, 0);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            texture->texName_for_rgb, 
                                                            texture->textureWidth, 
                                                            texture->textureHeight, 
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_SRC);

                        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                            texture->texName_for_rgb, 
                                                            texture->textureWidth, 
                                                            texture->textureHeight, 
                                                            texture->srcWidth,
                                                            texture->srcHeight,
                                                            E_TARGET_VIDEO_LEFT);

                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_SRC);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, texture);
                        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_LEFT);
                        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, texture);

                        NXT_Theme_DoEffect(
                            renderer->getRenderItemManager(), 
                            renderer->cts, 
                            cur_time, 
                            renderer->pActiveEffect->clipStartTime, 
                            renderer->pActiveEffect->clipEndTime, 
                            renderer->max_set_time, 
                            actualEffectStartCTS, 
                            actualEffectEndCTS, 
                            renderer->pActiveEffect->clip_index, 
                            renderer->pActiveEffect->clip_count);

                        NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
                        NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
                    }

                }

            }


            if(skip_normal_draw == 0){
                
                int first_processed = NexThemeRenderer_PrepStateForRender(renderer,&textureNode1,NULL,&texture,&mask, 1);
                GLfloat vertexCoord[] = {
                    -1.0,   1.0,
                    1.0,   1.0,
                    -1.0,  -1.0,   
                    1.0,  -1.0
                };
                GLfloat tmpCoords[8] = {0};
                glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
                CHECK_GL_ERROR();
                glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,tmpCoords);
                CHECK_GL_ERROR();
                
                
                // NexThemeRenderer_PrepStateForRender(renderer,&textureNode1,NULL,&texture,&mask, 1);
                float lowx = NexThemeRenderer_AdjTexCoordX(texture,0);
                float lowy = NexThemeRenderer_AdjTexCoordY(texture,0);
                float highx = NexThemeRenderer_AdjTexCoordX(texture,1);
                float highy = NexThemeRenderer_AdjTexCoordY(texture,1);
                LOGI("[%s %d] texture coordinates = %f %f %f %f", __func__, __LINE__, lowx, lowy, highx, highy);
                GLfloat texCoords[] = {
                    lowx,       lowy,
                    highx,      lowy,
                    lowx,       highy,
                    highx,      highy
                };
                glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
                CHECK_GL_ERROR();
                glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
                CHECK_GL_ERROR();
                LOGI("[%s %d] About to run glDrawArrays", __func__, __LINE__);
                glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
                CHECK_GL_ERROR();
            }
        }
//    renderWatermark_image(renderer);
    }
    
    finalAdjuster.apply();

    renderer->manageVideoTexture();
    renderer->manageDecoTexture();
#ifdef USE_ASSET_MANAGER
    if( !renderer->disable_watermark )
        renderLayers(renderer);
#else
    if( !renderer->disable_watermark )
        renderLayers(renderer);
    if( !renderer->disable_watermark )
        renderWatermark_effect(renderer,themeSet);
#endif    
 
	if( themeSet!=NULL ) {
		NXT_ThemeSet_Release(renderer->getContextManager(), themeSet, 0);
		themeSet = NULL;
	}

    LOGD("[%s %d] END", __func__, __LINE__);
}

void NXT_ThemeRenderer_SetEnableCustomRendering(NXT_HThemeRenderer renderer, unsigned int bEnable) {
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }
    
    renderer->bEnableCustomRendering = bEnable;
}

static void renderLayers(NXT_HThemeRenderer renderer) {
    LOGD("[%s %d] IN", __func__, __LINE__);
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }
    
    if( !renderer->bEnableCustomRendering )
        return;
#ifndef USE_ASSET_MANAGER
    if( renderer->disable_watermark )
        return;
#endif    
    
#ifndef USE_IMAGE_LOADER_FOR_LAYERS
    if( !renderer->customRenderCallback ) {
        LOGD("[%s %d] no customer render callback registered", __func__, __LINE__);
        return;
    }
#endif
    
    NXT_ShaderProgram_Textured *sp = NULL;
        
    if(renderer->outputType==NXT_RendererOutputType_RGBA_8888) {
        sp = &renderer->getContextManager()->texturedShaderRGB_for_layers;
    } else if(renderer->outputType==NXT_RendererOutputType_YUVA_8888) {
        sp = &renderer->getContextManager()->texturedShaderRGBAtoYUVA8888[renderer->channelFilter];
    }
    if( !sp ) {
        LOGI("[%s %d] WARNING: NO SUITABLE TEXTURED SHADER FOUND", __func__, __LINE__ );
        return;
    }

#ifdef USE_IMAGE_LOADER_FOR_LAYERS
    char loadPath[256];
    loadPath[sizeof(loadPath)-1]=0;     // snprintf doesn't null-terminate if the length is exceeded; this ensures null termination
    snprintf( loadPath, sizeof(loadPath)-1, "[Layer]%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             sp->shaderProgram, renderer->original_cts, sp->a_position, sp->a_texCoord,
             sp->u_mvp_matrix, sp->u_tex_matrix, sp->u_alpha, sp->u_realX, sp->u_realY, sp->u_colorconv, sp->u_textureSampler,
             sp->lightingInfo.u_ambient_light, sp->lightingInfo.u_diffuse_light, sp->lightingInfo.u_specular_light);

    NXT_ImageInfo imgInfo = {0};

    NXT_Error result = renderer->loadImageCallback(&imgInfo,loadPath, 0, renderer->imageCallbackPvtData);
    if( result != NXT_Error_None ) {
        LOGE("[%s %d] Error loading image (%d) : %s", __func__, __LINE__, result, loadPath );
    }
    if ( imgInfo.freeImageCallback ) {
        imgInfo.freeImageCallback(&imgInfo, renderer->imageCallbackPvtData);
        imgInfo.freeImageCallback = NULL;
    }

#else
    int flags = 0;
    if( renderer->rendererType==NXT_RendererType_InternalExportContext ) {
        flags |= LAYERRENDER_FLAG_EXPORT;
    }
    GLuint shaderProgram = 0;
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
    NXT_Error result = renderer->customRenderCallback(sp->base.shaderProgram,
#else
    NXT_Error result = renderer->customRenderCallback(sp->shaderProgram,
#endif
						renderer->original_cts,
						sp->a_position,
						sp->a_texCoord,
						sp->u_mvp_matrix,
						sp->u_tex_matrix,
						sp->u_alpha,
						sp->u_realX,
						sp->u_realY,
						sp->u_colorconv,
						sp->u_textureSampler,
						sp->u_textureSamplerY,
						sp->lightingInfo.u_ambient_light,
						sp->lightingInfo.u_diffuse_light,
						sp->lightingInfo.u_specular_light,
						flags,
						renderer->view_width,
						renderer->view_height);
													  
    if( result != NXT_Error_None ) {
        LOGE("[%s %d] Error rendering layers : %d (at cts=%d)", __func__, __LINE__, result, renderer->cts );
    }
#endif
}


static void renderWatermark_effect(NXT_HThemeRenderer renderer, NXT_HThemeSet themeSet) {
    
    LOGD("[%s %d] IN %d %d", __func__, __LINE__, renderer->bLoadedWatermark, renderer->bUseWatermark);
    
    if( !renderer || !themeSet || renderer->videoPlaceholder1 ) {
//        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }
    
    NXT_AppliedEffect* pPrevActiveEffect = renderer->pActiveEffect;
    renderer->pActiveEffect = &renderer->watermarkEffect;
    
    calcLocalTimeFromCurrentEffectAndCTS(renderer);
    
    unsigned int actualEffectStartCTS = renderer->pActiveEffect->effectStartTime;
    unsigned int actualEffectEndCTS = renderer->pActiveEffect->effectEndTime;

    
    NXT_HEffect cur_effect = NXT_ThemeSet_GetEffectById(themeSet, renderer->watermarkEffect.effectId);
    LOGD("[%s %d] wm:%s (0x%08X) actualEffectStartCTS=%d actualEffectEndCTS=%d renderer->cur_time=%d renderer->cts=%d", __func__, __LINE__, renderer->watermarkEffect.effectId, LOGPTR(cur_effect), actualEffectStartCTS, actualEffectEndCTS, renderer->cur_time, renderer->cts);
    if( !cur_effect ) {
        glClearColor(1.0, 0.5, 0.0, 1.0);
        CHECK_GL_ERROR();
        glDisable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR();
        glClear( GL_COLOR_BUFFER_BIT );
        CHECK_GL_ERROR();
        return;
    }
    
    renderer->effectType = NXT_Effect_GetType(cur_effect);
    renderer->clip_count = renderer->pActiveEffect->clip_count;
    renderer->clip_index = renderer->pActiveEffect->clip_index;
    
    timeClipEffect( renderer, cur_effect, renderer->cur_time, actualEffectStartCTS, actualEffectEndCTS );
    render_effect( renderer, cur_effect, renderer->cur_time);
 
    renderer->pActiveEffect = pPrevActiveEffect;

    LOGD("[%s %d] OUT", __func__, __LINE__);

}

static void renderWatermark_image(NXT_HThemeRenderer renderer) {

    LOGD("[%s %d] IN %d %d", __func__, __LINE__, renderer->bLoadedWatermark, renderer->bUseWatermark);
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }
    
	if( !renderer->bLoadedWatermark ) {
    	LOGD("[%s %d] Load", __func__, __LINE__);
        renderer->watermarkTexture.header.node_id = (char*)"@watermark";
        renderer->watermarkTexture.header.isa = &NXT_NodeClass_Texture;
    	renderer->watermarkTexture.header.isa->initNodeFunc(&renderer->watermarkTexture.header);
        renderer->watermarkTexture.header.isa->setAttrFunc(&renderer->watermarkTexture.header, (char*)"src", (char*)"@special:watermark.jpg");
    	LOGD("[%s %d] Precache", __func__, __LINE__);
//        renderer->watermarkTexture.header.isa->precacheFunc(&renderer->watermarkTexture.header, renderer, NXT_PrecacheMode_Upload);
//        if( renderer->watermarkTexture.imgInfo.width > 0 && renderer->watermarkTexture.imgInfo.height > 0 ) {
//	        renderer->bUseWatermark = 1;
//        }
    } else {
        if( !renderer->bUseWatermark ) {
    		return;
        }
    }
#ifndef USE_ASSET_MANAGER    
    if( renderer->disable_watermark == 1 )
        return;
#endif    
    renderer->watermarkTexture.header.isa->beginRenderFunc(&renderer->watermarkTexture.header, renderer);
    
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    NexThemeRenderer_PrepStateForRender(renderer,&renderer->watermarkTexture,NULL,&texture,&mask, 0);
    renderer->bLoadedWatermark = 1;
    
    if( !texture ) {
    	LOGD("[%s %d] texture is null", __func__, __LINE__);
		renderer->bUseWatermark = 0;
   		renderer->watermarkTexture.header.isa->endRenderFunc(&renderer->watermarkTexture.header, renderer);
        return;
    } else if (texture->srcHeight < 1 || texture->srcWidth < 1 ) {
    	LOGD("[%s %d] texture is too small %d %d", __func__, __LINE__, texture->srcHeight, texture->srcWidth);
		renderer->bUseWatermark = 0;
   		renderer->watermarkTexture.header.isa->endRenderFunc(&renderer->watermarkTexture.header, renderer);
        return;
    }
    renderer->bUseWatermark = 1;
    LOGD("[%s %d] begin render watermark", __func__, __LINE__);
    
    GLfloat srcWidth = texture->srcWidth;
    GLfloat srcHeight = texture->srcHeight;
    
    GLfloat canvasWidth = 1920;
    GLfloat canvasHeight = 1080;
    GLfloat rightMargin = 50;
    GLfloat bottomMargin = 50;
    
    GLfloat right = canvasWidth - rightMargin;
    GLfloat left = right-srcWidth;
    GLfloat bottom = bottomMargin;
    GLfloat top = bottom + srcHeight;
    
    left = (left/canvasWidth * 2.0f) - 1.0f;
    right = (right/canvasWidth * 2.0f) - 1.0f;
    top = (top/canvasHeight * 2.0f) - 1.0f;
    bottom = (bottom/canvasHeight * 2.0f) - 1.0f;
    
    GLfloat vertexCoord[] = {
        left,	top,
        right,	top,
        left,	bottom,
        right,  bottom
    };
    LOGI("[%s %d] vertex coordinates (%f x %f) = %f %f %f %f :  %f,%f %f,%f %f,%f %f,%f", __func__, __LINE__, srcWidth, srcHeight, left, top, right, bottom, vertexCoord[0], vertexCoord[1], vertexCoord[2], vertexCoord[3], vertexCoord[4], vertexCoord[5], vertexCoord[6], vertexCoord[7] );
    float lowx = NexThemeRenderer_AdjTexCoordX(texture,0);
    float lowy = NexThemeRenderer_AdjTexCoordY(texture,0);
    float highx = NexThemeRenderer_AdjTexCoordX(texture,1);
    float highy = NexThemeRenderer_AdjTexCoordY(texture,1);
    LOGI("[%s %d] texture coordinates = %f %f %f %f", __func__, __LINE__, lowx, lowy, highx, highy);
    GLfloat texCoords[] = {
        lowx,       lowy,
        highx,      lowy,
        lowx,       highy,
        highx,      highy
    };
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
    CHECK_GL_ERROR();
    LOGI("[%s %d] About to run glDrawArrays", __func__, __LINE__);
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    CHECK_GL_ERROR();
    LOGD("[%s %d] OUT", __func__, __LINE__);
    renderer->watermarkTexture.header.isa->endRenderFunc(&renderer->watermarkTexture.header, renderer);

}

void NXT_ThemeRenderer_ClearTextures(NXT_HThemeRenderer renderer1, NXT_HThemeRenderer renderer2) {

	LOGI("[%s %d] IN", __func__, __LINE__);

    renderer1->themeSet->rootNode = renderer2->themeSet->rootNode;

    LOGI("[%s %d] OUT", __func__, __LINE__);
}

int NXT_ThemeRenderer_GetPrecacheEffectResource(NXT_HThemeRenderer renderer, const char *effect_id, void* load_resource_list){

    LOGI("[%s %d] IN: %s", __func__, __LINE__,effect_id);

    if( !renderer || !effect_id ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return 1;
    }
    
    if( !renderer->themeSet || !renderer->themeSet->rootNode ) {
        LOGE("[%s %d] null themeset", __func__, __LINE__);
        return 1;
    }
    
    NXT_NodeHeader *effect = NXT_FindFirstNodeWithId((NXT_NodeHeader*)(renderer->themeSet->rootNode), effect_id, &NXT_NodeClass_Effect);
    if( !effect ) {

        int id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, effect_id);
        if(id >= 0){

            NXT_Theme_GetResourcelist(renderer->getRenderItemManager(), id, load_resource_list);
        }
        else{

            LOGI("[%s %d] SKIP", __func__, __LINE__);
        }
        return 0;
    }

    int ret = NXT_Theme_GetPrecacheResource((NXT_HEffect)effect, renderer, load_resource_list);

    LOGI("[%s %d] OUT", __func__, __LINE__);

    return ret;
}

int NXT_ThemeRenderer_PrecacheEffect(NXT_HThemeRenderer renderer, const char *effect_id, int* asyncmode, int* max_replaceable) {

    LOGI("[%s %d] IN: %s", __func__, __LINE__,effect_id);

    if( !renderer || !effect_id ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return 1;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();

    if( !ctx->contextIsAcquired ) {
        LOGE("[%s %d] no context", __func__, __LINE__);
        return 1;
    }

    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT", __func__, __LINE__ );
            return 1;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO GL CONTEXT", __func__, __LINE__ );
        }
#endif
    }
    
    if( !renderer->themeSet || !renderer->themeSet->rootNode ) {
        LOGE("[%s %d] null themeset", __func__, __LINE__);
        return 1;
    }
    
    NXT_NodeHeader *effect = NXT_FindFirstNodeWithId((NXT_NodeHeader*)(renderer->themeSet->rootNode), effect_id, &NXT_NodeClass_Effect);
    if( !effect ) {
   		
        int id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, effect_id);
        if(id >= 0){

            LOGI("[%s %d] RenderItem Precache", __func__, __LINE__);
            NXT_Theme_ForceBind(renderer->getRenderItemManager(), id);
        }
        else{

            LOGI("[%s %d] SKIP", __func__, __LINE__);
        }
        return 0;
    }

    int ret = NXT_Theme_DoPrecache((NXT_HEffect)effect, renderer, asyncmode, max_replaceable);

    LOGI("[%s %d] OUT", __func__, __LINE__);

    return ret;
}


//void NXT_ThemeRenderer_PrecacheImages(NXT_HThemeRenderer renderer, 
//                                      NXT_HTheme hTheme,    // Any theme in the set (doesn't matter which one)
//                                      NXT_PrecacheMode precacheMode)
//{
//    if( !renderer ) {
//        LOGE("[%s %d] null pointer", __func__, __LINE__);
//        return;
//    }
//    
//    if( !hTheme /*|| !NXT_Node_ClassCheck((NXT_NodeHeader*)hTheme,&NXT_NodeClass_Theme)*/ ) {
//        LOGE("[%s %d] null or invalid theme handle", __func__, __LINE__);
//        return;
//    }
//    
//    renderer->texture = NULL;
//    renderer->mask = NULL;
//    renderer->alpha  = 1.0;
//            
//    LOGI("[%s %d] BEGIN PRECACHE EFFECT; hTheme=0x%08X ThemeID='%s'", __func__, __LINE__, hTheme, NXT_Theme_GetID(hTheme));
//    
//    NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)hTheme, 
//                                           &NXT_NodeClass_Texture, 
//                                           NXT_StartAt_Root_IncludeEverything );
//    
//    LOGI("[%s %d] Made Iterator", __func__, __LINE__);
//    NXT_NodeHeader *texNode;
//    while( (texNode = iter.next(&iter)) ) {
//    	LOGI("[%s %d] Checking Node", __func__, __LINE__);
//        if( texNode->isa->precacheFunc ) {
//    		LOGI("[%s %d] Checking Node", __func__, __LINE__);
//            texNode->isa->precacheFunc( texNode, renderer, precacheMode );
//        }
//    }
//        
//    /*NXT_NodeHeader *texNode = NXT_FindFirstNodeOfClass( (NXT_NodeHeader*)renderer->effect, &NXT_NodeClass_Texture );
//    while( texNode ) {
//        if( texNode->isa->precacheFunc ) {
//            texNode->isa->precacheFunc( texNode, renderer );
//        }
//        texNode = NXT_FindNextNodeOfClass(texNode, &NXT_NodeClass_Texture );
//    }*/
//    
//    LOGI("[%s %d] DONE PRECACHE EFFECT", __func__, __LINE__);
//}

float NexThemeRenderer_AdjTexCoordX( NXT_TextureInfo *texture, float x ) {
/*    float inputWidth = texture->right - texture->left;
    float textureWidth = (float)texture->textureWidth;
    float result = (x * (inputWidth/textureWidth)) + (texture->left / textureWidth);*/
    if( x < 0.0 )
        return 0.0;
    else if( x > 1.0 )
        return 1.0;
    else
        return x;
}

float NexThemeRenderer_AdjTexCoordY( NXT_TextureInfo *texture, float y ) {
/*    float inputHeight = texture->top - texture->bottom;
    float srcHeight = (float)texture->srcHeight;
    float textureHeight = (float)texture->textureHeight;
    float result = ((y*(inputHeight/srcHeight)+(texture->bottom/srcHeight)))*(srcHeight/textureHeight);*/

    if( y < 0.0 )
        return 0.0;
    else if( y > 1.0 )
        return 1.0;
    else
        return y;
}


void NexThemeRenderer_PrepStateForRender2(NXT_HThemeRenderer renderer, 
                                         NXT_Node_Texture *texture,
                                         NXT_Node_Texture *mask,
                                         NXT_TextureInfo **useTexture,
                                         NXT_TextureInfo **useMask,
                                         NXT_Matrix4f* pmatrix,
                                         NXT_Matrix4f* pmatrix_for_colorconv,
                                         int bgr_flag)
{
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    if( !texture && renderer->texture )
        texture = renderer->texture;
    if( !mask && renderer->mask )
        mask = renderer->mask;
    if( !useMask )
        mask = NULL;
    if( !useTexture )
        texture = NULL;
        
    NXT_TextureInfo *texinfo = texture?&texture->texinfo_preview:NULL;
    NXT_TextureInfo *maskinfo = mask?&mask->texinfo_preview:NULL;
    
    NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix( renderer->proj, renderer->transform  );
                
    renderer->pSolidShaderActive    = NULL;
    renderer->pTexturedShaderActive = NULL;
    renderer->pMaskedShaderActive   = NULL;

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    switch(renderer->compMode) {
        case NXT_CompMode_Multiply:
            glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            break;
        case NXT_CompMode_Screen:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            break;
        case NXT_CompMode_Diff:
            glBlendEquationSeparate(GL_FUNC_SUBTRACT,GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case NXT_CompMode_Normal:
        default:
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
    CHECK_GL_ERROR();

	/*
    LOGI("MVP MATRIX:\n"
         "[%f %f %f %f\n"
         " %f %f %f %f\n"
         " %f %f %f %f\n"
         " %f %f %f %f]\n",
         mvp.e[0],  mvp.e[1],  mvp.e[2],  mvp.e[3],
         mvp.e[4],  mvp.e[5],  mvp.e[6],  mvp.e[7],
         mvp.e[8],  mvp.e[9],  mvp.e[10], mvp.e[11],
         mvp.e[12], mvp.e[13], mvp.e[14], mvp.e[15]
         );
    */
    unsigned int bYUV = 0;
    unsigned int bNV12 = 0;
    unsigned int bEGLI = 0;
    unsigned int bSTEN = 0;
	unsigned int bJPEG_ColorConv = 0;
	unsigned int bVideo = 0;
    
    int texid = -1;
    if(texture && texture->textureType==NXT_TextureType_Video && (!renderer->videoPlaceholder1 || !renderer->videoPlaceholder2) )
    {
    	bVideo = 1;
        if( renderer->bSwapV ) {
            switch(texture->videoSource) {
                case  1: texid = NXT_TextureID_Video_2; break;
                case  2: texid = NXT_TextureID_Video_1; break;
                default: texid = NXT_TextureID_Video_2; break;
            }
        } else {
            switch(texture->videoSource) {
                case  1: texid = NXT_TextureID_Video_1; break;
                case  2: texid = NXT_TextureID_Video_2; break;
                default: texid = NXT_TextureID_Video_1; break;
            }
        }
		
        if(renderer->pActiveEffect && renderer->cur_effect_overlap <= 0 ) 
        {
        	texid = NXT_TextureID_Video_1;
        }
		
        LOGI("[%s %d] is video (srcval=%d texid=%d/%s)", __func__, __LINE__, texture->videoSource, texid, (texid==NXT_TextureID_Video_1?"NXT_TextureID_Video_1":(texid==NXT_TextureID_Video_2?"NXT_TextureID_Video_2":"???")) );
        if( renderer->getVideoSrc(texid).bValidTexture ) {
            LOGI("[%s %d] valid texture:%d", __func__, __LINE__, texid );
            texinfo = &renderer->getVideoSrc(texid);
        }
    }
    
    if( texinfo ) {
        if( texinfo->textureFormat == NXT_PixelFormat_YUV ) {
            LOGI("[%s %d] (#%d=%d) is YUV", __func__, __LINE__, texid, texinfo->textureFormat );
            bYUV = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_NV12) {
            LOGI("[%s %d] (#%d=%d) is NV12", __func__, __LINE__, texid, texinfo->textureFormat );
            bNV12 = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_NV12_JPEG) {
            LOGI("[%s %d] (#%d=%d) is NV12", __func__, __LINE__, texid, texinfo->textureFormat );
            bNV12 = 1;
            bJPEG_ColorConv = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_EGLImage) {
            LOGI("[%s %d] (#%d=%d) is EGLImage", __func__, __LINE__, texid, texinfo->textureFormat );
            bEGLI = 1;
        }
    }

    
    if( renderer->renderDest == NXT_RenderDest_Stencil )
        bSTEN = 1;
    
    NXT_Matrix4f texmat = renderer->texture_transform;//!!!
    NXT_Matrix4f maskmat = renderer->mask_transform;//!!!
    
    if( texinfo ) {//!!!
        
        float tintBlue   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
		float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
		float tintRed  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
		(*pmatrix_for_colorconv) = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
		if(bgr_flag){

			static NXT_Matrix4f bgr_matrix = NXT_Matrix4f(  
							0.000,	0.000,	1.000,	0.000,
							0.000,	1.000,	0.000,	0.000,
							1.000,	0.000,	0.000,	0.000,
							0.000,	0.000,	0.000,	1.000);

			(*pmatrix_for_colorconv) = NXT_Matrix4f_MultMatrix((*pmatrix_for_colorconv), bgr_matrix);
		}
        
        if( texinfo->bHasInputRect ) {
            
#if 0
            // The final input rectangle is calculated as a proprotion of the texture width and
            // height, but at this point we only have pixel coordinates.  In order to convern them,
            // we need the original width and height, which are backwards if the texture has been
            // rotated 90 or 270 degrees.
            float rotatedPixelWidth, rotatedPixelHeight;
            if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                rotatedPixelWidth = texinfo->srcHeight;
                rotatedPixelHeight = texinfo->srcWidth;
            } else {
                rotatedPixelWidth = texinfo->srcWidth;
                rotatedPixelHeight = texinfo->srcHeight;
            }
            
            // Calculate proportional coordinates for input rectangle
            float left = texinfo->left / rotatedPixelWidth;
            float right = texinfo->right / rotatedPixelWidth;
            float top = texinfo->top / rotatedPixelHeight;
            float bottom = texinfo->bottom / rotatedPixelHeight;
#else
            // Calculate proportional coordinates for input rectangle
            float left = (float)texinfo->left / 100000.0f;
            float right = (float)texinfo->right / 100000.0f;
            float top = (float)texinfo->top / 100000.0f;
            float bottom = (float)texinfo->bottom / 100000.0f;
#endif

            
            // Translate and scale our rectangle to encompass exactly the required area
            NXT_pMatrix4f_ScaleX( &texmat, right-left );
            NXT_pMatrix4f_ScaleY( &texmat, top-bottom );
            NXT_pMatrix4f_TranslateX( &texmat, left );
            NXT_pMatrix4f_TranslateY( &texmat, bottom );

            // Rotate the texture coordinates.  This is done prior to scaling to match
            // the actual used area, which means the texture will be distorted; this is fixed
            // up later on when we crop to the input rectangle.
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            texmat = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Rotate(NXT_Vector4f(0.0,0.0,1.0,0.0), texinfo->rotation*PI_OVER_180), texmat);
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            // Handle mirroring
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            if( texinfo->mirror_h ) {
                NXT_pMatrix4f_ScaleX( &texmat, -1 );
            }
            if( texinfo->mirror_v ) {
                NXT_pMatrix4f_ScaleY( &texmat, -1 );
            }
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            // The actual image only takes up a portion of the texture, so we need to scale the
            // texture coordinates down from 1, 1 to enclose the actual portion that's used.
            float scalex = (float)texinfo->srcWidth / (float)texinfo->textureWidth;
            float scaley = (float)texinfo->srcHeight / (float)texinfo->textureHeight;
            
            NXT_pMatrix4f_ScaleX( &texmat, scalex );
            NXT_pMatrix4f_ScaleY( &texmat, scaley );
        } else {
        
            
            float inputWidth = texinfo->right - texinfo->left;
            float textureWidth = (float)texinfo->textureWidth;
            float inputHeight = texinfo->top - texinfo->bottom;
            float srcHeight = (float)texinfo->srcHeight;
            float textureHeight = (float)texinfo->textureHeight;
            float left = texinfo->left;
            float bottom = texinfo->bottom;
            
            if( !texinfo->bUseSurfaceTexture ) {
                inputWidth -= TWICE_TEX_BORDER_SIZE;
                inputHeight -= TWICE_TEX_BORDER_SIZE;
            }
            
            if ( texinfo->bAnimated ) {
                
                int fnum = (int)renderer->animframe;
                if( fnum < 0 )
                    fnum = 0;
                
                fnum %= texinfo->animFrames;
                
                unsigned int row = (fnum/texinfo->animCols);
                unsigned int col = fnum%texinfo->animCols;
                float colWidth = (1.0/(float)texinfo->animCols);
                float rowHeight = (1.0/(float)texinfo->animRows);

                LOGV("[%s %d] animated texture : row(%u) col(%u) rowHeight(%f) colWidth(%f) animRows(%d) animColumns(%d) animFrame(%f)->fnum(%u) ", __func__, __LINE__, row, col, rowHeight, colWidth, texinfo->animRows, texinfo->animCols, renderer->animframe, fnum );

                NXT_pMatrix4f_ScaleX(       &texmat, colWidth );
                NXT_pMatrix4f_ScaleY(       &texmat, rowHeight );
                NXT_pMatrix4f_TranslateX(   &texmat, colWidth*col );
                NXT_pMatrix4f_TranslateY(   &texmat, rowHeight*row );
                
            }
            
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            
            if( texinfo->bHasInputRect ) {
                if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                    float swap;
                    
                    swap = inputWidth;
                    inputWidth = inputHeight;
                    inputHeight = swap;
                    
                    swap = left;
                    left = inputHeight-bottom;
                    bottom = -swap;
                }
            } else if( texinfo->fitInWidth>0 && texinfo->fitInHeight>0 && inputWidth>32 && inputHeight>32 ) {
            
                float fitInWidth = (float)texinfo->fitInWidth;
                float fitInHeight = (float)texinfo->fitInHeight;
                float scaledInputWidth;
                float scaledInputHeight;

                if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                    scaledInputHeight = inputWidth;
                    scaledInputWidth = inputHeight;
                } else {
                    scaledInputWidth = inputWidth;
                    scaledInputHeight = inputHeight;
                }

                if( scaledInputWidth/fitInWidth > scaledInputHeight/fitInHeight ) {
                    scaledInputHeight *= (fitInWidth/scaledInputWidth);
                    scaledInputWidth = fitInWidth;
                } else {
                    scaledInputWidth *= (fitInHeight/scaledInputHeight);
                    scaledInputHeight = fitInHeight;
                }
                
            // for rotate ratio problem.
                NXT_pMatrix4f_ScaleX(       &texmat, (fitInWidth/scaledInputWidth) );
                NXT_pMatrix4f_ScaleY(       &texmat, (fitInHeight/scaledInputHeight) );
                // NXT_pMatrix4f_ScaleX(       &texmat, (1.0-scaledInputWidth/fitInWidth)*2.0+1.0 );
                // NXT_pMatrix4f_ScaleY(       &texmat, (1.0-scaledInputHeight/fitInHeight)*2.0+1.0 );
                
            }
            
            if( texinfo->rotation!=0 ) {
                texmat = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Rotate(NXT_Vector4f(0.0,0.0,1.0,0.0), texinfo->rotation*PI_OVER_180), texmat);
            }
            
            if( texinfo->mirror_h ) {
                NXT_pMatrix4f_ScaleX( &texmat, -1 );
            }
            if( texinfo->mirror_v ) {
                NXT_pMatrix4f_ScaleY( &texmat, -1 );
            }
            
            
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            
            
            
            if( texinfo->bUseSurfaceTexture ) 
            {
                LOGD("[%s %d] Using surface texture matrix", __func__, __LINE__ );
    //            texmat = NXT_Matrix4f_MultMatrix(texmat,NXT_Matrix4f_MultMatrix(texinfo->surfaceMatrix,NXT_Matrix4f_Scale(1, -1, 1)));
                NXT_Matrix4f adjmat = texinfo->surfaceMatrix;
                //adjmat.e[5] = -adjmat.e[5];
                //adjmat.e[13] = 0.0;
                texmat = NXT_Matrix4f_MultMatrix(texmat,adjmat);
            } else {
                LOGD("[%s %d] Computing texture matrix", __func__, __LINE__ );
                NXT_pMatrix4f_ScaleX(       &texmat, inputWidth / textureWidth) ;
                NXT_pMatrix4f_TranslateX(   &texmat, (left+TEX_BORDER_SIZE+1) / textureWidth );
                
                
                NXT_pMatrix4f_ScaleY(       &texmat, inputHeight / srcHeight );
                NXT_pMatrix4f_TranslateY(   &texmat, (bottom+TEX_BORDER_SIZE+1) / srcHeight );
                NXT_pMatrix4f_ScaleY(       &texmat, srcHeight / textureHeight );
            }
        }
        
    }

    
    
    
    
    
    if( texinfo ) {
        LOGI("[%s %d] texinfo l/b/r/t=%f/%f/%f/%f src w/h=%d/%d tex w/h=%d/%d format=%d bYUV=%d bNV12=%d renderer->outputType=%d mask=0x%08x texture=0x%08x", __func__, __LINE__, texinfo->left, texinfo->bottom, texinfo->right, texinfo->top, texinfo->srcWidth, texinfo->srcHeight, texinfo->textureWidth, texinfo->textureHeight, texinfo->textureFormat, bYUV, bNV12, renderer->outputType, (unsigned int)(size_t)mask, (unsigned int)(size_t)texture );
    } else {
        LOGI("[%s %d] NULL texinfo", __func__, __LINE__ );
    }

    
	
	if (texinfo){
		if (texinfo->texName_for_rgb == 0)
			texinfo->texName_for_rgb = texinfo->texName[0];
	}	
    if( useTexture )
        *useTexture = texinfo;
    if( useMask )
        *useMask = maskinfo;

    if(pmatrix)
    	*pmatrix = texmat;
}

bool simpleRenderLocker(NXT_HThemeRenderer renderer){

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();

	if( ctx->contextIsAcquired ) {
        if( pthread_equal(pthread_self(), ctx->contextOwner) ) {
            LOGI("[%s %d] Attempt to acquire context while already aquired in same thread; would deadlock.", __func__, __LINE__);
            return false;
        }
    }
    pthread_mutex_lock(&ctx->contextLock);
    ctx->contextOwner = pthread_self();
    ctx->contextIsAcquired = 1;
    return true;
}

void simpleRenderReleaser(NXT_HThemeRenderer renderer){

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    ctx->contextIsAcquired = 0;
    pthread_mutex_unlock(&ctx->contextLock);
}

int NXT_ThemeRenderer_SetBaseFilterRenderItemUID(NXT_HThemeRenderer renderer, const char* uid){

	bool release_required = simpleRenderLocker(renderer);

	LOGI("%s %d %s", __func__, __LINE__, uid);

	renderer->basefilter_renderitem_id_ = -1;
	renderer->basefilter_renderitem_uid_[0] = 0;

	int pos = strlen(uid);
	bool check_parameter = false;

	for(int i = 0; i < strlen(uid); ++i){

		if(uid[i] == ' ' || uid[i] == '\t'){

			pos = i;
			check_parameter = true;
			break;
		}

	}
	strncpy(renderer->basefilter_renderitem_uid_, uid, sizeof(renderer->basefilter_renderitem_uid_) - 1);
	if(pos < sizeof(renderer->basefilter_renderitem_uid_))
		renderer->basefilter_renderitem_uid_[pos] = 0;

	if(check_parameter){

		LOGI("%s %d %s", __func__, __LINE__, &uid[pos + 1]);
		NXT_ProcParamKeyValueFromString(NULL, &uid[pos + 1], &renderer->basefilter_keyvalues_);
	}
	if(release_required)
		simpleRenderReleaser(renderer);
	return 1;
}

void NXT_ThemeRenderer_RISetParameterKeyValue(NXT_HThemeRenderer renderer, NXT_RenderItem_KeyValues* pkeyvalues){

    NXT_Theme_ResetDefaultValuemap(renderer->getRenderItemManager());

    NXT_RenderItem_KeyValues::list_t& valuelist = pkeyvalues->items_;

    for(NXT_RenderItem_KeyValues::iter_t itor = valuelist.begin(); itor != valuelist.end(); ++itor){


        NXT_RenderItem_KeyValue& item = **itor;
        float* pvalue = &item.value_.front();
        char value_str[512];
        switch(item.type_){

            case NXT_Renderitem_KeyValueType_Color:
                snprintf(value_str, sizeof(value_str), "{x=0x%x,y=0x%x,z=0x%x,w=0x%x}", (int)pvalue[0], (int)pvalue[1], (int)pvalue[2], (int)pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Selection:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f,w=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Choice:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f,w=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Range:
                snprintf(value_str, sizeof(value_str), "%f", pvalue[0]);
                break;
            case NXT_Renderitem_KeyValueType_Rect:
                snprintf(value_str, sizeof(value_str), "{x0=%f,y0=%f,x1=%f,y1=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Point:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f}", pvalue[0], pvalue[1], pvalue[2]);
                break;
        };

        NXT_Theme_SetValue(renderer->getRenderItemManager(), item.key_.c_str(), value_str);
    }

    NXT_Theme_ApplyDefaultValues(renderer->getRenderItemManager());
}

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

void applyRenderItemOnTexture(NXT_HThemeRenderer renderer, NXT_Node_Texture *nodeData){

	if(renderer->basefilter_renderitem_id_ < 0){

		if(strlen(renderer->basefilter_renderitem_uid_) <= 0){

			return;
		}
	}

	if(NXT_ThemeRenderer_AquireContext(renderer) != NXT_Error_None)
		return;
	if(renderer->basefilter_renderitem_id_ < 0){

		if(strlen(renderer->basefilter_renderitem_uid_) <= 0){

			NXT_ThemeRenderer_ReleaseContext(renderer, 0);
			return;
		}

		int id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, renderer->basefilter_renderitem_uid_);
		if(id < 0){

			char* poutput = NULL;
			int length = 0;
			if(0 == renderer->loadFileCallback(&poutput, &length, renderer->basefilter_renderitem_uid_, renderer->imageCallbackPvtData)){

				NXT_ThemeRenderer_GetRenderItem(renderer, renderer->basefilter_renderitem_uid_, NULL, poutput, 0, renderer->loadFileCallback, renderer->imageCallbackPvtData);
				id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, renderer->basefilter_renderitem_uid_);
                if(poutput)
                    delete[] poutput;
                poutput = NULL;
			}
			else{

				NXT_ThemeRenderer_ReleaseContext(renderer, 0);
				return;				
			}
		}
		renderer->basefilter_renderitem_id_ = id;//NXT_ThemeRenderer_CreateRenderItem(renderer, id);

		if(renderer->basefilter_renderitem_id_ < 0){

			NXT_ThemeRenderer_ReleaseContext(renderer, 0);
			return;
		}
	}	

	NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    NXT_Vector4f savedColor(0.0f, 0.0f, 0.0f, 0.0f);
    NXT_Matrix4f texmat, colormat;

    NexThemeRenderer_PrepStateForRender2(renderer, nodeData, NULL, &texture, &mask, &texmat, &colormat, 0);
    texture->filtered_.serial_ = texture->serial_;
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, texture->filtered_.prender_target_);
    NXT_RenderTarget* prender_target = texture->filtered_.prender_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, 64, 64,  0, 0, 0);
    NXT_ThemeRenderer_SetRenderTarget(renderer, prender_target);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    static NXT_Matrix4f idmat(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );

    NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), renderer->basefilter_renderitem_id_, prender_target->width_, prender_target->height_, prender_target);
    NXT_ThemeRenderer_RISetParameterKeyValue(renderer, &renderer->basefilter_keyvalues_);
    if(texture){

        LOGI("[%s %d] prender_target->width_:%d prender_target->height_:%d texture->texName_for_rgb:%d", __func__, __LINE__, prender_target->width_, prender_target->height_, texture->texName_for_rgb);

        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                    texture->texName_for_rgb, 
                                                    texture->textureWidth, 
                                                    texture->textureHeight, 
                                                    texture->srcWidth,
                                                    texture->srcHeight,
                                                    E_TARGET_VIDEO_SRC);

        NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                    texture->texName_for_rgb, 
                                                    texture->textureWidth, 
                                                    texture->textureHeight, 
                                                    texture->srcWidth,
                                                    texture->srcHeight,
                                                    E_TARGET_VIDEO_LEFT);

        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), idmat.e, E_TARGET_VIDEO_SRC);
        NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), idmat.e, E_TARGET_VIDEO_LEFT);
        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), idmat.e, E_TARGET_VIDEO_SRC);
        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, texture);
        NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), idmat.e, E_TARGET_VIDEO_LEFT);
        NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, texture);
    }
    
    NXT_Theme_SetRect(renderer->getRenderItemManager(), -1, 1, 1, -1, 1);
    NXT_Theme_SetMatrix(renderer->getRenderItemManager(), idmat.e);
    int ret = NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
    NXT_Theme_ResetMatrix(renderer->getRenderItemManager());
    NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
    NXT_ThemeRenderer_SetRenderToDefault(renderer);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}

void setShaderInternal(
    NXT_HThemeRenderer renderer, 
    NXT_Node_Texture *texture,
    NXT_Node_Texture *mask,
    unsigned int bYUV,
    unsigned int bNV12,
    unsigned int bEGLI,
    unsigned int bSTEN,
    unsigned int bJPEG_ColorConv,
    unsigned int bVideo,
    NXT_TextureInfo *texinfo,
    NXT_TextureInfo *maskinfo,
    NXT_Matrix4f& mvp,
    NXT_Matrix4f& texmat,
    NXT_Matrix4f& back_texmat,
    NXT_Matrix4f& maskmat,
    int texName[3],
    NXT_PixelFormat textureFormat,
    int texName_for_rgb,
    int texName_for_back,
    NXT_ShaderProgram_Textured *sp_for_videotex,
    NXT_ShaderProgram_Masked *sp_for_masktex,
    NXT_ShaderProgram_Masked *sp_for_mask,
    NXT_ShaderProgram_Textured *sp_for_tex,
    NXT_ShaderProgram_Solid *sp_for_solid,
    const float* homography,
    const float* homography_mask
    ){

    if( texture 
     &&(texture->textureType==NXT_TextureType_Video)
     && texinfo && (texinfo->vignette > 0)
     && NXT_ThemeRenderer_GetVignetteTexID(renderer) 
     && (NULL == mask) &&(texture)){

        if(texture){//--------------- TEXTURE ONLY -----------------------------------------------------------------
        
            NXT_ShaderProgram_Textured *sp = NULL;
            
            sp = sp_for_videotex;

#ifdef LOAD_SHADER_ON_TIME
            NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
            glUseProgram( sp->shaderProgram );                                  CHECK_GL_ERROR();
#endif
            renderer->pTexturedShaderActive = sp;
            
            if( bSTEN ) {
                LOGI("[%s %d]USING SHADER//Texture+STENCIL (bind %d)", __func__, __LINE__, texName[0] );
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->tex_id_for_vignette_);   CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();

                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);   CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }                
            } else {
                LOGI("[%s %d]USING SHADER//Texture RGB (bind %d)", __func__, __LINE__, texName_for_rgb);
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName_for_rgb);                  CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->tex_id_for_vignette_);   CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);   CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));
                    CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                if( IS_VALID_UNIFORM(sp->u_colorconv) ) {
                    float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                    float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                    float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                    NXT_Matrix4f colorconv = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
                    glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
                }
            }
            
            glEnableVertexAttribArray( sp->a_position );                        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_tex_matrix, 1, GL_FALSE, texmat.e);        CHECK_GL_ERROR();
            glUniform1f(sp->u_alpha, renderer->alpha);                          CHECK_GL_ERROR();
            configureShaderForLighting(renderer,&sp->lightingInfo);
            setRealXY(sp->u_realX, sp->u_realY, texinfo, bEGLI);
            if (IS_VALID_UNIFORM(sp->u_hue))
            {
                glUniform1f(sp->u_hue, texinfo->hue);
                CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_moveY_value))
            {
                glUniform1f(sp->u_moveY_value, renderer->moveY_value);
                CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_moveX_value))
            {
                glUniform1f(sp->u_moveX_value, renderer->moveX_value);
                CHECK_GL_ERROR();
            }
            if(IS_VALID_ATTRIBUTE(sp->a_texCoord)){

                glEnableVertexAttribArray( sp->a_texCoord );                        CHECK_GL_ERROR();                
            }            
            if(homography && IS_VALID_UNIFORM(sp->u_homo_matrix)){

                glUniformMatrix3fv(sp->u_homo_matrix, 1, GL_FALSE, homography);     CHECK_GL_ERROR();
            }
            if(IS_VALID_UNIFORM(sp->u_back_tex_matrix)){

                glUniformMatrix4fv(sp->u_back_tex_matrix, 1, GL_FALSE, back_texmat.e);           CHECK_GL_ERROR();
            }
        }
     }
    else{
        if(mask && texture) { //--------------- MASK & TEXTURE -----------------------------------------------------------------
        
            NXT_ShaderProgram_Masked *sp = NULL;
            sp = sp_for_masktex;
            
#ifdef LOAD_SHADER_ON_TIME
            NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
            glUseProgram(sp->shaderProgram);                                    CHECK_GL_ERROR();
#endif
            renderer->pMaskedShaderActive = sp;
            
            if( bSTEN ) {
                LOGI("[%s %d]USING SHADER//Masked+Texture+STENCIL", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 1);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE1);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 1);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 2);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bYUV ) {
                LOGI("[%s %d]USING SHADER//Masked+Texture+YUV", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[1]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerU, 2);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[2]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerV, 3);                          CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE4);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 4);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE5);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 5);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bNV12 ) {
                float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                LOGI("[%s %d]USING SHADER//Masked+Texture+NV12", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[1]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerUV, 2);                         CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE3);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 3);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE4);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 4);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }                
                NXT_Matrix4f colorconv;
                colorconv = NXT_YUV2RGBWithColorAdjust(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1),bJPEG_ColorConv);
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();

            } else if(bEGLI) {
#ifdef EGL_SUPPORT
                float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                LOGI("[%s %d]USING SHADER//Masked+Texture+EGLI(%d,%d,%d,%d)", __func__, __LINE__, sp->u_textureSampler, sp->a_position, sp->a_texCoord, sp->a_maskCoord );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, texName[0]);             CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 1);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                NXT_Matrix4f colorconv;
                colorconv = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
#endif // EGL_SUPPORT
            } else {
                LOGI("[%s %d]USING SHADER//Masked+Texture", __func__, __LINE__ );
                float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName_for_rgb);                  CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 1);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                NXT_Matrix4f colorconv;
                colorconv = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, 
                    texinfo->contrast + renderer->adj_contrast, 
                    texinfo->saturation+renderer->adj_saturation, 
                    NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
            }
            
            NXT_Vector4f clearColor = (NXT_Vector4f){0.0,0.0,0.0,0.0};
            
            glActiveTexture(GL_TEXTURE0);                                                       CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, maskinfo->texName[0]);                                 CHECK_GL_ERROR();
            glUniform1i(sp->u_maskSampler, 0);                                                  CHECK_GL_ERROR();
            glUniform4fv(sp->u_color, 1, clearColor.e);                                         CHECK_GL_ERROR();
            glEnableVertexAttribArray( sp->a_position );                                        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);                           CHECK_GL_ERROR();            
            glUniformMatrix4fv(sp->u_tex_matrix, 1, GL_FALSE, texmat.e);                        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mask_matrix, 1, GL_FALSE, maskmat.e);                      CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_maskxfm, 1, GL_FALSE, renderer->mask_sample_transform.e);  CHECK_GL_ERROR();
            glUniform1f(sp->u_alpha, renderer->alpha);                                          CHECK_GL_ERROR();
            if(IS_VALID_ATTRIBUTE(sp->a_texCoord)){

                glEnableVertexAttribArray( sp->a_texCoord );                                    CHECK_GL_ERROR();
            }
            
            if(IS_VALID_ATTRIBUTE(sp->a_maskCoord)){

                glEnableVertexAttribArray( sp->a_maskCoord );                                   CHECK_GL_ERROR();
            }
            if(homography && IS_VALID_UNIFORM(sp->u_homo_matrix)){

                glUniformMatrix3fv(sp->u_homo_matrix, 1, GL_FALSE, homography);                 CHECK_GL_ERROR();
            }

            if(homography_mask && IS_VALID_UNIFORM(sp->u_mask_homo_matrix)){

                glUniformMatrix3fv(sp->u_mask_homo_matrix, 1, GL_FALSE, homography_mask);       CHECK_GL_ERROR();
            }
            if(IS_VALID_UNIFORM(sp->u_back_tex_matrix)){

                glUniformMatrix4fv(sp->u_back_tex_matrix, 1, GL_FALSE, back_texmat.e);           CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_hue))
            {
                glUniform1f(sp->u_hue, texinfo->hue);
                CHECK_GL_ERROR();
            }
            
        } else if(mask) { //--------------- MASK ONLY -----------------------------------------------------------------

            NXT_ShaderProgram_Masked *sp = NULL;
                            
            sp = sp_for_mask;

#ifdef LOAD_SHADER_ON_TIME
            NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
            glUseProgram(sp->shaderProgram);                                    CHECK_GL_ERROR();
#endif
            renderer->pMaskedShaderActive = sp;
            
            LOGI("[%s %d]USING SHADER//Mask Only (%.3f, %.3f, %.3f, %.3f) (bind %d)", __func__, __LINE__, renderer->color.e[0], renderer->color.e[1], renderer->color.e[2], renderer->color.e[3], maskinfo->texName[0] );

            if( bSTEN ) {
                LOGI("[%s %d]USING SHADER//Masked+STENCIL", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 1);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bYUV ) {
                LOGI("[%s %d]USING SHADER//Masked+YUV", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerU, 2);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerV, 3);                          CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE4);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 4);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    
                    glActiveTexture(GL_TEXTURE5);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 5);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bNV12 ) {
                LOGI("[%s %d]USING SHADER//Masked+NV12", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 1);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerUV, 2);                         CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE3);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 3);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE4);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 4);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else {
                LOGI("[%s %d]USING SHADER//Masked", __func__, __LINE__ );
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, renderer->getContextManager()->emptyTexture);           CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 1);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            }

            glActiveTexture(GL_TEXTURE0);                                       CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, maskinfo->texName_for_rgb);            CHECK_GL_ERROR();
            glUniform1i(sp->u_maskSampler, 0);                                  CHECK_GL_ERROR();
            glUniform4fv(sp->u_color, 1, renderer->color.e);                    CHECK_GL_ERROR();
            glEnableVertexAttribArray( sp->a_position );                        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_tex_matrix, 1, GL_FALSE, texmat.e);        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mask_matrix, 1, GL_FALSE, maskmat.e);      CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_maskxfm, 1, GL_FALSE, renderer->mask_sample_transform.e);      CHECK_GL_ERROR();
            glUniform1f(sp->u_alpha, renderer->alpha);                                              CHECK_GL_ERROR();
            if(IS_VALID_ATTRIBUTE(sp->a_texCoord)){

                glEnableVertexAttribArray( sp->a_texCoord );                                        CHECK_GL_ERROR();
            }
            
            if(IS_VALID_ATTRIBUTE(sp->a_maskCoord)){

                glEnableVertexAttribArray( sp->a_maskCoord );                                       CHECK_GL_ERROR();
            }
            if(homography && IS_VALID_UNIFORM(sp->u_homo_matrix)){

                glUniformMatrix3fv(sp->u_homo_matrix, 1, GL_FALSE, homography);                     CHECK_GL_ERROR();
            }

            if(homography_mask && IS_VALID_UNIFORM(sp->u_mask_homo_matrix)){

                glUniformMatrix3fv(sp->u_mask_homo_matrix, 1, GL_FALSE, homography_mask);           CHECK_GL_ERROR();
            }
            if(IS_VALID_UNIFORM(sp->u_back_tex_matrix)){

                glUniformMatrix4fv(sp->u_back_tex_matrix, 1, GL_FALSE, back_texmat.e);              CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_hue))
            {
                glUniform1f(sp->u_hue, 0.0f);
                CHECK_GL_ERROR();
            }
        
        } else if( texture ) { //--------------- TEXTURE ONLY -----------------------------------------------------------------
            
            NXT_ShaderProgram_Textured *sp = NULL;
            
            sp = sp_for_tex;

#ifdef LOAD_SHADER_ON_TIME
            NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
            glUseProgram( sp->shaderProgram );                                  CHECK_GL_ERROR();
#endif
            renderer->pTexturedShaderActive = sp;
            
            if( bSTEN ) {
                LOGI("[%s %d]USING SHADER//Texture+STENCIL (bind %d)", __func__, __LINE__, texName[0] );
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);                       CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE1);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 1);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 2);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bYUV ) {
                LOGI("[%s %d]USING SHADER//Texture+YUV bind %d,%d,%d", __func__, __LINE__, texName[0], texName[1], texName[2] );
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);              CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 0);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[1]);              CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerU, 1);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[2]);              CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerV, 2);                          CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE3);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 3);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE4);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 4);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
            } else if( bNV12 ) {
                LOGI("[%s %d]USING SHADER//Texture+NV12 (0x%08x, %d,%d,%d,%d) bind %d,%d", __func__, __LINE__, (unsigned int)(size_t)sp, texName[0], texName[1], (unsigned int)sp->u_textureSamplerY, (unsigned int)sp->u_textureSamplerUV, texName[0], texName[1] );
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[0]);              CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerY, 0);                          CHECK_GL_ERROR();
                glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName[1]);              CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSamplerUV, 1);                         CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE2);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 2);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                //            NXT_Matrix4f colorconv = NXT_YUV2RGBWithColorAdjust(0, 0, -0.2, (NXT_Vector4f){{64.0/255.0,48.0/255.0,24.0/255.0,1}});
                float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                NXT_Matrix4f colorconv;
                colorconv = NXT_YUV2RGBWithColorAdjust(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1),bJPEG_ColorConv);
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
            } else if(bEGLI) {
#ifdef EGL_SUPPORT
                LOGI("[%s %d]USING SHADER//Texture+EGLI (bind %d)", __func__, __LINE__, texName[0] );
                float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, texName[0]);    CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE1);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 1);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE3);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));   CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 3);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                NXT_Matrix4f colorconv;
                colorconv = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
#endif
            } else {

                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, texName_for_rgb);                  CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
                if(IS_VALID_UNIFORM(sp->u_back_textureSampler)){

                    glActiveTexture(GL_TEXTURE1);                               CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, texName_for_back);             CHECK_GL_ERROR();
                    glUniform1i(sp->u_back_textureSampler, 1);                  CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                    glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
                    glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));                    CHECK_GL_ERROR();
                    glUniform1i(sp->u_textureSampler_for_mask, 2);                          CHECK_GL_ERROR();
                }
                if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
                }
                if( IS_VALID_UNIFORM(sp->u_colorconv) ) {

                    float tintRed   = ((float)((texinfo->tintColor >> 16)&0xFF))/255.0;
                    float tintGreen = ((float)((texinfo->tintColor >>  8)&0xFF))/255.0;
                    float tintBlue  = ((float)((texinfo->tintColor      )&0xFF))/255.0;
                    NXT_Matrix4f colorconv = NXT_ColorAdjustRGB(texinfo->brightness + renderer->adj_brightness, texinfo->contrast + renderer->adj_contrast, texinfo->saturation+renderer->adj_saturation, NXT_Vector4f(tintRed+renderer->adj_tintColor.e[0],tintGreen+renderer->adj_tintColor.e[1],tintBlue+renderer->adj_tintColor.e[2],1));
                    glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
                }
            }
                
            glEnableVertexAttribArray( sp->a_position );                        CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_tex_matrix, 1, GL_FALSE, texmat.e);        CHECK_GL_ERROR();
            glUniform1f(sp->u_alpha, renderer->alpha);                          CHECK_GL_ERROR();
            configureShaderForLighting(renderer,&sp->lightingInfo);
            setRealXY(sp->u_realX, sp->u_realY, texinfo, bEGLI);
            if (IS_VALID_UNIFORM(sp->u_moveY_value))
            {
                glUniform1f(sp->u_moveY_value, renderer->moveY_value);
                CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_moveX_value))
            {
                glUniform1f(sp->u_moveX_value, renderer->moveX_value);
                CHECK_GL_ERROR();
            }
            if(IS_VALID_ATTRIBUTE(sp->a_texCoord)){

                glEnableVertexAttribArray( sp->a_texCoord );                                    CHECK_GL_ERROR();
            }
            
            if(homography && IS_VALID_UNIFORM(sp->u_homo_matrix)){

                glUniformMatrix3fv(sp->u_homo_matrix, 1, GL_FALSE, homography);                 CHECK_GL_ERROR();
            }
            if(IS_VALID_UNIFORM(sp->u_back_tex_matrix)){

                glUniformMatrix4fv(sp->u_back_tex_matrix, 1, GL_FALSE, back_texmat.e);           CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_hue))
            {
                glUniform1f(sp->u_hue, texinfo->hue);
                CHECK_GL_ERROR();
            }
        } else {  //--------------- SOLID -----------------------------------------------------------------
                
            NXT_ShaderProgram_Solid *sp = NULL;
            sp = sp_for_solid;
            
#ifdef LOAD_SHADER_ON_TIME
            NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
            glUseProgram( sp->shaderProgram );                                  CHECK_GL_ERROR();
#endif
            renderer->pSolidShaderActive    = sp;
            glEnableVertexAttribArray( sp->a_position );                        CHECK_GL_ERROR();
            glEnableVertexAttribArray( sp->a_color );                           CHECK_GL_ERROR();
            glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
            glUniform1f(sp->u_alpha, renderer->alpha);                          CHECK_GL_ERROR();
            
            configureShaderForLighting(renderer,&sp->lightingInfo);

            if(IS_VALID_UNIFORM(sp->u_colorconv)){

                NXT_Matrix4f colorconv;
                colorconv = NXT_ColorAdjustRGB(renderer->adj_brightness, renderer->adj_contrast, renderer->adj_saturation, NXT_Vector4f(renderer->adj_tintColor.e[0],renderer->adj_tintColor.e[1],renderer->adj_tintColor.e[2],1));   
                glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_textureSampler_for_mask)){
                glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
                glBindTexture(GL_TEXTURE_2D, NXT_ThemeRenderer_GetTextureNameForMask(renderer));                    CHECK_GL_ERROR();
                glUniform1i(sp->u_textureSampler_for_mask, 0);                          CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_inverse_Mask)){
                    glUniform1f(sp->u_inverse_Mask, renderer->renderTest == NXT_RenderTest_NegMask ? 1.0 : 0.0 );    CHECK_GL_ERROR();
            }
            if (IS_VALID_UNIFORM(sp->u_hue))
            {
                glUniform1f(sp->u_hue, 0);
                CHECK_GL_ERROR();
            }
        }
    }
}

//0 not processed
//1 normal
//2 full

int NexThemeRenderer_PrepStateForRender(NXT_HThemeRenderer renderer, 
                                         NXT_Node_Texture *texture,
                                         NXT_Node_Texture *mask,
                                         NXT_TextureInfo **useTexture,
                                         NXT_TextureInfo **useMask,
                                         unsigned int fullmode,
                                         const float* homography,
                                         const float* homography_mask)
{
	int ret = 0;
    if( !renderer ) {
        // ERROR: null pointer
        return ret;
    }
    ret = 1;

    if( !texture && renderer->texture )
        texture = renderer->texture;
    if( !mask && renderer->mask )
        mask = renderer->mask;
    if( !useMask )
        mask = NULL;
    if( !useTexture )
        texture = NULL;
        
    NXT_TextureInfo *texinfo = texture?&texture->texinfo_preview:NULL;
    NXT_TextureInfo *maskinfo = mask?&mask->texinfo_preview:NULL;
    
    NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->basetransform, NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform));
                
    renderer->pSolidShaderActive    = NULL;
    renderer->pTexturedShaderActive = NULL;
    renderer->pMaskedShaderActive   = NULL;

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    switch(renderer->compMode) {
        case NXT_CompMode_Multiply:
            glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            break;
        case NXT_CompMode_Screen:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            break;
        case NXT_CompMode_Diff:
            glBlendEquationSeparate(GL_FUNC_SUBTRACT,GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case NXT_CompMode_Normal:
        default:
            glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
    CHECK_GL_ERROR();

	/*
    LOGI("MVP MATRIX:\n"
         "[%f %f %f %f\n"
         " %f %f %f %f\n"
         " %f %f %f %f\n"
         " %f %f %f %f]\n",
         mvp.e[0],  mvp.e[1],  mvp.e[2],  mvp.e[3],
         mvp.e[4],  mvp.e[5],  mvp.e[6],  mvp.e[7],
         mvp.e[8],  mvp.e[9],  mvp.e[10], mvp.e[11],
         mvp.e[12], mvp.e[13], mvp.e[14], mvp.e[15]
         );
    */
    unsigned int bYUV = 0;
    unsigned int bNV12 = 0;
    unsigned int bEGLI = 0;
    unsigned int bSTEN = 0;
	unsigned int bJPEG_ColorConv = 0;
	unsigned int bVideo = 0;
    
    int texid = -1;
    if(texture && texture->textureType==NXT_TextureType_Video && (!renderer->videoPlaceholder1 || !renderer->videoPlaceholder2) )
    {
    	bVideo = 1;
        if( renderer->bSwapV ) {
            switch(texture->videoSource) {
                case  1: texid = NXT_TextureID_Video_2; break;
                case  2: texid = NXT_TextureID_Video_1; break;
                default: texid = NXT_TextureID_Video_2; break;
            }
        } else {
            switch(texture->videoSource) {
                case  1: texid = NXT_TextureID_Video_1; break;
                case  2: texid = NXT_TextureID_Video_2; break;
                default: texid = NXT_TextureID_Video_1; break;
            }
        }
		
        if(renderer->pActiveEffect && renderer->cur_effect_overlap <= 0 ) 
        {
        	// Override texture ID for effects that do not overlap (always use texture 1)
        	texid = NXT_TextureID_Video_1;
        }
		
        LOGI("[%s %d] is video (srcval=%d texid=%d/%s)", __func__, __LINE__, texture->videoSource, texid, (texid==NXT_TextureID_Video_1?"NXT_TextureID_Video_1":(texid==NXT_TextureID_Video_2?"NXT_TextureID_Video_2":"???")) );
        if( renderer->getVideoSrc(texid).bValidTexture ) {
            LOGI("[%s %d] valid texture", __func__, __LINE__ );
            // We have a valid video texture; use it rather than any placeholder
            texinfo = &renderer->getVideoSrc(texid);
        }
    }
    
    if( texinfo ) {
        if( texinfo->textureFormat == NXT_PixelFormat_YUV ) {
            LOGI("[%s %d] (#%d=%d) is YUV", __func__, __LINE__, texid, texinfo->textureFormat );
            bYUV = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_NV12) {
            LOGI("[%s %d] (#%d=%d) is NV12", __func__, __LINE__, texid, texinfo->textureFormat );
            bNV12 = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_NV12_JPEG) {
            LOGI("[%s %d] (#%d=%d) is NV12", __func__, __LINE__, texid, texinfo->textureFormat );
            bNV12 = 1;
            bJPEG_ColorConv = 1;
        } else if(texinfo->textureFormat == NXT_PixelFormat_EGLImage) {
            LOGI("[%s %d] (#%d=%d) is EGLImage", __func__, __LINE__, texid, texinfo->textureFormat );
            bEGLI = 1;
        }
    }

    
    if( renderer->renderDest == NXT_RenderDest_Stencil )
        bSTEN = 1;
    
    NXT_Matrix4f texmat = renderer->texture_transform;//!!!
    NXT_Matrix4f back_texmat = renderer->texture_transform;//!!!
    NXT_Matrix4f maskmat = renderer->mask_transform;//!!!

    int texName[3];
    NXT_PixelFormat textureFormat = NXT_PixelFormat_NONE;
    int texName_for_rgb = 0;
    int texName_for_back = 0;
    texName[0] = 0;
    texName[1] = 0;
    texName[2] = 0;
    
    if( texinfo ) {//!!!

    	textureFormat = texinfo->textureFormat;
	    texName_for_rgb = texinfo->texName_for_rgb;
	    texName[0] = texinfo->texName[0];
	    texName[1] = texinfo->texName[1];
	    texName[2] = texinfo->texName[2];        
        
        if( texinfo->bHasInputRect ) {
            
#if 0
            // The final input rectangle is calculated as a proprotion of the texture width and
            // height, but at this point we only have pixel coordinates.  In order to convern them,
            // we need the original width and height, which are backwards if the texture has been
            // rotated 90 or 270 degrees.
            float rotatedPixelWidth, rotatedPixelHeight;
            if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                rotatedPixelWidth = texinfo->srcHeight;
                rotatedPixelHeight = texinfo->srcWidth;
            } else {
                rotatedPixelWidth = texinfo->srcWidth;
                rotatedPixelHeight = texinfo->srcHeight;
            }
            
            // Calculate proportional coordinates for input rectangle
            float left = texinfo->left / rotatedPixelWidth;
            float right = texinfo->right / rotatedPixelWidth;
            float top = texinfo->top / rotatedPixelHeight;
            float bottom = texinfo->bottom / rotatedPixelHeight;
#else
            // Calculate proportional coordinates for input rectangle
            float left = (float)texinfo->left / 100000.0f;
            float right = (float)texinfo->right / 100000.0f;
            float top = (float)texinfo->top / 100000.0f;
            float bottom = (float)texinfo->bottom / 100000.0f;
#endif
            if(fullmode){

                if(strlen(renderer->basefilter_renderitem_uid_) <= 0){

                }
                else if(strcasecmp(renderer->basefilter_renderitem_uid_, "black") == 0){
                    
                }
                else{

                    float width = (float)texinfo->textureWidth;
                    float height = (float)texinfo->textureHeight;

                    if(texinfo->rotation == 90 || texinfo->rotation == 270){

                        float tmp = width;
                        width = height;
                        height = tmp;
                    }

                    float user_translate_x = (float)texinfo->translate_x * width / 100000.0f;
                    float user_translate_y = (float)texinfo->translate_y * height / 100000.0f;

                    float r_left = width * left;
                    float r_right = width * right;
                    float r_top = top * height;
                    float r_bottom = bottom * height;

                    float cx = (r_right - r_left) * 0.5f;
                    float cy = (r_top - r_bottom) * 0.5f;
                    float wnd_left = -cx;
                    float wnd_top = -cy;
                    float wnd_right = cx;
                    float wnd_bottom = cy;

                    float user_rotate = -(float)(texinfo->user_rotation % 360) * PI_OVER_180;
                    float cos_val = (float)cos(user_rotate);
                    float sin_val = (float)sin(user_rotate);

                    float pos0_x = cos_val * wnd_left - sin_val * wnd_top;
                    float pos0_y = sin_val * wnd_left + cos_val * wnd_top;
                    float pos1_x = cos_val * wnd_right - sin_val * wnd_top;
                    float pos1_y = sin_val * wnd_right + cos_val * wnd_top;
                    float pos2_x = cos_val * wnd_left - sin_val * wnd_bottom;
                    float pos2_y = sin_val * wnd_left + cos_val * wnd_bottom;
                    float pos3_x = cos_val * wnd_right - sin_val * wnd_bottom;
                    float pos3_y = sin_val * wnd_right + cos_val * wnd_bottom;
                    
                    float new_lt_x = std::min(std::min(std::min(pos0_x, pos1_x), pos2_x), pos3_x);
                    float new_lt_y = std::min(std::min(std::min(pos0_y, pos1_y), pos2_y), pos3_y);
                    float new_rb_x = std::max(std::max(std::max(pos0_x, pos1_x), pos2_x), pos3_x);
                    float new_rb_y = std::max(std::max(std::max(pos0_y, pos1_y), pos2_y), pos3_y);

                    float aabb_left = ((new_lt_x + cx) + user_translate_x + r_left);
                    float aabb_top = (new_lt_y + cy) + user_translate_y + r_bottom;
                    float aabb_right = (new_rb_x + cx) + user_translate_x + r_left;
                    float aabb_bottom = (new_rb_y + cy) + user_translate_y + r_bottom;

                    aabb_left /= width;
                    aabb_right /= width;
                    aabb_top /= height;
                    aabb_bottom /= height;

                    if( (aabb_left      <= 0.0f || aabb_left     >= 1.0f) ||
                        (aabb_right     <= 0.0f || aabb_right    >= 1.0f) ||
                        (aabb_top       <= 0.0f || aabb_top      >= 1.0f) ||
                        (aabb_bottom    <= 0.0f || aabb_bottom   >= 1.0f) )
                    {
                        if(strcasecmp(renderer->basefilter_renderitem_uid_, "white") == 0){

                            texName_for_back = renderer->getContextManager()->whiteTexture;
                        }
                        else if(strcasecmp(renderer->basefilter_renderitem_uid_, "transparent") == 0){

                            texName_for_back = renderer->getContextManager()->emptyTexture;
                        }
                        else{

                            texName_for_back = renderer->getContextManager()->emptyTexture;

                            if( (aabb_left      < 0.0f || aabb_left     > 1.0f) ||
                                (aabb_right     < 0.0f || aabb_right    > 1.0f) ||
                                (aabb_top       < 0.0f || aabb_top      > 1.0f) ||
                                (aabb_bottom    < 0.0f || aabb_bottom   > 1.0f) ){

                                if(texinfo->serial_ != texinfo->filtered_.serial_){

                                    applyRenderItemOnTexture(renderer, texture);
                                }
                                if(texinfo->filtered_.prender_target_)
                                    texName_for_back = texinfo->filtered_.prender_target_->target_texture_;
                            }
                        }
                        
                        if(texName_for_back){

                            float view_right = right > 1.0f?1.0f:(right < 0.0f)?0.0f:right;
                            float view_left = left > 1.0f?1.0f:(left < 0.0f)?0.0f:left;
                            float view_top = top > 1.0f?1.0f:(top < 0.0f)?0.0f:top;
                            float view_bottom = bottom > 1.0f?1.0f:(bottom < 0.0f)?0.0f:bottom;
                            float view_cx = ((view_right + view_left) * 0.5f - left) / (right - left);
                            float view_cy = ((view_top + view_bottom) * 0.5f - bottom) / (top - bottom);
                            float w_scale = (right - left) / (view_right - view_left);
                            float h_scale = (top - bottom) / (view_top - view_bottom);
                            float scale = (w_scale > h_scale)?w_scale:h_scale;
                            scale = 1.0f / scale;
                            
                            // Translate and scale our rectangle to encompass exactly the required area
                            NXT_pMatrix4f_TranslateX(&back_texmat, -0.5);
                            NXT_pMatrix4f_TranslateY(&back_texmat, -0.5);
                            NXT_pMatrix4f_ScaleX(&back_texmat, scale);
                            NXT_pMatrix4f_ScaleY(&back_texmat, scale);
                            NXT_pMatrix4f_TranslateX(&back_texmat, view_cx);
                            NXT_pMatrix4f_TranslateY(&back_texmat, view_cy);
                            ret = 2;
                        }
                    }
                }
            }
            if(texinfo->user_rotation % 360)
            {
                NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
                NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );

                float tex_width = (float)texinfo->textureWidth;
                float tex_height = (float)texinfo->textureHeight;
                if(texinfo->rotation == 90 || texinfo->rotation == 270){

                    float tmp = tex_width;
                    tex_width = tex_height;
                    tex_height = tmp;
                }
                float half_ortho_w = 0.5f * tex_width * (right - left);
                float half_ortho_h = 0.5f * tex_height * (top - bottom);
                NXT_Matrix4f ortho = NXT_Matrix4f_Ortho(-half_ortho_w, half_ortho_w, -half_ortho_h, half_ortho_h, -1.0f, 1.0f);
                NXT_pMatrix4f_ScaleX(&texmat, half_ortho_w * 2.0f);
                NXT_pMatrix4f_ScaleY(&texmat, half_ortho_h * 2.0f);
                texmat = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Rotate(NXT_Vector4f(0.0,0.0,1.0,0.0), texinfo->user_rotation*PI_OVER_180), texmat);
                texmat = NXT_Matrix4f_MultMatrix(ortho, texmat);
                NXT_pMatrix4f_ScaleX(&texmat, 0.5f);
                NXT_pMatrix4f_ScaleY(&texmat, 0.5f);
                NXT_pMatrix4f_TranslateX(   &texmat, 0.5 );
                NXT_pMatrix4f_TranslateY(   &texmat, 0.5 );
            }

            NXT_pMatrix4f_ScaleX( &texmat, right-left );
            NXT_pMatrix4f_ScaleY( &texmat, top-bottom );
            NXT_pMatrix4f_TranslateX( &texmat, left + (float)texinfo->translate_x / 100000.0f);
            NXT_pMatrix4f_TranslateY( &texmat, bottom + (float)texinfo->translate_y / 100000.0f);

            

            // Rotate the texture coordinates.  This is done prior to scaling to match
            // the actual used area, which means the texture will be distorted; this is fixed
            // up later on when we crop to the input rectangle.
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            texmat = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Rotate(NXT_Vector4f(0.0,0.0,1.0,0.0), texinfo->rotation*PI_OVER_180), texmat);
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            // Handle mirroring
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            if( texinfo->mirror_h ) {
                NXT_pMatrix4f_ScaleX( &texmat, -1 );
            }
            if( texinfo->mirror_v ) {
                NXT_pMatrix4f_ScaleY( &texmat, -1 );
            }
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            // The actual image only takes up a portion of the texture, so we need to scale the
            // texture coordinates down from 1, 1 to enclose the actual portion that's used.
            float scalex = (float)texinfo->srcWidth / (float)texinfo->textureWidth;
            float scaley = (float)texinfo->srcHeight / (float)texinfo->textureHeight;
            
            NXT_pMatrix4f_ScaleX( &texmat, scalex );
            NXT_pMatrix4f_ScaleY( &texmat, scaley );

                    
            if( texinfo->bUseSurfaceTexture ) 
            {

            } else {
#ifdef ANDROID 
                NXT_pMatrix4f_TranslateX(   &texmat, TEX_BORDER_SIZE / (float)texinfo->textureWidth);
            	NXT_pMatrix4f_TranslateY(   &texmat, TEX_BORDER_SIZE / (float)texinfo->textureHeight);
#elif __APPLE__ // NESI-331
                if((texinfo->textureWidth != texinfo->srcWidth)||(texinfo->textureHeight != texinfo->srcHeight)) {
                    NXT_pMatrix4f_TranslateX(   &texmat, TEX_BORDER_SIZE / (float)texinfo->textureWidth);
            	    NXT_pMatrix4f_TranslateY(   &texmat, TEX_BORDER_SIZE / (float)texinfo->textureHeight);
                }
#endif
            }

            if(texName_for_back){

                back_texmat = NXT_Matrix4f_MultMatrix(texmat,back_texmat);
            }
            if( texinfo->bUseSurfaceTexture && bEGLI) 
            {
                LOGD("[%s %d] Using surface texture matrix", __func__, __LINE__ );
                texmat = NXT_Matrix4f_MultMatrix(texinfo->surfaceMatrix, texmat);
                if(texName_for_back)
                    back_texmat = NXT_Matrix4f_MultMatrix(texinfo->surfaceMatrix, back_texmat);
            }
            
        }else{

            float inputWidth = texinfo->right - texinfo->left;
            float textureWidth = (float)texinfo->textureWidth;
            float inputHeight = texinfo->top - texinfo->bottom;
            float srcHeight = (float)texinfo->srcHeight;
            float textureHeight = (float)texinfo->textureHeight;
            float left = texinfo->left;
            float bottom = texinfo->bottom;
            
            if( !texinfo->bUseSurfaceTexture ) {
                inputWidth -= TWICE_TEX_BORDER_SIZE;
                inputHeight -= TWICE_TEX_BORDER_SIZE;
            }
            
            if ( texinfo->bAnimated ) {
                
                unsigned int fnum = (unsigned int)renderer->animframe;
                //if( fnum < 0 ) // no meaning. these two lines will be removed.
                //    fnum = 0;
                
                fnum %= texinfo->animFrames;
                
                unsigned int row = (fnum/texinfo->animCols);
                unsigned int col = fnum%texinfo->animCols;
                float colWidth = (1.0/(float)texinfo->animCols);
                float rowHeight = (1.0/(float)texinfo->animRows);

                LOGV("[%s %d] animated texture : row(%u) col(%u) rowHeight(%f) colWidth(%f) animRows(%d) animColumns(%d) animFrame(%f)->fnum(%u) ", __func__, __LINE__, row, col, rowHeight, colWidth, texinfo->animRows, texinfo->animCols, renderer->animframe, fnum );

                NXT_pMatrix4f_ScaleX(       &texmat, colWidth );
                NXT_pMatrix4f_ScaleY(       &texmat, rowHeight );
                NXT_pMatrix4f_TranslateX(   &texmat, colWidth*col );
                NXT_pMatrix4f_TranslateY(   &texmat, rowHeight*row );
                
            }
            
            NXT_pMatrix4f_TranslateX(   &texmat, -0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, -0.5 );
            
            if( texinfo->bHasInputRect ) {
                if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                    float swap;
                    
                    swap = inputWidth;
                    inputWidth = inputHeight;
                    inputHeight = swap;
                    
                    swap = left;
                    left = inputHeight-bottom;
                    bottom = -swap;
                }
            } else if( texinfo->fitInWidth>0 && texinfo->fitInHeight>0 && inputWidth>32 && inputHeight>32 ) {
            
                float fitInWidth = (float)texinfo->fitInWidth;
                float fitInHeight = (float)texinfo->fitInHeight;
                float scaledInputWidth;
                float scaledInputHeight;

                if( texinfo->rotation==90 || texinfo->rotation==270 ) {
                    scaledInputHeight = inputWidth;
                    scaledInputWidth = inputHeight;
                } else {
                    scaledInputWidth = inputWidth;
                    scaledInputHeight = inputHeight;
                }

                if( scaledInputWidth/fitInWidth > scaledInputHeight/fitInHeight ) {
                    scaledInputHeight *= (fitInWidth/scaledInputWidth);
                    scaledInputWidth = fitInWidth;
                } else {
                    scaledInputWidth *= (fitInHeight/scaledInputHeight);
                    scaledInputHeight = fitInHeight;
                }
                
            // for rotate ratio problem.
                NXT_pMatrix4f_ScaleX(       &texmat, (fitInWidth/scaledInputWidth) );
                NXT_pMatrix4f_ScaleY(       &texmat, (fitInHeight/scaledInputHeight) );
                // NXT_pMatrix4f_ScaleX(       &texmat, (1.0-scaledInputWidth/fitInWidth)*2.0+1.0 );
                // NXT_pMatrix4f_ScaleY(       &texmat, (1.0-scaledInputHeight/fitInHeight)*2.0+1.0 );
                
            }
            
            if( texinfo->rotation!=0 ) {
                texmat = NXT_Matrix4f_MultMatrix( NXT_Matrix4f_Rotate(NXT_Vector4f(0.0,0.0,1.0,0.0), texinfo->rotation*PI_OVER_180), texmat);
            }
            
            if( texinfo->mirror_h ) {
                NXT_pMatrix4f_ScaleX( &texmat, -1 );
            }
            if( texinfo->mirror_v ) {
                NXT_pMatrix4f_ScaleY( &texmat, -1 );
            }
            
            
            NXT_pMatrix4f_TranslateX(   &texmat, +0.5 );
            NXT_pMatrix4f_TranslateY(   &texmat, +0.5 );

            
            
            
            if( texinfo->bUseSurfaceTexture ) 
            {
                LOGD("[%s %d] Using surface texture matrix", __func__, __LINE__ );
                texmat = NXT_Matrix4f_MultMatrix(texinfo->surfaceMatrix, texmat);
            } else {
                LOGI("[%s %d] Computing texture matrix", __func__, __LINE__ );
                NXT_pMatrix4f_ScaleX(       &texmat, inputWidth / textureWidth);
                NXT_pMatrix4f_ScaleY(       &texmat, inputHeight / textureHeight );
                NXT_pMatrix4f_TranslateX(   &texmat, (left+TEX_BORDER_SIZE) / textureWidth );
                NXT_pMatrix4f_TranslateY(   &texmat, (bottom+TEX_BORDER_SIZE) / textureHeight );
            }
        }
        
    }
    
    if( texinfo ) {
        LOGI("[%s %d] texinfo l/b/r/t=%f/%f/%f/%f src w/h=%d/%d tex w/h=%d/%d format=%d bYUV=%d bNV12=%d renderer->outputType=%d mask=0x%08x texture=0x%08x", __func__, __LINE__, texinfo->left, texinfo->bottom, texinfo->right, texinfo->top, texinfo->srcWidth, texinfo->srcHeight, texinfo->textureWidth, texinfo->textureHeight, texinfo->textureFormat, bYUV, bNV12, renderer->outputType, (unsigned int)(size_t)mask, (unsigned int)(size_t)texture );
    } else {
        LOGI("[%s %d] NULL texinfo", __func__, __LINE__ );
    }

    bool screen_masking = (renderer->renderTest == NXT_RenderTest_Mask || renderer->renderTest == NXT_RenderTest_NegMask);

    NXT_ShaderProgram_Textured *sp_for_videotex = NULL;
    NXT_ShaderProgram_Masked *sp_for_masktex = NULL;
    NXT_ShaderProgram_Masked *sp_for_mask = NULL;
    NXT_ShaderProgram_Textured *sp_for_tex = NULL;
    NXT_ShaderProgram_Solid *sp_for_solid = NULL;

    NXT_ThemeRenderer_Context* pctx = renderer->getContextManager();

    if(screen_masking){
        if (texName_for_back)
        {
            if (homography || homography_mask)
            {
                sp_for_videotex = &pctx->screenMaskingType_texturedShaderRGB_Combined_vignette_Homo;
                sp_for_masktex = bSTEN ? &pctx->screenMaskingType_maskedShaderStencil_Homo : &pctx->screenMaskingType_maskedShaderRGB_Combined_Homo;
                sp_for_mask = bSTEN ? &pctx->screenMaskingType_maskedShaderStencil_Homo : &pctx->screenMaskingType_maskedShaderRGB;
                sp_for_tex = bSTEN ? &pctx->screenMaskingType_texturedShaderStencil_Homo : &pctx->screenMaskingType_texturedShaderRGB_Combined_Homo;
                sp_for_solid = &pctx->screenMaskingType_solidShaderRGB;
            }
            else
            {
                if (bYUV)
                    sp_for_videotex = &pctx->screenMaskingType_texturedShaderYUV_vignette; //jeff log not support
                else if (bNV12)
                    sp_for_videotex = &pctx->screenMaskingType_texturedShaderNV12_Combined_vignette;
                else if (bEGLI)
                    sp_for_videotex = &pctx->screenMaskingType_texturedShaderExternal_Combined_vignette;
                else
                    sp_for_videotex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette : &pctx->screenMaskingType_texturedShaderRGB_vignette; //jeff log when not video, do not support

                if (bSTEN)
                    sp_for_masktex = &pctx->screenMaskingType_maskedShaderStencil; //jeff log not support
                else if (bYUV)
                    sp_for_masktex = &pctx->screenMaskingType_maskedShaderYUV; //jeff log not support
                else if (bNV12)
                    sp_for_masktex = &pctx->screenMaskingType_maskedShaderNV12_Combined;
                else if (bEGLI)
                    sp_for_masktex = &pctx->screenMaskingType_maskedShaderExternal_Combined;
                else
                    sp_for_masktex = &pctx->screenMaskingType_maskedShaderRGB_Combined;

                if (bSTEN)
                    sp_for_mask = &pctx->screenMaskingType_maskedShaderStencil;
                else if (bYUV)
                    sp_for_mask = &pctx->screenMaskingType_maskedShaderYUV; //jeff log not support
                else if (bNV12)
                    sp_for_mask = &pctx->screenMaskingType_maskedShaderNV12_Combined;
                else if (bEGLI)
                    sp_for_mask = &pctx->screenMaskingType_maskedShaderExternal_Combined;
                else
                    sp_for_mask = &pctx->screenMaskingType_maskedShaderRGB_Combined;

                if (texinfo)
                {

                    if (0 == texinfo->is360video)
                    {
                        LOGI("(normal video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                        if (bSTEN)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderStencil; //jeff log not support
                        else if (bYUV)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderYUV; //jeff log not support
                        else if (bNV12)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderNV12_Combined;
                        else if (bEGLI)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderExternal_Combined;
                        else
                            sp_for_tex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos : &pctx->screenMaskingType_texturedShaderRGB; //jeff log only support video
                    }
                    else if (texinfo->is360video)
                    {
                        LOGI("(360 video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                        if (bSTEN)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderStencil; //jeff log not support
                        else if (bYUV)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderYUV; //jeff log not support
                        else if (bNV12)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderNV12_Combined; //jeff log not support
                        else if (bEGLI)
                            sp_for_tex = &pctx->screenMaskingType_texturedShaderExternal360; //jeff log logically there's no such case which I should support.
                        else
                            sp_for_tex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_For_Videos360 : &pctx->screenMaskingType_texturedShaderRGB; //jeff log logically there's no such case which I should support.
                    }
                }

                sp_for_solid = &pctx->screenMaskingType_solidShaderRGB; //jeff log not support
            }
        }
        else if (homography || homography_mask)
        {

            sp_for_videotex = &pctx->screenMaskingType_texturedShaderRGB_vignette_Homo;
            sp_for_masktex = bSTEN ? &pctx->screenMaskingType_maskedShaderStencil_Homo : &pctx->screenMaskingType_maskedShaderRGB_Homo;
            sp_for_mask = bSTEN ? &pctx->screenMaskingType_maskedShaderStencil_Homo : &pctx->screenMaskingType_maskedShaderRGB;
            sp_for_tex = bSTEN ? &pctx->screenMaskingType_texturedShaderStencil_Homo : &pctx->screenMaskingType_texturedShaderRGB_Homo;
            sp_for_solid = &pctx->screenMaskingType_solidShaderRGB;
        }
        else
        {
            if (bYUV)
                sp_for_videotex = &pctx->screenMaskingType_texturedShaderYUV_vignette;
            else if (bNV12)
                sp_for_videotex = &pctx->screenMaskingType_texturedShaderNV12_vignette;
            else if (bEGLI)
                sp_for_videotex = &pctx->screenMaskingType_texturedShaderExternal_vignette;
            else
                sp_for_videotex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_For_Videos_vignette : &pctx->screenMaskingType_texturedShaderRGB_vignette;

            if (bSTEN)
                sp_for_masktex = &pctx->screenMaskingType_maskedShaderStencil;
            else if (bYUV)
                sp_for_masktex = &pctx->screenMaskingType_maskedShaderYUV;
            else if (bNV12)
                sp_for_masktex = &pctx->screenMaskingType_maskedShaderNV12;
            else if (bEGLI)
                sp_for_masktex = &pctx->screenMaskingType_maskedShaderExternal;
            else
                sp_for_masktex = &pctx->screenMaskingType_maskedShaderRGB;

            if (bSTEN)
                sp_for_mask = &pctx->screenMaskingType_maskedShaderStencil;
            else if (bYUV)
                sp_for_mask = &pctx->screenMaskingType_maskedShaderYUV;
            else if (bNV12)
                sp_for_mask = &pctx->screenMaskingType_maskedShaderNV12;
            else if (bEGLI)
                sp_for_mask = NULL;
            else
                sp_for_mask = &pctx->screenMaskingType_maskedShaderRGB;

            if (texinfo)
            {

                if (0 == texinfo->is360video)
                {
                    LOGI("(normal video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                    if (bSTEN)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderStencil;
                    else if (bYUV)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderYUV;
                    else if (bNV12)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderNV12;
                    else if (bEGLI)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderExternal;
                    else
                        sp_for_tex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_For_Videos : &pctx->screenMaskingType_texturedShaderRGB;
                }
                else if (texinfo->is360video)
                {
                    LOGI("(360 video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                    if (bSTEN)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderStencil;
                    else if (bYUV)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderYUV;
                    else if (bNV12)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderNV12;
                    else if (bEGLI)
                        sp_for_tex = &pctx->screenMaskingType_texturedShaderExternal360;
                    else
                        sp_for_tex = bVideo ? &pctx->screenMaskingType_texturedShaderRGB_For_Videos360 : &pctx->screenMaskingType_texturedShaderRGB;
                }
            }

            sp_for_solid = &pctx->screenMaskingType_solidShaderRGB;
        }
    }
    else{

        if(texName_for_back){

            if(homography || homography_mask){

                sp_for_videotex = &pctx->texturedShaderRGB_Combined_vignette_Homo;
                sp_for_masktex = bSTEN?&pctx->maskedShaderStencil_Homo:&pctx->maskedShaderRGB_Combined_Homo;
                sp_for_mask = bSTEN?&pctx->maskedShaderStencil_Homo:&pctx->maskedShaderRGB;
                sp_for_tex = bSTEN?&pctx->texturedShaderStencil_Homo:&pctx->texturedShaderRGB_Combined_Homo;
                sp_for_solid = &pctx->solidShaderRGB;
            }
            else{

                if(bYUV) sp_for_videotex = &pctx->texturedShaderYUV_vignette;//jeff log not support
                else if(bNV12) sp_for_videotex = &pctx->texturedShaderNV12_Combined_vignette;
                else if(bEGLI) sp_for_videotex = &pctx->texturedShaderExternal_Combined_vignette;
                else sp_for_videotex = bVideo?&pctx->texturedShaderRGB_Combined_For_Videos_vignette:&pctx->texturedShaderRGB_vignette;//jeff log when not video, do not support

                if(bSTEN) sp_for_masktex = &pctx->maskedShaderStencil;//jeff log not support
                else if(    bYUV  ) sp_for_masktex = &pctx->maskedShaderYUV;//jeff log not support
                else if(    bNV12 ) sp_for_masktex = &pctx->maskedShaderNV12_Combined;
                else if(    bEGLI ) sp_for_masktex = &pctx->maskedShaderExternal_Combined;
                else                sp_for_masktex = &pctx->maskedShaderRGB_Combined;

                if(         bSTEN ) sp_for_mask = &pctx->maskedShaderStencil;
                else if(    bYUV  ) sp_for_mask = &pctx->maskedShaderYUV;//jeff log not support
                else if(    bNV12 ) sp_for_mask = &pctx->maskedShaderNV12_Combined;
                else if(    bEGLI ) sp_for_mask = &pctx->maskedShaderExternal_Combined;
                else                sp_for_mask = &pctx->maskedShaderRGB_Combined;

                if(texinfo){

                    if(0 == texinfo->is360video){
                        LOGI("(normal video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                        if(         bSTEN ) sp_for_tex = &pctx->texturedShaderStencil;//jeff log not support
                        else if(    bYUV  ) sp_for_tex = &pctx->texturedShaderYUV;//jeff log not support
                        else if(    bNV12 ) sp_for_tex = &pctx->texturedShaderNV12_Combined;
                        else if(    bEGLI ) sp_for_tex = &pctx->texturedShaderExternal_Combined;
                        else                sp_for_tex = bVideo?&pctx->texturedShaderRGB_Combined_For_Videos:&pctx->texturedShaderRGB;//jeff log only support video
                    }
                    else if(texinfo->is360video){
                        LOGI("(360 video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                        if(         bSTEN ) sp_for_tex = &pctx->texturedShaderStencil;//jeff log not support
                        else if(    bYUV  ) sp_for_tex = &pctx->texturedShaderYUV;//jeff log not support
                        else if(    bNV12 ) sp_for_tex = &pctx->texturedShaderNV12_Combined;//jeff log not support
                        else if(    bEGLI ) sp_for_tex = &pctx->texturedShaderExternal360;//jeff log logically there's no such case which I should support.
                        else                sp_for_tex = bVideo?&pctx->texturedShaderRGB_For_Videos360:&pctx->texturedShaderRGB;//jeff log logically there's no such case which I should support.
                    }
                }

                sp_for_solid = &pctx->solidShaderRGB;//jeff log not support
            }   

        }
        else    
        if(homography || homography_mask){

            sp_for_videotex = &pctx->texturedShaderRGB_vignette_Homo;;
            sp_for_masktex = bSTEN?&pctx->maskedShaderStencil_Homo:&pctx->maskedShaderRGB_Homo;
            sp_for_mask = bSTEN?&pctx->maskedShaderStencil_Homo:&pctx->maskedShaderRGB;
            sp_for_tex = bSTEN?&pctx->texturedShaderStencil_Homo:&pctx->texturedShaderRGB_Homo;
            sp_for_solid = &pctx->solidShaderRGB;
        }
        else{

            if(bYUV) sp_for_videotex = &pctx->texturedShaderYUV_vignette;
            else if(bNV12) sp_for_videotex = &pctx->texturedShaderNV12_vignette;
            else if(bEGLI) sp_for_videotex = &pctx->texturedShaderExternal_vignette;
            else sp_for_videotex = bVideo?&pctx->texturedShaderRGB_For_Videos_vignette:&pctx->texturedShaderRGB_vignette;

            if(bSTEN) sp_for_masktex = &pctx->maskedShaderStencil;
            else if(    bYUV  ) sp_for_masktex = &pctx->maskedShaderYUV;
            else if(    bNV12 ) sp_for_masktex = &pctx->maskedShaderNV12;
            else if(    bEGLI ) sp_for_masktex = &pctx->maskedShaderExternal;
            else                sp_for_masktex = &pctx->maskedShaderRGB;

            if(         bSTEN ) sp_for_mask = &pctx->maskedShaderStencil;
            else if(    bYUV  ) sp_for_mask = &pctx->maskedShaderYUV;
            else if(    bNV12 ) sp_for_mask = &pctx->maskedShaderNV12;
            else if(    bEGLI ) sp_for_mask = NULL;
            else                sp_for_mask = &pctx->maskedShaderRGB;

            if(texinfo){

                if(0 == texinfo->is360video){
                    LOGI("(normal video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                    if(         bSTEN ) sp_for_tex = &pctx->texturedShaderStencil;
                    else if(    bYUV  ) sp_for_tex = &pctx->texturedShaderYUV;
                    else if(    bNV12 ) sp_for_tex = &pctx->texturedShaderNV12;
                    else if(    bEGLI ) sp_for_tex = &pctx->texturedShaderExternal;
                    else                sp_for_tex = bVideo?&pctx->texturedShaderRGB_For_Videos:&pctx->texturedShaderRGB;
                }
                else if(texinfo->is360video){
                    LOGI("(360 video)[NexThemeRenderer.cpp %d]RGBA_8888 Check value\n 1.bSTEN: %d\n 2.bYUV: %d\n 3.bNV12: %d\n 4.bEGLI: %d", __LINE__, bSTEN, bYUV, bNV12, bEGLI);
                    if(         bSTEN ) sp_for_tex = &pctx->texturedShaderStencil;
                    else if(    bYUV  ) sp_for_tex = &pctx->texturedShaderYUV;
                    else if(    bNV12 ) sp_for_tex = &pctx->texturedShaderNV12;
                    else if(    bEGLI ) sp_for_tex = &pctx->texturedShaderExternal360;
                    else                sp_for_tex = bVideo?&pctx->texturedShaderRGB_For_Videos360:&pctx->texturedShaderRGB;
                }
            }

            sp_for_solid = &pctx->solidShaderRGB;
        }
    }

    setShaderInternal(
        renderer,
        texture,
        mask,
        bYUV,
        bNV12,
        bEGLI,
        bSTEN,
        bJPEG_ColorConv,
        bVideo,
        texinfo,
        maskinfo,
        mvp,
        texmat,
        back_texmat,
        maskmat,
        texName,
        textureFormat,
        texName_for_rgb,
        texName_for_back,
        sp_for_videotex,
        sp_for_masktex,
        sp_for_mask,
        sp_for_tex,
        sp_for_solid,
        homography,
        homography_mask);

    if (texinfo){
		if (texName_for_rgb == 0)
			texName_for_rgb = texName[0];
	}	
    if( useTexture )
        *useTexture = texinfo;
    if( useMask )
        *useMask = maskinfo;

    return ret;
}

void NXT_Theme_SetRealXYForRenderItem(void* pmanager, int target, NXT_TextureInfo* texinfo){

    float real_x = 1.0f;
    float real_y = 1.0f;

    if(texinfo->srcHeight != texinfo->textureHeight){

        real_y = (float)(texinfo->srcHeight + TEX_BORDER_SIZE) / (float)texinfo->textureHeight;
    }
    if(texinfo->srcWidth != texinfo->textureWidth){

        real_x = (float)(texinfo->srcWidth + TEX_BORDER_SIZE) / (float)texinfo->textureWidth;
    }

    NXT_Theme_SetRealXY(pmanager, real_x, real_y, target);
}

static void setRealXY(int x_index, int y_index, NXT_TextureInfo* texinfo, int bEGLI){

	if( IS_VALID_UNIFORM(y_index)) 
    {
    	if(texinfo->srcHeight != texinfo->textureHeight){

    		glUniform1f(y_index, (float)(texinfo->srcHeight + TEX_BORDER_SIZE) / (float)texinfo->textureHeight);
    	}
    	else{
    		if(bEGLI){

     			float maxy = texinfo->surfaceMatrix.e[5];
      			glUniform1f(y_index, maxy);
    		}
    		else{

        		glUniform1f(y_index, 1.0f);
        	}
    	}
        
        CHECK_GL_ERROR();
    }
    //LOGI("[%s %d]TEST W: (%f, %f)(%f)", __func__, __LINE__, (float)texinfo->srcWidth, (float)texinfo->textureWidth, (float)texinfo->srcWidth / (float)texinfo->textureWidth);
	if( IS_VALID_UNIFORM(x_index))
	{
		if(texinfo->srcWidth != texinfo->textureWidth){

			glUniform1f(x_index, (float)(texinfo->srcWidth + TEX_BORDER_SIZE) / (float)texinfo->textureWidth);
		}
		else{
    		glUniform1f(x_index, 1.0f);
		}
	    
	    CHECK_GL_ERROR();
	}
}

static void configureShaderForLighting( NXT_HThemeRenderer renderer, NXT_Shader_LightingInfo *lightingInfo ) {
	if(!IS_VALID_UNIFORM(lightingInfo->u_diffuse_light))
		return;
    glUniform4fv(lightingInfo->u_diffuse_light, 1, renderer->diffuseColor.e);		CHECK_GL_ERROR();
    glUniform4fv(lightingInfo->u_ambient_light, 1, renderer->ambientColor.e);		CHECK_GL_ERROR();
    glUniform4fv(lightingInfo->u_specular_light, 1, renderer->specularColor.e);		CHECK_GL_ERROR();
    glUniform3fv(lightingInfo->u_lightpos, 1, renderer->lightDirection.e);			CHECK_GL_ERROR();
    glUniform1f(lightingInfo->u_specexp, renderer->specularExponent);				CHECK_GL_ERROR();
    switch( renderer->lightingNormals ) {
    case NXT_LightingNormals_Back:
        glUniform1f(lightingInfo->u_light_f0, -1);				CHECK_GL_ERROR();
        glUniform1f(lightingInfo->u_light_f0, 0);				CHECK_GL_ERROR();
        break;
    case NXT_LightingNormals_Both:
        glUniform1f(lightingInfo->u_light_f0, 1);				CHECK_GL_ERROR();
        glUniform1f(lightingInfo->u_light_f0, -1);				CHECK_GL_ERROR();
        break;
    case NXT_LightingNormals_Front:
    default:
        glUniform1f(lightingInfo->u_light_f0, 1);				CHECK_GL_ERROR();
        glUniform1f(lightingInfo->u_light_f0, 0);				CHECK_GL_ERROR();
        break;
    }

	if(IS_VALID_ATTRIBUTE(lightingInfo->a_normal))
	{
		glDisableVertexAttribArray(lightingInfo->a_normal);			// Caller can enable if they will provide normal data
		CHECK_GL_ERROR();
	}
}

#ifdef ANDROID
void NXT_ThemeRenderer_SurfaceChanged(NXT_HThemeRenderer renderer, int w, int h)
{
    LOGI("[%s %d] NXT_ThemeRenderer_SurfaceChanged (%d, %d)", __func__, __LINE__, w, h);
    glViewport(0, 0, w, h);
    
    if( !renderer ) {
        // ERROR: null pointer
        return;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return;
        }
    }    

    renderer->nativeWindowWidth = w;
    renderer->nativeWindowHeight = h;
    renderer->view_width = w;
    renderer->view_height = h;
    renderer->bDidRender = 0;
}
#endif
                                                      
#ifdef __APPLE__
void NXT_ThemeRenderer_SurfaceChanged(NXT_HThemeRenderer renderer, int x, int y, int w, int h, int scale)
{
	LOGI("[%s %d] NXT_ThemeRenderer_SurfaceChanged (%d, %d)", __func__, __LINE__, w, h);

    if( !renderer ) {
        // ERROR: null pointer
        return;
    }

	NXT_Platform_RenderTargetInfo info = { x, y, { (uint32_t)w, (uint32_t)h}, (float)scale };

    NXT_ThemeRenderer_AquireContext(renderer);

    NXT_Platform_SetRenderTargetInfo(renderer, info);

	GLsizei swidth = scale * w;
	GLsizei sheight = scale * h;
	glViewport(0, 0, swidth, sheight);

	bool abort = false;
	if( !renderer->bInitialized ) {
		NXT_Error result = initRenderer( renderer );
		if( result!=NXT_Error_None || !renderer->bInitialized ) {
			LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
			abort = true;
		}
	}

	if (!abort) {
		renderer->view_width = swidth;
		renderer->view_height = sheight;
		renderer->bDidRender = 0;
	}
	NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}
#endif

void NXT_ThemeRenderer_SetTextureFitInFrame(NXT_HThemeRenderer renderer,
                                            NXT_TextureID texture_id,
                                            unsigned int width,
                                            unsigned int height)
{
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }

    unsigned int defaultWidth = renderer->view_width;
    unsigned int defaultHeight = renderer->view_height;

#ifdef ANDROID
	defaultWidth = renderer->nativeExportSurface?renderer->exportSurfaceWidth:renderer->nativeWindowWidth;
	defaultHeight = renderer->nativeExportSurface?renderer->exportSurfaceHeight:renderer->nativeWindowHeight;
#endif
	
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->fitInWidth = width==0?defaultWidth:width;
    tex->fitInHeight = height==0?defaultHeight:height;
    
    LOGI("[%s %d] Set fit-in frame for id=%d (%d,%d)", __func__, __LINE__, (int)texture_id, tex->fitInWidth, tex->fitInHeight);
}

//void NXT_ThemeRenderer_SetTextureTransform(NXT_HThemeRenderer renderer,
//                                           NXT_TextureID texture_id,
//                                           float* startMatrix,
//                                           unsigned int startTime,
//                                           float* endMatrix,
//                                           unsigned int endTime)
//{
//    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
//        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
//        return;// NXT_Error_ParamRange;
//    }
//    
//    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
//    
//}

void NXT_ThemeRenderer_SetUserRotation(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, unsigned int rotation){

    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->user_rotation = rotation;
}

void NXT_ThemeRenderer_SetTranslation(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, int translate_x, int translate_y){

    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->translate_x = translate_x;
    tex->translate_y = translate_y;
}


void NXT_ThemeRenderer_SetTextureRotation(NXT_HThemeRenderer renderer,
                                           NXT_TextureID texture_id,
                                           unsigned int rotation)
{
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    if( (rotation & 0x00010000) == 0x00010000 ) {
        tex->mirror_h = 1;
        rotation &= ~0x00010000;
    } else {
        tex->mirror_h = 0;
    }
    if( (rotation & 0x00020000) == 0x00020000 ) {
        tex->mirror_v = 1;
        rotation &= ~0x00020000;
    } else {
        tex->mirror_v = 0;
    }
    
//    tex->rotation = ((rotation/90)%4)*90;
    tex->rotation = rotation;
    
    if( rotation != tex->rotation ) {
        LOGE("[%s %d] Unsupported angle.  Adjusted %d -> %d", __func__, __LINE__, rotation, tex->rotation);
    } 
    LOGI("[%s %d] Set rotation for id=%d (%d)", __func__, __LINE__, (int)texture_id, tex->rotation);
	
//	if(tex->rotation == 90 || tex->rotation == 270)
//	{
//		renderer->y_coord = (float)tex->srcHeight / (float)tex->textureHeight;
////		LOGI("Y_COORD : %f = %f / %f", renderer->y_coord, (float)tex->srcHeight, (float)tex->textureHeight);
//	}
//	else
//	{
//		renderer->y_coord = (float)tex->srcWidth / (float)tex->textureWidth;
////		LOGI("Y_COORD : %f = %f / %f", renderer->y_coord, (float)tex->srcWidth, (float)tex->textureWidth);
//	}
		
}

void NXT_ThemeRenderer_SetTextureColorAdjust(NXT_HThemeRenderer renderer,
                                             NXT_TextureID texture_id,
                                             int brightness,
                                             int contrast,
                                             int saturation,
                                             unsigned int tintcolor,
                                             int hue,
                                             int vignette)
{
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->brightness = (float)brightness/255.0f;;
    tex->contrast = (float)contrast/255.0f;
    tex->saturation = (float)saturation/255.0f;
    tex->hue = (float)hue/180.0f;
    tex->tintColor = tintcolor;
    tex->vignette = vignette;
    
    LOGI("[%s %d] Set color adjustment for id=%d (%f,%f,%f,0x%08X)", __func__, __LINE__, (int)texture_id, tex->brightness, tex->contrast, tex->saturation, tex->tintColor);
}

void NXT_ThemeRenderer_LinkToBaseTrack(NXT_HThemeRenderer renderer, unsigned int uid, unsigned int base_uid){

    LOGI("TM30 uid:%d linkto base uid:%d", uid, base_uid);
    renderer->linkTrack(uid, base_uid);
}

void NXT_ThemeRenderer_SetTextureInputRect(NXT_HThemeRenderer renderer,
                                           NXT_TextureID texture_id,
                                           float left, float bottom,
                                           float right, float top)
{
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
    
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    /*
    if( right>tex->srcWidth-1 ) {
        LOGW("[%s %d] rect out-of-range right (L:%f B:%f R:%f T:%f)", __func__, __LINE__, left, bottom, right, top);
        right = tex->srcWidth-1;
    } 
    if( left < 0 ) {
        LOGW("[%s %d] rect out-of-range left (L:%f B:%f R:%f T:%f)", __func__, __LINE__, left, bottom, right, top);
        left = 0;
    }
    if( top>tex->srcHeight-1 ) {
        LOGW("[%s %d] rect out-of-range top (L:%f B:%f R:%f T:%f)", __func__, __LINE__, left, bottom, right, top);
        top = tex->srcHeight-1;
    } 
    if( bottom < 0 ) {
        LOGW("[%s %d] rect out-of-range bottom (L:%f B:%f R:%f T:%f)", __func__, __LINE__, left, bottom, right, top);
        bottom = 0;
    }
    
    if( left >= right || bottom>=top || left < 0 || bottom < 0 || right>tex->srcWidth-1 || top > tex->srcHeight-1 ) {
        LOGE("[%s %d] rect error (L:%f B:%f R:%f T:%f)", __func__, __LINE__, left, bottom, right, top);
        return;// NXT_Error_ParamRange
    }
    */
    
    tex->left = left;
    tex->bottom = bottom;
    tex->right = right;
    tex->top = top;
    tex->bHasInputRect = 1;
    LOGI("[%s %d] Set input rect for id=%d (L:%f B:%f R:%f T:%f)", __func__, __LINE__, (int)texture_id, left, bottom, right, top);
}

NXT_Error NXT_ThemeRenderer_SwapTextures(NXT_HThemeRenderer renderer, 
                                         NXT_TextureID texture_id1, 
                                         NXT_TextureID texture_id2 )
{
    // Confirm renderer is valid and initialized
    LOGI("[%s %d] swap textures(%d,%d)", __func__, __LINE__, (int)texture_id1, (int)texture_id2);
    
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }

    // Validate parameters
    if( texture_id1 < 0 || texture_id1 >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id1=%d", __func__, __LINE__, (int)texture_id1);
        return NXT_Error_ParamRange;
    }
    if( texture_id2 < 0 || texture_id2 >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id2=%d", __func__, __LINE__, (int)texture_id2);
        return NXT_Error_ParamRange;
    }
    if( texture_id1 == texture_id2 ) {
        LOGD("[%s %d] Texture IDs are the same; just returning (%d,%d)", __func__, __LINE__, (int)texture_id1, (int)texture_id2);
        return NXT_Error_None;
    }
    
    NXT_TextureInfo ti = renderer->getRawVideoSrc(texture_id1);
    renderer->getRawVideoSrc(texture_id1) = renderer->getRawVideoSrc(texture_id2);
    renderer->getRawVideoSrc(texture_id2) = ti;
    
    if( renderer->last_upload_tex_id==NXT_TextureID_Video_1 ) {
        renderer->last_upload_tex_id=NXT_TextureID_Video_2;
    } else if(renderer->last_upload_tex_id==NXT_TextureID_Video_2) {
        renderer->last_upload_tex_id=NXT_TextureID_Video_1;
    }
    
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_ResetTexture(NXT_HThemeRenderer renderer,
                                         NXT_TextureID texture_id,
                                         int width,
                                         int height)
{
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif

#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO GL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);

    tex->track_update_id_ = renderer->getTrackIssueID();//always increase to make sure that this texture has been changed.    
    // Reset input rect
    tex->bHasInputRect = 0;
    tex->left = 0;
    tex->bottom = 0;
    tex->right = width-1;
    tex->top = height-1;
    tex->rotation = 0;
    tex->mirror_h = 0;
    tex->mirror_v = 0;
    tex->fitInWidth = 0;
    tex->fitInHeight = 0;
    
    // Reset color adjustment
    tex->brightness = 0;
    tex->contrast = 0;
    tex->saturation = 0;
    tex->tintColor = 0;
    tex->vignette = 0;

	tex->texName_for_rgb = 0;
    renderer->releaseRenderTarget(tex->prendertarget_);
    tex->prendertarget_ = NULL;
    renderer->releaseRenderTarget(tex->filtered_.prender_target_);
    tex->filtered_.prender_target_ = NULL;
#ifdef ANDROID
    // Clear any existing native buffer linked to this texture
    if( tex->bUseSurfaceTexture ) {
        NXT_ThemeRenderer_UnsetSurfaceTexture(renderer,texture_id);
    }
    if( tex->bUseNativeBuffer ) {
        NXT_ThemeRenderer_UnsetNativeTextureBuffer(renderer,texture_id);
    }
#endif // not __APPLE__
#ifdef USE_PLATFORM_SUPPORT
    NXT_Platform_UnloadTexture(renderer, texture_id);
#endif
    
    // Delete any texture names
    if( tex->texNameInitCount>0 ) {
        GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
        tex->texNameInitCount = 0;
    }

    tex->bValidTexture = 0;
    
    return NXT_Error_None;
}

#ifdef ANDROID
#ifdef NEX_ENABLE_GLES_IMAGE
NXT_Error NXT_ThemeRenderer_UnsetNativeTextureBuffer(NXT_HThemeRenderer renderer,
                                                     NXT_TextureID texture_id)
{
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGF("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    if( !tex->bUseNativeBuffer )
        return NXT_Error_None;
    
    // Delete any texture names
    if( tex->texNameInitCount>0 ) {
        GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
        CHECK_GL_ERROR();
        tex->texNameInitCount = 0;
        tex->textureWidth = 0;
        tex->textureHeight = 0;
        tex->textureUVWidth = 0;
        tex->textureUVHeight = 0;
    }    
    
    // Destroy the EGLImage
    if( tex->eglImageID != EGL_NO_IMAGE_KHR )
    {
        eglDestroyImageKHR( eglGetCurrentDisplay(), tex->eglImageID );
        CHECK_EGL_ERROR();
        CHECK_GL_ERROR();
        tex->eglImageID = EGL_NO_IMAGE_KHR;
#if 0 // delete not use graphic buffer
	 if( tex->pEGLImage )
	 {
        	nexGraphicBuffer_releaseNativeBuffer(tex->pEGLImage);
	       nexGraphicBuffer_ReleaseRef(tex->pEGLImage);
       	tex->pEGLImage = 0;
	 }
#endif		   
    }
    
    tex->bUseNativeBuffer = 0;
    tex->bValidTexture = 0;

    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_SetNativeTextureBuffer(NXT_HThemeRenderer renderer,
                                                   NXT_TextureID texture_id,
                                                   int width,
                                                   int height,
                                                   int width_pitch,
                                                   int height_pitch,
                                                   void* pNativeBuffer)
{
    LOGI("[%s %d] start", __func__, __LINE__);
    
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO EGL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }

    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    // Reset input rect
    /*tex->left = 0;
    tex->bottom = 0;
    tex->right = width-1;
    tex->top = height-1;
    tex->rotation = 0;
    tex->fitInWidth = 0;
    tex->fitInHeight = 0;
    
    // Reset color adjustment
    tex->brightness = 0;
    tex->contrast = 0;
    tex->saturation = 0;
    tex->tintColor = 0;

    // Clear any existing native buffer linked to this texture
    if( tex->bUseNativeBuffer ) {
        NXT_ThemeRenderer_UnsetNativeTextureBuffer(renderer,texture_id);
    }
    
    // Delete any texture names
    if( tex->texNameInitCount>0 ) {
        GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
        tex->texNameInitCount = 0;
    }*/
    
    NXT_ThemeRenderer_ResetTexture(renderer,texture_id,width,height);
    
    // Generate OpenGL texture name and set texture parameters
    glActiveTexture(GL_TEXTURE0);

    GL_GenTextures( 1, &tex->texName[0] );
    tex->texNameInitCount = 1;
    CHECK_GL_ERROR();
    
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex->texName[0]);
    CHECK_GL_ERROR();

    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
        
    // Begin profiling
    NXT_BEGIN_PROFILE("setNativeTextureBuffer");

    
    // Make the EGLImage
    const EGLint attrs[] = {
        EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
        EGL_NONE,                   EGL_NONE
    };
    
    tex->eglImageID = (EGLImageKHR)eglCreateImageKHR((EGLDisplay)eglGetCurrentDisplay(), 
                                                     (EGLContext)eglGetCurrentContext(), 
                                                     EGL_NATIVE_BUFFER_ANDROID, 
                                                     (EGLClientBuffer)pNativeBuffer, 
                                                     attrs);
    CHECK_EGL_ERROR();
    CHECK_GL_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, tex->eglImageID);
    CHECK_GL_ERROR();

    // Done - end profiling and return    
    NXT_END_PROFILE();
    
    tex->srcWidth = width;
    tex->srcHeight = height;
    tex->textureWidth = width_pitch;
    tex->textureHeight = height_pitch;
    tex->textureFormat = NXT_PixelFormat_EGLImage;
    tex->bValidTexture = 1;
    tex->bUseNativeBuffer = 1;
    
    renderer->last_upload_tex_id = texture_id;
    
    return NXT_Error_None;
    
}

#if 0 // delete not use graphic buffer
NXT_Error NXT_ThemeRenderer_SetNativeTextureBuffer2(NXT_HThemeRenderer renderer,
                                                   NXT_TextureID texture_id,
                                                   int width,
                                                   int height,
                                                   int width_pitch,
                                                   int height_pitch,
                                                   void* pEGLImage)
{
    LOGI("[%s %d] start", __func__, __LINE__);
    
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }

    if( !pEGLImage ) {
        LOGE("[%s %d] pEGLImage is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    // Check that we have an OpenGL context
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            LOGE("[%s %d] NO EGL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
                LOGE("[%s %d] NO EGL CONTEXT - Set native buffer to texture failed", __func__, __LINE__ );
                return NXT_Error_NoContext;
            }
#endif
    }

    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    // Reset input rect
    /*tex->left = 0;
    tex->bottom = 0;
    tex->right = width-1;
    tex->top = height-1;
    tex->rotation = 0;
    tex->fitInWidth = 0;
    tex->fitInHeight = 0;
    
    // Reset color adjustment
    tex->brightness = 0;
    tex->contrast = 0;
    tex->saturation = 0;
    tex->tintColor = 0;
    
    // Clear any existing native buffer linked to this texture
    if( tex->bUseNativeBuffer ) {
        NXT_ThemeRenderer_UnsetNativeTextureBuffer(renderer,texture_id);
    }
    
    // Delete any texture names
    if( tex->texNameInitCount>0 ) {
        GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
        tex->texNameInitCount = 0;
    }*/
    NXT_ThemeRenderer_ResetTexture(renderer,texture_id,width,height);

    // Generate OpenGL texture name and set texture parameters
    glActiveTexture(GL_TEXTURE0);

    GL_GenTextures( 1, &tex->texName[0], __FILE__, __LINE__ );
    tex->texNameInitCount = 1;
    CHECK_GL_ERROR();
    
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex->texName[0]);
    CHECK_GL_ERROR();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
        
    // Begin profiling
    NXT_BEGIN_PROFILE("setNativeTextureBuffer");

    
    // Make the EGLImage
    const EGLint attrs[] = {
        EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
        EGL_NONE,                   EGL_NONE
    };

    nexGraphicBuffer_AddRef(pEGLImage);
    tex->pEGLImage = pEGLImage;
    tex->eglImageID = (EGLImageKHR)eglCreateImageKHR((EGLDisplay)eglGetCurrentDisplay(), 
                                                     (EGLContext)eglGetCurrentContext(), 
                                                     EGL_NATIVE_BUFFER_ANDROID, 
                                                     (EGLClientBuffer)nexGraphicBuffer_getNativeBuffer(pEGLImage), 
                                                     attrs);
    CHECK_EGL_ERROR();
    CHECK_GL_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, tex->eglImageID);
    CHECK_GL_ERROR();

    // Done - end profiling and return    
    NXT_END_PROFILE();
    
    tex->srcWidth = width;
    tex->srcHeight = height;
    tex->textureWidth = width_pitch;
    tex->textureHeight = height_pitch;
    tex->textureFormat = NXT_PixelFormat_EGLImage;
    tex->bValidTexture = 1;
    tex->bUseNativeBuffer = 1;
    renderer->last_upload_tex_id = texture_id;
    return NXT_Error_None;
    
}
#endif

#endif //NEX_ENABLE_GLES_IMAGE
#endif // ANDROID

static void renderSimpleBoxForFaceDebug(NXT_HThemeRenderer renderer, float left, float top, float right, float bottom);

void NXT_ThemeRenderer_ConvertTextureToRGB(NXT_HThemeRenderer renderer, NXT_TextureInfo* ptex, int texid_for_customlut_a, int texid_for_customlut_b, int custom_lut_power, int texid_for_lut, float* prenderinfo, int use_HDR10, float maxCLL){
    
	unsigned int bYUV = 0;
	unsigned int bNV12 = 0;
	unsigned int bEGLI = 0;
	unsigned int bJPEG_ColorConv = 0;
	NXT_ShaderProgram_Textured *sp = NULL;
	
	if (!ptex->bValidTexture)
		return;

	//LOGI("%s %d input format:%d", __func__, __LINE__, ptex->textureFormat);
	switch (ptex->textureFormat){
	case NXT_PixelFormat_YUV:
		bYUV = 1;
		break;
	case NXT_PixelFormat_NV12:
		bNV12 = 1;
		break;
	case NXT_PixelFormat_NV12_JPEG:
		bNV12 = 1;
		bJPEG_ColorConv = 1;
		break;
	case NXT_PixelFormat_EGLImage:
		bEGLI = 1;
		break;
	default:
		if(texid_for_lut == 0)
			ptex->texName_for_rgb = ptex->texName[0];
		else
			break;
		return;
	};

	NXT_ThemeRenderer_ReleaseRenderTarget(renderer, ptex->prendertarget_);
	ptex->prendertarget_ = NULL;	
	NXT_RenderTarget* prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, ptex->textureWidth, ptex->textureHeight, 0, 0, 0);

	NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
	
	glDisable(GL_DEPTH_TEST);
	CHECK_GL_ERROR();
	glDepthMask(GL_FALSE);
	CHECK_GL_ERROR();
	glDisable(GL_CULL_FACE);
	CHECK_GL_ERROR();

	// glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	CHECK_GL_ERROR();
	// glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR();	
	
	
	if (bYUV)		sp = &renderer->getContextManager()->texturedShaderPassthroughYUV;
	else if (bNV12) sp = &renderer->getContextManager()->texturedShaderPassthroughNV12;
	else if (bEGLI){
        if(use_HDR10){
            sp = &renderer->getContextManager()->texturedShaderPassthroughExternalHDR10;
        }
        else{
            sp = &renderer->getContextManager()->texturedShaderPassthroughExternal;
        }
    }     
	else            sp = &renderer->getContextManager()->texturedShaderPassthroughRGB;

#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
    glUseProgram(sp->shaderProgram);									CHECK_GL_ERROR();
#endif

	renderer->pTexturedShaderActive = sp;

	if (bYUV) {
		glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[0]);              CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSamplerY, 0);                          CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[1]);              CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSamplerU, 1);                          CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE2);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[2]);              CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSamplerV, 2);                          CHECK_GL_ERROR();
	}
	else if (bNV12) {
		glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[0]);              CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSamplerY, 0);                          CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE1);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[1]);              CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSamplerUV, 1);                         CHECK_GL_ERROR();

		float tintRed = ((float)((ptex->tintColor >> 16) & 0xFF)) / 255.0;
		float tintGreen = ((float)((ptex->tintColor >> 8) & 0xFF)) / 255.0;
		float tintBlue = ((float)((ptex->tintColor) & 0xFF)) / 255.0;
		NXT_Matrix4f colorconv;
		colorconv = NXT_YUV2RGBWithColorAdjust(0.0f, 0.0f, 0.0f, NXT_Vector4f(tintRed + renderer->adj_tintColor.e[0], tintGreen + renderer->adj_tintColor.e[1], tintBlue + renderer->adj_tintColor.e[2], 1), bJPEG_ColorConv);
		glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, colorconv.e);  CHECK_GL_ERROR();
	}
	else if (bEGLI) {
#ifdef ANDROID
		glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, ptex->texName[0]);    CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
		glUniformMatrix4fv(sp->u_tex_matrix, 1, GL_FALSE, ptex->surfaceMatrix.e); 	CHECK_GL_ERROR();
        if(use_HDR10){
            glUniform1f(sp->u_MaximumContentLightLevel, maxCLL);            CHECK_GL_ERROR();
            glUniform1f(sp->u_DeviceLightLevel, renderer->getDeviceLightLevel());  CHECK_GL_ERROR();
            glUniform1f(sp->u_Gamma, renderer->getDeviceGamma());                  CHECK_GL_ERROR();
        }
#endif // ANDROID
	}
	else {
		glActiveTexture(GL_TEXTURE0);                                   CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex->texName[0]);
		if (glGetError()){
			LOGE("[%s %i] glError texid:%d\n", __func__, __LINE__, ptex->texName[0]);
		}
		CHECK_GL_ERROR();
		glUniform1i(sp->u_textureSampler, 0);                           CHECK_GL_ERROR();
	}
	float real_x = 1.0f;
	float real_y = 1.0f;

	if(bEGLI){

		real_x = (float)ptex->srcWidth / (float)ptex->textureWidth;
		real_y = (float)ptex->srcHeight / (float)ptex->textureHeight;
	}
	else{

		real_x = (float)(ptex->srcWidth + TEX_BORDER_SIZE) / (float)ptex->textureWidth;
		real_y = (float)(ptex->srcHeight + TEX_BORDER_SIZE) / (float)ptex->textureHeight;
	}
	if (IS_VALID_UNIFORM(sp->u_realY))
	{
		glUniform1f(sp->u_realY, real_y);
		CHECK_GL_ERROR();
	}
	if (IS_VALID_UNIFORM(sp->u_realX))
	{
		glUniform1f(sp->u_realX, real_x);
		CHECK_GL_ERROR();
	}
    if (IS_VALID_UNIFORM(sp->u_hue))
    {
        glUniform1f(sp->u_hue, 0.0f);
        CHECK_GL_ERROR();
    }

	glEnableVertexAttribArray(sp->a_position);							CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_texCoord);							CHECK_GL_ERROR();

	GLfloat vertexCoord[] = {
		-1.0, 1.0,
		1.0, 1.0,
		-1.0, -1.0,
		1.0, -1.0
	};
	float lowx = 0.0f;
	float lowy = 1.0f;
	float highx = 1.0f;
	float highy = 0.0f;
		
	GLfloat texCoords[] = {
		lowx, lowy,
		highx, lowy,
		lowx, highy,
		highx, highy
	};


	glVertexAttribPointer(renderer->pTexturedShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();
	glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord, 2, GL_FLOAT, 0, 0, texCoords);
	CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();

#if 0//def FOR_PROJECT_Default

	if(prenderinfo)
		renderSimpleBoxForFaceDebug(renderer, prenderinfo[0] * real_x, prenderinfo[1] * real_y, prenderinfo[2] * real_x, prenderinfo[3] * real_y);

#endif//FOR_PROJECT_Default
	//For test
    if(texid_for_customlut_a > 0 || texid_for_customlut_b > 0){

        glEnable(GL_BLEND);
        CHECK_GL_ERROR();
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL_ERROR();   

        float boarder_size = 0.0f;

        if(custom_lut_power >= 100000){

            //draw by lutA only
            if(texid_for_customlut_a > 0){

                NXT_RenderTarget* psrc = prender_target;
                prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
                NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_a, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
            }
        }
        else if(custom_lut_power <= 0){

            //draw by lutB only
            if(texid_for_customlut_b > 0){
                
                NXT_RenderTarget* psrc = prender_target;
                prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
                NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_b, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
            }
        }
        else{

            if(texid_for_customlut_a > 0 && texid_for_customlut_b > 0){

                NXT_RenderTarget* psrc = prender_target;
                prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
                NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_a, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_b, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_, 1.0f - (float)custom_lut_power / 100000);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
            }
            else if(texid_for_customlut_a > 0){

                NXT_RenderTarget* psrc = prender_target;
                prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
                NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
                renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassthroughRGB, psrc->target_texture_, 1);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_a, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_, (float)custom_lut_power / 100000);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
            }
            else if(texid_for_customlut_b > 0){

                NXT_RenderTarget* psrc = prender_target;
                prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
                NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
                renderSimplePlane(renderer, &renderer->getContextManager()->texturedShaderPassthroughRGB, psrc->target_texture_, 1);
                renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_customlut_b, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_, 1.0f - (float)custom_lut_power / 100000);
                NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
            }
        }
    }

	if (texid_for_lut > 0)
	{
		NXT_RenderTarget* psrc = prender_target;
		prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, psrc->width_, psrc->height_, 0, 0, 0);
		NXT_ThemeRenderer_SetRenderTarget(renderer,prender_target);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float boarder_size = 0.0f;
		// if(bYUV || bNV12)
		// 	boarder_size = TEX_BORDER_SIZE;		
		renderSimplePlaneForLUT(renderer, &renderer->getContextManager()->texturedShaderLUT, psrc->target_texture_, texid_for_lut, 1, real_x, real_y, boarder_size / (float)psrc->width_, boarder_size / (float)psrc->height_);
		NXT_ThemeRenderer_ReleaseRenderTarget(renderer, psrc);
	}

	ptex->textureFormat = NXT_PixelFormat_RGBA8888;
	ptex->texName_for_rgb = prender_target->target_texture_;
    ptex->prendertarget_ = prender_target;
    NXT_ThemeRenderer_SetRenderToDefault(renderer);
}

static void renderSimpleBoxForFaceDebug(NXT_HThemeRenderer renderer, float left, float top, float right, float bottom){

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();   

	NXT_ShaderProgram_Solid* sp = &renderer->getContextManager()->solidShaderRGB;

#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UseShaderProgram(sp,sizeof(*sp));
#else
	glUseProgram(sp->shaderProgram);                                  CHECK_GL_ERROR();
#endif
	renderer->pSolidShaderActive = sp;

	glEnableVertexAttribArray(sp->a_position);                        CHECK_GL_ERROR();
	glEnableVertexAttribArray(sp->a_color);                        CHECK_GL_ERROR();

	float vtx_left = (left/100000.0f - 0.5f) * 2.0f;
	float vtx_top = (bottom/100000.0f - 0.5f) * 2.0f;
	float vtx_right = (right/100000.0f - 0.5f) * 2.0f;
	float vtx_bottom = (top/100000.0f - 0.5f) * 2.0f;

	LOGI("[%s %d] left:%f top:%f right:%f bottom:%f", __func__, __LINE__, left, top, right, bottom);

	GLfloat vertexCoord[8] = {
		vtx_left, vtx_top,
		vtx_right, vtx_top,
		vtx_left, vtx_bottom,
		vtx_right, vtx_bottom,
	};

	glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord);
	CHECK_GL_ERROR();

	NXT_Vector4f color;

	color.e[0] = 0.0f;
	color.e[1] = 0.0f;
	color.e[2] = 0.5f;
	color.e[3] = 0.5f;
        
    GLfloat vertexColor[16];
    memcpy(vertexColor+0, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+4, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+8, color.e, sizeof(GLfloat)*4);
    memcpy(vertexColor+12, color.e, sizeof(GLfloat)*4);

    NXT_Matrix4f mvp = NXT_Matrix4f_Identity();
    glUniformMatrix4fv(sp->u_mvp_matrix, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
    glUniformMatrix4fv(sp->u_colorconv, 1, GL_FALSE, mvp.e);           CHECK_GL_ERROR();
    glUniform1f(sp->u_alpha, 1.0f);                          			CHECK_GL_ERROR();
    configureShaderForLighting(renderer,&sp->lightingInfo);
    
    glVertexAttribPointer(sp->a_color,4,GL_FLOAT,0,0,vertexColor );
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERROR();
}

NXT_Error NXT_ThemeRenderer_UploadTexture(NXT_HThemeRenderer renderer,
                                     unsigned int track_id,
                                     NXT_TextureID texture_id,
                                     int width,
                                     int height,
                                     int pitch,
                                     NXT_PixelFormat textureFormat,
                                     unsigned char* data0,
                                     unsigned char* data1,
                                     unsigned char* data2,
                                     unsigned int lut_id,
                                     unsigned int customlut_a_id,
                                     unsigned int customlut_b_id,
                                     unsigned int customlut_power,
                                     float* prenderinfo )
{
    LOGI("[%s %d] start", __func__, __LINE__);

    // Confirm renderer is valid and initialized
#ifdef NEX_THEME_RENDERER_DEBUG_LOGGING
    unsigned int d0int0 = data0?((unsigned int*)data0)[0]:0;
    unsigned int d1int0 = data1?((unsigned int*)data1)[0]:0;
    unsigned int d2int0 = data2?((unsigned int*)data2)[0]:0;
    unsigned int d0int1 = data0?((unsigned int*)data0)[1]:0;
    unsigned int d1int1 = data1?((unsigned int*)data1)[1]:0;
    unsigned int d2int1 = data2?((unsigned int*)data2)[1]:0;
    LOGD("[%s %d] upload id=%d w/h/p=%d/%d/%d fmt=%d  data(0x%08x%08x, 0x%08x%08x, 0x%08x%08x)", __func__, __LINE__, (int)texture_id, width, height, pitch, (int)textureFormat, d0int0, d0int1, d1int0, d1int1, d2int0, d2int1);
#endif //NEX_THEME_RENDERER_DEBUG_LOGGING
    
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        NXT_Error result = initRenderer( renderer );
        if( result!=NXT_Error_None || !renderer->bInitialized ) {
            LOGE("[%s %d] ThemeRenderer failed to initialize (%d/%d)", __func__, __LINE__, result, renderer->bInitialized);
            return result;
        }
    }

    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    if( !data0 || (textureFormat==NXT_PixelFormat_YUV && (!data1 || !data2)) ) {
        LOGE("[%s %d] NXT_Error_MissingParam data0/1/2=0x%08x,0x%08x,0x%08x", __func__, __LINE__, (unsigned int)(size_t)data0, (unsigned int)(size_t)data1, (unsigned int)(size_t)data2);
        return NXT_Error_MissingParam;
    }
    
    // Check that we have an OpenGL context
    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT - Upload texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO GL CONTEXT - Upload texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }
    
    // Choose texture format
    GLint texInternalFormat;
    GLenum texDataFormat;
    GLenum texDataType;
    switch( textureFormat ) {
        case NXT_PixelFormat_RGB565:
            texInternalFormat=GL_RGB;
            texDataFormat=GL_RGB;
            texDataType=GL_UNSIGNED_SHORT_5_6_5;
            break;
        case NXT_PixelFormat_RGBA8888:
            texInternalFormat=GL_RGBA;
            texDataFormat=GL_RGBA;
            texDataType=GL_UNSIGNED_BYTE;
            break;
        case NXT_PixelFormat_Luminance:
            texInternalFormat=GL_LUMINANCE;
            texDataFormat=GL_LUMINANCE;
            texDataType=GL_UNSIGNED_BYTE;
            break;
        case NXT_PixelFormat_NV12:
        case NXT_PixelFormat_NV12_JPEG:
        case NXT_PixelFormat_YUV:
            texInternalFormat=GL_LUMINANCE;
            texDataFormat=GL_LUMINANCE;
            texDataType=GL_UNSIGNED_BYTE;
            break;
        /*case NXT_PixelFormat_NV12:
            texInternalFormat=GL_LUMINANCE_ALPHA;
            texDataFormat=GL_LUMINANCE_ALPHA;
            texDataType=GL_UNSIGNED_BYTE;
            break;*/
        default:
            LOGE("[%s %d] NXT_Error_BadParam (textureFormat=%d)", __func__, __LINE__, (int)textureFormat);
            return NXT_Error_BadParam;
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->track_id_ = track_id;
    tex->lut = lut_id;
    tex->custom_lut_a = customlut_a_id;
    tex->custom_lut_b = customlut_b_id;
    tex->custom_lut_power = customlut_power;
    tex->serial_++;
    NXT_ThemeRenderer_ResetTexture(renderer, texture_id, width, height);
    // Begin profiling
    NXT_BEGIN_PROFILE("uploadTexture");
    
    // Use texture unit 0 for everything (we only work with one texture at a time here)
    glActiveTexture(GL_TEXTURE0);
    
    // Generate OpenGL texture names, if necessary
    int reqTexName;
    switch( textureFormat ) {
        case NXT_PixelFormat_YUV: 
            reqTexName=3; 
            break;
        case NXT_PixelFormat_NV12:
        case NXT_PixelFormat_NV12_JPEG:
            reqTexName=2;
            break;
        default:
            reqTexName=1; 
            break;
    }
	
    if( tex->texNameInitCount>0 && (tex->textureFormat!=textureFormat||tex->texNameInitCount!=reqTexName)) {
        LOGI("[%s %d] FORMAT CHANGE; TEXTURE NAME RESET NEEDED (have %d; reset to 0)", __func__, __LINE__, tex->texNameInitCount);
        GL_DeleteTextures( tex->texNameInitCount, &tex->texName[0] );
        CHECK_GL_ERROR();
        tex->texNameInitCount = 0;
    }
	
    if( tex->texNameInitCount<reqTexName ) {
        LOGI("[%s %d] Making texture names (have %d; need %d)", __func__, __LINE__, tex->texNameInitCount, reqTexName);
        int i;
        for( i = tex->texNameInitCount; i<reqTexName; i++ ) {
            GL_GenTextures( 1, &tex->texName[i] );
            CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, tex->texName[i]);
            CHECK_GL_ERROR();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/ );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            CHECK_GL_ERROR();
            if( i==0 ) {
                tex->textureWidth = 0;
                tex->textureHeight = 0;
            } else {
                tex->textureUVWidth = 0;
                tex->textureUVHeight = 0;
            }
        }
        tex->texNameInitCount = reqTexName;
    }
    
    // Calculate necessary texture sizes
    unsigned int textureWidth = NXT_NextHighestPowerOfTwo(width+TWICE_TEX_BORDER_SIZE>pitch?
                                                          width+TWICE_TEX_BORDER_SIZE:pitch);
    unsigned int textureHeight = NXT_NextHighestPowerOfTwo(height+TWICE_TEX_BORDER_SIZE);
    
    NXT_PROFILE_CHECKPOINT("A");
    
    // Re-create textures if the size doesn't match (this also handles
    // the initial case becasue the sizes will have been initialized
    // to zero in the beginning)
    if( tex->textureWidth != textureWidth || tex->textureHeight != textureHeight 
        || tex->textureFormat!=textureFormat || tex->srcWidth!=width || tex->srcHeight!=height ) 
    {
        tex->srcWidth = width;
        tex->srcHeight = height;
        unsigned int texDataSize = textureWidth * textureHeight * 2;
        unsigned char *data = (unsigned char*)malloc(texDataSize);
        if( data ) {
            memset(data, 0x00, texDataSize);
        }
        
        LOGI("[%s %d] RE-INITIALIZE MAIN TEXTURE %d", __func__, __LINE__, texture_id);
        glBindTexture(GL_TEXTURE_2D, tex->texName[0]);
        CHECK_GL_ERROR();
        
        glTexImage2D( GL_TEXTURE_2D, 0, texInternalFormat, textureWidth, textureHeight, 0, texDataFormat, texDataType, data );
        CHECK_GL_ERROR();
        tex->textureWidth = textureWidth;
        tex->textureHeight = textureHeight;
        
        if( data ) {
            free(data);
            data = NULL;
        }

    }
    
    NXT_PROFILE_CHECKPOINT("B");
    
    if( textureFormat==NXT_PixelFormat_YUV ) {
        tex->srcUVWidth = (width+TWICE_TEX_BORDER_SIZE)/2;
        tex->srcUVHeight = (height+TWICE_TEX_BORDER_SIZE)/2;
        unsigned int textureUVWidth = textureWidth/2;//NXT_NextHighestPowerOfTwo(tex->srcUVWidth);
        unsigned int textureUVHeight = textureHeight/2;//NXT_NextHighestPowerOfTwo(tex->srcUVHeight);
        if( tex->textureUVWidth != textureUVWidth || tex->textureUVHeight != textureUVHeight
            || tex->textureFormat!=textureFormat ) 
        {
            LOGI("[%s %d] RE-INITIALIZE UV TEXTURE (YUV) %d", __func__, __LINE__, texture_id);
            glBindTexture(GL_TEXTURE_2D, tex->texName[1]);
            CHECK_GL_ERROR();
            glTexImage2D( GL_TEXTURE_2D, 0, texInternalFormat, textureUVWidth, textureUVHeight, 0, texDataFormat, texDataType, NULL );
            CHECK_GL_ERROR();
            glBindTexture(GL_TEXTURE_2D, tex->texName[2]);
            CHECK_GL_ERROR();
            glTexImage2D( GL_TEXTURE_2D, 0, texInternalFormat, textureUVWidth, textureUVHeight, 0, texDataFormat, texDataType, NULL );
            CHECK_GL_ERROR();
            tex->textureUVWidth = textureUVWidth;
            tex->textureUVHeight = textureUVHeight;
        }
        else
        {
            LOGI("[%s %d] KEEP USING CURRENT TEXTURE (YUV) %d", __func__, __LINE__, texture_id);
        }
    } else if( textureFormat==NXT_PixelFormat_NV12 || textureFormat==NXT_PixelFormat_NV12_JPEG ) {
        tex->srcUVWidth = (width+TWICE_TEX_BORDER_SIZE)/2;
        tex->srcUVHeight = (height+TWICE_TEX_BORDER_SIZE)/2;
        unsigned int textureUVWidth = textureWidth/2;// NXT_NextHighestPowerOfTwo(tex->srcUVWidth);
        unsigned int textureUVHeight = textureHeight/2;// NXT_NextHighestPowerOfTwo(tex->srcUVHeight);
        if( tex->textureUVWidth != textureUVWidth || tex->textureUVHeight != textureUVHeight
           || tex->textureFormat!=textureFormat ) 
        {
            unsigned int texUVDataSize = textureUVWidth * textureUVHeight * 2;
            unsigned char *data = (unsigned char*)malloc(texUVDataSize);
            if( data ) {
                memset(data, 0x80, texUVDataSize);
            }
            LOGI("[%s %d] RE-INITIALIZE UV TEXTURE (NV12) %d", __func__, __LINE__, texture_id);
            glBindTexture(GL_TEXTURE_2D, tex->texName[1]);
            CHECK_GL_ERROR();
#if defined(__APPLE__)
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RG_EXT, textureUVWidth, textureUVHeight, 0, GL_RG_EXT, GL_UNSIGNED_BYTE, data );
#else
            glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, textureUVWidth, textureUVHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
#endif
            CHECK_GL_ERROR();
            tex->textureUVWidth = textureUVWidth;
            tex->textureUVHeight = textureUVHeight;
            if( data ) {
                free(data);
                data = NULL;
            }
        }
        else
        {
            LOGI("[%s %d] KEEP USING CURRENT TEXTURE (NV12) %d", __func__, __LINE__, texture_id);
        }
    }
    
    NXT_PROFILE_CHECKPOINT("C");
 
    LOGI("[%s %d] texture_id=%i textureFormat=%i width=%i height=%i pitch=%i src(%i,%i) tex(%i,%i) srcUV(%i,%i) texUV(%i,%i) ", __func__, __LINE__, texture_id, textureFormat, width, height, pitch, tex->srcWidth, tex->srcHeight, tex->textureWidth, tex->textureHeight, tex->srcUVWidth, tex->srcUVHeight, tex->textureUVWidth, tex->textureUVHeight );
    
    // Upload the pixel data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glBindTexture(GL_TEXTURE_2D, tex->texName[0]);
    CHECK_GL_ERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, TEX_BORDER_SIZE, TEX_BORDER_SIZE, pitch, height, texDataFormat, texDataType, data0);
    CHECK_GL_ERROR();
    LOGD("[%s %d]Uploaded channel 1", __func__, __LINE__ );
    
    NXT_PROFILE_CHECKPOINT("D");
    
    if( textureFormat==NXT_PixelFormat_YUV ) {
        glBindTexture(GL_TEXTURE_2D, tex->texName[1]);
        CHECK_GL_ERROR();
        glTexSubImage2D(GL_TEXTURE_2D, 0, HALF_TEX_BORDER_SIZE, HALF_TEX_BORDER_SIZE, pitch/2, height/2, texDataFormat, texDataType, data1);
        CHECK_GL_ERROR();
        NXT_PROFILE_CHECKPOINT("D2");
        glBindTexture(GL_TEXTURE_2D, tex->texName[2]);
        CHECK_GL_ERROR();
        glTexSubImage2D(GL_TEXTURE_2D, 0, HALF_TEX_BORDER_SIZE, HALF_TEX_BORDER_SIZE, pitch/2, height/2, texDataFormat, texDataType, data2);
        CHECK_GL_ERROR();
        LOGD("[%s %d]Uploaded channels 2 and 3 (YUV)", __func__, __LINE__ );
    } else if( textureFormat==NXT_PixelFormat_NV12 || textureFormat==NXT_PixelFormat_NV12_JPEG ) {
        glBindTexture(GL_TEXTURE_2D, tex->texName[1]);
        CHECK_GL_ERROR();
#if defined(__APPLE__)
        glTexSubImage2D(GL_TEXTURE_2D, 0, HALF_TEX_BORDER_SIZE, HALF_TEX_BORDER_SIZE, pitch/2, height/2, GL_RG_EXT, GL_UNSIGNED_BYTE, data1);
#else
        glTexSubImage2D(GL_TEXTURE_2D, 0, HALF_TEX_BORDER_SIZE, HALF_TEX_BORDER_SIZE, pitch/2, height/2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data1);
#endif
        CHECK_GL_ERROR();
        LOGD("[%s %d]Uploaded channel 2 (UV Interleave)", __func__, __LINE__ );
    }
    NXT_PROFILE_CHECKPOINT("E");

    tex->bValidTexture = 1;
    tex->origin_textureformat_ = textureFormat;
    tex->textureFormat = textureFormat;
    renderer->last_upload_tex_id = texture_id;
//	if (texture_id >= NXT_TextureID_Video_Layer_START)
#ifdef FORCE_RGB_CONVERTER
#else
    if(lut_id > 0 || customlut_a_id > 0 || customlut_b_id > 0)
#endif
    {

        int custom_lut_a_texture = 0;
        int custom_lut_b_texture = 0;

        if(customlut_a_id > 0){

            custom_lut_a_texture = NXT_ThemeRenderer_GetLUTTexture(renderer, customlut_a_id);
            if(custom_lut_a_texture == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, customlut_a_id);
                if(tex_id != 0)
                    custom_lut_a_texture = NXT_ThemeRenderer_SetLUTTexture(renderer, customlut_a_id, tex_id);
            }
        }

        if(customlut_b_id > 0){

            custom_lut_b_texture = NXT_ThemeRenderer_GetLUTTexture(renderer, customlut_b_id);
            if(custom_lut_b_texture == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, customlut_b_id);
                if(tex_id != 0)
                    custom_lut_b_texture = NXT_ThemeRenderer_SetLUTTexture(renderer, customlut_b_id, tex_id);
            }
        }

        int tex_id_for_lut = 0;
        if(lut_id != 0){
            LOGI("[%s %d]", __func__, __LINE__);
           tex_id_for_lut = NXT_ThemeRenderer_GetLUTTexture(renderer, lut_id);
           LOGI("[%s %d]", __func__, __LINE__);
           if(tex_id_for_lut == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, lut_id);
                if(tex_id != 0)
                    tex_id_for_lut = NXT_ThemeRenderer_SetLUTTexture(renderer, lut_id, tex_id);
           }
        }
		NXT_ThemeRenderer_ConvertTextureToRGB(renderer, &renderer->getVideoSrc(texture_id), custom_lut_a_texture, custom_lut_b_texture, customlut_power, tex_id_for_lut, prenderinfo, 0, 10000.0);
    }
    NXT_END_PROFILE();
    
    return NXT_Error_None;
}

#ifdef NEX_THEME_RENDERER_DEBUG_LOGGING
//#ifdef NEX_THEME_RENDERER_PROFILE_READPIXELS
unsigned int NXT_Profile_TickCount( void )
{
	static unsigned int uPreTick = 0;
	unsigned int uRet;
	struct timeval tm;
    
	gettimeofday( &tm, NULL );
	uRet = ( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
	if ( uPreTick > uRet && (uPreTick-uRet) < 100000 )
		return uPreTick;
    
	uPreTick = uRet;
	return uRet;
}
//#endif
#else
unsigned int NXT_Profile_TickCount( void ) { return 0; }

#endif

NXT_RenderTarget* NXT_ThemeRenderer_::getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request){

    RenderTargetManager* pmanager = (RenderTargetManager*)getContextManager()->getRendertargetManager();
    return pmanager->getRenderTarget(width, height, alpha_request, depth_request, stencil_request);
}

NXT_RenderTarget* NXT_ThemeRenderer_::getDepthonly(int width, int height, int stencil_request){

    RenderTargetManager* pmanager = (RenderTargetManager*)getContextManager()->getRendertargetManager();
    return pmanager->getDepthonly(width, height, stencil_request);
}

NXT_RenderTarget* NXT_ThemeRenderer_GetRenderTarget(NXT_HThemeRenderer renderer, int width, int height, int alpha_request, int depth_request, int stencil_request){

    return renderer->getRenderTarget(width, height, alpha_request, depth_request, stencil_request);
}

NXT_RenderTarget* NXT_ThemeRenderer_GetDepthonly(NXT_HThemeRenderer renderer, int width, int height, int stencil_request){

    return renderer->getDepthonly(width, height, stencil_request);
}

void NXT_ThemeRenderer_::releaseRenderTarget(NXT_RenderTarget* prendertarget){

    RenderTargetManager* pmanager = (RenderTargetManager*)getContextManager()->getRendertargetManager();
    pmanager->releaseRenderTarget(prendertarget);
}

void NXT_ThemeRenderer_ReleaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prendertarget){

	renderer->releaseRenderTarget(prendertarget);
}

static void NXT_ThemeRenderer_UpdateRenderTargetManager(NXT_HThemeRenderer renderer){

	RenderTargetManager* pmanager = (RenderTargetManager*)renderer->getContextManager()->getRendertargetManager();
	pmanager->update();
}

int NXT_ThemeRenderer_::getLut(int id){

    LutResourceMapper_t::iterator itor = lut_resource_.find(id);
    if(itor != lut_resource_.end())
        return itor->second;
    return 0;
}

int NXT_ThemeRenderer_::releaseLut(int id){

    LutResourceMapper_t::iterator itor = lut_resource_.find(id);
    if(itor != lut_resource_.end())
        lut_resource_.erase(itor);
    return 0;
}
int NXT_ThemeRenderer_::setLutTexture(int id, int tex_id){

    LutResourceMapper_t::iterator itor = lut_resource_.find(id);
    if(itor != lut_resource_.end()){

        itor->second = tex_id;
    }
    else{

        lut_resource_.insert(std::make_pair(id, tex_id));
    }
    return tex_id;
}

int NXT_ThemeRenderer_GetLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id){

	return renderer->getLut(lut_resource_id);
}
//yoon
int NXT_ThemeRenderer_ReleaseLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id) {

    return renderer->releaseLut(lut_resource_id);
}


int NXT_ThemeRenderer_SetLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id, int tex_id){

	return renderer->setLutTexture(lut_resource_id, tex_id);
}

void NXT_ThemeRenderer_RegisterGetLutTexWithHash(NXT_HThemeRenderer renderer, int (*pfunc)(int, int)){

	renderer->getLutTextureWithID = pfunc;
}

void NXT_ThemeRenderer_RegisterGetEffectImagePath(NXT_HThemeRenderer renderer, int (*pfunc)(const char*, char*)){

	renderer->getEffectImagePath = pfunc;
}

void NXT_ThemeRenderer_RegisterGetVignetteTexID(NXT_HThemeRenderer renderer, int (*pfunc)(int)){

	renderer->getVignetteTexID = pfunc;
}

int NXT_ThemeRenderer_GetLutTexWithID(NXT_HThemeRenderer renderer, int lut_resource_id){

	if(renderer->getLutTextureWithID == NULL)
		return 0;
#if defined(ANDROID)
	return (*renderer->getLutTextureWithID)(lut_resource_id, renderer->export_mode_);
#elif defined(__APPLE__)
    return (*renderer->getLutTextureWithID)(lut_resource_id, renderer->rendererType == NXT_RendererType_InternalExportContext );
#endif
}

int NXT_ThemeRenderer_GetEffectImagePath(NXT_HThemeRenderer renderer, const char* src, char* output){

	if(NULL == renderer->getEffectImagePath)
		return 0;
	return (*renderer->getEffectImagePath)(src, output);
}

int NXT_ThemeRenderer_GetVignetteTexID(NXT_HThemeRenderer renderer){

	if(NULL == renderer->getVignetteTexID)
		return renderer->tex_id_for_vignette_ = 0;
	return renderer->tex_id_for_vignette_ = (*renderer->getVignetteTexID)(renderer->export_mode_);
}

void NXT_Theme_ResetTextureManager(NXT_HThemeRenderer renderer){

	NXT_Theme_ResetTextureManager_Internal(renderer->getTexManager());
}

void NXT_Theme_SetTexture360Flag(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, unsigned int flag)
{
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGI("[%s %d] texture_id range error (%d)", __func__, __LINE__, (int)texture_id);
        return;// NXT_Error_ParamRange;
    }
	
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    tex->is360video = flag;
    
    LOGI("[%s %d] texture_id:%d flag:%d", __func__, __LINE__, (int)texture_id, flag);
}

void NXT_ThemeRenderer_set360View(NXT_HThemeRenderer renderer, float x, float y){
	
	renderer->moveX_value = x / 360.0f;
	renderer->moveY_value = (y + 90.0f) / 180.0f;
}

void NXT_ThemeRenderer_SetExportMode(NXT_HThemeRenderer renderer){

	renderer->export_mode_ = 1;
}

void NXT_ThemeRenderer_GetRenderItem(NXT_HThemeRenderer renderer, const char* pid, const char* uid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* cbdata){
    
	NXT_Theme_GetRenderItem(renderer->getRenderItemManager(), pid, uid, src, child_count, pfunc, (void*)renderer->loadImageCallback, (void*)renderer->freeImageCallback, cbdata);
}

void NXT_ThemeRenderer_AddRenderItem(NXT_HThemeRenderer renderer, void* pitem){

	NXT_Theme_AddRenderItemToManager(renderer->getRenderItemManager(), pitem);
}

int NXT_ThemeRenderer_CountOfRenderItem(NXT_HThemeRenderer renderer){

	return NXT_Theme_CountOfRenderItem(renderer->getRenderItemManager());
}

int NXT_ThemeRenderer_GetRenderItemEffectID(NXT_HThemeRenderer renderer, const char* id){

	return NXT_Theme_GetEffectID(renderer->getRenderItemManager(), id);
}

void NXT_ThemeRenderer_ClearRenderItems(NXT_HThemeRenderer renderer){

	LOGI("[%s %d]", __func__, __LINE__);
	NXT_Theme_ClearRenderItems(renderer->getRenderItemManager());
}

int NXT_ThemeRenderer_CreateRenderItem(NXT_HThemeRenderer renderer, int effect_id){

	int i = 0;
	for(i = 0; i < 100; ++i){

		if(renderer->overlayEffectMark[i] == 0){

			renderer->overlayEffectMark[i] = 1;
			renderer->overlayEffect[i].render_item_id = effect_id;
			return i;
		}
	}

    return -1;
}

int NXT_ThemeRenderer_CreateNexEDL(NXT_HThemeRenderer renderer, const char* uid, NXT_HThemeSet themeset){

	int i = 0;
	for(i = 0; i < 100; ++i){

		if(renderer->overlayEffectMark[i] == 0){

			renderer->overlayEffectMark[i] = 1;
            strcpy(renderer->overlayEffect[i].effectId, uid);
			renderer->overlayEffect[i].nexEDLItem = NXT_ThemeSet_GetEffectById(themeset, uid);
			return i;
		}
	}

    return -1;
}

void NXT_ThemeRenderer_ReleaseNexEDL(NXT_HThemeRenderer renderer, int effect_id){

	if(effect_id < 0){

		int i = 0;
		for(i = 0; i < 100; ++i){

            NXT_AppliedEffect* appliedEffect = &renderer->overlayEffect[i];
			renderer->overlayEffectMark[i] = 0;
            if(appliedEffect->nexEDLItem){

                NXT_ThemeRenderer_ReleaseKEDLEffectID(renderer, appliedEffect->effectId, 0);
                appliedEffect->nexEDLItem = NULL;
            }
			freeAppliedEffect(appliedEffect); 
		}
		
		return;
	}

    NXT_AppliedEffect* appliedEffect = &renderer->overlayEffect[effect_id];

    if(appliedEffect->nexEDLItem){

        NXT_ThemeRenderer_ReleaseKEDLEffectID(renderer, appliedEffect->effectId, 0);
        appliedEffect->nexEDLItem = NULL;
    }
	renderer->overlayEffectMark[effect_id] = 0;
	freeAppliedEffect(appliedEffect);
}

void NXT_ThemeRenderer_ReleaseRenderItem(NXT_HThemeRenderer renderer, int effect_id){

    NXT_ThemeRenderer_AquireContext(renderer);
	NXT_ThemeRenderer_ReleaseNexEDL(renderer, effect_id);
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}

void NXT_ThemeRenderer_DrawRenderItemOverlay(NXT_HThemeRenderer renderer, int render_item_proxy_id, int tex_id_for_second, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha, int mask_enabled){

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	NXT_AppliedEffect* appliedEffect = &renderer->overlayEffect[render_item_proxy_id];

	int render_item_id = appliedEffect->render_item_id;
	setAppliedEffect(renderer, appliedEffect, NXT_EffectType_Transition, "renderitem",  // ID of clip effect, or NULL to clear current effect
                             effect_options,
                             0,         // Index of this clip, from 0
                             1,    // Total number of clips in the project
                             0,     // Start time for this clip
                             0,       // End time for this clip
                             0,   // Start time for this effect (must be >= clipStartTime)
                             0);
    NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), render_item_id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);

	applyRenderItemOption(renderer, appliedEffect);
	
	NXT_Theme_SaveSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
    NXT_Theme_SaveSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
    NXT_Theme_SaveSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_RIGHT);
    NXT_Theme_ClearSrcInfoTarget(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
    NXT_Theme_ClearSrcInfoTarget(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
    NXT_Theme_ClearSrcInfoTarget(renderer->getRenderItemManager(), E_TARGET_VIDEO_RIGHT);

    NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                        tex_id_for_second, 
                                                        512, 
                                                        512, 
                                                        512,
                                                        512,
                                                        E_TARGET_VIDEO_RIGHT);

	NXT_Theme_SetMatrix(renderer->getRenderItemManager(), pmatrix);
	NXT_Theme_SetRect(renderer->getRenderItemManager(), left, top, right, bottom, alpha, mask_enabled?true:false);
	int elapsed_time = current_time - start_time;
	int duration = end_time - start_time;
	float progress = (float)elapsed_time / (float)duration;
	NXT_Theme_DoEffect(renderer->getRenderItemManager(), elapsed_time, elapsed_time, start_time, end_time, duration, start_time, end_time, 0, 1);
	NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), progress);
	NXT_Theme_ResetMatrix(renderer->getRenderItemManager());

	NXT_Theme_RestoreSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
    NXT_Theme_RestoreSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
    NXT_Theme_RestoreSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_RIGHT);

	NXT_Theme_EndRenderItem(renderer->getRenderItemManager());

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

static float idmatrix[] = {

	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};



NXT_ThemeRenderer_Context::TexShader::~TexShader(){

	glDeleteProgram(program_);
}

void NXT_ThemeRenderer_Context::TexShader::init(){

	if(program_ == 0){

		static const GLchar* vertex_shader_src = {

			"uniform mat4 u_mvp_matrix;\n"
			"attribute vec4 a_position;\n"
			"attribute vec2 a_texCoord;\n"
			"varying highp vec2 v_texCoord;\n"
			"varying highp vec2 v_texCoord_for_mask;\n"
			"void main() {\n"
			"	v_texCoord = a_texCoord;\n"
			"	gl_Position = a_position * u_mvp_matrix;\n"
			"	v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;\n"
			"}\n"
		};

		static const GLchar* fragment_shader_src = {

			"varying highp vec2 v_texCoord;\n"
			"varying highp vec2 v_texCoord_for_mask;\n"
			"uniform highp float u_alpha;\n"
			"uniform highp int u_reverse;\n"
			"uniform sampler2D u_textureSampler;\n"
			"uniform sampler2D u_textureSampler_for_mask;\n"
			"void main() {\n"
			"	highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
			"	highp vec4 result = (texture2D(u_textureSampler, v_texCoord)).rgba * mask.r * u_alpha;\n"
			"	if(u_reverse > 0) gl_FragColor = result.bgra;\n"
			"	else gl_FragColor = result;\n"
			"}\n"
		};

		char error_text[1024];
		GLsizei length = 0;

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		GLint size = strlen(vertex_shader_src);
		glShaderSource(vertex_shader, 1, &vertex_shader_src, &size);
		glCompileShader(vertex_shader);
		
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		size = strlen(fragment_shader_src);
		glShaderSource(fragment_shader, 1, &fragment_shader_src, &size);
		glCompileShader(fragment_shader);

		
		glGetShaderInfoLog(fragment_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

		program_ = glCreateProgram();

		glAttachShader(program_, vertex_shader); CHECK_GL_ERROR();
		glAttachShader(program_, fragment_shader); CHECK_GL_ERROR();
		glLinkProgram(program_); CHECK_GL_ERROR();

		glDetachShader(program_, vertex_shader);
		glDetachShader(program_, fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		mvp_matrix_ = glGetUniformLocation(program_, "u_mvp_matrix"); CHECK_GL_ERROR();
		tex_sampler_ = glGetUniformLocation(program_, "u_textureSampler"); CHECK_GL_ERROR();
		mask_sampler_ = glGetUniformLocation(program_, "u_textureSampler_for_mask"); CHECK_GL_ERROR();
		alpha_ = glGetUniformLocation(program_, "u_alpha"); CHECK_GL_ERROR();
		reverse_ = glGetUniformLocation(program_, "u_reverse"); CHECK_GL_ERROR();
		a_pos_ = glGetAttribLocation(program_, "a_position"); CHECK_GL_ERROR();
		a_tex_ = glGetAttribLocation(program_, "a_texCoord"); CHECK_GL_ERROR();
	}
}

void NXT_ThemeRenderer_::drawTexBox(int tex, bool mask_enabled, float* pmatrix, float left, float top, float right, float bottom, float alpha, int no_filter, int reverse){

    NXT_HThemeRenderer_Context ctx = this->getContextManager();
    NXT_ThemeRenderer_Context::TexShader& tex_ = ctx->tex_;
	tex_.init();
	glUseProgram(tex_.program_);

	glUniformMatrix4fv(tex_.mvp_matrix_, 1, false, pmatrix?pmatrix:idmatrix);
	glUniform1f(tex_.alpha_, alpha);
	glUniform1i(tex_.reverse_, reverse);

	glActiveTexture(GL_TEXTURE0);                                        		  CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex);									          CHECK_GL_ERROR();
	if(no_filter){

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glUniform1i(tex_.tex_sampler_, 0);                                            CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE1);                                        		  CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, mask_enabled?NXT_ThemeRenderer_GetTextureNameForMask(this):ctx->whiteTexture);    CHECK_GL_ERROR();
	glUniform1i(tex_.mask_sampler_, 1);                                           CHECK_GL_ERROR();

	float texcoord_buffer[] =
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,

	};

	

	float position_buffer[] =
	{
		left, bottom, 0, 1,
		left, top, 0, 1,
		right, bottom, 0, 1,
		right, top, 0, 1,
	};

	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(tex_.a_pos_); CHECK_GL_ERROR();
	glEnableVertexAttribArray(tex_.a_tex_); CHECK_GL_ERROR();
	glVertexAttribPointer(tex_.a_pos_, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(tex_.a_tex_, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();

	if(no_filter){

		glActiveTexture(GL_TEXTURE0);                                        		  CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, tex);									          CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

#include "NexTheme_RendererState.h"

void NXT_ThemeRenderer_DrawNexEDLOverlay3D(NXT_HThemeRenderer renderer, int render_item_proxy_id, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha){

    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    NXT_AppliedEffect* appliedEffect = &renderer->overlayEffect[render_item_proxy_id];

    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    RendererState state;
    state << renderer;
    NXT_Matrix4f tmp;
    memcpy(tmp.e, pmatrix, sizeof(float)*16);
    NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(tmp, NXT_Matrix4f_Scale(float(right - left) * 0.5f, float(top - bottom) * 0.5f, 1.0f));
    renderer->basetransform = NXT_Matrix4f_MultMatrix(renderer->basetransform, mvp);
    renderer->proj = NXT_Matrix4f_Identity();
    renderer->transform = NXT_Matrix4f_Identity();
    renderer->pActiveEffect = appliedEffect;
    calcLocalTimeFromCurrentEffectAndCTS(renderer);

    NXT_TextureInfo* texinfo = &renderer->getVideoSrc(NXT_TextureID_Video_1);
    GLuint texid = texinfo->texName_for_rgb;
    GLuint texname_backup = texinfo->texName_for_rgb;
    NXT_PixelFormat textureFormat_backup = texinfo->textureFormat;
    texinfo->texName_for_rgb = renderer->getContextManager()->emptyTexture;
    texinfo->textureFormat = NXT_PixelFormat_RGBA8888;

    NXT_ThemeRenderer_SetAppliedEffect(renderer,
                         renderer->pActiveEffect,
                         NXT_EffectType_Transition,
                         appliedEffect->effectId,  // ID of clip effect, or NULL to clear current effect
                         effect_options,
                         0,         // Index of this clip, from 0
                         1,    // Total number of clips in the project
                         start_time,     // Start time for this clip
                         end_time,       // End time for this clip
                         start_time,   // Start time for this effect (must be >= clipStartTime)
                         end_time);

    NXT_Effect_UpdateUserFields(appliedEffect->nexEDLItem, renderer);

//Do something to render kedl item
    NXT_HEffect effect = appliedEffect->nexEDLItem;
    NXT_ThemeRenderer_TimeClipEffect(renderer, effect, renderer->cur_time, start_time, end_time);
    NXT_ThemeRenderer_RenderEffect(renderer, effect, renderer->cur_time);

    texinfo->texName_for_rgb = texname_backup;
    texinfo->textureFormat = textureFormat_backup;
    
    state >> renderer;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void NXT_ThemeRenderer_DrawNexEDLOverlay2D(NXT_HThemeRenderer renderer, int render_item_proxy_id, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha){

    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    NXT_AppliedEffect* appliedEffect = &renderer->overlayEffect[render_item_proxy_id];

    NXT_RenderTarget* pdepthonly = NXT_ThemeRenderer_GetDepthonly(renderer, renderer->view_width, renderer->view_height, 1);
    NXT_RenderTarget* prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 1, 0, 0);
    NXT_RenderTarget* pbackup_main_rendertarget = renderer->pmain_render_target_;
    renderer->pmain_render_target_ = prender_target;
    NXT_ThemeRenderer_SetRenderTargetCombined(renderer, prender_target, pdepthonly);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    RendererState state;
    state << renderer;
    NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform);
    renderer->basetransform = NXT_Matrix4f_MultMatrix(renderer->basetransform, mvp);
    renderer->proj = NXT_Matrix4f_Identity();
    renderer->transform = NXT_Matrix4f_Identity();
    renderer->pActiveEffect = appliedEffect;
    calcLocalTimeFromCurrentEffectAndCTS(renderer);

    NXT_TextureInfo* texinfo = &renderer->getVideoSrc(NXT_TextureID_Video_1);
    GLuint texid = texinfo->texName_for_rgb;
    GLuint texname_backup = texinfo->texName_for_rgb;
    NXT_PixelFormat textureFormat_backup = texinfo->textureFormat;
    texinfo->texName_for_rgb = renderer->getContextManager()->emptyTexture;
    texinfo->textureFormat = NXT_PixelFormat_RGBA8888;

    NXT_ThemeRenderer_SetAppliedEffect(renderer,
                         renderer->pActiveEffect,
                         NXT_EffectType_Transition,
                         appliedEffect->effectId,  // ID of clip effect, or NULL to clear current effect
                         effect_options,
                         0,         // Index of this clip, from 0
                         1,    // Total number of clips in the project
                         start_time,     // Start time for this clip
                         end_time,       // End time for this clip
                         start_time,   // Start time for this effect (must be >= clipStartTime)
                         end_time);

    NXT_Effect_UpdateUserFields(appliedEffect->nexEDLItem, renderer);

//Do something to render kedl item
    NXT_HEffect effect = appliedEffect->nexEDLItem;
    NXT_ThemeRenderer_TimeClipEffect(renderer, effect, renderer->cur_time, start_time, end_time);
    NXT_ThemeRenderer_RenderEffect(renderer, effect, renderer->cur_time);

    texinfo->texName_for_rgb = texname_backup;
    texinfo->textureFormat = textureFormat_backup;
    
    state >> renderer;
    renderer->pmain_render_target_ = pbackup_main_rendertarget;
    NXT_ThemeRenderer_UnbindDepth(renderer, prender_target);
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, pdepthonly);
    NXT_ThemeRenderer_SetRenderToDefault(renderer);

//Draw Texture
    renderer->drawTexBox(prender_target->target_texture_, false, pmatrix, left, top, right, bottom, alpha);
//Relase Rendertarget
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, prender_target);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

NXT_HThemeSet NXT_ThemeRenderer_GetKEDLEffectID(NXT_HThemeRenderer renderer, const char* id){

	KEDLManager* pmanager = (KEDLManager*)renderer->getKedlItemManager();
	return pmanager->get(id);
}

NXT_HThemeSet NXT_ThemeRenderer_GetKEDLItem(NXT_HThemeRenderer renderer, const char* uid, const char* src){

    LOGI("%s %d uid:%s", __func__, __LINE__, uid);

	KEDLManager* pmanager = (KEDLManager*)renderer->getKedlItemManager();
	return pmanager->put(uid, src);
}

void NXT_ThemeRenderer_DrawNexEDLOverlay(NXT_HThemeRenderer renderer, int render_item_proxy_id, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha, int mode){

    if(mode)
        NXT_ThemeRenderer_DrawNexEDLOverlay3D(renderer, render_item_proxy_id, effect_options, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha);
    else
        NXT_ThemeRenderer_DrawNexEDLOverlay2D(renderer, render_item_proxy_id, effect_options, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha);
}

void NXT_ThemeRenderer_ReleaseKEDLEffectID(NXT_HThemeRenderer renderer, const char* id, int isDetachedConext){

    
    NXT_ThemeRenderer_ReleaseKEDLEffectIDByContext(renderer->getContextManager(), id, isDetachedConext);
}

void NXT_ThemeRenderer_ReleaseKEDLEffectIDByContext(NXT_HThemeRenderer_Context context, const char* id, int isDetachedConext){

    KEDLManager* pmanager = (KEDLManager*)context->getKedlItemManager();
    pmanager->release(id, isDetachedConext);
}

int NXT_ThemeRenderer_CheckUploadOk(NXT_HThemeRenderer renderer){
#ifdef ANDROID
	if(renderer->nativeExportSurface != EGL_NO_SURFACE || renderer->nativeWindowSurface != EGL_NO_SURFACE) {
        return 1;
    }
#endif
#ifdef USE_PLATFORM_SUPPORT
    return NXT_Platform_PresentBuffer(renderer, PRESENT_MODE_PREVIEW) == NXT_Error_None ? 1 : 0;
#endif
    return 0;
}

bool NXT_ThemeRenderer_SetEffectOptions(NXT_HThemeRenderer renderer, unsigned int uid, const char* effectOptions){

    return renderer->setEffectOption(uid, effectOptions);
}


unsigned int NXT_ThemeRenderer_StartTitleEDL(NXT_HThemeRenderer renderer,
                                    const char* effectId,
                                    const char* pEffectOptions,
                                    int startTrackId,
                                    int effectIndex,
                                    int totalEffectCount,
                                    int effectStartTime,
                                    int effectEndTime){

    return renderer->startTitleEDL(effectId, pEffectOptions, startTrackId, effectIndex, totalEffectCount, effectStartTime, effectEndTime);
}

unsigned int NXT_ThemeRenderer_StartTransitionEDL(NXT_HThemeRenderer renderer,
                                            const char* effectId,
                                            const char* pEffectOptions,
                                            int effectIndex,
                                            int totalEffectCount,
                                            int effectStartTime,
                                            int effectEndTime){


    return renderer->startTransitionEDL(effectId, pEffectOptions, effectIndex, totalEffectCount, effectStartTime, effectEndTime);
}

void NXT_ThemeRenderer_EndEDL(NXT_HThemeRenderer renderer, unsigned int effect_uid){

    renderer->endEDL(effect_uid);
}

#include "nexTheme_CubeLUT.h"

int* NXT_ThemeRenderer_CreateCubeLUT(const char* stream){

    CubeLUT cubelut;
    std::string tmp_string(stream);
    std::istringstream tmp(tmp_string);
    cubelut.LoadCubeFile(tmp);
    return cubelut.createLUT();
}

// int* NXT_ThemeRenderer_CreateLGLUT_backup(const char* stream){

//     int* dst = new int[64*4096];
//     const char* readp = stream;
//     for(int h = 0; h < 64; ++h){

//         for(int i = 0; i < 64; ++i){

//             for(int j = 0; j < 64; ++j){

//                 unsigned char Y = (unsigned char)(*readp++);
//                 unsigned char Cb = (unsigned char)(*readp++);
//                 unsigned char Cr = (unsigned char)(*readp++);
//                 readp++;

//                 int r = (int)((float)Y + 1.40200*(float)(Cr - 0x80));
//                 int g = (int)((float)Y - 0.34414*(float)(Cb - 0x80) - 0.71414*(float)(Cr - 0x80));
//                 int b = (int)((float)Y + 1.77200*(float)(Cb - 0x80));

//                 int A = 255<<24;
//                 int R = max(0, min(255, r))<<16;
//                 int G = max(0, min(255, g))<<8;
//                 int B = max(0, min(255, b));

//                 dst[63*64*64 - h*64*64 + 63*64 - i*64 + j] = A|R|G|B;
//             }
//         }
//     }

//     return dst;
// }

int* NXT_ThemeRenderer_CreateLGLUT(const char* stream){

    int* dst = new int[64*4096];

    for(int b = 0; b < 64; ++b){

        for(int r = 0; r < 64; ++r){

            for(int g = 0; g < 64; ++g){

                float R = (float)r / 64.0f;
                float G = (float)g / 64.0f;
                float B = (float)b / 64.0f;

                float y = ((0.299 * R) + (0.587 * G) + (0.114 * B));
                float u = (-(0.169 * R) - (0.331 * G) + (0.500 * B)) + 0.5;
                float v = ((0.500 * R) - (0.419 * G) - (0.081 * B)) + 0.5;

                int ypos = max(min((int)(y * 64.0f), 63), 0);
                int upos = max(min((int)(u * 64.0f), 63), 0);
                int vpos = max(min((int)(v * 64.0f), 63), 0);

                int pos_in_stream = (ypos + upos*64 + vpos*64*64)*4;
                const char* readp = stream + pos_in_stream;
                unsigned char Y = (unsigned char)(*readp++);
                unsigned char Cb = (unsigned char)(*readp++);
                unsigned char Cr = (unsigned char)(*readp++);

                int conv_r = (int)((float)Y + 1.40200*(float)(Cr - 0x80));
                int conv_g = (int)((float)Y - 0.34414*(float)(Cb - 0x80) - 0.71414*(float)(Cr - 0x80));
                int conv_b = (int)((float)Y + 1.77200*(float)(Cb - 0x80));

                conv_r = max(min(conv_r, 255), 0);
                conv_g = max(min(conv_g, 255), 0);
                conv_b = max(min(conv_b, 255), 0);

                dst[63*64*64 - b*64*64 + 63*64 - r*64 + g] = (255<<24)|(conv_r<<16)|(conv_g<<8)|conv_b;
            }
        }
    }
    
    return dst;
}

void NXT_ThemeRenderer_SetForceRTT(NXT_HThemeRenderer renderer, int flag){

    renderer->setForceRTT(flag);
}

int NXT_ThemeRenderer_GetForceRTT(NXT_HThemeRenderer renderer){

    return renderer->getForceRTT();
}

void NXT_ThemeRenderer_::setBrightness(int value){

    final_brightness_ = value;
}

void NXT_ThemeRenderer_::setContrast(int value){

    final_contrast_ = value;
}

void NXT_ThemeRenderer_::setSaturation(int value){

    final_saturation_ = value;
}

int NXT_ThemeRenderer_::getBrightness(){

    return final_brightness_;
}

int NXT_ThemeRenderer_::getContrast(){

    return final_contrast_;
}

int NXT_ThemeRenderer_::getSaturation(){

    return final_saturation_;
}

void NXT_ThemeRenderer_::setDeviceLightLevel(int lightLevel){
    deviceLightLevel = lightLevel;
}
void NXT_ThemeRenderer_::setDeviceGamma(float gamma){
    deviceGamma = gamma;
}
int NXT_ThemeRenderer_::getDeviceLightLevel(){
    return deviceLightLevel;
}
float NXT_ThemeRenderer_::getDeviceGamma(){
    return deviceGamma;
}

void NXT_ThemeRenderer_::setVignette(int value){

    final_vignette_ = value;
}

int NXT_ThemeRenderer_::getVignette(){

    return final_vignette_;
}
void NXT_ThemeRenderer_::setVignetteRange(int value){

    final_vignetteRange_ = value;
}

int NXT_ThemeRenderer_::getVignetteRange(){

    return final_vignetteRange_;
}

void NXT_ThemeRenderer_::setSharpness(int value){

    final_sharpness_ = value;
}

int NXT_ThemeRenderer_::getSharpness(){

    return final_sharpness_;
}

void NXT_ThemeRenderer_SetBrightness(NXT_HThemeRenderer renderer, int value){

    renderer->setBrightness(value);
}

void NXT_ThemeRenderer_SetContrast(NXT_HThemeRenderer renderer, int value){

    renderer->setContrast(value);
}

void NXT_ThemeRenderer_SetSaturation(NXT_HThemeRenderer renderer, int value){

    renderer->setSaturation(value);
}

int NXT_ThemeRenderer_GetBrightness(NXT_HThemeRenderer renderer){

    return renderer->getBrightness();
}

int NXT_ThemeRenderer_GetContrast(NXT_HThemeRenderer renderer){

    return renderer->getContrast();
}

int NXT_ThemeRenderer_GetSaturation(NXT_HThemeRenderer renderer){

    return renderer->getSaturation();
}

void NXT_ThemeRenderer_SetVignette(NXT_HThemeRenderer renderer, int value){

    renderer->setVignette(value);
}

int NXT_ThemeRenderer_GetVignette(NXT_HThemeRenderer renderer){

    return renderer->getVignette();
}
void NXT_ThemeRenderer_SetVignetteRange(NXT_HThemeRenderer renderer, int value){

    renderer->setVignetteRange(value);
}

int NXT_ThemeRenderer_GetVignetteRange(NXT_HThemeRenderer renderer){

    return renderer->getVignetteRange();
}

void NXT_ThemeRenderer_SetSharpness(NXT_HThemeRenderer renderer, int value){

    renderer->setSharpness(value);
}

int NXT_ThemeRenderer_GetSharpness(NXT_HThemeRenderer renderer){

    return renderer->getSharpness();
}

//TODO
//many duplicated codes...for src capture
//refactoring for this should be done before 2017/09/15

static void render_internal_for_capturesrc( NXT_HThemeRenderer renderer )
{
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return;
    }

    renderer->basetransform = NXT_Matrix4f_Identity();

    NXT_Node_Texture textureNode1 = {0};
    textureNode1.textureType = NXT_TextureType_Video;
    textureNode1.videoSource = 1;
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    
    int first_processed = NexThemeRenderer_PrepStateForRender(renderer,&textureNode1,NULL,&texture,&mask, 1);
    GLfloat vertexCoord[] = {
        -1.0,   1.0,
         1.0,   1.0,
        -1.0,  -1.0,   
         1.0,  -1.0
    };
    GLfloat tmpCoords[8] = {0};
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,tmpCoords);
    CHECK_GL_ERROR();
    
    
    // NexThemeRenderer_PrepStateForRender(renderer,&textureNode1,NULL,&texture,&mask, 1);
    float lowx = NexThemeRenderer_AdjTexCoordX(texture,0);
    float lowy = NexThemeRenderer_AdjTexCoordY(texture,0);
    float highx = NexThemeRenderer_AdjTexCoordX(texture,1);
    float highy = NexThemeRenderer_AdjTexCoordY(texture,1);
    LOGI("[%s %d] texture coordinates = %f %f %f %f", __func__, __LINE__, lowx, lowy, highx, highy);
    GLfloat texCoords[] = {
        lowx,       lowy,
        highx,      lowy,
        lowx,       highy,
        highx,      highy
    };
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
    CHECK_GL_ERROR();
    glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
    CHECK_GL_ERROR();
    LOGI("[%s %d] About to run glDrawArrays", __func__, __LINE__);
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    CHECK_GL_ERROR();
    LOGD("[%s %d] END", __func__, __LINE__);
}

static NXT_Error NXT_ThemeRenderer_GLDraw_Main_For_SrcCapture(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType){
    NXT_RendererOutputType targetOutputType = outputType;
    
    unsigned int renderer_view_width = 0;
    unsigned int renderer_view_height = 0;
    outputType = NXT_RendererOutputType_RGBA_8888;
    int forced = ((outputType != targetOutputType)||(renderer->only_rgb_possible_surface_))?1:renderer->getForceRTT();
    NXT_Error err = beginRenderer(renderer, outputType);
    if (NXT_Error_None != err)
        return err;

    renderer_view_width = renderer->view_width;
    renderer_view_height = renderer->view_height;
    setRenderTargetForRTT(renderer, outputType, forced);
    renderer->setGLRenderStart();

    NXT_ThemeRenderer_UpdateRenderTargetManager(renderer);
    NXT_Theme_UpdateTextureManager(renderer->getTexManager());
    NXT_Theme_UpdateRenderItemManager(renderer->getRenderItemManager(), renderer->pmain_mask_target_->target_texture_, renderer->pmain_blend_target_->target_texture_, NXT_ThemeRenderer_GetTextureNameForWhite(renderer));

    //For simplifying the solution, I will render scene with only RGBA output.
    //Color space conversion will be done with result RGBA Render Target Texture.

    NXT_BEGIN_PROFILE("Render for output (RGBA_8888)");

    setViewportForRGB(renderer);
    renderer->setGLStateForRGBAOutput();
    
    render_internal_for_capturesrc(renderer);
#ifdef DEBUG_OUTPUT
    static int dbg_pos = 0;
    renderSimpleBoxForDebug(renderer, dbg_pos);
    dbg_pos = (dbg_pos + 1) % 4;
#endif//DEBUG_OUTPUT
    NXT_END_PROFILE();
    renderer->scaleDownRender();
    renderer->view_width = renderer_view_width;
    renderer->view_height = renderer_view_height;
    outputType = targetOutputType;
    convertColorspace(renderer, outputType);
    freeReadpixels(renderer, outputType);
    resetMainRenderTarget(renderer);
    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_GLDrawSrc(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int uid){

    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }

    NXT_HThemeRenderer_Context ctx = renderer->getContextManager();
    
    if( !ctx->contextIsAcquired && renderer->bIsInternalContext ) {
        LOGW("[%s %d] WARNING: Attempt to render with no context; BYPASSING", __func__, __LINE__);
        return NXT_Error_None;
    }

    int src_id = renderer->getDecoVideoSrcForUID(uid);
    LOGI("[%s %d] begin src_id:%d", __func__, __LINE__, src_id);

    if( renderer->bIsInternalContext ) {
#ifdef ANDROID
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
#ifdef USE_PLATFORM_SUPPORT
        if (NXT_Platform_CheckInternalContextAvailable(renderer) != NXT_Error_None) {
            LOGE("[%s %d] NO EGL CONTEXT", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
#endif
    }

    renderer->lut_texture_ = 0;    

    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();

    NXT_Error result = NXT_Error_None;

    if(NXT_RendererOutputType_JUST_CLEAR == outputType)
        glClear(GL_COLOR_BUFFER_BIT);
    else {

        renderer->setVideoSrc(NXT_TextureID_Video_1, &renderer->getRawVideoSrc(src_id));
        result = NXT_ThemeRenderer_GLDraw_Main_For_SrcCapture(renderer, outputType);
    }
    
    CHECK_GL_ERROR();
    CHECK_EGL_ERROR();
    return result;
}

int NXT_ThemeRenderer_SetDeviceLightLevel(NXT_HThemeRenderer renderer, int lightLevel){

    renderer->setDeviceLightLevel(lightLevel);
    return 1;
}

int NXT_ThemeRenderer_SetDeviceGamma(NXT_HThemeRenderer renderer, float gamma){

    renderer->setDeviceGamma(gamma);
    return 1;
}


NXT_ThemeRenderer_Context::NXT_ThemeRenderer_Context(int max_texture_cache_mem_size, unsigned int bIsInternalContext, unsigned int bIsMI5C):
#ifdef ANDROID
	  eglContext(EGL_NO_CONTEXT),
#endif
	  ptex_manager_(NULL),
	  prenderitem_manager_(NULL),
	  pkedlitem_manager_(NULL){

#ifdef ANDROID
    dummy_surface_ = NULL;
#endif
    contextIsAcquired = 0;
    memset(&contextOwner, 0x0, sizeof(contextOwner));
    memset(&contextLock, 0x0, sizeof(contextLock));
    pthread_mutex_init(&contextLock,NULL);
    prenderitem_manager_ = NXT_Theme_CreateRenderItemManager();
    ptex_manager_ = NXT_Theme_CreateTextureManager(10000000, max_texture_cache_mem_size);
    pkedlitem_manager_ = new KEDLManager(this);

    bRequestMultisample = 0;
	requestDepthBufferSize = 16;

#ifdef ANDROID
    eglDisplay = EGL_NO_DISPLAY;
    memset(&eglConfig, 0x0, sizeof(eglConfig));
#endif

    emptyTexture = 0;
    whiteTexture = 0;

    if(bIsInternalContext) {
#ifdef ANDROID
		NXT_Error result;
        if((result = createInternalContext(this, bIsMI5C))!=NXT_Error_None) {
            LOGF("[%s %d] Create internal context failed (%d)", __func__, __LINE__, result);
        }
        EGLint attr_list[] = {
            EGL_HEIGHT, 1,
            EGL_WIDTH, 1,
            EGL_TEXTURE_TARGET, EGL_NO_TEXTURE, // EGL_NO_TEXTURE, EGL_TEXTURE_2D
            EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE, // EGL_NO_TEXTURE, EGL_TEXTURE_RGB, EGL_TEXTURE_RGBA
            EGL_NONE
        };
        dummy_surface_ = eglCreatePbufferSurface(eglDisplay, eglConfig, attr_list);
        CHECK_EGL_ERROR();
#endif
    }

#ifdef ANDROID
    if(dummy_surface_){
    
        if(eglMakeCurrent(eglDisplay, dummy_surface_, dummy_surface_, eglContext) != EGL_TRUE) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglMakeCurrent failed", __func__, __LINE__);
        }
    }
#endif
          
    emptyTexture = whiteTexture = 0;
          
#ifdef ANDROID
    if(dummy_surface_){

        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
#endif
}
                                                      
void NXT_ThemeRenderer_Context::initDefaultTextures() {
    GLbyte emptybuf[8*8*4] = {0};

    if (emptyTexture == 0) {
        glActiveTexture(GL_TEXTURE0);
        GL_GenTextures(1, &emptyTexture);
        glBindTexture(GL_TEXTURE_2D, emptyTexture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        CHECK_GL_ERROR();
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptybuf);
    }
    if (whiteTexture == 0) {
        glActiveTexture(GL_TEXTURE0);
        GL_GenTextures(1, &whiteTexture);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERROR();
        {
            int i = 0;
            for (i = 0; i < sizeof(emptybuf); ++i)
                emptybuf[i] = 0xff;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, emptybuf);
    }
}

NXT_ThemeRenderer_Context::~NXT_ThemeRenderer_Context(){

    if(contextIsAcquired) {
        if(!pthread_equal(pthread_self(), contextOwner)) {
            LOGE("[%s %d] Attempt to acquire context while already aquired in same thread; would deadlock.(contextIsAcquired:%d)", __func__, __LINE__, contextIsAcquired);
        }
    }
    pthread_mutex_lock(&contextLock);
    pthread_t exContextOwner = contextOwner;
    contextOwner = pthread_self();
    contextIsAcquired = 1;

#ifdef ANDROID
    if(dummy_surface_){
    
        if(eglMakeCurrent(eglDisplay, dummy_surface_, dummy_surface_, eglContext) != EGL_TRUE) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglMakeCurrent failed(0x%X)", __func__, __LINE__, LOGPTR(exContextOwner));
        }
    }
#endif

    KEDLManager* pkedlmanager = (KEDLManager*)pkedlitem_manager_;
    if(pkedlmanager)
        pkedlmanager->destroy(0);

    NXT_Theme_DestroyRenderItemManager(prenderitem_manager_);
    NXT_Theme_ReleaseTextureManager(ptex_manager_, 0);
    render_target_manager_.destroy();
#ifdef LOAD_SHADER_ON_TIME
    NexThemeRenderer_UnloadShaders2(this);
#endif
    if(whiteTexture > 0)
        GL_DeleteTextures(1, &whiteTexture);
    if(emptyTexture > 0)
        GL_DeleteTextures(1, &emptyTexture);
        
#ifdef ANDROID
    if(dummy_surface_){

        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(eglDisplay, dummy_surface_);
    }
#endif
    CHECK_EGL_ERROR();
    pthread_mutex_unlock(&contextLock);
    pthread_mutex_destroy(&contextLock);
}

void NXT_ThemeRenderer_::lockCachedBitmap(){

    pthread_mutex_lock(&cachedbitmaplock_);
}
void NXT_ThemeRenderer_::unlockCachedBitmap(){

    pthread_mutex_unlock(&cachedbitmaplock_);
}

int NXT_ThemeRenderer_::getCachedBitmap(const char* key, NXT_ImageInfo* pinfo, unsigned int live_start, unsigned int live_end){

    
	CachedBitmapMapper_t::iterator itor = cachedbitmap_mapper_.find(std::string(key));
	if(itor != cachedbitmap_mapper_.end()){

		(*pinfo) = itor->second.info_;
        itor->second.live_start_ = live_start;
        itor->second.live_end_ = live_end;
		return 1;
	}
	return 0;
}

int NXT_ThemeRenderer_::addCachedBitmap(const char* key, NXT_ImageInfo* pinfo, unsigned int live_start, unsigned int live_end){

	CachedBitmapMapper_t::iterator itor = cachedbitmap_mapper_.find(std::string(key));
	if(itor == cachedbitmap_mapper_.end()){

        NXT_ImageInfo_Cache cache;
        cache.info_ = *pinfo;
        if(live_start || live_end){

            cache.live_start_ = live_start;
            cache.live_end_ = live_end;
        }
		cachedbitmap_mapper_.insert(make_pair(std::string(key), cache));
		return 1;
	}
	return 0;
}

void NXT_ThemeRenderer_::clearCachedBitmap(unsigned int live_start, unsigned int live_end){

	for(CachedBitmapMapper_t::iterator itor = cachedbitmap_mapper_.begin(); itor != cachedbitmap_mapper_.end();){

		if(freeImageCallback){

			NXT_ImageInfo_Cache& cache = itor->second;
            if(cache.live_end_ < live_start){

                LOGI("clearCachedBitmap %s", (itor->first).c_str());
                NXT_ImageInfo& imginfo = cache.info_;
			    imginfo.freeImageCallback(&imginfo, imageCallbackPvtData);
                CachedBitmapMapper_t::iterator itor_cur = itor;
                itor++;
                cachedbitmap_mapper_.erase(itor_cur);
            }
            else
                ++itor;
		}
	}
    LOGI("clearCachedBitmap %d - remain", cachedbitmap_mapper_.size());
}

void simpleAquireContext(void* prenderer){

    NXT_ThemeRenderer_AquireContext((NXT_HThemeRenderer)prenderer);
}

void simpleReleaseContext(void* prenderer){

    NXT_ThemeRenderer_ReleaseContext((NXT_HThemeRenderer)prenderer, 0);
}

void NXT_ThemeRenderer_DoPlayCacheWork(NXT_HThemeRenderer renderer, NXT_PrecacheResourceInfo* pinfo, unsigned int live_start, unsigned int live_end){

    if(pinfo->type == 0){

        renderer->lockCachedBitmap();

        if(renderer->loadImageCallback){

            NXT_ImageInfo imgInfo;
            imgInfo.cbprivate0 = NULL;            // Private data for use by freeImageCallback
            imgInfo.cbprivate1 = NULL;            // Private data for use by freeImageCallback

            if(!renderer->getCachedBitmap(pinfo->name, &imgInfo, live_start, live_end)){

                renderer->loadImageCallback(&imgInfo,
                                            (char*)pinfo->name,
                                            0,
                                            renderer->imageCallbackPvtData);
                premultAlpha_internal((unsigned int*)imgInfo.pixels, imgInfo.width*imgInfo.height);
                renderer->addCachedBitmap(pinfo->name, &imgInfo, live_start, live_end);
            }
        }

        renderer->unlockCachedBitmap();
    }
    else if(pinfo->type == 1){

        NXT_Theme_PrecacheRI(renderer->getRenderItemManager(), pinfo, renderer, simpleAquireContext, simpleReleaseContext);
    }	
}

void NXT_ThemeRenderer_ClearCachedBitmap(NXT_HThemeRenderer renderer, unsigned int live_start, unsigned int live_end){

    renderer->lockCachedBitmap();
	renderer->clearCachedBitmap(live_start, live_end);
    renderer->unlockCachedBitmap();
}

void NXT_ThemeRenderer_SetMapper(NXT_HThemeRenderer renderer, void* pmapper) {

    NXT_ThemeRenderer_AquireContext(renderer);
    renderer->setMapper(static_cast<std::map<int, int>*>(pmapper));
    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}
