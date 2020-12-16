#pragma once
#include "../Vectors.h"
#include "../Matrices.h"

typedef int                 BOOL;
typedef unsigned long       DWORD;
typedef unsigned short		WORD;
typedef unsigned char       BYTE;

struct NexVertex;
struct NexVertexForLightmap;
struct SkinVertex;
struct ColoredVertex;
struct EffectVertex;
struct RenderItemContext;
class InputStream;

template<typename T>
class RangedVector{

	std::vector<T> obj_;
public:
	size_t size(){

		return obj_.size();
	}

	T operator[](size_t i){

		if(i >= obj_.size())
			return obj_.back();
		return obj_[i];
	}

	std::vector<T>& get(){

		return obj_;
	}

	void push_back(T& val){

		obj_.push_back(val);
	}

	void clear(){

		obj_.clear();
	}
};

typedef RangedVector<float> Floatlist_t;
typedef RangedVector<Vector3> Vec3list_t;

class Material{

	bool alphablending_flag_;
public:
	Material(){

	}
	virtual ~Material(){

	}
	virtual Material* clone() = 0;

	virtual bool deserialize(InputStream& stream) = 0;

    virtual bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p)) = 0;

	bool getAlphaBlendingFlag(){

		return alphablending_flag_;
	}

	void setAlphaBlendingFlag(bool flag){

		alphablending_flag_ = flag;
	}

	virtual void setEyeDir(float x, float y, float z){

		//Do print some debug strings.
	}

	virtual void setLightDir(float x, float y, float z){
		
		//Do print some debug strings.
	}

	virtual void setShaderSetting(NexVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov){

		//Do print some debug strings.
	
	};

	virtual void setShaderSetting(NexVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, int frame, float aspect_ratio, float fov){

		//Do print some debug strings.
	
	};

	virtual void setShaderSetting(NexVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov){


	}

	virtual void setShaderSetting(EffectVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, float aspect_ratio, float fov){

		//Do print some debug strings.
	
	};

	virtual void setShaderSetting(EffectVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov){


	}

	virtual void setShaderSetting(NexVertexForLightmap* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov){

		//Do print some debug strings.	
	};

	virtual void setShaderSetting(SkinVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov){
	
		//Do print some debug strings.
	};

	virtual void setShaderSetting(ColoredVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov){

		//Do print some debug strings.
	};

	virtual void setShaderSetting(ColoredVertex* pvertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate, float aspect_ratio, float fov){

		//Do print some debug strings.
	};

	virtual int getWidth(){

		return 64;
	}

	virtual int getHeight(){

		return 64;
	}

	virtual int getPlaceHolderFlag(){

		return 0;
	}
};

class Basic3DMaterial :public Material{

	TextureBin* diffuse_map_;
public:
	Basic3DMaterial();
	virtual ~Basic3DMaterial();

	virtual bool deserialize(InputStream& stream);

    virtual bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));
};

struct TextureChunk
{
	unsigned int idx_;
	unsigned int size_of_compressed_;
	unsigned int size_of_uncompressed_;

	TextureChunk();

	~TextureChunk();

	void free();

	bool deserialize(InputStream& stream);

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));
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
	MAT_NEX_EFFECT_1 = 131,
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

	void deserialize(InputStream& stream);
    void deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void* prenderer));
	BOOL			enable_;
	Vector3			color_;
};

#define MAX_ITEM_LEN	128
#define MAX_LIGHTMAP		4

struct MatC_Shader{

	GLuint program_idx;
	GLint vertex_attrib;
	GLint texcoord_attrib;
	GLint texcoord_for_lightmap_attrib;
	GLint normal_attrib;
	GLint tangent_attrib;
	GLint binormal_attrib;
	GLint eye_dir_uniform;
	GLint light_dir_uniform;
	GLint texture_uniform;
	GLint texture_for_lightmap_uniform;
	GLint spec_uniform;
	GLint normal_uniform;
	GLint world_matrix_uniform;
	GLint mvp_matrix_uniform;

	MatC_Shader();
	~MatC_Shader();
};


struct MatC :NexMtl
{
	enum
	{
		DIFFUSE,
		SPEC,
		NORMAL,
		END_OF_MATERIAL,
	};

	int origin_flag_;
	MatC_Shader& shader_;

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

	MatC(MatC_Shader& shader, int origin_flag = 0);

	MatC(const MatC* pmtl);

	virtual ~MatC();

	Material* clone();

	bool deserialize(InputStream& stream);

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

	void setShaderSetting(NexVertexForLightmap* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov);
};

struct MatSolidColor_Shader{

	GLuint program_idx;
	GLint vertex_attrib;
	GLint uv_attrib;
	GLint color_attrib;
	GLint texture_uniform;
	GLint mvp_matrix_uniform;

	MatSolidColor_Shader();
	~MatSolidColor_Shader();
};

struct MatSolidColor :NexMtl{

	int origin_flag_;
	MatSolidColor_Shader& shader_;

	TextureBin* ptex_;

	MatSolidColor(MatSolidColor_Shader& shader, int origin_flag = 0);

	MatSolidColor(const MatSolidColor* mtl);

	virtual ~MatSolidColor(){

		if(origin_flag_)
			delete &shader_;
		SAFE_RELEASE(ptex_);
	}

	Material* clone(){

		return new MatSolidColor(shader_);
	}

	void setTex(TextureBin* ptex){

		SAFE_RELEASE(ptex_);
		ptex_ = ptex;
		SAFE_ADDREF(ptex_);
	}

	bool deserialize(InputStream& stream){

		return true;
	}

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p)){

        return true;
    }

	void setShaderSetting(ColoredVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov);

	void setShaderSetting(ColoredVertex* pvertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate, float aspect_ratio, float fov);
};

struct MatA_Shader{

	GLuint program_idx;
	GLint vertex_attrib;
	GLint texcoord_attrib;
	GLint normal_attrib;
	GLint tangent_attrib;
	GLint binormal_attrib;
	GLint eye_dir_uniform;
	GLint light_dir_uniform;
	GLint texture_uniform;
	GLint spec_uniform;
	GLint normal_uniform;
	GLint world_matrix_uniform;
	GLint mvp_matrix_uniform;

	MatA_Shader();
	~MatA_Shader();
};

struct MatA :NexMtl{

	enum{

		DIFFUSE,
		SPEC,
		SSSMAP,
		NORMAL,
		END_OF_MATERIAL,
	};

	int origin_flag_;
	MatA_Shader& shader_;

	

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

	MatA(MatA_Shader& shader, int origin_flag = 0);

	MatA(const MatA* mtl);

	virtual ~MatA(){

		if(origin_flag_)
			delete &shader_;
	}

	Material* clone();

	bool deserialize(InputStream& stream);

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

	void setEyeDir(float x, float y, float z);

	void setLightDir(float x, float y, float z);

	void setShaderSetting(NexVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov);

	void setShaderSetting(SkinVertex* pvertices, float x, float y, float z, float r, float aspect_ratio, float fov);

	void setShaderSetting(NexVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float aspect_ratio, float fov);
};

struct MatSkin_Shader{

	GLuint program_idx;
	GLint vertex_attrib;
	GLint texcoord_attrib;
	GLint normal_attrib;
	GLint eye_dir_uniform;
	GLint light_dir_uniform;
	GLint texture_uniform;
	GLint spec_uniform;
	GLint world_matrix_uniform;
	GLint mvp_matrix_uniform;
	GLint tex_matrix_uniform;
	GLint color_matrix_uniform;
	GLint opacity_uniform;
	GLint check_diffusemap_uniform;
	GLint check_specmap_uniform;
	GLint check_alphatest_uniform;
	GLint system_source_uniform;
	GLint diffuse_color_uniform;
	GLint ambient_color_uniform;
	GLint spec_color_uniform;
	GLint spec_level_uniform;
	GLint spec_gloss_uniform;
	GLint real_x_uniform;
	GLint real_y_uniform;

	MatSkin_Shader();
	~MatSkin_Shader();
};

struct MatSkin :NexMtl{

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

	int origin_flag_;
	MatSkin_Shader& shader_;
	int texture_option_[END_OF_MATERIAL];
	TextureBin* texture_[END_OF_MATERIAL];

	Vector3 diffuse_color_;
	Vector3 ambient_color_;
	Vector3 specular_color_;
	float specular_level_;
	float glossiness_;
	float opacity_;
	Vec3list_t diffuse_list_;
	Vec3list_t ambient_list_;
	Vec3list_t specular_list_;
	Floatlist_t specular_level_list_;
	Floatlist_t glossiness_list_;
	Floatlist_t opacity_list_;

	MatSkin(MatSkin_Shader& shader, int origin_flag = 0);

	virtual ~MatSkin(){

		if(origin_flag_)
			delete &shader_;
	}

	Material* clone();

	int getWidth();

	int getHeight();

	int getPlaceHolderFlag();

	void setEyeDir(float x, float y, float z);

	void setLightDir(float x, float y, float z);

	bool deserialize(InputStream& stream);

	bool deserialize_v10(InputStream& stream);

	bool deserialize_v11(InputStream& stream);

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

    bool deserialize_v10(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

    bool deserialize_v11(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

	void setShaderSetting(NexVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, int frame, float aspect_ratio, float fov);

	void setShaderSetting(NexVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov);
};

struct MatNexEffect01_Shader{

	GLuint program_idx;
	GLint vertex_attrib;
	GLint texcoord_attrib;
	GLint mvp_matrix_uniform;
	GLint diffuse_level_uniform;
	GLint alpha_level_uniform;
	GLint check_alphatest_uniform;
	GLint diffuse_color_uniform;
	GLint texture_uniform;
	GLint alphatexture_uniform;

	MatNexEffect01_Shader();
	~MatNexEffect01_Shader();
};

struct MatNexEffect01 :NexMtl{

	enum
	{
		DIFFUSE,
		OPACITY,
		END_OF_MATERIAL,
	};

	int origin_flag_;
	MatNexEffect01_Shader& shader_;
	TextureBin* texture_[END_OF_MATERIAL];

	Vector3 diffuse_color_;
	float diffuse_level_;
	float alpha_level_;
	//////////////////////////////////////////////////////////////////////////
	Floatlist_t alpha_level_list_;
	Floatlist_t diffuse_level_list_;
	//////////////////////////////////////////////////////////////////////////

	MatNexEffect01(MatNexEffect01_Shader& shader, int origin_flag = 0);

	virtual ~MatNexEffect01(){

		if(origin_flag_)
			delete &shader_;
	}

	Material* clone();

	int getWidth();

	int getHeight();

	int getPlaceHolderFlag();

	bool deserialize(InputStream& stream);

    bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p));

	void setShaderSetting(EffectVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, float aspect_ratio, float fov);

	void setShaderSetting(EffectVertex* pvertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov);
};
