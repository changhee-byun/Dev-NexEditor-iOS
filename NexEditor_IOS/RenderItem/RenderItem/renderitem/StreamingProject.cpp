#include "streamingProject.h"
#include <algorithm>

struct compStreamingItem{

	bool operator()(const StreamingItem& left, const StreamingItem& right){

		if (left.layer_ != right.layer_)
			return left.layer_ < right.layer_;
		return left.order_ < right.order_;
	}
};

int StreamingProject::reload(){

	freeResource();
	return loadProject(filename_.c_str());
}

int StreamingProject::loadProject(const char* filename){

	std::string cwd(get_current_dir_name_mine());
	Chdir(dirname(std::string(filename)).c_str());

	std::string buf;

	readFromFile(basename(std::string(filename)).c_str(), buf, true);

	rapidxml::xml_document<> doc;
	char* src_text = new char[buf.length()];
	memcpy(src_text, buf.c_str(), sizeof(char)* buf.length());
	doc.parse<0>(src_text);

	int perf = getTickCount();
	int ret = loadProject(&doc, item_list_);
	perf = getTickCount() - perf;
	printf("_Load Perf:%d\n", perf);

	Chdir(cwd.c_str());

	filename_ = std::string(filename);

	return ret;
}

int StreamingProject::loadProject(rapidxml::xml_node<> *node, ImageItemList_t& itemlist){

	int total_play_tick = 0;

	rapidxml::xml_node<> *frame_node = getNode(node, "project", NULL, NULL);
	while (frame_node){

		rapidxml::xml_node<> *image_node = getNode(frame_node, "object", NULL, NULL);

		while (image_node){

			char* src_val = getAttribStr(image_node, "src");
			char* media_val = getAttribStr(image_node, "media");
			char* start_val = getAttribStr(image_node, "start");
			char* duration_val = getAttribStr(image_node, "duration");
			char* layer_val = getAttribStr(image_node, "layer");
			char* child_count_val = getAttribStr(image_node, "child_count");

			printf("%s %s %s %s %s %s\n", src_val, media_val, start_val, duration_val, layer_val, child_count_val);

			StreamingItem item;

			float w = 0;
			float h = 0;
			int child_count = child_count_val ? atoi(child_count_val) : 0;
			item.tex_id_ = media_val ? Img::loadImage(media_val, &w, &h) : 0;
#ifdef RENDERITEM_OLD_LOAD
			item.pitem_ = new RenderItem(child_count);
			if (src_val) item.pitem_->load(src_val);
#else
			std::string check_name = std::string(get_current_dir_name_mine()) + std::string("\\") + std::string(src_val);
			ResourceManager<RenderItemBin>& manager = *(Singleton<ResourceManager<RenderItemBin> >::getInstance());
			RenderItemBin* prender_item_bin = manager.get(check_name);
			if (prender_item_bin == NULL){

				prender_item_bin = new RenderItemBin();
				prender_item_bin->set(src_val, child_count);
				manager.insert(check_name, prender_item_bin);
			}
			prender_item_bin->addRef();
			RenderItem* prender_item = prender_item_bin->get();
			item.pitem_ = new RenderItem(prender_item);
#endif//RENDERITEM_OLD_LOAD
			item.pitem_->setChildCapacity(child_count);

			item.layer_ = layer_val ? atoi(layer_val) : 0;
			item.start_time_ = start_val ? atoi(start_val) : 0;
			item.duration_ = duration_val ? atoi(duration_val) : 0;
			itemlist.push_back(item);
			int tick = item.start_time_ + item.duration_;
			total_play_tick = tick > total_play_tick ? tick : total_play_tick;

			image_node = getNextNode(image_node, "object", NULL, NULL);
		}

		frame_node = getNextNode(frame_node, "project", NULL, NULL);
	}

	return total_play_tick;
}



StreamingProject::StreamingProject(){

}

StreamingProject::~StreamingProject(){

	freeResource();
}

void StreamingProject::doPlay(int elapsed_tick){

	int part_type = NXT_PartType_All;

	int clip_index = 0;

	current_item_list_.clear();

	for (ImageItemList_t::iterator itor = item_list_.begin(); itor != item_list_.end(); ++itor){

		StreamingItem& item = (*itor);
		if (item.start_time_ <= elapsed_tick && (item.start_time_ + item.duration_) >= elapsed_tick){

			float current_time_overall = (float)(elapsed_tick - item.start_time_) / (float)item.duration_;
			float current_time_in_part = (float)(elapsed_tick - item.start_time_) / (float)item.duration_;

			bool inserted(false);
			for (ImageItemList_t::iterator itor_for_insert = current_item_list_.begin(); itor_for_insert != current_item_list_.end(); ++itor_for_insert){

				StreamingItem& cmp = (*itor_for_insert);

				if (cmp.layer_ < item.layer_)
					continue;
				else if (cmp.layer_ > item.layer_){
					current_item_list_.insert(itor_for_insert, item);
					inserted = true;
					break;
				}
				else if (cmp.start_time_ > item.start_time_){

					current_item_list_.insert(itor_for_insert, item);
					inserted = true;
					break;
				}
			}

			if (!inserted){

				current_item_list_.push_back(item);
			}

			item.pitem_->doEffect(elapsed_tick, current_time_in_part, current_time_overall, part_type, clip_index, item_list_.size());
		}
		++clip_index;
	}

	for (size_t i = 0; i < current_item_list_.size(); ++i){

		StreamingItem& image_item = current_item_list_[i];
		image_item.order_ = i;
		image_item.pitem_->broadcastGlobalVariable("system.video_src", image_item.tex_id_);
		image_item.pitem_->broadcastGlobalVariable("system.play_percentage", (float)(elapsed_tick - image_item.start_time_) / (float)image_item.duration_);
	}

	for (ImageItemList_t::iterator itor = current_item_list_.begin(); itor != current_item_list_.end();){

		bool changed = false;
		StreamingItem& item = *itor;
		for (ImageItemList_t::iterator itor_cmp = itor + 1; itor_cmp != current_item_list_.end(); ++itor_cmp){

			StreamingItem& check_item = (*itor_cmp);
			if (check_item.layer_ <= item.layer_)
				continue;
			if (check_item.pitem_->getChildCapacity() > 0){

				check_item.pitem_->addChild(item.pitem_, item.order_);

				if (item.order_ < check_item.order_)
					check_item.order_ = item.order_;

				current_item_list_.erase(itor);
				std::sort(current_item_list_.begin(), current_item_list_.end(), compStreamingItem());
				itor = current_item_list_.begin();
				changed = true;
				break;
			}
		}

		if (!changed)
			++itor;
	}

	std::sort(current_item_list_.begin(), current_item_list_.end());
}

void StreamingProject::doRender(NXT_HThemeRenderer renderer){

	for (ImageItemList_t::iterator itor = current_item_list_.begin(); itor != current_item_list_.end(); ++itor){

		glClear(GL_DEPTH_BUFFER_BIT);
		StreamingItem& item = *itor;
		item.pitem_->loop(renderer);
	}
}

void StreamingProject::freeResource(){

	for (ImageItemList_t::iterator itor = item_list_.begin(); itor != item_list_.end(); ++itor){

		(*itor).release();
	}
	item_list_.clear();
}

size_t StreamingProject::getItemCount(){

	return item_list_.size();
}