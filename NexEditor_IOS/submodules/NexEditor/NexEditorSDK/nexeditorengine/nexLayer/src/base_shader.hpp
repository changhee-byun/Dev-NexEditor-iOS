#pragma once

class BaseShader:public ShaderDecorator{

	int a_position_;
	int a_texCoord_;
	int u_textureSampler_;
	int u_mvp_matrix_;
	int u_alpha_test_;
	
public:
	virtual ShaderDecorator* clone(){

		return new BaseShader();
	}

	void getAttribIndex(int program){

		a_position_ = glGetAttribLocation(program, "a_position");
		a_texCoord_ = glGetAttribLocation(program, "a_texCoord");
		u_mvp_matrix_ = glGetUniformLocation(program, "u_mvp_matrix");
		u_textureSampler_ = glGetUniformLocation(program, "u_textureSampler");
		u_alpha_test_ = glGetUniformLocation(program, "u_alpha_test");
	}

	void setAttribValues(RendererResource& res){
		
		RendererInterface& renderer = res.getRenderer();
		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);										CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getTexname());									CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_, tex_target);										CHECK_GL_ERROR();
		glUniform1f(u_alpha_test_, renderer.getAlphatestValue());						CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_position_);											CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_texCoord_);											CHECK_GL_ERROR();

		glUniformMatrix4fvT(u_mvp_matrix_, 1, true, res.getMvpMatrix(), 0);					CHECK_GL_ERROR();

		glVertexAttribPointer(a_position_, 4, GL_FLOAT, false, 0, res.getVertices());	CHECK_GL_ERROR();
		glVertexAttribPointer(a_texCoord_, 2, GL_FLOAT, false, 0, res.getTexcoords());	CHECK_GL_ERROR();
	}

	BaseShader():ShaderDecorator(BASESHADER){

		setFuncNameForVertexshader("doBaseVertexWork");
		setFuncNameForFragmentshader("applyBase");
		setFuncBodyForVertexshader(
			string("//Base Vertex Block\n") +
			"attribute vec4 a_position;\n" +
			"attribute vec2 a_texCoord;" +
			"uniform mat4 u_mvp_matrix;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"void doBaseVertexWork(){\n" +
			"        v_texCoord = a_texCoord;\n" +
			"        gl_Position = a_position * u_mvp_matrix;\n" +
			"}"
			);
		setFuncBodyForFragmentshader(
			string("varying highp vec2 v_texCoord;\n") +
			"uniform sampler2D u_textureSampler;\n" +
			"uniform highp float u_alpha_test;\n" +
			"highp vec4 applyBase(){\n" +
			"        highp vec4 color;\n" +
			"        color = (texture2D(u_textureSampler, v_texCoord));\n" +
			"        if(color.a < u_alpha_test) return vec4(0,0,0,0);\n" +
			"        return color;\n" +
			"}"
			);
	}
};

#ifndef __APPLE__
class BaseShaderExternal:public ShaderDecorator{

	int a_position_;
	int a_texCoord_;
	int u_textureSampler_;
	int u_tex_matrix_;
	int u_mvp_matrix_;
	int u_alpha_test_;
	
public:
	virtual ShaderDecorator* clone(){

		return new BaseShaderExternal();
	}

	void getAttribIndex(int program){
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "%s %d", __func__, __LINE__);
		a_position_ = glGetAttribLocation(program, "a_position");
		a_texCoord_ = glGetAttribLocation(program, "a_texCoord");
		u_mvp_matrix_ = glGetUniformLocation(program, "u_mvp_matrix");
		u_tex_matrix_ = glGetUniformLocation(program, "u_tex_matrix");
		u_textureSampler_ = glGetUniformLocation(program, "u_textureSampler");
		u_alpha_test_ = glGetUniformLocation(program, "u_alpha_test");
	}

	void setAttribValues(RendererResource& res){
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "%s %d", __func__, __LINE__);
		RendererInterface& renderer = res.getRenderer();
		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);										CHECK_GL_ERROR();
		//glBindTexture(GL_TEXTURE_2D, res.getTexname());									CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, res.getTexname());						CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_, tex_target);										CHECK_GL_ERROR();
		glUniform1f(u_alpha_test_, renderer.getAlphatestValue());						CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_position_);											CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_texCoord_);											CHECK_GL_ERROR();

		glUniformMatrix4fvT(u_mvp_matrix_, 1, true, res.getMvpMatrix(), 0);					CHECK_GL_ERROR();
		glUniformMatrix4fvT(u_tex_matrix_, 1, true, res.getTexMatrix(), 0);					CHECK_GL_ERROR();

		glVertexAttribPointer(a_position_, 4, GL_FLOAT, false, 0, res.getVertices());	CHECK_GL_ERROR();
		glVertexAttribPointer(a_texCoord_, 2, GL_FLOAT, false, 0, res.getTexcoords());	CHECK_GL_ERROR();
	}

	BaseShaderExternal():ShaderDecorator(BASESHADER_EXTERNAL){
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "%s %d", __func__, __LINE__);
		setFuncNameForVertexshader("doBaseVertexWork");
		setFuncNameForFragmentshader("applyBase");
		setFuncBodyForVertexshader(
			string("//Base Vertex Block\n") +
			"attribute vec4 a_position;\n" +
			"attribute vec2 a_texCoord;" +
			"uniform mat4 u_mvp_matrix;\n" +
			"uniform mat4 u_tex_matrix;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"void doBaseVertexWork(){\n" +
			"        v_texCoord = (vec4(a_texCoord.x, a_texCoord.y, 1, 1) * u_tex_matrix).xy;\n" +
			"        gl_Position = a_position * u_mvp_matrix;\n" +
			"}"
			);
		setFuncBodyForFragmentshader(
			string("#extension GL_OES_EGL_image_external : require\n") +
			"precision mediump float;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"uniform samplerExternalOES u_textureSampler;\n" +
			"uniform highp float u_alpha_test;\n" +
			"highp vec4 applyBase(){\n" +
			"        highp vec4 color;\n" +
			"        color = (texture2D(u_textureSampler, v_texCoord));\n" +
			"        if(color.a < u_alpha_test) return vec4(0,0,0,0);\n" +
			"        return color;\n" +
			"}"
			);
	}
};
#endif
