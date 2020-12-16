#pragma once

class ColorConvShader:public ShaderDecorator{

	int u_colorconv_;
	int u_hue_;

public:
	virtual ShaderDecorator* clone(){

		return new ColorConvShader();
	}

	void getAttribIndex(int program){

		u_colorconv_ = glGetUniformLocation(program, "u_colorconv");
		u_hue_ = glGetUniformLocation(program, "u_hue");
	}

	void setAttribValues(RendererResource& res){

		RendererInterface& renderer = res.getRenderer();
		glUniformMatrix4fvT(u_colorconv_, 1, false, res.getColorMatrix(), 0);		CHECK_GL_ERROR();
		glUniform1f(u_hue_, renderer.getHue() /180);								CHECK_GL_ERROR();
	}

	ColorConvShader() :ShaderDecorator(COLORCONVSHADER){

		setFuncNameForVertexshader("");
		setFuncNameForFragmentshader("applyColorConv");
		setFuncBodyForVertexshader("");
		setFuncBodyForFragmentshader(
			string("uniform highp mat4 u_colorconv;\n") +
			"uniform highp float u_hue;\n" +

			"highp vec3 rgb2hsv(highp vec3 c)\n" +
			"{\n" +
			"	highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n" +
			"	highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n" +
			"	highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n" +

			"	highp float d = q.x - min(q.w, q.y);\n" +
			"	highp float e = 1.0e-10;\n" +
			"	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n" +
			"}\n" +


			"highp vec3 hsv2rgb(highp vec3 c)\n" +
			"{\n" +
			"	highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n" +
			"	highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n" +
			"	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n" +
			"}\n" +

			"highp vec4 applyHue(highp vec4 color) {\n" +

			"	highp vec3 hsv = rgb2hsv(color.rgb);\n" +
			"	hsv.r += u_hue;\n" +
			"	color = vec4(hsv2rgb(hsv).rgb, color.a);\n" +
			"	return color;\n" +
			"}\n" +

			"highp vec4 applyColorConv(highp vec4 color){\n" +
			"		color = color * u_colorconv;\n" +
			"		color = clamp(color, 0.0, 1.0);\n" +
			"		color = applyHue(color);\n" +
			"		return color;\n" +
			"}"
			);
	}
};

class OverlayShader:public ShaderDecorator{

	int u_overlaycolor_;
public:
	virtual ShaderDecorator* clone(){

		return new OverlayShader();
	}

	void getAttribIndex(int program){

		u_overlaycolor_ = glGetUniformLocation(program, "u_overlaycolor");
	}

	
	void setAttribValues(RendererResource& res){

		glUniform4fv(u_overlaycolor_, 1, res.getOverlaycolor());
	}

	OverlayShader() :ShaderDecorator(OVERLAYSHADER){

		setFuncNameForVertexshader("");
		setFuncNameForFragmentshader("applyOverlay");
		setFuncBodyForVertexshader("");
		setFuncBodyForFragmentshader(
			string("//Overlay Block\n") +
			"uniform highp vec4 u_overlaycolor;\n" +
			"highp vec4 applyOverlay(highp vec4 color){\n" +
			"        color.rgb = color.rgb * (1.0 - u_overlaycolor.a) + u_overlaycolor.rgb * color.a;\n" +
			"        return color;\n" +
			"}"
			);
	}
};