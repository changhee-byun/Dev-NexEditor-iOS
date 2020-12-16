#include <string>
#include "util.h"
#include "glext_loader.h"
#include "rapidxml.hpp"
#include "NexSAL_Internal.h"

static const char* getGLErrorString(int error) {

	switch (error) {
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	default: return "?";
	}
}

void CHECK_GL_ERROR() {

	int err;
	while ((err = glGetError()) != GL_NO_ERROR) {

		nexSAL_TraceCat(0, 0, "GL_ERROR:%s", getGLErrorString(err));
	}
}



rapidxml::xml_node<>* getNode(rapidxml::xml_node<>* parent_node, const char* node_type, const char* attrib_id, const char* name){

	rapidxml::xml_node<>* node = parent_node->first_node(node_type);

	if(attrib_id && name){

		while(node){

			rapidxml::xml_attribute<>* attrib = node->first_attribute(attrib_id);

			if(attrib && !strcmp(attrib->value(), name)){

				return node;
			}
			node = node->next_sibling();
		}	
	}
	else
		return node;

	return NULL;
}

rapidxml::xml_node<>* getNextNode(rapidxml::xml_node<>* prev_node, const char* node_type, const char* attrib_id, const char* name){

	rapidxml::xml_node<>* node = prev_node->next_sibling(node_type);

	if (attrib_id && name){

		while (node){

			rapidxml::xml_attribute<>* attrib = node->first_attribute(attrib_id);

			if (attrib && !strcmp(attrib->value(), name)){

				return node;
			}
			node = node->next_sibling();
		}
	}
	else
		return node;

	return NULL;
}

char* getAttribStr(rapidxml::xml_node<>* node, const char* attrib_id){

	rapidxml::xml_attribute<>* attrib = node->first_attribute(attrib_id);
	if(!attrib)
		return NULL;
	return attrib->value();
}

rapidxml::xml_attribute<>* getFirstAttribChain(rapidxml::xml_node<>* node){

	rapidxml::xml_attribute<>* attrib = node->first_attribute();
	if (!attrib)
		return NULL;
	return attrib;
}

rapidxml::xml_node<>* getCDataNode(rapidxml::xml_node<>* node){

	rapidxml::xml_node<>* data_node = node->first_node();

	while (data_node){

		if (data_node->type() == rapidxml::node_cdata){

			return data_node;
		}

		data_node = data_node->next_sibling();
	}
	return NULL;
}

void readFromFile(const char* filename, std::string& buf, bool finalize){

	FILE* fp = NULL;
	fp = fopen(filename, "rt");
	if (!fp)
		return;
	while (!feof(fp)){

		char c = fgetc(fp);
		if ((char)EOF == c) {

			if (finalize)
				buf.push_back(0);
			break;
		}

		buf.push_back(c);
	}
	fclose(fp);
}

#include <algorithm>

struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '\\' || ch == '/';
	}
};

std::string basename(std::string const& pathname){

	return std::string(
		std::find_if(pathname.rbegin(), pathname.rend(),
		MatchPathSeparator()).base(),
		pathname.end());
}

std::string dirname(const std::string& pathname){

	std::string copy_str(pathname);

	copy_str.erase(std::find_if(copy_str.rbegin(), copy_str.rend(),
		MatchPathSeparator()).base(),
		copy_str.end());

	return copy_str;
}

const char* get_current_dir_name_mine(void){

	static char dirname[260];

#ifdef DEMONSTRATION

	GetCurrentDirectoryA(sizeof(dirname), dirname);

#endif//DEMONSTRATION

	return dirname;
}

#ifdef DEMONSTRATION
#include <direct.h>
int Chdir(const char* dir){

	return chdir(dir);

}
#else

int Chdir(const char* dir){

	return 0;
}
#endif

#define IS_FLOAT_CHAR(xx) (((xx)>='0'&&(xx)<='9')||(xx)=='.'||(xx)=='-')
#define IS_WHITESPACE(x) ( (x)==' ' || (x)=='\t' || (x)=='\n' )

RITM_TFU RITM::NXT_ParseBool(const char* string) {
    if( strcasecmp(string,"")==0  || strcasecmp(string,"on")==0 || strcasecmp(string,"yes")==0 || strcasecmp(string,"true")==0 || strcasecmp(string,"1")==0 ) {
        return RITM_TFUTrue;
    } else if( strcasecmp(string,"no")==0 || strcasecmp(string,"off")==0 || strcasecmp(string,"0")==0 || strcasecmp(string,"false")==0 ) {
        return RITM_TFUFalse;
    } else {
        return RITM_TFUUnknown;
    }
}

int RITM::NXT_PartialFloatVectorFromString( const char** string, float* vector, int numElements ) {
    // Vector may be NULL, in which case we just compute and return
    // the number of elements up to the maximum of numElements
    if( !string || !*string) {
        return 0;
    }
    
    int elementsFound = 0;
    
    const char *s = *string;
    while( *s && elementsFound<numElements ) {
        float sign=1.0;
        float result=0.0;
        while( *s && IS_WHITESPACE(*s) )
            s++;
        if( *s=='#' ) {
            unsigned int value=0;
            int digits=0;
            s++;
            for(;;) {
                if( *s >= '0' && *s <='9' ) {
                    value *= 0x10;
                    value += (*s-'0');
                } else if( *s >= 'a' && *s <='z' ) {
                    value *= 0x10;
                    value += (*s-'a'+0xA);
                } else if( *s >= 'A' && *s <='Z' ) {
                    value *= 0x10;
                    value += (*s-'A'+0xA);
                } else {
                    break;
                }
                s++;
                digits++;
            }
            if( digits==3 ) {
                while( digits > 0 ) {
                    int el = (value >> ((digits-1)*4))&0xF;
                    if( elementsFound<numElements ) {
                        if( vector ) {
                            *vector = (float)el / (float)15.0;
                            vector++;
                        }
                        elementsFound++;
                    }
                    digits -= 1;
                }
            } else {
                digits += (digits % 2); // Round up to an even number
                while( digits > 0 ) {
                    int el = (value >> ((digits-2)*4))&0xFF;
                    if( elementsFound<numElements ) {
                        if( vector ) {
                            *vector = (float)el / (float)255.0;
                            vector++;
                        }
                        elementsFound++;
                    }
                    digits -= 2;
                }
            }
            continue;
        }
        if( *s && *s!=',' && !IS_FLOAT_CHAR(*s) )
            break;
        if( *s=='-' ) {
            sign = -sign;
            s++;
        }
        while( *s>='0' && *s<='9' ) {
            result = (result*10.0) + (float)(*s - '0');
            s++;
        }
        if( *s=='.' ) {
            s++;
            float fct = 0.1;
            while( *s>='0' && *s<='9' ) {
                result = result + (fct * (float)(*s - '0'));
                fct /= 10.0;
                s++;
            }
        }
        if( vector ) {
            *vector = sign*result;
            vector++;
        }
        elementsFound++;
        while( *s && IS_WHITESPACE(*s) )
            s++;
        if( *s && *s==',' )
            s++;
        while( *s && IS_WHITESPACE(*s) )
            s++;
    }
    
    *string = s;
    return elementsFound;
}

int RITM::NXT_FloatVectorFromString(const char* string, float* vector, int numElements) {

	int elementsFound = 0;
	int i;

	if (!string) {
		return 0;
	}

	const char *s = string;
	while (*s && elementsFound<numElements) {
		float sign = 1.0;
		float result = 0.0;
		while (*s && !IS_FLOAT_CHAR(*s))
			s++;
		if (*s == '-') {
			sign = -sign;
			s++;
		}
		while (*s >= '0' && *s <= '9') {
			result = (result*10.0f) + (float)(*s - '0');
			s++;
		}
		if (*s == '.') {
			s++;
			float fct = 0.1f;
			while (*s >= '0' && *s <= '9') {
				result = result + (fct * (float)(*s - '0'));
				fct /= 10.0;
				s++;
			}
		}
		*vector = sign*result;
		vector++;
		elementsFound++;
	}

	for (i = elementsFound; i<numElements; i++) {
		if (elementsFound == 3)
			*vector = 1.0;
		else
			*vector = 0.0;
		vector++;
	}

	return elementsFound;
}

AutoRetDirectory::AutoRetDirectory(const char* dir){

	std::string cwd(get_current_dir_name_mine());
	Chdir(dir);
}

AutoRetDirectory::~AutoRetDirectory(){

	Chdir(cwd_.c_str());
}

const std::string& getPrototypeForSetter(int type){

	static std::map<int, std::string> setter_prototype;

	if (setter_prototype.empty()){

		setter_prototype[GL_FLOAT] = std::string(
			"function %class_name.%setter_name(val)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetUniform1f(program.%class_name.%uniform_name, val)\n"
			"end\n"
			);

        setter_prototype[GL_INT] = std::string(
            "function %class_name.%setter_name(val)\n"
            "   kmUseProgram(program.%class_name.id)\n"
            "   kmSetUniform1i(program.%class_name.%uniform_name, val)\n"
            "end\n"
            );

		setter_prototype[GL_FLOAT_VEC2] = std::string(
			"function %class_name.%setter_name(val0, val1)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetUniform2f(program.%class_name.%uniform_name, val0, val1)\n"
			"end\n"
			);

		setter_prototype[GL_FLOAT_VEC3] = std::string(
			"function %class_name.%setter_name(val0, val1, val2)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetUniform3f(program.%class_name.%uniform_name, val0, val1, val2)\n"
			"end\n"
			);

		setter_prototype[GL_FLOAT_VEC4] = std::string(
			"function %class_name.%setter_name(val0, val1, val2, val3)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetUniform4f(program.%class_name.%uniform_name, val0, val1, val2, val3)\n"
			"end\n"
			);

		setter_prototype[GL_FLOAT_MAT4] = std::string(
			"function %class_name.%setter_name(val)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetUniformMat4(program.%class_name.%uniform_name, val)\n"
			"end\n"
			);
		setter_prototype[GL_SAMPLER_2D] = std::string(
			"function %class_name.%setter_name(val)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetTexture(%tex_target_id, val, program.%class_name.%uniform_name)\n"
			"end\n"
			);
#ifdef ANDROID
		setter_prototype[GL_SAMPLER_EXTERNAL_OES] = std::string(
			"function %class_name.%setter_name(val)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmSetTextureExternal(%tex_target_id, val, program.%class_name.%uniform_name)\n"
			"end\n"
			);
#endif
	}

	return setter_prototype[type];
}

const strstrmapper_t& getPrototypeForFunction(){

	static strstrmapper_t function_prototype;

	if (function_prototype.empty()){

		function_prototype["drawRect"] = std::string(
			"function %class_name.%setter_name(val0, val1)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawRect(program.%class_name.a_position, program.%class_name.a_texCoord, val0, val1)\n"
			"end\n"
			);

		function_prototype["drawBound"] = std::string(
			"function %class_name.%setter_name(val0, val1, val2, val3)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawBound(program.%class_name.a_position, program.%class_name.a_texCoord, val0, val1, val2, val3)\n"
			"end\n"
			);

		function_prototype["drawRectPos"] = std::string(
			"function %class_name.%setter_name(val0, val1, val2, val3)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawRectWithPos(program.%class_name.a_position, program.%class_name.a_texCoord, val0, val1, val2, val3)\n"
			"end\n"
			);

		function_prototype["drawArray"] = std::string(
			"function %class_name.%setter_name(val0, val1, val2, val3, val4)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawArray(program.%class_name.a_position, program.%class_name.a_texCoord, val0, val1, val2, val3, val4)\n"
			"end\n"
			);
		function_prototype["drawElement"] = std::string(
			"function %class_name.%setter_name(val0, val1, val2, val3, val4, val5)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawElement(program.%class_name.a_position, program.%class_name.a_texCoord, val0, val1, val2, val3, val4, val5)\n"
			"end\n"
			);
		function_prototype["drawSphere"] = std::string(
			"function %class_name.%setter_name()\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawSphere(program.%class_name.a_position, program.%class_name.a_texCoord)\n"
			"end\n"
			);
		function_prototype["drawScreen"] = std::string(
			"function %class_name.%setter_name(val0)\n"
			"	kmUseProgram(program.%class_name.id)\n"
			"	kmDrawScreen(val0, program.%class_name.u_mvp_matrix, program.%class_name.a_position, program.%class_name.a_texCoord, effect.matrix, effect.left, effect.top, effect.right, effect.bottom)\n"
			"end\n"
			);
        function_prototype["drawScreenDirect"] = std::string(
            "function %class_name.%setter_name(val0)\n"
            "   kmUseProgram(program.%class_name.id)\n"
            "   kmDrawScreenDirect(val0, program.%class_name.u_mvp_matrix, program.%class_name.a_position, program.%class_name.a_texCoord, effect.matrix, effect.left, effect.top, effect.right, effect.bottom)\n"
            "end\n"
            );
	}

	return function_prototype;
}

#ifndef DEMONSTRATION

#include <sys/time.h>

unsigned int getTickCount(){

	struct timeval gettick;
	unsigned int tick;
	int ret;
	gettimeofday(&gettick, NULL);

	tick = gettick.tv_sec * 1000 + gettick.tv_usec / 1000;

	return tick;

}
#else
unsigned int getTickCount(){

	return GetTickCount();

}
#endif//DEMONSTRATION
