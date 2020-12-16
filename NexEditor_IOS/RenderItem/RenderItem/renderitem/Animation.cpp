#include "glext_loader.h"
#include "Animation.h"
#include <vector>

void Animation::add(const AniFrame& ani){

	anilist_.push_back(ani);
}

SnapshotOfAniFrame Animation::getAniFrame(float timing){

	SnapshotOfAniFrame ret;

	for (anilist_t::iterator itor = anilist_.begin(); itor != anilist_.end(); ++itor){

		AniFrame& frame = *itor;

		ret.tex_id_ = frame.ptex_->getTexID();
		ret.alpha_ = frame.alpha_;

		if (timing <= frame.timing_){

			return ret;
		}
	}

	return ret;
}

void Animation::arrangeTiming(){

	float prev_timing = 0.0f;
	std::vector<AniFrame*> affected_anilist;

	for (anilist_t::iterator itor = anilist_.begin(); itor != anilist_.end(); ++itor){

		AniFrame& frame = *itor;
		if (frame.timing_ >= 0.0f){

			if (affected_anilist.size() > 0){

				float timing_gap = (frame.timing_ - prev_timing) / (float)(affected_anilist.size() + 1);

				for (size_t i = 0; i < affected_anilist.size(); ++i){

					affected_anilist[i]->timing_ = prev_timing + timing_gap * (float)(i + 1);
				}
			}
			prev_timing = frame.timing_;
			affected_anilist.clear();
		}
		else{

			affected_anilist.push_back(&frame);
		}
	}

	if (affected_anilist.size() > 0){

		float timing_gap = (1.0f - prev_timing) / (float)(affected_anilist.size());

		for (size_t i = 0; i < affected_anilist.size(); ++i){

			affected_anilist[i]->timing_ = prev_timing + timing_gap * (float)(i + 1);
		}
	}
}

void Animation::load(rapidxml::xml_node<> *node){

	rapidxml::xml_node<> *frame_node = getNode(node, "frame", NULL, NULL);
	while (frame_node){

		float timing = -10000.0f;

		char* timing_value = getAttribStr(frame_node, "timing");

		if (timing_value){

			timing = (float)atof(timing_value);
		}

		rapidxml::xml_node<> *image_node = getNode(frame_node, "texture", NULL, NULL);

		while (image_node){

			float tex_alpha = 1.0f;
			char* alpha_value = getAttribStr(image_node, "alpha");
			if (alpha_value){

				tex_alpha = (float)atof(alpha_value);
			}

			char* filename = getAttribStr(image_node, "src");

			std::string str_for_file("");

			if (filename){
				str_for_file = std::string(filename);
			}

			ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
			TextureBin* image = manager.get(str_for_file);

			if (NULL == image){

				float w(0), h(0);
				image = new TextureBin();
				image->setTexID(Img::loadImage(str_for_file.c_str(), &w, &h));
				image->setWidth((int)w);
				image->setHeight((int)h);

				manager.insert(str_for_file, image);
			}

			add(AniFrame(timing, tex_alpha, image));
			image_node = getNextNode(image_node, "texture", NULL, NULL);
		}

		frame_node = getNextNode(frame_node, "frame", NULL, NULL);
	}
	arrangeTiming();
}