#include "../types.h"
#include "material.h"
#include "../util.h"
#include "../Singleton.h"
#include "../ResourceManager.h"
#include "../img.h"

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif


Basic3DMaterial::Basic3DMaterial(){


}

Basic3DMaterial::~Basic3DMaterial(){

	
}

bool Basic3DMaterial::deserialize(FILE* fp){

	char resource_name[256];
	
	fread(resource_name, sizeof(char), sizeof(resource_name) - 1, fp);
	resource_name[strlen(resource_name)] = 0;

	std::string key(resource_name);
	
	ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
	
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


bool Basic3DMaterial::serialize(FILE* fp){

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

bool TextureChunk::deserialize(FILE* fp)
{
	free();
	unsigned int check_type(0);
	fread(&check_type, sizeof(check_type), 1, fp);
	if (check_type != TEXTURE_CHUNK)
	{
		fseek(fp, -4, SEEK_CUR);
		return false;
	}
	fread(&idx_, sizeof(idx_), 1, fp);
	fread(&size_of_compressed_, sizeof(size_of_compressed_), 1, fp);
	fread(&size_of_uncompressed_, sizeof(size_of_uncompressed_), 1, fp);

	unsigned char* pbyte_for_compress(NULL);
	unsigned char* pbyte_for_tex(NULL);

	pbyte_for_compress = (unsigned char*)malloc(size_of_compressed_);
	pbyte_for_tex = (unsigned char*)malloc(size_of_uncompressed_);

	fread(pbyte_for_compress, sizeof(BYTE), size_of_compressed_, fp);

	char filename[256];
	sprintf(filename, "%d", idx_);
	
	TextureBin* image = NULL;
	ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
	image = manager.get(std::string(filename));

	if (NULL == image){

		uncompress(pbyte_for_tex, (uLongf*)&size_of_uncompressed_, pbyte_for_compress, size_of_compressed_);
		int w, h;
		unsigned int tex_id = SOIL_load_OGL_texture_from_memory(pbyte_for_tex, size_of_uncompressed_, 0, 0, SOIL_FLAG_DDS_LOAD_DIRECT | SOIL_FLAG_TEXTURE_REPEATS, &w, &h);

		image = new TextureBin();
		image->setTexID(tex_id);
		image->setWidth(w);
		image->setHeight(h);
		manager.insert(std::string(filename), image);
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

void Lightmap::deserialize(FILE* fp)
{
	freadT(&enable_, sizeof(enable_), 1, fp);
	freadT(&color_, sizeof(color_), 1, fp);

	char temp[MAX_ITEM_LEN] = { 0, };
	freadT(temp, sizeof(temp), MAX_ITEM_LEN, fp);
}

Lightmap::~Lightmap()
{
}

#define GLSL(shader) #shader

GLuint MatSolidColor::program_idx = 0;
GLint MatSolidColor::vertex_attrib = 0;
GLint MatSolidColor::uv_attrib = 0;
GLint MatSolidColor::color_attrib = 0;
GLint MatSolidColor::texture_uniform = 0;
GLint MatSolidColor::mvp_matrix_uniform = 0;

MatSolidColor::MatSolidColor():ptex_(NULL){

	if (program_idx <= 0){

		const GLchar vtx_shader[] = "#version 120\n" GLSL(

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

		const GLchar frg_shader[] = "#version 120\n" GLSL(

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

	type_ = MAT_SOLID_COLOR;
}

MatSolidColor::MatSolidColor(const MatSolidColor* mtl){

}

GLuint MatA::program_idx = 0;
GLint MatA::vertex_attrib = 0;
GLint MatA::texcoord_attrib = 0;
GLint MatA::binormal_attrib = 0;
GLint MatA::normal_attrib = 0;
GLint MatA::tangent_attrib = 0;
GLint MatA::eye_dir_uniform = 0;
GLint MatA::light_dir_uniform = 0;
GLint MatA::texture_uniform = 0;
GLint MatA::spec_uniform = 0;
GLint MatA::normal_uniform = 0;
GLint MatA::world_matrix_uniform = 0;
GLint MatA::mvp_matrix_uniform = 0;

MatA::MatA()
{
	if (MatA::program_idx <= 0){

		const GLchar vtx_shader[] = 
			"#version 120\n" GLSL(

			attribute vec3 a_position; \n
			attribute vec2 a_texCoord; \n
			attribute vec3 a_normal; \n
			attribute vec3 a_tangent; \n
			attribute vec3 a_binormal; \n

			uniform mat4 u_world_matrix; \n
			uniform mat4 u_mvp_matrix; \n
			uniform vec3 u_light_dir; \n
			uniform vec3 u_eye_dir; \n

			varying highp vec3 v_light; \n
			varying highp vec3 v_eye; \n
			varying highp vec2 v_texCoord; \n

			void main() {\n
				vec3 tangent = mat3(u_world_matrix) * a_tangent; \n
				vec3 normal = mat3(u_world_matrix) * a_normal; \n
				vec3 binormal = mat3(u_world_matrix) * a_binormal; \n

				v_texCoord = a_texCoord; \n

				mat3 tangentconv = transpose(mat3(
						tangent,
						binormal,
						normal) );

				v_light = tangentconv * u_light_dir; \n
				v_eye = tangentconv * u_eye_dir; \n
				gl_Position = u_mvp_matrix * vec4(a_position.xyz, 1); \n
			}
		);

		const GLchar frg_shader[] = 
			"#version 120\n" GLSL(

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

				normal = normalize(2.0 * normal.rgb - 1.0);
				vec3 Rvec = reflect(v_eye, normal);
    			float spcLight = (1 + dot(v_light, Rvec)) * 0.5;
    			spec *= spcLight;

				color.rgb *= clamp(dot(normal, v_light), 0, 1);
				color.rgb += spec;
				gl_FragColor = vec4(color.r, color.g, color.b, color.a); \n
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
		texcoord_attrib = glGetAttribLocation(program, "a_texCoord");
		normal_attrib = glGetAttribLocation(program, "a_normal");
		tangent_attrib = glGetAttribLocation(program, "a_tangent");
		binormal_attrib = glGetAttribLocation(program, "a_binormal");
		eye_dir_uniform = glGetUniformLocation(program_idx, "u_eye_dir");
		light_dir_uniform = glGetUniformLocation(program_idx, "u_light_dir");
		world_matrix_uniform = glGetUniformLocation(program_idx, "u_world_matrix");
		mvp_matrix_uniform = glGetUniformLocation(program_idx, "u_mvp_matrix");
		texture_uniform = glGetUniformLocation(program_idx, "u_textureSampler");
		spec_uniform = glGetUniformLocation(program_idx, "u_specSampler");
		normal_uniform = glGetUniformLocation(program_idx, "u_normalSampler");
	}

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

MatA::MatA(const MatA* mtl)
{
}

void MatA::setEyeDir(float x, float y, float z){

	glUniform3f(eye_dir_uniform, x, y, z);
}

void MatA::setLightDir(float x, float y, float z){

	glUniform3f(light_dir_uniform, x, y, z);
}

bool MatA::deserialize(FILE *fp){

	freadT(&burn_, sizeof(burn_), 1, fp);
	freadT(&mid_, sizeof(mid_), 1, fp);
	freadT(&bias_, sizeof(bias_), 1, fp);
	freadT(&sss_color_, sizeof(sss_color_), 1, fp);
	freadT(&sss_level_, sizeof(sss_level_), 1, fp);
	freadT(&sss_width_, sizeof(sss_width_), 1, fp);
	freadT(&rim_color_, sizeof(rim_color_), 1, fp);
	freadT(&rim_density_, sizeof(rim_density_), 1, fp);
	freadT(&rim_width_, sizeof(rim_width_), 1, fp);
	freadT(&normal_scale_, sizeof(normal_scale_), 1, fp);
	freadT(&spec_color_, sizeof(spec_color_), 1, fp);
	freadT(&spec_level_, sizeof(spec_level_), 1, fp);
	freadT(&spec_gloss_, sizeof(spec_gloss_), 1, fp);
	freadT(&brush_color_, sizeof(brush_color_), 1, fp);
	freadT(&brush_center_, sizeof(brush_center_), 1, fp);
	freadT(&brush_level_, sizeof(brush_level_), 1, fp);
	freadT(&brush_gloss_, sizeof(brush_gloss_), 1, fp);

	ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
	

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		freadT(temp, sizeof(temp), MAX_ITEM_LEN, fp);
		texture_[i] = manager.get(std::string(temp));
		SAFE_ADDREF(texture_[i]);
	}

	return true;
}

GLuint MatC::program_idx = 0;
GLint MatC::vertex_attrib = 0;
GLint MatC::texcoord_attrib = 0;
GLint MatC::texcoord_for_lightmap_attrib = 0;
GLint MatC::binormal_attrib = 0;
GLint MatC::normal_attrib = 0;
GLint MatC::tangent_attrib = 0;
GLint MatC::eye_dir_uniform = 0;
GLint MatC::light_dir_uniform = 0;
GLint MatC::texture_uniform = 0;
GLint MatC::texture_for_lightmap_uniform = 0;
GLint MatC::spec_uniform = 0;
GLint MatC::normal_uniform = 0;
GLint MatC::world_matrix_uniform = 0;
GLint MatC::mvp_matrix_uniform = 0;

Material* MatC::clone(){

	return new MatC();
}

MatC::MatC()
{
	if (MatC::program_idx <= 0){

		const GLchar vtx_shader[] = 
			"#version 120\n"
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
			"#version 120\n"

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
				color.rgb += spec;
				gl_FragColor = vec4(color.r, color.g, color.b, color.a); \n
			}
		);
	}
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


}

MatC::MatC(const MatC* pmtl)
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

bool MatC::serialize(FILE* fp){

	return true;
}

bool MatC::deserialize(FILE* fp)
{
	freadT(&bias_, sizeof(bias_), 1, fp);
	freadT(&normal_enalbe_, sizeof(normal_enalbe_), 1, fp);
	freadT(&normal_scale_, sizeof(normal_scale_), 1, fp);
	freadT(&spec_level_, sizeof(spec_level_), 1, fp);
	freadT(&spec_gloss_, sizeof(spec_gloss_), 1, fp);
	freadT(&spec_color_, sizeof(spec_color_), 1, fp);
	freadT(&lightmap_intensity_, sizeof(lightmap_intensity_), 1, fp);
	freadT(&lightmap_shift_, sizeof(lightmap_shift_), 1, fp);
	freadT(&lightmap_add_, sizeof(lightmap_add_), 1, fp);

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		char temp[MAX_ITEM_LEN] = { 0, };
		freadT(temp, sizeof(temp), MAX_ITEM_LEN, fp);
	}

	for (int i = 0; i < MAX_LIGHTMAP; ++i)
		lightmap_[i].deserialize(fp);

	lightmap_channel_ = 0;
	for (int i = 0; i < MAX_LIGHTMAP; ++i){

		if (lightmap_[i].enable_){

			lightmap_channel_ = i;
			break;
		}
	}

	return true;
}

Material* MatSkin::clone(){

	return new MatSkin();
}

bool MatSkin::deserialize(FILE* fp){

	int matching[END_OF_MATERIAL] = {

		MatA::DIFFUSE,//TOP_DIFFUSE
		-1,//BOTTOM_DIFFUSE,
		-1,//THIRD_DIFFUSE,
		MatA::SPEC,//SPEC_LEVEL,
		MatA::NORMAL,//NORMAL,
		-1,//REFLECTION,
	};

	ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());

	for (int i = 0; i < END_OF_MATERIAL; ++i){

		int texstore_point = matching[i];
		int check_mtl = 0;

		
		freadT(&check_mtl, sizeof(check_mtl), 1, fp);

		if (check_mtl == 0){

			if (texstore_point >= 0){

				texture_[texstore_point] = manager.get(std::string(""));
				if (NULL == texture_[texstore_point]){

					texture_[texstore_point] = new TextureBin();
					manager.insert(std::string(""), texture_[texstore_point]);
				}

				SAFE_ADDREF(texture_[texstore_point]);
				
			}
			continue;
		}
			

		char temp[MAX_ITEM_LEN] = { 0, };
		freadT(temp, sizeof(temp), MAX_ITEM_LEN, fp);

		

		if (texstore_point >= 0){

			texture_[texstore_point] = manager.get(std::string(temp));
			SAFE_ADDREF(texture_[texstore_point]);
		}
	}

	return true;
}