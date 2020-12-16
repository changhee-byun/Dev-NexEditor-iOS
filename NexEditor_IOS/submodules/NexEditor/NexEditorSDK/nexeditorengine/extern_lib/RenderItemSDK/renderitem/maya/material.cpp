#include "../types.h"
#include "material.h"
#include "../util.h"
#include "../ResourceManager.h"
#include "../img.h"
#include "../RenderItem.h"
#include "NexSAL_Internal.h"

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define OPACITY_CHUNK_HEADER	0xFADEDACE
#define STDMAT_V11_PARAM_HEADER	0xACEEFFEC
#define AMBIENT_CHUNK_HEADER	0xAAAEFFEC
#define DIFFUSE_CHUNK_HEADER	0xAABEFFEC
#define SPECULAR_CHUNK_HEADER	0xAACEFFEC
#define SPECLEVEL_CHUNK_HEADER	0xAADEFFEC
#define GLOSSINESS_CHUNK_HEADER	0xAAEEFFEC


Basic3DMaterial::Basic3DMaterial(){


}

Basic3DMaterial::~Basic3DMaterial(){

	
}

bool Basic3DMaterial::deserialize(InputStream& stream){

	char resource_name[256];
	
	stream.read(resource_name, sizeof(char), sizeof(resource_name) - 1);
	resource_name[strlen(resource_name)] = 0;

	std::string key(resource_name);
	
	ResourceManager<TextureBin>& manager = stream.getContext()->texture_manager_;
	
	diffuse_map_ = manager.get(key);

	if (NULL == diffuse_map_){

		float w(0), h(0);
		diffuse_map_ = new TextureBin();
		diffuse_map_->setTexID(Img::loadImage(key.c_str(), &w, &h));
		diffuse_map_->setWidth((int)w);
		diffuse_map_->setHeight((int)h);

		manager.insert(key, diffuse_map_);
	}

	return true;
}

bool Basic3DMaterial::deserialize(InputStream& stream, char* uid, void *prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

	char resource_name[256];
	
	stream.read(resource_name, sizeof(char), sizeof(resource_name) - 1);
	resource_name[strlen(resource_name)] = 0;

	std::string key(resource_name);
	
    (*pAquireContextFunc)(prenderer);
	ResourceManager<TextureBin>& manager = stream.getContext()->texture_manager_;
	diffuse_map_ = manager.get(key);

	if (NULL == diffuse_map_){

		float w(0), h(0);
		diffuse_map_ = new TextureBin();
		diffuse_map_->setTexID(Img::loadImage(key.c_str(), &w, &h));
		diffuse_map_->setWidth((int)w);
		diffuse_map_->setHeight((int)h);
		manager.insert(key, diffuse_map_);
        (*pReleaseContextFunc)(prenderer);
	}
    else{

        (*pReleaseContextFunc)(prenderer);
    }

	return true;
}


#define TEXTURE_CHUNK	0xeffeda01

TextureChunk::TextureChunk() :idx_(0), size_of_compressed_(0), size_of_uncompressed_(0)
{

}

TextureChunk::~TextureChunk()
{
	free();
}

void TextureChunk::free()
{
	idx_ = 0;
	size_of_compressed_ = 0;
	size_of_uncompressed_ = 0;
}

#include <zlib.h>
#include "../soil/SOIL.h"


bool TextureChunk::deserialize(InputStream& stream)
{
	free();
	unsigned int check_type(0);
	stream.read(&check_type, sizeof(check_type), 1);
	if (check_type != TEXTURE_CHUNK)
	{
		stream.seek(-4, SEEK_CUR);
		return false;
	}
	stream.read(&idx_, sizeof(idx_), 1);
	int video_place_holder = 0;
	stream.read(&video_place_holder, sizeof(video_place_holder), 1);
	stream.read(&size_of_compressed_, sizeof(size_of_compressed_), 1);
	stream.read(&size_of_uncompressed_, sizeof(size_of_uncompressed_), 1);

	unsigned char* pbyte_for_compress(NULL);
	unsigned char* pbyte_for_tex(NULL);

	pbyte_for_compress = (unsigned char*)malloc(size_of_compressed_);
	pbyte_for_tex = (unsigned char*)malloc(size_of_uncompressed_);

	stream.read(pbyte_for_compress, sizeof(BYTE), size_of_compressed_);

	char filename[256];
	snprintf(filename, sizeof(filename), "%d", idx_);
	
	TextureBin* image = NULL;
	ResourceManager<TextureBin>& manager = stream.getContext()->texture_manager_;
	image = manager.get(std::string(filename));

	if (NULL == image){

		uLong size_of_uncompressed = (uLong)size_of_uncompressed_;

		int zret = uncompress(pbyte_for_tex, (uLongf *)&size_of_uncompressed, pbyte_for_compress, size_of_compressed_);
		int w, h, channels;
		unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)pbyte_for_tex, size_of_uncompressed_, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);

		nexSAL_TraceCat(0, 0, "[%s %d] filename:%s tex_id:%d w:%d h:%d", __func__, __LINE__, filename, tex_id, w, h);

		image = new TextureBin();
		image->setTexID(tex_id);
		image->setWidth(w);
		image->setHeight(h);
		image->setChannels(channels);
		image->setPlaceHolderFlag(video_place_holder);
		manager.insert(std::string(filename), image);
	}

	::free(pbyte_for_tex);
	::free(pbyte_for_compress);

	return true;
}

bool TextureChunk::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void *p))
{
	free();
	unsigned int check_type(0);
	stream.read(&check_type, sizeof(check_type), 1);
	if (check_type != TEXTURE_CHUNK)
	{
		stream.seek(-4, SEEK_CUR);
		return false;
	}
	stream.read(&idx_, sizeof(idx_), 1);
	int video_place_holder = 0;
	stream.read(&video_place_holder, sizeof(video_place_holder), 1);
	stream.read(&size_of_compressed_, sizeof(size_of_compressed_), 1);
	stream.read(&size_of_uncompressed_, sizeof(size_of_uncompressed_), 1);

	unsigned char* pbyte_for_compress(NULL);
	unsigned char* pbyte_for_tex(NULL);

	pbyte_for_compress = (unsigned char*)malloc(size_of_compressed_);
	pbyte_for_tex = (unsigned char*)malloc(size_of_uncompressed_);

	stream.read(pbyte_for_compress, sizeof(BYTE), size_of_compressed_);

	char filename[256];
	snprintf(filename, sizeof(filename), "%d", idx_);
	
	TextureBin* image = NULL;
    (*pAquireContextFunc)(prenderer);
	ResourceManager<TextureBin>& manager = stream.getContext()->texture_manager_;
	image = manager.get(std::string(filename));

	if (NULL == image){

        image = new TextureBin();
        manager.insert(std::string(filename), image);
        (*pReleaseContextFunc)(prenderer);

		uLong size_of_uncompressed = (uLong)size_of_uncompressed_;

		int zret = uncompress(pbyte_for_tex, (uLongf *)&size_of_uncompressed, pbyte_for_compress, size_of_compressed_);
		int w, h, channels;
        (*pAquireContextFunc)(prenderer);
		unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)pbyte_for_tex, size_of_uncompressed_, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);
        (*pReleaseContextFunc)(prenderer);

		nexSAL_TraceCat(0, 0, "[%s %d] filename:%s tex_id:%d w:%d h:%d", __func__, __LINE__, filename, tex_id, w, h);

		
		image->setTexID(tex_id);
		image->setWidth(w);
		image->setHeight(h);
		image->setChannels(channels);
		image->setPlaceHolderFlag(video_place_holder);
		
	}
    else{

        (*pReleaseContextFunc)(prenderer);
    }

	::free(pbyte_for_tex);
	::free(pbyte_for_compress);

	return true;
}

Lightmap::Lightmap()
{
	enable_ = FALSE;
	color_ = Vector3(0, 0, 0);
}

void Lightmap::deserialize(InputStream& stream)
{	
	stream.read(&enable_, sizeof(enable_), 1);
	stream.read(&color_, sizeof(color_), 1);

	char temp[MAX_ITEM_LEN] = { 0, };
	stream.read(temp, sizeof(temp), MAX_ITEM_LEN);
}

void Lightmap::deserialize(InputStream& stream, char* uid, void *prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *prenderer))
{	
	stream.read(&enable_, sizeof(enable_), 1);
	stream.read(&color_, sizeof(color_), 1);

	char temp[MAX_ITEM_LEN] = { 0, };
	stream.read(temp, sizeof(temp), MAX_ITEM_LEN);
}

Lightmap::~Lightmap()
{
}

#define GLSL(shader) #shader

MatSolidColor_Shader::~MatSolidColor_Shader(){

	glDeleteProgram(program_idx);
}

MatSolidColor_Shader::MatSolidColor_Shader(){

	const GLchar vtx_shader[] = GLSL(

		attribute vec3 a_position; \n
		attribute vec2 a_uv; \n
		attribute vec3 a_color; \n

		uniform mat4 u_mvp_matrix; \n
		
		varying highp vec3 v_color; \n
		varying highp vec2 v_uv; \n

		void main() {\n
			
			v_color = a_color;
			v_uv = a_uv;
			gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1); \n
		}
	);

	const GLchar frg_shader[] = GLSL(

		varying highp vec3 v_color;\n
		varying highp vec2 v_uv; \n
		uniform sampler2D u_textureSampler; \n
		
		void main(void){\n

			gl_FragColor = texture2D(u_textureSampler, v_uv) * vec4(
				smoothstep(0.08, 0.20, v_color.r),
				smoothstep(0.00, 0.10, v_color.g) * smoothstep(0.20, 0.10, v_color.g),
				smoothstep(0.12, 0.00, v_color.b),
				1.0f); \n
		}
	);


	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLint size_for_vtx = sizeof(vtx_shader);
	GLint size_for_frg = sizeof(frg_shader);
	const GLchar* pvtx = vtx_shader;
	const GLchar* pfrg = frg_shader;

	glShaderSource(vertex_shader, 1, &pvtx, &size_for_vtx);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &pfrg, &size_for_frg);
	glCompileShader(fragment_shader);

	GLuint program = glCreateProgram(); CHECK_GL_ERROR();
	glAttachShader(program, vertex_shader); CHECK_GL_ERROR();
	glAttachShader(program, fragment_shader); CHECK_GL_ERROR();
	glLinkProgram(program); CHECK_GL_ERROR();

	char error_text[1024];
	GLsizei length = 0;
	glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	program_idx = program;

	vertex_attrib = glGetAttribLocation(program, "a_position");
	uv_attrib = glGetAttribLocation(program, "a_uv");
	color_attrib = glGetAttribLocation(program, "a_color");
	texture_uniform = glGetUniformLocation(program_idx, "u_textureSampler");
	mvp_matrix_uniform = glGetUniformLocation(program_idx, "u_mvp_matrix");
}

MatSolidColor::MatSolidColor(MatSolidColor_Shader& shader, int origin_flag):shader_(shader), ptex_(NULL), origin_flag_(origin_flag){

	type_ = MAT_SOLID_COLOR;
}

MatSolidColor::MatSolidColor(const MatSolidColor* mtl):shader_(mtl->shader_), origin_flag_(0){

}

MatA_Shader::~MatA_Shader(){

	glDeleteProgram(program_idx);
}

MatA_Shader::MatA_Shader(){

	const GLchar vtx_shader[] = 
		GLSL(

		attribute vec3 a_position; \n
		attribute vec2 a_texCoord; \n
		attribute vec3 a_normal; \n
		// attribute vec3 a_tangent; \n
		// attribute vec3 a_binormal; \n

		uniform mat4 u_world_matrix; \n
		uniform mat4 u_mvp_matrix; \n
		uniform vec3 u_light_dir; \n
		uniform vec3 u_eye_dir; \n

		varying highp vec3 v_light; \n
		varying highp vec3 v_eye; \n
		varying highp vec2 v_texCoord; \n

		void main() {\n
			// vec3 tangent = mat3(u_world_matrix) * a_tangent; \n
			// vec3 normal = mat3(u_world_matrix) * a_normal; \n
			// vec3 binormal = mat3(u_world_matrix) * a_binormal; \n

			v_texCoord = a_texCoord; \n

			// mat3 tangentconv = mat3(
			// 		tangent.x,binormal.x,normal.x,
			// 		tangent.y,binormal.y,normal.y,
			// 		tangent.z,binormal.z,normal.z
			// 		);

			// v_light = tangentconv * u_light_dir; \n
			// v_eye = tangentconv * u_eye_dir; \n
			v_light = u_light_dir; \n
			v_eye = u_eye_dir; \n
			gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1); \n
		}
	);

	const GLchar frg_shader[] = 
		GLSL(

		varying highp vec2 v_texCoord; \n
		varying highp vec3 v_eye; \n
		varying highp vec3 v_light; \n

		uniform sampler2D u_textureSampler; \n
		uniform sampler2D u_specSampler; \n
		uniform sampler2D u_normalSampler; \n

		

		void main(void){\n

			highp vec2 texcoord = v_texCoord; \n
			highp vec4 origin_color = texture2D(u_textureSampler, texcoord); \n
			highp vec4 color = origin_color;
			highp vec3 normal = texture2D(u_normalSampler, texcoord).rgb;\n
			highp vec3 spec = texture2D(u_specSampler, texcoord).rgb;\n

			normal = normalize(2.0 * normal.rgb - 1.0);
			highp vec3 Rvec = reflect(v_eye, normal);
			highp float spcLight = (1.0 + dot(v_light, Rvec)) * 0.5;
			spec *= spcLight;

			color.rgb *= clamp(dot(normal, v_light), 0.0, 1.0);
			color.rgb += clamp(spec, 0.0, 1.0);
			gl_FragColor = vec4(color.r, color.g, color.b, color.a); \n
		}
	);


	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLint size_for_vtx = sizeof(vtx_shader);
	GLint size_for_frg = sizeof(frg_shader);
	const GLchar* pvtx = vtx_shader;
	const GLchar* pfrg = frg_shader;

	char error_text[1024] = "no error";
	GLsizei length = 0;

	glShaderSource(vertex_shader, 1, &pvtx, &size_for_vtx);
	glCompileShader(vertex_shader);

	glGetShaderInfoLog(vertex_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] vtx output:%s", __func__, __LINE__, error_text);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &pfrg, &size_for_frg);
	glCompileShader(fragment_shader);

	glGetShaderInfoLog(fragment_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] frg output:%s", __func__, __LINE__, error_text);

	GLuint program = glCreateProgram(); CHECK_GL_ERROR();
	glAttachShader(program, vertex_shader); CHECK_GL_ERROR();
	glAttachShader(program, fragment_shader); CHECK_GL_ERROR();
	glLinkProgram(program); CHECK_GL_ERROR();

	
	glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

	nexSAL_TraceCat(0, 0, "[%s %d] %s program:%d", __func__, __LINE__, error_text, program);

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	program_idx = program;

	vertex_attrib = glGetAttribLocation(program, "a_position");
	texcoord_attrib = glGetAttribLocation(program, "a_texCoord");
	normal_attrib = glGetAttribLocation(program, "a_normal");
	// tangent_attrib = glGetAttribLocation(program, "a_tangent");
	// binormal_attrib = glGetAttribLocation(program, "a_binormal");
	eye_dir_uniform = glGetUniformLocation(program_idx, "u_eye_dir");
	light_dir_uniform = glGetUniformLocation(program_idx, "u_light_dir");
	world_matrix_uniform = glGetUniformLocation(program_idx, "u_world_matrix");
	mvp_matrix_uniform = glGetUniformLocation(program_idx, "u_mvp_matrix");
	texture_uniform = glGetUniformLocation(program_idx, "u_textureSampler");
	spec_uniform = glGetUniformLocation(program_idx, "u_specSampler");
	normal_uniform = glGetUniformLocation(program_idx, "u_normalSampler");
}

MatA::MatA(MatA_Shader& shader, int origin_flag):shader_(shader), origin_flag_(origin_flag)
{
	type_ = MAT_BGFX_A;
	burn_ = 0.0;
	mid_ = 0.0f;
	bias_ = 0.0f;
	sss_color_ = Vector4(0, 0, 0, 0);
	sss_level_ = 0.0f;
	sss_width_ = 0.0f;
	rim_color_ = Vector4(0, 0, 0, 0);
	rim_density_ = 0.0f;
	rim_width_ = 0.0f;
	normal_scale_ = 0.0f;
	spec_color_ = Vector4(0, 0, 0, 0);
	spec_level_ = 0.0f;
	spec_gloss_ = 0.0f;
	brush_color_ = Vector4(0, 0, 0, 0);
	brush_center_ = 0.0f;
	brush_level_ = 0.0f;
	brush_gloss_ = 0.0f;
}

MatA::MatA(const MatA* mtl):shader_(mtl->shader_), origin_flag_(0)
{
}

Material* MatA::clone(){

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
	return new MatA(shader_);
}

void MatA::setEyeDir(float x, float y, float z){

	glUniform3f(shader_.eye_dir_uniform, x, y, z);
}

void MatA::setLightDir(float x, float y, float z){

	glUniform3f(shader_.light_dir_uniform, x, y, z);
}

bool MatA::deserialize(InputStream& stream){

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
	stream.read(&burn_, sizeof(burn_), 1);
	stream.read(&mid_, sizeof(mid_), 1);
	stream.read(&bias_, sizeof(bias_), 1);
	stream.read(&sss_color_, sizeof(sss_color_), 1);
	stream.read(&sss_level_, sizeof(sss_level_), 1);
	stream.read(&sss_width_, sizeof(sss_width_), 1);
	stream.read(&rim_color_, sizeof(rim_color_), 1);
	stream.read(&rim_density_, sizeof(rim_density_), 1);
	stream.read(&rim_width_, sizeof(rim_width_), 1);
	stream.read(&normal_scale_, sizeof(normal_scale_), 1);
	stream.read(&spec_color_, sizeof(spec_color_), 1);
	stream.read(&spec_level_, sizeof(spec_level_), 1);
	stream.read(&spec_gloss_, sizeof(spec_gloss_), 1);
	stream.read(&brush_color_, sizeof(brush_color_), 1);
	stream.read(&brush_center_, sizeof(brush_center_), 1);
	stream.read(&brush_level_, sizeof(brush_level_), 1);
	stream.read(&brush_gloss_, sizeof(brush_gloss_), 1);

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

	RenderItemContext* ctx = stream.getContext();
	RenderItem* prenderitem = ctx->theme_renderer_.getCurrentRenderItem();

	ResourceManager<TextureBin>& manager = ctx->texture_manager_;
	

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);
		manager.get(value);		

		TextureBin* image = manager.get(value);

		if (NULL == image){

			image = new TextureBin();
			if(strlen(temp) > 0){

				char* pch = strrchr(temp, '.');
				if(pch){

					int video_place_holder = atoi(pch + 1);
					image->setPlaceHolderFlag(video_place_holder);
				}

				char* poutput = NULL;
				int length = 0;
				if(0 == (*ctx->file_ops_.ploadfunc)(&poutput, &length, (char*)(prenderitem->getNamespace() + std::string("/TEXTURE/") + value).c_str(), ctx->file_ops_.cbdata_for_loadfunc)){

					int w, h, channels;
					unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)poutput, length, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);

					image->setTexID(tex_id);
					image->setWidth(w);
					image->setHeight(h);
					image->setChannels(channels);

					delete[] poutput;
				}				
			}
			
			manager.insert(value, image);
		}
		texture_[i] = image;
		SAFE_ADDREF(texture_[i]);
	}

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

	return true;
}

bool MatA::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
	stream.read(&burn_, sizeof(burn_), 1);
	stream.read(&mid_, sizeof(mid_), 1);
	stream.read(&bias_, sizeof(bias_), 1);
	stream.read(&sss_color_, sizeof(sss_color_), 1);
	stream.read(&sss_level_, sizeof(sss_level_), 1);
	stream.read(&sss_width_, sizeof(sss_width_), 1);
	stream.read(&rim_color_, sizeof(rim_color_), 1);
	stream.read(&rim_density_, sizeof(rim_density_), 1);
	stream.read(&rim_width_, sizeof(rim_width_), 1);
	stream.read(&normal_scale_, sizeof(normal_scale_), 1);
	stream.read(&spec_color_, sizeof(spec_color_), 1);
	stream.read(&spec_level_, sizeof(spec_level_), 1);
	stream.read(&spec_gloss_, sizeof(spec_gloss_), 1);
	stream.read(&brush_color_, sizeof(brush_color_), 1);
	stream.read(&brush_center_, sizeof(brush_center_), 1);
	stream.read(&brush_level_, sizeof(brush_level_), 1);
	stream.read(&brush_gloss_, sizeof(brush_gloss_), 1);

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

	RenderItemContext* ctx = stream.getContext();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;
	

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);

        (*pAquireContextFunc)(prenderer);
		TextureBin* image = manager.get(value);

		if (NULL == image){

			image = new TextureBin();
            manager.insert(value, image);
            (*pReleaseContextFunc)(prenderer);
			if(strlen(temp) > 0){

				char* pch = strrchr(temp, '.');
				if(pch){

					int video_place_holder = atoi(pch + 1);
					image->setPlaceHolderFlag(video_place_holder);
				}

				char* poutput = NULL;
				int length = 0;
				if(0 == (*ctx->file_ops_.ploadfunc)(&poutput, &length, (char*)(std::string(uid) + std::string("/TEXTURE/") + value).c_str(), ctx->file_ops_.cbdata_for_loadfunc)){

					int w, h, channels;
                    (*pAquireContextFunc)(prenderer);
					unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)poutput, length, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);
                    (*pReleaseContextFunc)(prenderer);

					image->setTexID(tex_id);
					image->setWidth(w);
					image->setHeight(h);
					image->setChannels(channels);

					delete[] poutput;
				}				
			}
		}
        else{

            (*pReleaseContextFunc)(prenderer);
        }
		texture_[i] = image;
		SAFE_ADDREF(texture_[i]);
	}

	nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

	return true;
}

Material* MatC::clone(){

	return new MatC(shader_);
}

MatC_Shader::~MatC_Shader(){


}

MatC_Shader::MatC_Shader(){

	const GLchar vtx_shader[] = 
		GLSL(

		attribute vec3 a_position; \n
		attribute vec2 a_texCoord; \n
		attribute vec2 a_texCoord_for_lightmap;\n
		attribute vec3 a_normal; \n
		attribute vec3 a_tangent; \n
		attribute vec3 a_binormal; \n

		uniform mat4 u_mvp_matrix; \n
		uniform vec3 u_light_dir; \n
		uniform vec3 u_eye_dir; \n

		varying highp vec3 v_light; \n
		varying highp vec3 v_eye; \n
		varying highp vec2 v_texCoord; \n
		varying highp vec2 v_texCoord_for_lightmap; \n

		void main() {\n

			vec3 tangent = mat3(u_mvp_matrix) * a_tangent; \n
			vec3 normal = mat3(u_mvp_matrix) * a_normal; \n
			vec3 binormal = mat3(u_mvp_matrix) * a_binormal; \n

			v_texCoord = a_texCoord; \n

			mat3 tangentconv;

			tangentconv[0] = vec3(tangent.x, binormal.x, normal.x);
			tangentconv[1] = vec3(tangent.y, binormal.y, normal.y);
			tangentconv[2] = vec3(tangent.z, binormal.z, normal.z);

			v_light = tangentconv * u_light_dir; \n
			v_eye = tangentconv * u_eye_dir; \n
			gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1); \n
		}
	);

	const GLchar frg_shader[] = 

		GLSL(

		varying highp vec2 v_texCoord; \n
		varying highp vec3 v_eye; \n
		varying highp vec3 v_light; \n

		uniform sampler2D u_textureSampler; \n
		uniform sampler2D u_specSampler; \n
		uniform sampler2D u_normalSampler; \n

		

		void main(void){\n

			vec2 texcoord = v_texCoord; \n
			vec4 color = texture2D(u_textureSampler, texcoord); \n
			vec3 normal = texture2D(u_normalSampler, texcoord).rgb;\n
			vec3 spec = texture2D(u_specSampler, texcoord).rgb;\n

			normal = 2.0 * normal - 1.0;
			vec3 Rvec = reflect(v_eye, normal);
			float spcLight = (1 + dot(v_light, Rvec)) * 0.5;
			spec *= spcLight;

			color.rgb *= ((1 + dot(v_light, normal)) * 0.5 + 0.0);\n
			color.rgb += clamp(spec, 0.0, 1.0);
			gl_FragColor = vec4(color.r, color.g, color.b, color.a); \n
		}
	);
}

MatC::MatC(MatC_Shader& shader, int origin_flag):shader_(shader), origin_flag_(origin_flag)
{
	type_ = MAT_BGFX_C;
	bias_ = 0.0f;
	normal_enalbe_ = FALSE;
	normal_scale_ = 0.0f;
	spec_level_ = 0.0f;
	spec_gloss_ = 0.0f;
	spec_color_ = Vector4(0, 0, 0, 0);
	lightmap_intensity_ = 1.0f;
	lightmap_shift_ = 1.0f;
	lightmap_add_ = 0.0f;
}

MatC::~MatC(){

	if(origin_flag_)
		delete &shader_;
}

MatC::MatC(const MatC* pmtl):shader_(pmtl->shader_), origin_flag_(0)
{
	type_ = MAT_BGFX_C;
	bias_ = pmtl->bias_;
	normal_enalbe_ = pmtl->normal_enalbe_;
	normal_scale_ = pmtl->normal_scale_;
	spec_level_ = pmtl->spec_level_;
	spec_gloss_ = pmtl->spec_gloss_;
	spec_color_ = pmtl->spec_color_;
	lightmap_intensity_ = pmtl->lightmap_intensity_;
	lightmap_shift_ = pmtl->lightmap_shift_;
	lightmap_add_ = pmtl->lightmap_add_;
}

bool MatC::deserialize(InputStream& stream)
{
	stream.read(&bias_, sizeof(bias_), 1);
	stream.read(&normal_enalbe_, sizeof(normal_enalbe_), 1);
	stream.read(&normal_scale_, sizeof(normal_scale_), 1);
	stream.read(&spec_level_, sizeof(spec_level_), 1);
	stream.read(&spec_gloss_, sizeof(spec_gloss_), 1);
	stream.read(&spec_color_, sizeof(spec_color_), 1);
	stream.read(&lightmap_intensity_, sizeof(lightmap_intensity_), 1);
	stream.read(&lightmap_shift_, sizeof(lightmap_shift_), 1);
	stream.read(&lightmap_add_, sizeof(lightmap_add_), 1);

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), MAX_ITEM_LEN);
	}

	for (int i = 0; i < MAX_LIGHTMAP; ++i)
		lightmap_[i].deserialize(stream);

	lightmap_channel_ = 0;
	for (int i = 0; i < MAX_LIGHTMAP; ++i){

		if (lightmap_[i].enable_){

			lightmap_channel_ = i;
			break;
		}
	}

	return true;
}

bool MatC::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

	stream.read(&bias_, sizeof(bias_), 1);
	stream.read(&normal_enalbe_, sizeof(normal_enalbe_), 1);
	stream.read(&normal_scale_, sizeof(normal_scale_), 1);
	stream.read(&spec_level_, sizeof(spec_level_), 1);
	stream.read(&spec_gloss_, sizeof(spec_gloss_), 1);
	stream.read(&spec_color_, sizeof(spec_color_), 1);
	stream.read(&lightmap_intensity_, sizeof(lightmap_intensity_), 1);
	stream.read(&lightmap_shift_, sizeof(lightmap_shift_), 1);
	stream.read(&lightmap_add_, sizeof(lightmap_add_), 1);

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), MAX_ITEM_LEN);
	}

	for (int i = 0; i < MAX_LIGHTMAP; ++i)
		lightmap_[i].deserialize(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);

	lightmap_channel_ = 0;
	for (int i = 0; i < MAX_LIGHTMAP; ++i){

		if (lightmap_[i].enable_){

			lightmap_channel_ = i;
			break;
		}
	}

	return true;
}


MatSkin::MatSkin(MatSkin_Shader& shader, int origin_flag):shader_(shader), origin_flag_(origin_flag){


}

Material* MatSkin::clone(){

	return new MatSkin(shader_);
}

void MatSkin::setEyeDir(float x, float y, float z){

	glUniform3f(shader_.eye_dir_uniform, x, y, z);
}

void MatSkin::setLightDir(float x, float y, float z){

	glUniform3f(shader_.light_dir_uniform, x, y, z);
}

MatSkin_Shader::~MatSkin_Shader(){

	glDeleteProgram(program_idx);
}

MatSkin_Shader::MatSkin_Shader(){

	const GLchar vtx_shader[] = 
		GLSL(

		attribute vec3 a_position;
		attribute vec2 a_texCoord;
		attribute vec3 a_normal;

		uniform mat4 u_mvp_matrix;
		uniform mat4 u_world_matrix;
		uniform mat4 u_tex_matrix;

		varying highp vec2 v_texcoord;
		varying highp vec3 v_normal;

		void main() {
			
			v_texcoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
			v_normal = normalize((u_world_matrix * vec4(a_normal.xyz, 0)).xyz);
			gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1);
		}
	);

	const GLchar frg_shader[] = 
		GLSL(

		varying highp vec2 v_texcoord;
		varying highp vec3 v_normal;

		uniform highp mat4 u_color_matrix;
		uniform sampler2D u_textureSampler;
		uniform sampler2D u_specSampler;
		uniform int u_check_diffusemap;
		uniform int u_check_specmap;
		uniform int u_check_alphatest;
		uniform int u_system_source;
		uniform highp float u_opacity;
		uniform highp vec4 u_diffuse_color;
		uniform highp vec3 u_ambient_color;
		uniform highp vec3 u_spec_color;
		uniform highp vec3 u_light_dir;
		uniform highp vec3 u_eye_dir;
		uniform highp float u_spec_level;
		uniform highp float u_spec_gloss;
		uniform highp float u_real_x;
		uniform highp float u_real_y;

		void main(void){

			if(u_check_diffusemap > 0){

				highp float s = 1.0;

				if(u_system_source > 0){

					s = step(0.0, v_texcoord.y) * step(0.0, v_texcoord.x) * step(v_texcoord.y, u_real_y) * step(v_texcoord.x, u_real_x);
				}

				if(s > 0.0){

					highp vec4 color = texture2D(u_textureSampler, v_texcoord);
					if(u_check_alphatest > 0){

						if(color.a * u_opacity < 1.0)
							discard;
					}
					if(u_check_alphatest < 0){

						if(color.a * u_opacity >= 1.0)
							discard;
					}
					highp vec3 spec = u_check_specmap > 0?texture2D(u_specSampler, v_texcoord).rgb:u_spec_color;

					color = color * u_color_matrix;
					color = clamp(color, 0.0, 1.0);

					highp vec3 Rvec = reflect(-u_eye_dir, v_normal);
					highp float spcLight = clamp(dot(u_light_dir, Rvec), 0.0, 1.0);
					highp float light = clamp(dot(v_normal, u_light_dir), 0.0, 1.0);
					spec = clamp(spec * u_spec_level * pow(spcLight, u_spec_gloss), 0.0, 1.0);
					highp vec3 light_comp = clamp(vec3(u_ambient_color.x + light, u_ambient_color.y + light, u_ambient_color.z + light), 0.0, 1.0);
					color.rgb *= light_comp;
					color.rgb += spec;
					//color = clamp(color, vec4(0, 0, 0, 0), vec4(1, 1, 1, 1));
					gl_FragColor = vec4(color.rgb * u_opacity, color.a * u_opacity);// * 0.0001 + vec4(v_normal.x, v_normal.y, v_normal.z, 1.0);
				}
				else{

					gl_FragColor = vec4(0, 0, 0, 1);
				}

			}
			else{

				highp vec4 color = u_diffuse_color;
				if(u_check_alphatest > 0){

					if(color.a * u_opacity < 1.0)
						discard;
				}
				if(u_check_alphatest < 0){

					if(color.a * u_opacity >= 1.0)
						discard;
				}
				highp vec3 spec = u_check_specmap > 0?texture2D(u_specSampler, v_texcoord).rgb:u_spec_color;

				highp vec3 Rvec = reflect(-u_eye_dir, v_normal);
				highp float spcLight = clamp(dot(u_light_dir, Rvec), 0.0, 1.0);
				highp float light = clamp(dot(v_normal, u_light_dir), 0.0, 1.0);
				spec = clamp(spec * u_spec_level * pow(spcLight, u_spec_gloss), 0.0, 1.0);
				highp vec3 light_comp = clamp(vec3(u_ambient_color.x + light, u_ambient_color.y + light, u_ambient_color.z + light), 0.0, 1.0);
				color.rgb *= light_comp;
				color.rgb += spec;
				gl_FragColor = vec4(color.rgb * u_opacity, color.a * u_opacity);// * 0.0001 + vec4(v_normal.x, v_normal.y, v_normal.z, 1.0);

			}
		}
	);


	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLint size_for_vtx = sizeof(vtx_shader);
	GLint size_for_frg = sizeof(frg_shader);
	const GLchar* pvtx = vtx_shader;
	const GLchar* pfrg = frg_shader;

	char error_text[1024] = "no error";
	GLsizei length = 0;

	glShaderSource(vertex_shader, 1, &pvtx, &size_for_vtx);
	glCompileShader(vertex_shader);

	glGetShaderInfoLog(vertex_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] vtx output:%s", __func__, __LINE__, error_text);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &pfrg, &size_for_frg);
	glCompileShader(fragment_shader);

	glGetShaderInfoLog(fragment_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] frg output:%s", __func__, __LINE__, error_text);

	GLuint program = glCreateProgram(); CHECK_GL_ERROR();
	glAttachShader(program, vertex_shader); CHECK_GL_ERROR();
	glAttachShader(program, fragment_shader); CHECK_GL_ERROR();
	glLinkProgram(program); CHECK_GL_ERROR();

	
	glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

	nexSAL_TraceCat(0, 0, "[%s %d] %s program:%d", __func__, __LINE__, error_text, program);

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	program_idx = program;

	vertex_attrib = glGetAttribLocation(program, "a_position");
	texcoord_attrib = glGetAttribLocation(program, "a_texCoord");
	normal_attrib = glGetAttribLocation(program, "a_normal");
	eye_dir_uniform = glGetUniformLocation(program_idx, "u_eye_dir");
	light_dir_uniform = glGetUniformLocation(program_idx, "u_light_dir");
	world_matrix_uniform = glGetUniformLocation(program_idx, "u_world_matrix");
	mvp_matrix_uniform = glGetUniformLocation(program_idx, "u_mvp_matrix");
	tex_matrix_uniform = glGetUniformLocation(program_idx, "u_tex_matrix");
	opacity_uniform = glGetUniformLocation(program_idx, "u_opacity");
	color_matrix_uniform = glGetUniformLocation(program_idx, "u_color_matrix");
	texture_uniform = glGetUniformLocation(program_idx, "u_textureSampler");
	spec_uniform = glGetUniformLocation(program_idx, "u_specSampler");
	check_diffusemap_uniform = glGetUniformLocation(program_idx, "u_check_diffusemap");
	check_alphatest_uniform = glGetUniformLocation(program_idx, "u_check_alphatest");
	system_source_uniform = glGetUniformLocation(program_idx, "u_system_source");
	check_specmap_uniform = glGetUniformLocation(program_idx, "u_check_specmap");
	diffuse_color_uniform = glGetUniformLocation(program_idx, "u_diffuse_color");
	ambient_color_uniform = glGetUniformLocation(program_idx, "u_ambient_color");
	spec_color_uniform = glGetUniformLocation(program_idx, "u_spec_color");
	spec_level_uniform = glGetUniformLocation(program_idx, "u_spec_level");
	spec_gloss_uniform = glGetUniformLocation(program_idx, "u_spec_gloss");
	real_x_uniform = glGetUniformLocation(program_idx, "u_real_x");
	real_y_uniform = glGetUniformLocation(program_idx, "u_real_y");
}

MatNexEffect01_Shader::~MatNexEffect01_Shader(){

	glDeleteProgram(program_idx);
}

MatNexEffect01_Shader::MatNexEffect01_Shader(){

	const GLchar vtx_shader[] = 
		GLSL(

		attribute vec3 a_position;
		attribute vec2 a_texCoord;

		uniform mat4 u_mvp_matrix;

		varying highp vec2 v_texcoord;

		void main() {
			
			v_texcoord = a_texCoord;//(vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
			gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1);
		}
	);

	const GLchar frg_shader[] = 
		GLSL(

		varying highp vec2 v_texcoord;
		varying highp vec3 v_normal;

		uniform highp float u_diffuse_level;
		uniform highp float u_alpha_level;
		uniform highp vec3 u_diffuse_color;
		uniform int u_check_alphatest;

		uniform sampler2D u_textureSampler;
		uniform sampler2D u_alphatextureSampler;

		void main(void){

			highp vec4 color = texture2D(u_textureSampler, v_texcoord);
			highp vec4 alpha = texture2D(u_alphatextureSampler, v_texcoord);

			if(u_check_alphatest > 0){

				if(alpha.a * u_alpha_level < 1.0)
					discard;
			}
			if(u_check_alphatest < 0){

				if(alpha.a * u_alpha_level >= 1.0)
					discard;
			}

			gl_FragColor = vec4(color.rgb * u_diffuse_color * u_diffuse_level * u_alpha_level, u_alpha_level * alpha.a);
		}
	);


	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLint size_for_vtx = sizeof(vtx_shader);
	GLint size_for_frg = sizeof(frg_shader);
	const GLchar* pvtx = vtx_shader;
	const GLchar* pfrg = frg_shader;

	char error_text[1024] = "no error";
	GLsizei length = 0;

	glShaderSource(vertex_shader, 1, &pvtx, &size_for_vtx);
	glCompileShader(vertex_shader);

	glGetShaderInfoLog(vertex_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] vtx output:%s", __func__, __LINE__, error_text);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &pfrg, &size_for_frg);
	glCompileShader(fragment_shader);

	glGetShaderInfoLog(fragment_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
	nexSAL_TraceCat(0, 0, "[%s %d] frg output:%s", __func__, __LINE__, error_text);

	GLuint program = glCreateProgram(); CHECK_GL_ERROR();
	glAttachShader(program, vertex_shader); CHECK_GL_ERROR();
	glAttachShader(program, fragment_shader); CHECK_GL_ERROR();
	glLinkProgram(program); CHECK_GL_ERROR();

	
	glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();

	nexSAL_TraceCat(0, 0, "[%s %d] %s program:%d", __func__, __LINE__, error_text, program);

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	program_idx = program;

	vertex_attrib = glGetAttribLocation(program, "a_position");
	texcoord_attrib = glGetAttribLocation(program, "a_texCoord");
	mvp_matrix_uniform = glGetUniformLocation(program_idx, "u_mvp_matrix");
	diffuse_level_uniform = glGetUniformLocation(program_idx, "u_diffuse_level");
	alpha_level_uniform = glGetUniformLocation(program_idx, "u_alpha_level");
	check_alphatest_uniform = glGetUniformLocation(program_idx, "u_check_alphatest");
	diffuse_color_uniform = glGetUniformLocation(program_idx, "u_diffuse_color");
	texture_uniform = glGetUniformLocation(program_idx, "u_textureSampler");
	alphatexture_uniform = glGetUniformLocation(program_idx, "u_alphatextureSampler");
};


template<typename T>
void readList(RangedVector<T>& list, InputStream& stream)
{
	list.clear();
	unsigned int count = 0;
	stream.read(&count, sizeof(count), 1);
	for (unsigned int i = 0; i < count; ++i)
	{
		T val;
		stream.read(&val, sizeof(val), 1);
		list.push_back(val);
	}
}

template<typename T>
void readChunkData(RangedVector<T>& list, int chunk_id, InputStream& stream){

	list.clear();
	int check_chunk_id = 0;
	stream.read(&check_chunk_id, sizeof(check_chunk_id), 1);
	if (check_chunk_id == chunk_id){

		readList(list, stream);
	}
	else{

		stream.seek(-(int)sizeof(check_chunk_id), SEEK_CUR);
	}
}

TextureBin* createTexture(RenderItemContext* ctx, RenderItem* prenderitem, ResourceManager<TextureBin>& manager, char* name){

	std::string value = std::string(name);

	TextureBin* image = manager.get(value);

	if (NULL == image){

		image = new TextureBin();
		int length = strlen(name);
		if(length > 0){

			int video_place_holder = 0;
			char* pch = strrchr(name, '.');
			if(pch && (length - (int)(pch - name)) == 3){

				video_place_holder = atoi(pch + 1);
				
			}
			else{

				if(pch)
					*pch = 0;

				if (!strcmp("video_src_0", name)){
					video_place_holder = 1;
				}
				else if (!strcmp("filtered_video_src_0", name)){

					video_place_holder = 0x11;
				}
				else if (!strcmp("video_src_1", name)){

					video_place_holder = 2;
				}
				else if (!strcmp("filtered_video_src_1", name)){

					video_place_holder = 0x12;
				}
				else if (!strcmp("text_src_0", name)){

					video_place_holder = 3;
				}
				else if (!strcmp("text_src_1", name)){

					video_place_holder = 4;
				}
				else if (!strcmp("text_src_2", name)){

					video_place_holder = 5;
				}
				else if (!strcmp("text_src_3", name)){

					video_place_holder = 6;
				}
				else if (!strcmp("text_src_4", name)){

					video_place_holder = 7;
				}
				else if (!strcmp("text_src_5", name)){

					video_place_holder = 8;
				}
				else if (!strcmp("text_src_6", name)){

					video_place_holder = 9;
				}
			}

			image->setPlaceHolderFlag(video_place_holder);

			char* poutput = NULL;
			int length = 0;
			if(0 == (*ctx->file_ops_.ploadfunc)(&poutput, &length, (char*)(prenderitem->getNamespace() + std::string("/TEXTURE/") + value).c_str(), ctx->file_ops_.cbdata_for_loadfunc)){

				int w, h, channels;
				unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)poutput, length, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);

				image->setTexID(tex_id);
				image->setWidth(w);
				image->setHeight(h);
				image->setChannels(channels);

				delete[] poutput;
			}				
		}
		
		manager.insert(value, image);
	}

	return image;
}

TextureBin* createTexture(RenderItemContext* ctx, char* uid, ResourceManager<TextureBin>& manager, char* name, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p)){

	std::string value = std::string(name);

    (*pAquireContextFunc)(prenderer);
	TextureBin* image = manager.get(value);
	if (NULL == image){

		image = new TextureBin();
        manager.insert(value, image);
        (*pReleaseContextFunc)(prenderer);
		int length = strlen(name);
		if(length > 0){

			int video_place_holder = 0;
			char* pch = strrchr(name, '.');
			if(pch && (length - (int)(pch - name)) == 3){

				video_place_holder = atoi(pch + 1);
				
			}
			else{

				if(pch)
					*pch = 0;

				if (!strcmp("video_src_0", name)){
					video_place_holder = 1;
				}
				else if (!strcmp("filtered_video_src_0", name)){

					video_place_holder = 0x11;
				}
				else if (!strcmp("video_src_1", name)){

					video_place_holder = 2;
				}
				else if (!strcmp("filtered_video_src_1", name)){

					video_place_holder = 0x12;
				}
				else if (!strcmp("text_src_0", name)){

					video_place_holder = 3;
				}
				else if (!strcmp("text_src_1", name)){

					video_place_holder = 4;
				}
				else if (!strcmp("text_src_2", name)){

					video_place_holder = 5;
				}
				else if (!strcmp("text_src_3", name)){

					video_place_holder = 6;
				}
				else if (!strcmp("text_src_4", name)){

					video_place_holder = 7;
				}
				else if (!strcmp("text_src_5", name)){

					video_place_holder = 8;
				}
				else if (!strcmp("text_src_6", name)){

					video_place_holder = 9;
				}
			}

			image->setPlaceHolderFlag(video_place_holder);

			char* poutput = NULL;
			int length = 0;
			if(0 == (*ctx->file_ops_.ploadfunc)(&poutput, &length, (char*)(std::string(uid) + std::string("/TEXTURE/") + value).c_str(), ctx->file_ops_.cbdata_for_loadfunc)){

				int w, h, channels;
                (*pAquireContextFunc)(prenderer);
				unsigned int tex_id = SOIL_load_OGL_texture_from_memory((const unsigned char *)poutput, length, 0, 0, SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h, &channels);
                (*pReleaseContextFunc)(prenderer);

				image->setTexID(tex_id);
				image->setWidth(w);
				image->setHeight(h);
				image->setChannels(channels);

				delete[] poutput;
			}				
		}
	}
    else{

        (*pReleaseContextFunc)(prenderer);
    }

	return image;
}

bool MatSkin::deserialize_v10(InputStream& stream){

	RenderItemContext* ctx = stream.getContext();
	RenderItem* prenderitem = ctx->theme_renderer_.getCurrentRenderItem();
	ResourceManager<TextureBin>& manager = stream.getContext()->texture_manager_;

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&ambient_color_, sizeof(ambient_color_), 1);
	stream.read(&specular_color_, sizeof(specular_color_), 1);
	stream.read(&opacity_, sizeof(opacity_), 1);

	specular_level_ = 0.0f;
	glossiness_ = 0.0f;

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		int check_mtl = 0;

		stream.read(&check_mtl, sizeof(check_mtl), 1);

		texture_option_[i] = check_mtl;

		if (check_mtl == 0){

			texture_[i] = NULL;
			continue;
		}

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, prenderitem, manager, temp);
		SAFE_ADDREF(texture_[i]);
	}

	readChunkData(opacity_list_, OPACITY_CHUNK_HEADER, stream);

	if(opacity_list_.size() > 0){

		setAlphaBlendingFlag(true);
	}
	else if(opacity_ < 1.0f)
		setAlphaBlendingFlag(true);
	else if(texture_[0]){

		if(texture_[0]->getChannels() >= 4){

			setAlphaBlendingFlag(true);
		}
	}

	return true;
}
	
bool MatSkin::deserialize_v11(InputStream& stream){

	RenderItemContext* ctx = stream.getContext();
	RenderItem* prenderitem = ctx->theme_renderer_.getCurrentRenderItem();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&ambient_color_, sizeof(ambient_color_), 1);
	stream.read(&specular_color_, sizeof(specular_color_), 1);
	stream.read(&specular_level_, sizeof(specular_level_), 1);
	stream.read(&glossiness_, sizeof(glossiness_), 1);
	stream.read(&opacity_, sizeof(opacity_), 1);

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		int check_mtl = 0;

		stream.read(&check_mtl, sizeof(check_mtl), 1);

		texture_option_[i] = check_mtl;

		if (check_mtl == 0){

			texture_[i] = NULL;
			continue;
		}
			
		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, prenderitem, manager, temp);
		SAFE_ADDREF(texture_[i]);
	}

	readChunkData(diffuse_list_, DIFFUSE_CHUNK_HEADER, stream);
	readChunkData(ambient_list_, AMBIENT_CHUNK_HEADER, stream);
	readChunkData(specular_list_, SPECULAR_CHUNK_HEADER, stream);
	readChunkData(specular_level_list_, SPECLEVEL_CHUNK_HEADER, stream);
	readChunkData(glossiness_list_, GLOSSINESS_CHUNK_HEADER, stream);
	readChunkData(opacity_list_, OPACITY_CHUNK_HEADER, stream);

	if(opacity_list_.size() > 0){

		setAlphaBlendingFlag(true);
	}
	else if(opacity_ < 1.0f)
		setAlphaBlendingFlag(true);
	else if(texture_[0]){

		if(texture_[0]->getChannels() >= 4){

			setAlphaBlendingFlag(true);
		}
	}

	return true;
}

bool MatSkin::deserialize(InputStream& stream){

	int sub_version = 0;
	stream.read(&sub_version, sizeof(sub_version), 1);
	if(sub_version == STDMAT_V11_PARAM_HEADER)
		return deserialize_v11(stream);
	stream.seek(-(int)sizeof(sub_version), SEEK_CUR);
	return deserialize_v10(stream);
}

bool MatSkin::deserialize_v10(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void *p)){

	RenderItemContext* ctx = stream.getContext();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&ambient_color_, sizeof(ambient_color_), 1);
	stream.read(&specular_color_, sizeof(specular_color_), 1);
	stream.read(&opacity_, sizeof(opacity_), 1);

	specular_level_ = 0.0f;
	glossiness_ = 0.0f;

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		int check_mtl = 0;

		stream.read(&check_mtl, sizeof(check_mtl), 1);

		texture_option_[i] = check_mtl;

		if (check_mtl == 0){

			texture_[i] = NULL;
			continue;
		}

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, uid, manager, temp, prenderer, pAquireContextFunc, pReleaseContextFunc);
		SAFE_ADDREF(texture_[i]);
	}

	readChunkData(opacity_list_, OPACITY_CHUNK_HEADER, stream);

	if(opacity_list_.size() > 0){

		setAlphaBlendingFlag(true);
	}
	else if(opacity_ < 1.0f)
		setAlphaBlendingFlag(true);
	else if(texture_[0]){

		if(texture_[0]->getChannels() >= 4){

			setAlphaBlendingFlag(true);
		}
	}

	return true;
}
	
bool MatSkin::deserialize_v11(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void *p)){

	RenderItemContext* ctx = stream.getContext();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&ambient_color_, sizeof(ambient_color_), 1);
	stream.read(&specular_color_, sizeof(specular_color_), 1);
	stream.read(&specular_level_, sizeof(specular_level_), 1);
	stream.read(&glossiness_, sizeof(glossiness_), 1);
	stream.read(&opacity_, sizeof(opacity_), 1);

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		int check_mtl = 0;

		stream.read(&check_mtl, sizeof(check_mtl), 1);

		texture_option_[i] = check_mtl;

		if (check_mtl == 0){

			texture_[i] = NULL;
			continue;
		}
			
		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, uid, manager, temp, prenderer, pAquireContextFunc, pReleaseContextFunc);
		SAFE_ADDREF(texture_[i]);
	}

	readChunkData(diffuse_list_, DIFFUSE_CHUNK_HEADER, stream);
	readChunkData(ambient_list_, AMBIENT_CHUNK_HEADER, stream);
	readChunkData(specular_list_, SPECULAR_CHUNK_HEADER, stream);
	readChunkData(specular_level_list_, SPECLEVEL_CHUNK_HEADER, stream);
	readChunkData(glossiness_list_, GLOSSINESS_CHUNK_HEADER, stream);
	readChunkData(opacity_list_, OPACITY_CHUNK_HEADER, stream);

	if(opacity_list_.size() > 0){

		setAlphaBlendingFlag(true);
	}
	else if(opacity_ < 1.0f)
		setAlphaBlendingFlag(true);
	else if(texture_[0]){

		if(texture_[0]->getChannels() >= 4){

			setAlphaBlendingFlag(true);
		}
	}

	return true;
}

bool MatSkin::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void *p)){

	int sub_version = 0;
	stream.read(&sub_version, sizeof(sub_version), 1);
	if(sub_version == STDMAT_V11_PARAM_HEADER)
		return deserialize_v11(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);
	stream.seek(-(int)sizeof(sub_version), SEEK_CUR);
	return deserialize_v10(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);
}

int MatSkin::getWidth(){

	if(texture_[TOP_DIFFUSE]){

		return texture_[TOP_DIFFUSE]->getWidth();
	}

	return Material::getWidth();
}

int MatSkin::getHeight(){

	if(texture_[TOP_DIFFUSE]){

		return texture_[TOP_DIFFUSE]->getHeight();
	}

	return Material::getHeight();
}

int MatSkin::getPlaceHolderFlag(){

	if(texture_[TOP_DIFFUSE]){

		return texture_[TOP_DIFFUSE]->getPlaceHolderFlag();
	}
	
	return Material::getPlaceHolderFlag();
}

MatNexEffect01::MatNexEffect01(MatNexEffect01_Shader& shader, int origin_flag):shader_(shader), origin_flag_(origin_flag){


}

Material* MatNexEffect01::clone(){

	return new MatNexEffect01(shader_);
}

int MatNexEffect01::getWidth(){

	if(texture_[DIFFUSE]){

		return texture_[DIFFUSE]->getWidth();
	}

	return Material::getWidth();
}

int MatNexEffect01::getHeight(){

	if(texture_[DIFFUSE]){

		return texture_[DIFFUSE]->getHeight();
	}

	return Material::getHeight();
}

int MatNexEffect01::getPlaceHolderFlag(){

	if(texture_[DIFFUSE]){

		return texture_[DIFFUSE]->getPlaceHolderFlag();
	}

	return Material::getPlaceHolderFlag();
}

bool MatNexEffect01::deserialize(InputStream& stream){

	RenderItemContext* ctx = stream.getContext();
	RenderItem* prenderitem = ctx->theme_renderer_.getCurrentRenderItem();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, prenderitem, manager, temp);
		SAFE_ADDREF(texture_[i]);
	}

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&diffuse_level_, sizeof(diffuse_level_), 1);
	stream.read(&alpha_level_, sizeof(alpha_level_), 1);

	readList(alpha_level_list_, stream);
	readList(diffuse_level_list_, stream);
	setAlphaBlendingFlag(true);
	return true;
}

bool MatNexEffect01::deserialize(InputStream& stream, char* uid, void *prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

	RenderItemContext* ctx = stream.getContext();
	ResourceManager<TextureBin>& manager = ctx->texture_manager_;

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		stream.read(temp, sizeof(temp), 1);
		nexSAL_TraceCat(0, 0, "[%s %d] %s", "LoadTexture", __LINE__, temp);
		std::string value = std::string(temp);
		texture_[i] = createTexture(ctx, uid, manager, temp, prenderer, pAquireContextFunc, pReleaseContextFunc);
		SAFE_ADDREF(texture_[i]);
	}

	stream.read(&diffuse_color_, sizeof(diffuse_color_), 1);
	stream.read(&diffuse_level_, sizeof(diffuse_level_), 1);
	stream.read(&alpha_level_, sizeof(alpha_level_), 1);

	readList(alpha_level_list_, stream);
	readList(diffuse_level_list_, stream);
	setAlphaBlendingFlag(true);
	return true;
}
