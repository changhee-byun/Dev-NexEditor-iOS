/******************************************************************************
* File Name   :	NexThemeRenderer_SurfaceTexture.h
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

#ifndef NexThemeTest_NexThemeRenderer_SurfaceTexture_h
#define NexThemeTest_NexThemeRenderer_SurfaceTexture_h
#include "NexThemeRenderer.h"

#ifdef __cplusplus
extern "C" {
#endif

    NXT_Error NXT_ThemeRenderer_InitForSurfaceTexture( NXT_HThemeRenderer renderer );

    NXT_Error NXT_ThemeRenderer_DeinitForSurfaceTexture( NXT_HThemeRenderer renderer );

    NXT_Error NXT_ThemeRenderer_CreateSurfaceTexture( NXT_HThemeRenderer renderer, ANativeWindow** ppNativeWindow );

    NXT_Error NXT_ThemeRenderer_DestroySurfaceTexture( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow );

    NXT_Error NXT_ThemeRenderer_WaitForFrameToBeAvailable( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, int nTimeOutInMS );

    NXT_Error NXT_ThemeRenderer_CreateOutputSurface( NXT_HThemeRenderer renderer, ANativeWindow** ppNativeWindow );
	
    NXT_Error NXT_ThemeRenderer_DestroyOutputSurface( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow );
	

    NXT_Error NXT_ThemeRenderer_UnsetSurfaceTexture(NXT_HThemeRenderer renderer,
                                                    NXT_TextureID texture_id);

    NXT_Error NXT_ThemeRenderer_SetSurfaceTexture(NXT_HThemeRenderer renderer,
                                                  unsigned int track_id,
                                                  NXT_TextureID texture_id,
                                                  int width,
                                                  int height,
                                                  int width_pitch,
                                                  int height_pitch,
                                                  ANativeWindow *pNativeWindow,
                                                  int convert_to_rgb_flag,
                                                  int tex_id_for_lut,
                                                  int tex_id_for_customlut_a,
                                                  int tex_id_for_customlut_b,
                                                  int tex_id_for_customlut_power,
                                                  int color_transfer_function_id = 0,
                                                  float max_cll = 0.0f);

    NXT_Error NXT_ThemeRenderer_DiscardSurfaceTexture(NXT_HThemeRenderer renderer,
                                                      ANativeWindow* pNativeWindow);
#ifdef __cplusplus
}
#endif

    
#endif //NexThemeTest_NexThemeRenderer_SurfaceTexture_h
