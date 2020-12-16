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
#include "Vectors.h"

#include "maya/mesh.h"

using namespace RI;

namespace RI{

    struct NXT_PrecacheResourceInfo{

        int type;
        char* name;
        char* uid;
    };
};

typedef std::map<std::string, AnimationBin*> AnimationMapper_t;
typedef std::vector<NXT_Node_AnimatedValue*> AnimatedNodeList_t;

typedef enum NXT_PixelFormat_ {
    NXT_PixelFormat_NONE        = 0,
    NXT_PixelFormat_RGBA8888    = 1,
    NXT_PixelFormat_RGB565      = 2,
    NXT_PixelFormat_Luminance   = 3,
    NXT_PixelFormat_YUV         = 4,
    NXT_PixelFormat_NV12        = 5,
    NXT_PixelFormat_EGLImage    = 6,
    NXT_PixelFormat_NV12_JPEG   = 7
} NXT_PixelFormat;

typedef enum NXT_RenderDest_ {
    NXT_RenderDest_Screen       = 0,
    NXT_RenderDest_Stencil      = 1,
	NXT_RenderDest_Mask      	= 2
} NXT_RenderDest;

typedef enum NXT_RenderTest_ {
    NXT_RenderTest_None         = 0,
    NXT_RenderTest_Stencil      = 1,
    NXT_RenderTest_NegStencil   = 2,
	NXT_RenderTest_Mask			= 3,
	NXT_RenderTest_NegMask		= 4,
	
} NXT_RenderTest;

typedef enum NXT_ScreenDef_{

	NXT_ScreenDef_ALPHA = 0x01,
	NXT_ScreenDef_STENCIL = 0x02,
} NXT_ScreenDef;

struct NXT_ImageInfo;

typedef int (*LOADTHEMECALLBACKFUNC)(char** ppOutputData, int* pLength, char* path, void* cbdata);
typedef int (*LOADTHEMEIMAGECALLBACKFUNC)(NXT_ImageInfo*, char* path, int asyncmode, void* cbdata);
typedef int (*FREETHEMEIMAGECALLBACKFUNC)(NXT_ImageInfo*, void* cbdata);

struct NXT_ImageInfo{
        int                     width;                  // Width of image (in pixels)
        int                     height;                 // Height of image (in pixels)
        int                     pitch;                  // Pitch of image (in pixels)
        NXT_PixelFormat         pixelFormat;            // One of NXT_PixelFormat_????
        int                     *pixels;                // Pointer to pixels
        FREETHEMEIMAGECALLBACKFUNC   freeImageCallback;      // Function to call to free pixel data
        void                    *cbprivate0;            // Private data for use by freeImageCallback
        void                    *cbprivate1;            // Private data for use by freeImageCallback
        void                    *userdata;
    };

struct FileOperatorForRenderItem{
	void* 						cbdata_for_loadfunc;
	LOADTHEMECALLBACKFUNC 		ploadfunc;
	LOADTHEMEIMAGECALLBACKFUNC 	ploadimagefunc;
	FREETHEMEIMAGECALLBACKFUNC 	pfreeimagefunc;
};

struct RenderItemContext;

class RenderItem{

	struct ChildItem{

		RenderItem* pitem_;
		int order_;

		ChildItem():pitem_(NULL), order_(0){
			
		}

		ChildItem(RenderItem* pitem, int order):pitem_(pitem), order_(order){


		}

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
	char* 	xml_doc_buffer_;
	bool 	flag_for_ready_;
	bool	flag_for_prs_;
	Vector3 position_;
	Vector3 rotate_axis_;
	float 	angle_;
	Vector3 scale_;

	bool 	flag_for_matrix_;
	float*	pmatrix_;
	Vector2 lefttop_for_rect_;
	Vector2 rightbottom_for_rect_;
	float 	prs_matrix_alpha_;

	int 	screen_def_;
public:
	bool 	gmask_enabled_;
	int makeVideoSrc();

    int makeVideoSrcForDirect();

	void drawPRS(NXT_RenderTarget* psrc, const Vector3& pos, const Vector3& scale, const Vector3& rotate_axis, float angle, float scr_ratio_w, float scr_ratio_h);
	
	void drawMATRIX(NXT_RenderTarget* psrc, float* pmatrix, float left, float top, float right, float bottom, float alpha);

	RenderItem(const RenderItem& item);

	RenderItem(RenderItem* pitem);

	RenderItem(int child_capacity);

	~RenderItem();

	void setScreenDef(int flag);
	
	int getScreenDef();

	void setContext(RenderItemContext* ctx);

	RenderItemContext* getContext();

	bool load(const char* name, const char* src);

	void load_late();

    void getResourcelist(void* resource_load_list);

	bool is_ready();

	void freeResource();

#ifdef DEMONSTRATION
	bool loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor);
#endif//DEMONSTRATION

	int loop(NXT_HThemeRenderer renderer);

	void setTexMatrix(void* pmatrix, int target);

	void setColorMatrix(void* pmatrix, int target);

	void setRealXY(float real_x, float real_y, int target);

	void setValue(const char* key, float value);

	void setValue(const char* key, int value);

	void setValue(const char* key, const char* value);

	void setValue(const char* key, float* matrix);

	void resetPRS();

	bool getPRSState();

	void setPosition(Vector3& vec);

	void setRotation(Vector3& axis, float angle);

	void setScale(Vector3& scale);

	void setMatrix(float* pmatrix);

	void resetMatrix();

	void setRect(float left, float top, float right, float bottom, float alpha, bool gmask_enabled);

	void doEffect(int elapsed_tick, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doEffect(float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doEffect(lua_State* lua_state, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count);

	void doListenerWork(const char* szkey, const char* val);

	bool broadcastGlobalVariable(const std::string& key, std::string& value);

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

    int getReverse();

	int getRepeatType();

	int getEffectOverlap();

	void setOutputType(int type);

	void setChildCapacity(int capacity);

	MeshGroupBin* getMeshGroupBin(int id);

	const float* getVertex(int id);

	const GLushort* getIndices(int id);

	const float* getTexcoord(int id);

	const char* getName();

	const std::string& getNamespace();

	const char* getUID();

	SnapshotOfAniFrame getAniFrame(const char* name, float timing);

	void resetDefaultValuemap();
	void applyDefaultValues();
	static std::string getUniqueID(const char* src);
private:

	void checkValuemap(const std::string& key, std::string& value);
	//For Common Data Region
	int					output_type_;
	int					capacity_of_childs_;
	
	glProgrammap_t		program_map_;
	glProgramIndexmap_t program_idx_map_;

	glFloatDataList_t	vertex_data_list_;
	glFloatDataList_t	texcoord_data_list_;
	glUShortDataList_t	polygon_index_data_list_;
	MeshGroupBinList_t	meshgroupbin_list_;

	glDataIdxMapper_t	vertex_data_map_;
	glDataIdxMapper_t	texcoord_data_map_;
	glDataIdxMapper_t	polygon_index_data_map_;
	glDataIdxMapper_t	meshgroupbin_data_map_;

	AnimatedNodeList_t	animated_node_list_;
	AnimationMapper_t	animation_mapper_;
	std::string			namespace_;
	std::string			uid_;

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
    int                 reverse_;
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

	defaultvaluemap_t	defaultvaluemap_;	
	valuemap_t			valuemap_;
	listenermap_t		listenermap_;
	RenderItemContext* 	ctx_;

	void processRenderItemAttr(rapidxml::xml_node<>* pnode);

	void loadProgram(rapidxml::xml_document<>& doc, const char* name, std::string& program_buf, LuaBin* luabin);

	void loadProgram(rapidxml::xml_document<>& doc, std::string& program_buf, LuaBin* luabin);

	bool loadXML_from_buffer(rapidxml::xml_document<>& doc, const char* src, char** ppbuffer);

	void deinitSystem();

	void loadResource(rapidxml::xml_document<>& doc, LuaBin* luabin);

#ifdef DEMONSTRATION

	void addSlider(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, float min_value, float max_value, float default_value, float step);

	void addCheckBox(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, bool value);

	float addColorPicker(Frame* pframe, lua_State* L, bool alpha_mode, const char* title, const char* script_target, float x, float y, const char* value);

	void loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor, lua_State* L);
#endif//DEMONSTRATION
	void loadUI(rapidxml::xml_document<>& doc, std::string& program_buf, lua_State* L);

	void loadLua(rapidxml::xml_document<>& doc, lua_State* L, std::string& program_buf, LuaBin* luabin);

	bool loadLua(lua_State* L, LuaBin* luabin);

	void loadResource(rapidxml::xml_document<>& doc);

	void doChild(NXT_HThemeRenderer renderer);

	void releaseRenderTarget(NXT_HThemeRenderer renderer);

	void makeDefaultFunc(std::string& program_buf, const char* name);

	void makeSetterForShaderProgram(std::string& program_buf, const char* name, const char* uniform_name, int type, int& tex_target_count);

public:
	int mainLoop(NXT_HThemeRenderer renderer);
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

	void set(const char* name, const char* buffer, int child_capacity){

		SAFE_DELETE(data_);
		data_ = new RenderItem(child_capacity);
		if (name)
			data_->load(name, buffer);
	}
};

class InputStream{

	int size_;
	int read_;
	unsigned char* pdata_;
	RenderItemContext* pctx_;
public:
	InputStream(RenderItemContext* pctx, unsigned char* pdata, int size):pctx_(pctx), pdata_(pdata), size_(size), read_(0){

	}

	void seek(int rel_pos, int mode){

		if(mode == SEEK_SET){

			read_ = rel_pos;
		}
		else if(mode == SEEK_END){

			read_ = size_ - rel_pos;
		}
		else if(mode == SEEK_CUR){

			read_ += rel_pos;
		}

		if(read_ < 0)
			read_ = 0;
		else if(read_ > size_)
			read_ = size_;
	}

	void* getAddr(){

		return pdata_ + read_;
	}

	size_t read(void* ptarget, size_t size_of_element, size_t count_of_element){

		int left_size = size_ - read_;
		if(left_size >= size_of_element * count_of_element){

			memcpy(ptarget, pdata_ + read_, size_of_element * count_of_element);
			read_ += (size_of_element * count_of_element);
			return count_of_element;
		}

		size_t count_of_read = (left_size - left_size % size_of_element) / size_of_element;
		memcpy(ptarget, pdata_ + read_, size_of_element * count_of_read);
		read_ += (size_of_element * count_of_read);

		return count_of_read;
	}

	RenderItemContext* getContext(){

		return pctx_;
	}
};



class MeshGroupBin:public Bin{

	MeshGroup* data_;
public:
	MeshGroupBin():data_(NULL){

		setLife(1);
	}

	~MeshGroupBin(){

		SAFE_DELETE(data_);
	}

	MeshGroup* get(){

		return data_;
	}

	void set(RenderItemContext* pctx, unsigned char* src_val, int length);

    void set(RenderItemContext* pctx, unsigned char* src_val, int length, char* uid, void* prenderer, void (*pAquireContextFunc)(void *p), void (*pReleaseContextFunc)(void* p));
};

struct RenderItemContext{

	NXT_ThemeRenderer 				theme_renderer_;
	glTexInfomap_t 					texinfo_map_;
	ResourceManager<RenderItemBin> 	renderitem_manager_;
	ResourceManager<ShaderBin> 		shader_manager_;
	ResourceManager<ProgramBin> 	program_manager_;
	ResourceManager<AnimationBin> 	animation_manager_;
	ResourceManager<TextureBin>		texture_manager_;
	ResourceManager<LuaBin>			lua_manager_;
	ResourceManager<MeshGroupBin>	meshgroup_manager_;
	Factory<Material>				material_factory_;
	Factory<Mesh> 					mesh_factory_;
	Factory<MeshGroup>				meshgroup_factory_;
	FileOperatorForRenderItem		file_ops_;

	RITM_NXT_Vector4f   fillColor;
    RITM_NXT_Vector4f   strokeColor;
    RITM_NXT_Vector4f   shadowColor;
    RITM_NXT_Vector4f   bgColor;    
    RITM_NXT_Vector4f   shadowOffset;
    
    RenderItemContext();

    ~RenderItemContext();

    void init_late();

    int getVideoSrc(int src_id);
    const float* getTexMatrixForSrc(int src_id);
	const float* getColorMatrixForSrc(int src_id);
	float getRealXForSrc(int src_id);
	float getRealYForSrc(int src_id);

	void setVideoSrc(int src_id, int tex_id);
    void setTexMatrixForSrc(int src_id, float* pmatrix);
	void setColorMatrixForSrc(int src_id, float* pmatrix);
	void setRealXYForSrc(int src_id, float x, float y);

	void clearSrcInfo();

	void clearSrcInfo(int target);

	void saveSrcInfo(int target);

	void restoreSrcInfo(int target);

	void addDeleteTexture(unsigned int tex_id);

	void deleteTextures();
private:

	void initMeshFactory();

	bool ready_;

	struct VideoInfo{

    	int tex_id;
    	int filtered;
    	int check_for_texmatrix;
    	int check_for_colormatrix;
    	float matrix_for_tex[16];
    	float matrix_for_colorconv[16];
    	float real_x;
    	float real_y;
    };

    VideoInfo info_[10];
    VideoInfo backup_info_[10];

    std::vector<unsigned int> deleted_tex_list_;
};
