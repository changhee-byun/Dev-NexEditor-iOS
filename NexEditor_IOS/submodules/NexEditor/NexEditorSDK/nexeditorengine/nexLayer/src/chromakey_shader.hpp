#pragma once

class ChromakeyShader:public ShaderDecorator{

	int u_keyvalue_;
	int u_division_;
	int u_strength_;
	int u_mask_view_flag;
public:
	virtual ShaderDecorator* clone(){

		return new ChromakeyShader();
	}

	void getAttribIndex(int program){
		
		u_keyvalue_ = glGetUniformLocation(program, "u_keyvalue");
		u_division_ = glGetUniformLocation(program, "u_division");
		u_strength_ = glGetUniformLocation(program, "u_strength");
		u_mask_view_flag = glGetUniformLocation(program, "CKMaskOnOff");
	}

	void setAttribValues(RendererResource& res){
	
		ChromakeyInfo& chromakey_info = res.getRenderer().getChromakeyColor();
		glUniform3f(u_keyvalue_, chromakey_info.r, chromakey_info.g, chromakey_info.b);
		glUniform4f(u_division_, chromakey_info.di0, chromakey_info.di1, chromakey_info.di2, chromakey_info.di3);
		glUniform3f(u_strength_, chromakey_info.s1, chromakey_info.s2, chromakey_info.s3);
		glUniform1i(u_mask_view_flag, res.getRenderer().getChromakeyViewMaskEnabled());
	}

	ChromakeyShader() :ShaderDecorator(CHROMAKEYSHADER){

		setFuncNameForVertexshader("");
		setFuncNameForFragmentshader("applyChromakey");
		setFuncBodyForVertexshader("");
		setFuncBodyForFragmentshader(
			string("uniform highp vec3 u_keyvalue;\n") +
			"uniform highp vec3 u_strength;\n" +
			"uniform highp vec4 u_division;\n" +
			"uniform int CKMaskOnOff;\n" +
			"highp float proportionalstep(highp float edge0, highp float edge1, highp float x){\n" +
			"x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);\n" +
			"return x;\n" +
			"}\n" +
			"highp vec4 applyChromakey(highp vec4 color){\n" +
			"const highp mat4 rgbyuv = mat4(\n" +
			"\t\t\t0.300, 0.589, 0.111, -0.003,\n" +
			"\t\t\t-0.169, -0.332, 0.502, 0.502,\n" +
			"\t\t\t0.499, -0.420, -0.079, 0.502,\n" +
			"\t\t\t0.000, 0.000, 0.000, 1.000);\n" +
			"const highp mat4 yuvrgb = mat4(\n" +
			"\t\t\t1.000, 0.000, 1.402, -0.701,\n" +
			"\t\t\t1.000, -0.334, -0.714, 0.529,\n" +
			"\t\t\t1.000, 1.772, 0.000, -0.886,\n" +
			"\t\t\t0.000, 0.000, 0.000, 1.000);\n" +
			"highp vec4 yuv = color * rgbyuv;\n" +
			"highp vec4 yuv_key = vec4(u_keyvalue, 1.0) * rgbyuv;\n" +
			"highp float radius = length(vec2(yuv.gb) - vec2(yuv_key.gb));\n" +
			"highp float alpha = proportionalstep(u_division.x, u_division.y, radius) * u_strength.x\n" +
			"+ proportionalstep(u_division.y, u_division.z, radius) * u_strength.y\n" +
			"+ proportionalstep(u_division.z, u_division.w, radius) * u_strength.z;\n" +
			"if(CKMaskOnOff == 1){\n" +
			"color = color * 0.000001 + vec4(alpha, alpha, alpha, 1.0);}\n" +
			"else{\n" +
			"color = color * alpha;}\n" +
			"\t\treturn color;\n" +
			"\t}");
	}
};
