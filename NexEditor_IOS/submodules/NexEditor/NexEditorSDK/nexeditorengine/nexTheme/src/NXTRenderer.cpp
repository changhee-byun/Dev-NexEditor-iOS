//
//  NXTRenderer.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#include "NXTRenderer.h"

#define LOG_TAG "NXTRenderer.cpp"
#include "NXTEffect.h"
#include "NXTVertexAttr.h"
#include <algorithm>


template<typename K, typename V>
const V& getValueOrDefault(const std::map<K,V>&  m, const K& k, const V& def ) {
    const auto &e = m.find(k);
    if( e == m.end() ) {
        return def;
    } else {
        return e->second;
    }
}



//void NXTRenderer::setDefaultColorMask() {
//    
//    glColorMask(colorMask[0]?GL_TRUE:GL_FALSE,
//                colorMask[1]?GL_TRUE:GL_FALSE,
//                colorMask[2]?GL_TRUE:GL_FALSE,
//                colorMask[3]?GL_TRUE:GL_FALSE);
//    
//}

NXT_Error NXTRenderer::init(NXTRendererType renderType,
                            const std::map<NXTRenderProp,int>& properties)
{

    if( initialized_ ) {
        return NXT_Error_InvalidState;
    }

    renderer_type_ = renderType;
    
    requestMultisample      = getValueOrDefault(properties,NXTRenderProp::Multisample,0)?true:false;
    requestDepthBufferBits  = getValueOrDefault(properties,NXTRenderProp::DepthBufferBits,16);
    
    NXT_Error err = createContext();
    if( err!=NXT_Error_None ) {
        return err;
    }
    
    initialized_ = true;
    return NXT_Error_None;
}

NXTRenderer::NXTRenderer() {
}

NXTRenderer::~NXTRenderer() {
    for( auto& effect: loaded_effects_ ) {
        delete effect.second;
    }
    loaded_effects_.clear();
}

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
    {NULL}
};

NXT_Error NXTRenderer::createContext() {
    
    NXT_Error result = ensureInitEGL();
    if( result!=NXT_Error_None ) {
        return result;
    }
    
    // EGL : Choose the configuration
    
    EGLint attrib_list_preview[] = {
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_ALPHA_SIZE,             8,
        EGL_STENCIL_SIZE,           8,
        EGL_DEPTH_SIZE,             requestDepthBufferBits,
        EGL_NONE
    };
    
    EGLint attrib_list_export[] = {
        EGL_SURFACE_TYPE,           EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_RECORDABLE_ANDROID,     EGL_TRUE,
        EGL_DEPTH_SIZE,             requestDepthBufferBits,
        EGL_STENCIL_SIZE,           1,
        EGL_NONE
    };
    
    EGLint attrib_list_preview_multisample[] = {
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_ALPHA_SIZE,             8,
        EGL_STENCIL_SIZE,           8,
		EGL_SAMPLE_BUFFERS,			1 /* true */,
		EGL_SAMPLES,				2,
        EGL_DEPTH_SIZE,             requestDepthBufferBits,
        EGL_NONE
    };
    
    EGLint attrib_list_export_multisample[] = {
        EGL_SURFACE_TYPE,           EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,        EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,               8,
        EGL_GREEN_SIZE,             8,
        EGL_BLUE_SIZE,              8,
        EGL_RECORDABLE_ANDROID,     EGL_TRUE,
        EGL_STENCIL_SIZE,           1,
		EGL_SAMPLE_BUFFERS,			1 /* true */,
		EGL_SAMPLES,				2,
        EGL_DEPTH_SIZE,             requestDepthBufferBits,
        EGL_NONE
    };
    
    EGLint *attrib_list_a = NULL;
    EGLint *attrib_list_b = NULL;
    EGLint *attrib_list = NULL;
    switch( renderer_type_ ) {
        case NXTRendererType::Export:
			if( requestMultisample ) {
				attrib_list_a = attrib_list_export_multisample;
				attrib_list_b = attrib_list_export;
			} else {
				attrib_list_a = attrib_list_export;
				attrib_list_b = NULL;
			}
            break;
        case NXTRendererType::Preview:
			if( requestMultisample ) {
				attrib_list_a = attrib_list_preview_multisample;
				attrib_list_b = attrib_list_preview;
			} else {
				attrib_list_a = attrib_list_preview;
				attrib_list_b = NULL;
			}
            break;
        default:
            return NXT_Error_InvalidState;
            break;
    }
    
    EGLint numConfig = 0;
	attrib_list = attrib_list_a;
    if( eglChooseConfig( egl_display_, attrib_list, NULL, 0, &numConfig ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglChooseConfig failed (returned false getting count)", __func__, __LINE__);
        return NXT_Error_GLInitFail;
    }
    if( numConfig < 1 )
    {
		availableMultisample = 0;
		LOGI("[%s %d] Failed to find config choice A; trying choice B", __func__, __LINE__);
		if( attrib_list_b ) {
			attrib_list = attrib_list_b;
			if( eglChooseConfig( egl_display_, attrib_list, NULL, 0, &numConfig ) != EGL_TRUE ) {
				CHECK_EGL_ERROR();
				LOGE("[%s %d] eglChooseConfig failed (returned false getting count)", __func__, __LINE__);
				return NXT_Error_GLInitFail;
			}
		}
    } else {
		availableMultisample = requestMultisample;
	}
	
	if( numConfig < 1 ) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned true but no configs)", __func__, __LINE__);
		return NXT_Error_GLInitFail;
	}
	
	EGLConfig configs[numConfig];
	EGLint actualNumConfig = 0;
	CHECK_EGL_ERROR();
	if( eglChooseConfig( egl_display_, attrib_list, configs, numConfig, &actualNumConfig ) != EGL_TRUE ) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned false getting list)", __func__, __LINE__);
		return NXT_Error_GLNoConfig;
	} else if( actualNumConfig < 1 ) {
		CHECK_EGL_ERROR();
		LOGE("[%s %d] eglChooseConfig failed (returned true but no configs)", __func__, __LINE__);
		return NXT_Error_GLNoConfig;
	} else if( actualNumConfig > numConfig) {
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
				if( eglGetConfigAttrib( egl_display_, configs[i], p_dumpattr->attr, &value ) != EGL_TRUE ) {
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
		
		egl_config_ = configs[useConfig];
		
		EGLint context_attr_list[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		egl_context_ = eglCreateContext( egl_display_, egl_config_, EGL_NO_CONTEXT, context_attr_list);
		CHECK_EGL_ERROR();
		if( egl_context_ == EGL_NO_CONTEXT ) {
			LOGE("[%s %d] eglCreateContext failed", __func__, __LINE__);
			return NXT_Error_GLInitFail;
		}
		
		LOGD("[%s %d] eglCreateContext(0x%x)", __func__, __LINE__, LOGPTR(egl_context_));
		
		return NXT_Error_None;
	}
    
    return NXT_Error_None;
}

NXT_Error NXTRenderer::ensureInitEGL() {
    
    if( egl_display_ != EGL_NO_DISPLAY )
        return NXT_Error_None; // EGL already initialized and we have a display, so there's nothing to do
    
    egl_display_ = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    CHECK_EGL_ERROR();
    
    if( egl_display_ == EGL_NO_DISPLAY ) {
        return NXT_Error_GLNoDisplay;
    }
    
    // EGL : Initialize for this display (and get version number)
    EGLint egl_major = 0;
    EGLint egl_minor = 0;
    if( eglInitialize( egl_display_, &egl_major, &egl_minor ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglInitialize failed", __func__, __LINE__);
        return NXT_Error_GLInitFail;
    }
    CHECK_EGL_ERROR();
    
    LOGI("[%s %d] EGL %d.%d  display(0x%X)", __func__, __LINE__, egl_major, egl_minor, LOGPTR(egl_display_));

    return NXT_Error_None;

}

NXT_Error NXTRenderer::initGraphics() {
    
    if(graphics_initialized_)
        return NXT_Error_None;  // Nothing to do if it's already initialized
    
    shader_instances_ = std::auto_ptr<NXTShaderInstances>(new NXTShaderInstances);
    
    graphics_initialized_ = true;
    
    return NXT_Error_None;
}

void NXTRenderer::set_java_vm( JavaVM* vm ) {
    java_vm_ = vm;
}

NXT_Error NXTRenderer::clear_screen( int iTag ) {
    
    
    if( iTag==1 ) {
        
        int view_width = getViewWidth();
        int view_height = getViewHeight();
        
        NXT_Error result = acquire_context();
        if( result!=NXT_Error_None )
            return result;
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        CHECK_GL_ERROR();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        CHECK_GL_ERROR();
        glViewport(0,0,view_width,view_height);
        CHECK_GL_ERROR();
        glScissor(0,0,view_width,view_height);
        CHECK_GL_ERROR();
        glDisable(GL_SCISSOR_TEST);
        CHECK_GL_ERROR();
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
        CHECK_GL_ERROR();
        
        result = release_context(true);
        
        return result;
        
    } else {
        
        return NXT_Error_BadParam;
        
    }
    
}

void NXTRenderer::register_image_callbacks(NXT_LoadImageCallback loadImageCallback,
                                           NXT_FreeImageCallback freeImageCallback,
                                           void *cbdata)
{
    load_image_callback_ = loadImageCallback;
    free_image_callback_ = freeImageCallback;
    image_callback_private_data_ = cbdata;
}

NXT_Error NXTRenderer::acquire_context() {

    if( context_is_acquired_ && pthread_equal(pthread_self(), context_owner_ ) ) {
        return NXT_Error_WouldDeadlock;
    }
    
    NXTMutexAutoLock lock(contextLock);
    context_owner_ = pthread_self();
    context_is_acquired_ = true;
    
    if( native_surface_ != EGL_NO_SURFACE ) {
        if( eglMakeCurrent(egl_display_,
                           native_surface_,
                           native_surface_,
                           egl_context_ ) != EGL_TRUE )
        {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] eglMakeCurrent failed", __func__, __LINE__);
            context_is_acquired_ = false;
            return NXT_Error_GLContextErr;
        }
        CHECK_EGL_ERROR();
    } else {
        context_is_acquired_ = false;
        return NXT_Error_None;
    }
    
    if( !graphics_initialized_ )
        initGraphics();
    
    CHECK_EGL_ERROR();
    glViewport(0,0,surface_width_,surface_height_);
    CHECK_GL_ERROR();
    
    lock.retain_lock();
    return NXT_Error_None;

}

NXT_Error NXTRenderer::release_context( bool bSwapBuffers ) {
    
    NXT_Error result = NXT_Error_None;
    
    if( !context_is_acquired_ )
        return NXT_Error_None;
    
    if( bSwapBuffers ) {
        result = swap_buffers();
    }
    
    eglMakeCurrent( egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    CHECK_EGL_ERROR();
    
    context_is_acquired_ = false;
    contextLock.unlock();
    
	return result;

}

NXT_Error NXTRenderer::swap_buffers() {

    if( !context_is_acquired_ )
        return NXT_Error_NoContext;

    if( native_surface_ != EGL_NO_SURFACE ) {
        if( eglSwapBuffers( egl_display_, native_surface_) != EGL_TRUE ) {
            CHECK_EGL_ERROR();
            return NXT_Error_OpenGLFailure;
        }
    }

    CHECK_EGL_ERROR();
    return NXT_Error_None;

}

void NXTRenderer::set_cts( unsigned int cts ) {
    cts_ = cts;
}

NXT_Error NXTRenderer::set_preview_surface(ANativeWindow *pNativeWindow,
                                           int reqWidth, int reqHeight )
{

    if( renderer_type_ != NXTRendererType::Preview ) {
        return NXT_Error_WrongRendererType;
    }
    
    int nativeWindowWidth = 0;
    int nativeWindowHeight = 0;
	
	bool size_requested = (reqWidth>0 && reqHeight>0);
    
    if( pNativeWindow && native_window_ == pNativeWindow ) {
        
        nativeWindowWidth = ANativeWindow_getWidth(pNativeWindow);
        nativeWindowHeight = ANativeWindow_getHeight(pNativeWindow);
		
        if( nativeWindowWidth == surface_width_ && nativeWindowHeight == surface_height_ &&
           (!size_requested || (nativeWindowWidth==reqWidth && nativeWindowHeight==reqHeight) ) )
        {
            clear_screen(1);
            return NXT_Error_None;
        }
    }
    
    if( native_window_ ) {
        if( native_surface_ != EGL_NO_SURFACE ) {
            eglDestroySurface(egl_display_,native_surface_);
            CHECK_EGL_ERROR();
            native_surface_ = EGL_NO_SURFACE;
        }
        if( native_window_ != pNativeWindow) {
            ANativeWindow_release(native_window_);
            native_window_ = nullptr;
        }
    }
    
    if( pNativeWindow ) {
        
        if( native_window_ != pNativeWindow ) {
            LOGI("[%s %d] Acquiring native window(0x%X)", __func__, __LINE__, LOGPTR(pNativeWindow));
            ANativeWindow_acquire(pNativeWindow);
            native_window_ = pNativeWindow;
        }
        
        EGLint attr_list[] = {
            EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
            EGL_NONE
        };
        
        nativeWindowWidth = ANativeWindow_getWidth(pNativeWindow);
        nativeWindowHeight = ANativeWindow_getHeight(pNativeWindow);
        if( !size_requested ) {
            reqWidth = nativeWindowWidth;
            reqHeight = nativeWindowHeight;
        }
        
        ANativeWindow_setBuffersGeometry(pNativeWindow, reqWidth, reqHeight, WINDOW_FORMAT_RGBX_8888);
        
        native_surface_ = eglCreateWindowSurface(egl_display_,
                                                 egl_config_,
                                                 (EGLNativeWindowType)pNativeWindow,
                                                 attr_list );
        CHECK_EGL_ERROR();
        if( native_surface_ == EGL_NO_SURFACE ) {
            LOGE("[%s %d] eglCreateWindowSurface failed", __func__, __LINE__);
            ANativeWindow_release(native_window_);
            native_window_ = NULL;
            return NXT_Error_GLSurfaceErr;
        }
		
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
            bQueryOK = ( eglQuerySurface( egl_display_, native_surface_, getattrs[i].attr, &surfaceParamValue ) == EGL_TRUE );
            CHECK_EGL_ERROR();
            
            if( !bQueryOK )
            {
                eglDestroySurface(egl_display_, native_surface_);
                native_surface_ = EGL_NO_SURFACE;
                ANativeWindow_release(native_window_);
                native_window_ = nullptr;
                return NXT_Error_GLSurfaceErr;
            }
            
            *(getattrs[i].val) = surfaceParamValue;
        }
        CHECK_EGL_ERROR();
        
        surface_width_ = surfaceWidth == 1 ? reqWidth : surfaceWidth;
        surface_height_ = surfaceHeight == 1 ? reqHeight : surfaceHeight;
        
    }
    return NXT_Error_None;


}

NXT_Error NXTRenderer::begin_export( ANativeWindow *pNativeWindow ) {
    
    if( renderer_type_ != NXTRendererType::Export ) {
        return NXT_Error_WrongRendererType;
    }
    if( is_exporting_ ) {
        return NXT_Error_InvalidState;
    }
    
    native_surface_ = eglCreateWindowSurface(egl_display_,
                                             egl_config_,
                                             (EGLNativeWindowType)pNativeWindow,
                                             NULL );
    
    if( native_surface_ == EGL_NO_SURFACE ) {
        CHECK_EGL_ERROR();
        return NXT_Error_GLSurfaceErr;
    }

    EGLint surfaceWidth = 0;
    EGLint surfaceHeight = 0;
    
    if( eglQuerySurface( egl_display_, native_surface_, EGL_WIDTH, &surfaceWidth ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglQuerySurface failed", __func__, __LINE__);
        eglDestroySurface(egl_display_, native_surface_);
        native_surface_ = EGL_NO_SURFACE;
        return NXT_Error_GLSurfaceErr;
    }
    
    if( eglQuerySurface( egl_display_, native_surface_, EGL_HEIGHT, &surfaceHeight ) != EGL_TRUE ) {
        CHECK_EGL_ERROR();
        LOGE("[%s %d] eglQuerySurface failed", __func__, __LINE__);
        eglDestroySurface(egl_display_, native_surface_);
        native_surface_ = EGL_NO_SURFACE;
        return NXT_Error_GLInitFail;
    }
    
    surface_width_ = surfaceWidth;
    surface_height_ = surfaceHeight;
    is_exporting_ = true;

    return NXT_Error_None;
}

NXT_Error NXTRenderer::end_export() {
    
    if( renderer_type_ != NXTRendererType::Export ) {
        return NXT_Error_WrongRendererType;
    }
    if( !is_exporting_ ) {
        return NXT_Error_InvalidState;
    }
    
    
    LOGI("[%s %d] RENDERER END EXPORT", __func__, __LINE__);
    eglDestroySurface(egl_display_, native_surface_);
    CHECK_EGL_ERROR();
    
    native_surface_ = EGL_NO_SURFACE;
    is_exporting_ = false;
    
    return NXT_Error_None;
}

//NXT_Error NXTRenderer::load_kedl( const char* kedl_data, int iFlags ) {
//
//    
//
//}

const char* NXTRenderer::get_last_parse_error() {
    return last_parse_error_.c_str();
}


NXT_Error NXTRenderer::load_effect( const char* effect_id, const char* effect_data ) {
    
    IEffect* effect = new NXTEffect();
    
    auto iter = loaded_effects_.find(effect_id);
    if( iter != loaded_effects_.end() ) {
        delete iter->second;
        loaded_effects_.erase(iter);
    }
    
    NXT_Error result = effect->loadFromString(effect_data);
    if( result!=NXT_Error_None ) {
        last_parse_error_ = effect->get_parse_error();
        delete effect;
        return NXT_Error_Syntax;
    }
    
    loaded_effects_[effect_id] = effect;
    
    return NXT_Error_None;
}

NXT_Error NXTRenderer::unload_effect( const char* effect_id ) {
    
    auto iter = loaded_effects_.find(effect_id);
    if( iter != loaded_effects_.end() ) {
        delete iter->second;
        loaded_effects_.erase(iter);
        return NXT_Error_None;
    }
    return NXT_Error_NoEffect;
}

IEffect* NXTRenderer::get_effect( const char* effect_id ) {
    auto iter = loaded_effects_.find(effect_id);
    if( iter != loaded_effects_.end() ) {
        return iter->second;
    }
    return nullptr;
}

void NXTRenderer::seed( int seed1, int seed2 ) {
    NXT_SRandom(&mSeed, seed1, seed2);
}

float NXTRenderer::randf() {
    return NXT_FRandom(&mSeed);
}

bool NXTRenderer::isEffectActive() {
    return activeEffect.effectId.length() > 0;
//    return activeEffect >= 1 && activeEffect <= 3;
}

NXTAppliedEffect& NXTRenderer::getActiveEffect() {
    return activeEffect;
//    switch( activeEffect ) {
//        case 1:
//            return clipEffect;
//        case 2:
//            return transitionEffect;
//        case 3:
//            return watermarkEffect;
//        default:
//            return noneEffect;
//    }
}

void NXTRenderer::setTexture( const NXTImageAndSettings& texture_image ) {
    texture_ = texture_image;
}

void NXTRenderer::setMask( const NXTImageAndSettings& texture_image ) {
    mask_ = texture_image;
}

void NXTRenderer::clearTexture() {
    texture_.source = PixelSource::Image;
    texture_.pImage = nullptr;
    texture_.pSettings = nullptr;
}

void NXTRenderer::clearMask() {
    mask_.source = PixelSource::Image;
    mask_.pImage = nullptr;
    mask_.pSettings = nullptr;
}


void NXTRenderer::prepStateForRender() {
    // TODO: prepStateForRender
    
    NXTMatrix mvp = getProjectionMatrix() * getTransformMatrix(NXTTransformTarget::Vertex);

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();
    
    ITexture* pTexture = nullptr;
    NXTImageSettings *pSettings = texture_.pSettings;
    
    // -- Select the input source (texture node or surface with rendered clip)
    
    switch( texture_.source ) {
        case PixelSource::Video:
            pTexture = clip_img_left_;
            pSettings = clip_img_left_settings_;
            break;
        case PixelSource::VideoNext:
            pTexture = clip_img_right_;
            break;
        case PixelSource::Image:
            pTexture = texture_.pImage;
            break;
        default:
            break;
    }
    
    if( !pTexture ) {
        pTexture = &tex_solid_black_;
    }
    
    // -- Calculate texture and mask transformations
    
    NXTMatrix texmat = getTransformMatrix(NXTTransformTarget::Texture);
    NXTMatrix maskmat = getTransformMatrix(NXTTransformTarget::Mask);
    
// TODO: !!!

    // -- Select the appropriate shader
    
    ITexture::Format inputFormat = pTexture->getTextureFormat();
    
    active_shader_ = nullptr;
    
    if( render_style_.renderDest==NXT_RenderDest_Stencil) {
        active_shader_ = &shader_instances_->rgba_to_stencil;
    } else if( output_type_==NXT_RendererOutputType_RGBA_8888 ) {
        if( inputFormat==ITexture::Format::RGBA ) {
            active_shader_ = &shader_instances_->rgba_to_rgba;
        } else if( inputFormat==ITexture::Format::YUV ) {
            active_shader_ = &shader_instances_->yuv_to_rgba;
        } else if( inputFormat==ITexture::Format::NV12 ) {
            active_shader_ = &shader_instances_->nv12_to_rgba;
        } else if( inputFormat==ITexture::Format::SURFACE_TEXTURE ) {
            active_shader_ = &shader_instances_->ext_to_rgba;
        }
    } else if ( output_type_==NXT_RendererOutputType_YUVA_8888 ) {
        if( inputFormat==ITexture::Format::RGBA ) {
            active_shader_ = &shader_instances_->rgb_to_yuva8888_all;
        } else if( inputFormat==ITexture::Format::YUV ) {
            active_shader_ = &shader_instances_->yuv_to_rgba;
        } else if( inputFormat==ITexture::Format::NV12 ) {
            active_shader_ = &shader_instances_->nv12_to_rgba;
        } else if( inputFormat==ITexture::Format::SURFACE_TEXTURE ) {
            active_shader_ = &shader_instances_->ext_to_rgba;
        }
    } else if ( output_type_==NXT_RendererOutputType_Y2CrA_8888 ) {
        if( inputFormat==ITexture::Format::RGBA ) {
            active_shader_ = &shader_instances_->rgb_to_yuva8888(channel_filter_);
        } else if( inputFormat==ITexture::Format::YUV ) {
        } else if( inputFormat==ITexture::Format::NV12 ) {
            active_shader_ = &shader_instances_->nv12_to_yuva8888(channel_filter_);
        } else if( inputFormat==ITexture::Format::SURFACE_TEXTURE ) {
            active_shader_ = &shader_instances_->ext_to_yuva8888(channel_filter_);
        }
    }
    
    // -- Set shader attributes and uniforms
    
    // TODO
    
}

void NXTRenderer::render(NXT_RendererOutputType output_type, const char* options ) {
    
    NXTMutexAutoLock lock(contextLock);
    
    channel_filter_ = NXTChannelFilter::ALL;
    
    // Find out which visual clips are active by checking our current time stamp
    // against registered VisualClip layer start and end times.  Normally, there
    // will be just one visual clip active, but there could be two active during
    // a transition.
    
    NXTLayer* left_clip = nullptr;
    NXTLayer* right_clip = nullptr;
    
    for( NXTLayer* layer: layers_ ) {
        if( layer->layer_type_ == LayerType::VisualClip ) {
            if( cts_ >= layer->start_cts_ && cts_ <= layer->end_cts_ ) {
                if( !left_clip ) {
                    left_clip = layer;
                } else if( !right_clip ) {
                    right_clip = layer;
                } else {
                    LOGW("[%s %d] too many VisualClip layers at cts=%d", __func__, __LINE__, cts_ );
                }
            }
        }
    }
    
    if( !left_clip ) {
        LOGW("[%s %d] no VisualClip layers at cts=%d", __func__, __LINE__, cts_ );
    } else if( left_clip && right_clip ) {
        if( right_clip->start_cts_ < left_clip->start_cts_ ) {
            std::swap(left_clip, right_clip);
        }
    }

    // Process render options
    render_options_.parse(options);

    // Render the left and right clip layers
    // with their respective clip effects, if any
    if( left_clip ) {
        output_type_ = NXT_RendererOutputType_RGBA_8888;
        left_video_surf_.bind();
        render_layer(left_clip);
        left_video_surf_.unbind();
    }
    
    if( right_clip ) {
        output_type_ = NXT_RendererOutputType_RGBA_8888;
        right_video_surf_.bind();
        render_layer(right_clip);
        right_video_surf_.unbind();
    }
    
    // Check if there's a transition in effect here
    
    IEffect* transition = nullptr;
    
    if( cts_ >= transition_start_cts_ && cts_ <= transition_end_cts_ && transition_effect_id_!="" ) {
        const auto &e = loaded_effects_.find(transition_effect_id_);
        if( e != loaded_effects_.end() ) {
            transition = e->second;
        }
    }
    
    //--------> REPEAT FOR EACH RENDERING PASS ------
    
    // Render the transition effect (if there is one),
    // or the left clip (if there is no transition)
    
    render_with_output_format(output_type, [this,left_clip,right_clip,transition](){
        
        clip_img_left_ = left_clip?&left_video_surf_:nullptr;
        clip_img_right_ = right_clip?&right_video_surf_:nullptr;
        clip_img_left_settings_ = nullptr;
        clip_img_right_settings_ = nullptr;
        set_default_state();
        if( transition ) {
            transition->render(*this);
        } else {
            setTexture(NXTImageAndSettings(PixelSource::Video));
            clearMask();
            prepStateForRender();
            fillRect(-1, -1, 1, 1);
        }
        clip_img_left_ = nullptr;
        clip_img_right_ = nullptr;
        
        // Render overlay and watermark layers
        
        std::sort(layers_.begin(), layers_.end(), [](NXTLayer* lhs, NXTLayer* rhs){
            return  (lhs->layer_type_ <  rhs->layer_type_) ||
            (lhs->layer_type_ == rhs->layer_type_ && lhs->z_order <  rhs->z_order) ||
            (lhs->layer_type_ == rhs->layer_type_ && lhs->z_order == rhs->z_order && lhs->start_cts_ <  rhs->start_cts_ ) ||
            (lhs->layer_type_ == rhs->layer_type_ && lhs->z_order == rhs->z_order && lhs->start_cts_ == rhs->start_cts_ && lhs->layer_id_ < rhs->layer_id_ );
        });
        
        for( NXTLayer* layer: layers_ ) {
            if( layer->layer_type_ == LayerType::Overlay || layer->layer_type_ == LayerType::Watermark ) {
                if( cts_ >= layer->start_cts_ && cts_ <= layer->end_cts_ ) {
                    render_layer(layer);
                }
            }
        }
        
    });
}

NXT_Error NXTRenderer::render_with_output_format_impl( NXT_RendererOutputType output_type, RenderCB* cb ) {
    switch( output_type ) {
        case NXT_RendererOutputType_RGBA_8888:
        {
            int trim =  surface_height_ == 1088 ? 8 :
                        surface_height_ == 736 ? 16 :
                        surface_height_ == 544 ? 4:
                        surface_height_ == 368 ? 8:
                        0;
            
            glViewport(0,trim,surface_width_, surface_height_);
            glScissor(0,trim,surface_width_, surface_height_);
            
            colorMask[0] = GL_TRUE;
            colorMask[1] = GL_TRUE;
            colorMask[2] = GL_TRUE;
            colorMask[3] = GL_TRUE;
            
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClearColor(0.0,0.0,0.0,1.0);
            CHECK_GL_ERROR();
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            CHECK_GL_ERROR();
            
            output_type_ = output_type;
            channel_filter_ = NXTChannelFilter::ALL;
            cb->cb();
            
            break;
        }
        case NXT_RendererOutputType_Y2CrA_8888:
        {

            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Prep frame render");
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClearColor(0.0,0.0,0.5,1.0);
            CHECK_GL_ERROR();
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
            CHECK_GL_ERROR();
            glEnable(GL_SCISSOR_TEST);
            NXT_END_PROFILE();
            
            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 1");
            glScissor(0,surface_height_/2,surface_width_,surface_height_/2);
            glViewport(0,surface_height_/2,surface_width_,surface_height_);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            colorMask[0] = GL_TRUE;
            colorMask[1] = GL_FALSE;
            colorMask[2] = GL_FALSE;
            colorMask[3] = GL_FALSE;
            output_type_ = NXT_RendererOutputType_YUVA_8888;
            channel_filter_ = NXTChannelFilter::Y;
            cb->cb();
            NXT_END_PROFILE();
            
            NXT_BEGIN_PROFILE("Render Y2CrA8888 : Y Pass 2");
            glScissor(0,surface_height_/2,surface_width_,surface_height_/2);
            glViewport(0,0,surface_width_,surface_height_);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            colorMask[0] = GL_FALSE;
            colorMask[1] = GL_TRUE;
            colorMask[2] = GL_FALSE;
            colorMask[3] = GL_FALSE;
            output_type_ = NXT_RendererOutputType_YUVA_8888;
            channel_filter_ = NXTChannelFilter::Y;
            cb->cb();
            NXT_END_PROFILE();
            
            NXT_BEGIN_PROFILE("Render Y2CrA8888 : U Pass");
            glScissor(0,surface_height_/2,surface_width_/2,surface_height_/2);
            glViewport(0,surface_height_/2,surface_width_/2,surface_height_/2);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            colorMask[0] = GL_FALSE;
            colorMask[1] = GL_FALSE;
            colorMask[2] = GL_TRUE;
            colorMask[3] = GL_FALSE;
            output_type_ = NXT_RendererOutputType_YUVA_8888;
            channel_filter_ = NXTChannelFilter::U;
            cb->cb();
            NXT_END_PROFILE();
            
            NXT_BEGIN_PROFILE("Render Y2CrA8888 : V Pass");
            glScissor(0+(surface_width_/2),surface_height_/2,surface_width_/2,surface_height_/2);
            glViewport(0+(surface_width_/2),surface_height_/2,surface_width_/2,surface_height_/2);
            glClear( GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            colorMask[0] = GL_FALSE;
            colorMask[1] = GL_FALSE;
            colorMask[2] = GL_TRUE;
            colorMask[3] = GL_FALSE;
            output_type_ = NXT_RendererOutputType_YUVA_8888;
            channel_filter_ = NXTChannelFilter::V;
            cb->cb();
            NXT_END_PROFILE();
            
            colorMask[0] = GL_TRUE;
            colorMask[1] = GL_TRUE;
            colorMask[2] = GL_TRUE;
            colorMask[3] = GL_TRUE;
            channel_filter_ = NXTChannelFilter::ALL;
            output_type_ = NXT_RendererOutputType_Y2CrA_8888;
            glViewport(0,0,surface_width_,surface_height_);
            glScissor(0,0,surface_width_,surface_height_);
            glDisable(GL_SCISSOR_TEST);
            
            break;
        }
        case NXT_RendererOutputType_YUVA_8888:
        {
            
            glViewport(0,0,surface_width_,surface_height_);
            glScissor(0,0,surface_width_,surface_height_);
            colorMask[0] = GL_TRUE;
            colorMask[1] = GL_TRUE;
            colorMask[2] = GL_TRUE;
            colorMask[3] = GL_TRUE;
            
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClearColor(0.0,0.5,0.5,1.0);
            CHECK_GL_ERROR();
            glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            CHECK_GL_ERROR();
            
            output_type_ = output_type;
            channel_filter_ = NXTChannelFilter::ALL;
            cb->cb();
            
            break;
        }
        default:
            break;
    }
}


void NXTRenderer::set_default_state() {
    
    getTransformMatrix(NXTTransformTarget::Vertex).setIdentity();
    getTransformMatrix(NXTTransformTarget::Texture).setIdentity();
    getTransformMatrix(NXTTransformTarget::Mask).setIdentity();
    getTransformMatrix(NXTTransformTarget::MaskSample).setIdentity();
    getTransformMatrix(NXTTransformTarget::Color).setIdentity();
    getProjectionMatrix().setIdentity();
    resetColorMask();
    
    glDisable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
    glDepthMask(GL_TRUE);
    CHECK_GL_ERROR();
    glDisable(GL_CULL_FACE);
    CHECK_GL_ERROR();
    
}

NXT_Error NXTRenderer::render_layer( NXTLayer* layer ) {


    clip_img_left_ = &layer->getImage();
    clip_img_left_settings_ = &layer->getImageSettings();
    clip_img_right_ = nullptr;
    clip_img_right_settings_ = nullptr;
    set_default_state();
    
    IEffect* pEffect = get_effect(layer->clip_effect_id_.c_str());
    if( pEffect ) {
        pEffect->render(*this);
    } else {
        setTexture(NXTImageAndSettings(PixelSource::Video));
        clearMask();
        prepStateForRender();
        fillRect(-1, -1, 1, 1);
    }

    clip_img_left_ = nullptr;
    clip_img_right_ = nullptr;
    clip_img_left_settings_ = nullptr;

}

void NXTRenderer::fillRect( float left, float bottom, float right, float top) {
    
    GLfloat vertexCoord[] = {
        left,   bottom,
        right,  bottom,
        left,   top,
        right,  top
    };
    
    GLfloat texCoords[] = {
        0,      0,
        1,      0,
        0,      1,
        1,      1
    };
    
    GLfloat maskCoords[] = {
        0,      0,
        1,      0,
        0,      1,
        1,      1
    };

    NXTVector color = getColor();
    GLfloat vertexColor[16];
    for( int i=0; i<16; i++ ) {
        vertexColor[i] = color[i%4];
    }

    getShader().vertexLocations().set(vertexCoord,2);
    getShader().vertexTexCoords().set(texCoords,2);
    getShader().vertexMaskCoords().set(maskCoords,2);
    getShader().vertexColors().set(vertexColor,2);
    drawArrays(NXTDrawArraysMode::TriangleStrip);
}

void NXTRenderer::setShader(NXTShader &shader) {
    current_shader_ = &shader;
}

//NXT_Error NXTRenderer::render_internal() {
//    
//}

ILayer* NXTRenderer::get_layer( int layer_id, LayerType layer_type ) {
    for( NXTLayer* layer: layers_ ) {
        if( layer && layer->layer_id_ == layer_id && layer->layer_type_ == layer_type) {
            return layer;
        }
    }
    return nullptr;
}

ILayer* NXTRenderer::create_layer(int layer_id,
                                  LayerType layer_type,
                                  unsigned int start_cts,
                                  unsigned int end_cts)
{
    delete_layer(layer_id,layer_type); // Make sure an existing layer with this ID is removed
    NXTLayer* pLayer = new NXTLayer(layer_id, layer_type, start_cts, end_cts);
    layers_.push_back(pLayer);
    return pLayer;
}

void NXTRenderer::delete_layer( int layer_id, LayerType layer_type ) {
    for( auto it = layers_.begin(); it != layers_.end(); ++it ) {
        if( *it && (*it)->layer_id_ == layer_id && (*it)->layer_type_ == layer_type) {
            delete *it;
            layers_.erase(it);
            return;
        }
    }
}

void NXTRenderer::setTransitionEffect(const std::string& effect_id,
                                 const std::string& effect_options,
                                 unsigned int start_cts,
                                 unsigned int end_cts)
{
    transition_effect_id_ = effect_id;
    transition_start_cts_ = start_cts;
    transition_end_cts_ = end_cts;
    transition_effect_options_.parse(effect_options.c_str());
}



// TODO: getActiveShader
//NXTShader& NXTRenderer::getActiveShader() {}


// !!!: _________________________________________________________________________________________
// TODO: Implement all methods below
NXT_Error NXTRenderer::wait_for_render() {}
NXT_Error NXTRenderer::get_pixels(int &width, int &height, int &size, void* pdata ) {}
NXT_Error NXTRenderer::set_video_placeholders(const char *video1,
                                 const char *video2) {}
void NXTRenderer::set_log_level( int level ) {}


bool NXTRenderer::isMaskActive() {}
bool NXTRenderer::isTextureActive() {}

NXTCullFace NXTRenderer::getCullFace() {}
void NXTRenderer::setCullFace( NXTCullFace cullFace ) {}
NXTVector NXTRenderer::getColor() {}
void NXTRenderer::setColor(const NXTVector& color) {}
NXTShader& NXTRenderer::getShader() {}
void NXTRenderer::drawArrays(NXTDrawArraysMode mode, int first, int count) {}

NXTMatrix& NXTRenderer::getTransformMatrix(NXTTransformTarget target) {}
void NXTRenderer::skipChildren(bool skip) {}
bool NXTRenderer::isSkipChildren() {}

NXTPart NXTRenderer::getCurrentPart() {}
int NXTRenderer::getTotalClipCount() {}
int NXTRenderer::getCurrentClipIndex() {}
int NXTRenderer::getEffectStartTime() {}
int NXTRenderer::getEffectEndTime() {}
int NXTRenderer::getTotalProjectTime() {}
float NXTRenderer::getEffectProgressOverall() {}
float NXTRenderer::getEffectProgressInPart() {}


NXTMatrix& NXTRenderer::getProjectionMatrix() {}
int NXTRenderer::getViewWidth() {}
int NXTRenderer::getViewHeight() {}
NXTRenderStyle& NXTRenderer::getRenderStyle() {}
void NXTRenderer::resetColorMask() {
    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
}
int NXTRenderer::getCurrentTime() {}
NXTShadingMode NXTRenderer::getShadingMode() {}

void check_gl_error( int line, const char* func ) {}
