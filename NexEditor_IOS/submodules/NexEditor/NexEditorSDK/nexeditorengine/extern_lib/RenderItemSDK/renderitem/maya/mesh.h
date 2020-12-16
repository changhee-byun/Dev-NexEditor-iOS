/******************************************************************************
* File Name   :	mesh.h
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#pragma once
#include "material.h"

#include <vector>

class Material;

class InputStream;

class Mesh{

	Material* mtl_;
public:
	Mesh();
	virtual ~Mesh();

	Material* getMtl();
	void setMtl(Material* mtl);

	virtual Mesh* clone() = 0;
	virtual void free() = 0;
	virtual bool deserialize(InputStream& stream) = 0;
    virtual bool deserialize(InputStream& stream, char* uid, void* renderer, void (*pAquireContextFunc)(void*), void (*pReleaseContextFunc)(void*)) = 0;
	virtual void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov) = 0;
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
	virtual bool deserialize(InputStream& stream);
    virtual bool deserialize(InputStream& stream, char* uid, void* renderer, void (*pAquireContextFunc)(void*), void (*pReleaseContextFunc)(void*));
	virtual void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov);
	std::vector<T>& getVertices();
	std::vector<iT>& getIndices();
};

class MeshGroup{

protected:
	std::vector<Mesh*> meshes_;
public:
	MeshGroup();
	virtual ~MeshGroup();

	virtual void free();

	virtual void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov);

	virtual void draw(RenderItemContext* pctx, float frame, int ab_flag, int reverse, float aspect_ratio, float fov);

	virtual void drawFull(RenderItemContext* pctx, float frame, int reverse, float aspect_ratio, float fov);

	virtual int getFrame();

	virtual MeshGroup* clone();
	virtual bool deserialize(InputStream& stream);
    virtual bool deserialize(InputStream& stream, char* uid, void* renderer, void (*pAquireContextFunc)(void*), void (*pReleaseContextFunc)(void*));

	virtual int getTexWidthOnSlot(int placeholder_id);
	virtual int getTexHeightOnSlot(int placeholder_id);
};

struct NexVertexForLightmap
{
	Vector3 pos_;
	Vector3 normal_;
	Vector3 tangent_;
	Vector2 uv_;
	Vector2 uv_for_lightmap_[MAX_LIGHTMAP];

	void deserialize(InputStream& stream);
};

struct EffectVertex{

	Vector3 pos_;
	Vector2 uv_;
};

struct NexVertex
{
	Vector3		pos_;//12
	Vector2		uv_;//8
	Vector3		normal_;//12
	// Vector3		tangent_;
	// Vector3		binormal_;

	void deserialize(InputStream& stream);
};

struct ColoredVertex{

	Vector3 pos_;
	Vector3 color_;
	Vector2 uv_;
};

struct BoneAnimator;

struct MatrixKey{

	int frame_;
	Matrix4 matrix_;
};

typedef std::vector<MatrixKey> Matrixkeylist_t;

class Track{

	int			index_for_parent_bone_;
	int			index_for_bone_;
	Matrix4		matrix_for_world_;
	Matrixkeylist_t matrixkey_list_;

public:
	void init();

	bool deserializerMatrixKey(InputStream& stream, int total_frame);

	Matrix4 getMatrix(float frame);

	int getBoneIndex(){

		return index_for_bone_;
	}
};

class SkinGroup :public MeshGroup{

public:
	SkinGroup();
	~SkinGroup();

	virtual MeshGroup* clone();

	void draw(float x, float y, float z, float r, BoneAnimator& animator, float aspect_ratio, float fov);
	virtual bool deserialize(InputStream& stream);
    virtual bool deserialize(InputStream& stream, char* uid, void* renderer, void (*pAquireContextFunc)(void*), void (*pReleaseContextFunc)(void*));
};

typedef std::vector<Track*> TrackList_t;

class EffectGroup:public MeshGroup{

	int count_of_frame_;
	TrackList_t node_list_;
	Track track_;

	void prepare(Matrix4* pmatrices, Matrix4& view, Vector3& light_dir, Vector3& eye_dir, float frame, int reverse);
public:
	EffectGroup();
	~EffectGroup();

	virtual MeshGroup* clone();
	virtual int getFrame();
	virtual void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov);
	virtual void draw(RenderItemContext* pctx, float frame, int ab_flag, int reverse, float aspect_ratio, float fov);
	virtual void drawFull(RenderItemContext* pctx, float frame, int reverse, float aspect_ratio, float fov);
	virtual bool deserialize(InputStream& stream);
    virtual bool deserialize(InputStream& stream, char* uid, void* renderer, void (*pAquireContextFunc)(void*), void (*pReleaseContextFunc)(void*));
};