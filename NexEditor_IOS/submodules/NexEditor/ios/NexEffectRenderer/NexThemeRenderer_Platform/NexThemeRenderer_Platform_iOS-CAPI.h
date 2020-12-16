/**
 * File Name   : NexThemeRendererInternalAPI-C.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#ifndef NexthemeRendererInternalAPI_C_h
#define NexthemeRendererInternalAPI_C_h
#include "NexThemeRenderer.h"
#include "NexThemeRenderer_Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

    NXT_Error NXT_ThemeRenderer_GetPixelBuffer(NXT_HThemeRenderer renderer,
                                               void **ppData,
                                               int *pSize);

    NXT_Error NXT_ThemeRenderer_ReturnPixelBufferForReuse(NXT_HThemeRenderer renderer, void *pPixelBuffer);

    NXT_Error NXT_ThemeRenderer_UploadTextureForHWDecoder(NXT_HThemeRenderer renderer,
                                                          NXT_TextureID texture_id,
                                                          unsigned char* data0,
                                                          unsigned char* data1,
                                                          unsigned char* data2,
                                                          int convert_to_rgb_flag,
                                                          int lut_id);
    
    
    /// Requests for a new output pixel buffer from internal pixel buffer supply and use it for the next rendering
    NXT_Error NXT_ThemeRenderer_SwitchToNextOutputPixelBuffer(NXT_HThemeRenderer renderer);
    
    NXT_Error NXT_ThemeRenderer_SetRenderTargetInformation( NXT_HThemeRenderer renderer, int reqWidth, int reqHeight );
    
    void NXT_ThemeRenderer_GetRendererViewSize(NXT_HThemeRenderer renderer, unsigned int *width, unsigned int *height);
    
#ifdef __cplusplus
};
#endif

#endif /* NexthemeRendererInternalAPI_C_h */
