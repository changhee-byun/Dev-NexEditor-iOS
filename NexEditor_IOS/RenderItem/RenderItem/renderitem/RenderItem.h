#pragma once

#include "RenderTarget.h"
#include "lua_extension_for_renderItem.h"
#include "rapidxml.hpp"
#ifdef DEMONSTRATION
#include "message_processor.h"
#endif//DEMONSTRATION
#include "Animation.h"
#include "AnimatedValue.h"
#include "ResourceManager.h"
#include "types.h"


typedef std::map<std::string, AnimationBin*> AnimationMapper_t;
typedef std::vector<NXT_Node_AnimatedValue*> AnimatedNodeList_t;

typedef int (*LOADTHEMECALLBACKFUNC)(char** ppOutputData, int* pLength, char* path, void* cbdata);

struct FileOperatorForRenderItem{
	void* cbdata_for_loadfunc;
	LOADTHEMECALLBACKFUNC ploadfunc;
};

struct RenderItemContext;

class RenderItem{

	struct ChildItem{

		RenderItem* pitem_;
		int order_;

		bool operator <(const ChildItem& right) const{

			return this->order_ < right.order_;
		}
	};

	struct PrefixedChildItem:ChildItem{

		int start_time_;
		int duration_;

		bool operator <(const ChildItem& right){

			return this->order_ < right.order_;
		}
	};

	typedef std::vector<ChildItem> childlist_t;
	typedef std::vector<PrefixedChildItem> prefixedchildlist_t;

	rapidxml::xml_document<> doc_;
	bool flag_for_ready_;

public:
	RenderItem(const RenderItem& item);

	RenderItem(RenderItem* pitem);

	RenderItem(int child_capacity);

	~RenderItem();

	void setContext(RenderItemContext* ctx);

	RenderItemContext* getContext();

	bool load(const char* name);

	bool load(const char* name, const char* src);

	void load_late();

	bool is_ready();

	void freeResource();

#ifdef DEMONSTRATION
	bool loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor);
#endif//DEMONSTRATION

	int loop(NXT_HThemeRenderer renderer);

	void setTexMatrix(void* pmatrix);

	void setValue(const char* key, float value);

	void setValue(const char* key, int value);

	void setValue(const char* key, const char* value);

	void setValue(const char* key, float* matrix);

	void doEffect(int elapsed_tick, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doEffect(float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doEffect(lua_State* lua_state, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doListenerWork(const char* szkey, const char* val);

	bool broadcastGlobalVariable(const char* szkey, const char* val);

	bool broadcastGlobalVariable(const char* szkey, int val);

	bool broadcastGlobalVariable(const char* szkey, unsigned int val);

	bool broadcastGlobalVariable(const char* szkey, float val);

	lua_State* getLuaState();

	void addChild(RenderItem* pitem, int order = -1);

	void addPrefixedChild(RenderItem* pitem, int order, int start, int duration);

	void setParent(RenderItem* pitem);

	void getReady();

	int getChildCapacity();

	int getOutputType();

	int getEffectType();

	int getInTime();

	int getInTimeFirst();

	int getOutTime();

	int getOutTimeLast();

	int getCycleTime();

	int getRepeatType();

	int getEffectOverlap();

	void setOutputType(int type);

	void setChildCapacity(int capacity);

	const float* getVertex(int id);

	const GLushort* getIndices(int id);

	const float* getTexcoord(int id);

	const char* getName();

	SnapshotOfAniFrame getAniFrame(const char* name, float timing);
private:
	//For Common Data Region
	int					output_type_;
	int					capacity_of_childs_;
	
	glProgrammap_t		program_map_;
	glProgramIndexmap_t program_idx_map_;

	glFloatDataList_t	vertex_data_list_;
	glFloatDataList_t	texcoord_data_list_;
	glUShortDataList_t	polygon_index_data_list_;

	glDataIdxMapper_t	vertex_data_map_;
	glDataIdxMapper_t	texcoord_data_map_;
	glDataIdxMapper_t	polygon_index_data_map_;

	AnimatedNodeList_t	animated_node_list_;
	AnimationMapper_t	animation_mapper_;
	std::string			namespace_;

	NXT_EffectType      effect_type_;
	char*               name_;
	char*               category_title_;
	char*               icon_;
	char*               encoded_name_;
	int                 effect_offset_;
	int                 video_overlap_;
	int                 min_duration_;
	int                 max_duration_;
	int                 default_duration_;
	int                 in_time_;
	int                 out_time_;
	int                 in_time_first_;
	int                 out_time_last_;
	int                 cycle_time_;
	int                 max_title_duration_;
	unsigned int        flag_for_user_duration_;
	NXT_RepeatType      repeat_type_;
	unsigned int        effect_flags_;
	//For Individual Data Region
	RenderItem*			prender_item_body_;
	int					count_of_childs_;
	RenderItem*			parent_;
	childlist_t			childlist_;
	prefixedchildlist_t prefixed_childlist_;
	NXT_RenderTarget*	prender_target_[5];

	glTexmap_t			texmap_;
	lua_State*			lua_state_;
	
	valuemap_t			valuemap_;
	listenermap_t		listenermap_;
	RenderItemContext* 	ctx_;

	void processRenderItemAttr(rapidxml::xml_node<>* pnode);

	void loadProgram(rapidxml::xml_document<>& doc, const char* name, std::string& program_buf, LuaBin* luabin);

	void loadProgram(rapidxml::xml_document<>& doc, std::string& program_buf, LuaBin* luabin);

	bool loadXML(rapidxml::xml_document<>& doc, const char* filename);

	bool loadXML_from_buffer(rapidxml::xml_document<>& doc, const char* src);

	void deinitSystem();

	void loadResource(rapidxml::xml_document<>& doc, LuaBin* luabin);

#ifdef DEMONSTRATION

	void addSlider(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, float min_value, float max_value, float default_value, float step);

	void addCheckBox(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, bool value);

	float addColorPicker(Frame* pframe, lua_State* L, bool alpha_mode, const char* title, const char* script_target, float x, float y, const char* value);

	void loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor, lua_State* L);
#endif//DEMONSTRATION

	void loadLua(rapidxml::xml_document<>& doc, lua_State* L, std::string& program_buf, LuaBin* luabin);

	bool loadLua(lua_State* L, LuaBin* luabin);

	void loadResource(rapidxml::xml_document<>& doc);

	void doChild(NXT_HThemeRenderer renderer);

	void releaseRenderTarget(NXT_HThemeRenderer renderer);

	void makeDefaultFunc(std::string& program_buf, const char* name);

	void makeSetterForShaderProgram(std::string& program_buf, const char* name, const char* uniform_name, int type, int& tex_target_count);

public:
	int mainLoop();
};

class RenderItemBin :public Bin{

	RenderItem* data_;
public:
	RenderItemBin() :data_(NULL){

		setLife(10);
	}

	~RenderItemBin(){

		SAFE_DELETE(data_);
	}

	RenderItem* get(){

		return data_;
	}

	void set(const char* src_val, int child_capacity){

		SAFE_DELETE(data_);
		data_ = new RenderItem(child_capacity);
		if (src_val)
			data_->load(src_val);
	}

	void set(const char* name, const char* buffer, int child_capacity){

		SAFE_DELETE(data_);
		data_ = new RenderItem(child_capacity);
		if (name)
			data_->load(name, buffer);
	}
};

struct RenderItemContext{

	NXT_ThemeRenderer 				theme_renderer_;
	std::string						namespace_;
	glTexInfomap_t 					texinfo_map_;
	ResourceManager<RenderItemBin> 	renderitem_manager_;
	ResourceManager<ShaderBin> 		shader_manager_;
	ResourceManager<ProgramBin> 	program_manager_;
	ResourceManager<AnimationBin> 	animation_manager_;
	ResourceManager<TextureBin>		texture_manager_;
	ResourceManager<LuaBin>			lua_manager_;
	FileOperatorForRenderItem		file_ops_;
};