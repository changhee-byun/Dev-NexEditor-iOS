#include "types.h"
#include "RenderItem.h"
#include "util.h"
#ifdef DEMONSTRATION
#include "slider.h"
#include "checkbox.h"
#include "colorpicker.h"
#include "image_dropbox.h"
#endif //DEMONSTRATION
#include "simplexnoise1234.h"
#include <algorithm>
#include <sstream>
#include <math.h>
#include <ctype.h>
//#include "NexSAL_Internal.h"


RenderItem::RenderItem(int child_capacity) :lua_state_(NULL), capacity_of_childs_(child_capacity), prender_item_body_(NULL), flag_for_ready_(false), ctx_(NULL){

	output_type_ = 0;
	default_duration_ = 1000;
	min_duration_ = 500;
	max_duration_ = 2000;
	in_time_ = 2000;
	out_time_ = 2000;
	in_time_first_ = 0;
	out_time_last_ = 0;
	cycle_time_ = 2000;
	max_title_duration_ = 0;
	flag_for_user_duration_ = 1;
	repeat_type_ = NXT_RepeatType_None;
	effect_type_ = NXT_EffectType_NONE;
	name_ = NULL;
	category_title_ = NULL;
	icon_ = NULL;
	encoded_name_ = NULL;
	effect_offset_ = 0;
	video_overlap_ = 0;
	
	effect_flags_ = 0;

	lua_state_ = luaL_newstate();
}

void RenderItem::freeResource(){

	if(false == is_ready())
		return;
	deinitSystem();

	for (AnimationMapper_t::iterator itor = animation_mapper_.begin(); itor != animation_mapper_.end(); ++itor){

		AnimationBin* panimation = itor->second;
		panimation->release();
		if ((panimation->getRef() <= 0) && (panimation->isOrphant())){

			delete panimation;
		}
	}

	for (glTexmap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

		TextureBin* pbin = itor->second;
		SAFE_RELEASE(pbin);
	}

	for (glProgrammap_t::iterator itor = program_map_.begin(); itor != program_map_.end(); ++itor){

		ProgramBin* pbin = itor->second;
		SAFE_RELEASE(pbin);
	}

	//free
	if (name_)
		free(name_);
	if (icon_)
		free(icon_);
	if (category_title_)
		free(category_title_);
	if (encoded_name_)
		free(encoded_name_);

	flag_for_ready_ = false;
}

RenderItem::~RenderItem(){

	RenderItemBin* prender_item = getContext()->renderitem_manager_.get(namespace_);
	if (prender_item)
		prender_item->decRef();

	freeResource();
}

int RenderItem::getChildCapacity(){

	if (capacity_of_childs_ < count_of_childs_)
		return 0;
	return capacity_of_childs_ - count_of_childs_;
}

int RenderItem::getOutputType(){

	return output_type_;
}

void RenderItem::setOutputType(int type){

	output_type_ = type;
}

void RenderItem::setContext(RenderItemContext* ctx){

	ctx_ = ctx;
	ctx_->namespace_ = namespace_;
}

RenderItemContext* RenderItem::getContext(){

	return ctx_;
}

bool RenderItem::load(const char* name){

	AUTO_RET_DIR(get_current_dir_name_mine());

	namespace_ = std::string(get_current_dir_name_mine()) + std::string("\\") + std::string(name);

	rapidxml::xml_document<> doc;

	loadXML(doc, basename(std::string(name)).c_str());

	ResourceManager<LuaBin>& manager = getContext()->lua_manager_;
	LuaBin* luabin = manager.get(namespace_);

	if (NULL == luabin){

		luabin = new LuaBin();
		manager.insert(namespace_, luabin);
	}

	luabin->addRef();
	loadResource(doc, luabin);

	return true;
}

bool RenderItem::is_ready(){
	
	return flag_for_ready_;
}

bool RenderItem::load(const char* name, const char* src){

	// lua_state_ = luaL_newstate();

	namespace_ = std::string(name);

	doc_.clear();

	loadXML_from_buffer(doc_, src);

	// ResourceManager<LuaBin>& manager = *Singleton<ResourceManager<LuaBin> >::getInstance();
	// LuaBin* luabin = manager.get(namespace_);

	// if (NULL == luabin){

	// 	luabin = new LuaBin();
	// 	manager.insert(namespace_, luabin);
	// }

	// luabin->addRef();
	// loadResource(doc_, luabin);

	return true;
}

#ifdef DEMONSTRATION

bool RenderItem::loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor){

	loadUI(doc, message_processor, lua_state_);

	return true;
}

#endif//DEMONSTRATION

int RenderItem::loop(NXT_HThemeRenderer renderer){

	renderer->pcurrent_render_item_ = this;
	NXT_RenderTarget* prender_target = renderer->pdefault_render_target_;
	doChild(renderer);
	setRenderTargetAsDefault(renderer, prender_target);
	int ret = mainLoop();
	releaseRenderTarget(renderer);
	return ret;
}

void RenderItem::doListenerWork(const char* szkey, const char* val){

	std::string key(szkey);
	std::string value(val);
	valuemap_t::iterator itor = valuemap_.find(key);
	if (itor != valuemap_.end()){

		if (value == itor->second){

			return;
		}
		valuemap_.erase(itor);
	}

	valuemap_.insert(make_pair(key, value));
	listenermap_t::iterator itor_for_listener = listenermap_.find(key);
	if (itor_for_listener != listenermap_.end()){

		listenerlist_t& list = itor_for_listener->second;
		for (listenerlist_t::iterator itor_list = list.begin(); itor_list != list.end(); ++itor_list){

			Listener* plistener = *itor_list;
			plistener->doWork(lua_state_, value);
		}
	}
}

bool RenderItem::broadcastGlobalVariable(const char* szkey, const char* val){

	std::string key(szkey);
	std::string value(val);
	valuemap_t::iterator itor = valuemap_.find(key);
	if (itor != valuemap_.end()){

		if (value == itor->second){

			return false;
		}
		valuemap_.erase(itor);
	}

	valuemap_.insert(make_pair(key, value));

	char cmd[256];
	sprintf(cmd, "%s = %s", key.c_str(), value.c_str());
	luaL_dostring(lua_state_, cmd);

	listenermap_t::iterator itor_for_listener = listenermap_.find(key);
	if (itor_for_listener != listenermap_.end()){

		listenerlist_t& list = itor_for_listener->second;
		for (listenerlist_t::iterator itor_list = list.begin(); itor_list != list.end(); ++itor_list){

			Listener* plistener = *itor_list;
			plistener->doWork(lua_state_, value);
		}
	}

	return true;
}

bool RenderItem::broadcastGlobalVariable(const char* szkey, unsigned int val){

	char value_str[256];
	sprintf(value_str, "%d", val);

	return broadcastGlobalVariable(szkey, value_str);
}

bool RenderItem::broadcastGlobalVariable(const char* szkey, int val){

	char value_str[256];
	sprintf(value_str, "%d", val);

	return broadcastGlobalVariable(szkey, value_str);
}

bool RenderItem::broadcastGlobalVariable(const char* szkey, float val){

	char value_str[256];
	sprintf(value_str, "%f", val);

	return broadcastGlobalVariable(szkey, value_str);
}

lua_State* RenderItem::getLuaState(){

	return lua_state_;
}

void readFromFile_internal(const std::string& filename, std::string& buf, FileOperatorForRenderItem& ops){

	char* poutput = NULL;
	int length = 0;
	if(0 == (*ops.ploadfunc)(&poutput, &length, (char*)filename.c_str(), ops.cbdata_for_loadfunc)){

//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d] %s", __func__, __LINE__, poutput);
		buf.append(poutput);
	}
}

void RenderItem::loadProgram(rapidxml::xml_document<>& doc, const char* name, std::string& program_buf, LuaBin* luabin){

	CHECK_GL_ERROR();
	std::string program_buf_for_vertex;
	std::string program_buf_for_fragment;

	std::string vertex_shader_key;
	std::string fragment_shader_key;

	std::string program_key;

	ShaderBin* pvertex_shader_bin = NULL;
	ShaderBin* pfragment_shader_bin = NULL;

	GLuint vertex_shader = 0;
	GLuint fragment_shader = 0;

	ResourceManager<ShaderBin>& manager = getContext()->shader_manager_;

	rapidxml::xml_node<> *root_node = NULL, *node = NULL;
	root_node = node = getNode(&doc, "renderitem", NULL, NULL);
	if (node) node = getNode(node, "program", "name", name);
	if (node) {
		const char* vtx_shader_name = getAttribStr(node, "vertexShader");
		const char* frg_shader_name = getAttribStr(node, "fragmentShader");
		
		if (vtx_shader_name){

			node = getNode(root_node, "shader", "name", vtx_shader_name);

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] vtx_shader_name:%s", __func__, __LINE__, vtx_shader_name);

			if (node){

				char* filename = getAttribStr(node, "src");
				if (filename){

					vertex_shader_key = std::string(filename);
					pvertex_shader_bin = manager.get(vertex_shader_key);

					if (NULL == pvertex_shader_bin){

						readFromFile_internal(namespace_ + std::string("/") + vertex_shader_key, program_buf_for_vertex, getContext()->file_ops_);
					}
				}
				else{

					vertex_shader_key = namespace_ + std::string(":") + std::string(vtx_shader_name);
					pvertex_shader_bin = manager.get(vertex_shader_key);

					if (NULL == pvertex_shader_bin){
						node = getCDataNode(node);
						if (!node)
							return;
						program_buf_for_vertex.append(node->value());
					}				
				}
			}
		}

		if (frg_shader_name){

			node = getNode(root_node, "shader", "name", frg_shader_name);

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] frg_shader_name:%s", __func__, __LINE__, frg_shader_name);

			if (node){

				char* filename = getAttribStr(node, "src");
				if (filename){

					fragment_shader_key = std::string(filename);
					pfragment_shader_bin = manager.get(fragment_shader_key);

					if (NULL == pfragment_shader_bin){

						readFromFile_internal(namespace_ + std::string("/") + fragment_shader_key, program_buf_for_fragment, getContext()->file_ops_);
					}
				}
				else{

					fragment_shader_key = namespace_ + std::string(":") + std::string(frg_shader_name);
//					nexSAL_TraceCat(0, 0, "RenderItem [%s %d] fragment_shader_key:%s", __func__, __LINE__, fragment_shader_key.c_str());
					pfragment_shader_bin = manager.get(fragment_shader_key);

					if (NULL == pfragment_shader_bin){
						node = getCDataNode(node);
						if (!node)
							return;
//						nexSAL_TraceCat(0, 0, "RenderItem [%s %d] frg_shader_name:%s", __func__, __LINE__, fragment_shader_key.c_str());
						program_buf_for_fragment.append(node->value());
//						nexSAL_TraceCat(0, 0, "RenderItem [%s %d] frg_shader_name:%s", __func__, __LINE__, fragment_shader_key.c_str());
					}
				}
			}
		}
	}
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	if (program_buf_for_vertex.size()){

//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

		pvertex_shader_bin = new ShaderBin();

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* pchar = program_buf_for_vertex.c_str();
		const GLint size = program_buf_for_vertex.size();
		glShaderSource(vertex_shader, 1, &pchar, &size);
		glCompileShader(vertex_shader);

		char error_text[1024];
		GLsizei length = 0;
		glGetShaderInfoLog(vertex_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d] error_text:%s size:%d pchar:%s", __func__, __LINE__, error_text, program_buf_for_vertex.size(),pchar);

		pvertex_shader_bin->set(vertex_shader);
		manager.insert(vertex_shader_key, pvertex_shader_bin);
	}

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

	if (program_buf_for_fragment.size()){

//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

		pfragment_shader_bin = new ShaderBin();
		
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		const GLchar* pchar = program_buf_for_fragment.c_str();
		const GLint size = program_buf_for_fragment.size();
		glShaderSource(fragment_shader, 1, &pchar, &size);
		glCompileShader(fragment_shader);

		char error_text[1024];
		GLsizei length = 0;
		glGetShaderInfoLog(fragment_shader, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d] error_text:%s size:%d pchar:%s", __func__, __LINE__, error_text, size, pchar);


		pfragment_shader_bin->set(fragment_shader);
		manager.insert(fragment_shader_key, pfragment_shader_bin);
	}

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

	CHECK_GL_ERROR();
	if (pvertex_shader_bin && pfragment_shader_bin){

//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

		program_key = vertex_shader_key + std::string("||") + fragment_shader_key;
		ResourceManager<ProgramBin>& manager = getContext()->program_manager_;
		ProgramBin* pprogram_bin = manager.get(program_key);

//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

		if (NULL == pprogram_bin){

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

			GLuint program = glCreateProgram(); CHECK_GL_ERROR();

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] program:%d", __func__, __LINE__, program);
			glAttachShader(program, pvertex_shader_bin->get()); CHECK_GL_ERROR();
//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] pvertex_shader_bin->get():%d", __func__, __LINE__, pvertex_shader_bin->get());
			glAttachShader(program, pfragment_shader_bin->get()); CHECK_GL_ERROR();
//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] pfragment_shader_bin->get():%d", __func__, __LINE__, pfragment_shader_bin->get());
			glLinkProgram(program); CHECK_GL_ERROR();
//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
			char error_text[1024];
			GLsizei length = 0;
			glGetProgramInfoLog(program, sizeof(error_text), &length, error_text); CHECK_GL_ERROR();
			// if (length > 0 && !strcasecmp(error_text, "no errors."))
			// 	printf("GLLinkError\n%s\n", error_text);

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] error_text:%s", __func__, __LINE__, error_text);
			
			if (program > 0)
			{
				pprogram_bin = new ProgramBin();

				pprogram_bin->set(pvertex_shader_bin, pfragment_shader_bin, program);
				manager.insert(program_key, pprogram_bin);
			}
//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
		}
//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

		if(luabin->getSize() > 0){

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

			program_map_.insert(make_pair(std::string(name), pprogram_bin));

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d] pprogram_bin:0x%x", __func__, __LINE__, pprogram_bin);

			pprogram_bin->addRef();
			glProgramIndexmap_t::iterator itor = program_idx_map_.find(pprogram_bin->get());
			int tex_target_count = 0;

			if (itor != program_idx_map_.end()){

				glResmap_t map;
				int total = -1;
				
				glGetProgramiv(pprogram_bin->get(), GL_ACTIVE_UNIFORMS, &total);
				for (int i = 0; i < total; ++i){

					char uniform_name[255];
					GLsizei ret_size = 0;
					GLsizei uniform_size = 0;
					GLenum type = 0;
					glGetActiveUniform(pprogram_bin->get(), i, sizeof(uniform_name)-1, &ret_size, &uniform_size, &type, uniform_name); CHECK_GL_ERROR();
					if (ret_size > 0){

						uniform_name[ret_size] = 0;
						GLuint index = glGetUniformLocation(pprogram_bin->get(), uniform_name); CHECK_GL_ERROR();
						map.insert(make_pair(std::string(uniform_name), index));
					}
				}

				glGetProgramiv(pprogram_bin->get(), GL_ACTIVE_ATTRIBUTES, &total);
				for (int i = 0; i < total; ++i){

					char uniform_name[255];
					GLsizei ret_size = 0;
					GLsizei uniform_size = 0;
					GLenum type = 0;
					glGetActiveAttrib(pprogram_bin->get(), i, sizeof(uniform_name)-1, &ret_size, &uniform_size, &type, uniform_name);
					if (ret_size > 0){

						uniform_name[ret_size] = 0;
						GLuint index = glGetAttribLocation(pprogram_bin->get(), uniform_name); CHECK_GL_ERROR();
						map.insert(make_pair(std::string(uniform_name), index));
					}
				}
				program_idx_map_.insert(make_pair(pprogram_bin->get(), map));
			}
		}
		else{

			CHECK_GL_ERROR();
			
//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

			program_map_.insert(make_pair(std::string(name), pprogram_bin));

//			nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

			pprogram_bin->addRef();
			glProgramIndexmap_t::iterator itor = program_idx_map_.find(pprogram_bin->get());
			int tex_target_count = 0;
			
			program_buf.append(std::string(name) + std::string("={}\n"));
			if (itor == program_idx_map_.end()){

				glResmap_t map;
				int total = -1;
				glGetProgramiv(pprogram_bin->get(), GL_ACTIVE_UNIFORMS, &total);

//				nexSAL_TraceCat(0, 0, "RenderItem [%s %d] prog:%d GL_ACTIVE_UNIFORMS total:%d", __func__, __LINE__, pprogram_bin->get(), total);

				for (int i = 0; i < 255; ++i){

					char uniform_name[255];
					GLsizei ret_size = 0;
					GLsizei uniform_size = 0;
					GLenum type = 0;
					glGetActiveUniform(pprogram_bin->get(), i, sizeof(uniform_name)-1, &ret_size, &uniform_size, &type, uniform_name); CHECK_GL_ERROR();
					
					if (ret_size > 0){

						uniform_name[ret_size] = 0;
//						nexSAL_TraceCat(0, 0, "RenderItem [%s %d] uniform_name:%s", __func__, __LINE__, uniform_name);
						GLuint index = glGetUniformLocation(pprogram_bin->get(), uniform_name); CHECK_GL_ERROR();
						map.insert(make_pair(std::string(uniform_name), index));
						makeSetterForShaderProgram(program_buf, name, uniform_name, type, tex_target_count);
					}
				}
				
				glGetProgramiv(pprogram_bin->get(), GL_ACTIVE_ATTRIBUTES, &total);

//				nexSAL_TraceCat(0, 0, "RenderItem [%s %d] GL_ACTIVE_ATTRIBUTES total:%d", __func__, __LINE__, total);

				for (int i = 0; i < 255; ++i){

					char uniform_name[255];
					GLsizei ret_size = 0;
					GLsizei uniform_size = 0;
					GLenum type = 0;
					glGetActiveAttrib(pprogram_bin->get(), i, sizeof(uniform_name)-1, &ret_size, &uniform_size, &type, uniform_name); CHECK_GL_ERROR();
					if (ret_size > 0){

						uniform_name[ret_size] = 0;
						GLuint index = glGetAttribLocation(pprogram_bin->get(), uniform_name); CHECK_GL_ERROR();
						map.insert(make_pair(std::string(uniform_name), index));
					}
				}

				//Let's make default function
				makeDefaultFunc(program_buf, name);
				program_idx_map_.insert(make_pair(pprogram_bin->get(), map));
			}
		}
	}
}

void RenderItem::makeSetterForShaderProgram(std::string& program_buf, const char* name, const char* uniform_name, int type, int& tex_target_count){

	//make setter string
	std::string class_id(name);
	std::string func_id;
	std::string uniform_id(uniform_name);
//	std::string tex_target_id = static_cast<std::ostringstream*>(&(std::ostringstream() << tex_target_count))->str();
    std::ostringstream ostr;
    ostr << tex_target_count;
    std::string tex_target_id = ostr.str();
	{

		std::string cmp("u_");
		std::string setter_name(uniform_name);
		std::size_t found = setter_name.find(cmp);
		if (0 == found){
			setter_name.erase(found, cmp.length());
		}
		cmp = std::string("_");
		found = setter_name.find(cmp);
		while (found != std::string::npos){
			setter_name.erase(found, cmp.length());
			setter_name.replace(found, 1, 1, toupper(setter_name.at(found)));
			found = setter_name.find(cmp);
		}
		setter_name.replace(0, 1, 1, toupper(setter_name.at(0)));
		setter_name.insert(0, "set");

		func_id = setter_name;
	}

	std::string function_body(getPrototypeForSetter(type));
	if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE)
		++tex_target_count;

	std::vector<tagConverter> cmpcmp;
	cmpcmp.push_back({ std::string("%class_name"), class_id });
	cmpcmp.push_back({ std::string("%setter_name"), func_id });
	cmpcmp.push_back({ std::string("%uniform_name"), uniform_id });
	cmpcmp.push_back({ std::string("%tex_target_id"), tex_target_id });

	for (size_t i = 0; i < cmpcmp.size(); ++i)
	{
		tagConverter& conv = cmpcmp[i];
		std::size_t found = function_body.find(conv.target_);
		while (found != std::string::npos){

			function_body.replace(found, conv.target_.length(), conv.name_);
			found = function_body.find(conv.target_);
		}
	}
	program_buf.append(function_body);
}

void RenderItem::makeDefaultFunc(std::string& program_buf, const char* name){

	const strstrmapper_t prototype = getPrototypeForFunction();
	for (strstrmapper_t::const_iterator itor = prototype.begin(); itor != prototype.end(); ++itor){

		std::vector<tagConverter> cmpcmp;
		cmpcmp.push_back({ std::string("%class_name"), std::string(name) });
		cmpcmp.push_back({ std::string("%setter_name"), std::string(itor->first) });
		std::string function_body = itor->second;

		for (size_t i = 0; i < cmpcmp.size(); ++i)
		{
			tagConverter& conv = cmpcmp[i];
			std::size_t found = function_body.find(conv.target_);
			while (found != std::string::npos){

				function_body.replace(found, conv.target_.length(), conv.name_);
				found = function_body.find(conv.target_);
			}
		}
		program_buf.append(function_body);
	}
}

void RenderItem::loadProgram(rapidxml::xml_document<>& doc, std::string& program_buf, LuaBin* luabin){

	rapidxml::xml_node<> *root_node = NULL, *node = NULL;
	root_node = node = getNode(&doc, "renderitem", NULL, NULL);
	if (node){
		rapidxml::xml_node<> *program_node = NULL;
		program_node = getNode(node, "program", NULL, NULL);
		while (program_node){
			char* name = getAttribStr(program_node, "name");
			if (name)
				loadProgram(doc, name, program_buf, luabin);
			program_node = getNextNode(program_node, "program", NULL, NULL);
		}
	}
}

bool RenderItem::loadXML(rapidxml::xml_document<>& doc, const char* filename){

	std::string buf;
	readFromFile(filename, buf, true);
	char* src_text = new char[buf.length() + 1];
	src_text[buf.length()] = 0;
	memcpy(src_text, buf.c_str(), sizeof(char)* buf.length());
	doc.parse<0>(src_text);

	return true;
}

bool RenderItem::loadXML_from_buffer(rapidxml::xml_document<>& doc, const char* src){

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] src:0x%x", __func__, __LINE__, src);
	std::string buf(src);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] buf_length:%d", __func__, __LINE__, buf.length());
	char* src_text = new char[buf.length() + 1];
	src_text[buf.length()] = 0;
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	memcpy(src_text, buf.c_str(), sizeof(char)* buf.length());
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	doc.parse<0>(src_text);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

	return true;
}

void RenderItem::deinitSystem(){

	lua_close(lua_state_);
	LuaBin* luabin = getContext()->lua_manager_.get(namespace_);
	if (luabin)
		luabin->decRef();
}

void RenderItem::loadResource(rapidxml::xml_document<>& doc, LuaBin* luabin){

	// lua_state_ = luaL_newstate();
	std::string program_buf;

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	loadProgram(doc, program_buf, luabin);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	loadResource(doc);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

	loadLua(doc, lua_state_, program_buf, luabin);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
}

#ifdef DEMONSTRATION

void RenderItem::addSlider(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, float min_value, float max_value, float default_value, float step){

	Slider* pslider = new Slider(pframe, "ui/slider_panel.png");
	pslider->setRenderItem(this);
	pslider->setPos(x, y);
	pslider->setTitle(title);
	pslider->setScriptTarget(script_target);
	pslider->setLuaState(L);

	SliderBody* pslider_body = new SliderBody(NULL, "ui/slider_body.png");
	pslider_body->setRenderItem(this);

	Frame* pbutton2 = new Frame(NULL, "ui/slider_btn.png");
	pbutton2->setRenderItem(this);
	pslider_body->setSliderRect(10, 12);
	pslider_body->setSliderLength(104);
	pslider_body->setSliderMinValue(min_value);
	pslider_body->setSliderMaxValue(max_value);
	pslider_body->setSliderValue(default_value);
	pslider_body->setSliderStep(step);
	pbutton2->setPos(10, 0);
	pslider_body->addSlider(pbutton2);
	pslider_body->setMovable(false);
	pslider->addSlider(pslider_body);
	pframe->addChild(pslider);
}

void RenderItem::addCheckBox(Frame* pframe, lua_State* L, const char* title, const char* script_target, float x, float y, bool value){

	Checkbox* pcheckbox = new Checkbox("ui/check_box_for_true.png", "ui/check_box_for_false.png");
	pcheckbox->setRenderItem(this);
	pcheckbox->setPos(x, y);
	pcheckbox->setLuaState(L);
	pcheckbox->setTitle(title);
	pcheckbox->setScriptTarget(script_target);
	pcheckbox->setState(value);
	pframe->addChild(pcheckbox);
}

float RenderItem::addColorPicker(Frame* pframe, lua_State* L, bool alpha_mode, const char* title, const char* script_target, float x, float y, const char* value){


	lua_newtable(L);

	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "r");
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "g");
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "b");

	if (alpha_mode){

		lua_pushinteger(L, 0);
		lua_setfield(L, -2, "a");
	}

	lua_setglobal(L, script_target);

	ColorPicker* pcolor_picker = new ColorPicker(NULL, NULL, alpha_mode);
	pcolor_picker->setRenderItem(this);
	pcolor_picker->setLuaState(L);
	pcolor_picker->setTitle(title);
	pcolor_picker->setScriptTarget(script_target);
	pcolor_picker->setPos(x, y);
	pcolor_picker->setValue(value);
	pframe->addChild(pcolor_picker);
	y += pcolor_picker->getHeight();

	return y;
}

void RenderItem::loadUI(rapidxml::xml_document<>& doc, MessageProcessor& message_processor, lua_State* L){

	Frame* pframe = message_processor.getTopFrame();

	rapidxml::xml_node<> *node = NULL;

	node = getNode(&doc, "userfields", NULL, NULL);

	while (node){

		float x = 0.0f;
		float y = 0.0f;
		rapidxml::xml_node<> *node_for_slider = NULL;
		node_for_slider = getNode(node, "slider", NULL, NULL);
		while (node_for_slider){

			char* title = getAttribStr(node_for_slider, "name");
			char script_target[256];
			sprintf(script_target, "%s", title);
			float min_value = (float)atof(getAttribStr(node_for_slider, "minvalue"));
			float max_value = (float)atof(getAttribStr(node_for_slider, "maxvalue"));
			float default_value = (float)atof(getAttribStr(node_for_slider, "default"));
			float step = (float)atof(getAttribStr(node_for_slider, "step"));

			addSlider(pframe, L, title, script_target, x, y, min_value, max_value, default_value, step);
			y += 45.0f;

			node_for_slider = getNextNode(node_for_slider, "slider", NULL, NULL);
		}

		y += 10.0f;

		rapidxml::xml_node<> *node_for_checkbox = NULL;

		node_for_checkbox = getNode(node, "checkbox", NULL, NULL);
		while (node_for_checkbox){

			char* title = getAttribStr(node_for_checkbox, "name");
			char script_target[256];
			sprintf(script_target, "%s", title);
			char* value = getAttribStr(node_for_checkbox, "default");

			bool default_value = false;
			if (0 == strcmp(value, "true")){

				default_value = true;
			}

			addCheckBox(pframe, L, title, script_target, x, y, default_value);
			y += 45.0f;
			node_for_checkbox = getNextNode(node_for_checkbox, "checkbox", NULL, NULL);
		}

		y += 10.0f;

		rapidxml::xml_node<> *node_for_colorpicker = NULL;

		node_for_colorpicker  = getNode(node, "colorpicker", NULL, NULL);
		while (node_for_colorpicker ){

			char* title = getAttribStr(node_for_colorpicker , "name");
			char script_target[256];
			sprintf(script_target, "%s", title);
			char* value = getAttribStr(node_for_colorpicker , "default");
			
			y = addColorPicker(pframe, L, false, title, script_target, x, y, value);

			node_for_colorpicker  = getNextNode(node_for_colorpicker , "colorpicker", NULL, NULL);
		}

		node_for_colorpicker = getNode(node, "colorpickerWithAlpha", NULL, NULL);
		while (node_for_colorpicker){

			char* title = getAttribStr(node_for_colorpicker, "name");
			char script_target[256];
			sprintf(script_target, "%s", title);
			char* value = getAttribStr(node_for_colorpicker, "default");

			y = addColorPicker(pframe, L, true, title, script_target, x, y, value);

			node_for_colorpicker = getNextNode(node_for_colorpicker, "colorpickerWithAlpha", NULL, NULL);
		}

		y += 10.0f;

		rapidxml::xml_node<> *node_for_imagedropbox = NULL;

		node_for_imagedropbox = getNode(node, "image", NULL, NULL);
		while (node_for_imagedropbox){

			char* title = getAttribStr(node_for_imagedropbox, "name");
			char script_target[256];
			sprintf(script_target, "image.%s", title);
			char* value = getAttribStr(node_for_imagedropbox, "default");

			ImageDropbox* pImgframe = new ImageDropbox(NULL, "ui/slider_panel.png", script_target, value);

			pImgframe->setRenderItem(this);

			pImgframe->setMovable(false);

			pframe->addChild(pImgframe);

			pImgframe->setPos(x, y);

			y += pImgframe->getHeight();

			

			node_for_imagedropbox = getNextNode(node_for_imagedropbox, "image", NULL, NULL);
		}

		node = getNextNode(node, "userfields", NULL, NULL);
	}
	

	message_processor.updateUI();
}

#endif//DEMONSTRATION


static int luaDumpCB(lua_State* L, unsigned char* str, size_t len, struct luaL_Buffer *buf){

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] %s", __func__, __LINE__, (const char*)str);
	luaL_addlstring(buf, (const char*)str, len);
	return 0;
}

void RenderItem::loadLua(rapidxml::xml_document<>& doc, lua_State* L, std::string& program_buf, LuaBin* luabin){

	if (luabin->getSize() <= 0){

		const char* system_values[] = {

			"play_percentage",
			"play_part",
			"play_part_percentage",
			"video_src",
			"video_left",
			"video_right",
			"texture_matrix",
		};

		char cmd[256];

		program_buf.append("system={\n");
		for (int i = 0; i < sizeof(system_values) / sizeof(char*); ++i){

			sprintf(cmd, "%s=0,\n", system_values[i]);
			program_buf.append(cmd);
		}
		program_buf.append("}\n");

		program_buf.append("animation={\n");
		for (AnimationMapper_t::iterator itor = animation_mapper_.begin(); itor != animation_mapper_.end(); ++itor){

			const std::string& str = itor->first;

			sprintf(cmd, "%s=\"%s\",\n", str.c_str(), str.c_str());
			program_buf.append(cmd);
		}
		program_buf.append("}\n");

		program_buf.append("animatedvalue={\n");
		for (AnimatedNodeList_t::iterator itor = animated_node_list_.begin(); itor != animated_node_list_.end(); ++itor){

			NXT_Node_AnimatedValue& item = *(*itor);

			sprintf(cmd, "%s={\n", item.id_.c_str());
			program_buf.append(cmd);

			const char* elements_names[] = {
				"x",
				"y",
				"z",
				"w"
			};

			for (int i = 0; i < item.element_count; ++i){

				sprintf(cmd, "%s=0,\n", elements_names[i]);
				program_buf.append(cmd);
			}
			program_buf.append("},\n");
		}
		program_buf.append("}\n");

		program_buf.append("texture={}\n");
		for (glTexmap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

			const std::string& str = itor->first;
			TextureBin* image = itor->second;

			sprintf(cmd, "%s=%d\n", str.c_str(), image->getTexID());
			program_buf.append(cmd);
		}

		program_buf.append("program={\n");
		for (glProgrammap_t::iterator itor = program_map_.begin(); itor != program_map_.end(); ++itor){

			const std::string& str = itor->first;
			ProgramBin* pprogram_bin = itor->second;
			GLuint program_idx = pprogram_bin->get();

			glProgramIndexmap_t::iterator itor_for_idmap = program_idx_map_.find(program_idx);
			if (itor_for_idmap == program_idx_map_.end())
				continue;

			sprintf(cmd, "%s={\n", str.c_str());
			program_buf.append(cmd);

			sprintf(cmd, "id=%d,\n", program_idx);
			program_buf.append(cmd);
			glResmap_t& program_id_map = itor_for_idmap->second;
			for (glResmap_t::iterator itor = program_id_map.begin(); itor != program_id_map.end(); ++itor){

				const std::string& str = itor->first;
				int idx = itor->second;

				sprintf(cmd, "%s=%d,\n", str.c_str(), idx);
				program_buf.append(cmd);
			}

			program_buf.append("},\n");
		}
		program_buf.append("}\n");

		program_buf.append("indexdata={\n");
		for (glDataIdxMapper_t::iterator itor = polygon_index_data_map_.begin(); itor != polygon_index_data_map_.end(); ++itor){

			const std::string& str = itor->first;
			int data_id = itor->second;
			std::vector<GLushort>& data = polygon_index_data_list_[data_id];

			sprintf(cmd, "%s={\n", str.c_str());
			program_buf.append(cmd);
			sprintf(cmd, "size=%d,\n", data.size());
			program_buf.append(cmd);
			sprintf(cmd, "data=%d\n", data_id);
			program_buf.append(cmd);
			program_buf.append("},\n");
		}
		program_buf.append("}\n");

		program_buf.append("texcoord={\n");
		for (glDataIdxMapper_t::iterator itor = texcoord_data_map_.begin(); itor != texcoord_data_map_.end(); ++itor){

			const std::string& str = itor->first;
			int data_id = itor->second;
			std::vector<float>& data = texcoord_data_list_[data_id];

			sprintf(cmd, "%s={\n", str.c_str());
			program_buf.append(cmd);
			sprintf(cmd, "size=%lu,\n", data.size());
			program_buf.append(cmd);
			sprintf(cmd, "data=%d\n", data_id);
			program_buf.append(cmd);
			program_buf.append("},\n");
		}
		program_buf.append("}\n");

		program_buf.append("vertexdata={\n");
		for (glDataIdxMapper_t::iterator itor = vertex_data_map_.begin(); itor != vertex_data_map_.end(); ++itor){

			const std::string& str = itor->first;
			int data_id = itor->second;
			std::vector<float>& data = vertex_data_list_[data_id];

			sprintf(cmd, "%s={\n", str.c_str());
			program_buf.append(cmd);
			sprintf(cmd, "element_size=%d,\n", (int)data[0]);
			program_buf.append(cmd);
			sprintf(cmd, "size=%lu,\n", data.size());
			program_buf.append(cmd);
			sprintf(cmd, "data=%d", data_id);
			program_buf.append(cmd);
			program_buf.append("},\n");
		}
		program_buf.append("}\n");

		rapidxml::xml_node<> *root_node = NULL, *node = NULL;
		root_node = node = getNode(&doc, "renderitem", NULL, NULL);
		if (node) {

			node = getNode(node, "script", "language", "lua");
			while (node){
				char* filename = getAttribStr(node, "src");
				
				if (filename){

//					nexSAL_TraceCat(0, 0, "RenderItem [%s %d] readFromFile(%s, program_buf)", __func__, __LINE__, filename);
					readFromFile_internal(namespace_ + std::string("/") + std::string(filename), program_buf, getContext()->file_ops_);
				}
				else{
					rapidxml::xml_node<> *program_node = getCDataNode(node);
					if (program_node){
						program_buf.append(program_node->value());
					}
				}
				node = getNextNode(node, "script", "language", "lua");
			}
		}

		if (program_buf.size() <= 0)
			return;
	}

	if (loadLua(L, luabin))
		return;
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);

	int ret = luaL_loadstring(L, program_buf.c_str());
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] ret:%d", __func__, __LINE__, ret);
//	if(1){
//
//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d] length:%d ret:%s", __func__, __LINE__, program_buf.length(), program_buf.c_str());
//	}

	int res;
	luaL_Buffer buf;

	luaL_buffinit(L, &buf);

	res = lua_dump(L, (lua_Writer)luaDumpCB, &buf, 1);

	luaL_pushresult(&buf);

	luabin->set(buf.b, buf.n);

	lua_pop(L, 1);

	lua_pcall(L, 0, LUA_MULTRET, 0);

	CHECK_GL_ERROR();
}

Listener::Listener(){


}

Listener::~Listener(){


}

void Listener::doWork(lua_State* lua_state, std::string& value){


}

class TextureListener :public Listener{

	std::string target_;
	glTexmap_t& texmap_;
	RenderItemContext* ctx_;

public:

	TextureListener(RenderItemContext* ctx, glTexmap_t& texmap, std::string& target) :Listener(), ctx_(ctx), texmap_(texmap), target_(target){

	}

	TextureListener(glTexmap_t& texmap, const TextureListener& item):texmap_(texmap), target_(item.target_){

		ctx_ = item.ctx_;
	}

	~TextureListener(){

	}

	virtual void doWork(lua_State* lua_state, std::string& value){

		glTexmap_t::iterator itor = texmap_.find(target_);
		if (itor != texmap_.end()){

			TextureBin* image = itor->second;
			texmap_.erase(itor);
			image->decRef();
		}

		ResourceManager<TextureBin>& manager = ctx_->texture_manager_;

		TextureBin* image = manager.get(value);

		if (NULL == image){

			float w(0), h(0);
			image = new TextureBin();

			char* poutput = NULL;
			int length = 0;
			if(0 == (*ctx_->file_ops_.ploadfunc)(&poutput, &length, (char*)(ctx_->namespace_ + std::string("/") + value).c_str(), ctx_->file_ops_.cbdata_for_loadfunc)){

//				nexSAL_TraceCat(0, 0, "RenderItem [%s %d] %s", __func__, __LINE__, poutput);

				image->setTexID(Img::loadImage(value.c_str(), poutput, length, &w, &h));
				delete[] poutput;
				image->setWidth((int)w);
				image->setHeight((int)h);
			}
			
			manager.insert(value, image);
		}

		texmap_.insert(make_pair(std::string(target_), image));
		image->addRef();
		
		if (image->getTexID() > 0){

			texInfo info;

			info.width_ = image->getWidth();
			info.height_ = image->getHeight();
			info.src_width_ = info.width_;
			info.src_height_ = info.height_;

			ctx_->texinfo_map_.insert(std::pair<GLuint, texInfo>(image->getTexID(), info));
		}

		if (lua_state){

			char cmd[256];
			sprintf(cmd, "%s = %d", target_.c_str(), image->getTexID());
			luaL_dostring(lua_state, cmd);
		}
	}
};

class SystemVariableListener :public Listener{

	std::string target_;

public:

	SystemVariableListener(std::string& target) :Listener(), target_(target){

	}

	SystemVariableListener(const SystemVariableListener& item){

		target_ = item.target_;
	}

	~SystemVariableListener(){

	}

	virtual void doWork(lua_State* lua_state, std::string& value){

		if (lua_state){

			char cmd[256];
			sprintf(cmd, "%s = %s", target_.c_str(), value.c_str());
			luaL_dostring(lua_state, cmd);
		}
	}
};


void RenderItem::processRenderItemAttr(rapidxml::xml_node<>* pnode){

	if (NULL == pnode){

		//default setting
		return;
	}

	setOutputType(0);

	rapidxml::xml_attribute<>* pattrib = getFirstAttribChain(pnode);
	while (pattrib){

		char* attrName = pattrib->name();
		char* attrValue = pattrib->value();
		
		if (strcasecmp(attrName, "name") == 0) {
			if (name_)
				free(name_);
			name_ = (char*)malloc(strlen(attrValue) + 1);
			strcpy(name_, attrValue);
		}
		else if (strcasecmp(attrName, "output_type") == 0){

			if (strcasecmp(attrValue, "resource") == 0){

				setOutputType(1);
			}
		}
		else if (strcasecmp(attrName, "icon") == 0) {
			if (icon_)
				free(icon_);
			icon_ = (char*)malloc(strlen(attrValue) + 1);
			strcpy(icon_, attrValue);
		}
		else if (strcasecmp(attrName, "categoryTitle") == 0) {
			if (category_title_)
				free(category_title_);
			category_title_ = (char*)malloc(strlen(attrValue) + 1);
			strcpy(category_title_, attrValue);
		}
		else if (strcasecmp(attrName, "type") == 0) {
			if (strcasecmp(attrValue, "transition") == 0) {
				effect_type_ = NXT_EffectType_Transition;
				if (!(effect_flags_ & NXT_EFFECT_FLAG_SETOFFSET)) {
					effect_offset_ = -100;
				}
				if (!(effect_flags_ & NXT_EFFECT_FLAG_SETOVERLAP)) {
					video_overlap_ = 100;
				}
			}
			else if (strcasecmp(attrValue, "title") == 0) {
				effect_type_ = NXT_EffectType_Title;
				if (!(effect_flags_ & NXT_EFFECT_FLAG_SETOFFSET)) {
					effect_offset_ = 0;
				}
				if (!(effect_flags_ & NXT_EFFECT_FLAG_SETOVERLAP)) {
					video_overlap_ = 0;
				}
			}
			else {
				// TODO: Error
			}
		}
		else if (strcasecmp(attrName, "repeat") == 0) {
			if (strcasecmp(attrValue, "no") == 0 || strcasecmp(attrValue, "none") == 0) {
				repeat_type_ = NXT_RepeatType_None;
			}
			else if (strcasecmp(attrValue, "yes") == 0 || strcasecmp(attrValue, "repeat") == 0) {
				repeat_type_ = NXT_RepeatType_Repeat;
			}
			else if (strcasecmp(attrValue, "sawtooth") == 0) {
				repeat_type_ = NXT_RepeatType_Sawtooth;
			}
			else if (strcasecmp(attrValue, "sine") == 0) {
				repeat_type_ = NXT_RepeatType_Sine;
			}
			else {
				// TODO: Error
			}
		}
		else if (strcasecmp(attrName, "userduration") == 0) {
			if (strcasecmp(attrValue, "false") == 0) {
				flag_for_user_duration_ = 0;
			}
			else if (strcasecmp(attrValue, "true") == 0) {
				flag_for_user_duration_ = 1;
			}
			else {
				// TODO: Error
			}
		}
		else if (strcasecmp(attrName, "intime") == 0) {
			in_time_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "outtime") == 0) {
			out_time_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "intimefirst") == 0) {
			in_time_first_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "outtimelast") == 0) {
			out_time_last_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "cycleTime") == 0) {
			cycle_time_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "minduration") == 0) {
			min_duration_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "maxduration") == 0) {
			max_duration_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "maxtitleduration") == 0) {
			max_title_duration_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "defaultduration") == 0) {
			default_duration_ = atoi(attrValue);
		}
		else if (strcasecmp(attrName, "effectoffset") == 0) {
			effect_offset_ = atoi(attrValue);
			effect_flags_ |= NXT_EFFECT_FLAG_SETOFFSET;
		}
		else if (strcasecmp(attrName, "videooverlap") == 0) {
			video_overlap_ = atoi(attrValue);
			effect_flags_ |= NXT_EFFECT_FLAG_SETOVERLAP;
		}
		else {
			// TODO: Error        
		}
		
		pattrib = pattrib->next_attribute();
	}
}

SnapshotOfAniFrame RenderItem::getAniFrame(const char* name, float timing){

	if (prender_item_body_)
		return prender_item_body_->getAniFrame(name, timing);

	SnapshotOfAniFrame ret;
	
	AnimationMapper_t::iterator itor = animation_mapper_.find(std::string(name));

	if (itor == animation_mapper_.end())
		return ret;

	Animation* pani = (itor->second)->get();

	return pani->getAniFrame(timing);
}

void RenderItem::loadResource(rapidxml::xml_document<>& doc){

	rapidxml::xml_node<> *root_node = NULL;
	root_node = getNode(&doc, "renderitem", NULL, NULL);
	processRenderItemAttr(root_node);
	if (root_node){

		rapidxml::xml_node<> *node = NULL;

		node = getNode(root_node, "animation", NULL, NULL);
		while (node){

			char* name = getAttribStr(node, "name");
			char* src = getAttribStr(node, "src");

			AnimationBin* panimation_bin = NULL;

			AnimationMapper_t::iterator itor = animation_mapper_.find(std::string(name));
			if (itor != animation_mapper_.end()){

				node = getNextNode(node, "animation", NULL, NULL);
				continue;
			}
				
			ResourceManager<AnimationBin>& manager = getContext()->animation_manager_;
			if (src){

				panimation_bin = manager.get(std::string(src));
			}

			if (!panimation_bin){

				panimation_bin = new AnimationBin();
				Animation* panimation = new Animation();
				panimation_bin->set(panimation);

				if (src){

					AUTO_RET_DIR(get_current_dir_name_mine());

					rapidxml::xml_document<> ani_doc;
					loadXML(ani_doc, basename(std::string(src)).c_str());
					panimation->load(&ani_doc);

					manager.insert(std::string(src), panimation_bin);
				}
				else{
					panimation_bin->setOrphant(true);
					panimation->load(node);
				}
			}
			
			animation_mapper_.insert(make_pair(std::string(name), panimation_bin));
			SAFE_ADDREF(panimation_bin);			
			node = getNextNode(node, "animation", NULL, NULL);
		}

		node = getNode(root_node, "animatedvalue", NULL, NULL);
		while (node){

			NXT_Node_AnimatedValue& animatedValue = *(new NXT_Node_AnimatedValue);

			rapidxml::xml_attribute<>* attrib = getFirstAttribChain(node);
			
			while (attrib){

				char* attrName = attrib->name();
				char* attrValue = attrib->value();

				animatedValue.setAttr(attrName, attrValue);

				attrib = attrib->next_attribute();
			}

			rapidxml::xml_node<> *child_node = getNode(node, NULL, NULL, NULL);
			animatedValue.processChildNode(child_node);
			animated_node_list_.push_back(&animatedValue);
			node = getNextNode(node, "animatedvalue", NULL, NULL);
		}

		node = getNode(root_node, "texture", NULL, NULL);
		while (node){
			char* res_name = getAttribStr(node, "name");
			char* filename = getAttribStr(node, "src");

			std::string resource_key = std::string("texture.") + std::string(res_name);
			std::string str_for_file("");

			if (filename){

				if (filename[0] == '@'){

					std::string key(&filename[1]);
					filename = NULL;

					listenermap_t::iterator itor = listenermap_.find(key);
					if (itor == listenermap_.end()){

						listenerlist_t list;
						itor = listenermap_.insert(make_pair(key, list)).first;
					}
					
					listenerlist_t& list = itor->second;

					if (0 == key.compare(0, strlen("system."), "system."))
						list.push_back(new SystemVariableListener(resource_key));
					else
						list.push_back(new TextureListener(getContext(), texmap_, resource_key));
				}
				else
					str_for_file = std::string(filename);
			}

			ResourceManager<TextureBin>& manager = getContext()->texture_manager_;
			TextureBin* image = manager.get(str_for_file);

			if (NULL == image){

				float w(0), h(0);
				image = new TextureBin();

				char* poutput = NULL;
				int length = 0;
				if(0 == (*ctx_->file_ops_.ploadfunc)(&poutput, &length, (char*)(namespace_ + std::string("/") + str_for_file).c_str(), ctx_->file_ops_.cbdata_for_loadfunc)){

					GLuint tex_id = Img::loadImage(str_for_file.c_str(), poutput, length, &w, &h);
//					nexSAL_TraceCat(0, 0, "RenderItem [%s %d] tex_id:%d poutput:0x%x length:%d w:%d h:%d", __func__, __LINE__, tex_id, poutput, length, w, h);

					image->setTexID(tex_id);
					delete[] poutput;
					image->setWidth((int)w);
					image->setHeight((int)h);
				}

				manager.insert(str_for_file, image);
			}

			texmap_.insert(make_pair(resource_key, image));
			image->addRef();

			if (image->getTexID() > 0){

				texInfo info;

				info.width_ = image->getWidth();
				info.height_ = image->getHeight();

				getContext()->texinfo_map_.insert(std::pair<GLuint, texInfo>(image->getTexID(), info));
			}
			node = getNextNode(node, "texture", NULL, NULL);
		}

		node = getNode(root_node, "vertexdata", NULL, NULL);
		while (node){

			char* res_name = getAttribStr(node, "name");
			char* filename = getAttribStr(node, "src");
			if (filename){

			}
			else{
				char* element_size_str = getAttribStr(node, "element_size");
				if (element_size_str){
					int element_size = atoi(element_size_str);
					rapidxml::xml_node<>* datanode = getCDataNode(node);
					if (datanode){

						std::vector<float> vertexbuf;
						vertexbuf.push_back(float(element_size));

						std::string str_res_name(res_name);
						std::string str_value(datanode->value());
						
						readVtxData<float>(vertex_data_map_, vertex_data_list_, str_res_name, str_value, vertexbuf);
					}
				}
			}
			node = getNextNode(node, "vertexdata", NULL, NULL);
		}

		node = getNode(root_node, "texcoord", NULL, NULL);
		while (node){
			char* res_name = getAttribStr(node, "name");
			char* filename = getAttribStr(node, "src");
			if (filename){

			}
			else{
				rapidxml::xml_node<>* datanode = getCDataNode(node);
				if (datanode){
					
					std::vector<float> tmp;
					std::string str_res_name(res_name);
					std::string str_value(datanode->value());
					readVtxData<float>(texcoord_data_map_, texcoord_data_list_, str_res_name, str_value, tmp);
				}
			}
			node = getNextNode(node, "texcoord", NULL, NULL);
		}

		node = getNode(root_node, "indexdata", NULL, NULL);
		while (node){
			char* res_name = getAttribStr(node, "name");
			char* filename = getAttribStr(node, "src");
			if (filename){

			}
			else{
				rapidxml::xml_node<>* datanode = getCDataNode(node);
				if (datanode){
					 
					std::vector<GLushort> tmp;
					std::string str_res_name(res_name);
					std::string str_value(datanode->value());
					readVtxData<GLushort>(polygon_index_data_map_, polygon_index_data_list_, str_res_name, str_value, tmp);
				}
			}
			node = getNextNode(node, "indexdata", NULL, NULL);
		}
	}
}

void RenderItem::doEffect(lua_State* lua_state, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count){

	RITM_NXT_RandSeed rseed;

	RITM_NXT_SRandom(&rseed, 0x1E32A7B * clip_index, 0x59B1C13 + clip_index + clip_count);

	for (int i = 0; i<clip_index; i++) {

		RITM_NXT_FRandom(&rseed);
	}

	lua_getglobal(lua_state, "animatedvalue");

	for (AnimatedNodeList_t::iterator itor = animated_node_list_.begin(); itor != animated_node_list_.end(); ++itor){

		NXT_Node_AnimatedValue* pAnimVal = *itor;

		int i;
		float time;

		RITM_NXT_Vector4f timing;
		timing.e[0] = pAnimVal->timing[0];
		timing.e[1] = pAnimVal->timing[1];
		timing.e[2] = pAnimVal->timing[2];
		timing.e[3] = pAnimVal->timing[3];

		if (pAnimVal->basis == NXT_InterpBasis_ClipIndex) {

			timing.e[0] = 0.0f;
			timing.e[1] = 0.0f;
			timing.e[2] = 1.0f;
			timing.e[3] = 1.0f;

			time = (float)clip_index / ((float)clip_count - 1.0f);
		}
		else if (pAnimVal->basis == NXT_InterpBasis_ClipIndexPermuted) {

			timing.e[0] = 0.0f;
			timing.e[1] = 0.0f;
			timing.e[2] = 1.0f;
			timing.e[3] = 1.0f;

			time = RITM_NXT_FRandom(&rseed);
		}
		else if (pAnimVal->bUseOverallTime) {
			time = currentTimeOverall;
		}
		else {
			time = currentTimeInPart;
		}

		if (pAnimVal->noiseType == NXT_NoiseType_PerlinSimplex) {
			int octave;
			float factor = 1.0;
			float noise = 0.0;
			float maxval = 0.0;
			float p = time*pAnimVal->noiseSampleScale + pAnimVal->noiseSampleBias;
			switch (pAnimVal->noiseFunc) {
			case NXT_NoiseFunction_Fractal:
				for (octave = 0; octave < pAnimVal->noiseOctaves; octave++) {
					noise += (1.0f / factor)*RITM_simplex_noise1(p*factor);
					maxval += (1.0f / factor);
					factor++;
				}
				time = (noise / maxval)*pAnimVal->noiseScale + pAnimVal->noiseBias;
				break;
			case NXT_NoiseFunction_Turbulence:
				for (octave = 0; octave < pAnimVal->noiseOctaves; octave++) {
					float n = RITM_simplex_noise1(p*factor);
					if (n<0)
						noise += (1.0f / factor)*(-n);
					else
						noise += (1.0f / factor)*n;
					maxval += (1.0f / factor);
					factor++;
				}
				time = (noise / maxval)*pAnimVal->noiseScale + pAnimVal->noiseBias;
				break;
			case NXT_NoiseFunction_WaveFractal:
				for (octave = 0; octave < pAnimVal->noiseOctaves; octave++) {
					float n = RITM_simplex_noise1(p*factor);
					if (n<0)
						noise += (1.0f / factor)*(-n);
					else
						noise += (1.0f / factor)*n;
					factor++;
				}
				noise = sinf(p + noise);
				time = noise*pAnimVal->noiseScale + pAnimVal->noiseBias;
				break;
			case NXT_NoiseFunction_Pure:
			default:
				time = RITM_simplex_noise1(p)*pAnimVal->noiseScale + pAnimVal->noiseBias;
				break;
			}
			if (time < 0.0f)
				time = 0.0f;
			if (time > 1.0f)
				time = 1.0f;
		}

		float e[4] = { 0 };

		NXT_KeyFrameSet *kfset = NULL;
		NXT_KeyFrameSet *pDefaultSet = NULL;
		unsigned int bSkip;

		for (i = 0; i<pAnimVal->numKeyframeSets; i++) {
			bSkip = 0;
			if (pAnimVal->keyframeSet[i].bDefault) {
				pDefaultSet = &pAnimVal->keyframeSet[i];
				bSkip = 1;
			}
			if (pAnimVal->keyframeSet[i].partType != NXT_PartType_All && part != pAnimVal->keyframeSet[i].partType) {
				bSkip = 1;
			}
			switch (pAnimVal->keyframeSet[i].clipType) {
			case NXT_ClipType_All:
				break;
			case NXT_ClipType_First:
				if (clip_index != 0)
					bSkip = 1;
				break;
			case NXT_ClipType_Middle:
				if (clip_count>0 && clip_index <= 0 && clip_index >= (clip_count - 1))
					bSkip = 1;
				break;
			case NXT_ClipType_Last:
				if (clip_index<(clip_count - 1))
					bSkip = 1;
				break;
			case NXT_ClipType_Even:
				if ((clip_index % 2) != 0)
					bSkip = 1;
				break;
			case NXT_ClipType_Odd:
				if ((clip_index % 2) == 0)
					bSkip = 1;
				break;
			case NXT_ClipType_NotFirst:
				if (clip_index == 0)
					bSkip = 1;
				break;
			case NXT_ClipType_NotMiddle:
				if (!(clip_count>0 && clip_index <= 0 && clip_index >= (clip_count - 1)))
					bSkip = 1;
				break;
			case NXT_ClipType_NotLast:
				if (clip_index >= (clip_count - 1))
					bSkip = 1;
				break;
			case NXT_ClipType_NotEven:
				if ((clip_index % 2) == 0)
					bSkip = 1;
				break;
			case NXT_ClipType_NotOdd:
				if ((clip_index % 2) != 0)
					bSkip = 1;
				break;
			default:
				break;
			}

			if (!bSkip) {
				kfset = &pAnimVal->keyframeSet[i];
				break;
			}

		}

		if (!kfset && pDefaultSet) {
			kfset = pDefaultSet;
		}

		if (!kfset) {

			e[0] = 0;
			e[1] = 0;
			e[2] = 0;
			e[3] = 0;
		}
		else if (kfset->numKeyFrames == 1) {

			e[0] = kfset->keyframes[0].e[0];
			e[1] = kfset->keyframes[0].e[1];
			e[2] = kfset->keyframes[0].e[2];
			e[3] = kfset->keyframes[0].e[3];
		}
		else if (kfset->numKeyFrames > 0) {

			if (time < kfset->keyframes[0].t) {

				e[0] = kfset->keyframes[0].e[0];
				e[1] = kfset->keyframes[0].e[1];
				e[2] = kfset->keyframes[0].e[2];
				e[3] = kfset->keyframes[0].e[3];
			}
			else if (time > kfset->keyframes[kfset->numKeyFrames - 1].t) {

				e[0] = kfset->keyframes[kfset->numKeyFrames - 1].e[0];
				e[1] = kfset->keyframes[kfset->numKeyFrames - 1].e[1];
				e[2] = kfset->keyframes[kfset->numKeyFrames - 1].e[2];
				e[3] = kfset->keyframes[kfset->numKeyFrames - 1].e[3];
			}
			else {

				float animTotal = kfset->keyframes[kfset->numKeyFrames - 1].t - kfset->keyframes[0].t;
				float animElapsed = time - kfset->keyframes[0].t;
				float progress = animElapsed / animTotal;
				float interp = RITM_NXT_CubicBezierAtTime(progress,
					timing.e[0], timing.e[1],
					timing.e[2], timing.e[3],
					10.0);
				float adjtime = kfset->keyframes[0].t + (animTotal*interp);


				for (i = 0; i<kfset->numKeyFrames - 1; i++) {
					if (adjtime >= kfset->keyframes[i].t && adjtime <= kfset->keyframes[i + 1].t) {
						float elapsedTime = adjtime - kfset->keyframes[i].t;
						float totalTime = kfset->keyframes[i + 1].t - kfset->keyframes[i].t;
						float factor;
						if (totalTime <= 0.0) {
							factor = 1.0;
						}
						else {
							factor = elapsedTime / totalTime;
							factor = RITM_NXT_CubicBezierAtTime(factor,
								kfset->keyframes[i + 1].timing[0], kfset->keyframes[i + 1].timing[1],
								kfset->keyframes[i + 1].timing[2], kfset->keyframes[i + 1].timing[3],
								10.0);
						}
						e[0] = kfset->keyframes[i].e[0] + (kfset->keyframes[i + 1].e[0] - kfset->keyframes[i].e[0])*factor;
						e[1] = kfset->keyframes[i].e[1] + (kfset->keyframes[i + 1].e[1] - kfset->keyframes[i].e[1])*factor;
						e[2] = kfset->keyframes[i].e[2] + (kfset->keyframes[i + 1].e[2] - kfset->keyframes[i].e[2])*factor;
						e[3] = kfset->keyframes[i].e[3] + (kfset->keyframes[i + 1].e[3] - kfset->keyframes[i].e[3])*factor;

						break;
					}
				}
			}

		}

		int len = 0;

		const char *element_names[] = {

			"x",
			"y",
			"z",
			"w",
		};

		lua_pushstring(lua_state, pAnimVal->id_.c_str());
		lua_gettable(lua_state, -2);



		for (int i = 0; i < pAnimVal->element_count; ++i){

			lua_pushstring(lua_state, element_names[i]);
			lua_pushnumber(lua_state, e[i]);
			lua_settable(lua_state, -3);
		}

		lua_pop(lua_state, 1);
	}
	lua_pop(lua_state, 1);
}

void RenderItem::setTexMatrix(void* pmatrix){

	lua_State* lua_state = this->getLuaState();
	lua_getglobal(lua_state, "system");
	lua_pushstring(lua_state, "texture_matrix");
	void* pdata = lua_newuserdata(lua_state, sizeof(RITM_NXT_Matrix4f));
	memcpy(pdata, pmatrix, sizeof(RITM_NXT_Matrix4f));
	lua_settable(lua_state, -3);
	lua_pop(lua_state, 1);
}

void RenderItem::setValue(const char* key, int value){

	broadcastGlobalVariable(key, value);
}

void RenderItem::setValue(const char* key, float value){

	broadcastGlobalVariable(key, value);
}

void RenderItem::setValue(const char* key, const char* value){

	broadcastGlobalVariable(key, value);
}

void RenderItem::setValue(const char* key, float* value){

	lua_State* lua_state = this->getLuaState();
	void* pdata = lua_newuserdata(lua_state, sizeof(RITM_NXT_Matrix4f));
	memcpy(pdata, value, sizeof(RITM_NXT_Matrix4f));
	lua_setglobal(lua_state, key);
}

void RenderItem::doEffect(float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count)
{
	if (prender_item_body_)
		return prender_item_body_->doEffect(this->getLuaState(), currentTimeInPart, currentTimeOverall, part, clip_index, clip_count);

	this->doEffect(this->getLuaState(), currentTimeInPart, currentTimeOverall, part, clip_index, clip_count);
}

int RenderItem::mainLoop(){

//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] In", __func__, __LINE__);
	int ret_count = 0;
	int ret = 0;
	if (getOutputType() == 1)
		ret_count = 1;
	lua_getglobal(lua_state_, "main");
	ret = lua_pcall(lua_state_, 0, ret_count, 0);
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] lua_state_:0x%x lua_pcall return:%d", __func__, __LINE__, lua_state_, ret);
//	if(0 != ret){
//
//		nexSAL_TraceCat(0, 0, "RenderItem [%s %d] lua_pcall return:%s", __func__, __LINE__, lua_tostring(lua_state_, -1));
//	}
	if (ret_count > 0){

		ret = (int)lua_tointeger(lua_state_, -1);
		lua_pop(lua_state_, ret_count);
	}
//	nexSAL_TraceCat(0, 0, "RenderItem [%s %d] Out", __func__, __LINE__);
	return ret;
}

void RenderItem::addChild(RenderItem* pitem, int order){

	childlist_.push_back({ pitem, order });
	count_of_childs_ = childlist_.size();
	pitem->setParent(this);
}

void RenderItem::setParent(RenderItem* pitem){

	parent_ = pitem;
}

void RenderItem::getReady(){

	count_of_childs_ = 0;
	childlist_.clear();
	parent_ = NULL;
}

void RenderItem::doEffect(int elapsed_tick, float currentTimeInPart, float currentTimeOverall, int part, int clip_index, int clip_count){

	getReady();

	for (prefixedchildlist_t::iterator itor = prefixed_childlist_.begin(); itor != prefixed_childlist_.end(); ++itor){

		PrefixedChildItem& item = *itor;
		if (item.start_time_ <= elapsed_tick && (item.start_time_ + item.duration_) >= elapsed_tick){

			float current_time_overall = (float)(elapsed_tick - item.start_time_) / (float)item.duration_;
			float current_time_in_part = (float)(elapsed_tick - item.start_time_) / (float)item.duration_;

			addChild(item.pitem_, item.order_ - 10000);
			item.pitem_->doEffect(elapsed_tick, current_time_in_part, current_time_overall, part, clip_index, clip_count);
		}
	}
	this->doEffect(currentTimeInPart, currentTimeOverall, part, clip_index, clip_count);
}

void RenderItem::doChild(NXT_HThemeRenderer renderer){


	const char *target[] = {

		"system.video_left",
		"system.video_right",
		"system.video_add_0",
		"system.video_add_1",
		"system.video_add_2",
	};

	std::sort(childlist_.begin(), childlist_.end());

	GLuint src = 0;

	for (int i = 0; i < count_of_childs_; ++i){

		RenderItem* pchild = childlist_[i].pitem_;
		if (pchild->getOutputType() == 1){

			prender_target_[i] = NULL;
			GLuint idx = pchild->loop(renderer);
			broadcastGlobalVariable(target[i], idx);
			src = idx;
		}
		else{

			prender_target_[i] = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 0, 0, 0);
			setRenderTargetAsDefault(renderer, prender_target_[i]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			pchild->loop(renderer);
			broadcastGlobalVariable(target[i], prender_target_[i]->target_texture_);
			src = prender_target_[i]->target_texture_;
		}
	}

	for (int i = count_of_childs_; i < capacity_of_childs_; ++i){

		broadcastGlobalVariable(target[i], 0);
	}

	if (1 == count_of_childs_){
		broadcastGlobalVariable("system.video_src", src);
	}
}

void RenderItem::releaseRenderTarget(NXT_HThemeRenderer renderer){

	for (int i = 0; i < count_of_childs_; ++i){

		NXT_ThemeRenderer_ReleaseRenderTarget(renderer, prender_target_[i]);
	}
}



void RenderItem::addPrefixedChild(RenderItem* pitem, int order, int start, int duration){

	PrefixedChildItem item;
	item.order_ = order;
	item.start_time_ = start;
	item.duration_ = duration;
	item.pitem_ = pitem;

	prefixed_childlist_.push_back(item);
}




void loadObject(rapidxml::xml_node<>* node){

	// //---create RenderItem---
	// char* src_val = getAttribStr(node, "src");
	// char* start_val = getAttribStr(node, "start");
	// char* duration_val = getAttribStr(node, "duration");
	// char* layer_val = getAttribStr(node, "layer");
	// char* child_count_val = getAttribStr(node, "child_count");
	// //-----------------------
	// rapidxml::xml_node<> *image_node = getNode(node, "object", NULL, NULL);

	// while (image_node){

	// 	loadObject(image_node);
	// 	image_node = getNextNode(image_node, "object", NULL, NULL);
	// }
}

bool RenderItem::loadLua(lua_State* L, LuaBin* luabin){

	luaL_openlibs(L);
	registerLuaExtensionFunc(L);
	setDefaultGLDefineToLUA(L);
	
	if (luabin->getSize() > 0){

		luaL_loadbuffer(L, luabin->get(), luabin->getSize(), NULL);
		lua_pcall(L, 0, LUA_MULTRET, 0);

		return true;
	}
	
	return false;
}

void RenderItem::setChildCapacity(int capacity){

	capacity_of_childs_ = capacity;
}

// RenderItem::RenderItem(RenderItem* pitem){

// 	RenderItem& item = *pitem;

// 	prender_item_body_ = pitem;

// 	output_type_ = item.output_type_;
// 	capacity_of_childs_ = item.capacity_of_childs_;
// 	count_of_childs_ = 0;
// 	parent_ = NULL;
// 	childlist_.clear();
// 	prefixed_childlist_.clear();

// 	namespace_ = item.namespace_;
// 	texmap_ = item.texmap_;

// 	effect_type_ = item.effect_type_;

// 	for (glTexmap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

// 		TextureBin* pbin = itor->second;
// 		SAFE_ADDREF(pbin);
// 	}

// 	for (listenermap_t::const_iterator itor = item.listenermap_.begin(); itor != item.listenermap_.end(); ++itor){

// 		const std::string& key = itor->first;

// 		if (0 == key.compare(0, strlen("system."), "system.")){

// 			listenerlist_t inputlist;
// 			const listenerlist_t& list = itor->second;
// 			for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

// 				const SystemVariableListener* plistener = (const SystemVariableListener*)*itor_for_list;
// 				inputlist.push_back(new SystemVariableListener(*plistener));
// 			}

// 			listenermap_.insert(make_pair(key, list));
// 		}
// 		else{

// 			listenerlist_t inputlist;
// 			const listenerlist_t& list = itor->second;
// 			for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

// 				const TextureListener* plistener = (const TextureListener*)*itor_for_list;
// 				inputlist.push_back(new TextureListener(texmap_, *plistener));
// 			}

// 			listenermap_.insert(make_pair(key, list));
// 		}
// 	}

// 	name_ = NULL;
// 	category_title_ = NULL;
// 	icon_ = NULL;
// 	encoded_name_ = NULL;


// 	effect_offset_ = item.effect_offset_;
// 	video_overlap_ = item.video_overlap_;
// 	min_duration_ = item.min_duration_;
// 	max_duration_ = item.max_duration_;
// 	default_duration_ = item.default_duration_;
// 	in_time_ = item.in_time_;
// 	out_time_ = item.out_time_;
// 	in_time_first_ = item.in_time_first_;
// 	out_time_last_ = item.out_time_last_;
// 	cycle_time_ = item.cycle_time_;
// 	max_title_duration_ = item.max_title_duration_;
// 	flag_for_user_duration_ = item.flag_for_user_duration_;
// 	repeat_type_ = item.repeat_type_;
// 	effect_flags_ = item.effect_flags_;

// 	LuaBin* luabin = Singleton<ResourceManager<LuaBin> >::getInstance()->get(namespace_);
// 	luabin->addRef();
// 	lua_state_ = luaL_newstate();

// 	loadLua(lua_state_, luabin);
// 	flag_for_ready_ = true;
// }

RenderItem::RenderItem(RenderItem* pitem):lua_state_(NULL), capacity_of_childs_(0), prender_item_body_(pitem), flag_for_ready_(false), ctx_(NULL){

	if(prender_item_body_)
		namespace_ = prender_item_body_->namespace_;
	output_type_ = 0;
	default_duration_ = 1000;
	min_duration_ = 500;
	max_duration_ = 2000;
	in_time_ = 2000;
	out_time_ = 2000;
	in_time_first_ = 0;
	out_time_last_ = 0;
	cycle_time_ = 2000;
	max_title_duration_ = 0;
	flag_for_user_duration_ = 1;
	repeat_type_ = NXT_RepeatType_None;
	effect_type_ = NXT_EffectType_NONE;
	name_ = NULL;
	category_title_ = NULL;
	icon_ = NULL;
	encoded_name_ = NULL;
	effect_offset_ = 0;
	video_overlap_ = 0;
	
	effect_flags_ = 0;

	lua_state_ = luaL_newstate();
	ctx_ = pitem->ctx_;
}


RenderItem::RenderItem(const RenderItem& item):ctx_(NULL){

	prender_item_body_ = NULL;
	output_type_ = item.output_type_;
	capacity_of_childs_ = item.capacity_of_childs_;
	count_of_childs_ = 0;
	parent_ = NULL;
	childlist_.clear();
	prefixed_childlist_.clear();

	namespace_ = item.namespace_;
	texmap_ = item.texmap_;

	program_map_ = item.program_map_;
	program_idx_map_ = item.program_idx_map_;
	vertex_data_map_ = item.vertex_data_map_;
	texcoord_data_map_ =item.texcoord_data_map_;
	polygon_index_data_map_ = item.polygon_index_data_map_;
	
	valuemap_ = item.valuemap_;

	effect_type_ = item.effect_type_;

	for (AnimatedNodeList_t::const_iterator itor = item.animated_node_list_.begin(); itor != item.animated_node_list_.end(); ++itor){

		NXT_Node_AnimatedValue* pvalue = new NXT_Node_AnimatedValue(*(*itor));
		animated_node_list_.push_back(pvalue);
	}

	for (AnimationMapper_t::const_iterator itor = item.animation_mapper_.begin(); itor != item.animation_mapper_.end(); ++itor){

		AnimationBin* panimation = itor->second;
		animation_mapper_.insert(make_pair(itor->first, panimation));
		panimation->addRef();
	}

	for (glTexmap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

		TextureBin* pbin = itor->second;
		SAFE_ADDREF(pbin);
	}

	for (glProgrammap_t::iterator itor = program_map_.begin(); itor != program_map_.end(); ++itor){

		ProgramBin* pbin = itor->second;
		SAFE_ADDREF(pbin);
	}

	for (listenermap_t::const_iterator itor = item.listenermap_.begin(); itor != item.listenermap_.end(); ++itor){

		const std::string& key = itor->first;

		if (0 == key.compare(0, strlen("system."), "system.")){

			listenerlist_t inputlist;
			const listenerlist_t& list = itor->second;
			for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

				const SystemVariableListener* plistener = (const SystemVariableListener* )*itor_for_list;
				inputlist.push_back(new SystemVariableListener(*plistener));
			}

			listenermap_.insert(make_pair(key, list));
		}
		else{

			listenerlist_t inputlist;
			const listenerlist_t& list = itor->second;
			for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

				const TextureListener* plistener = (const TextureListener*)*itor_for_list;
				inputlist.push_back(new TextureListener(texmap_, *plistener));
			}

			listenermap_.insert(make_pair(key, list));
		}
	}

	name_ = NULL;
	category_title_ = NULL;
	icon_ = NULL;
	encoded_name_ = NULL;


	if (item.name_){

		int len = strlen(item.name_) + 1;
		name_ = (char*)malloc(len);
		strcpy(name_, item.name_);
	}

	if (item.icon_){

		int len = strlen(item.icon_) + 1;
		icon_ = (char*)malloc(len);
		strcpy(icon_, item.icon_);
	}

	if (item.category_title_){

		int len = strlen(item.category_title_) + 1;
		category_title_ = (char*)malloc(len);
		strcpy(category_title_, item.category_title_);
	}

	if (item.encoded_name_){

		int len = strlen(item.encoded_name_) + 1;
		encoded_name_ = (char*)malloc(len);
		strcpy(encoded_name_, item.encoded_name_);
	}

	effect_offset_ = item.effect_offset_;
	video_overlap_ = item.video_overlap_;
	min_duration_ = item.min_duration_;
	max_duration_ = item.max_duration_;
	default_duration_ = item.default_duration_;
	in_time_ = item.in_time_;
	out_time_ = item.out_time_;
	in_time_first_ = item.in_time_first_;
	out_time_last_ = item.out_time_last_;
	cycle_time_ = item.cycle_time_;
	max_title_duration_ = item.max_title_duration_;
	flag_for_user_duration_ = item.flag_for_user_duration_;
	repeat_type_ = item.repeat_type_;
	effect_flags_ = item.effect_flags_;

	LuaBin* luabin = getContext()->lua_manager_.get(namespace_);
	luabin->addRef();
	lua_state_ = luaL_newstate();

	loadLua(lua_state_, luabin);
	flag_for_ready_ = true;
}

const float* RenderItem::getVertex(int id){

	if (prender_item_body_)
		return prender_item_body_->getVertex(id);
	return (&vertex_data_list_[id][1]);
}

const GLushort* RenderItem::getIndices(int id){

	if (prender_item_body_)
		return prender_item_body_->getIndices(id);
	return &polygon_index_data_list_[id][0];
}

const float* RenderItem::getTexcoord(int id){

	if (prender_item_body_)
		return prender_item_body_->getTexcoord(id);
	return &texcoord_data_list_[id][0];
}

const char* RenderItem::getName(){

	return namespace_.c_str();
}

void RenderItem::load_late(){

	if(is_ready())
		return;

	flag_for_ready_ = true;

	if(prender_item_body_){

//		nexSAL_TraceCat(0, 0, "[%s %d] prender_item_body load_late", __func__, __LINE__);
		prender_item_body_->load_late();

		RenderItem& item = *prender_item_body_;
		output_type_ = item.output_type_;
		capacity_of_childs_ = item.capacity_of_childs_;
		count_of_childs_ = 0;
		parent_ = NULL;
		childlist_.clear();
		prefixed_childlist_.clear();

		namespace_ = item.namespace_;
		texmap_ = item.texmap_;

		effect_type_ = item.effect_type_;

		for (glTexmap_t::iterator itor = texmap_.begin(); itor != texmap_.end(); ++itor){

			TextureBin* pbin = itor->second;
			SAFE_ADDREF(pbin);
		}

		for (listenermap_t::const_iterator itor = item.listenermap_.begin(); itor != item.listenermap_.end(); ++itor){

			const std::string& key = itor->first;

			if (0 == key.compare(0, strlen("system."), "system.")){

				listenerlist_t inputlist;
				const listenerlist_t& list = itor->second;
				for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

					const SystemVariableListener* plistener = (const SystemVariableListener*)*itor_for_list;
					inputlist.push_back(new SystemVariableListener(*plistener));
				}

				listenermap_.insert(make_pair(key, list));
			}
			else{

				listenerlist_t inputlist;
				const listenerlist_t& list = itor->second;
				for (listenerlist_t::const_iterator itor_for_list = list.begin(); itor_for_list != list.end(); ++itor_for_list){

					const TextureListener* plistener = (const TextureListener*)*itor_for_list;
					inputlist.push_back(new TextureListener(texmap_, *plistener));
				}

				listenermap_.insert(make_pair(key, list));
			}
		}

		name_ = NULL;
		category_title_ = NULL;
		icon_ = NULL;
		encoded_name_ = NULL;


		effect_offset_ = item.effect_offset_;
		video_overlap_ = item.video_overlap_;
		min_duration_ = item.min_duration_;
		max_duration_ = item.max_duration_;
		default_duration_ = item.default_duration_;
		in_time_ = item.in_time_;
		out_time_ = item.out_time_;
		in_time_first_ = item.in_time_first_;
		out_time_last_ = item.out_time_last_;
		cycle_time_ = item.cycle_time_;
		max_title_duration_ = item.max_title_duration_;
		flag_for_user_duration_ = item.flag_for_user_duration_;
		repeat_type_ = item.repeat_type_;
		effect_flags_ = item.effect_flags_;

		LuaBin* luabin = getContext()->lua_manager_.get(namespace_);
		luabin->addRef();
		// lua_state_ = luaL_newstate();

		loadLua(lua_state_, luabin);

		return;
	}

//	nexSAL_TraceCat(0, 0, "[%s %d] prender_item_body load_late", __func__, __LINE__);

	ResourceManager<LuaBin>& manager = getContext()->lua_manager_;
	LuaBin* luabin = manager.get(namespace_);

	if (NULL == luabin){

		luabin = new LuaBin();
		manager.insert(namespace_, luabin);
	}

	luabin->addRef();
	loadResource(doc_, luabin);

	doc_.clear();
}

int RenderItem::getEffectType(){

	return effect_type_;
}

int RenderItem::getEffectOverlap(){

	return video_overlap_;
}

int RenderItem::getInTime(){

	return in_time_;
}

int RenderItem::getInTimeFirst(){

	return in_time_first_;
}

int RenderItem::getOutTime(){

	return out_time_;
}

int RenderItem::getOutTimeLast(){

	return out_time_last_;
}

int RenderItem::getCycleTime(){

	return cycle_time_;
}

int RenderItem::getRepeatType(){

	return repeat_type_;
}