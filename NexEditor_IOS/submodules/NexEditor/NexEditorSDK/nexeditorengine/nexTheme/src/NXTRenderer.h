//
//  NXTRenderer.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#ifndef __NexVideoEditor__NXTRenderer__
#define __NexVideoEditor__NXTRenderer__

#include <iostream>
#include <memory>
#include <map>
#include <functional>
//#include <set>
#include "NXTDrawingMode.h"
#include "NXTRenderStyle.h"
#include "NXTShader.h"
#include "NXTAppliedEffect.h"
#include "NexTheme_Math.h"
#include "NXTMutex.h"
#include "NXTLayer.h"
#include "NXTSurface.h"
#include "IRenderer.h"
#include "IRenderContext.h"
#include "NXTRenderOptions.h"
#include "NXTEffectOptions.h"
#include "ITexture.h"
#include "NXTBlankTexture.cpp"

class NXTNodeTexture;
class NXTImageAndSettings;

#define  CHECK_GL_ERROR() (check_gl_error(__LINE__,__func__))

enum class NXTRendererType : int {
    Preview        = 1,
    Export         = 2
};

enum class NXTRenderProp : int {
    Multisample            = 1,
    DepthBufferBits        = 2
};
    
void check_gl_error( int line, const char* func );
    
class NXTRenderer : public IRenderer, public IRenderContext {
    
    friend class NXTExternalPixelData;
    
public:
    
    NXTRenderer();
    ~NXTRenderer();
    
    // --- From IRenderer --------------------------------------------------------
    virtual void set_java_vm( JavaVM* vm ) override;
    virtual NXT_Error clear_screen( int iTag ) override;
    virtual void register_image_callbacks(NXT_LoadImageCallback loadImageCallback,
                                          NXT_FreeImageCallback freeImageCallback,
                                          void *cbdata) override;
    virtual void render(NXT_RendererOutputType output_type, const char* options = nullptr) override;
    virtual NXT_Error acquire_context() override;
    virtual NXT_Error release_context( bool bSwapBuffers = false ) override;
    virtual NXT_Error swap_buffers() override;
    virtual void set_cts( unsigned int cts ) override;
    virtual NXT_Error set_preview_surface( ANativeWindow *pNativeWindow, int width=0, int height=0 ) override;
    virtual NXT_Error begin_export( ANativeWindow *pNativeWindow ) override;
    virtual NXT_Error end_export() override;
    virtual const char* get_last_parse_error() override;
    virtual NXT_Error load_effect( const char* effect_id, const char* effect_data ) override;
    virtual NXT_Error unload_effect( const char* effect_id ) override;
    virtual IEffect* get_effect( const char* effect_id ) override;
    //    virtual NXT_Error load_kedl( const char* kedl_data, int iFlags ) override;
    virtual NXT_Error wait_for_render() override;
    virtual NXT_Error get_pixels(int &width, int &height, int &size, void* pdata = nullptr) override;
    virtual NXT_Error set_video_placeholders(const char *video1,
                                             const char *video2) override;
    virtual void set_log_level( int level ) override;
    
    virtual ILayer* get_layer( int layer_id, LayerType layer_type ) override;
    virtual ILayer* create_layer(int layer_id,
                                 LayerType layer_type,
                                 unsigned int start_cts,
                                 unsigned int end_cts) override;
    virtual void delete_layer( int layer_id, LayerType layer_type ) override;
    virtual void setTransitionEffect(const std::string& effect_id,
                                     const std::string& effect_options,
                                     unsigned int start_cts,
                                     unsigned int end_cts);

    
    // --- From IRenderContext ------------------------------------------------------
    virtual void setTexture( const NXTImageAndSettings& texture_image ) override;
    virtual void setMask( const NXTImageAndSettings& texture_image ) override;
    virtual void clearTexture() override;
    virtual void clearMask() override;
    
    virtual bool isMaskActive() override;
    virtual bool isTextureActive() override;
    
    virtual void prepStateForRender() override;
    
    virtual NXTCullFace getCullFace() override;
    virtual void setCullFace( NXTCullFace cullFace ) override;
    virtual NXTVector getColor() override;
    virtual void setColor(const NXTVector& color) override;
    virtual NXTShader& getShader() override;
    virtual void drawArrays(NXTDrawArraysMode mode = NXTDrawArraysMode::TriangleStrip,
                            int first = 0,
                            int count = -1) override;
    
    virtual NXTMatrix& getTransformMatrix(NXTTransformTarget target = NXTTransformTarget::Vertex) override;
    virtual void skipChildren(bool skip=true) override;
    virtual bool isSkipChildren() override;
    
    virtual NXTPart getCurrentPart() override;
    virtual int getTotalClipCount() override;
    virtual int getCurrentClipIndex() override;
    virtual int getEffectStartTime();
    virtual int getEffectEndTime();
    virtual int getTotalProjectTime();
    virtual float getEffectProgressOverall();
    virtual float getEffectProgressInPart();
    
    virtual NXTMatrix& getProjectionMatrix() override;
    virtual int getViewWidth() override;
    virtual int getViewHeight() override;
    virtual NXTRenderStyle& getRenderStyle() override;
    virtual void resetColorMask() override;
    virtual void seed( int seed1, int seed2 ) override;
    virtual float randf() override;
    virtual int getCurrentTime() override;
    virtual NXTAppliedEffect& getActiveEffect() override;
    virtual bool isEffectActive() override;
    virtual NXTShadingMode getShadingMode() override;
    virtual void fillRect( float left, float bottom, float right, float top) override;

    
    // --- Internal ---
    // Used by C wrapper when making and manafging instances
    
    NXT_Error init(NXTRendererType renderType, const std::map<NXTRenderProp,int>& properties);
    
    bool isInitialized() {
        return initialized_;
    }

private:
    
    NXTSurface left_video_surf_;
    NXTSurface right_video_surf_;
    
    bool initialized_ = false;
    bool graphics_initialized_ = false;
    bool colorMask[4] = {true,true,true,true};
    NXT_RandSeed mSeed = (NXT_RandSeed){0,0};
    
//    NXTAppliedEffect watermarkEffect;
//    NXTAppliedEffect transitionEffect;
//    NXTAppliedEffect clipEffect;
//    NXTAppliedEffect noneEffect;
//    int activeEffect = 0;       // 0-none, 1-clip, 2-transition, 3-watermark
    NXTAppliedEffect activeEffect;
    
    std::auto_ptr<NXTShaderInstances> shader_instances_;
    NXTShader* current_shader_;
    
    bool requestMultisample = false;
    bool availableMultisample = false;
    int requestDepthBufferBits = 16;

    EGLContext egl_context_ = EGL_NO_CONTEXT;
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLConfig egl_config_;
    ANativeWindow *native_window_ = nullptr;
    EGLSurface native_surface_ = EGL_NO_SURFACE; // was nativeWindowSurface

    NXTMutex contextLock;
    NXTRendererType renderer_type_;
    JavaVM* java_vm_;
    std::vector<NXTLayer*> layers_;
//    std::multiset<NXTLayer*,layer_compare_> layers_;
    NXT_LoadImageCallback load_image_callback_ = nullptr;
    NXT_FreeImageCallback free_image_callback_ = nullptr;
    void *image_callback_private_data_ = nullptr;
    pthread_t context_owner_;
    bool context_is_acquired_ = false;
    bool is_exporting_ = false;
    
    std::string last_parse_error_;
    
    int surface_width_ = 0;
    int surface_height_ = 0;
    unsigned int cts_ = 0;
    
    NXT_RendererOutputType output_type_;
    
    std::string transition_effect_id_;
    unsigned int transition_start_cts_;
    unsigned int transition_end_cts_;
    NXTEffectOptions transition_effect_options_;
    
    std::map<std::string,IEffect*> loaded_effects_;
//    std::vector<IEffect*> loaded_effects_;
    NXTRenderStyle render_style_;
    
    NXTImageAndSettings texture_;
    NXTImageAndSettings mask_;
    NXTBlankTexture tex_solid_white_{0xFFFFFFFF};
    NXTBlankTexture tex_solid_black_{0x000000FF};
    
    NXTRenderOptions render_options_;
    NXTChannelFilter channel_filter_ = NXTChannelFilter::ALL;

    ITexture *clip_img_left_;       // Left clip image (or only clip image, if not rendering a transition)
    ITexture *clip_img_right_;      // Right clip image (null if not rendering a transition)
    NXTImageSettings *clip_img_left_settings_;
    NXTImageSettings *clip_img_right_settings_;
    
//    NXTRenderStyle renderStyle;
//    NXTDrawingMode drawingMode;
    
    NXTShader* active_shader_;
    
    void setShader( NXTShader& shader );
    
    NXT_Error initGraphics();
    NXT_Error ensureInitEGL();
    NXT_Error createContext();
    
//    NXT_Error render_internal();
    class RenderCB {
    public:
        virtual NXT_Error cb() = 0;
    };

    NXT_Error render_with_output_format_impl( NXT_RendererOutputType output_type, RenderCB* cb );
    
    template <typename CALLBACK> NXT_Error render_with_output_format( NXT_RendererOutputType output_type, CALLBACK c ) {
        class cbwrapper : public RenderCB {
            CALLBACK& cbref_;
        public:
            cbwrapper( CALLBACK& cbref ) : cbref_(cbref) {}
            virtual NXT_Error cb() override {
                cbref_();
            }
        };
        cbwrapper wrapper = cbwrapper(c);
        render_with_output_format_impl( output_type, &wrapper );
    }
    
    NXT_Error render_layer( NXTLayer* layer );
    
    void set_default_state();
    
};

#endif /* defined(__NexVideoEditor__NXTRenderer__) */
