#pragma once

#include "glext_loader.h"
#include <vector>

enum
{
	MAX_RENDER_TARGET_RESOURCE = 15,
};

typedef struct NXT_RenderTarget_
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
} NXT_RenderTarget;

typedef struct RenderTargetResource_{
	NXT_RenderTarget* prender_target_;
	int unused_counter_;
}RenderTargetResource;

typedef struct RenderTargetManager_{
	int count_of_render_target_resource_;
	RenderTargetResource render_target_resource_[MAX_RENDER_TARGET_RESOURCE];
	RenderTargetManager_() :count_of_render_target_resource_(0){}
}RenderTargetManager;

class RenderItem;

typedef std::vector<NXT_RenderTarget*> RenderTargetStack_t;

struct NXT_ThemeRenderer_ {

	//Jeff RTT------------------------------------------------------------------------------------------
	RenderTargetManager render_target_manager_;
	RenderTargetStack_t render_target_stack_;
	//--------------------------------------------------------------------------------------------------
	unsigned int view_width;
	unsigned int view_height;

	NXT_RenderTarget* pdefault_render_target_;

	RenderItem* pcurrent_render_item_;

	GLuint current_program_id_;
};

typedef struct NXT_ThemeRenderer_ NXT_ThemeRenderer;
typedef struct NXT_ThemeRenderer_ *NXT_HThemeRenderer;

int initRenderTargetElement(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target, int w, int h, int alpha_request, int depth_request, int stencil_request);

void releaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target, unsigned int isDetachedConext);

void setRenderTargetAsDefault(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target);

void setRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target);

NXT_RenderTarget* NXT_ThemeRenderer_GetRenderTarget(NXT_HThemeRenderer renderer, int width, int height, int alpha_request, int depth_request, int stencil_request);

void NXT_ThemeRenderer_ReleaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target);

void NXT_ThemeRenderer_UpdateRenderTargetManager(NXT_HThemeRenderer renderer);

void NXT_ThemeRenderer_UseProgram(NXT_HThemeRenderer renderer, GLuint program_id);