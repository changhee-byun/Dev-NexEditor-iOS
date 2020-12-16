#pragma once

class MaskShader:public ShaderDecorator{
	
	int u_textureSampler_for_mask_;

public:
	virtual ShaderDecorator* clone(){

		return new MaskShader();
	}

	void getAttribIndex(int program){
	
		u_textureSampler_for_mask_ = glGetUniformLocation(program, "u_textureSampler_for_mask");
	}

	void setAttribValues(RendererResource& res){
		
		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);                                CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getRenderer().getMaskTexID());                                   CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_for_mask_, tex_target);                      CHECK_GL_ERROR();
	}

	MaskShader() :ShaderDecorator(MASKSHADER){

		setFuncNameForVertexshader("maskWork");
		setFuncNameForFragmentshader("applyMask");
		setFuncBodyForVertexshader(

			string("\nvarying highp vec2 v_texCoord_for_mask;\n") +
			"void maskWork() {\n" +
			"v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;\n" +
			"}\n");
		setFuncBodyForFragmentshader(

			string("\nvarying highp vec2 v_texCoord_for_mask;\n") +
			"uniform sampler2D u_textureSampler_for_mask;\n" +
			"highp vec4 applyMask(highp vec4 color) {\n" +
			"highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n" +
			"color *= mask.r;\n" +
			"return color;\n" +
			"}\n");
	}
};