#pragma once

#include <vector>
#include "types.h"
#include "util.h"
#include "img.h"


struct AniFrame{

	float		timing_;
	TextureBin* ptex_;
	float		alpha_;

	AniFrame(float timing, float alpha, TextureBin* pbin) :timing_(timing), alpha_(alpha), ptex_(pbin){

		SAFE_ADDREF(pbin);
	}

	AniFrame(){

		timing_ = 0.0f;
		ptex_ = NULL;
		alpha_ = 0.0f;
	}

	AniFrame(const AniFrame& ani){

		timing_ = ani.timing_;
		ptex_ = ani.ptex_;
		alpha_ = ani.alpha_;
		SAFE_ADDREF(ptex_);
	}

	~AniFrame(){

		SAFE_RELEASE(ptex_);
	}
};

struct SnapshotOfAniFrame{

	GLuint tex_id_;
	float alpha_;

	SnapshotOfAniFrame() :tex_id_(0), alpha_(0.0f){

	}
};

class RenderItemContext;

class Animation{
public:
	void load(RenderItem* ctx, rapidxml::xml_node<> *node);
	SnapshotOfAniFrame getAniFrame(float timing);

private:
	typedef std::vector<AniFrame> anilist_t;
	anilist_t anilist_;

	void arrangeTiming();
	void add(const AniFrame& ani);
};

class AnimationBin :public Bin{

	bool orphant_;
	Animation* data_;
public:
	AnimationBin() :data_(NULL), orphant_(false){

		setLife(1);
	}

	~AnimationBin(){

		SAFE_DELETE(data_);
	}

	Animation* get(){

		return data_;
	}

	void set(Animation* data){

		data_ = data;
	}

	void setOrphant(bool orphant){

		orphant_ = orphant;
	}

	bool isOrphant(){

		return orphant_;
	}
};
