//
//  NexThemeRenderer_Platform.h
//  NexEffectRenderer
//
//  Created by Simon Kim on 8/29/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#ifndef NexThemeRenderer_Platform_h
#define NexThemeRenderer_Platform_h

#include "NexThemeRenderer.h"
#include <stdint.h>

struct NXT_ThemeRenderer_;

typedef enum {
    THEMERENDERER_DESTROY_PHASE_INTERNAL_CONTEXT
} THEMERENDERER_DESTROY_PHASE;

typedef enum {
    SET_CONTEXT_MODE_RELEASE,
    SET_CONTEXT_MODE_PREVIEW,
    SET_CONTEXT_MODE_EXPORT,
    SET_CONTEXT_MODE_JUST_FOR_ALIGN = 0x7fffffff
} NEXTHEMERENDERER_SET_CONEXTEXT_MODE;

typedef enum {
    PRESENT_MODE_PREVIEW,
    PRESENT_MODE_EXPORT,
    PRESENT_MODE_JUST_FOR_ALIGN = 0x7fffffff
} NEXTHEMERENDERER_PRESENT_MODE;

typedef struct {
    uint32_t width;
    uint32_t height;
} NXT_Size;

typedef struct {
    int32_t x;
    int32_t y;
    NXT_Size size;
    float scale;
} NXT_Platform_RenderTargetInfo;

#ifdef __cplusplus
extern "C" {
#endif

    NXT_Error NXT_Platform_CreateInternalContext(struct NXT_ThemeRenderer_ * hRenderer, struct NXT_ThemeRenderer_ * hAnotherRendererForShareGroup);
    NXT_Error NXT_Platform_DeleteInternalContext( struct NXT_ThemeRenderer_ * hRenderer );
    NXT_Error NXT_Platform_CheckInternalContextAvailable(struct NXT_ThemeRenderer_ * hRenderer);
    NXT_Error NXT_Platform_SetCurrentInternalContext( struct NXT_ThemeRenderer_ * hRenderer, NEXTHEMERENDERER_SET_CONEXTEXT_MODE mode );
    NXT_Error NXT_Platform_ReleaseContext(struct NXT_ThemeRenderer_ * hRenderer);

    NXT_Error NXT_Platform_PresentBuffer(struct NXT_ThemeRenderer_ * hRenderer, NEXTHEMERENDERER_PRESENT_MODE presentMode);
    void NXT_Platform_AfterDraw(struct NXT_ThemeRenderer_ * hRenderer);

    NXT_Error NXT_Platform_SetRenderTargetInfo(struct NXT_ThemeRenderer_ * hRenderer, NXT_Platform_RenderTargetInfo info);
    NXT_Error NXT_Platform_UnloadTexture(struct NXT_ThemeRenderer_ * hRenderer, NXT_TextureID texture_id);

    NXT_Error NXT_Platform_GetProperty(struct NXT_ThemeRenderer_ * hRenderer, int nPropertyName, void**ppRetValue);

#ifdef __cplusplus
};
#endif
#endif /* NexThemeRenderer_Platform_h */
