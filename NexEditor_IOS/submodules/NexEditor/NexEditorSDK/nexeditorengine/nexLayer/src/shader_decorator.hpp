#pragma once
//BaseLine Shader(Color Feeder)
#define PREVIEWMODE		0x00000000
#define EXPORTMODE		0x80000000

#define BASELINEBITCOUNT	8
#define BASELINEFILTER	0x000000FF
#define BASELINECHECKER	0x00000080

#define BASESHADER			0x00000000
#define BASESHADER_EXTERNAL	0x00000001
#define HBLURSHADER			0x00000010
#define VBLURSHADER			0x00000020
#define MOSAICSHADER		0x00000030
#define COLORCONVSHADER		0x00000100
#define OVERLAYSHADER		0x00000200
#define CHROMAKEYSHADER		0x00000400
#define MASKSHADER			0x00000800
#define COLORLUTSHADER		0x00001000
#define USERALPHASHADER		0x00002000





#define SHADERSTRMDLMTR	0xFFFFFFFF

class ShaderDecorator{

	ShaderDecorator* decoratee_;

	unsigned int attrib_flag_;
	string func_name_;
	string func_name_for_vertexshader_;
	string func_name_for_fragmentshader_;
	string func_body_for_vertexshader_;
	string func_body_for_fragmentshader_;

public:
	ShaderDecorator(unsigned int attrib_flag):attrib_flag_(attrib_flag){

	}

    virtual ~ShaderDecorator() {}
    
	virtual ShaderDecorator* clone() = 0;

	unsigned int getAttrib(){

		return attrib_flag_;
	}

	virtual void setCopyValue(ShaderDecorator* reference){

	}

	void setFuncNameForVertexshader(string name){

		func_name_for_vertexshader_ = name;
	}

	void setFuncNameForFragmentshader(string name){

		func_name_for_fragmentshader_ = name;
	}

	void setFuncBodyForVertexshader(string body){

		func_body_for_vertexshader_ = body;
	}

	void setFuncBodyForFragmentshader(string body){
		func_body_for_fragmentshader_ = body;
	}

	string getFuncNameForVertexshader(){

		return func_name_for_vertexshader_;
	}

	string getFuncNameForFragmentshader(){

		return func_name_for_fragmentshader_;
	}

	string getFuncBodyForVertexshader(){

		return func_body_for_vertexshader_;
	}

	string getFuncBodyForFragmentshader(){

		return func_body_for_fragmentshader_;
	}

	void decorate(ShaderDecorator* decoratee){

		decoratee_ = decoratee;
	}

	string getCompForVertexshader(){

		string ret = getFuncNameForVertexshader();

		if(ret != "")
			ret += "();";

		if(decoratee_ == NULL){

			return ret;
		}

		ret = decoratee_->getCompForVertexshader() + ret;
		return ret;
	}

	string getCompForFragmentshader(){

		string ret = getFuncNameForFragmentshader();

		if(ret != ""){

			string inner = decoratee_?decoratee_->getCompForFragmentshader():"";
			ret = ret + "(" + inner + ")";
		}
		return ret;
	}

	string makeVertexBody(){

		return (decoratee_?decoratee_->makeVertexBody():"") + getFuncBodyForVertexshader();
	}

	string makeVertexshader(){

		return makeVertexBody() +
			"\nvoid main(){\n" + getCompForVertexshader() + "\n}\n";
	}

	string makeFragmentBody(){

		return (decoratee_?decoratee_->makeFragmentBody():"") + getFuncBodyForFragmentshader();
	}

	string makeFragmentshader(){

		string ret = makeFragmentBody() +
			"\n"+
			// "uniform highp float alpha_test_value_;\n" +
			"void main(){\n" +
			"highp vec4 color = " + getCompForFragmentshader() + ";\n"+
			"if(color.a > 0.0) gl_FragColor = color;\n" +
			"else discard;\n" +
			"}\n";

		return ret;
	}

	virtual void getAttribIndex(int program){

	}

	void makeAttribIndex(int program){

		getAttribIndex(program);
		if(decoratee_)
			decoratee_->makeAttribIndex(program);
	}

	virtual void setAttribValues(RendererResource& res){

	}

	void feedAttribValues(RendererResource& res){

		setAttribValues(res);
		if(decoratee_)
			decoratee_->feedAttribValues(res);
	}

};

void glUniformMatrix4fvT(int location, int count, bool transpose, float* matrix, int offset) {
	if (transpose) {

		float tmp_matrix[16];
		Matrix::transposeM(tmp_matrix, 0, matrix, 0);
		glUniformMatrix4fv(location, count, false, tmp_matrix);
	}
	else {
		glUniformMatrix4fv(location, count, false, matrix);
	}
}
