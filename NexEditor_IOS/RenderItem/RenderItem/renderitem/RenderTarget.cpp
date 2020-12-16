#include "RenderTarget.h"
#include "util.h"
//#include "NexSAL_Internal.h"

void NXT_ThemeRenderer_UseProgram(NXT_HThemeRenderer renderer, GLuint program_id){

	//nexSAL_TraceCat(0, 0, "RenderItem [%s %d] program_id:%d", __func__, __LINE__, program_id);
	if (program_id == renderer->current_program_id_)
		return;
	renderer->current_program_id_ = program_id;
	glUseProgram(program_id);
}

int initRenderTargetElement(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target, int w, int h, int alpha_request, int depth_request, int stencil_request){
	prender_target->width_ = w;
	prender_target->height_ = h;
	prender_target->alpha_ = alpha_request;
	prender_target->depth_ = depth_request;
	prender_target->stencil_ = stencil_request;

	glGenFramebuffers(1, &prender_target->fbo_);
	if (depth_request){
		glGenRenderbuffers(1, &prender_target->depth_buffer_);
	}
	glGenTextures(1, &prender_target->target_texture_);

	int color_format = alpha_request ? GL_RGBA : GL_RGB;

	glBindTexture(GL_TEXTURE_2D, prender_target->target_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, color_format, w, h, 0, color_format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (depth_request){
		glBindRenderbuffer(GL_RENDERBUFFER, prender_target->depth_buffer_);
		if (stencil_request){
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
//#if defined(ANDROID)
//			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, w, h);
//#else
//			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
//#endif
			
		}
		else{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, prender_target->fbo_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prender_target->target_texture_, 0);
	if (depth_request){
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, prender_target->depth_buffer_);
	}
	if (stencil_request){
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, prender_target->depth_buffer_);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return 0;
	prender_target->inited_ = 1;
	return 1;
}

void releaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target, unsigned int isDetachedConext){
	if (prender_target->inited_){
		if (0 == isDetachedConext){
			if (prender_target->depth_buffer_ > 0) glDeleteRenderbuffers(1, &prender_target->depth_buffer_);
			if (prender_target->fbo_ > 0) glDeleteFramebuffers(1, &prender_target->fbo_);
			if (prender_target->target_texture_ > 0) glDeleteTextures(1, &prender_target->target_texture_);
		}
		prender_target->target_texture_ = 0;
		prender_target->depth_buffer_ = 0;
		prender_target->fbo_ = 0;
		prender_target->width_ = 0;
		prender_target->height_ = 0;
		prender_target->inited_ = 0;
	}
}

void setRenderTargetAsDefault(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target){

	renderer->pdefault_render_target_ = prender_target;
	setRenderTarget(renderer, prender_target);
}

void setRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target){

	if (NULL == prender_target){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderer->view_width, renderer->view_height);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, prender_target->fbo_);
	glViewport(0, 0, prender_target->width_, prender_target->height_);
}

NXT_RenderTarget* NXT_ThemeRenderer_GetRenderTarget(NXT_HThemeRenderer renderer, int width, int height, int alpha_request, int depth_request, int stencil_request){
	RenderTargetManager* pmanager = &renderer->render_target_manager_;
	int i = 0;
	for (i = pmanager->count_of_render_target_resource_ - 1; i >= 0; --i){
		RenderTargetResource* pres = &pmanager->render_target_resource_[i];
		if (
			(pres->prender_target_->width_ == width)
			&& (pres->prender_target_->height_ == height)
			&& (pres->prender_target_->alpha_ == alpha_request)
			&& (pres->prender_target_->depth_ == depth_request)
			&& (pres->prender_target_->stencil_ == stencil_request)
			){
			NXT_RenderTarget* pret = pres->prender_target_;
			pmanager->render_target_resource_[i] = pmanager->render_target_resource_[--pmanager->count_of_render_target_resource_];
			return pret;
		}
	}
	NXT_RenderTarget* prender_target = (NXT_RenderTarget*)malloc(sizeof(NXT_RenderTarget));
	initRenderTargetElement(renderer, prender_target, width, height, alpha_request, depth_request, stencil_request);
	return prender_target;
}

void NXT_ThemeRenderer_ReleaseRenderTarget(NXT_HThemeRenderer renderer, NXT_RenderTarget* prender_target){
	if (NULL == prender_target)
		return;
	RenderTargetManager* pmanager = &renderer->render_target_manager_;
	RenderTargetResource* pres = &pmanager->render_target_resource_[pmanager->count_of_render_target_resource_];
	pres->prender_target_ = prender_target;
	pres->unused_counter_ = 0;
	pmanager->count_of_render_target_resource_++;
}

void NXT_ThemeRenderer_UpdateRenderTargetManager(NXT_HThemeRenderer renderer){
	RenderTargetManager* pmanager = &renderer->render_target_manager_;
	int i = 0;
	for (i = 0; i < pmanager->count_of_render_target_resource_;){
		RenderTargetResource* pres = &pmanager->render_target_resource_[i];
		if (pres->unused_counter_ > 10){
			releaseRenderTarget(renderer, pres->prender_target_, 0);
			free(pres->prender_target_);
			pres->prender_target_ = NULL;
			pmanager->count_of_render_target_resource_--;
			pmanager->render_target_resource_[i] = pmanager->render_target_resource_[pmanager->count_of_render_target_resource_];
		}
		else
			++i;
		pres->unused_counter_++;
	}
}