#pragma once

class SimpleRenderInterface{

	typedef vector<ShaderDecorator*> decorator_list;
	decorator_list decorator_list_;
	
	int program_;
	unsigned int attrib_flag_;

	int loadShader(int type, const string& shaderCode){

		int shader = 0;
		//GLint size_for_shader = 0;
		shader = glCreateShader(type);
		const char* pshader_str = shaderCode.c_str();
		glShaderSource(shader, 1, &pshader_str, NULL);
		glCompileShader(shader);
		GLchar tmp_error[1024];
		int error_length = 0;
		glGetShaderInfoLog(shader, sizeof(tmp_error) - 1, &error_length, tmp_error);
        if(error_length > 0)
		    LOGE("%s type:%d result:%s", __func__, type, tmp_error);
		return shader;
	}
public:
	SimpleRenderInterface() :program_(0), attrib_flag_(0x0){

	}

	virtual ~SimpleRenderInterface(){

		for (decorator_list::iterator itor = decorator_list_.begin(); itor != decorator_list_.end(); ++itor){

			ShaderDecorator* decorator = *itor;
			delete decorator;
		}
	}

	void addDecorator(ShaderDecorator* decorator){

		decorator_list_.push_back(decorator);
		attrib_flag_ = 0x0;
	}

	unsigned int getAttribFlag(){

		if (attrib_flag_ == 0x0){

			for (decorator_list::iterator itor = decorator_list_.begin(); itor != decorator_list_.end(); ++itor){

				ShaderDecorator* decorator = *itor;
				attrib_flag_ |= decorator->getAttrib();
			}
		}

		return attrib_flag_;
	}

	void releaseResource(){

		glDeleteProgram(program_);
		program_ = 0;
	}
	
	void createProgram(){

		ShaderDecorator* master_decorator = decorator_list_.back();
		int vertex_shader = loadShader(GL_VERTEX_SHADER, master_decorator->makeVertexshader());
		int fragment_shader = loadShader(GL_FRAGMENT_SHADER, master_decorator->makeFragmentshader());
		program_ = glCreateProgram();
		glAttachShader(program_, vertex_shader);
		glAttachShader(program_, fragment_shader);
		glLinkProgram(program_);

		glDetachShader(program_, vertex_shader);
		glDetachShader(program_, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		char error_text[1024];
		GLsizei length = 0;
		glGetProgramInfoLog(program_, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
		LOGE("%s program:%d result:%s", __func__, program_, error_text);
		master_decorator->makeAttribIndex(program_);
	}

	void draw(RendererResource& resource) {

		if (0 == program_)
			createProgram();
		ShaderDecorator* master_decorator = decorator_list_.back();
		glUseProgram(program_);
		master_decorator->feedAttribValues(resource);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
	}
};
