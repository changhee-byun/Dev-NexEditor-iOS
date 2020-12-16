#pragma once

#include "glext_loader.h"
#include "lua.hpp"
#include <vector>
#include <map>
#include <string>

#ifndef SAFE_FREE
#define SAFE_FREE(x)	if(x){ free(x); (x) = NULL;}
#endif//SAFE_FREE

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	{if(x){ (x)->release(); (x) = NULL;}}
#endif//SAFE_RELEASE

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	{if(x){ delete (x); (x) = NULL;}}
#endif//SAFE_DELETE

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(x) {if(x){ (x)->addRef();}}
#endif//SAFE_ADDREF

struct texInfo{
	int width_;
	int height_;
	int src_width_;
	int src_height_;
};

struct tagConverter{

	std::string target_;
	std::string name_;
};

class Bin{

	int ref_;
	int life_;
public:
	Bin() :ref_(0), life_(0){

	}

	virtual ~Bin(){


	}

	void release(){

		decRef();
	}

	void addRef(){

		++ref_;
	}

	void decRef(){

		--ref_;
	}

	int getRef(){

		return ref_;
	}

	int decLife(){

		--life_;
		return life_;
	}

	int getLife(){

		return life_;
	}

	void setLife(int life){

		life_ = life;
	}
};

class TextureBin :public Bin	{

	GLuint tex_id_;
	int w_;
	int h_;
public:
	TextureBin() :tex_id_(0){

		setLife(5);
	}

	~TextureBin(){

		if (tex_id_ > 0)
			glDeleteTextures(1, &tex_id_);
	}

	GLuint getTexID(){

		return tex_id_;
	}

	void setTexID(GLuint tex_id){

		tex_id_ = tex_id;
	}

	void setWidth(int w){

		w_ = w;
	}

	int getWidth(){

		return w_;
	}

	void setHeight(int h){

		h_ = h;
	}

	int getHeight(){

		return h_;
	}
};

class ShaderBin :public Bin {

	GLuint id_;
public:
	ShaderBin() :id_(0){

		setLife(5);
	}

	~ShaderBin(){

		if (id_ > 0)
			glDeleteShader(id_);
	}

	void set(GLuint id){

		id_ = id;
	}

	GLuint get(){

		return id_;
	}
};

class ProgramBin :public Bin{

	GLuint program_;
	ShaderBin* vertex_shader_;
	ShaderBin* fragment_shader_;
public:
	ProgramBin() :program_(0), vertex_shader_(NULL), fragment_shader_(NULL){

		setLife(5);
	}

	~ProgramBin(){

		if (program_ > 0){

			if (vertex_shader_){

				glDetachShader(program_, vertex_shader_->get());
			}

			if (fragment_shader_){

				glDetachShader(program_, fragment_shader_->get());
			}
			glDeleteProgram(program_);

			SAFE_RELEASE(vertex_shader_);
			SAFE_RELEASE(fragment_shader_);
		}
	}

	void set(ShaderBin* vertex_shader, ShaderBin* fragment_shader, GLuint program){

		program_ = program;
		SAFE_RELEASE(vertex_shader_);
		SAFE_RELEASE(fragment_shader_);
		vertex_shader_ = vertex_shader;
		fragment_shader_ = fragment_shader;
		SAFE_ADDREF(vertex_shader);
		SAFE_ADDREF(fragment_shader);
	}

	GLuint get(){

		return program_;
	}
};

class LuaBin :public Bin{

	size_t size_;
	char* data_;
public:
	LuaBin() :data_(NULL), size_(0){

	}

	LuaBin(const char* data, size_t size){

		data_ = new char[size];
		for (size_t i = 0; i < size; ++i){

			data_[i] = data[i];
		}
		size_ = size;
	}

	~LuaBin(){

		SAFE_DELETE(data_);
	}

	char* get(){

		return data_;
	}

	void set(const char* data, size_t size){

		SAFE_DELETE(data_);
		data_ = new char[size];
		data_[size - 1] = 0;
		for (size_t i = 0; i < size; ++i){

			data_[i] = data[i];
		}
		size_ = size;
	}

	size_t getSize(){

		return size_;
	}
};

typedef std::map<std::string, GLuint> glResmap_t;
typedef std::map<std::string, ProgramBin*> glProgrammap_t;
typedef std::map<std::string, TextureBin*> glTexmap_t;
typedef std::map<GLuint, texInfo> glTexInfomap_t;
typedef std::map<GLuint, glResmap_t> glProgramIndexmap_t;
typedef std::map<std::string, int> glDataIdxMapper_t;
typedef std::vector<std::vector<float> > glFloatDataList_t;
typedef std::vector<std::vector<GLushort> > glUShortDataList_t;
typedef std::map<std::string, std::string> strstrmapper_t;

class Listener{
public:
	Listener();

	virtual ~Listener();

	virtual void doWork(lua_State* lua_state, std::string& value);
};


typedef std::vector<Listener*> listenerlist_t;
typedef std::map<std::string, std::string> valuemap_t;
typedef std::map<std::string, listenerlist_t> listenermap_t;