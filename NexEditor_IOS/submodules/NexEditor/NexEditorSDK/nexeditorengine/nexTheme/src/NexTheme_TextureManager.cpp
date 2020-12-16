#include "NexTheme_Config.h"
#include <map>
#include <string>
#include "NexTheme_Nodes.h"
//#include "NexThemeRenderer_Internal.h"
#include "nexTexturesLogger.h"

#define LOG_TAG "NexTheme_TextureManager"

using namespace std;

struct KeyInfo{

	string key_;
	KeyInfo* pre_;
	KeyInfo* next_;

	KeyInfo(const char* key):key_(key), pre_(NULL), next_(NULL){

	}

	KeyInfo():pre_(NULL), next_(NULL){

	}

	~KeyInfo(){

		unlink();
	}

	void unlink(){

		if(pre_) pre_->next_ = next_;
		if(next_) next_->pre_ = pre_;
		pre_ = NULL;
		next_ = NULL;
	}

	void link(KeyInfo* node){

		node->unlink();
		node->pre_ = this;
		node->next_ = next_;
		if(next_) next_->pre_ = node;
		next_ = node;
	}
};

struct TexInfo{

	GLuint tex_id_;
	unsigned int width_;
	unsigned int height_;
	int release_counter_;
	KeyInfo* pkeyinfo_;
};


class TextureManager{

	typedef map<string, TexInfo> TexMap_t;
	TexMap_t texmap_;
	KeyInfo head_;
	KeyInfo tail_;
	int timeout_;
	unsigned int total_mem_size_;
	unsigned int max_mem_size_;

	int GetDefaultReleaseCount(){

		return timeout_;
	}

    unsigned int getHighestPowerOfTwo(unsigned int v) {

        // v--;
        // v |= v >> 1;
        // v |= v >> 2;
        // v |= v >> 4;
        // v |= v >> 8;
        // v |= v >> 16;
        // v++;
        return v;
    }

public:
	TextureManager(int timeout, int max_mem_size):timeout_(timeout), max_mem_size_(max_mem_size){

		head_.link(&tail_);
		total_mem_size_ = 0;
	}

	~TextureManager(){

	}

	void clear(){

		//return;

		for(TexMap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor ){

			TexInfo& info = itor->second;
			GL_DeleteTextures(1, &info.tex_id_);
		}
		texmap_.clear();

		for(KeyInfo* node = head_.next_; node != &tail_; ){

			KeyInfo* nextnode = node->next_;
			delete node;
			node = nextnode;
		}
		total_mem_size_ = 0;
		LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) delete all", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f);
	}

	void reset(){

		return;

		for(TexMap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

			TexInfo& info = itor->second;
			info.release_counter_ = 0;
		}
	}

	void remove(const char* path, bool empty_delete){

		TexMap_t::iterator itor = texmap_.find(path);
		if(itor != texmap_.end()){

			TexInfo& info = itor->second;
			bool delete_flag = true;
			if(empty_delete){

				if(info.width_ != 0 || info.height_ != 0){

					delete_flag = false;
					LOGI("NexTextureManager path:%s is not null, no delete", path);
				}
			}
			
			if(delete_flag){

				string str = itor->first;
				GL_DeleteTextures(1, &info.tex_id_);
				total_mem_size_ -= (info.width_ * info.height_);
				if(info.pkeyinfo_) delete info.pkeyinfo_;
				texmap_.erase(itor);
				LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) remove member:%s", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f, str.c_str());				
			}
		}
	}
	
	void set(const char* path, NXT_TextureInfo* ptex_info){

		TexMap_t::iterator itor = texmap_.find(path);
		if(itor != texmap_.end()){

			string str = itor->first;
			TexInfo& info = itor->second;
			GL_DeleteTextures(1, &info.tex_id_);
			total_mem_size_ -= (info.width_ * info.height_);
			if(info.pkeyinfo_) delete info.pkeyinfo_;
			texmap_.erase(itor);
			LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) delete member:%s", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f, str.c_str());
		}

        int width = getHighestPowerOfTwo(ptex_info->srcWidth);
        int height = getHighestPowerOfTwo(ptex_info->srcHeight);

		unsigned int mem_add = width * height;

		while(total_mem_size_ + mem_add > max_mem_size_){

			KeyInfo* node = tail_.pre_;
			itor = texmap_.find(node->key_);
			if(itor != texmap_.end()){

				string str = itor->first;
				TexInfo& info = itor->second;
				GL_DeleteTextures(1, &info.tex_id_);
				total_mem_size_ -= (info.width_ * info.height_);
				if(info.pkeyinfo_) delete info.pkeyinfo_;
				texmap_.erase(itor);
				LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) delete member:%s", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f, str.c_str());
			}
		}

		TexInfo info;
		
		info.tex_id_ = ptex_info->texName[0];
		info.width_ = width;
		info.height_ = height;
		info.release_counter_ = GetDefaultReleaseCount();
		info.pkeyinfo_ = new KeyInfo(path);
		head_.link(info.pkeyinfo_);
		total_mem_size_ += mem_add;
		texmap_.insert(make_pair(path, info));
		LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) new member:%s", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f, path);
	}

	int get(const char* path, NXT_TextureInfo* ptex_info){

		TexMap_t::iterator itor = texmap_.find(path);
		if(itor != texmap_.end()){

			TexInfo& info = itor->second;

			ptex_info->texName[0] = info.tex_id_;
			info.release_counter_ = GetDefaultReleaseCount();
			ptex_info->texName_for_rgb = ptex_info->texName[0];
			ptex_info->srcWidth = info.width_;
			ptex_info->srcHeight = info.height_;
			head_.link(info.pkeyinfo_);

			return 1;
		}

		return 0;
	}

	void update(){

		for(TexMap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ){

			TexInfo& info = itor->second;
			if(--info.release_counter_ <= 0){

				string str = itor->first;
				GL_DeleteTextures(1, &info.tex_id_);
				total_mem_size_ -= (info.width_ * info.height_);
				if(info.pkeyinfo_) delete info.pkeyinfo_;
				texmap_.erase(itor++);
				LOGI("NexTextureManager size:%d cached_memory:%d(%.3f) delete member:%s", texmap_.size(), total_mem_size_, float(total_mem_size_) / float(max_mem_size_) * 100.0f, str.c_str());
			}
			else
				++itor;
		}
	}
};

void* NXT_Theme_CreateTextureManager(int timeout, int max_mem_size){

	return (void*)(new TextureManager(timeout, max_mem_size));
}

void NXT_Theme_ReleaseTextureManager(void* pmanager, int isDetachedContext){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!isDetachedContext)
		ptexmanager->clear();
	delete ptexmanager;
}

void NXT_Theme_SetTextureInfo(void* pmanager, const char* path, NXT_TextureInfo* ptex_info){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!ptexmanager)
		return;
	ptexmanager->set(path, ptex_info);
	ptex_info->texNameInitCount = 0;
}

void NXT_Theme_RemoveTextureInfo(void* pmanager, const char* path){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!ptexmanager)
		return;
	ptexmanager->remove(path, true);
}

int NXT_Theme_GetTextureInfo(void* pmanager, const char* path, NXT_TextureInfo* ptex_info){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!ptexmanager)
		return 0;
	return ptexmanager->get(path, ptex_info);	
}

void NXT_Theme_UpdateTextureManager(void* pmanager){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!ptexmanager)
		return;
	ptexmanager->update();
}

void NXT_Theme_ResetTextureManager_Internal(void* pmanager){

	TextureManager* ptexmanager = (TextureManager*)pmanager;
	if(!ptexmanager)
		return;
	ptexmanager->reset();
}