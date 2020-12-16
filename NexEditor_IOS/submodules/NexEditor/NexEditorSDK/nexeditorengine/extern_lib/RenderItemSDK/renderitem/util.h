#pragma once

#include <string>
#include "rapidxml.hpp"
#include "ResourceManager.h"
#include "types.h"

//#if defined(WIN32) || defined(_WIN64) 
#if defined(USE_GLEXT_LOADER)
//#define snprintf _snprintf 
//#define vsnprintf _vsnprintf 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp 
#else
#include <string.h>
#include <strings.h>
#endif

std::string basename(std::string const& pathname);

std::string dirname(const std::string& pathname);

const char* get_current_dir_name_mine(void);

int Chdir(const char* dir);

void CHECK_GL_ERROR();

rapidxml::xml_node<>* getNode(rapidxml::xml_node<>* parent_node, const char* node_type, const char* attrib_id, const char* name);

rapidxml::xml_node<>* getNextNode(rapidxml::xml_node<>* prev_node, const char* node_type, const char* attrib_id, const char* name);

rapidxml::xml_attribute<>* getFirstAttribChain(rapidxml::xml_node<>* node);

char* getAttribStr(rapidxml::xml_node<>* node, const char* attrib_id);

rapidxml::xml_node<>* getCDataNode(rapidxml::xml_node<>* node);

void readFromFile(const char* filename, std::string& buf, bool finalize = false);

typedef enum {
    RITM_TFUTrue = 1, RITM_TFUFalse = 2, RITM_TFUUnknown = 0
} RITM_TFU;

namespace RITM{

	int NXT_PartialFloatVectorFromString( const char** string, float* vector, int numElements );

	int NXT_FloatVectorFromString(const char* string, float* vector, int numElements);

	RITM_TFU NXT_ParseBool(const char* string);
};

struct AutoRetDirectory{

	AutoRetDirectory(const char* dir);
	~AutoRetDirectory();

private:
	std::string cwd_;
};

#ifndef AUTO_RET_DIR
#define AUTO_RET_DIR(x)	AutoRetDirectory __KINE_RET__(x)
#endif//AUTO_RET_DIR

template<typename T>
inline void readVtxData(glDataIdxMapper_t& vertex_data_map_, std::vector<std::vector<T> >& vertex_data_list_, std::string& key, std::string& buf, std::vector<T>& vertexbuf){

	std::size_t pos = 0;
	while (1){
		std::size_t new_pos = 0;
		if (pos)
			new_pos = buf.find(",", ++pos);
		else
			new_pos = buf.find(",");
		std::size_t len = new_pos - pos;
		if (new_pos == std::string::npos){
			len = buf.size() - pos;
		}
		char cbuf[32];
		buf.copy(cbuf, len, pos);
		cbuf[len] = 0;
		pos = new_pos;
		vertexbuf.push_back((T)atof(cbuf));
		if (new_pos == std::string::npos){

			vertex_data_list_.push_back(vertexbuf);
			vertex_data_map_.insert(make_pair(key, vertex_data_list_.size() - 1));
			break;
		}
	}
}

template<>
inline void readVtxData<GLushort>(glDataIdxMapper_t& vertex_data_map_, std::vector<std::vector<GLushort> >& vertex_data_list_, std::string& key, std::string& buf, std::vector<GLushort>& vertexbuf){

	std::size_t pos = 0;
	while (1){
		std::size_t new_pos = 0;
		if (pos)
			new_pos = buf.find(",", ++pos);
		else
			new_pos = buf.find(",");
		std::size_t len = new_pos - pos;
		if (new_pos == std::string::npos){
			len = buf.size() - pos;
		}
		char cbuf[32];
		buf.copy(cbuf, len, pos);
		cbuf[len] = 0;
		pos = new_pos;
		vertexbuf.push_back((GLushort)atoi(cbuf));
		if (new_pos == std::string::npos){

			vertex_data_list_.push_back(vertexbuf);
			vertex_data_map_.insert(make_pair(key, vertex_data_list_.size() - 1));
			break;
		}
	}
}

const std::string& getPrototypeForSetter(int type);
const strstrmapper_t& getPrototypeForFunction();

template<typename T>
class Factory{

	typedef typename std::map<int, T*> prototype_t;
	typedef typename std::map<int, T*>::iterator iterator_t;

	prototype_t prototype_;
public:
	T* create(int type){

		return prototype_[type]->clone();
	}

	void set(int type, T* proto){

		SAFE_DELETE(prototype_[type]);
		prototype_[type] = proto;
	}

	~Factory(){

		releaseAll();
	}

	void releaseAll(){

		for(iterator_t itor = prototype_.begin(); itor != prototype_.end(); ++itor)
			delete itor->second;

		prototype_.clear();
	}
};

unsigned int getTickCount();