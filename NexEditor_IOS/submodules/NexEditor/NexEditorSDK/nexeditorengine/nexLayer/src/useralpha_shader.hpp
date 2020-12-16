#pragma once

class UserAlphaShader:public ShaderDecorator{

	int u_alpha_;
public:
	virtual ShaderDecorator* clone(){

		return new UserAlphaShader();
	}

	void getAttribIndex(int program){
		
		u_alpha_ = glGetUniformLocation(program, "u_alpha");
	}

	void setAttribValues(RendererResource& res){

		glUniform1f(u_alpha_, res.getRenderer().getAlpha());
	}

	UserAlphaShader():ShaderDecorator(USERALPHASHADER){

		setFuncNameForVertexshader("");
		setFuncNameForFragmentshader("applyUserAlpha");
		setFuncBodyForVertexshader("");
		setFuncBodyForFragmentshader(
			string("uniform highp float u_alpha;\n") +
			"highp vec4 applyUserAlpha(highp vec4 color){\n" +
			"        return color * u_alpha;\n" +
			"}");
	}
};