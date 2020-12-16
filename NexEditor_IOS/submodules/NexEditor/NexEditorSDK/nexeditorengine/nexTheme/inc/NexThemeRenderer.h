/******************************************************************************
* File Name   :	NexThemeRenderer.h
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

#ifndef NEXTHEMERENDERER_H
#define NEXTHEMERENDERER_H

#include "NexTheme_Errors.h"

#ifdef ANDROID
#include <android/native_window.h>
#include <jni.h>
#else
struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;
#endif

#define LOAD_THEME_PREIVEW 0x00000001

        
    typedef void* NXT_HSurfaceTextureClient;


    // --- Opaque Handles ----------------------------------------------------------
    
    struct NXT_ThemeRenderer_;
    struct NXT_AnimationLayer_;
    struct NXT_ThemeRenderer_Context;
    typedef struct NXT_ThemeRenderer_ NXT_ThemeRenderer;
    typedef struct NXT_ThemeRenderer_ *NXT_HThemeRenderer;
    typedef struct NXT_ThemeRenderer_Context *NXT_HThemeRenderer_Context;
    typedef struct NXT_TextureInfo_ NXT_TextureInfo;
    typedef unsigned int NXT_AnimationLayerId;
    
    // --- Callback Functions ------------------------------------------------------
    
    typedef struct NXT_ImageInfo_ NXT_ImageInfo;
    
    /**
     * Called by the renderer when a theme image needs to be loaded.
     *
     * The callback should load the image and fill in the information about the
     * image in the pinfo structure.
     *
     */
    typedef int (*NXT_LoadFileCallback)(char**, int*, char*, void* cbdata);
    typedef NXT_Error (*NXT_LoadImageCallback)( NXT_ImageInfo* pinfo, char* path, int asyncmode, /*NXT_PixelFormat requestedColorFormat, */ void* cbdata );
    
    typedef NXT_Error (*NXT_FreeImageCallback)( NXT_ImageInfo* pinfo, void* cbdata );
    
    typedef NXT_Error (*NXT_CustomRenderCallback)( int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int , int );
    

    // --- Renderer properties -----------------------------------------------------
    typedef enum NXT_RenderProp_ {
        NXT_RenderProp_Multisample		= 1,		// boolean
        NXT_RenderProp_DepthBufferBits	= 2			// boolean
    } NXT_RenderProp;
	
    // --- Types -------------------------------------------------------------------
    
    typedef enum NXT_PixelFormat_ {
        NXT_PixelFormat_NONE        = 0,
        NXT_PixelFormat_RGBA8888    = 1,
        NXT_PixelFormat_RGB565      = 2,
        NXT_PixelFormat_Luminance   = 3,
        NXT_PixelFormat_YUV         = 4,
        NXT_PixelFormat_NV12        = 5,
        NXT_PixelFormat_EGLImage    = 6,
        NXT_PixelFormat_NV12_JPEG   = 7,
        NXT_PixelFormat_Opaque      = 8,
    } NXT_PixelFormat;
    
    typedef enum {
        NXT_RendererOutputType_JUST_CLEAR    = 0,
        NXT_RendererOutputType_RGBA_8888     = 1,
        NXT_RendererOutputType_YUVA_8888     = 2,
        NXT_RendererOutputType_Y2CrA_8888    = 3,
        NXT_RendererOutputType_RGBA_8888_yflip = 4,
    } NXT_RendererOutputType;
    
    typedef enum {
        NXT_RendererType_None = 0,
        NXT_RendererType_ExternalContext        = 1,            // Use if the context is managed externally (for example, by a GLSurfaceView in Java)
        NXT_RendererType_InternalPreviewContext = 2,            // Use if the context is managed internally (for example, when using a native window or native surface)
        NXT_RendererType_InternalExportContext  = 3             // Use if the context is managed internally (for example, when using a native window or native surface)
    } NXT_RendererType;
    
    typedef enum NXT_TextureID_ {
        NXT_TextureID_Video_1       = 0,
        NXT_TextureID_Video_2,
		NXT_TextureID_Video_Layer_START,
		NXT_TextureID_Video_Layer_0 = NXT_TextureID_Video_Layer_START,
		NXT_TextureID_Video_Layer_1,
		NXT_TextureID_Video_Layer_2,
		NXT_TextureID_Video_Layer_3,
		NXT_TextureID_Video_Layer_4,
		NXT_TextureID_Video_Layer_5,
        NXT_TextureID_COUNT = 32
    } NXT_TextureID;

	typedef enum NXT_TEXTURE_USE_FLAG_{
		STATE_UNUSED = 0,
        STATE_CHECK,
        STATE_CHECK_INUSE,
        STATE_CHECK_NOUSE,
		STATE_INUSE
	}NXT_TEXTURE_USE_FLAG;
    
    struct NXT_ImageInfo_ {
        int                     width;                  // Width of image (in pixels)
        int                     height;                 // Height of image (in pixels)
        int                     pitch;                  // Pitch of image (in pixels)
        NXT_PixelFormat         pixelFormat;            // One of NXT_PixelFormat_????
        int                     *pixels;                // Pointer to pixels
        NXT_FreeImageCallback   freeImageCallback;      // Function to call to free pixel data
        void                    *cbprivate0;            // Private data for use by freeImageCallback
        void                    *cbprivate1;            // Private data for use by freeImageCallback
        void                    *userdata;
    };

    struct NXT_PrecacheResourceInfo{

        int type;
        char* name;
        char* uid;
    };
	
	typedef enum NXT_PrecacheMode_ {
		NXT_PrecacheMode_NoUpload       = 1,
		NXT_PrecacheMode_Upload         = 2
	} NXT_PrecacheMode;
	
#define DEFAULT_MAX_TEXTURE_CACHE_SIZE (1024*1024*16)    
#define TR_PROPERTY_NAME_IOS_LAYER 0x00000001
#ifdef __cplusplus
extern "C" {
#endif
    // --- Theme Functions ---------------------------------------------------------
    NXT_Error NXT_ThemeRenderer_GetProperty(NXT_HThemeRenderer hRenderer,int nPropertyName,void**ppRetValue);
    
    /**
     * Creates a renderer instance.
     *
     * A renderer can render regular video, and also video with effects from a
     * theme.
     *
     * At this time, there can be only one renderer instance at a time.
     *
     * Be sure to destroy the renderer when you are done with it by calling
     * ::NXT_ThemeRenderer_Destroy.
     *
     * \param rendererType    One of:
     *                          NXT_RendererType_ExternalContext
     *                          NXT_RendererType_InternalPreviewContext
     *                          NXT_RendererType_InternalExportContext
     */
    NXT_HThemeRenderer NXT_ThemeRenderer_Create(NXT_RendererType rendererType, int* properties, NXT_HThemeRenderer_Context ctx, NXT_HThemeRenderer anotherRendererForShareGroup);
    NXT_HThemeRenderer_Context NXT_ThemeRendererContext_Create(int max_texture_size, unsigned int bIsInternalContext, unsigned int bIsMI5C);
    void NXT_ThemeRendererContext_Destroy(NXT_HThemeRenderer_Context ctx);


#ifdef ANDROID
    NXT_Error NXT_ThemeRenderer_SetJavaVM( NXT_HThemeRenderer renderer, JavaVM * java_vm );

    NXT_Error NXT_ThemeRenderer_SetWrapperForSurfaceTextureListener( NXT_HThemeRenderer renderer, jclass wstl );
#endif
    
    /**
     * Sets a native window to use for rendering.
     *
     * If an existing native window is set, it will be automatically released.
     *
     * Specify NULL to release any native window without setting a new one.
	 *
	 * If the width and height are both >0, the specified width and height will be
	 * used for the rendering surface.  If either width or height is <=0, the native
	 * window's existing size will be used.  (Width and height are ignored when 
	 * pNativeWindow is NULL)
     *
     * This function will fail if the renderer type is not NXT_RendererType_InternalPreviewContext.
     */
    NXT_Error NXT_ThemeRenderer_SetNativeWindow( NXT_HThemeRenderer renderer, ANativeWindow *pNativeWindow, int width, int height );
    
    /**
     * Specifies a NexGraphicsBuffer (returned by nexEglImage_Create) wrapping
     * a native surface to use for rendering output.
     *
     * Set to NULL to clear the current native surface.
     *
     * As long as a native surface is set, all rendering output goes to the
     * native surface rather than to the native window.
     *
     * This function will fail if the renderer type is not NXT_RendererType_InternalPreviewContext.
     */
    NXT_Error NXT_ThemeRenderer_SetNativeSurface( NXT_HThemeRenderer renderer, void* nexGraphicsBuffer );
    
    NXT_Error NXT_ThemeRenderer_BeginExport( NXT_HThemeRenderer renderer, int width, int height, ANativeWindow *pNativeWindow );

    NXT_Error NXT_ThemeRenderer_EndExport( NXT_HThemeRenderer renderer );

    void NXT_ThemeRenderer_SetExportMode(NXT_HThemeRenderer renderer);

    
    NXT_Error NXT_ThemeRenderer_ClearScreen(NXT_HThemeRenderer renderer, int iTag);

    /*
    NXT_AnimationLayerId NXT_ThemeRenderer_NewAnimationLayer(NXT_HThemeRenderer renderer,
                                                            void* data,
                                                            unsigned int data_length,
                                                            unsigned int start_trim_time,
                                                            unsigned int end_trim_time,
                                                            unsigned int duration);
    
    void NXT_ThemeRenderer_DeleteAnimationLayer( NXT_HThemeRenderer, NXT_AnimationLayerId layer_id );
    
    void NXT_ThemeRenderer_SetAnimationLayerTime( NXT_HThemeRenderer, NXT_AnimationLayerId layer_id, unsigned int time );
     */
    
    /**
     * Registers callbacks to be used for loading image resources referenced
     * by themes.
     *
     * Themes can use external images such as PNG or JPEG images.  The theme engine
     * doesn't incldue a loader for these.  Instead, a callback should be provided
     * which can load the image via an appropriate operating system API.
     *
     * \param loadImageCallback A function to call to load an image and get pixel data.
     * \param freeImageCallback A function to call to free the pixel data allocated by \c loadImageCallback.
     * \param cbdata            Extra user data to pass to the callbacks
     */
    void NXT_ThemeRenderer_RegisterImageCallbacks(NXT_HThemeRenderer renderer,
                                                  NXT_LoadImageCallback loadImageCallback,
                                                  NXT_FreeImageCallback freeImageCallback,
                                                  NXT_LoadFileCallback LoadFileCallback,
                                                  void *cbdata);
    
    void NXT_ThemeRenderer_RegisterLoadfileCallback(NXT_HThemeRenderer renderer, NXT_LoadFileCallback loadFileCallback);

    /**
     * Registers a custom rendering callback to call after other rendering is completed
     * but before rendering the watermark effect.  To unregister, call again with NULL.
     */
    void NXT_ThemeRenderer_RegisterCustomRenderCallback(NXT_HThemeRenderer renderer,
                                                        NXT_CustomRenderCallback customRenderCallback);
    
    /**
     * Enables or disables use of the custom render callback (if no callback is registered, this has
     * no effect).
     */
    void NXT_ThemeRenderer_SetEnableCustomRendering(NXT_HThemeRenderer renderer, unsigned int bEnable);
    
    /**
     * Precaches all images required for the specified effect.  This must be called
     * with an aquired context.
     */
    int NXT_ThemeRenderer_PrecacheEffect(NXT_HThemeRenderer renderer, const char *effect_id, int* asyncmode, int* max_replaceable);

    int NXT_ThemeRenderer_GetPrecacheEffectResource(NXT_HThemeRenderer renderer, const char *effect_id, void* load_resource_list);

    /**
     * Precaches all images for all themes in a set.  If the images are 
     * already cached, has no effect.  This also precaches any shared images
     * at the theme level, if they have not been already precached.
     */
//    void NXT_ThemeRenderer_PrecacheImages(NXT_HThemeRenderer renderer, 
//                                          NXT_HTheme hTheme,    // Any theme in the set (doesn't matter which one)
//                                          NXT_PrecacheMode precacheMode);
    
    /**
     * Sets the current effect to use when rendering video.
     *
     * The effect may be set to NULL, in which case only the primary video
     * is rendered without any special effect.
     *
     * The clipIndex and totalClipCount parameters are used for positional
     * variation in the effect behavior.
     *
     * clipIndex is the index of the clip (from zero) that this effect is
     * attached to.
     *
     *      first clip :   clipIndex==0
     *      last clip  :   clipIndex==(totalClipCount-1)
     *
     * totalClipCount is the total number of clips in the project.  If this is
     * zero, positional clip effects are disabled and clipIndex is ignored.
     */
	// DEPRECATED
//    void NXT_ThemeRenderer_SetEffect(NXT_HThemeRenderer renderer, const char *effectId, int clipIndex, int totalClipCount);
    
    NXT_Error NXT_ThemeRenderer_LoadThemesAndEffects(NXT_HThemeRenderer renderer, const char* themeAndEffectData, int iFlags );
	NXT_Error NXT_ThemeRenderer_ClearThemesAndEffects(NXT_HThemeRenderer renderer, int iFlags );
    NXT_Error NXT_ThemeRenderer_LoadRenderItem(NXT_HThemeRenderer renderer, const char* pID, const char* renderitemData, int iFlags);

    
    void NXT_ThemeRenderer_SetCTS(NXT_HThemeRenderer renderer, int cts);
    void NXT_ThemeRenderer_SetWatermarkEffect(NXT_HThemeRenderer renderer,
                                              const char *effectId,  // ID of clip effect, or NULL to clear current effect
                                              int projectDuration);
    void NXT_ThemeRenderer_SetTransitionEffect(NXT_HThemeRenderer renderer,
                                               const char *effectId,  // ID of transition effect, or NULL to clear current effect
                                               const char *pEffectOptions,
                                               int clipIndex,         // Index of the transition's clip (the clip before the transition), from 0
                                               int totalClipCount,    // Total number of clips in the project
                                               int effectStartTime,   // Start time for this effect
                                               int effectEndTime );   // End time for this effect
    void NXT_ThemeRenderer_SetClipEffect(NXT_HThemeRenderer renderer,
                                         const char *effectId,  // ID of clip effect, or NULL to clear current effect
                                         const char *pEffectOptions,
                                         int clipIndex,         // Index of this clip, from 0
                                         int totalClipCount,    // Total number of clips in the project
                                         int clipStartTime,     // Start time for this clip
                                         int clipEndTime,       // End time for this clip
                                         int effectStartTime,   // Start time for this effect (must be >= clipStartTime)
                                         int effectEndTime );   // End time for this effect (must be <= clipEndTime)
    void NXT_ThemeRenderer_ClearWatermarkEffect(NXT_HThemeRenderer renderer);
    void NXT_ThemeRenderer_ClearTransitionEffect(NXT_HThemeRenderer renderer);
    void NXT_ThemeRenderer_ClearClipEffect(NXT_HThemeRenderer renderer);

    bool NXT_ThemeRenderer_SetEffectOptions(NXT_HThemeRenderer renderer, unsigned int uid, const char* effectOptions);

    unsigned int NXT_ThemeRenderer_StartTitleEDL(NXT_HThemeRenderer renderer,
                                    const char* effectId,
                                    const char* pEffectOptions,
                                    int startTrackId,
                                    int effectIndex,
                                    int totalEffectCount,
                                    int effectStartTime,
                                    int effectEndTime);

    unsigned int NXT_ThemeRenderer_StartTransitionEDL(NXT_HThemeRenderer renderer,
                                            const char* effectId,
                                            const char* pEffectOptions,
                                            int effectIndex,
                                            int totalEffectCount,
                                            int effectStartTime,
                                            int effectEndTime);

    void NXT_ThemeRenderer_EndEDL(NXT_HThemeRenderer renderer, unsigned int effect_uid);

	
//    void NXT_ThemeRenderer_SetTextureEffect(NXT_HThemeRenderer renderer,
//											NXT_TextureID textureId,
//											const char *effectId,
//											int clipIndex,
//											int totalClipCount,
//											int clipStartTime,
//											int clipEndTime,
//											int currentTime);
//	
//    void NXT_ThemeRenderer_SetTransition(NXT_HThemeRenderer renderer,
//											const char *effectId,
//											int clipIndex,
//											int totalClipCount,
//											float time);
	
    /**
     * Sets the text to use when rendering title effects.
     *
     * This remains set until the function is called again.  This setting has
     * no effect on effects that don't use text.
     *
     * @deprecated  DO NOT USE; SetEffectOptions should be used instead
     */
//    void NXT_ThemeRenderer_SetTitleText( NXT_HThemeRenderer renderer, const char *titleText );
    
    /**
     * Sets the options to use when rendering effects.  This includes any title
     * text.
     * 
     * This remains set until the function is called again.  This setting effects
     * both title and transition effects (the options string is divided into
     * title and transition portions)
     */
//    void NXT_ThemeRenderer_SetEffectOptions( NXT_HThemeRenderer renderer, const char *pEffectOptions );
    
    /**
     * Sets the effect time for the current effect.
     *
     * If no effect is active, this does nothing.
     *
     * \param time      A value between 0.0 (beginning of the effect) and 1.0
     *                  (the end of the effect)
     */
	// DEPRECATED
//    void NXT_ThemeRenderer_SetTime(NXT_HThemeRenderer renderer, float time);
    
    /**
     * Sets the effect time for the current effect in milliseconds.
     *
     * If no effect is active, this does nothing.
     *
     * \param currenttime     The current effect time, from the beginning of the
     *                        effect (in ms).
     *
     * \param duration        The effect duration in milliseconds.
     */
	// DEPRECATED
//    void NXT_ThemeRenderer_SetTimeMS(NXT_HThemeRenderer renderer, int currenttime, int duration);
    
    NXT_Error NXT_ThemeRenderer_AquireContext(NXT_HThemeRenderer renderer);
    NXT_Error NXT_ThemeRenderer_SwapBuffers(NXT_HThemeRenderer renderer);
    NXT_Error NXT_ThemeRenderer_ReleaseContext(NXT_HThemeRenderer renderer, unsigned int bSwapBuffers );

    int NXT_ThemeRenderer_CheckUploadOk(NXT_HThemeRenderer renderer);
    /**
     * Performs the actual OpenGL ES 2.0 rendering for the current effect frame.
     *
     * This is normally called from within a platform-dependent OpenGL rendering
     * callback.  When this is executed, a valid OpenGL context must be active.
     * This must be the same OpenGL context that was in use when the renderer was
     * initialized.
     *
     * \param renderer  The renderer handle
     *
     * \param outputType The desired output format; one of:
     *
     *                      NXT_RendererOutputType_RGBA_8888
     *                          
     *                          Regular RGBA output (Alpha channel will be 255 for all
     *                          pixels).  This is the same format that rendering is
     *                          performed in for display.
     *                          
     *                      NXT_RendererOutputType_YUVA_8888
     *
     *                          Full-size Y, U and V channels (Alpha channel will be
     *                          255 for all pixels).  This is an interleaved format
     *                          just like RGBA, not a planar format.
     *
     *                      NXT_RendererOutputType_Y2CrA_8888
     *
     *                          Interleaved format; full width, half height.  32 bits
     *                          per pixel, as follows:
     *
     *                          Byte 1:    Y1 (Y top half)
     *                          Byte 2:    Y2 (Y bottom half)
     *                          Byte 3:    Chroma (U on left half, V on right half)
     *                          Byte 4:    Alpha (currently unused and should be ignored)
     *
     * \param bExport The render type
     *
     *                      0
     *                          Preview Render
     *
     *                      1
     *                          Export Render
     *
     * \return          NXT_Error_None if successful; otherwise an error code.
     */
    NXT_Error NXT_ThemeRenderer_GLDraw(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark);
    
    NXT_Error NXT_ThemeRenderer_fastOptionPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, const char *pOptions);
    
    NXT_Error NXT_ThemeRenderer_Clear(NXT_HThemeRenderer renderer);
    NXT_Error NXT_ThemeRenderer_Freeze(NXT_HThemeRenderer renderer, unsigned int bFreeze);
    
    /**
     * Waits for the last call to NXT_ThemeRenderer_GLDraw() to complete.  If there
     * is not rendering in progress, returns immediately.
     *
     * It is not necessary to call this before calling NXT_ThemeRenderer_GetPixels()
     * but you can do so if you wish to profile rendering time.
     *
     * Calling this in other cases (when you don't need to get the rendered pixels)
     * may adversely impact performance.
     *
     * \param renderer  The renderer handle
     * \return          Result of the most recent render pass.
     *
     */
    NXT_Error NXT_ThemeRenderer_GLWaitToFinishRendering(NXT_HThemeRenderer renderer);
    
    /**
     * Waits for the last call to NXT_ThemeRenderer_GLDraw() to complete and returns
     * a pointer to the rendered pixels.
     *
     * \param renderer  The renderer handle
     *
     * \param pWidth    Pointer to a variable to receive the width of the 
     *                  rendered frame (in pixels)
     *
     * \param pHeight   Pointer to a variable to receive the height of the 
     *                  rendered frame (in pixels)
     *
     * \param pSize     Pointer to a variable to receive the size of the 
     *                  rendered frame (in bytes)
     *
     * \param ppData    Pointer to a variable to receive a pointer to the actual
     *                  pixel data.  This pointer is valid until the next call
     *                  to NXT_ThemeRenderer_GLRenderAndGetPixels or 
     *                  NXT_ThemeRenderer_GLDraw or NXT_ThemeRenderer_Destroy.
     *
     * \param bCallerOwnsMemory  If this is 1, the caller is responsible for
     *                  freeing the memory pointed to by pData; if this is false
     *                  the theme renderer will manage the memory internally.
     *
     * \return          NXT_Error_None if successful; otherwise an error code.
     */
    NXT_Error NXT_ThemeRenderer_GetPixels(NXT_HThemeRenderer renderer, 
                                          int *pWidth, 
                                          int *pHeight, 
                                          int *pSize,
                                          unsigned char **ppData,
                                          unsigned int bCallerOwnsMemory);
    
    
    /**
     * Waits for the last call to NXT_ThemeRenderer_GLDraw() to complete and returns
     * a pointer to the rendered pixels.
     *
     * \param renderer  The renderer handle
     *
     * \param pWidth    Pointer to a variable to receive the width of the 
     *                  rendered frame (in pixels)
     *
     * \param pHeight   Pointer to a variable to receive the height of the 
     *                  rendered frame (in pixels)
     *
     * \param pSize     Pointer to a variable to receive the size of the 
     *                  rendered frame (in bytes)
     *
     * \param pData    Pointer to a variable to receive a pointer to the actual
     *                  pixel data.
     *
     * \return          NXT_Error_None if successful; otherwise an error code.
     */
    NXT_Error NXT_ThemeRenderer_GetPixels2(NXT_HThemeRenderer renderer,
                                      int *pWidth,
                                      int *pHeight,
                                      int *pSize,
                                      unsigned char *pData);
    
    
    /**
     * Call this when the size of the OpenGL output surface changes.
     */
#ifdef __APPLE__
    void NXT_ThemeRenderer_SurfaceChanged(NXT_HThemeRenderer renderer, int x, int y, int w, int h, int scale);
#else
    void NXT_ThemeRenderer_SurfaceChanged(NXT_HThemeRenderer renderer, int w, int h);
#endif
    
    /**
     * Destroys the specified renderer and releases any ThemeSet that was
     * in use by the renderer.
     */
    void NXT_ThemeRenderer_Destroy(NXT_HThemeRenderer renderer, unsigned int isDetachedContext);
    
    
    /**
     * Upload a texture to OpenGL.
     *
     * This is used to upload video frames.
     *
     * For RGB, RGBA and Luminance:
     *  - data0 points to the pixel data
     *  - data1 should be NULL
     *  - data2 should be NULL
     *
     * For YUV (YUV420):
     *  - data0 points to the Y plane
     *  - data1 points to the U plane (width, height and pitch are half)
     *  - data2 points to the V plane (width, height and pitch are half)
     *
     * For NV12:
     *  - data0 points to the Y plane
     *  - data1 points to the UV plane (U and V interleaved; full width and pitch; half height)
     *  - data2 should be NULL
     *
     * Uploading a texture resets the texture input rectangle to the full size
     * of the texture, and rotation is reset to zero.
     *
     * Returns NXT_Error_None if the upload was successful, or one of the following
     * error codes:
     * -  NXT_Error_ParamRange: texture_id was invalid
     */
    NXT_Error NXT_ThemeRenderer_UploadTexture(NXT_HThemeRenderer renderer,
                                              unsigned int track_id,
                                              NXT_TextureID texture_id,
                                              int width,
                                              int height,
                                              int pitch,
                                              NXT_PixelFormat textureFormat,
                                              unsigned char* data0,          // RGB, RGBA, Luminance or Y channel
                                              unsigned char* data1,          // U channel (for NXT_PixelFormat_YUV; NULL for other formats)
                                              unsigned char* data2,          // V channel (for NXT_PixelFormat_YUV; NULL for other formats)
                                              unsigned int lut_id,
                                              unsigned int tex_id_for_customlut_a,
                                              unsigned int tex_id_for_customlut_b,
                                              unsigned int tex_id_for_customlut_power,
                                              float* prenderinfo);
    
    NXT_Error NXT_ThemeRenderer_SetNativeTextureBuffer(NXT_HThemeRenderer renderer,
                                                       NXT_TextureID texture_id,
                                                       int width,
                                                       int height,
                                                       int width_pitch,
                                                       int height_pitch,
                                                       void* pNativeBuffer);

#if 0 // delete not use graphic buffer
    NXT_Error NXT_ThemeRenderer_SetNativeTextureBuffer2(NXT_HThemeRenderer renderer,
                                                       NXT_TextureID texture_id,
                                                       int width,
                                                       int height,
                                                       int width_pitch,
                                                       int height_pitch,
                                                       void* pEGLImage);
#endif
    NXT_Error NXT_ThemeRenderer_UnsetNativeTextureBuffer(NXT_HThemeRenderer renderer,
                                                         NXT_TextureID texture_id);

    
    NXT_Error NXT_ThemeRenderer_ResetTexture(NXT_HThemeRenderer renderer,
                                             NXT_TextureID texture_id,
                                             int width,
                                             int height);
    
    /**
     * Swaps the contents of the specified two textues.
     *
     * This is done by pointer manipualtion, so it is effectively instant.
     *
     * Example:
     *
     *      NXT_ThemeRenderer_SwapTextures( hRenderer, NXT_TextureID_Video_1, NXT_TextureID_Video_2 );
     *
     * \param renderer      The renderer handle
     *
     * \param texture_id1   The ID of one of the textures to swap (NXT_TextureID_????)
     *
     * \param texture_id2   The ID of theother texture to swap (NXT_TextureID_????)
     *
     * \return          NXT_Error_None if successful; otherwise an error code.
     *
     */  
    NXT_Error NXT_ThemeRenderer_SwapTextures(NXT_HThemeRenderer renderer, 
                                             NXT_TextureID texture_id1, 
                                             NXT_TextureID texture_id2 );

    void NXT_ThemeRenderer_LinkToBaseTrack(NXT_HThemeRenderer renderer, unsigned int uid, unsigned int base_uid);
    
    /**
     * Sets an input rectangle within the texture to render instead of using the
     * whole texture.
     *
     * The coordinates are in pixels, and are measured from the bottom of the
     * texture.
     *
     * Calling NXT_ThemeRenderer_UploadTexture will reset the input rectangle
     * to the whole texture.
     *
     * The following constraints must be satisfied or this will fail:
     *    texture_width > right > left >= 0
     *    texture_height > top > bottom >= 0
     */
    void NXT_ThemeRenderer_SetTextureInputRect(NXT_HThemeRenderer renderer,
                                               NXT_TextureID texture_id,
                                               float left, float bottom,
                                               float right, float top);
    
    
    /**
     * Sets color adjustment for a texture.  The color adjustment is
     * reset every time NXT_ThemeRenderer_UploadTexture is called, so
     * be sure to call this after NXT_ThemeRenderer_UploadTexture if
     * you want to adjust the colors in the texture.
     *
     * \param renderer      The renderer handle
     *
     * \param texture_id    The ID of the texture to which to apply color adjustment (NXT_TextureID_????)
     *
     * \param brightness    The brightness adjustment.  A value between -255 and 255.  Zero to leave brightness unchanged.
     *
     * \param contrast      The contrast adjustment.  A value between -255 and 255.  Zero to leave contrast unchanged.
     *
     * \param saturation    The saturation adjustment.  A value between -255 and 255.  Zero to leave saturation unchanged.
     *
     * \param tintcolor     The tint color in ARGB format to use when reducing the saturation.  The alpha component is
     *                      ignored.  For standard saturation control, use 0x00000000.  To apply a specific tint to the
     *                      color as the saturation is reduced, specify that color here.
     *
     * \param vignette      flag for applying vignette frame on image.
     */
    void NXT_ThemeRenderer_SetTextureColorAdjust(NXT_HThemeRenderer renderer,
                                                 NXT_TextureID texture_id,
                                                 int brightness,
                                                 int contrast,
                                                 int saturation,
                                                 unsigned int tintcolor,
                                                 int hue,
                                                 int vignette);

    /**
     * Sets a post-cropping rotation value for the texture.
     *
     * This is used after the texture is cropped by the input rectangle, but
     * before it is scaled to fit in a 1280x720 frame.  Supported rotations
     * are 0, 90, 180 and 270.
     *
     * Calling NXT_ThemeRenderer_UploadTexture will reset the rotation
     * to zero.
     *
     * The following constraints must be satisfied or this will fail:
     *    rotation % 90 = 0
     */
    void NXT_ThemeRenderer_SetTextureRotation(NXT_HThemeRenderer renderer,
                                              NXT_TextureID texture_id,
                                              unsigned int rotation);

    void NXT_ThemeRenderer_SetUserRotation(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, unsigned int rotation);

    void NXT_ThemeRenderer_SetTranslation(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, int translate_x, int translate_y);
    
    /**
     * Sets a fit-in-frame for the texture.
     *
     * When a fit-in frame is set, extra space will be added on two opposite
     * edges of the texture (left and right OR top and bottom) so that the
     * texture appears as large as possible centered in a frame of the specified
     * size without being cropped and without being distorted.
     *
     * For example, if you want to render 720p video, but the texture may not
     * be 720p, calling this with (1280,720) will scale the texture so it fits
     * within 720p exactly, adding black bars on the edges if necessary to
     * maintain the aspect ratio while still filling 720p.
     *
     * If either value (with or height) is zero, fitting is disabled and the
     * texture is stretched to fit whatever output rectangle it is given in
     * the theme file.
     *
     * Calling NXT_ThemeRenderer_UploadTexture will reset the width and height
     * to zero, effectively disabling fitting.
     */
    void NXT_ThemeRenderer_SetTextureFitInFrame(NXT_HThemeRenderer renderer,
                                                NXT_TextureID texture_id,
                                                unsigned int width,
                                                unsigned int height);
    
    
    // For testing
    void NXT_ThemeRenderer_SetVideoPlaceholders(NXT_HThemeRenderer renderer,
                                                const char *video1,
                                                const char *video2);
    
    /**
     * Sets the log level for the NexTheme and NexThemeRenderer components.
     *
     * The following log levels are supported:
     *
     *         -1  No log output at all
     *          0  Log verison number on start
     *          1  Log fatal errors (crashes, etc)
     *          2  Log all errors (DEFAULT)
     *          3  Log warnings
     *          4  Log general information
     *          5  Log debugging information
     *          6  Log verbose information (like a trace)
     *          7  Log extra verbose information (like a trace but with extra info)
     */
    void NXT_ThemeRenderer_SetLogLevel( int level );
    

    void NXT_ThemeRenderer_SetTextureTransform(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, float* startMatrix, unsigned int startTime, float* endMatrix, unsigned int endTime);

    typedef int (*LOADTHEMECALLBACKFUNC)(char** ppOutputData, int* pLength, char* path, void* cbdata);
	//Jeff
	void NXT_ThemeRenderer_SetHQScale(NXT_HThemeRenderer renderer, int src_w, int src_h, int dest_w, int dest_h);
	//Jeff VIDEO LAYER----------------------------------------------------------------------------------
	int NXT_ThemeRenderer_CreateTextureForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid);
	int NXT_ThemeRenderer_ReleaseTextureForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image);
    int NXT_ThemeRenderer_CreateTextureForTrackID(NXT_HThemeRenderer renderer, unsigned int uid);
    int NXT_ThemeRenderer_ReleaseTextureForTrackID(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image);
    int NXT_ThemeRenderer_CreateDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid);
    int NXT_ThemeRenderer_GetDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid);
    int NXT_ThemeRenderer_ReleaseDecoTexture(NXT_HThemeRenderer renderer, unsigned int uid, int clear_image);
    int NXT_ThemeRenderer_GetTextureNameForVideoLayer(NXT_HThemeRenderer renderer, unsigned int uid);
    int NXT_ThemeRenderer_GetTextureNameForVideoLayerExternal(NXT_HThemeRenderer renderer, unsigned int uid, const float** ppmatrix);
	int NXT_ThemeRenderer_GetTextureNameForMask(NXT_HThemeRenderer renderer);
    int NXT_ThemeRenderer_GetTextureNameForBlend(NXT_HThemeRenderer renderer);
	int NXT_ThemeRenderer_GetTextureNameForWhite(NXT_HThemeRenderer renderer);
	int NXT_ThemeRenderer_CleanupMaskWithWhite(NXT_HThemeRenderer renderer);
    int NXT_ThemeRenderer_SwapBlendMain(NXT_HThemeRenderer renderer);
	int NXT_ThemeRenderer_SetRenderToMask(NXT_HThemeRenderer renderer);
	int NXT_ThemeRenderer_SetRenderToDefault(NXT_HThemeRenderer renderer);
	int NXT_ThemeRenderer_SetTextureNameForLUT(NXT_HThemeRenderer renderer, unsigned int uid, float x, float y);
    void NXT_ThemeRenderer_ConvertTextureToRGB(NXT_HThemeRenderer renderer, NXT_TextureInfo* ptex, int texid_for_customlut_a, int texid_for_customlut_b, int custom_lut_power, int texid_for_lut, float* prenderinfo, int use_HDR10, float maxCLL);
    int NXT_ThemeRenderer_GetLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id);
    int NXT_ThemeRenderer_SetLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id, int tex_id);
    void NXT_ThemeRenderer_SetLUT(NXT_HThemeRenderer renderer, unsigned int uid, int lut);
    void NXT_ThemeRenderer_SetCustomLUTA(NXT_HThemeRenderer renderer, unsigned int uid, int lut);
    void NXT_ThemeRenderer_SetCustomLUTB(NXT_HThemeRenderer renderer, unsigned int uid, int lut);
    void NXT_ThemeRenderer_SetCustomLUTPower(NXT_HThemeRenderer renderer, unsigned int uid, int power);
    void NXT_ThemeRenderer_RegisterGetLutTexWithHash(NXT_HThemeRenderer renderer, int (*pfunc)(int, int));
    void NXT_ThemeRenderer_RegisterGetEffectImagePath(NXT_HThemeRenderer renderer, int (*pfunc)(const char*, char*));
    void NXT_ThemeRenderer_RegisterGetVignetteTexID(NXT_HThemeRenderer renderer, int (*pfunc)(int));
    int NXT_ThemeRenderer_GetLutTexWithID(NXT_HThemeRenderer renderer, int lut_resource_id);
    int NXT_ThemeRenderer_GetEffectImagePath(NXT_HThemeRenderer renderer, const char*, char*);
    int NXT_ThemeRenderer_GetVignetteTexID(NXT_HThemeRenderer renderer);

    typedef struct NXT_ThemeSet_    *NXT_HThemeSet;
	
    int NXT_ThemeRenderer_CreateRenderItem(NXT_HThemeRenderer renderer, int effect_id);
    int NXT_ThemeRenderer_CreateNexEDL(NXT_HThemeRenderer renderer, const char* uid, NXT_HThemeSet themeset);
    void NXT_ThemeRenderer_ReleaseRenderItem(NXT_HThemeRenderer renderer, int effect_id);
    void NXT_ThemeRenderer_DrawRenderItemOverlay(NXT_HThemeRenderer renderer, int effect_id, int tex_id_for_second, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mask_enabled);
    void NXT_ThemeRenderer_DrawNexEDLOverlay(NXT_HThemeRenderer renderer, int effect_id, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mode);
    void NXT_ThemeRenderer_ClearRenderItems(NXT_HThemeRenderer renderer);
    int NXT_ThemeRenderer_GetRenderItemEffectID(NXT_HThemeRenderer renderer, const char* id);
    void NXT_ThemeRenderer_GetRenderItem(NXT_HThemeRenderer renderer, const char* pid, const char* uid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* cbdata);
    void NXT_ThemeRenderer_AddRenderItem(NXT_HThemeRenderer renderer, void* pitem);
    int NXT_ThemeRenderer_CountOfRenderItem(NXT_HThemeRenderer renderer);
    //--------------------------------------------------------------------------------------------------
    void* NXT_Theme_CreateTextureManager(int timeout, int max_mem_size);
    void NXT_Theme_ReleaseTextureManager(void* pmanager, int isDetachedContext);
    void NXT_Theme_UpdateTextureManager(void* pmanager);
    void NXT_Theme_SetTextureInfo(void* pmanager, const char* path, NXT_TextureInfo* ptex_info);
    void NXT_Theme_RemoveTextureInfo(void* pmanager, const char* path);
    void NXT_Theme_SetTextureInfoAsync(NXT_HThemeRenderer renderer, const char* path, int* pixels, int width, int height);
    void NXT_Theme_RemoveTextureInfoAsync(NXT_HThemeRenderer renderer, const char* path);
    int NXT_Theme_GetTextureInfo(void* pmanager, const char* path, NXT_TextureInfo* ptex_info);
    void NXT_Theme_ResetTextureManager_Internal(void* pmanager);
    void NXT_Theme_ResetTextureManager(NXT_HThemeRenderer renderer);
    //For 360 Video Info
    void NXT_Theme_SetTexture360Flag(NXT_HThemeRenderer renderer, NXT_TextureID texture_id, unsigned int flag);
    void NXT_ThemeRenderer_set360View(NXT_HThemeRenderer rendererm, float x, float y);
    
    //Case
    NXT_Error NXT_ThemeRenderer_InitFastPreview(NXT_HThemeRenderer renderer, int w, int h, int maxCount, NXT_RendererOutputType outputType);
    NXT_Error NXT_ThemeRenderer_DeinitFastPreview(NXT_HThemeRenderer renderer);
	NXT_Error NXT_ThemeRenderer_GetFastPreviewBufferTime(NXT_HThemeRenderer renderer, unsigned int* pStart, unsigned int* pEnd);
    NXT_Error NXT_ThemeRenderer_AddFastPreviewFrame(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark, int iTime);
    NXT_Error NXT_ThemeRenderer_GLDrawForFastPreview(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int disable_watermark, int iTime);
    
    void NXT_ThemeRenderer_SetDefaultFrameBuffer(NXT_HThemeRenderer renderer,unsigned int frameBufferNum);

    //yoon
    int NXT_ThemeRenderer_ReleaseLUTTexture(NXT_HThemeRenderer renderer, int lut_resource_id);

    //NEW TODO 2017.02.15

    NXT_HThemeSet NXT_ThemeRenderer_GetKEDLEffectID(NXT_HThemeRenderer renderer, const char* id);
    NXT_HThemeSet NXT_ThemeRenderer_GetKEDLItem(NXT_HThemeRenderer renderer, const char* uid, const char* src);
    void NXT_ThemeRenderer_ReleaseKEDLEffectID(NXT_HThemeRenderer renderer, const char* id, int isDetachedConext);
    void NXT_ThemeRenderer_ReleaseKEDLEffectIDByContext(NXT_HThemeRenderer_Context context, const char* id, int isDetachedConext);

    int NXT_ThemeRenderer_SetBaseFilterRenderItemUID(NXT_HThemeRenderer renderer, const char* uid);

    int* NXT_ThemeRenderer_CreateCubeLUT(const char* stream);
    int* NXT_ThemeRenderer_CreateLGLUT(const char* stream);

    void NXT_ThemeRenderer_SetForceRTT(NXT_HThemeRenderer renderer, int flag);

    int NXT_ThemeRenderer_GetForceRTT(NXT_HThemeRenderer renderer);

    void NXT_ThemeRenderer_SetBrightness(NXT_HThemeRenderer renderer, int value);

    void NXT_ThemeRenderer_SetContrast(NXT_HThemeRenderer renderer, int value);

    void NXT_ThemeRenderer_SetSaturation(NXT_HThemeRenderer renderer, int value);

    int NXT_ThemeRenderer_GetBrightness(NXT_HThemeRenderer renderer);

    int NXT_ThemeRenderer_GetContrast(NXT_HThemeRenderer renderer);

    int NXT_ThemeRenderer_GetSaturation(NXT_HThemeRenderer renderer);

    void NXT_ThemeRenderer_SetVignette(NXT_HThemeRenderer renderer, int value);
    
    void NXT_ThemeRenderer_SetVignetteRange(NXT_HThemeRenderer renderer, int value);
    
    void NXT_ThemeRenderer_SetSharpness(NXT_HThemeRenderer renderer, int value);

    int NXT_ThemeRenderer_GetVignette(NXT_HThemeRenderer renderer);
    
    int NXT_ThemeRenderer_GetVignetteRange(NXT_HThemeRenderer renderer);
    
    int NXT_ThemeRenderer_GetSharpness(NXT_HThemeRenderer renderer);
    
    NXT_Error NXT_ThemeRenderer_GLDrawSrc(NXT_HThemeRenderer renderer, NXT_RendererOutputType outputType, int src_id);

    //sangjun.LEE
    int NXT_ThemeRenderer_SetDeviceLightLevel(NXT_HThemeRenderer renderer, int lightLevel);
    int NXT_ThemeRenderer_SetDeviceGamma(NXT_HThemeRenderer renderer, float gamma);

    void NXT_ThemeRenderer_DoPlayCacheWork(NXT_HThemeRenderer renderer, struct NXT_PrecacheResourceInfo* pinfo, unsigned int live_start, unsigned int live_end);
    void NXT_ThemeRenderer_ClearCachedBitmap(NXT_HThemeRenderer renderer, unsigned int live_start, unsigned int live_end); 
    void NXT_ThemeRenderer_SetMapper(NXT_HThemeRenderer renderer, void* pmapper);
#ifdef __cplusplus
};
#endif
#endif //NEXTHEMERENDERER_H
