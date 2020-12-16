#pragma once

#include "RenderItem.h"

struct StreamingItem{

	GLuint tex_id_;
	RenderItem* pitem_;
	int start_time_;
	int duration_;
	int layer_;
	int order_;

	StreamingItem() :pitem_(NULL), tex_id_(0), start_time_(0), duration_(0), layer_(0), order_(0){

	}

	~StreamingItem(){

	}

	void release(){

		SAFE_DELETE(pitem_);
		if (tex_id_ > 0)
			glDeleteTextures(1, &tex_id_);
	}

	bool operator <(const StreamingItem& right) const{

		return this->order_ < right.order_;
	}
};

class StreamingProject
{
	typedef std::vector<StreamingItem> ImageItemList_t;

	ImageItemList_t item_list_;
	ImageItemList_t current_item_list_;
	std::string		filename_;

	int loadProject(rapidxml::xml_node<> *node, ImageItemList_t& itemlist);
public:
	StreamingProject();
	~StreamingProject();

	int reload();

	int loadProject(const char* filename);

	void doPlay(int elapsed_tick);

	void doRender(NXT_HThemeRenderer renderer);

	void freeResource();

	size_t getItemCount();
};

