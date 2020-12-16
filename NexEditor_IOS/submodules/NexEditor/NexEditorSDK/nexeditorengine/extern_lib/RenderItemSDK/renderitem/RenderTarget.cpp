#include "RenderTarget.h"
#include "util.h"
#include "NexSAL_Internal.h"
#include "types.h"
#include "NexMath.h"
#include "nexTexturesLogger.h"

using namespace RI;

GLuint RI_ThemeRenderer_::getWhiteTexID(){

	return white_tex_id_;
}

GLuint RI_ThemeRenderer_::getMaskTexID(){

	return current_mask_id_;
}

GLuint RI_ThemeRenderer_::getBlendTexID(){

	return current_blend_id_;
}

void RI_ThemeRenderer_::updateDefaultTexID(GLuint mask, GLuint blend, GLuint white){
    
	current_mask_id_ = mask;
	current_blend_id_ = blend;
    white_tex_id_ = white;
}

void RI_ThemeRenderer_::resetCurrentProgramID(){

	current_program_id_ = 0xFFFFFFFF;
}

void RI_ThemeRenderer_::resetToDefaultRenderTarget(){

	setRenderTarget(pdefault_render_target_);
}

NXT_RenderTarget* RI_ThemeRenderer_::getDefaultRenderTarget(){

	return pdefault_render_target_;
}

void RI_ThemeRenderer_::setCurrentRenderItem(RenderItem* prenderitem){

	pcurrent_render_item_ = prenderitem;
}

RenderItem* RI_ThemeRenderer_::getCurrentRenderItem(){

	return pcurrent_render_item_;
}

int RI_ThemeRenderer_::getRenderTargetStackPoint(){

	return render_target_stack_.size();
}

void RI_ThemeRenderer_::releaseRenderTargetStackToThePoint(int point){

	while(render_target_stack_.size() > point){

		NXT_RenderTarget* prender_target = render_target_stack_.back();
		releaseRenderTarget(prender_target);
		render_target_stack_.pop_back();
	}
}

void RI_ThemeRenderer_::pushRenderTarget(NXT_RenderTarget* prender_target){

	render_target_stack_.push_back(prender_target);
}

void RI_ThemeRenderer_::useProgram(GLuint program_id){

	if (program_id == current_program_id_)
		return;
	current_program_id_ = program_id;
	glUseProgram(program_id);
}

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

	GLint old_texture = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_texture);
	glBindTexture(GL_TEXTURE_2D, target_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, color_format, w, h, 0, color_format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, old_texture);

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

int RI_ThemeRenderer_::initRenderTargetElement(NXT_RenderTarget* prender_target, int w, int h, int alpha_request, int depth_request, int stencil_request){

	if(prender_target->init(w, h, alpha_request, depth_request, stencil_request))
		return 1;
	return 0;
}

void RI_ThemeRenderer_::releaseRenderTarget(NXT_RenderTarget* prender_target, unsigned int isDetachedConext){

	prender_target->destroy(isDetachedConext);
}

void RI_ThemeRenderer_::setRenderTargetAsDefault(NXT_RenderTarget* prender_target, bool set_immediately){

	pdefault_render_target_ = prender_target;
	if(set_immediately)
		setRenderTarget(prender_target);
}

void RI_ThemeRenderer_::setRenderTarget(NXT_RenderTarget* prender_target){

	if (NULL == prender_target){
		glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_num_);
		glViewport(0, 0, view_width, view_height);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, prender_target->fbo_);
	glViewport(0, 0, prender_target->width_, prender_target->height_);
}

NXT_RenderTarget* RI_ThemeRenderer_::getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request){

	RenderTargetManager* pmanager = &this->render_target_manager_;
	
	return pmanager->getRenderTarget(width, height, alpha_request, depth_request, stencil_request);
}

void RI_ThemeRenderer_::setDefaultFramebufferNum(unsigned int default_framebuffer_num){

	default_framebuffer_num_ = default_framebuffer_num;
}

NXT_RenderTarget* RenderTargetManager::getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request){

    for(RenderTargetResourceList_t::iterator itor = resource_.begin(); itor != resource_.end(); ++itor){

        RenderTargetResource& res = *itor;
        if((res.prender_target_->width_ == width)
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

void RI_ThemeRenderer_::releaseRenderTarget(NXT_RenderTarget* prender_target){
	
	RenderTargetManager* pmanager = &this->render_target_manager_;
	pmanager->releaseRenderTarget(prender_target);
}

void RI_ThemeRenderer_::updateRenderTargetManager(){

	RenderTargetManager* pmanager = &this->render_target_manager_;
	pmanager->update();
}

RI_ThemeRenderer_::RI_ThemeRenderer_():default_framebuffer_num_(0){

	colorMask[0] = 1;
	colorMask[1] = 1;
	colorMask[2] = 1;
	colorMask[3] = 1;
    max_texture_size = 2048;
}

RI_ThemeRenderer_::~RI_ThemeRenderer_(){

	while(render_target_stack_.size() > 0){

		NXT_RenderTarget* prender_target = render_target_stack_.back();
		releaseRenderTarget(prender_target);
		render_target_stack_.pop_back();
	}

	render_target_manager_.destroy();
}

RI_ThemeRenderer_::TexShader::~TexShader(){

	glDeleteProgram(program_);
}

void RI_ThemeRenderer_::TexShader::init(){

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

RI_ThemeRenderer_::SolidShader::~SolidShader(){

	glDeleteProgram(program_);
}

void RI_ThemeRenderer_::SolidShader::init(){

	if(program_ == 0){

		static const GLchar* vertex_shader_src = {

			"uniform mat4 u_mvp_matrix;\n"
			"attribute vec4 a_position;\n"
			"void main() {\n"
			"	gl_Position = a_position * u_mvp_matrix;\n"
			"}\n"
		};

		static const GLchar* fragment_shader_src = {

			"void main() {\n"
			"	gl_FragColor = vec4(1, 1, 1, 1);\n"
			"}\n"
		};

		char error_text[1024];
		GLsizei length = 0;

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		GLint size = strlen(vertex_shader_src);
		glShaderSource(vertex_shader, 1, &vertex_shader_src, &size);
		glCompileShader(vertex_shader);
		
		glGetShaderInfoLog(vertex_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

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
		a_pos_ = glGetAttribLocation(program_, "a_position"); CHECK_GL_ERROR();
	}	
}

void RI_ThemeRenderer_::drawPRS(NXT_RenderTarget* psrc, const Vector3& pos, const Vector3& scale, const Vector3& rotate_axis, float angle, float scr_ratio_w, float scr_ratio_h){

	tex_.init();
	glUseProgram(tex_.program_);
	glUniform1f(tex_.alpha_, 1.0f);

	static bool inited = false;
	static RITM_NXT_Matrix4f default_matrix;
	if (!inited){

		float fov = 45.0f;
		float aspect = 1280.0f / 720.0f;
		float near_plane = 0.01f;
		float far_plane = 3000.0f;
		RITM_NXT_Vector4f axis = { -640.0f, -360.0f, -870.0f, 0.0f };
		default_matrix = RITM_NXT_Matrix4f_MultMatrix(RITM_NXT_Matrix4f_Perspective(fov, aspect, near_plane, far_plane), RITM_NXT_Matrix4f_Scale(1.0f, -1.0f, 1.0f));
		default_matrix = RITM_NXT_Matrix4f_MultMatrix(default_matrix, RITM_NXT_Matrix4f_Translate(axis));
		inited = true;
	}
	float half_w = 1280.0f * scr_ratio_w * 0.5f;//float(psrc->width_) * 0.5f;
	float half_h = 720.0f * scr_ratio_h * 0.5f;//float(psrc->height_) * 0.5f;

	float radian = angle / 180.0f * M_PI;
	RITM_NXT_Vector4f axis = { rotate_axis.x, rotate_axis.y, rotate_axis.z, 0.0f };
	RITM_NXT_Vector4f trans = { pos.x, pos.y, pos.z, 0.0f };
	RITM_NXT_Vector4f pretrans = { half_w * scale.x, half_h * scale.y, 0.0f, 0.0f };

	RITM_NXT_Matrix4f matrix = RITM_NXT_Matrix4f_Identity();
	//matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Translate(pretrans));
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, default_matrix);
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Translate(trans));
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Translate(pretrans));
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Rotate(axis, radian));
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Scale(scale.x, scale.y, scale.z));
	

	glUniformMatrix4fv(tex_.mvp_matrix_, 1, false, matrix.e);

	glActiveTexture(GL_TEXTURE0);                                        CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, psrc->target_texture_);                 CHECK_GL_ERROR();
	glUniform1i(tex_.tex_sampler_, 0);                                           CHECK_GL_ERROR();

	float texcoord_buffer[] =
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,

	};

	

	float position_buffer[] =
	{
		-half_w, half_h, 0, 1,
		-half_w, -half_h, 0, 1,
		half_w, half_h, 0, 1,
		half_w, -half_h, 0, 1,

	};
	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(tex_.a_pos_); CHECK_GL_ERROR();
	glEnableVertexAttribArray(tex_.a_tex_); CHECK_GL_ERROR();
	glVertexAttribPointer(tex_.a_pos_, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(tex_.a_tex_, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
}

void RI_ThemeRenderer_::drawMATRIX(NXT_RenderTarget* psrc, float* pmatrix, float left, float top, float right, float bottom, float alpha){

	drawTexBox(psrc->target_texture_, false, pmatrix, left, top, right, bottom, alpha);
}

static float idmatrix[] = {

	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

void RI_ThemeRenderer_::drawTexBoxWTexCoord(int tex, bool mask_enabled, float* pmatrix, float* ltrb, float* tx_ltrb, float alpha, int no_filter, int reverse){

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
	glBindTexture(GL_TEXTURE_2D, mask_enabled?current_mask_id_:white_tex_id_);    CHECK_GL_ERROR();
	glUniform1i(tex_.mask_sampler_, 1);                                           CHECK_GL_ERROR();

    float tx_left = tx_ltrb[0];
    float tx_top = tx_ltrb[1];
    float tx_right = tx_ltrb[2];
    float tx_bottom = tx_ltrb[3];

	float texcoord_buffer[] =
	{
		tx_left, tx_bottom,
		tx_left, tx_top,
		tx_right, tx_bottom,
		tx_right, tx_top,
	};

	float left = ltrb[0];
    float top = ltrb[1];
    float right = ltrb[2];
    float bottom = ltrb[3];

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

void RI_ThemeRenderer_::drawTexBox(int tex, bool mask_enabled, float* pmatrix, float left, float top, float right, float bottom, float alpha, int no_filter, int reverse){

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
	glBindTexture(GL_TEXTURE_2D, mask_enabled?current_mask_id_:white_tex_id_);    CHECK_GL_ERROR();
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

void RI_ThemeRenderer_::drawSolid(float* pmatrix, float left, float top, float right, float bottom){

	solid_.init();
	glUseProgram(solid_.program_);

	glUniformMatrix4fv(solid_.mvp_matrix_, 1, false, pmatrix?pmatrix:idmatrix);

	float position_buffer[] =
	{
		left, bottom, 0, 1,
		left, top, 0, 1,
		right, bottom, 0, 1,
		right, top, 0, 1,
	};

	glDisable(GL_CULL_FACE);
	glEnableVertexAttribArray(solid_.a_pos_); CHECK_GL_ERROR();
	glVertexAttribPointer(solid_.a_pos_, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
}
