#pragma once

#include <vector>

class Material;

class Mesh{

	Material* mtl_;
public:
	Mesh();
	virtual ~Mesh();

	Material* getMtl();
	void setMtl(Material* mtl);

	virtual Mesh* clone() = 0;
	virtual void free() = 0;
	virtual bool deserialize(FILE* fp) = 0;
	virtual bool serialize(FILE* fp) = 0;
	virtual void draw(float x, float y, float z, float r) = 0;
};

template<typename T, typename iT>
class BaseMesh:public Mesh{

	std::vector<T> vertices_;
	std::vector<iT> indices_;
public:
	typedef std::vector<T> VerticesType;
	typedef std::vector<iT> IndicesType;

	BaseMesh();
	virtual ~BaseMesh();

	Mesh* clone();
	virtual void free();
	virtual bool deserialize(FILE* fp);
	virtual bool serialize(FILE* fp);
	virtual void draw(float x, float y, float z, float r);
	std::vector<T>& getVertices();
	std::vector<iT>& getIndices();
};

class MeshGroup{

protected:
	std::vector<Mesh*> meshes_;
public:
	MeshGroup();
	virtual ~MeshGroup();

	void free();

	void draw(float x, float y, float z, float r);

	virtual MeshGroup* clone();
	virtual bool deserialize(FILE* fp);
	virtual bool serialize(FILE* fp);
};

void initMeshFactory();
void testBone(const char* filename);
void testAni(const char* filename);
void testMeshGroup(const char* filename);

struct NinjaVertexForLightmap
{
	Vector3 pos_;
	Vector3 normal_;
	Vector3 tangent_;
	Vector2 uv_;
	Vector2 uv_for_lightmap_[MAX_LIGHTMAP];

	void deserialize(FILE* fp);
};

struct NinjaVertex
{
	Vector3		pos_;//12
	Vector2		uv_;//8
	Vector3		normal_;//12
	Vector3		tangent_;
	Vector3		binormal_;


	void deserialize(FILE* fp);
};

struct ColoredVertex{

	Vector3 pos_;
	Vector3 color_;
	Vector2 uv_;
};

struct BoneAnimator;

class SkinGroup :public MeshGroup{

public:
	SkinGroup();
	~SkinGroup();

	virtual SkinGroup* clone();

	void draw(float x, float y, float z, float r, BoneAnimator& animator);
	virtual bool deserialize(FILE* fp);
	virtual bool serialize(FILE* fp);
};