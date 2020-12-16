#pragma once

class BlurShaderHorizontal:public ShaderDecorator{

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

		return new BlurShaderHorizontal();
	}

	void getAttribIndex(int program){

			a_position_ = glGetAttribLocation(program, "a_position");
			a_texCoord_ = glGetAttribLocation(program, "a_texCoord");
			u_mvp_matrix_ = glGetUniformLocation(program, "u_mvp_matrix");
			u_textureSampler_ = glGetUniformLocation(program, "u_textureSampler");
			u_maskSampler_ = glGetUniformLocation(program, "u_maskSampler");
			u_alpha_test_ = glGetUniformLocation(program, "u_alpha_test");
			u_block_size_ = glGetUniformLocation(program, "u_block_size");
			u_texture_size_ = glGetUniformLocation(program, "u_texture_size");
	}

	void setAttribValues(RendererResource& res){

		RendererInterface& renderer = res.getRenderer();
		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);			CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getTexname());				CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_, tex_target);			CHECK_GL_ERROR();
		tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);			CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, renderer.getMaskTexID());			CHECK_GL_ERROR();
		glUniform1i(u_maskSampler_, tex_target);			CHECK_GL_ERROR();
		glUniform1f(u_alpha_test_, renderer.getAlphatestValue());		CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_position_);				CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_texCoord_);				CHECK_GL_ERROR();

		glUniformMatrix4fvT(u_mvp_matrix_, 1, true, res.getMvpMatrix(), 0);						CHECK_GL_ERROR();

		glVertexAttribPointer(a_position_, 4, GL_FLOAT, false, 0, res.getVertices());		CHECK_GL_ERROR();
		glVertexAttribPointer(a_texCoord_, 2, GL_FLOAT, false, 0, res.getTexcoords());        CHECK_GL_ERROR();

		glUniform2f(u_block_size_, renderer.getEffectStrength() * 0.2f, renderer.getEffectStrength() * 0.2f);	CHECK_GL_ERROR();
		glUniform2f(u_texture_size_, (float)renderer.getEffectTextureWidth(), (float)renderer.getEffectTextureHeight());		CHECK_GL_ERROR();
	}

	BlurShaderHorizontal():ShaderDecorator(HBLURSHADER){

		setFuncNameForVertexshader("doBaseVertexWork");
		setFuncNameForFragmentshader("applyBlurBase");
		setFuncBodyForVertexshader(
			string("attribute vec4 a_position;\n") +
			"attribute vec2 a_texCoord;" +
			"uniform mat4 u_mvp_matrix;\n" +
			"uniform highp vec2 u_texture_size;\n" +
			"uniform highp vec2 u_block_size;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"varying highp vec2 v_blurTexCoords[14];\n" +
			"void doBaseVertexWork(){\n" +
			"       v_texCoord = a_texCoord;\n" +
			"       v_blurTexCoords[ 0] = v_texCoord + vec2(-7.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 1] = v_texCoord + vec2(-6.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 2] = v_texCoord + vec2(-5.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 3] = v_texCoord + vec2(-4.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 4] = v_texCoord + vec2(-3.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 5] = v_texCoord + vec2(-2.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 6] = v_texCoord + vec2(-1.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 7] = v_texCoord + vec2( 1.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 8] = v_texCoord + vec2( 2.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[ 9] = v_texCoord + vec2( 3.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[10] = v_texCoord + vec2( 4.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[11] = v_texCoord + vec2( 5.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[12] = v_texCoord + vec2( 6.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       v_blurTexCoords[13] = v_texCoord + vec2( 7.0 / u_texture_size.x * u_block_size.x,0.0);\n" +
			"       gl_Position = a_position * u_mvp_matrix;\n" +
			"}"
			);
		setFuncBodyForFragmentshader(

			string("varying highp vec2 v_texCoord;\n") +
			"varying highp vec2 v_blurTexCoords[14];\n" +
			"uniform highp vec2 u_texture_size;\n" +
			"uniform sampler2D u_textureSampler;\n" +
			"uniform sampler2D u_maskSampler;\n" +
			"uniform highp vec2 u_block_size;\n" +
			"uniform highp float u_alpha_test;\n" +
			"highp vec4 getTexColor(highp vec2 uv){\n" +
			"highp float dx = u_block_size.x * (1.0 / u_texture_size.x);\n" +
			"highp float dy = u_block_size.y * (1.0 / u_texture_size.y);\n" +
			"highp vec4 color = vec4(0.0);\n" +
			"highp vec2 coord = vec2(dx*floor(uv.x / dx) + 1. / u_texture_size.x, dy*floor(uv.y / dy) + 1. / u_texture_size.y);\n" +
			"color.rgba = (texture2D(u_textureSampler, coord));\n" +
			"return color;\n" +
			"}\n" +

			"highp vec4 applyBlurBase(){\n" +
			"       highp vec4 color;\n" +
			"       highp vec4 mask = texture2D(u_maskSampler, v_texCoord);\n" +
			"       if(mask.x == 0.0) return vec4(0, 0, 0, 0);\n" +
			"       color.bgra = getTexColor(v_texCoord) * 0.159576912161;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[ 0])*0.0044299121055113265;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[1])*0.00895781211794;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[2])*0.0215963866053;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[3])*0.0443683338718;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[4])*0.0776744219933;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[5])*0.115876621105;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[6])*0.147308056121;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[7])*0.147308056121;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[8])*0.115876621105;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[9])*0.0776744219933;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[10])*0.0443683338718;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[11])*0.0215963866053;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[12])*0.00895781211794;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[13])*0.0044299121055113265;\n" +
			"        if(color.a < u_alpha_test) return vec4(0,0,0,0);\n" +
			"        return color.bgra;\n" +
			"}\n"
			);
	}
};

class BlurShaderVertical:public ShaderDecorator{

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

		return new BlurShaderVertical();
	}

	void getAttribIndex(int program){
			
		a_position_ = glGetAttribLocation(program, "a_position");
		a_texCoord_ = glGetAttribLocation(program, "a_texCoord");
		u_mvp_matrix_ = glGetUniformLocation(program, "u_mvp_matrix");
		u_textureSampler_ = glGetUniformLocation(program, "u_textureSampler");
		u_maskSampler_ = glGetUniformLocation(program, "u_maskSampler");
		u_alpha_test_ = glGetUniformLocation(program, "u_alpha_test");
		u_block_size_ = glGetUniformLocation(program, "u_block_size");
		u_texture_size_ = glGetUniformLocation(program, "u_texture_size");
	}

	void setAttribValues(RendererResource& res){

		RendererInterface& renderer = res.getRenderer();

		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);			CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getTexname());				CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_, tex_target);			CHECK_GL_ERROR();
		tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);			CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, renderer.getMaskTexID());			CHECK_GL_ERROR();
		glUniform1i(u_maskSampler_, tex_target);			CHECK_GL_ERROR();
		glUniform1f(u_alpha_test_, renderer.getAlphatestValue());		CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_position_);				CHECK_GL_ERROR();
		glEnableVertexAttribArray(a_texCoord_);				CHECK_GL_ERROR();

		glUniformMatrix4fvT(u_mvp_matrix_, 1, true, res.getMvpMatrix(), 0);											CHECK_GL_ERROR();

		glVertexAttribPointer(a_position_, 4, GL_FLOAT, false, 0, res.getVertices());							CHECK_GL_ERROR();
		glVertexAttribPointer(a_texCoord_, 2, GL_FLOAT, false, 0, res.getTexcoords());							CHECK_GL_ERROR();

		glUniform2f(u_block_size_, renderer.getEffectStrength() * 0.2f, renderer.getEffectStrength() * 0.2f);	CHECK_GL_ERROR();
		glUniform2f(u_texture_size_, (float)renderer.getEffectTextureWidth(), (float)renderer.getEffectTextureHeight());		CHECK_GL_ERROR();
	}

	BlurShaderVertical():ShaderDecorator(VBLURSHADER){

		setFuncNameForVertexshader("doBaseVertexWork");
		setFuncNameForFragmentshader("applyBlurBase");
		setFuncBodyForVertexshader(

			string("attribute vec4 a_position;\n") +
			"attribute vec2 a_texCoord;" +
			"uniform mat4 u_mvp_matrix;\n" +
			"uniform highp vec2 u_block_size;\n" +
			"uniform highp vec2 u_texture_size;\n" +
			"varying highp vec2 v_texCoord;\n" +
			"varying highp vec2 v_blurTexCoords[14];\n" +
			"void doBaseVertexWork(){\n" +
			"       v_texCoord = a_texCoord;\n" +
			"       v_blurTexCoords[ 0] = v_texCoord + vec2(0.0, -7.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -6.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 2] = v_texCoord + vec2(0.0, -5.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -4.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 4] = v_texCoord + vec2(0.0, -3.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 5] = v_texCoord + vec2(0.0, -2.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 6] = v_texCoord + vec2(0.0, -1.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 7] = v_texCoord + vec2(0.0,  1.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 8] = v_texCoord + vec2(0.0,  2.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[ 9] = v_texCoord + vec2(0.0,  3.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[10] = v_texCoord + vec2(0.0,  4.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[11] = v_texCoord + vec2(0.0,  5.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[12] = v_texCoord + vec2(0.0,  6.0 / u_texture_size.y * u_block_size.y);\n" +
			"       v_blurTexCoords[13] = v_texCoord + vec2(0.0,  7.0 / u_texture_size.y * u_block_size.y);\n" +
			"       gl_Position = a_position * u_mvp_matrix;\n" +
			"}"
			);

		setFuncBodyForFragmentshader(

			string("varying highp vec2 v_texCoord;\n") +
			"varying highp vec2 v_blurTexCoords[14];\n" +
			"uniform highp vec2 u_texture_size;\n" +
			"uniform sampler2D u_textureSampler;\n" +
			"uniform sampler2D u_maskSampler;\n" +
			"uniform highp vec2 u_block_size;\n" +
			"uniform highp float u_alpha_test;\n" +
			"highp vec4 getTexColor(highp vec2 uv){\n" +
			"highp float dx = u_block_size.x * (1.0 / u_texture_size.x);\n" +
			"highp float dy = u_block_size.y * (1.0 / u_texture_size.y);\n" +
			"highp vec4 color = vec4(0.0);\n" +
			"highp vec2 coord = vec2(dx*floor(uv.x / dx) + 1. / u_texture_size.x, dy*floor(uv.y / dy) + 1. / u_texture_size.y);\n" +
			"color.rgba = (texture2D(u_textureSampler, coord));\n" +
			"return color;\n" +
			"}\n" +

			"highp vec4 applyBlurBase(){\n" +
			"       highp vec4 color;\n" +
			"       highp vec4 mask = texture2D(u_maskSampler, v_texCoord);\n" +
			"       if(mask.x == 0.0) return vec4(0, 0, 0, 0);\n" +
			"       color.bgra = getTexColor(v_texCoord) * 0.159576912161;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[ 0])*0.0044299121055113265;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[1])*0.00895781211794;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[2])*0.0215963866053;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[3])*0.0443683338718;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[4])*0.0776744219933;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[5])*0.115876621105;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[6])*0.147308056121;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[7])*0.147308056121;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[8])*0.115876621105;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[9])*0.0776744219933;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[10])*0.0443683338718;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[11])*0.0215963866053;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[12])*0.00895781211794;\n" +
			"       color.bgra += getTexColor(v_blurTexCoords[13])*0.0044299121055113265;\n" +
			"        if(color.a < u_alpha_test) return vec4(0,0,0,0);\n" +
			"        return color.rgba;\n" +
			"}\n"
			);
	}
};