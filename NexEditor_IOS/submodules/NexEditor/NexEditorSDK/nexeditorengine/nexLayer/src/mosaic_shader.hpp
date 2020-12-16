#pragma once

class MosaicShader:public ShaderDecorator {

	int a_position_;
	int a_texCoord_;
	int u_textureSampler_;
	int u_maskSampler_;
	int u_mvp_matrix_;
	int u_alpha_test_;
	int u_block_size_;
	int u_texture_size_;
public:
	virtual ShaderDecorator* clone(){

		return new MosaicShader();
	}

	void getAttribIndex(int program) {

		a_position_ = glGetAttribLocation(program, "a_position");
		a_texCoord_ = glGetAttribLocation(program, "a_texCoord");
		u_mvp_matrix_ = glGetUniformLocation(program, "u_mvp_matrix");
		u_textureSampler_ = glGetUniformLocation(program, "u_textureSampler");
		u_maskSampler_ = glGetUniformLocation(program, "u_maskSampler");
		u_alpha_test_ = glGetUniformLocation(program, "u_alpha_test");
		u_block_size_ = glGetUniformLocation(program, "u_block_size");
		u_texture_size_ = glGetUniformLocation(program, "u_texture_size");
	}

	void setAttribValues(RendererResource& res) {

		RendererInterface& renderer = res.getRenderer();
		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);            CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getTexname());            CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_, tex_target);            CHECK_GL_ERROR();

		tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);            CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, renderer.getMaskTexID());            CHECK_GL_ERROR();
		glUniform1i(u_maskSampler_, tex_target);            CHECK_GL_ERROR();

		glUniform1f(u_alpha_test_, renderer.getAlphatestValue());
		glEnableVertexAttribArray(a_position_);            CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_texCoord_);            CHECK_GL_ERROR();

		glUniformMatrix4fvT(u_mvp_matrix_, 1, true, res.getMvpMatrix(), 0);            CHECK_GL_ERROR();

		glVertexAttribPointer(a_position_, 4, GL_FLOAT, false, 0, res.getVertices());            CHECK_GL_ERROR();
		glVertexAttribPointer(a_texCoord_, 2, GL_FLOAT, false, 0, res.getTexcoords());            CHECK_GL_ERROR();

		glUniform2f(u_block_size_, renderer.getEffectStrength(), renderer.getEffectStrength());
		glUniform2f(u_texture_size_, (float)renderer.getEffectTextureWidth(), (float)renderer.getEffectTextureHeight());
	}

	MosaicShader():ShaderDecorator(MOSAICSHADER){

		setFuncNameForVertexshader("doMosaicBaseVertexWork");
		setFuncNameForFragmentshader("mosaicBase");
		setFuncBodyForVertexshader(

			string("//Base Vertex Block\n") +
			"attribute vec4 a_position;\n" +
			"attribute vec2 a_texCoord;" +
			"uniform mat4 u_mvp_matrix;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"void doMosaicBaseVertexWork(){\n" +
			"        v_texCoord = a_texCoord;\n" +
			"        gl_Position = a_position * u_mvp_matrix;\n" +
			"}"
			);
		setFuncBodyForFragmentshader(
			
			string("varying highp vec2 v_texCoord;\n") +
			"uniform sampler2D u_textureSampler;\n" +
			"uniform sampler2D u_maskSampler;\n" +
			"uniform highp float u_alpha_test;\n" +
			"uniform highp vec2 u_block_size;\n" +
			"uniform highp vec2 u_texture_size;\n" +
			"highp vec4 mosaicBase(){\n" +
			"   highp vec2 uv = v_texCoord;\n" +
			"   highp vec4 mask = texture2D(u_maskSampler, uv);\n" +
			"   if(mask.x == 0.0) return vec4(0, 0, 0, 0);\n" +
			"   highp float dx = u_block_size.x * (1.0 / u_texture_size.x);\n" +
			"   highp float dy = u_block_size.y * (1.0 / u_texture_size.y);\n" +
			"   highp vec2 coord = vec2(dx*floor(uv.x / dx) + 1. / u_texture_size.x, dy*floor(uv.y / dy) + 1. / u_texture_size.y);\n" +
			"   highp vec4 color = texture2D(u_textureSampler, coord);\n" +
			"   if(color.a < u_alpha_test) return vec4(0,0,0,0);\n" +
			"   return color;\n" +
			"}"
			);
	}
};