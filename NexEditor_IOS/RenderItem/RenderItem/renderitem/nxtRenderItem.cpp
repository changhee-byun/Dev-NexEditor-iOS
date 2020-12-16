#include "nxtRenderItem.h"
#include "RenderItem.h"
#include <vector>
//#include "NexSAL_Internal.h"
#include "lua.hpp"
#include "math.h"

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

	~RenderItemManager(){};

	RenderItemContext& getContext(){

		return ctx_;
	}

	void update(){

		if(getTickCount() - last_gc_time_ < gc_cycle_){

			return;
		}

		ctx_.renderitem_manager_.gc();
		ctx_.shader_manager_.gc();
		ctx_.program_manager_.gc();
		ctx_.animation_manager_.gc();
		ctx_.texture_manager_.gc();
		ctx_.lua_manager_.gc();
		// nexSAL_TraceCat(0, 0, "[%s %d] item:%d", __func__, __LINE__, renderitem_list_.size());

		// for(RenderItemList_t::iterator itor = renderitem_list_.begin(); itor != renderitem_list_.end(); ++itor){

		// 	RenderItem* pitem = *itor;
		// 	pitem->load_late();
		// }
	}

	void delete_all(){

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
//		nexSAL_TraceCat(0, 0, "[%s %d] renderitem_list_.size():%d pitem->namespace_:%s", __func__, __LINE__, renderitem_list_.size(), pitem->getName());
	}

	int count(){

		return renderitem_list_.size();
	}

	void beginRenderItem(int effect_id){

		if(effect_id < 0)
			return;

		if(renderitem_list_.size() <= effect_id)
			return;

		if(pctx_render_item_ != NULL){

			render_item_stack_.push_back(pctx_render_item_);
		}

		pctx_render_item_ = renderitem_list_[effect_id];
		ctx_.theme_renderer_.current_program_id_ = 0xFFFFFFFF;
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

	void setTexInfo(int texid, int width, int height, int src_width, int src_height, int target){

		if(pctx_render_item_ == NULL)
			return;

		if(target >= E_TARGET_OVER || target < 0)
			return;

		const char* target_str[] = {

			"system.video_src",
			"system.video_left",
			"system.video_right",
		};

		initLate(pctx_render_item_);

		setTexInfo(texid, width, height, src_width, src_height);
		pctx_render_item_->broadcastGlobalVariable(target_str[target], texid);
	}

	void setTexMatrix(void* pmatrix){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);

		pctx_render_item_->setTexMatrix(pmatrix);
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

	void apply(float progress){

		if(pctx_render_item_ == NULL)
			return;

//		nexSAL_TraceCat(0, 0, "[%s %d] progress:%f", __func__, __LINE__, progress);
		pctx_render_item_->broadcastGlobalVariable("system.play_percentage", progress);
		pctx_render_item_->mainLoop();
	}

	void doEffect(int elapsed_time, int cur_time, int clipStartTime, int clipEndTime, int max_time, int actualEffectStartCTS, int actualEffectEndCTS, int clipindex, int total_clip_count){

		if(pctx_render_item_ == NULL)
			return;

		initLate(pctx_render_item_);
	    
	    int inTime = pctx_render_item_->getInTime();
	    int outTime = pctx_render_item_->getOutTime();
	    int cycleTime = pctx_render_item_->getCycleTime();
	    int repeatType = pctx_render_item_->getRepeatType();
	    
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

	    pctx_render_item_->doEffect(elapsed_time, time, (float)cur_time / (float)max_time, part, clipindex, total_clip_count);
	}

	void setTexTarget(int texid, int target){

		if(pctx_render_item_ == NULL)
			return;

		if(target >= E_TARGET_OVER || target < 0)
			return;

		const char* target_str[] = {

			"system.video_src",
			"system.video_left",
			"system.video_right",
		};

		initLate(pctx_render_item_);

//		nexSAL_TraceCat(0, 0, "[%s %d] texid:%d target:%s", __func__, __LINE__, texid, target_str[target]);

		pctx_render_item_->broadcastGlobalVariable(target_str[target], texid);
	}

	void setTexInfo(int texid, int width, int height, int src_width, int src_height){

		texInfo info;

		info.width_ = width;
		info.height_ = height;
		info.src_width_ = src_width;
		info.src_height_ = src_height;

//		nexSAL_TraceCat(0, 0, "[%s %d] texid:%d width:%d height:%d src_width:%d src_height:%d", __func__, __LINE__, texid, width, height, src_width, src_height);

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
		ctx_.theme_renderer_.current_program_id_ = 0xFFFFFFFF;
		
		
		if(!pitem->is_ready())
			pitem->load_late();

		pitem->broadcastGlobalVariable("system.video_left", texid_left);
		pitem->broadcastGlobalVariable("system.video_right", texid_right);
		pitem->broadcastGlobalVariable("system.play_percentage", progress);
		pitem->mainLoop();
	}

	void applyEffect(int texid, int effect_id, float progress){

		if(effect_id < 0)
			return;
		if(renderitem_list_.size() <= effect_id)
			return;

		RenderItem* pitem = renderitem_list_[effect_id];
		ctx_.theme_renderer_.current_program_id_ = 0xFFFFFFFF;
		
		
		if(!pitem->is_ready())
			pitem->load_late();

		pitem->broadcastGlobalVariable("system.video_src", texid);
		pitem->broadcastGlobalVariable("system.play_percentage", progress);
		pitem->mainLoop();
	}

	int getEffectID(const char* name){

		int effect_id = 0;

//		nexSAL_TraceCat(0, 0, "[%s %d] name:%s renderitem_list_.size:%d", __func__, __LINE__, name, renderitem_list_.size());

		for(RenderItemList_t::iterator itor = renderitem_list_.begin(); itor != renderitem_list_.end(); ++itor){

			RenderItem* pitem = *itor;
//			nexSAL_TraceCat(0, 0, "[%s %d] pitem->getName:%s", __func__, __LINE__, pitem->getName());
			if(NULL != pitem->getName())
				if(0 == strcasecmp(pitem->getName(), name))
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
};

void* NXT_Theme_GetRenderItem(void* pmanager, const char* pid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* cbdata){

//	nexSAL_TraceCat(0, 0, "[%s %d] name:%s", __func__, __LINE__, pid);
	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->getContext().file_ops_.cbdata_for_loadfunc = cbdata;
	pitemmanager->getContext().file_ops_.ploadfunc = pfunc;

	ResourceManager<RenderItemBin>& manager = pitemmanager->getContext().renderitem_manager_;
	RenderItemBin* prender_item_bin = manager.get(pid);

	if (prender_item_bin == NULL){

		prender_item_bin = new RenderItemBin();
		prender_item_bin->set(pid, src, child_count);
		manager.insert(pid, prender_item_bin);
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

void NXT_Theme_AddRenderItemToManager(void* pmanager, void* pitem){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	pitemmanager->add((RenderItem*)pitem);
}

void NXT_Theme_UpdateRenderItemManager(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;
	pitemmanager->update();
}

int NXT_Theme_CountOfRenderItem(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->count();	
}

void NXT_Theme_BeginRenderItem(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->beginRenderItem(effect_id);
}

void NXT_Theme_EndRenderItem(void* pmanager){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->endRenderItem();
}

void NXT_Theme_ApplyRenderItem(void* pmanager, float progress){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->apply(progress);
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

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->delete_all();
}

int NXT_Theme_GetEffectType(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->getEffectType(effect_id);
}

int NXT_Theme_GetEffectOverlap(void* pmanager, int effect_id){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	return pitemmanager->getEffectOverlap(effect_id);
}

void NXT_Theme_SetTexMatrix(void* pmanager, void* pmatrix){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setTexMatrix(pmatrix);
}

void NXT_Theme_SetValueMatrix(void* pmanager, const char* key, float* value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_SetValue(void* pmanager, const char* key, const char* value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_SetValueInt(void* pmanager, const char* key, int value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
}

void NXT_Theme_SetValueFloat(void* pmanager, const char* key, float value){

	RenderItemManager* pitemmanager = (RenderItemManager*)pmanager;

	pitemmanager->setValue(key, value);
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