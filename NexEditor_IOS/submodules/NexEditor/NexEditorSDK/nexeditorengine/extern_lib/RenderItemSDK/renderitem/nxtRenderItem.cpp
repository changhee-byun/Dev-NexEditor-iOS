#include "nxtRenderItem.h"
#include "RenderItem.h"
#include <vector>
#include "NexSAL_Internal.h"
#include "lua.hpp"

class RenderItemManager{

	typedef std::vector<RenderItem*> RenderItemList_t;

	RenderItemList_t render_item_stack_;
	RenderItem* pctx_render_item_;


	RenderItemList_t renderitem_list_;
	RenderItemContext ctx_;

	void initLate(RenderItem* pitem){

		if(!pitem->is_ready())
			pitem->load_late();

	}

    void getResourcelist(RenderItem* pitem, void* resource_load_list){

		if(!pitem->is_ready())
			pitem->getResourcelist(resource_load_list);
	}

	void freeResource(RenderItem* pitem){

		pitem->freeResource();
	}

	time_t last_gc_time_;

	int gc_cycle_;

public:
	RenderItemManager():pctx_render_item_(NULL){
		gc_cycle_ = 60 * 1000;
		last_gc_time_ = getTickCount();
	};

	~RenderItemManager(){

		delete_all();
	};

	RenderItemContext& getContext(){

		return ctx_;
	}

	void initContext(){

		// nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

		ctx_.init_late();
	}

    void setTextureMaxSize(int max_size) {

        ctx_.theme_renderer_.max_texture_size = max_size;
    }

	void update(unsigned int mask_tex_id, unsigned int blend_tex_id, unsigned int white_tex_id){

		ctx_.deleteTextures();
		ctx_.theme_renderer_.updateDefaultTexID(mask_tex_id, blend_tex_id, white_tex_id);
		
		if(getTickCount() - last_gc_time_ < gc_cycle_){

			return;
		}

		ctx_.renderitem_manager_.gc();
		ctx_.shader_manager_.gc();
		ctx_.program_manager_.gc();
		ctx_.animation_manager_.gc();
		ctx_.texture_manager_.gc();
		ctx_.lua_manager_.gc();
        ctx_.meshgroup_manager_.gc();

		ctx_.theme_renderer_.updateRenderTargetManager();
		last_gc_time_ = getTickCount();
		// nexSAL_TraceCat(0, 0, "[%s %d] item:%d", __func__, __LINE__, renderitem_list_.size());

		// for(RenderItemList_t::iterator itor = renderitem_list_.begin(); itor != renderitem_list_.end(); ++itor){

		// 	RenderItem* pitem = *itor;
		// 	pitem->load_late();
		// }
	}

	void delete_all(){

		ctx_.deleteTextures();

		for(RenderItemList_t::iterator itor = renderitem_list_.begin(); itor != renderitem_list_.end(); ++itor){

			RenderItem* pitem = *itor;
			delete pitem;
		}

		renderitem_list_.clear();
	}

	void add(RenderItem* pitem){

		pitem->setContext(&ctx_);
		lua_setRenderItemContext(pitem->getLuaState(), &ctx_);
		renderitem_list_.push_back(pitem);
		// nexSAL_TraceCat(0, 0, "[%s %d] renderitem_list_.size():%d pitem->namespace_:%s", __func__, __LINE__, renderitem_list_.size(), pitem->getName());
	}

	int count(){

		return renderitem_list_.size();
	}

	    void beginRenderItem(int effect_id, int width, int height, void* pdefault_render_target){

		// nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);

		if(effect_id < 0)
			return;

		if(renderitem_list_.size() <= effect_id)
			return;

		if(pctx_render_item_ != NULL){

			render_item_stack_.push_back(pctx_render_item_);
		}

		pctx_render_item_ = renderitem_list_[effect_id];
		ctx_.theme_renderer_.resetCurrentProgramID();
		ctx_.theme_renderer_.view_width = width;
		ctx_.theme_renderer_.view_height = height;
		ctx_.theme_renderer_.setRenderTargetAsDefault((NXT_RenderTarget*)pdefault_render_target, false);
	}

	void endRenderItem(){

		if(render_item_stack_.size() <= 0){

			pctx_render_item_ = NULL;
			return;
		}
		pctx_render_item_ = render_item_stack_.back();
		render_item_stack_.pop_back();
	}

	void freeForce(int effect_id){

		if(effect_id < 0)
			return;

		if(renderitem_list_.size() <= effect_id)
			return;

		freeResource(renderitem_list_[effect_id]);
	}

	void initForce(int effect_id){

		if(effect_id < 0)
			return;

		if(renderitem_list_.size() <= effect_id)
			return;

		initLate(renderitem_list_[effect_id]);
	}

    void getResourcelist(int effect_id, void* resource_load_list){

        if(effect_id < 0 )
            return;
        
        if(renderitem_list_.size() <= effect_id)
            return;
        
        getResourcelist(renderitem_list_[effect_id], resource_load_list);
    }

    void precache(NXT_PrecacheResourceInfo* pinfo, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void *p)){

        std::string str_for_file(pinfo->name);
		ResourceManager<MeshGroupBin>& manager = ctx_.meshgroup_manager_;
		(*pAquireContextFunc)(prenderer);
		MeshGroupBin* meshgroupbin = manager.get(str_for_file);

		if (NULL == meshgroupbin){

			meshgroupbin = new MeshGroupBin();
			manager.insert(str_for_file, meshgroupbin);
			(*pReleaseContextFunc)(prenderer);

			char* poutput = NULL;
			int length = 0;
			if(0 == (*ctx_.file_ops_.ploadfunc)(&poutput, &length, (char*)(std::string(pinfo->uid) + std::string("/") + str_for_file).c_str(), ctx_.file_ops_.cbdata_for_loadfunc)){

				meshgroupbin->set(&ctx_, (unsigned char*)poutput, length, pinfo->uid, prenderer, pAquireContextFunc, pReleaseContextFunc);
				delete[] poutput;
			}
		}
		else{
			(*pReleaseContextFunc)(prenderer);
		}
		
    }

	void setTexInfo(int texid, int width, int height, int src_width, int src_height, int target){

		if(pctx_render_item_ == NULL)
			return;

		if(target >= E_TARGET_OVER || target < 0)
			return;

		const char* target_str[] = {

			"system.video_src.id",
			"system.video_left.id",
			"system.video_right.id",
		};

		initLate(pctx_render_item_);

		setTexInfo(texid, width, height, src_width, src_height);
		ctx_.setVideoSrc(target, texid);
		pctx_render_item_->broadcastGlobalVariable(target_str[target], texid);
	}

	void clearSrcInfo(){

		ctx_.clearSrcInfo();
	}

	void clearSrcInfo(int target){

		ctx_.clearSrcInfo(target);
	}

	void saveSrcInfo(int target){

		ctx_.saveSrcInfo(target);
	}

	void restoreSrcInfo(int target){

		ctx_.restoreSrcInfo(target);
	}

	void setTexMatrix(void* pmatrix, int target){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);
		ctx_.setTexMatrixForSrc(target, (float*)pmatrix);
		pctx_render_item_->setTexMatrix((void*)ctx_.getTexMatrixForSrc(target), target);
	}

	void setColorconvMatrix(void* pmatrix, int target){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);
		ctx_.setColorMatrixForSrc(target, (float*)pmatrix);
		pctx_render_item_->setColorMatrix((void*)ctx_.getColorMatrixForSrc(target), target);
	}

	void setRealXY(float x, float y, int target){

		ctx_.setRealXYForSrc(target, x, y);

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);
		pctx_render_item_->setRealXY(x, y, target);
	}

	void applyDefaultValues(){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->applyDefaultValues();
	}

	void resetDefaultValuemap(){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->resetDefaultValuemap();
	}

	void setValue(const char* key, float* value){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->setValue(key, value);
	}

	void setValue(const char* key, int value){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->setValue(key, value);
	}

	void setValue(const char* key, float value){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->setValue(key, value);
	}

	void setValue(const char* key, const char* value){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->setValue(key, value);
	}

	void setPosition(float x, float y, float z){

		if(pctx_render_item_ == NULL)
			return;

		Vector3 param(x, y, z);
		pctx_render_item_->setPosition(param);
	}

	void setRotation(float x, float y, float z, float angle){

		if(pctx_render_item_ == NULL)
			return;

		Vector3 param(x, y, z);
		pctx_render_item_->setRotation(param, angle);
	}

	void setScale(float x, float y, float z){

		if(pctx_render_item_ == NULL)
			return;

		Vector3 param(x, y, z);
		pctx_render_item_->setScale(param);
	}

	void resetPRS(){

		if(pctx_render_item_ == NULL)
			return;

		pctx_render_item_->resetPRS();
	}

	void setMatrix(float* pmatrix){

		if(pctx_render_item_ == NULL)
			return;

		pctx_render_item_->setMatrix(pmatrix);
	}

	void resetMatrix(){

		if(pctx_render_item_ == NULL)
			return;

		pctx_render_item_->resetMatrix();
	}

	void setRect(float left, float top, float right, float bottom, float alpha, bool gmask_enabled){

		if(pctx_render_item_ == NULL)
			return;

		pctx_render_item_->setRect(left, top, right, bottom, alpha, gmask_enabled);
	}

	void apply(float progress){

		if(pctx_render_item_ == NULL)
			return;

        int reverse = pctx_render_item_->getReverse();

        if(reverse)
            progress = 1.0f - progress;

		initLate(pctx_render_item_);

		// nexSAL_TraceCat(0, 0, "[%s %d] progress:%f", __func__, __LINE__, progress);
		glCullFace(GL_BACK);
		pctx_render_item_->broadcastGlobalVariable("system.play_percentage", progress);
		pctx_render_item_->loop(&ctx_.theme_renderer_);
	}

	void doEffect(int elapsed_time, int cur_time, int clipStartTime, int clipEndTime, int max_time, int actualEffectStartCTS, int actualEffectEndCTS, int clipindex, int total_clip_count){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);
	    
	    int inTime = pctx_render_item_->getInTime();
	    int outTime = pctx_render_item_->getOutTime();
	    int cycleTime = pctx_render_item_->getCycleTime();
	    int repeatType = pctx_render_item_->getRepeatType();
        int reverse = pctx_render_item_->getReverse();
	    
	    if( actualEffectStartCTS < clipStartTime + 33 ) {
	        
	        inTime = pctx_render_item_->getInTimeFirst();
	    }
	    if( actualEffectEndCTS > clipEndTime - 33 ) {

	        outTime = pctx_render_item_->getOutTimeLast();
	    }

	    if(inTime + outTime + cycleTime > max_time) {

	        int divisor = 1;
	        int partMaxTime;
	        if(inTime > 0)
	            divisor++;
	        if(outTime > 0)
	            divisor++;
	        partMaxTime = max_time / divisor;

	        if( inTime > partMaxTime  )
	            inTime = partMaxTime;
	        if( outTime > partMaxTime )
	            outTime = partMaxTime;
	        partMaxTime = max_time - (inTime + outTime);
	        if( cycleTime > partMaxTime )
	            cycleTime = partMaxTime;
	    }

	    int part = 0;
	    float time = 0.0f;
	    
	    if( cur_time < inTime ) {
	        part = NXT_PartType_In;
	        time = (float)cur_time / (float)inTime;
	    } else if (cur_time > max_time - outTime ) {
	        part = NXT_PartType_Out;
	        time = (float)(cur_time-(max_time - outTime)) / (float)outTime;
	    } else if (repeatType == NXT_RepeatType_None || cycleTime==0 ) {
	        part = NXT_PartType_Mid;
	        time = (float)(cur_time-inTime) / (float)(max_time - (inTime+outTime));
	    } else {
	        part = NXT_PartType_Mid;
	        int midtime = (max_time - (inTime+outTime));
	        int numcycles = midtime / cycleTime;
	        if( numcycles<1 )
	            numcycles = 1;
	        int actualCycleTime = midtime / numcycles;
	        int curCycleTime = (cur_time-inTime)%actualCycleTime;
	        time = (float)curCycleTime / (float)actualCycleTime;
	        switch( repeatType ) {
	            case NXT_RepeatType_Sawtooth:
	                if( time <= 0.5f ) {
	                    time *= 2.0f;
	                } else {
	                    time = (1.0f - time)*2.0f;
	                }
	                break;
	            case NXT_RepeatType_Sine:
	                time = 1.0f - cosf(time * 3.14159265f * 2.0f);
	                break;
	            case NXT_RepeatType_Repeat:
	            case NXT_RepeatType_None:
	                break;
	        }
	    }

        if(reverse){

            time = 1.0f - time;
        }

        pctx_render_item_->broadcastGlobalVariable("system.play_part", part);
        pctx_render_item_->broadcastGlobalVariable("system.play_part_percentage", time);

	    pctx_render_item_->doEffect(elapsed_time, time, (float)cur_time / (float)max_time, part, clipindex, total_clip_count);
	}

	void setTexTarget(int texid, int target){

		if(pctx_render_item_ == NULL)
			return;

		if(target >= E_TARGET_OVER || target < 0)
			return;

		const char* target_str[] = {

			"system.video_src.id",
			"system.video_left.id",
			"system.video_right.id",
		};

		initLate(pctx_render_item_);

		// nexSAL_TraceCat(0, 0, "[%s %d] texid:%d target:%s", __func__, __LINE__, texid, target_str[target]);

		pctx_render_item_->broadcastGlobalVariable(target_str[target], texid);
	}

	void setTexInfo(int texid, int width, int height, int src_width, int src_height){

		texInfo info;

		info.width_ = width;
		info.height_ = height;
		info.src_width_ = src_width;
		info.src_height_ = src_height;

		// nexSAL_TraceCat(0, 0, "[%s %d] texid:%d width:%d height:%d src_width:%d src_height:%d", __func__, __LINE__, texid, width, height, src_width, src_height);

		glTexInfomap_t::iterator itor = ctx_.texinfo_map_.find(texid);
		if(itor == ctx_.texinfo_map_.end())
			ctx_.texinfo_map_.insert(std::pair<GLuint, texInfo>(texid, info));
		else
			itor->second = info;
	}

	void applyTransition(int texid_left, int texid_right, int effect_id, float progress){

		if(effect_id < 0)
			return;
		if(renderitem_list_.size() <= effect_id)
			return;

		RenderItem* pitem = renderitem_list_[effect_id];
		ctx_.theme_renderer_.resetCurrentProgramID();
		
		
		if(!pitem->is_ready())
			pitem->load_late();

		pitem->broadcastGlobalVariable("system.video_left.id", texid_left);
		pitem->broadcastGlobalVariable("system.video_right.id", texid_right);
		pitem->broadcastGlobalVariable("system.play_percentage", progress);
		pitem->loop(&ctx_.theme_renderer_);
	}

	void applyEffect(int texid, int effect_id, float progress){

		if(effect_id < 0)
			return;
		if(renderitem_list_.size() <= effect_id)
			return;

		RenderItem* pitem = renderitem_list_[effect_id];
		ctx_.theme_renderer_.resetCurrentProgramID();
		
		
		if(!pitem->is_ready())
			pitem->load_late();

		pitem->broadcastGlobalVariable("system.video_src.id", texid);
		pitem->broadcastGlobalVariable("system.play_percentage", progress);
		pitem->loop(&ctx_.theme_renderer_);
	}

	int getEffectID(const char* name){

		int effect_id = 0;

		// nexSAL_TraceCat(0, 0, "[%s %d] name:%s renderitem_list_.size:%d", __func__, __LINE__, name, renderitem_list_.size());

		for(RenderItemList_t::iterator itor = renderitem_list_.begin(); itor != renderitem_list_.end(); ++itor){

			RenderItem* pitem = *itor;
			// nexSAL_TraceCat(0, 0, "[%s %d] pitem->getName:%s", __func__, __LINE__, pitem->getName());
			if(NULL != pitem->getUID())
				if(0 == strcasecmp(pitem->getUID(), name))
					return effect_id;
			++effect_id;
		}

		return -1;
	}

	int getEffectType(int effect_id){

		if(effect_id < 0)
			return 0;
		if(renderitem_list_.size() <= effect_id)
			return 0;

		RenderItem* pitem = renderitem_list_[effect_id];

		if(!pitem->is_ready())
			pitem->load_late();

		return pitem->getEffectType();
	}

	int getEffectOverlap(int effect_id){

		if(effect_id < 0)
			return 0;
		if(renderitem_list_.size() <= effect_id)
			return 0;

		RenderItem* pitem = renderitem_list_[effect_id];

		if(!pitem->is_ready())
			pitem->load_late();

		return pitem->getEffectOverlap();
	}

	void setDefaultFramebufferNum(unsigned int framebuffer_num){

		ctx_.theme_renderer_.setDefaultFramebufferNum(framebuffer_num);
	}
};

void* NXT_Theme_GetRenderItem(void* pmanager, const char* pid, const char* uid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* pfunc_for_loadimagecb, void* pfunc_for_freeimagecb, void* cbdata){

	// nexSAL_TraceCat(0, 0, "[%s %d] name:%s", __func__, __LINE__, pid);
	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->getContext().file_ops_.cbdata_for_loadfunc = cbdata;
	pitemmanager->getContext().file_ops_.ploadfunc 		= pfunc;
	pitemmanager->getContext().file_ops_.ploadimagefunc = (LOADTHEMEIMAGECALLBACKFUNC) pfunc_for_loadimagecb;
	pitemmanager->getContext().file_ops_.pfreeimagefunc = (FREETHEMEIMAGECALLBACKFUNC) pfunc_for_freeimagecb;

	std::string effect_uid;

	if(NULL == uid){

		effect_uid = RenderItem::getUniqueID(src);
	}
	else
		effect_uid = std::string(uid);

	ResourceManager<RenderItemBin>& manager = pitemmanager->getContext().renderitem_manager_;
	RenderItemBin* prender_item_bin = manager.get(effect_uid.c_str());

	if (prender_item_bin == NULL){

		prender_item_bin = new RenderItemBin();
		prender_item_bin->set(pid, src, child_count);
		manager.insert(effect_uid.c_str(), prender_item_bin);
	}

	prender_item_bin->addRef();
	RenderItem* prender_item = prender_item_bin->get();
	prender_item->setContext(&pitemmanager->getContext());
	RenderItem* pitem = new RenderItem(prender_item);
	pitem->setChildCapacity(child_count);

	pitemmanager->add((RenderItem*)pitem);

	return pitem;
}

void* NXT_Theme_CreateRenderItemManager(){

	return new RenderItemManager();
}

void NXT_Theme_DestroyRenderItemManager(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	SAFE_DELETE(pitemmanager);
}

void NXT_Theme_InitContextOnManager(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	pitemmanager->initContext();
}

void NXT_Theme_AddRenderItemToManager(void* pmanager, void* pitem){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	pitemmanager->add((RenderItem*)pitem);
}

void NXT_Theme_UpdateRenderItemManager(void* pmanager, unsigned int current_mask_id, unsigned int current_blend_id, unsigned int white_tex_id){
	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	pitemmanager->update(current_mask_id, current_blend_id, white_tex_id);
}

int NXT_Theme_CountOfRenderItem(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->count();	
}

void NXT_Theme_BeginRenderItem(void* pmanager, int effect_id, int width, int height, void* pdefault_render_target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->beginRenderItem(effect_id, width, height, pdefault_render_target);
}

void NXT_Theme_EndRenderItem(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->endRenderItem();
}

void NXT_Theme_ResetPRS(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->resetPRS();
}

void NXT_Theme_SetPosition(void* pmanager, float x, float y, float z){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setPosition(x, y, z);
}

void NXT_Theme_SetRotation(void* pmanager, float x, float y, float z, float angle){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setRotation(x, y, z, angle);
}

void NXT_Theme_SetScale(void* pmanager, float x, float y, float z){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setScale(x, y, z);
}

void NXT_Theme_SetMatrix(void* pmanager, float* pmatrix){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setMatrix(pmatrix);
}

void NXT_Theme_SetRect(void* pmanager, float left, float top, float right, float bottom, float alpha_val, bool gmask_enabled){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setRect(left, top, right, bottom, alpha_val, gmask_enabled);
}

void NXT_Theme_ResetMatrix(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->resetMatrix();
}

int NXT_Theme_ApplyRenderItem(void* pmanager, float progress){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->apply(progress);

	return 1;
}

void NXT_Theme_ClearSrcInfo(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->clearSrcInfo();
}

void NXT_Theme_ClearSrcInfoTarget(void* pmanager, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->clearSrcInfo(target);
}

void NXT_Theme_SaveSrcInfo(void* pmanager, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->saveSrcInfo(target);
}

void NXT_Theme_RestoreSrcInfo(void* pmanager, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->restoreSrcInfo(target);
}

void NXT_Theme_SetTextureInfoTargetRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTexInfo(texid, width, height, src_width, src_height, target);
}

void NXT_Theme_SetTexTargetRenderItem(void* pmanager, int texid, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTexTarget(texid, target);
}

void NXT_Theme_SetTextureInfoRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTexInfo(texid, width, height, src_width, src_height);
}

void NXT_Theme_ApplyTransitionRenderItem(void* pmanager, int texid_left, int texid_right, int effect_id, float progress){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->applyTransition(texid_left, texid_right, effect_id, progress);
}

void NXT_Theme_ApplyEffectRenderItem(void* pmanager, int texid, int effect_id, float progress){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->applyEffect(texid, effect_id, progress);
}

int NXT_Theme_GetEffectID(void* pmanager, const char* id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->getEffectID(id);
}

void NXT_Theme_ClearRenderItems(void* pmanager){

	// RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	// pitemmanager->delete_all();
}

int NXT_Theme_GetEffectType(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->getEffectType(effect_id);
}

int NXT_Theme_GetEffectOverlap(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->getEffectOverlap(effect_id);
}

void NXT_Theme_SetTexMatrix(void* pmanager, void* pmatrix, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTexMatrix(pmatrix, target);
}

void NXT_Theme_SetColorconvMatrix(void* pmanager, void* pmatrix, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setColorconvMatrix(pmatrix, target);
}

void NXT_Theme_SetRealXY(void* pmanager, float x, float y, int target){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setRealXY(x, y, target);
}

void NXT_Theme_SetValueMatrix(void* pmanager, const char* key, float* value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_SetValue(void* pmanager, const char* key, const char* value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_ResetDefaultValuemap(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->resetDefaultValuemap();
}

void NXT_Theme_ApplyDefaultValues(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->applyDefaultValues();
}

void NXT_Theme_SetValueInt(void* pmanager, const char* key, int value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_SetValueFloat(void* pmanager, const char* key, float value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_GetResourcelist(void* pmanager, int effect_id, void* resource_load_list){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	
	pitemmanager->getResourcelist(effect_id, resource_load_list);
}

void NXT_Theme_PrecacheRI(void* pmanager, void* pinfo, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p)){

    RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->precache((NXT_PrecacheResourceInfo*)pinfo, prenderer, pAquireContextFunc, pReleaseContextFunc);
}

void NXT_Theme_ForceBind(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->initForce(effect_id);
}

void NXT_Theme_ForceUnbind(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->freeForce(effect_id);
}


void NXT_Theme_DoEffect(void* pmanager, int elapsed_time, int cur_time, int clipStartTime, int clipEndTime, int max_time, int actualEffectStartCTS, int actualEffectEndCTS, int clipindex, int total_clip_count){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->doEffect(elapsed_time, cur_time, clipStartTime, clipEndTime, max_time, actualEffectStartCTS, actualEffectEndCTS, clipindex, total_clip_count);
}

void NXT_Theme_SetDefaultFrameBufferNum(void* pmanager, unsigned int framebuffer_num){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setDefaultFramebufferNum(framebuffer_num);
}

void NXT_Theme_SetTextureMaxSize(void* pmanager, int max_size){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTextureMaxSize(max_size);
}