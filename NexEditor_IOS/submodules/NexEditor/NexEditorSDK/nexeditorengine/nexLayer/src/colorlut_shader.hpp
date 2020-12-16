#pragma once
#ifdef _ANDROID
#include "NexTheme_Config.h"
#else
#import "NexTheme_Config.h"
#endif

class ColorLutShader:public ShaderDecorator{

	int u_textureSampler_for_lut_;
	int u_strength_for_lut_;

public:
	virtual ShaderDecorator* clone(){

		return new ColorLutShader();
	}

	void getAttribIndex(int program){

		u_textureSampler_for_lut_ = glGetUniformLocation(program, "u_textureSampler_for_lut");
		u_strength_for_lut_ = glGetUniformLocation(program, "u_strength_for_lut");
	}

	void setAttribValues(RendererResource& res){

		int tex_target = res.getTexTarget();
		glActiveTexture(GL_TEXTURE0 + tex_target);                                CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, res.getRenderer().getLUT());                                   CHECK_GL_ERROR();
		glUniform1i(u_textureSampler_for_lut_, tex_target);                       CHECK_GL_ERROR();
		glUniform1f(u_strength_for_lut_, 1.0f);                              CHECK_GL_ERROR();
	}
#ifndef USE_FRAGMENT_SHADER_LUT_64x4096
	ColorLutShader():ShaderDecorator(COLORLUTSHADER){

		setFuncNameForVertexshader("");
		setFuncNameForFragmentshader("applyLUT");
		setFuncBodyForVertexshader("");
		setFuncBodyForFragmentshader(
			string("const highp float block_factor = 64.0;\n") +
			"\tuniform sampler2D u_textureSampler_for_lut;\n" +
			"\tuniform highp float u_strength_for_lut;\n" +
			"\n" +
			"\thighp vec4 applyLUT(highp vec4 color) {\n" +
			"\t\thighp float block = (block_factor - 1.0) * color.b;\n" +
			"\t\thighp float low = floor(block);\n" +
			"\t\thighp float high = ceil(block);\n" +
			"\t\thighp float y0 = mod(low, 8.0);\n" +
			"\t\thighp float x0 = (low - y0) / 8.0;\n" +
			"\t\thighp float y1 = mod(high, 8.0);\n" +
			"\t\thighp float x1 = (high - y1) / 8.0;\n" +
			"\t\tx0 /= 8.0;\n" +
			"\t\ty0 /= 8.0;\n" +
			"\t\tx1 /= 8.0;\n" +
			"\t\ty1 /= 8.0;\n" +
			"\t\thighp vec2 rg_pos = (63.0 / 512.0) * color.rg + 0.5 / 512.0;\n" +
			"\t\thighp vec4 color0 = texture2D(u_textureSampler_for_lut, vec2(x0, y0) + rg_pos.yx).rgba;\n" +
			"\t\thighp vec4 color1 = texture2D(u_textureSampler_for_lut, vec2(x1, y1) + rg_pos.yx).rgba;\n" +
			"\t\treturn vec4(mix(color.rgb, mix(color0, color1, fract(block)).rgb, u_strength_for_lut).rgb * color.a, color.a);\n" +
			"\t}"
			);
	}
#else
    ColorLutShader():ShaderDecorator(COLORLUTSHADER){
        setFuncNameForVertexshader("");
        setFuncNameForFragmentshader("applyLUT");
        setFuncBodyForVertexshader("");
        setFuncBodyForFragmentshader(
            string("const highp float block_factor = 64.0;\n") +
            "\tuniform sampler2D u_textureSampler_for_lut;\n" +
            "\tuniform highp float u_strength_for_lut;\n" +
            "\n" +
            "\thighp vec4 applyLUT(highp vec4 color) {\n" +
            "\t\thighp float block = (block_factor - 1.0) * color.b;\n" +
            "\t\tcolor.r = (62.0 / 64.0) * color.r + 1.0 / 64.0;\n" +
            "\t\thighp float t0 = 1.0 - (floor(block) + color.r) / block_factor;\n" +
            "\t\thighp float t1 = 1.0 - (ceil(block) + color.r) / block_factor;\n" +
            "\t\thighp vec4 color0 = texture2D(u_textureSampler_for_lut, vec2(color.g, t0)).rgba;\n" +
            "\t\thighp vec4 color1 = texture2D(u_textureSampler_for_lut, vec2(color.g, t1)).rgba;\n" +
            "\t\treturn vec4(mix(color.bgr, mix(color0, color1, fract(block)).bgr, u_strength_for_lut).bgr * color.a, color.a);\n" +
            "\t}"
            );
    }
#endif
};
