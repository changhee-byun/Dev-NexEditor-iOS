/******************************************************************************
* File Name   :	mesh.cpp
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

#include "../types.h"
#include "material.h"
#include "mesh.h"
#include "../Matrices.h"
#include "../Vectors.h"
#include "../util.h"
#include "../RenderItem.h"
#include "NexSAL_Internal.h"

#define MAX_BONE_INDEX	4

class BaseAniMesh;

struct BoneWeight{

	int idx_;
	float weight_;
};

struct SkinVertex_deprecated
{
	Vector3		pos_;//12
	Vector3		normal_;//12
	Vector3		tangent_;
	Vector3		binormal_;
	Vector2		uv_;//8
	BoneWeight	weight_[MAX_BONE_INDEX];//32..64 8000 * 64 = 512000 

	void deserialize(InputStream& stream){

		stream.read(&pos_, sizeof(pos_), 1);
		stream.read(&uv_, sizeof(uv_), 1);
		stream.read(&normal_, sizeof(normal_), 1);
		stream.read(&tangent_, sizeof(tangent_), 1);
		stream.read(&binormal_, sizeof(binormal_), 1);

		for (int i = 0; i < MAX_BONE_INDEX; ++i){

			stream.read(&weight_[i].idx_, sizeof(weight_[i].idx_), 1);
			stream.read(&weight_[i].weight_, sizeof(weight_[i].weight_), 1);
		}
	}
};

struct SkinVertex{

	BoneWeight weight_[MAX_BONE_INDEX];

	Vector3 pos_;
	Vector2 uv_;
	Vector3 normal_;
	Vector3	tangent_;
	Vector3	binormal_;

	void deserialize(InputStream& stream){

		stream.read(&pos_, sizeof(pos_), 1);
		stream.read(&uv_, sizeof(uv_), 1);
		stream.read(&normal_, sizeof(normal_), 1);
		stream.read(&tangent_, sizeof(tangent_), 1);
		stream.read(weight_, sizeof(weight_), MAX_BONE_INDEX);
	}

	void operator=(const SkinVertex_deprecated& data){

		pos_ = data.pos_;
		normal_ = data.normal_;
		tangent_ = data.tangent_;
		binormal_ = data.binormal_;
		uv_ = data.uv_;
		for (int i = 0; i < MAX_BONE_INDEX; ++i)
			weight_[i] = data.weight_[i];
	}
};

class Skin :public BaseMesh<SkinVertex, unsigned short>{

public:
	Skin(){

	}
	virtual ~Skin(){

	}
	virtual Mesh* clone(){

		return new Skin();
	}

	void draw(float x, float y, float z, float r, BoneAnimator& animator, float aspect_ratio, float fov);
};

void NexVertexForLightmap::deserialize(InputStream& stream){

	Vector3 binormal;

	stream.read(&pos_, sizeof(pos_), 1);
	stream.read(&uv_, sizeof(uv_), 1);
	stream.read(&normal_, sizeof(normal_), 1);
	stream.read(&binormal, sizeof(binormal), 1);
	stream.read(&tangent_, sizeof(tangent_), 1);
	stream.read(uv_for_lightmap_, sizeof(uv_for_lightmap_), MAX_LIGHTMAP);
}

void NexVertex::deserialize(InputStream& stream){

	// Vector3 binormal;

	stream.read(&pos_, sizeof(pos_), 1);
	stream.read(&uv_, sizeof(uv_), 1);
	stream.read(&normal_, sizeof(normal_), 1);
	// normal_.x = -normal_.x;
	// normal_.z = -normal_.z;
	// stream.read(&binormal, sizeof(binormal), 1);
	// stream.read(&tangent_, sizeof(tangent_), 1);
}

Mesh::Mesh():mtl_(NULL){


}

Mesh::~Mesh(){

	SAFE_DELETE(mtl_);
}

Material* Mesh::getMtl(){

	return mtl_;
}

void Mesh::setMtl(Material* mtl){

	SAFE_DELETE(mtl_);
	mtl_ = mtl;
}

template<typename T, typename iT>
BaseMesh<T, iT>::BaseMesh(){


}

template<typename T, typename iT>
BaseMesh<T, iT>::~BaseMesh(){


}

template<typename T, typename iT>
Mesh* BaseMesh<T, iT>::clone(){

	return new BaseMesh<T, iT>();
}

template<typename T, typename iT>
bool BaseMesh<T, iT>::deserialize(InputStream& stream){

	free();
	
	int count_of_vertices = 0;
	int count_of_faces = 0;

	stream.read(&count_of_vertices, sizeof(int), 1);
	vertices_.reserve(count_of_vertices);

	for (int i = 0; i < count_of_vertices; ++i){

		T vtx;
		vtx.deserialize(stream);
		vertices_.push_back(vtx);
	}

	stream.read(&count_of_faces, sizeof(int), 1);
	indices_.reserve(count_of_faces * 3);

	for (int i = 0; i < count_of_faces; ++i){

		iT vtx[3];
		stream.read(vtx, sizeof(iT), 3);
		indices_.push_back(vtx[0]);
		indices_.push_back(vtx[1]);
		indices_.push_back(vtx[2]);
	}

	//vertices_.shrink_to_fit();
	//indices_.shrink_to_fit();

	return true;
}

template<typename T, typename iT>
bool BaseMesh<T, iT>::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void*)){

	free();
	
	int count_of_vertices = 0;
	int count_of_faces = 0;

	stream.read(&count_of_vertices, sizeof(int), 1);
	vertices_.reserve(count_of_vertices);

	for (int i = 0; i < count_of_vertices; ++i){

		T vtx;
		vtx.deserialize(stream);
		vertices_.push_back(vtx);
	}

	stream.read(&count_of_faces, sizeof(int), 1);
	indices_.reserve(count_of_faces * 3);

	for (int i = 0; i < count_of_faces; ++i){

		iT vtx[3];
		stream.read(vtx, sizeof(iT), 3);
		indices_.push_back(vtx[0]);
		indices_.push_back(vtx[1]);
		indices_.push_back(vtx[2]);
	}

	//vertices_.shrink_to_fit();
	//indices_.shrink_to_fit();

	return true;
}

template<>
bool BaseMesh<SkinVertex, unsigned short>::deserialize(InputStream& stream){

	free();

	int count_of_vertices = 0;
	int count_of_faces = 0;

	stream.read(&count_of_vertices, sizeof(int), 1);
	vertices_.reserve(count_of_vertices);

	for (int i = 0; i < count_of_vertices; ++i){

		SkinVertex_deprecated vtx_old;
		SkinVertex vtx;
		vtx_old.deserialize(stream);
		vtx = vtx_old;
		vertices_.push_back(vtx);
	}

	stream.read(&count_of_faces, sizeof(int), 1);
	count_of_faces /= 3;
	indices_.reserve(count_of_faces * 3);

	for (int i = 0; i < count_of_faces; ++i){

		int vtx[3];
		stream.read(vtx, sizeof(int), 3);
		indices_.push_back(vtx[0]);
		indices_.push_back(vtx[1]);
		indices_.push_back(vtx[2]);
	}

	return true;
}

template<typename T, typename iT>
void BaseMesh<T, iT>::free(){

	setMtl(NULL);
	vertices_.clear();
	indices_.clear();
}

template<typename T, typename iT>
std::vector<T>& BaseMesh<T, iT>::getVertices(){

	return vertices_;
}

template<typename T, typename iT>
std::vector<iT>& BaseMesh<T, iT>::getIndices(){

	return indices_;
}

struct Tris
{
	WORD m_wIndex[ 3 ];
};

struct TriGroup
{
	TriGroup():ptris_( NULL )
	{

	};
	~TriGroup()
	{
		SAFE_DELETE(ptris_);
	}
	Tris* ptris_;
};

struct Vertices
{
	Vertices():count_of_vertices_( 0 ), pvertices_( NULL )
	{

	};
	~Vertices()
	{
		if(pvertices_)
			free(pvertices_);
	}

	int		time_value_;
	int		count_of_vertices_;
	void*	pvertices_;
};

struct NexParticle{

	Vector3 m_vecPos;
	float m_fRadius;
};

void MatSolidColor::setShaderSetting(ColoredVertex* vertices, float x, float y, float z, float r, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	trans.translate(x, y, z);
	rotate.rotateX(r);

	world = trans * rotate;
	matrix = matrix * world;

	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.color_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.color_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &vertices[0].color_);    CHECK_GL_ERROR();
}

void MatSolidColor::setShaderSetting(ColoredVertex* vertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	trans.translate(x, y, z);
	rotate.rotateX(xrotate);
	rotate.rotateY(yrotate);	
	rotate.rotateZ(zrotate);

	world = trans * rotate;
	matrix = matrix * view * world;

	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());

	if (ptex_){
		glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex_->getTexID());                                   CHECK_GL_ERROR();
		glUniform1i(shader_.texture_uniform, 0);                                           CHECK_GL_ERROR();
	}

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.uv_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.color_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.uv_attrib, 2, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].uv_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.color_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &vertices[0].color_);    CHECK_GL_ERROR();
}

void MatA::setShaderSetting(SkinVertex* vertices, float x, float y, float z, float r, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	trans.translate(x, y, z);
	rotate.rotateY(r);

	world = trans * rotate;
	matrix = matrix * world;

	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(shader_.world_matrix_uniform, 1, false, world.get());
	glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.texture_uniform, 0);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[NORMAL]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.normal_uniform, 1);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 2);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[SPEC]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.spec_uniform, 2);                                           CHECK_GL_ERROR();

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.tangent_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.binormal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.tangent_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].tangent_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.binormal_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].binormal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

void MatA::setShaderSetting(NexVertex* vertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	Matrix4 matrix;
	Matrix4 view(pmatrix_for_view);
	Matrix4 world(pmatrix_for_world);

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	matrix = matrix * view * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(shader_.world_matrix_uniform, 1, false, world.get());
	glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
	if(texture_[0])
		glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.texture_uniform, 0);                                           CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
	if(texture_[NORMAL])
		glBindTexture(GL_TEXTURE_2D, texture_[NORMAL]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.normal_uniform, 1);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 2);                                             CHECK_GL_ERROR();
	if(texture_[SPEC])
		glBindTexture(GL_TEXTURE_2D, texture_[SPEC]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.spec_uniform, 2);                                           CHECK_GL_ERROR();

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	// glEnableVertexAttribArray(shader_.tangent_attrib); CHECK_GL_ERROR();
	// glEnableVertexAttribArray(shader_.binormal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	// glVertexAttribPointer(shader_.tangent_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].tangent_);    CHECK_GL_ERROR();
	// glVertexAttribPointer(shader_.binormal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].binormal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
};

void MatA::setShaderSetting(NexVertex* vertices, float x, float y, float z, float r, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	trans.translate(x, y, z);
	rotate.rotateY(r);

	world = trans * rotate;
	matrix = matrix * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(shader_.world_matrix_uniform, 1, false, world.get());
	glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
	if(texture_[0])
		glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.texture_uniform, 0);                                           CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
	if(texture_[NORMAL])
		glBindTexture(GL_TEXTURE_2D, texture_[NORMAL]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.normal_uniform, 1);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 2);                                             CHECK_GL_ERROR();
	if(texture_[SPEC])
		glBindTexture(GL_TEXTURE_2D, texture_[SPEC]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(shader_.spec_uniform, 2);                                           CHECK_GL_ERROR();

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	// glEnableVertexAttribArray(shader_.tangent_attrib); CHECK_GL_ERROR();
	// glEnableVertexAttribArray(shader_.binormal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	// glVertexAttribPointer(shader_.tangent_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].tangent_);    CHECK_GL_ERROR();
	// glVertexAttribPointer(shader_.binormal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].binormal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

void MatC::setShaderSetting(NexVertexForLightmap* vertices, float x, float y, float z, float r, float aspect_ratio, float fov){

	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.tangent_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_for_lightmap_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.vertex_attrib, sizeof(vertices[0].pos_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].pos_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, sizeof(vertices[0].normal_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.tangent_attrib, sizeof(vertices[0].tangent_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].tangent_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, sizeof(vertices[0].uv_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].uv_);  CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_for_lightmap_attrib, sizeof(vertices[0].uv_for_lightmap_[lightmap_channel_]) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].uv_for_lightmap_[lightmap_channel_]);  CHECK_GL_ERROR();
}

static int alphatest_values[3] = {255, -255, 0};

void MatSkin::setShaderSetting(NexVertex* vertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov){

	static Matrix4 idmatrix(1, 0, 0, 0, 
							0, 1, 0, 0, 
							0, 0, 1, 0, 
							0, 0, 0, 1);
	glUseProgram(shader_.program_idx);

	int alphatest_value = alphatest_values[ab_flag];
	glUniform1i(shader_.check_alphatest_uniform, alphatest_value);

	Matrix4 matrix;
	Matrix4 view(pmatrix_for_view);
	Matrix4 world(pmatrix_for_world);

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	matrix = matrix * view * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(shader_.world_matrix_uniform, 1, false, world.get());
	if(opacity_list_.size() > 0)
		glUniform1f(shader_.opacity_uniform, opacity_list_[frame]);
	else
		glUniform1f(shader_.opacity_uniform, opacity_);

	if(specular_level_list_.size() > 0)
		glUniform1f(shader_.spec_level_uniform, specular_level_list_[frame]);
	else
		glUniform1f(shader_.spec_level_uniform, specular_level_);

	if(glossiness_list_.size() > 0)
		glUniform1f(shader_.spec_gloss_uniform, glossiness_list_[frame] * 100.0f);
	else
		glUniform1f(shader_.spec_gloss_uniform, glossiness_ * 100.0f);
	

	if(texture_[0]){

		glUniform1i(shader_.check_diffusemap_uniform, 1); CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE0 + 0);                 CHECK_GL_ERROR();

		int src_id = texture_[0]->getPlaceHolderFlag();
		int tex_id = src_id > 0?pctx->getVideoSrc(src_id):texture_[0]->getTexID();
		bool is_video_src = src_id > 0 && (src_id&0x0F) <= TextureBin::VID1_PLACE_HOLDER && tex_id > 0;
		bool is_system_src = src_id > 0 && (src_id&0x0F) <= TextureBin::TEXT_PLACE_HOLDER && tex_id > 0;

		if(texture_option_[0] & (0x02|0x08|0x04|0x10)){

		}
		else{
			
			is_system_src = true;
		}

		const float* pmatrix_for_video_src = is_video_src?pctx->getTexMatrixForSrc(src_id):idmatrix.get();
		const float* pmatrix_for_colorconv = is_video_src?pctx->getColorMatrixForSrc(src_id):idmatrix.get();
		float real_x = is_video_src?pctx->getRealXForSrc(src_id):is_system_src?1.0f:10000.0f;
		float real_y = is_video_src?pctx->getRealYForSrc(src_id):is_system_src?1.0f:10000.0f;

		glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, pmatrix_for_video_src);
		glUniformMatrix4fv(shader_.color_matrix_uniform, 1, false, pmatrix_for_colorconv);
		glUniform1f(shader_.real_x_uniform, real_x);
		glUniform1f(shader_.real_y_uniform, real_y);

		
		glUniform1i(shader_.system_source_uniform, is_system_src?1:0);
		glBindTexture(GL_TEXTURE_2D, tex_id?tex_id:texture_[0]->getTexID());           CHECK_GL_ERROR();
		glUniform1i(shader_.texture_uniform, 0);           CHECK_GL_ERROR();
	}
	else{

		glUniform1i(shader_.check_diffusemap_uniform, 0); CHECK_GL_ERROR();
		if(diffuse_list_.size() > 0)
			glUniform4f(shader_.diffuse_color_uniform, diffuse_list_[frame].x, diffuse_list_[frame].y, diffuse_list_[frame].z, 1.0f);
		else
			glUniform4f(shader_.diffuse_color_uniform, diffuse_color_.x, diffuse_color_.y, diffuse_color_.z, 1.0f);
	}

	if(texture_[SPEC_LEVEL]){

		glUniform1i(shader_.check_specmap_uniform, 1); CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, texture_[SPEC_LEVEL]->getTexID());                                   CHECK_GL_ERROR();
		glUniform1i(shader_.spec_uniform, 1);                                           CHECK_GL_ERROR();
	}
	else{

		glUniform1i(shader_.check_specmap_uniform, 0); CHECK_GL_ERROR();
		if(specular_list_.size() > 0)
			glUniform3f(shader_.spec_color_uniform, specular_list_[frame].x, specular_list_[frame].y, specular_list_[frame].z);
		else
			glUniform3f(shader_.spec_color_uniform, specular_color_.x, specular_color_.y, specular_color_.z);
	}

	if(ambient_list_.size() > 0)
		glUniform3f(shader_.ambient_color_uniform, ambient_list_[frame].x, ambient_list_[frame].y, ambient_list_[frame].z);
	else
		glUniform3f(shader_.ambient_color_uniform, ambient_color_.x, ambient_color_.y, ambient_color_.z); CHECK_GL_ERROR();
	
	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

void MatSkin::setShaderSetting(NexVertex* vertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, int frame, float aspect_ratio, float fov){

	static Matrix4 idmatrix(1, 0, 0, 0, 
							0, 1, 0, 0, 
							0, 0, 1, 0, 
							0, 0, 0, 1);
	glUseProgram(shader_.program_idx);

	int alphatest_value = alphatest_values[ab_flag];
	glUniform1i(shader_.check_alphatest_uniform, alphatest_value);

	Matrix4 matrix;
	Matrix4 view(pmatrix_for_view);
	Matrix4 world(pmatrix_for_world);

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	matrix = matrix * view * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(shader_.world_matrix_uniform, 1, false, world.get());
	glUniformMatrix4fv(shader_.color_matrix_uniform, 1, false, idmatrix.get());
	if(opacity_list_.size() > 0)
		glUniform1f(shader_.opacity_uniform, opacity_list_[frame]);
	else
		glUniform1f(shader_.opacity_uniform, opacity_);

	if(specular_level_list_.size() > 0)
		glUniform1f(shader_.spec_level_uniform, specular_level_list_[frame]);
	else
		glUniform1f(shader_.spec_level_uniform, specular_level_);

	if(glossiness_list_.size() > 0)
		glUniform1f(shader_.spec_gloss_uniform, glossiness_list_[frame] * 100.0f);
	else
		glUniform1f(shader_.spec_gloss_uniform, glossiness_ * 100.0f);

	if(texture_[0]){

		glUniform1i(shader_.check_diffusemap_uniform, 1); CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
		switch(texture_[0]->getPlaceHolderFlag()){

			case TextureBin::NON_PLACE_HOLDER:
				glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, idmatrix.get());
				glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
				glUniform1f(shader_.real_x_uniform, 10000.0f);
				glUniform1f(shader_.real_y_uniform, 10000.0f);
			break;
			case TextureBin::VID0_PLACE_HOLDER:
				if(pmatrix_for_video_src_0){
					glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, pmatrix_for_video_src_0);
					glUniform1f(shader_.real_x_uniform, 1.0f);
					glUniform1f(shader_.real_y_uniform, 1.0f);
				}
				else{
					glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, idmatrix.get());
					glUniform1f(shader_.real_x_uniform, 10000.0f);
					glUniform1f(shader_.real_y_uniform, 10000.0f);
				}
				glBindTexture(GL_TEXTURE_2D, video_src_0?video_src_0:texture_[0]->getTexID());           CHECK_GL_ERROR();
			break;
			case TextureBin::VID1_PLACE_HOLDER:
				if(pmatrix_for_video_src_1){
					glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, pmatrix_for_video_src_1);
					glUniform1f(shader_.real_x_uniform, 1.0f);
					glUniform1f(shader_.real_y_uniform, 1.0f);
				}
				else{
					glUniformMatrix4fv(shader_.tex_matrix_uniform, 1, false, idmatrix.get());
					glUniform1f(shader_.real_x_uniform, 10000.0f);
					glUniform1f(shader_.real_y_uniform, 10000.0f);
				}
				glBindTexture(GL_TEXTURE_2D, video_src_1?video_src_1:texture_[0]->getTexID());           CHECK_GL_ERROR();
			break;

		};		
		glUniform1i(shader_.texture_uniform, 0);                                           CHECK_GL_ERROR();
	}
	else{

		glUniform1i(shader_.check_diffusemap_uniform, 0); CHECK_GL_ERROR();
		if(diffuse_list_.size() > 0)
			glUniform4f(shader_.diffuse_color_uniform, diffuse_list_[frame].x, diffuse_list_[frame].y, diffuse_list_[frame].z, 1.0f);
		else
			glUniform4f(shader_.diffuse_color_uniform, diffuse_color_.x, diffuse_color_.y, diffuse_color_.z, 1.0f);
	}

	if(texture_[SPEC_LEVEL]){

		glUniform1i(shader_.check_specmap_uniform, 1); CHECK_GL_ERROR();
		glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, texture_[SPEC_LEVEL]->getTexID());                                   CHECK_GL_ERROR();
		glUniform1i(shader_.spec_uniform, 1);                                           CHECK_GL_ERROR();
	}
	else{

		glUniform1i(shader_.check_specmap_uniform, 0); CHECK_GL_ERROR();
		if(specular_list_.size() > 0)
			glUniform3f(shader_.spec_color_uniform, specular_list_[frame].x, specular_list_[frame].y, specular_list_[frame].z);
		else
			glUniform3f(shader_.spec_color_uniform, specular_color_.x, specular_color_.y, specular_color_.z);
	}

	if(ambient_list_.size() > 0)
		glUniform3f(shader_.ambient_color_uniform, ambient_list_[frame].x, ambient_list_[frame].y, ambient_list_[frame].z);
	else
		glUniform3f(shader_.ambient_color_uniform, ambient_color_.x, ambient_color_.y, ambient_color_.z); CHECK_GL_ERROR();
	
	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.normal_attrib, 3, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(NexVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
};

void MatNexEffect01::setShaderSetting(EffectVertex* vertices, const float* pmatrix_for_view, const float* pmatrix_for_world, int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, int ab_flag, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	int alphatest_value = alphatest_values[ab_flag];
	glUniform1i(shader_.check_alphatest_uniform, alphatest_value);

	Matrix4 matrix;
	Matrix4 view(pmatrix_for_view);
	Matrix4 world(pmatrix_for_world);

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	matrix = matrix * view * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());

	for(int i = 0; i < END_OF_MATERIAL; ++i){

		glActiveTexture(GL_TEXTURE0 + i);                 		CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, texture_[i]->getTexID());  CHECK_GL_ERROR();
		glUniform1i(shader_.texture_uniform, i);           		CHECK_GL_ERROR();
	}

	glUniform1f(shader_.diffuse_level_uniform, diffuse_level_);
	glUniform1f(shader_.alpha_level_uniform, alpha_level_);
	glUniform3f(shader_.diffuse_color_uniform, diffuse_color_.x, diffuse_color_.y, diffuse_color_.z);
	
	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(EffectVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(EffectVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}



void MatNexEffect01::setShaderSetting(EffectVertex* vertices, const float* pmatrix_for_view, const float* pmatrix_for_world, RenderItemContext* pctx, int ab_flag, int frame, float aspect_ratio, float fov){

	glUseProgram(shader_.program_idx);

	int alphatest_value = alphatest_values[ab_flag];
	glUniform1i(shader_.check_alphatest_uniform, alphatest_value);

	Matrix4 matrix;
	Matrix4 view(pmatrix_for_view);
	Matrix4 world(pmatrix_for_world);

	matrix.perspective(fov, aspect_ratio, 1.0f, 1500.0f);
	matrix = matrix * view * world;
		
	glUniformMatrix4fv(shader_.mvp_matrix_uniform, 1, false, matrix.get());

	for(int i = 0; i < END_OF_MATERIAL; ++i){

		glActiveTexture(GL_TEXTURE0 + i);                 		CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, texture_[i]->getTexID());  CHECK_GL_ERROR();
		glUniform1i(shader_.texture_uniform, i);           		CHECK_GL_ERROR();
	}

	float diffuse_level = diffuse_level_list_[frame];
	float alpha_level = alpha_level_list_[frame];

	glUniform1f(shader_.diffuse_level_uniform, diffuse_level);
	glUniform1f(shader_.alpha_level_uniform, alpha_level);
	glUniform3f(shader_.diffuse_color_uniform, diffuse_color_.x, diffuse_color_.y, diffuse_color_.z);
	
	glEnableVertexAttribArray(shader_.vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(shader_.texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(shader_.vertex_attrib, 3, GL_FLOAT, false, sizeof(EffectVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(shader_.texcoord_attrib, 2, GL_FLOAT, false, sizeof(EffectVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

template<typename T, typename iT>
void BaseMesh<T, iT>::draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov){
	
	getMtl()->setShaderSetting(&vertices_[0], 0, 0, 0, 0, aspect_ratio, fov);

	Vector3 light_dir(0.0f, 0.0f, 1.0f);
	Vector3 eye_dir(0.0f, 0.0f, -1.0f);
	/*Matrix4 matrix;
	matrix.rotateY(r);
	eye_dir = eye_dir * matrix;*/
	getMtl()->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
	getMtl()->setLightDir(light_dir.x, light_dir.y, light_dir.z);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);	CHECK_GL_ERROR();
}

template<>
void BaseMesh<SkinVertex, unsigned short>::draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov){

	getMtl()->setShaderSetting(&vertices_[0], 0, 0, 0, 0, aspect_ratio, fov);

	Vector3 light_dir(-1.0f, 0.0f, 0.0f);
	Vector3 eye_dir(0.0f, 0.0f, -1.0f);
	/*Matrix4 matrix;
	matrix.rotateY(r);
	eye_dir = eye_dir * matrix;*/
	getMtl()->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
	getMtl()->setLightDir(light_dir.x, light_dir.y, light_dir.z);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_SHORT, &indices_[0]);	CHECK_GL_ERROR();
}

MeshGroup::MeshGroup(){


}

MeshGroup::~MeshGroup(){


}

struct Quaternion{

	float x;
	float y;
	float z;
	float w;

	Quaternion() :x(0), y(0), z(0), w(0){


	}

	Quaternion(float ix, float iy, float iz, float iw) :x(ix), y(iy), z(iz), w(iw){

	}

	static float dot(const Quaternion& quat0, const Quaternion& quat1){

		return (quat0.x * quat1.x +
			quat0.y * quat1.y +
			quat0.z * quat1.z +
			quat0.w * quat1.w);
	}

	static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t){

		float dot, epsilon;

		epsilon = 1.0f;
		dot = Quaternion::dot(q1, q2);
		if (dot < 0.0f)
			epsilon = -1.0f;
		Quaternion ret;

		ret.x = (1.0f - t) * q1.x + epsilon * t * q2.x;
		ret.y = (1.0f - t) * q1.y + epsilon * t * q2.y;
		ret.z = (1.0f - t) * q1.z + epsilon * t * q2.z;
		ret.w = (1.0f - t) * q1.w + epsilon * t * q2.w;
		return ret;
	}

	void operator *=(float val){

		x *= val;
		y *= val;
		z *= val;
		w *= val;
	}

	Quaternion operator +(const Quaternion& quat){

		x += quat.x;
		y += quat.y;
		z += quat.z;
		w += quat.w;

		return *this;
	}

	Quaternion operator *(float val){

		x *= val;
		y *= val;
		z *= val;
		w *= val;

		return *this;
	}

	Quaternion normalize(){

		float mag2 = w * w + x * x + y * y + z * z;
		if (fabs(mag2) > 0.000001f && fabs(mag2 - 1.0f) > 0.000001f) {
			float mag = sqrt(mag2);
			w /= mag;
			x /= mag;
			y /= mag;
			z /= mag;
		}

		return *this;
	}

	Quaternion operator* (const Quaternion &rq) const
	{
		return Quaternion(
			rq.w * x + rq.x * w + rq.y * z - rq.z * y,
			rq.w * y - rq.x * z + rq.y * w + rq.z * x,
			rq.w * z + rq.x * y - rq.y * x + rq.z * w,
			rq.w * w - rq.x * x - rq.y * y - rq.z * z
			);
	}

	Matrix4 getMatrix() const
	{
		float x2 = x * x;
		float y2 = y * y;
		float z2 = z * z;
		float xy = x * y;
		float xz = x * z;
		float yz = y * z;
		float wx = w * x;
		float wy = w * y;
		float wz = w * z;

		return Matrix4(
			1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
			2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
			2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
};

struct NormalVertex{

	Vector3 pos_;
	Vector3 normal_;
	Vector2	uv_;
};

void MeshGroup::free(){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		delete *itor;
	}

	meshes_.clear();
}

int MeshGroup::getFrame(){

	return 1;
}

void MeshGroup::draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Mesh* mesh = *itor;
		mesh->draw(video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, frame, ab_flag, aspect_ratio, fov);
	}
}

void MeshGroup::draw(RenderItemContext* pctx, float frame, int ab_flag, int reverse, float aspect_ratio, float fov){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Mesh* mesh = *itor;
		mesh->draw(0, 0, 0, 0, frame, ab_flag, aspect_ratio, fov);
	}
}

void MeshGroup::drawFull(RenderItemContext* pctx, float frame, int reverse, float aspect_ratio, float fov){

	for(int i = 0; i < 3; ++i){

		for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

			Mesh* mesh = *itor;
			mesh->draw(0, 0, 0, 0, frame, i, aspect_ratio, fov);
		}		
	}	
}

#define MESH_V11	0xa2efab13
#define MESH_V12	0x3d12efef
#define MESH_V13	0x3d13efef
#define EFFECT_V13	0xeeff0013

#define NEXSKIN_V11	0x3defab13
#define NEXSKIN_V12	0x3defab15
#define NEXSKIN_V13	0x3defab17
#define NEXSKIN_V14	0x3defab18

MeshGroup* MeshGroup::clone(){

	return new MeshGroup();
}

struct SineValue
{
	int min_;
	int max_;
	int speed_;
	int start_;

	SineValue() :min_(0), max_(0), speed_(0), start_(0){

	};

	void deserialize(InputStream& stream){

		stream.read(&min_, sizeof(min_), 1);
		stream.read(&max_, sizeof(max_), 1);
		stream.read(&speed_, sizeof(speed_), 1);
		stream.read(&start_, sizeof(start_), 1);
	}

	bool equal(const SineValue& val){

		if (min_ != val.min_)
			return false;
		if (max_ != val.max_)
			return false;
		if (speed_ != val.speed_)
			return false;
		if (start_ != val.start_)
			return false;
		return true;
	}

	bool operator==(const SineValue& val){

		return equal(val);
	}

	bool operator!=(const SineValue& val){

		if (equal(val))
			return false;
		return true;
	}
};

struct MeshEffect
{
	int blend_mode_;//0 For Normal 1 For Additive
	int billboard_mode_;//0 For Disable, 1 For X, 2 For Y, 3 For XY
	int culling_mode_;//0 For CCW 1 For NONE
	int u_for_uvani_;//1000 ±âÁØ Á¤¼ö°ªÀÌ´Ù. 1ÀÌ¸é ÃÊ´ç 1 / 1000 * 30¾¿ ¿òÁ÷ÀÎ´Ù.
	int v_for_uvani_;//1000 ±âÁØ Á¤¼ö°ªÀÌ´Ù. 1ÀÌ¸é ÃÊ´ç 1 / 1000 * 30¾¿ ¿òÁ÷ÀÎ´Ù.

	SineValue alpha_;
	SineValue red_;
	SineValue green_;
	SineValue blue_;

	MeshEffect() :blend_mode_(0), billboard_mode_(0), culling_mode_(0), u_for_uvani_(0), v_for_uvani_(0){

	};

	void deserialize(InputStream& stream){

		stream.read(&blend_mode_, sizeof(blend_mode_), 1);
		stream.read(&billboard_mode_, sizeof(billboard_mode_), 1);
		stream.read(&culling_mode_, sizeof(culling_mode_), 1);
		stream.read(&u_for_uvani_, sizeof(u_for_uvani_), 1);
		stream.read(&v_for_uvani_, sizeof(v_for_uvani_), 1);
		alpha_.deserialize(stream);
		red_.deserialize(stream);
		green_.deserialize(stream);
		blue_.deserialize(stream);
	}
};

struct MeshEffect2
{
	int billboard_mode_;
	int culling_mode_;
	int blend_mode_;
	int alphablend_;
	int alpharef_;
	int special_render_mode_;
	int u_for_uvani_;
	int v_for_uvani_;
	SineValue alpha_;
	SineValue red_;
	SineValue green_;
	SineValue blue_;

	MeshEffect2() :billboard_mode_(0), culling_mode_(0), blend_mode_(0), alpharef_(64), alphablend_(1), special_render_mode_(0), u_for_uvani_(0), v_for_uvani_(0){
	
	};

	bool operator==(const MeshEffect2& mesheffect){

		if (billboard_mode_ != mesheffect.billboard_mode_)
			return false;
		if (culling_mode_ != mesheffect.culling_mode_)
			return false;
		if (blend_mode_ != mesheffect.blend_mode_)
			return false;
		if (alphablend_ != mesheffect.alphablend_)
			return false;
		if (special_render_mode_ != mesheffect.special_render_mode_)
			return false;
		if (u_for_uvani_ != mesheffect.u_for_uvani_)
			return false;
		if (v_for_uvani_ != mesheffect.v_for_uvani_)
			return false;
		if (alpha_ != mesheffect.alpha_)
			return false;
		if (red_ != mesheffect.red_)
			return false;
		if (green_ != mesheffect.green_)
			return false;
		if (blue_ != mesheffect.blue_)
			return false;
		return true;
	}

	void deserialize(InputStream& stream){
		
		stream.read(&billboard_mode_, sizeof(billboard_mode_), 1);
		stream.read(&culling_mode_, sizeof(culling_mode_), 1);
		stream.read(&blend_mode_, sizeof(blend_mode_), 1);
		stream.read(&alpharef_, sizeof(alpharef_), 1);
		stream.read(&alphablend_, sizeof(alphablend_), 1);
		stream.read(&special_render_mode_, sizeof(special_render_mode_), 1);
		stream.read(&u_for_uvani_, sizeof(u_for_uvani_), 1);
		stream.read(&v_for_uvani_, sizeof(v_for_uvani_), 1);
		alpha_.deserialize(stream);
		red_.deserialize(stream);
		green_.deserialize(stream);
		blue_.deserialize(stream);
	}
};

bool MeshGroup::deserialize(InputStream& stream){

	int count_of_meshes = 0;
	int count_of_physxobj = 0;
	
	stream.read(&count_of_meshes, sizeof(int), 1);
	stream.read(&count_of_physxobj, sizeof(int), 1);
	Factory<Material>& material_factory = stream.getContext()->material_factory_;
	
	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(stream);

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream);

		Mesh* mesh = stream.getContext()->mesh_factory_.create(type_of_mtl);
		mesh->deserialize(stream);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

bool MeshGroup::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void *p)){

	int count_of_meshes = 0;
	int count_of_physxobj = 0;
	
	stream.read(&count_of_meshes, sizeof(int), 1);
	stream.read(&count_of_physxobj, sizeof(int), 1);
	Factory<Material>& material_factory = stream.getContext()->material_factory_;
	
	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(stream);

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);

		Mesh* mesh = stream.getContext()->mesh_factory_.create(type_of_mtl);
		mesh->deserialize(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

void RenderItemContext::initMeshFactory(){

	mesh_factory_.set(MAT_BGFX_A, new BaseMesh<NexVertex, int>());
	mesh_factory_.set(MAT_SKINMAT, new BaseMesh<NexVertex, int>());
	mesh_factory_.set(MAT_BGFX_C, new BaseMesh<NexVertexForLightmap, int>());
	meshgroup_factory_.set(MESH_V13, new MeshGroup());
	meshgroup_factory_.set(EFFECT_V13, new EffectGroup());
	meshgroup_factory_.set(NEXSKIN_V14, new SkinGroup());
}



int MeshGroup::getTexWidthOnSlot(int placeholder_id){

	std::vector<Mesh*> meshes_;
	for(std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Material* pmtl = (*itor)->getMtl();
		if(!pmtl)
			continue;
		if(placeholder_id == pmtl->getPlaceHolderFlag()){

			return pmtl->getWidth();
		}
	}

	return 0;
}

int MeshGroup::getTexHeightOnSlot(int placeholder_id){

	std::vector<Mesh*> meshes_;
	for(std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Material* pmtl = (*itor)->getMtl();
		if(!pmtl)
			continue;
		if(placeholder_id == pmtl->getPlaceHolderFlag()){

			return pmtl->getHeight();
		}
	}

	return 0;
}

template<typename T>
struct AniKey{

	int frame_;
	T	value_;
};

template<typename T>
T getKey(int frame, std::vector<AniKey<T> >& keylist){

	AniKey<T> prev_key = keylist.front();
	AniKey<T> next_key = keylist.back();

	if (frame >= next_key.frame_){

		return next_key.value_;
	}

	if (frame <= prev_key.frame_){

		return prev_key.value_;
	}

	for (typename std::vector<AniKey<T> >::iterator itor = keylist.begin(); itor != keylist.end(); ++itor){

		AniKey<T>& key = *itor;
		if (key.frame_ == frame){

			return key.value_;
		}
		if (key.frame_ > frame){

			next_key = key;
			break;
		}
		prev_key = key;
	}

	int frame_gap = next_key.frame_ - prev_key.frame_;
	int frame_adv = frame - prev_key.frame_;

	if (frame_gap <= 0)
		return prev_key.value_;

	float progress = float(frame_adv) / float(frame_gap);
	T ret = prev_key.value_ * (1.0f - progress) + next_key.value_ * progress;

	return ret;
}

template<>
Quaternion getKey(int frame, std::vector<AniKey<Quaternion> >& keylist){

	AniKey<Quaternion> prev_key = keylist.front();
	AniKey<Quaternion> next_key = keylist.back();

	if (frame >= next_key.frame_){

		return next_key.value_;
	}

	if (frame <= prev_key.frame_){

		return prev_key.value_;
	}

	for (std::vector<AniKey<Quaternion> >::iterator itor = keylist.begin(); itor != keylist.end(); ++itor){

		AniKey<Quaternion>& key = *itor;
		if (key.frame_ == frame){

			return key.value_;
		}
		if (key.frame_ > frame){

			next_key = key;
			break;
		}
		prev_key = key;
	}

	int frame_gap = next_key.frame_ - prev_key.frame_;
	int frame_adv = frame - prev_key.frame_;

	if (frame_gap <= 0)
		return prev_key.value_;

	float progress = float(frame_adv) / float(frame_gap);
	
	return Quaternion::slerp(prev_key.value_, next_key.value_, progress);
}

struct KeyData{

	enum{

		E_POS_KEY,
		E_ROT_KEY,
		E_SCL_KEY,
		E_END_KEY,
	};

	int duration_tick_;
	int fps_;

	std::vector<AniKey<Vector3> > position_key_;
	std::vector<AniKey<Quaternion> > rotation_key_;
	std::vector<AniKey<Vector3> > scale_key_;

	KeyData() :fps_(30), duration_tick_(1){

	}

	KeyData(const KeyData& keydata){

		fps_ = keydata.fps_;
		duration_tick_ = keydata.duration_tick_;
		position_key_ = keydata.position_key_;
		rotation_key_ = keydata.rotation_key_;
		scale_key_ = keydata.scale_key_;
	}

	void setDuration(int duration){

		duration_tick_ = duration;
	}

	bool deserialize(InputStream& stream){

		int count, data_type, bone_index;

		stream.read(&bone_index, sizeof(bone_index), 1);

		while (1){

			stream.read(&data_type, sizeof(data_type), 1);
			
			if (data_type >= E_END_KEY) 
				return true;

			if (data_type == E_POS_KEY){

				AniKey<Vector3> key;
				stream.read(&count, sizeof(count), 1);

				for (int i = 0; i < count; ++i){

					stream.read(&key.frame_, sizeof(key.frame_), 1);
					stream.read(&key.value_, sizeof(key.value_), 1);
					position_key_.push_back(key);
				}
			}
			else if (data_type == E_ROT_KEY){

				AniKey<Quaternion> key;
				stream.read(&count, sizeof(count), 1);
				
				Quaternion preQuat(0, 0, 0, 1);
				Quaternion tQuat;
				
				for (int i = 0; i < count; ++i){

					stream.read(&key.frame_, sizeof(key.frame_), 1);
					stream.read(&tQuat, sizeof(tQuat), 1);
					key.value_ = preQuat * tQuat;
					preQuat = key.value_;
					rotation_key_.push_back(key);
				}
			}
			else if (data_type == E_SCL_KEY){

				AniKey<Vector3> key;
				stream.read(&count, sizeof(count), 1);

				for (int i = 0; i < count; ++i){

					stream.read(&key.frame_, sizeof(key.frame_), 1);
					stream.read(&key.value_, sizeof(key.value_), 1);
					scale_key_.push_back(key);
				}
			}
		}
		return false;
	}

	int getDuration(){

		return duration_tick_;
	}

	int getFrame(int tick, bool repeat){

		int frame = tick;// *fps_ / 1000;
		if (true == repeat){

			frame %= duration_tick_;
		}
		
		if (frame >= duration_tick_){

			frame = duration_tick_;
		}

		return frame;
	}

	Vector3 getPositionKey(int tick, bool repeat){

		return getKey(getFrame(tick, repeat), position_key_);
	}

	Quaternion getRotationKey(int tick, bool repeat){

		return getKey(getFrame(tick, repeat), rotation_key_);
	}

	Vector3 getScaleKey(int tick, bool repeat){

		return getKey(getFrame(tick, repeat), scale_key_);
	}
};

class BoneAnimation{

	Vector3 position_;
	Quaternion rotation_;
	Vector3 scale_;
	Matrix4 matrix_;
	bool matrix_remake_;

	KeyData* data_;

	int getTickFromTiming(float timing){

		return (int)timing;
	}

public:
	BoneAnimation(KeyData& data) :data_(&data), matrix_remake_(true){


	}

	~BoneAnimation(){


	}

	void setKeyData(KeyData& data){

		data_ = &data;
	}

	void animate(float timing, bool repeat){

		position_ = data_->getPositionKey(getTickFromTiming(timing), repeat);
		rotation_ = data_->getRotationKey(getTickFromTiming(timing), repeat);
		scale_ = data_->getScaleKey(getTickFromTiming(timing), repeat);
		matrix_remake_ = true;
	}

	const Vector3& getPosition() const{

		return position_;
	}

	const Quaternion& getRotation() const{

		return rotation_;
	}

	const Vector3& getScale() const{

		return scale_;
	}

	const Matrix4& getMatrix(){

		if (matrix_remake_){

			matrix_ = rotation_.getMatrix();
			matrix_[3] = position_.x;
			matrix_[7] = position_.y;
			matrix_[11] = position_.z;
			matrix_[0] *= scale_.x;
			matrix_[5] *= scale_.y;
			matrix_[10] *= scale_.z;
			matrix_remake_ = false;
		}

		return matrix_;
	}
};

struct KeyAnimation{

	std::vector<KeyData> data_;

	int start_frame_;
	int end_frame_;

	bool deserialize(InputStream& stream){

		int count_of_bone;

		stream.read(&count_of_bone, sizeof(count_of_bone), 1);
		data_.resize(count_of_bone);
		stream.read(&start_frame_, sizeof(start_frame_), 1);
		stream.read(&end_frame_, sizeof(end_frame_), 1);
		int duration = end_frame_ - start_frame_;

		for (int i = 0; i < count_of_bone; ++i){

			data_[i].setDuration(duration);
			data_[i].deserialize(stream);
		}
		return true;
	}

	size_t getCount(){

		return data_.size();
	}

	void setDataToBoneAni(std::vector<BoneAnimation>& bone_ani){

		for (std::vector<KeyData>::iterator itor = data_.begin(); itor != data_.end(); ++itor){

			bone_ani.push_back(BoneAnimation(*itor));
		}
	}
};




struct KeyAniInstance{

	KeyAnimation* animation_data_;
	std::vector<BoneAnimation> bone_ani_;

	void setKeyAnimation(KeyAnimation* animation_data){

		bone_ani_.clear();
		animation_data_ = animation_data;
		
		if (NULL == animation_data_)
			return;

		bone_ani_.reserve(animation_data_->getCount());
		animation_data_->setDataToBoneAni(bone_ani_);
		//bone_ani_.shrink_to_fit();
	}

	void animate(float timing, bool repeat){

		for (std::vector<BoneAnimation>::iterator itor = bone_ani_.begin(); itor != bone_ani_.end(); ++itor){

			(*itor).animate(timing, repeat);
		}
	}

	const Vector3& getPosition(int idx) const{

		return bone_ani_[idx].getPosition();
	}

	const Quaternion& getRotation(int idx) const{

		return bone_ani_[idx].getRotation();
	}

	const Vector3& getScale(int idx) const{

		return bone_ani_[idx].getScale();
	}

	const Matrix4& getMatrix(int idx){

		return bone_ani_[idx].getMatrix();
	}
};


struct Bone{

	struct Node{

		int parent_bone_index_;
		int kinematic_flag_;
		Matrix4 world_matrix_;
		Matrix4 inv_world_matrix_;
		std::vector<int> childs_;

		bool deserialize(InputStream& stream){

			float m[16];

			stream.read(&parent_bone_index_, sizeof(parent_bone_index_), 1);
			stream.read(m, sizeof(float), 16);
			world_matrix_.set(m);
			world_matrix_.transpose();
			stream.read(&kinematic_flag_, sizeof(kinematic_flag_), 1);
			inv_world_matrix_ = world_matrix_;			
			inv_world_matrix_.invert();
			return true;
		}
	};


	std::vector<Node> nodelist_;

	size_t getCount(){

		return nodelist_.size();
	}

	bool deserialize(InputStream& stream){

		int version_check = 0;
		int count_of_node = 0;

		stream.read(&version_check, sizeof(version_check), 1);
		stream.read(&count_of_node, sizeof(count_of_node), 1);

		nodelist_.resize(count_of_node);
		//nodelist_.shrink_to_fit();

		for (int i = 0; i < count_of_node; ++i){
		
			nodelist_[i].deserialize(stream);
		}

		for (int i = 0; i < count_of_node; ++i){

			int parent_idx = nodelist_[i].parent_bone_index_;
			if (parent_idx > -1){

				nodelist_[parent_idx].childs_.push_back(i);
			}
		}
		return true;
	}
};

struct BoneAnimator{

	std::vector<Matrix4> matrix_for_bones_;
	std::vector<Matrix4> matrix_palette_;

	void setSize(size_t size){

		matrix_for_bones_.resize(size);
		matrix_palette_.resize(size);

		//matrix_for_bones_.shrink_to_fit();
		//matrix_palette_.shrink_to_fit();
	}

	void update_node(Bone& bone, KeyAniInstance& ani, int iBoneIndex){

		const Bone::Node& boneTemp(bone.nodelist_[iBoneIndex]);
		int iParentBoneidx = boneTemp.parent_bone_index_;
		const Bone::Node& boneParent(bone.nodelist_[iParentBoneidx]);

		const Matrix4 matLocal = boneTemp.world_matrix_ * boneParent.inv_world_matrix_;
		const Matrix4& matAni = ani.getMatrix(iBoneIndex);

		Matrix4 matTemp = matLocal * matAni;

		if (matAni[3] == 0.0f && matAni[7] == 0.0f && matAni[11] == 0.0f)
		{
			matTemp[3] = matLocal[3];
			matTemp[7] = matLocal[7];
			matTemp[11] = matLocal[11];
		}
		else
		{
			matTemp[3] = matAni[3];
			matTemp[7] = matAni[7];
			matTemp[11] = matAni[11];
		}
		
		matTemp = matTemp * matrix_for_bones_[iParentBoneidx];
		matrix_for_bones_[iBoneIndex] = matTemp;
		matrix_palette_[iBoneIndex] = boneTemp.inv_world_matrix_ *matTemp;

		for (size_t i = 0; i < boneTemp.childs_.size(); ++i) 
			update_node(bone, ani, boneTemp.childs_[i]);
	}

	void update(Bone& bone, KeyAniInstance& ani){

		setSize(bone.getCount());
		int iBoneIndex = 0;
		const Bone::Node& boneTemp(bone.nodelist_[iBoneIndex]);
		const Matrix4 matLocal = boneTemp.world_matrix_;		
		const Matrix4& matAni = ani.getMatrix(iBoneIndex);

		Matrix4 matTemp = matLocal * matAni;

		if (matAni[3] == 0.0f && matAni[7] == 0.0f && matAni[11] == 0.0f)
		{
			matTemp[3] = matLocal[3];
			matTemp[7] = matLocal[7];
			matTemp[11] = matLocal[11];
		}
		else
		{
			matTemp[3] = matAni[3];
			matTemp[7] = matAni[7];
			matTemp[11] = matAni[11];
		}

		matrix_for_bones_[iBoneIndex] = matTemp;
		matrix_palette_[iBoneIndex] = boneTemp.inv_world_matrix_ * matTemp;
		for (size_t i = 0; i < boneTemp.childs_.size(); ++i) 
			update_node(bone, ani, boneTemp.childs_[i]);
	}
};

SkinGroup::SkinGroup(){

}

SkinGroup::~SkinGroup(){

}


MeshGroup* SkinGroup::clone(){

	return new SkinGroup();
}

bool SkinGroup::deserialize(InputStream& stream){

	int count_of_meshes = 0;
	int count_of_cutbox = 0;

	stream.read(&count_of_meshes, sizeof(int), 1);
	stream.read(&count_of_cutbox, sizeof(int), 1);
	Factory<Material>& material_factory = stream.getContext()->material_factory_;

	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(stream);

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream);

		Mesh* mesh = new Skin();
		mesh->deserialize(stream);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

bool SkinGroup::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

	int count_of_meshes = 0;
	int count_of_cutbox = 0;

	stream.read(&count_of_meshes, sizeof(int), 1);
	stream.read(&count_of_cutbox, sizeof(int), 1);
	Factory<Material>& material_factory = stream.getContext()->material_factory_;

	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(stream);

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);

		Mesh* mesh = new Skin();
		mesh->deserialize(stream);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

EffectGroup::EffectGroup(){


}

EffectGroup::~EffectGroup(){


}

int EffectGroup::getFrame(){

	return count_of_frame_;
}

MeshGroup* EffectGroup::clone(){

	return new EffectGroup();
}

void Skin::draw(float x, float y, float z, float r, BoneAnimator& animator, float aspect_ratio, float fov){
	
	// NexVertex vtx[4096];

	// VerticesType& vertices = getVertices();
	// IndicesType& indices = getIndices();

	// for (size_t i = 0; i < vertices.size(); ++i){

	// 	SkinVertex& skin_vtx = vertices[i];
	// 	vtx[i].pos_ = Vector3();
	// 	vtx[i].tangent_ = Vector3();
	// 	vtx[i].normal_ = Vector3();
	// 	vtx[i].binormal_ = Vector3();
	// 	vtx[i].uv_ = skin_vtx.uv_;

	// 	for (int j = 0; j < MAX_BONE_INDEX; ++j){

	// 		int bone_idx = skin_vtx.weight_[j].idx_;
	// 		if (bone_idx < 0){

	// 			if (j == 0){

	// 				bone_idx = 0;
	// 			}
	// 			else break;
	// 		}
			
	// 		float weight = skin_vtx.weight_[j].weight_;
	// 		if (weight <= 0.0f){

	// 			if (j == 0){

	// 				weight = 1.0f;
	// 			}
	// 			else break;
	// 		}
			
	// 		Vector4 tmp = (Vector4(skin_vtx.pos_.x, skin_vtx.pos_.y, skin_vtx.pos_.z, 1.0f) * animator.matrix_palette_[bone_idx]);

	// 		vtx[i].pos_ += (Vector3(tmp.x, tmp.y, tmp.z) * weight);
	// 		vtx[i].normal_ += ((skin_vtx.normal_ * animator.matrix_palette_[bone_idx]) * weight);
	// 		vtx[i].tangent_ += ((skin_vtx.tangent_ * animator.matrix_palette_[bone_idx]) * weight);
	// 		vtx[i].binormal_ += ((skin_vtx.binormal_ * animator.matrix_palette_[bone_idx]) * weight);
	// 	}
	// }

	// getMtl()->setShaderSetting(vtx, x, y, z, 180.0f);

	// Vector3 light_dir(1.0f, 0.0f, 0.0f);
	// Vector3 eye_dir(0.0f, 0.0f, -1.0f);
	// Matrix4 matrix;
	// matrix.rotateY(r);
	// light_dir = light_dir * matrix;
	// getMtl()->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
	// getMtl()->setLightDir(light_dir.x, light_dir.y, light_dir.z);
	// glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);	CHECK_GL_ERROR();
}

void SkinGroup::draw(float x, float y, float z, float r, BoneAnimator& animator, float aspect_ratio, float fov){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Skin* mesh = (Skin*)*itor;
		mesh->draw(x + 0.0f, y, z, r, animator, aspect_ratio, fov);
	}
}

void Track::init(){

}

Matrix4 Track::getMatrix(float frame){

	int base_frame = (int)(floor(frame));
	Matrix4* pmatrix = NULL;
	for (Matrixkeylist_t::iterator itor = matrixkey_list_.begin(); itor != matrixkey_list_.end(); ++itor){

		MatrixKey& key = *itor;
		if (key.frame_ <= base_frame){

			pmatrix = &key.matrix_;
		}
		else
			break;
	}

	if (pmatrix)
		return *pmatrix;
	Matrix4 id;
	id.identity();
	return id;
}

bool Track::deserializerMatrixKey(InputStream& stream, int total_frame){

	matrixkey_list_.clear();
	stream.read(&index_for_parent_bone_, sizeof(index_for_parent_bone_), 1);
	stream.read(&index_for_bone_, sizeof(index_for_bone_), 1);
	matrix_for_world_.set((float*)stream.getAddr());
	stream.seek(sizeof(float) * 16, SEEK_CUR);
	int count = 0;
	stream.read(&count, sizeof(count), 1);
	
	for (int i = 0; i < count; ++i){

		MatrixKey key;
		stream.read(&key.frame_, sizeof(key.frame_), 1);
		key.matrix_.set((float*)stream.getAddr());
		stream.seek(sizeof(float) * 16, SEEK_CUR);
		matrixkey_list_.push_back(key);
	}

	return true;
}

class BaseAniMesh:public Mesh{

	int			index_of_node_;
	int			type_;
	int			num_of_faces_;
	int			count_of_vertices_;
	WORD*		ptris_;

	int			count_of_vertlist_;
	Vertices*	pvertices_;
	MeshEffect2	mesheffect_;
	bool		flag_for_special_;

	Vector3 	light_dir_;
	Vector3		eye_dir_;

public:

	BaseAniMesh():index_of_node_(-1), type_(-1), num_of_faces_(0), count_of_vertices_(0), ptris_(NULL), count_of_vertlist_(0), pvertices_(NULL), flag_for_special_(false){

	}

	virtual ~BaseAniMesh(){

		free();
	}

	Mesh* clone(){

		return new BaseAniMesh();
	}

	int getAlphaBlendFlag(){

		return mesheffect_.alphablend_;
	}

	int getFlareFlag(){

		return mesheffect_.blend_mode_;
	}

	virtual void free(){

		if(ptris_)
			delete[] ptris_;
		if(pvertices_)
			delete[] pvertices_;

		index_of_node_ = -1;
		type_ = -1;
		num_of_faces_ = 0;
		count_of_vertices_ = 0;
		ptris_ = NULL;
		count_of_vertlist_ = 0;
		pvertices_ = NULL;
		flag_for_special_ = false;
	}

	virtual bool deserialize(InputStream& stream){

		free();

		Factory<Material>& material_factory = stream.getContext()->material_factory_;

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream);

		setMtl(mtl);

		stream.read(&index_of_node_, sizeof(index_of_node_), 1);
		mesheffect_.deserialize(stream);
		if(mtl->getAlphaBlendingFlag())
			mesheffect_.alphablend_ = 1;
		stream.read(&type_, sizeof(type_), 1);
		stream.read(&num_of_faces_, sizeof(num_of_faces_), 1);

		if(num_of_faces_ > 0){

			ptris_ = new WORD[num_of_faces_ * 3];
			stream.read(ptris_, sizeof(WORD), num_of_faces_ * 3);
			for(int i = 0; i < num_of_faces_; ++i){

				WORD tmp = ptris_[i * 3 + 1];
				ptris_[i * 3 + 1] = ptris_[i * 3 + 2];
				ptris_[i * 3 + 2] = tmp;
			}
		}

		stream.read(&count_of_vertlist_, sizeof(count_of_vertlist_), 1);
		if(count_of_vertlist_ > 0){

			pvertices_ = new Vertices[count_of_vertlist_];
		}
		
		if(type_ == 2){//VTTYPE_PARTICLE

			//	mesheffect_.m_iBlendOps = 1;
			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&( pvertices_[ i ].count_of_vertices_ ), sizeof( pvertices_[ i ].count_of_vertices_ ), 1);
				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}

				NexParticle* pSkinVertex = (NexParticle*)malloc(sizeof(NexParticle) * pvertices_[i].count_of_vertices_);
				pvertices_[ i ].pvertices_ = pSkinVertex;
				
				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					NexParticle vtx;
					stream.read(&vtx.m_vecPos, sizeof(vtx.m_vecPos), 1);
					stream.read(&vtx.m_fRadius, sizeof(vtx.m_fRadius), 1);
					pSkinVertex[j] = vtx;
				}
			}
		}
		else if(type_ == 1){//VTTYPE_SKIN

			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&(pvertices_[i].time_value_), sizeof(pvertices_[i].time_value_), 1);
				stream.read(&(pvertices_[i].count_of_vertices_), sizeof(pvertices_[i].count_of_vertices_), 1);

				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}

				NexVertex* pSkinVertex = (NexVertex*)malloc(sizeof(NexVertex) * pvertices_[i].count_of_vertices_);
				pvertices_[i].pvertices_ = pSkinVertex;

				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					NexVertex nvtx_old;
					nvtx_old.deserialize(stream);
					pSkinVertex[j] = nvtx_old;
				}

			}
		}
		else if(type_ == 0){//VTTYPE_EFFECT

			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&(pvertices_[i].time_value_), sizeof(pvertices_[i].time_value_), 1);
				stream.read(&(pvertices_[i].count_of_vertices_), sizeof(pvertices_[i].count_of_vertices_), 1);
				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}
				EffectVertex* pEffectVertex = (EffectVertex*)malloc(sizeof(EffectVertex) * pvertices_[i].count_of_vertices_);
				pvertices_[ i ].pvertices_ = pEffectVertex;
				
				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					stream.read(&pEffectVertex[j].pos_, sizeof(pEffectVertex[j].pos_), 1);
					stream.read(&pEffectVertex[j].uv_, sizeof(pEffectVertex[j].uv_), 1);
				}
			}
		}
		if(count_of_vertlist_ > 0)
			count_of_vertices_ = pvertices_[0].count_of_vertices_;
		return true;
	}

    virtual bool deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

		free();

		Factory<Material>& material_factory = stream.getContext()->material_factory_;

		int type_of_mtl = 0;
		stream.read(&type_of_mtl, sizeof(int), 1);
		Material* mtl = material_factory.create(type_of_mtl);
		mtl->deserialize(stream, uid, prenderer, pAquireContextFunc, pReleaseContextFunc);

		setMtl(mtl);

		stream.read(&index_of_node_, sizeof(index_of_node_), 1);
		mesheffect_.deserialize(stream);
		if(mtl->getAlphaBlendingFlag())
			mesheffect_.alphablend_ = 1;
		stream.read(&type_, sizeof(type_), 1);
		stream.read(&num_of_faces_, sizeof(num_of_faces_), 1);

		if(num_of_faces_ > 0){

			ptris_ = new WORD[num_of_faces_ * 3];
			stream.read(ptris_, sizeof(WORD), num_of_faces_ * 3);
			for(int i = 0; i < num_of_faces_; ++i){

				WORD tmp = ptris_[i * 3 + 1];
				ptris_[i * 3 + 1] = ptris_[i * 3 + 2];
				ptris_[i * 3 + 2] = tmp;
			}
		}

		stream.read(&count_of_vertlist_, sizeof(count_of_vertlist_), 1);
		if(count_of_vertlist_ > 0){

			pvertices_ = new Vertices[count_of_vertlist_];
		}
		
		if(type_ == 2){//VTTYPE_PARTICLE

			//	mesheffect_.m_iBlendOps = 1;
			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&( pvertices_[ i ].count_of_vertices_ ), sizeof( pvertices_[ i ].count_of_vertices_ ), 1);
				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}

				NexParticle* pSkinVertex = (NexParticle*)malloc(sizeof(NexParticle) * pvertices_[i].count_of_vertices_);
				pvertices_[ i ].pvertices_ = pSkinVertex;
				
				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					NexParticle vtx;
					stream.read(&vtx.m_vecPos, sizeof(vtx.m_vecPos), 1);
					stream.read(&vtx.m_fRadius, sizeof(vtx.m_fRadius), 1);
					pSkinVertex[j] = vtx;
				}
			}
		}
		else if(type_ == 1){//VTTYPE_SKIN

			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&(pvertices_[i].time_value_), sizeof(pvertices_[i].time_value_), 1);
				stream.read(&(pvertices_[i].count_of_vertices_), sizeof(pvertices_[i].count_of_vertices_), 1);

				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}

				NexVertex* pSkinVertex = (NexVertex*)malloc(sizeof(NexVertex) * pvertices_[i].count_of_vertices_);
				pvertices_[i].pvertices_ = pSkinVertex;

				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					NexVertex nvtx_old;
					nvtx_old.deserialize(stream);
					pSkinVertex[j] = nvtx_old;
				}

			}
		}
		else if(type_ == 0){//VTTYPE_EFFECT

			for(int i = 0; i < count_of_vertlist_; ++i){

				stream.read(&(pvertices_[i].time_value_), sizeof(pvertices_[i].time_value_), 1);
				stream.read(&(pvertices_[i].count_of_vertices_), sizeof(pvertices_[i].count_of_vertices_), 1);
				if(!flag_for_special_){

					if(i > 0){

						if(pvertices_[i].count_of_vertices_ != pvertices_[0].count_of_vertices_)
							flag_for_special_ = true;
					}
				}

				if(pvertices_[i].count_of_vertices_ <= 0){

					pvertices_[i].pvertices_ = NULL;
					continue;
				}
				EffectVertex* pEffectVertex = (EffectVertex*)malloc(sizeof(EffectVertex) * pvertices_[i].count_of_vertices_);
				pvertices_[ i ].pvertices_ = pEffectVertex;
				
				for(int j = 0; j < pvertices_[i].count_of_vertices_; ++j){

					stream.read(&pEffectVertex[j].pos_, sizeof(pEffectVertex[j].pos_), 1);
					stream.read(&pEffectVertex[j].uv_, sizeof(pEffectVertex[j].uv_), 1);
				}
			}
		}
		if(count_of_vertlist_ > 0)
			count_of_vertices_ = pvertices_[0].count_of_vertices_;
		return true;
	}

	void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov){


	}

	void setLightDir(float x, float y, float z){

		light_dir_.set(x, y, z);
	}

	void setEyeDir(float x, float y, float z){

		eye_dir_.set(x, y, z);
	}

	void draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, Matrix4* pmatrix, Matrix4* pview, int ab_flag, float aspect_ratio, float fov){

		if(num_of_faces_ <= 0)
			return; 
		int iFrame = int(floor(frame));

		if( pvertices_[ 0 ].count_of_vertices_ > 0 )
		{
			Vertices* pvertice = NULL;
			
			for (int i = 0; i < count_of_vertlist_; ++i){

				if (pvertices_[i].time_value_ <= iFrame)
					pvertice = &pvertices_[i];
			}

			if(pvertice == NULL){

				return;
			}

			switch(type_)
			{
			case 0:
				{

					getMtl()->setShaderSetting((EffectVertex*)pvertice->pvertices_, pview->get(), pmatrix[index_of_node_].get(), video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, ab_flag, aspect_ratio, fov);
					getMtl()->setEyeDir(eye_dir_.x, eye_dir_.y, eye_dir_.z);
					getMtl()->setLightDir(light_dir_.x, light_dir_.y, light_dir_.z);
					
					if(mesheffect_.culling_mode_ == 0)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);

					glDrawElements(GL_TRIANGLES, num_of_faces_ * 3, GL_UNSIGNED_SHORT, ptris_);	CHECK_GL_ERROR();
				}
				break;
			case 1:
				{

					getMtl()->setShaderSetting((NexVertex*)pvertice->pvertices_, pview->get(), pmatrix[index_of_node_].get(), video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, ab_flag, iFrame, aspect_ratio, fov);
					getMtl()->setEyeDir(eye_dir_.x, eye_dir_.y, eye_dir_.z);
					getMtl()->setLightDir(light_dir_.x, light_dir_.y, light_dir_.z);
					
					if(mesheffect_.culling_mode_ == 0)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);

					glDrawElements(GL_TRIANGLES, num_of_faces_ * 3, GL_UNSIGNED_SHORT, ptris_);	CHECK_GL_ERROR();
				}
				break;
			case 2:
				nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
				break;
			};
		}
	}

	void draw(RenderItemContext* pctx, float frame, Matrix4* pmatrix, Matrix4* pview, int ab_flag, float aspect_ratio, float fov){

		int iFrame = int(floor(frame));

		if( pvertices_[ 0 ].count_of_vertices_ > 0 )
		{
			Vertices* pvertice = NULL;
			
			for (int i = 0; i < count_of_vertlist_; ++i){

				if (pvertices_[i].time_value_ <= iFrame)
					pvertice = &pvertices_[i];
			}

			if(pvertice == NULL){

				return;
			}

			switch(type_)
			{
			case 0:
				{
					getMtl()->setShaderSetting((EffectVertex*)pvertice->pvertices_, pview->get(), pmatrix[index_of_node_].get(), pctx, ab_flag, iFrame, aspect_ratio, fov);
					getMtl()->setEyeDir(eye_dir_.x, eye_dir_.y, eye_dir_.z);
					getMtl()->setLightDir(light_dir_.x, light_dir_.y, light_dir_.z);

					if(mesheffect_.culling_mode_ == 0)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);

					glDrawElements(GL_TRIANGLES, num_of_faces_ * 3, GL_UNSIGNED_SHORT, ptris_);	CHECK_GL_ERROR();
				}
				break;
			case 1:
				{
					getMtl()->setShaderSetting((NexVertex*)pvertice->pvertices_, pview->get(), pmatrix[index_of_node_].get(), pctx, ab_flag, iFrame, aspect_ratio, fov);
					getMtl()->setEyeDir(eye_dir_.x, eye_dir_.y, eye_dir_.z);
					getMtl()->setLightDir(light_dir_.x, light_dir_.y, light_dir_.z);

					if(mesheffect_.culling_mode_ == 0)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);

					glDrawElements(GL_TRIANGLES, num_of_faces_ * 3, GL_UNSIGNED_SHORT, ptris_);	CHECK_GL_ERROR();
				}
				break;
			case 2:
				nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
				break;
			};
		}
	}

	int getNode(){

		return index_of_node_;
	}
};

void EffectGroup::prepare(Matrix4* pmatrices, Matrix4& view, Vector3& light_dir, Vector3& eye_dir, float frame, int reverse){

	Matrix4 matrix_for_camera = track_.getMatrix(frame);
	Vector4 tmp = matrix_for_camera * Vector4(0, 0, 0, 1);
	Vector3 eye_pos = Vector3(tmp.x, tmp.y, tmp.z);
	Vector3 eye_normal = matrix_for_camera * Vector3(0, -1, 0);
	Vector3 eye_look = eye_pos + eye_normal;

	Vector3 up(0, 1, 0);
	view.lookAt(eye_pos, eye_look, up);
	
	switch(reverse)
	{
		case 1:	//	VERTICAL
			{
				glFrontFace(GL_CW);
				view.scale(1, -1, 1);
			}
			break;
		case 2: //	HORIZONTAL
			{
				glFrontFace(GL_CW);
				view.scale(-1, 1, 1);
			}
			break;
		case 3:	// VERTICAL & HORIZONTAL
			{
				glFrontFace(GL_CCW);
				view.scale(-1, -1, 1);
			}
			break;
		case 0: // DEFAULT
		default:
			{
				glFrontFace(GL_CCW);
			}
			break;
	}

	for (TrackList_t::iterator itor = node_list_.begin(); itor != node_list_.end(); ++itor){

		Track* ptrack = *itor;

		pmatrices[ptrack->getBoneIndex()] = ptrack->getMatrix(frame);
	}

	light_dir = eye_pos - eye_look;
	eye_dir = light_dir.normalize();
	light_dir += Vector3(0.0f, 0.6f, 0.0f);
	light_dir.normalize();
}

void EffectGroup::draw(RenderItemContext* pctx, float frame, int ab_flag, int reverse, float aspect_ratio, float fov){

	Matrix4 matNode[2048];
	Vector3 light_dir, eye_dir;
	Matrix4 view;

	prepare(matNode, view, light_dir, eye_dir, frame, reverse);

	for(std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		BaseAniMesh* pmesh = (BaseAniMesh*)(*itor);
		
		if(ab_flag == 2){

			if(!pmesh->getFlareFlag()){

				continue;
			}
		}
		else{

			if(pmesh->getFlareFlag())
				continue;
		}
		
		if(ab_flag == 1){

			if(!pmesh->getAlphaBlendFlag())
				continue;
		}

		pmesh->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
		pmesh->setLightDir(light_dir.x, light_dir.y, light_dir.z);
		pmesh->draw(pctx, frame, matNode, &view, ab_flag, aspect_ratio, fov);
	}

	switch(reverse)
	{
		case 1:	//	VERTICAL
			{
				glFrontFace(GL_CCW);
			}
			break;
		case 2: //	HORIZONTAL
			{
				glFrontFace(GL_CCW);
			}
			break;
		case 0: // DEFAULT
		case 3: // VERTICAL & HORIZONTAL
		default:
			break;
	}
}

void EffectGroup::drawFull(RenderItemContext* pctx, float frame, int reverse, float aspect_ratio, float fov){

	Matrix4 matNode[2048];
	Vector3 light_dir, eye_dir;
	Matrix4 view;

	prepare(matNode, view, light_dir, eye_dir, frame, reverse);

	for(int ab_flag = 0; ab_flag < 3; ++ab_flag){

		if(ab_flag == 1){

			glDepthMask(GL_FALSE);
		}
		else if(ab_flag == 2){

			glBlendFunc(GL_ONE, GL_ONE);
		}

		for(std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

			BaseAniMesh* pmesh = (BaseAniMesh*)(*itor);
			
			if(ab_flag == 2){

				if(!pmesh->getFlareFlag()){

					continue;
				}
			}
			else{

				if(pmesh->getFlareFlag())
					continue;
			}
			
			if(ab_flag == 1){

				if(!pmesh->getAlphaBlendFlag())
					continue;
			}

			pmesh->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
			pmesh->setLightDir(light_dir.x, light_dir.y, light_dir.z);
			pmesh->draw(pctx, frame, matNode, &view, ab_flag, aspect_ratio, fov);
		}

	}

	if(reverse)
		glFrontFace(GL_CCW);
}

void EffectGroup::draw(int video_src_0, const float* pmatrix_for_video_src_0, int video_src_1, const float* pmatrix_for_video_src_1, float frame, int ab_flag, float aspect_ratio, float fov){

	Matrix4 matNode[2048];
	Vector3 light_dir, eye_dir;
	Matrix4 view;

	prepare(matNode, view, light_dir, eye_dir, frame, 0);

	for(std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		BaseAniMesh* pmesh = (BaseAniMesh*)(*itor);
		
		if(ab_flag == 2){

			if(!pmesh->getFlareFlag()){

				continue;
			}
		}
		else{

			if(pmesh->getFlareFlag())
				continue;
		}

		if(ab_flag == 1){

			if(!pmesh->getAlphaBlendFlag())
				continue;
		}

		pmesh->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
		pmesh->setLightDir(light_dir.x, light_dir.y, light_dir.z);
		pmesh->draw(video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, frame, matNode, &view, ab_flag, aspect_ratio, fov);
	}
}

bool EffectGroup::deserialize(InputStream& stream){

	int count_of_meshes = 0;

	count_of_frame_ = 0;

	stream.read(&count_of_meshes, sizeof(count_of_meshes), 1);
	stream.read(&count_of_frame_, sizeof(count_of_frame_), 1);
	
	int total_node = 0;
	stream.read(&total_node, sizeof(total_node), 1);
	nexSAL_TraceCat(0, 0, "[%s %d] total_node:%d count_of_frame_:%d count_of_meshes:?%d", __func__, __LINE__, total_node, count_of_frame_, count_of_meshes);
	for (int i = 0; i < total_node; ++i){

		Track* ptrack = new Track();
		ptrack->init();
		ptrack->deserializerMatrixKey(stream, count_of_frame_);
		node_list_.push_back(ptrack);
	}

	for(int i = 0; i < count_of_meshes; ++i){

		BaseAniMesh* pmesh = new BaseAniMesh();
		if(!pmesh->deserialize(stream)){

			SAFE_DELETE( pmesh );
		}

		if(pmesh){

			meshes_.push_back(pmesh);
		}
	}

	track_.init();
	track_.deserializerMatrixKey(stream, count_of_frame_);
    return true;
}

bool EffectGroup::deserialize(InputStream& stream, char* uid, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pRelaseContextFunc)(void* p)){

	int count_of_meshes = 0;

	count_of_frame_ = 0;

	stream.read(&count_of_meshes, sizeof(count_of_meshes), 1);
	stream.read(&count_of_frame_, sizeof(count_of_frame_), 1);
	
	int total_node = 0;
	stream.read(&total_node, sizeof(total_node), 1);
	nexSAL_TraceCat(0, 0, "[%s %d] total_node:%d count_of_frame_:%d count_of_meshes:?%d", __func__, __LINE__, total_node, count_of_frame_, count_of_meshes);
	for (int i = 0; i < total_node; ++i){

		Track* ptrack = new Track();
		ptrack->init();
		ptrack->deserializerMatrixKey(stream, count_of_frame_);
		node_list_.push_back(ptrack);
	}

	for(int i = 0; i < count_of_meshes; ++i){

		BaseAniMesh* pmesh = new BaseAniMesh();
		if(!pmesh->deserialize(stream, uid, prenderer, pAquireContextFunc, pRelaseContextFunc)){

			SAFE_DELETE( pmesh );
		}

		if(pmesh){

			meshes_.push_back(pmesh);
		}
	}

	track_.init();
	track_.deserializerMatrixKey(stream, count_of_frame_);
    return true;
}