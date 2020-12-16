//
//  IRenderer.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/22/14.
//
//

#ifndef NexVideoEditor_IRenderer_h
#define NexVideoEditor_IRenderer_h

class ILayer;
class IEffect;

enum class LayerType {
    VisualClip, Overlay, Watermark
};

class IRenderer {
public:
    
    IRenderer(const IRenderer& other) = delete;
    IRenderer& operator=( const IRenderer& other) = delete;
    IRenderer() = default;
    
    virtual ~IRenderer(){};

    virtual void set_java_vm( JavaVM* vm ) = 0;
    virtual NXT_Error clear_screen( int iTag ) = 0;
    virtual void register_image_callbacks(NXT_LoadImageCallback loadImageCallback,
                                          NXT_FreeImageCallback freeImageCallback,
                                          void *cbdata) = 0;
    virtual void render(NXT_RendererOutputType output_type, const char* options = nullptr) = 0;
    virtual NXT_Error acquire_context() = 0;
    virtual NXT_Error release_context( bool bSwapBuffers = false ) = 0;
    virtual NXT_Error swap_buffers() = 0;
    virtual void set_cts( unsigned int cts ) = 0;
    virtual NXT_Error set_preview_surface( ANativeWindow *pNativeWindow, int width = 0, int height = 0 ) = 0;
    virtual NXT_Error begin_export( ANativeWindow *pNativeWindow ) = 0;
    virtual NXT_Error end_export() = 0;
    virtual const char* get_last_parse_error() = 0;
    virtual NXT_Error load_effect( const char* effect_id, const char* effect_data ) = 0;
    virtual NXT_Error unload_effect( const char* effect_id ) = 0;
    virtual IEffect* get_effect( const char* effect_id ) = 0;
    virtual NXT_Error wait_for_render() = 0;
    virtual NXT_Error get_pixels(int &width, int &height, int &size, void* pdata = nullptr) = 0;
    virtual NXT_Error set_video_placeholders(const char *video1,
                                             const char *video2) = 0;
    
    virtual void setTransitionEffect(const std::string& effect_id,
                                     const std::string& effect_options,
                                     unsigned int start_cts,
                                     unsigned int end_cts) = 0;
    
    virtual void set_log_level( int level ) = 0;
    
    virtual ILayer* get_layer( int layer_id, LayerType layer_type ) = 0;
    virtual ILayer* create_layer(int layer_id,
                                 LayerType layer_type,
                                 unsigned int start_cts,
                                 unsigned int end_cts) = 0;
    virtual void delete_layer( int layer_id, LayerType layer_type ) = 0;
    
};

#endif
