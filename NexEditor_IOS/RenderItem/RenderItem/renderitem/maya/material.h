#pragma once

#include "Vectors.h"
#include "Matrices.h"

typedef int                 BOOL;
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;

struct NinjaVertex;
struct NinjaVertexForLightmap;
struct SkinVertex;
struct ColoredVertex;

class Material{
public:
	Material(){

	}
	virtual ~Material(){

	}
	virtual Material* clone() = 0;
	virtual bool deserialize(FILE* fp) = 0;
	virtual bool serialize(FILE* fp) = 0;

	virtual void setEyeDir(float x, float y, float z){

		//Do print some debug strings.
	}

	virtual void setLightDir(float x, float y, float z){
		
		//Do print some debug strings.
	}

	virtual void setShaderSetting(NinjaVertex* pvertices, float x, float y, float z, float r){

		//Do print some debug strings.
	
	};

	virtual void setShaderSetting(NinjaVertexForLightmap* pvertices, float x, float y, float z, float r){

		//Do print some debug strings.	
	};

	virtual void setShaderSetting(SkinVertex* pvertices, float x, float y, float z, float r){
	
		//Do print some debug strings.
	};

	virtual void setShaderSetting(ColoredVertex* pvertices, float x, float y, float z, float r){

		//Do print some debug strings.
	};

	virtual void setShaderSetting(ColoredVertex* pvertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate){

		//Do print some debug strings.
	};
};

class Basic3DMaterial :public Material{

	TextureBin* diffuse_map_;
public:
	Basic3DMaterial();
	virtual ~Basic3DMaterial();
	virtual bool deserialize(FILE* fp);
	virtual bool serialize(FILE* fp);
};

struct TextureChunk
{
	unsigned int idx_;
	unsigned int size_of_compressed_;
	unsigned int size_of_uncompressed_;

	TextureChunk();

	~TextureChunk();

	void free();

	bool deserialize(FILE* fp);
};

enum
{
	MAT_NOMAT = 0,
	MAT_STDMAT,
	MAT_MSOMAT,
	MAT_SKINMAT,
	MAT_HAIRMAT,
	MAT_GEOMAT,
	MAT_SPLATMAT,
	MAT_EFFECTMAT,
	MAT_FXMAT = 100,
	MAT_BGFX_TERRAIN = 110,
	MAT_BGFX_A = 111,
	MAT_BGFX_B = 112,
	MAT_BGFX_C = 113,
	MAT_BGFX_SKY = 120,
	MAT_BGFX_WATER = 121,
	MAT_EFFECT_1,
	MAT_EFFECT_2,
	MAT_EFFECT_3,
	MAT_EFFECT_4,
	MAT_EFFECT_5,
	MAT_SOLID_COLOR,
	MAT_UNKNOWN = 999,
};

struct BaseMtl :public Material
{
	int		blend_;
	int		type_;
	DWORD	crc_;
	//-------------------------------------------------------------
	BaseMtl(){};
	virtual ~BaseMtl(){};
	Material* clone(){

		return NULL;
	}
};

struct NexMtl :BaseMtl
{
	NexMtl() :BaseMtl(){};

	virtual ~NexMtl()
	{

	}

	Material* clone(){

		return NULL;
	}
};

struct Lightmap
{
	Lightmap();

	virtual ~Lightmap();

	void deserialize(FILE* fp);
	BOOL			enable_;
	Vector3			color_;
};

#define MAX_ITEM_LEN	128
#define MAX_LIGHTMAP		4

struct MatC :NexMtl
{
	enum
	{
		DIFFUSE,
		SPEC,
		NORMAL,
		END_OF_MATERIAL,
	};
	
	static GLuint program_idx;
	static GLint vertex_attrib;
	static GLint texcoord_attrib;
	static GLint texcoord_for_lightmap_attrib;
	static GLint normal_attrib;
	static GLint tangent_attrib;
	static GLint binormal_attrib;
	static GLint eye_dir_uniform;
	static GLint light_dir_uniform;
	static GLint texture_uniform;
	static GLint texture_for_lightmap_uniform;
	static GLint spec_uniform;
	static GLint normal_uniform;
	static GLint world_matrix_uniform;
	static GLint mvp_matrix_uniform;

	float		bias_;
	BOOL		normal_enalbe_;
	float		normal_scale_;
	float		spec_level_;
	float		spec_gloss_;
	Vector4		spec_color_;
	float		lightmap_intensity_;
	float		lightmap_shift_;
	float		lightmap_add_;
	int			lightmap_channel_;
	Lightmap	lightmap_[MAX_LIGHTMAP];

	MatC();

	MatC(const MatC* pmtl);

	virtual ~MatC();

	Material* clone();

	bool deserialize(FILE* fp);
	bool serialize(FILE* fp);

	void setShaderSetting(NinjaVertexForLightmap* pvertices, float x, float y, float z, float r);
};

struct MatSolidColor :NexMtl{

	static GLuint program_idx;
	static GLint vertex_attrib;
	static GLint uv_attrib;
	static GLint color_attrib;
	static GLint texture_uniform;
	static GLint mvp_matrix_uniform;

	TextureBin* ptex_;

	MatSolidColor();

	MatSolidColor(const MatSolidColor* mtl);

	virtual ~MatSolidColor(){

		SAFE_RELEASE(ptex_);
	}

	Material* clone(){

		return new MatSolidColor();
	}

	void setTex(TextureBin* ptex){

		SAFE_RELEASE(ptex_);
		ptex_ = ptex;
		SAFE_ADDREF(ptex_);
	}

	bool deserialize(FILE *fp){

		return true;
	}

	bool serialize(FILE* fp){

		return true;
	}

	void setShaderSetting(ColoredVertex* pvertices, float x, float y, float z, float r);

	void setShaderSetting(ColoredVertex* pvertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate);
};

struct MatA :NexMtl{

	enum{

		DIFFUSE,
		SPEC,
		SSSMAP,
		NORMAL,
		END_OF_MATERIAL,
	};

	static GLuint program_idx;
	static GLint vertex_attrib;
	static GLint texcoord_attrib;
	static GLint normal_attrib;
	static GLint tangent_attrib;
	static GLint binormal_attrib;
	static GLint eye_dir_uniform;
	static GLint light_dir_uniform;
	static GLint texture_uniform;
	static GLint spec_uniform;
	static GLint normal_uniform;
	static GLint world_matrix_uniform;
	static GLint mvp_matrix_uniform;

	float burn_;
	float mid_;
	float bias_;
	Vector4 sss_color_;
	float sss_level_;
	float sss_width_;
	Vector4 rim_color_;
	float rim_density_;
	float rim_width_;
	float normal_scale_;
	Vector4 spec_color_;
	float spec_level_;
	float spec_gloss_;
	Vector4 brush_color_;
	float brush_center_;
	float brush_level_;
	float brush_gloss_;

	TextureBin* texture_[END_OF_MATERIAL];

	MatA();

	MatA(const MatA* mtl);

	virtual ~MatA(){

	}

	Material* clone(){

		return new MatA();
	}

	bool deserialize(FILE *fp);

	bool serialize(FILE* fp){

		return true;
	}

	void setEyeDir(float x, float y, float z);

	void setLightDir(float x, float y, float z);

	void setShaderSetting(NinjaVertex* pvertices, float x, float y, float z, float r);

	void setShaderSetting(SkinVertex* pvertices, float x, float y, float z, float r);
};

struct MatSkin :public MatA{

	enum
	{
		TOP_DIFFUSE = 0,
		BOTTOM_DIFFUSE,
		THIRD_DIFFUSE,
		SPEC_LEVEL,
		NORMAL,
		REFLECTION,
		END_OF_MATERIAL,
	};

	Material* clone();

	bool deserialize(FILE* fp);
};