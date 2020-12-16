#include "../types.h"
#include "material.h"
#include "mesh.h"
#include "../Singleton.h"
#include "Matrices.h"
#include "Vectors.h"
#include "../util.h"

#define MAX_BONE_INDEX	4

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

	void deserialize(FILE* fp){

		freadT(&pos_, sizeof(pos_), 1, fp);
		freadT(&uv_, sizeof(uv_), 1, fp);
		freadT(&normal_, sizeof(normal_), 1, fp);
		freadT(&tangent_, sizeof(tangent_), 1, fp);
		freadT(&binormal_, sizeof(binormal_), 1, fp);

		for (int i = 0; i < MAX_BONE_INDEX; ++i){

			freadT(&weight_[i].idx_, sizeof(weight_[i].idx_), 1, fp);
			freadT(&weight_[i].weight_, sizeof(weight_[i].weight_), 1, fp);
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

	void deserialize(FILE* fp){

		freadT(&pos_, sizeof(pos_), 1, fp);
		freadT(&uv_, sizeof(uv_), 1, fp);
		freadT(&normal_, sizeof(normal_), 1, fp);
		freadT(&tangent_, sizeof(tangent_), 1, fp);
		freadT(weight_, sizeof(weight_), MAX_BONE_INDEX, fp);
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
	virtual Skin* clone(){

		return new Skin();
	}

	void draw(float x, float y, float z, float r, BoneAnimator& animator);
};

void NinjaVertexForLightmap::deserialize(FILE* fp){

	Vector3 binormal;

	freadT(&pos_, sizeof(pos_), 1, fp);
	freadT(&uv_, sizeof(uv_), 1, fp);
	freadT(&normal_, sizeof(normal_), 1, fp);
	freadT(&binormal, sizeof(binormal), 1, fp);
	freadT(&tangent_, sizeof(tangent_), 1, fp);
	freadT(uv_for_lightmap_, sizeof(uv_for_lightmap_), MAX_LIGHTMAP, fp);
}

void NinjaVertex::deserialize(FILE* fp){

	Vector3 binormal;

	freadT(&pos_, sizeof(pos_), 1, fp);
	freadT(&uv_, sizeof(uv_), 1, fp);
	freadT(&normal_, sizeof(normal_), 1, fp);
	freadT(&binormal, sizeof(binormal), 1, fp);
	freadT(&tangent_, sizeof(tangent_), 1, fp);
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
bool BaseMesh<T, iT>::deserialize(FILE* fp){

	free();
	
	int count_of_vertices = 0;
	int count_of_faces = 0;

	fread(&count_of_vertices, sizeof(int), 1, fp);	
	vertices_.reserve(count_of_vertices);

	for (int i = 0; i < count_of_vertices; ++i){

		T vtx;
		vtx.deserialize(fp);
		vertices_.push_back(vtx);
	}

	fread(&count_of_faces, sizeof(int), 1, fp);
	indices_.reserve(count_of_faces * 3);

	for (int i = 0; i < count_of_faces; ++i){

		iT vtx[3];
		fread(vtx, sizeof(iT), 3, fp);
		indices_.push_back(vtx[0]);
		indices_.push_back(vtx[1]);
		indices_.push_back(vtx[2]);
	}

	//vertices_.shrink_to_fit();
	//indices_.shrink_to_fit();

	return true;
}



template<>
bool BaseMesh<SkinVertex, unsigned short>::deserialize(FILE* fp){

	free();

	int count_of_vertices = 0;
	int count_of_faces = 0;

	fread(&count_of_vertices, sizeof(int), 1, fp);
	vertices_.reserve(count_of_vertices);

	for (int i = 0; i < count_of_vertices; ++i){

		SkinVertex_deprecated vtx_old;
		SkinVertex vtx;
		vtx_old.deserialize(fp);
		vtx = vtx_old;
		vertices_.push_back(vtx);
	}

	fread(&count_of_faces, sizeof(int), 1, fp);
	count_of_faces /= 3;
	indices_.reserve(count_of_faces * 3);

	for (int i = 0; i < count_of_faces; ++i){

		int vtx[3];
		fread(vtx, sizeof(int), 3, fp);
		indices_.push_back(vtx[0]);
		indices_.push_back(vtx[1]);
		indices_.push_back(vtx[2]);
	}

	return true;
}

template<typename T, typename iT>
bool BaseMesh<T, iT>::serialize(FILE* fp){

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

void MatSolidColor::setShaderSetting(ColoredVertex* vertices, float x, float y, float z, float r){

	glUseProgram(program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.prespective(30.0f, 1280.0f / 720.0f, 0.01f, 6000.0f);
	trans.translate(x, y, z);
	rotate.rotateX(r);

	world = trans * rotate;
	matrix = matrix * world;

	glUniformMatrix4fv(mvp_matrix_uniform, 1, false, matrix.get());

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(color_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(color_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &vertices[0].color_);    CHECK_GL_ERROR();
}

void MatSolidColor::setShaderSetting(ColoredVertex* vertices, Matrix4& view, float x, float y, float z, float xrotate, float yrotate, float zrotate){

	glUseProgram(program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.prespective(30.0f, 1280.0f / 720.0f, 0.01f, 12000.0f);
	trans.translate(x, y, z);
	rotate.rotateX(xrotate);
	rotate.rotateY(yrotate);	
	rotate.rotateZ(zrotate);

	world = trans * rotate;
	matrix = matrix * view * world;

	glUniformMatrix4fv(mvp_matrix_uniform, 1, false, matrix.get());

	if (ptex_){
		glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, ptex_->getTexID());                                   CHECK_GL_ERROR();
		glUniform1i(texture_uniform, 0);                                           CHECK_GL_ERROR();
	}

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(uv_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(color_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, false, sizeof(ColoredVertex), &(vertices[0].uv_));    CHECK_GL_ERROR();
	glVertexAttribPointer(color_attrib, 3, GL_FLOAT, false, sizeof(ColoredVertex), &vertices[0].color_);    CHECK_GL_ERROR();
}

void MatA::setShaderSetting(SkinVertex* vertices, float x, float y, float z, float r){

	glUseProgram(program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.prespective(30.0f, 1280.0f / 720.0f, 0.01f, 3000.0f);
	trans.translate(x, y, z);
	rotate.rotateY(r);

	world = trans * rotate;
	matrix = matrix * world;

	glUniformMatrix4fv(mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(world_matrix_uniform, 1, false, world.get());
	glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(texture_uniform, 0);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[NORMAL]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(normal_uniform, 1);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 2);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[SPEC]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(spec_uniform, 2);                                           CHECK_GL_ERROR();

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(tangent_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(binormal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();

	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(tangent_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].tangent_);    CHECK_GL_ERROR();
	glVertexAttribPointer(binormal_attrib, 3, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].binormal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, sizeof(SkinVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

void MatA::setShaderSetting(NinjaVertex* vertices, float x, float y, float z, float r){

	glUseProgram(program_idx);

	Matrix4 matrix;
	Matrix4 world;
	Matrix4 trans;
	Matrix4 rotate;

	matrix.prespective(30.0f, 1280.0f / 720.0f, 0.01f, 3000.0f);
	trans.translate(x, y, z);
	rotate.rotateY(r);

	world = trans * rotate;
	matrix = matrix * world;
		
	glUniformMatrix4fv(mvp_matrix_uniform, 1, false, matrix.get());
	glUniformMatrix4fv(world_matrix_uniform, 1, false, world.get());
	glActiveTexture(GL_TEXTURE0 + 0);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[0]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(texture_uniform, 0);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 1);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[NORMAL]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(normal_uniform, 1);                                           CHECK_GL_ERROR();

	glActiveTexture(GL_TEXTURE0 + 2);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture_[SPEC]->getTexID());                                   CHECK_GL_ERROR();
	glUniform1i(spec_uniform, 2);                                           CHECK_GL_ERROR();

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(tangent_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(binormal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	
	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, false, sizeof(NinjaVertex), &(vertices[0].pos_));    CHECK_GL_ERROR();
	glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, false, sizeof(NinjaVertex), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(tangent_attrib, 3, GL_FLOAT, false, sizeof(NinjaVertex), &vertices[0].tangent_);    CHECK_GL_ERROR();
	glVertexAttribPointer(binormal_attrib, 3, GL_FLOAT, false, sizeof(NinjaVertex), &vertices[0].binormal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, sizeof(NinjaVertex), &vertices[0].uv_);  CHECK_GL_ERROR();
}

void MatC::setShaderSetting(NinjaVertexForLightmap* vertices, float x, float y, float z, float r){

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(normal_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(tangent_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_for_lightmap_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, sizeof(vertices[0].pos_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].pos_);    CHECK_GL_ERROR();
	glVertexAttribPointer(normal_attrib, sizeof(vertices[0].normal_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].normal_);    CHECK_GL_ERROR();
	glVertexAttribPointer(tangent_attrib, sizeof(vertices[0].tangent_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].tangent_);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, sizeof(vertices[0].uv_) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].uv_);  CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_for_lightmap_attrib, sizeof(vertices[0].uv_for_lightmap_[lightmap_channel_]) / sizeof(float), GL_FLOAT, false, sizeof(vertices[0]), &vertices[0].uv_for_lightmap_[lightmap_channel_]);  CHECK_GL_ERROR();
}

template<typename T, typename iT>
void BaseMesh<T, iT>::draw(float x, float y, float z, float r){
	
	getMtl()->setShaderSetting(&vertices_[0], x, y, z, r);

	Vector3 light_dir(1.0f, 0.0f, 0.0f);
	Vector3 eye_dir(0.0f, 0.0f, -1.0f);
	/*Matrix4 matrix;
	matrix.rotateY(r);
	eye_dir = eye_dir * matrix;*/
	getMtl()->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
	getMtl()->setLightDir(light_dir.x, light_dir.y, light_dir.z);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);	CHECK_GL_ERROR();
}

template<>
void BaseMesh<SkinVertex, unsigned short>::draw(float x, float y, float z, float r){

	getMtl()->setShaderSetting(&vertices_[0], x, y, z, r);

	Vector3 light_dir(1.0f, 0.0f, 0.0f);
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

void MeshGroup::draw(float x, float y, float z, float r){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Mesh* mesh = *itor;
		mesh->draw(x, y, z, r);
	}
}

#define MESH_V11	0xa2efab13
#define MESH_V12	0x3d12efef
#define MESH_V13	0x3d13efef

#define NINJASKIN_V11	0x3defab13
#define NINJASKIN_V12	0x3defab15
#define NINJASKIN_V13	0x3defab17
#define NINJASKIN_V14	0x3defab18

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

	void serialize(FILE* fp){

		fwrite(&min_, sizeof(min_), 1, fp);
		fwrite(&max_, sizeof(max_), 1, fp);
		fwrite(&speed_, sizeof(speed_), 1, fp);
		fwrite(&start_, sizeof(start_), 1, fp);
	}

	void deserialize(FILE* fp){

		fread(&min_, sizeof(min_), 1, fp);
		fread(&max_, sizeof(max_), 1, fp);
		fread(&speed_, sizeof(speed_), 1, fp);
		fread(&start_, sizeof(start_), 1, fp);
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

	void serialize(FILE* fp){

		fwrite(&blend_mode_, sizeof(blend_mode_), 1, fp);
		fwrite(&billboard_mode_, sizeof(billboard_mode_), 1, fp);
		fwrite(&culling_mode_, sizeof(culling_mode_), 1, fp);
		fwrite(&u_for_uvani_, sizeof(u_for_uvani_), 1, fp);
		fwrite(&v_for_uvani_, sizeof(v_for_uvani_), 1, fp);

		alpha_.serialize(fp);
		red_.serialize(fp);
		green_.serialize(fp);
		blue_.serialize(fp);
	}

	void deserialize(FILE* fp){

		fread(&blend_mode_, sizeof(blend_mode_), 1, fp);
		fread(&billboard_mode_, sizeof(billboard_mode_), 1, fp);
		fread(&culling_mode_, sizeof(culling_mode_), 1, fp);
		fread(&u_for_uvani_, sizeof(u_for_uvani_), 1, fp);
		fread(&v_for_uvani_, sizeof(v_for_uvani_), 1, fp);
		alpha_.deserialize(fp);
		red_.deserialize(fp);
		green_.deserialize(fp);
		blue_.deserialize(fp);
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

	void serialize(FILE* fp){

		fwrite(&billboard_mode_, sizeof(billboard_mode_), 1, fp);
		fwrite(&culling_mode_, sizeof(culling_mode_), 1, fp);
		fwrite(&blend_mode_, sizeof(blend_mode_), 1, fp);
		fwrite(&alpharef_, sizeof(alpharef_), 1, fp);
		fwrite(&alphablend_, sizeof(alphablend_), 1, fp);
		fwrite(&special_render_mode_, sizeof(special_render_mode_), 1, fp);
		fwrite(&u_for_uvani_, sizeof(u_for_uvani_), 1, fp);
		fwrite(&v_for_uvani_, sizeof(v_for_uvani_), 1, fp);
		alpha_.serialize(fp);
		red_.serialize(fp);
		green_.serialize(fp);
		blue_.serialize(fp);
	}

	void deserialize(FILE* fp){
		
		fread(&billboard_mode_, sizeof(billboard_mode_), 1, fp);
		fread(&culling_mode_, sizeof(culling_mode_), 1, fp);
		fread(&blend_mode_, sizeof(blend_mode_), 1, fp);
		fread(&alpharef_, sizeof(alpharef_), 1, fp);
		fread(&alphablend_, sizeof(alphablend_), 1, fp);
		fread(&special_render_mode_, sizeof(special_render_mode_), 1, fp);
		fread(&u_for_uvani_, sizeof(u_for_uvani_), 1, fp);
		fread(&v_for_uvani_, sizeof(v_for_uvani_), 1, fp);
		alpha_.deserialize(fp);
		red_.deserialize(fp);
		green_.deserialize(fp);
		blue_.deserialize(fp);
	}
};

bool MeshGroup::deserialize(FILE* fp){

	int count_of_meshes = 0;
	int count_of_physxobj = 0;
	
	fread(&count_of_meshes, sizeof(int), 1, fp);
	fread(&count_of_physxobj, sizeof(int), 1, fp);
	
	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(fp);

		int type_of_mtl = 0;
		fread(&type_of_mtl, sizeof(int), 1, fp);
		Material* mtl = Singleton<Factory<Material> >::getInstance()->create(type_of_mtl);
		mtl->deserialize(fp);

		Mesh* mesh = Singleton<Factory<Mesh> >::getInstance()->create(type_of_mtl);
		mesh->deserialize(fp);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

void initMeshFactory(){

	Singleton<Factory<MeshGroup> >::getInstance()->set(MESH_V13, new MeshGroup());
	Singleton<Factory<MeshGroup> >::getInstance()->set(NINJASKIN_V14, new SkinGroup());

	Singleton<Factory<Material> >::getInstance()->set(MAT_SKINMAT, new MatSkin());

	Singleton<Factory<Material> >::getInstance()->set(MAT_BGFX_C, new MatC());

	Singleton<Factory<Mesh> >::getInstance()->set(MAT_BGFX_C, new BaseMesh<NinjaVertexForLightmap, int>());

	Singleton<Factory<Material> >::getInstance()->set(MAT_BGFX_A, new MatA());

	Singleton<Factory<Mesh> >::getInstance()->set(MAT_BGFX_A, new BaseMesh<NinjaVertex, int>());

	Singleton<Factory<Mesh> >::getInstance()->set(MAT_SKINMAT, new BaseMesh<NinjaVertex, int>());
}



bool MeshGroup::serialize(FILE* fp){

	return true;
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

	bool deserialize(FILE* fp){

		int count, data_type, bone_index;

		freadT(&bone_index, sizeof(bone_index), 1, fp);

		while (1){

			freadT(&data_type, sizeof(data_type), 1, fp);
			
			if (data_type >= E_END_KEY) 
				return true;

			if (data_type == E_POS_KEY){

				AniKey<Vector3> key;
				freadT(&count, sizeof(count), 1, fp);

				for (int i = 0; i < count; ++i){

					freadT(&key.frame_, sizeof(key.frame_), 1, fp);
					freadT(&key.value_, sizeof(key.value_), 1, fp);
					position_key_.push_back(key);
				}
			}
			else if (data_type == E_ROT_KEY){

				AniKey<Quaternion> key;
				freadT(&count, sizeof(count), 1, fp);
				
				Quaternion preQuat(0, 0, 0, 1);
				Quaternion tQuat;
				
				for (int i = 0; i < count; ++i){

					freadT(&key.frame_, sizeof(key.frame_), 1, fp);
					freadT(&tQuat, sizeof(tQuat), 1, fp);
					key.value_ = preQuat * tQuat;
					preQuat = key.value_;
					rotation_key_.push_back(key);
				}
			}
			else if (data_type == E_SCL_KEY){

				AniKey<Vector3> key;
				freadT(&count, sizeof(count), 1, fp);

				for (int i = 0; i < count; ++i){

					freadT(&key.frame_, sizeof(key.frame_), 1, fp);
					freadT(&key.value_, sizeof(key.value_), 1, fp);
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

	bool deserialize(FILE* fp){

		int count_of_bone;

		freadT(&count_of_bone, sizeof(count_of_bone), 1, fp);
		data_.resize(count_of_bone);
		freadT(&start_frame_, sizeof(start_frame_), 1, fp);
		freadT(&end_frame_, sizeof(end_frame_), 1, fp);
		int duration = end_frame_ - start_frame_;

		for (int i = 0; i < count_of_bone; ++i){

			data_[i].setDuration(duration);
			data_[i].deserialize(fp);
		}
		return true;
	}

	bool serialize(FILE* fp){

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

		bool deserialize(FILE* fp){

			float m[16];

			freadT(&parent_bone_index_, sizeof(parent_bone_index_), 1, fp);
			fread(m, sizeof(float), 16, fp);
			world_matrix_.set(m);
			world_matrix_.transpose();
			fread(&kinematic_flag_, sizeof(kinematic_flag_), 1, fp);
			inv_world_matrix_ = world_matrix_;			
			inv_world_matrix_.invert();
			return true;
		}
	};


	std::vector<Node> nodelist_;

	size_t getCount(){

		return nodelist_.size();
	}

	bool deserialize(FILE* fp){

		int version_check = 0;
		int count_of_node = 0;

		freadT(&version_check, sizeof(version_check), 1, fp);
		freadT(&count_of_node, sizeof(count_of_node), 1, fp);

		nodelist_.resize(count_of_node);
		//nodelist_.shrink_to_fit();

		for (int i = 0; i < count_of_node; ++i){
		
			nodelist_[i].deserialize(fp);
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


SkinGroup* SkinGroup::clone(){

	return new SkinGroup();
}

bool SkinGroup::deserialize(FILE* fp){

	int count_of_meshes = 0;
	int count_of_cutbox = 0;

	fread(&count_of_meshes, sizeof(int), 1, fp);
	fread(&count_of_cutbox, sizeof(int), 1, fp);

	for (int i = 0; i < count_of_meshes; ++i){

		MeshEffect2 mesheffect;
		mesheffect.deserialize(fp);

		int type_of_mtl = 0;
		fread(&type_of_mtl, sizeof(int), 1, fp);
		Material* mtl = Singleton<Factory<Material> >::getInstance()->create(type_of_mtl);
		mtl->deserialize(fp);

		Mesh* mesh = new Skin();
		mesh->deserialize(fp);
		mesh->setMtl(mtl);

		meshes_.push_back(mesh);
	}

	return true;
}

bool SkinGroup::serialize(FILE* fp){

	return true;
}

std::vector<MeshGroup*> testMeshList;
Bone test_bone;
KeyAnimation test_ani;
KeyAniInstance test_ani_instance;
BoneAnimator test_animator;

void testMeshGroup(const char* filename){

	FILE* fp = NULL;
	fp = fopen(filename, "rb");

	if (NULL == fp)
		return;

	int chk = 0;
	TextureChunk chunk;
	while (chunk.deserialize(fp)){

		++chk;
	}
	if (chk <= 0){

		printf("%s has No Texture Chunk\n", filename);
	}

	int type_of_meshgroup = 0;
	fread(&type_of_meshgroup, sizeof(int), 1, fp);

	MeshGroup* meshgroup = Singleton<Factory<MeshGroup> >::getInstance()->create(type_of_meshgroup);
	meshgroup->deserialize(fp);
	testMeshList.push_back(meshgroup);
	fclose(fp);
}

void testBone(const char* filename){

	FILE* fp = NULL;
	fp = fopen(filename, "rb");

	if (NULL == fp)
		return;

	test_bone.deserialize(fp);

	fclose(fp);
}

void testAni(const char* filename){

	FILE* fp = NULL;
	fp = fopen(filename, "rb");

	if (NULL == fp)
		return;

	test_ani.deserialize(fp);

	fclose(fp);

	test_ani_instance.setKeyAnimation(&test_ani);
}

void Skin::draw(float x, float y, float z, float r, BoneAnimator& animator){
	
	NinjaVertex vtx[4096];

	VerticesType& vertices = getVertices();
	IndicesType& indices = getIndices();

	for (size_t i = 0; i < vertices.size(); ++i){

		SkinVertex& skin_vtx = vertices[i];
		vtx[i].pos_ = Vector3();
		vtx[i].tangent_ = Vector3();
		vtx[i].normal_ = Vector3();
		vtx[i].binormal_ = Vector3();
		vtx[i].uv_ = skin_vtx.uv_;

		for (int j = 0; j < MAX_BONE_INDEX; ++j){

			int bone_idx = skin_vtx.weight_[j].idx_;
			if (bone_idx < 0){

				if (j == 0){

					bone_idx = 0;
				}
				else break;
			}
			
			float weight = skin_vtx.weight_[j].weight_;
			if (weight <= 0.0f){

				if (j == 0){

					weight = 1.0f;
				}
				else break;
			}
			
			Vector4 tmp = (Vector4(skin_vtx.pos_.x, skin_vtx.pos_.y, skin_vtx.pos_.z, 1.0f) * animator.matrix_palette_[bone_idx]);

			vtx[i].pos_ += (Vector3(tmp.x, tmp.y, tmp.z) * weight);
			vtx[i].normal_ += ((skin_vtx.normal_ * animator.matrix_palette_[bone_idx]) * weight);
			vtx[i].tangent_ += ((skin_vtx.tangent_ * animator.matrix_palette_[bone_idx]) * weight);
			vtx[i].binormal_ += ((skin_vtx.binormal_ * animator.matrix_palette_[bone_idx]) * weight);
		}
	}

	getMtl()->setShaderSetting(vtx, x, y, z, 180.0f);

	Vector3 light_dir(1.0f, 0.0f, 0.0f);
	Vector3 eye_dir(0.0f, 0.0f, -1.0f);
	Matrix4 matrix;
	matrix.rotateY(r);
	light_dir = light_dir * matrix;
	getMtl()->setEyeDir(eye_dir.x, eye_dir.y, eye_dir.z);
	getMtl()->setLightDir(light_dir.x, light_dir.y, light_dir.z);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);	CHECK_GL_ERROR();
}

void SkinGroup::draw(float x, float y, float z, float r, BoneAnimator& animator){

	for (std::vector<Mesh*>::iterator itor = meshes_.begin(); itor != meshes_.end(); ++itor){

		Skin* mesh = (Skin*)*itor;
		mesh->draw(x + 0.0f, y, z, r, animator);
	}
}