#include <math.h>
#include "lua_extension_for_renderItem.h"
#include "lua.hpp"
#include "RenderTarget.h"
#include "util.h"
#include "RenderItem.h"
//#include "NexSAL_Internal.h"

class SolidSphere
{
public:
	std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLushort> indices;

    SolidSphere(float radius, unsigned int rings, unsigned int sectors)
    {
        float const R = 1.0f/(float)(rings-1);
        float const S = 1.0f/(float)(sectors-1);
        unsigned int r, s;

        vertices.resize(rings * sectors * 3);
        normals.resize(rings * sectors * 3);
        texcoords.resize(rings * sectors * 2);
        std::vector<GLfloat>::iterator v = vertices.begin();
        std::vector<GLfloat>::iterator n = normals.begin();
        std::vector<GLfloat>::iterator t = texcoords.begin();
        for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
                float const y = (float)sin( -M_PI * 0.5 + M_PI * r * R );
                float const x = (float)(cos(2*M_PI * s * S) * sin( M_PI * r * R ));
                float const z = (float)(sin(2*M_PI * s * S) * sin( M_PI * r * R ));

                *t++ = s*S;
                *t++ = r*R;

                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                *n++ = x;
                *n++ = y;
                *n++ = z;
        }

        indices.resize(rings * sectors * 3 * 2);
        std::vector<GLushort>::iterator i = indices.begin();
        for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
                *i++ = r * sectors + s;
                *i++ = (r+1) * sectors + (s+1);
                *i++ = r * sectors + (s+1);

                *i++ = r * sectors + s;
                *i++ = (r+1) * sectors + s;
                *i++ = (r+1) * sectors + (s+1);
        }
    }
};

static int l_kmSetFlare(lua_State* L){

	glBlendFunc(GL_ONE, GL_ONE);    CHECK_GL_ERROR();
	return 0;
}

static int getRenderItemContext(lua_State* L){

	int render_item_context = 0;
	lua_getglobal(L, "render_item_context");
	render_item_context = (int)lua_tonumber(L, -1);
	return render_item_context;
}

static int l_kmSetBlend(lua_State* L){

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);    CHECK_GL_ERROR();
	return 0;
}

static int l_kmDrawSphere(lua_State* L){

	static SolidSphere* sphere = NULL;
	if(!sphere){

		sphere = new SolidSphere(2.0f, 30, 30);
	}
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, false, 0, &sphere->vertices[0]);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, &sphere->texcoords[0]);  CHECK_GL_ERROR();
	glDrawElements(GL_TRIANGLES, sphere->indices.size(), GL_UNSIGNED_SHORT, &sphere->indices[0]);	CHECK_GL_ERROR();
	return 0;
}

static int l_kmDrawRect(lua_State* L){
	

	float texcoord_buffer[] = 
	{
		0, 1,
		0, 0,
		1, 1,
		1, 0,
		
	};
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	float width = (GLfloat)lua_tonumber(L, 3);
	float height = (GLfloat)lua_tonumber(L, 4);
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;
	
	float position_buffer[] =
	{
		-half_w, half_h, 0, 1,
		-half_w, -half_h, 0, 1,
		half_w, half_h, 0, 1,
		half_w, -half_h, 0, 1,
		
	};

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
	return 0;
}

static int l_kmDrawRectWithPos(lua_State* L){


	float texcoord_buffer[] =
	{
		0, 1,
		0, 0,
		1, 1,
		1, 0,

	};
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	
	float pos_x = (GLfloat)lua_tonumber(L, 3);
	float pos_y = (GLfloat)lua_tonumber(L, 4);
	float width = (GLfloat)lua_tonumber(L, 5);
	float height = (GLfloat)lua_tonumber(L, 6);

	float half_w = width * 0.5f;
	float half_h = height * 0.5f;

	float position_buffer[] =
	{
		-half_w + pos_x, half_h + pos_y, 0, 1,
		-half_w + pos_x, -half_h + pos_y, 0, 1,
		half_w + pos_x, half_h + pos_y, 0, 1,
		half_w + pos_x, -half_h + pos_y, 0, 1,

	};

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
	return 0;
}

static int l_kmSetTexture(lua_State* L){

	int tex_target = (GLuint)lua_tointeger(L, 1);
	int tex_name = (GLuint)lua_tointeger(L, 2);
	int tex_uniform = (GLuint)lua_tointeger(L, 3);

	glActiveTexture(GL_TEXTURE0 + tex_target);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, tex_name);                                   CHECK_GL_ERROR();
	glUniform1i(tex_uniform, tex_target);                                           CHECK_GL_ERROR();

	return 0;
}

static int l_kmSetUniform1f(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLfloat value = (GLfloat)lua_tonumber(L, 2);
	glUniform1f(uniform_idx, value);
	return 0;
}

static int l_kmUseProgram(lua_State* L){

	GLuint program = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_ThemeRenderer_UseProgram(&pctx->theme_renderer_, program);
	return 0;
}

static int l_kmGetTick(lua_State* L){

	lua_pushinteger(L, getTickCount());
	return 1;
}

static int l_kmNewMatrix(lua_State* L){

	//nexSAL_TraceCat(0, 0, "RenderItem [%s %d]", __func__, __LINE__);
	lua_newuserdata(L, sizeof(RITM_NXT_Matrix4f));
	return 1;
}

static int l_kmSetPerspective(lua_State* L){

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*) lua_touserdata(L, 1));
	float fov = (float)lua_tonumber(L, 2);
	float aspect = (float)lua_tonumber(L, 3);
	float near_plane = (float)lua_tonumber(L, 4);
	float far_plane = (float)lua_tonumber(L, 5);
	if (near_plane == 0.0f)
		near_plane = 0.01f;
	matrix = RITM_NXT_Matrix4f_Perspective(fov, aspect, near_plane, far_plane);
	return 0;
}

static int l_kmGetDefault3DMatrix(lua_State* L){

	static bool inited = false;
	static RITM_NXT_Matrix4f default_matrix;
	if (!inited){

		float fov = 45.0f;
		float aspect = 1280.0f / 720.0f;
		float near_plane = 0.01f;
		float far_plane = 3000.0f;
		RITM_NXT_Vector4f axis = { -640.0f, -360.0f, -870.0f, 0.0f };
		default_matrix = RITM_NXT_Matrix4f_MultMatrix(RITM_NXT_Matrix4f_Perspective(fov, aspect, near_plane, far_plane), RITM_NXT_Matrix4f_Scale(1.0f, -1.0f, 1.0f));
		default_matrix = RITM_NXT_Matrix4f_MultMatrix(default_matrix, RITM_NXT_Matrix4f_Translate(axis));
		inited = true;
	}

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_newuserdata(L, sizeof(RITM_NXT_Matrix4f)));
	matrix = default_matrix;
	return 1;
}

static int l_kmSetUniformMat4(lua_State* L){

	GLuint idx = (GLuint)lua_tointeger(L, 1);
	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 2));
	glUniformMatrix4fv(idx, 1, false, matrix.e);
	return 0;
}

static int l_kmRotate(lua_State* L){

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	float radian = (float)(lua_tonumber(L, 2) / 180.0f * M_PI);
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float z = (float)lua_tonumber(L, 5);

	RITM_NXT_Vector4f axis = { x, y, z, 0.0f };
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Rotate(axis, radian));
	return 0;
}

static int l_kmTranslate(lua_State* L){

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	RITM_NXT_Vector4f axis = { x, y, z, 0.0f };
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Translate(axis));
	return 0;
}

static int l_kmScale(lua_State* L){

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	matrix = RITM_NXT_Matrix4f_MultMatrix(matrix, RITM_NXT_Matrix4f_Scale(x, y, z));
	return 0;
}

static int l_kmLoadIdentity(lua_State* L){

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	matrix = RITM_NXT_Matrix4f_Identity();
	return 0;
}

static int l_kmSetUniform2f(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLfloat value0 = (GLfloat)lua_tonumber(L, 2);
	GLfloat value1 = (GLfloat)lua_tonumber(L, 3);
	glUniform2f(uniform_idx, value0, value1);
	return 0;
}

static int l_kmSetUniform3f(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLfloat value0 = (GLfloat)lua_tonumber(L, 2);
	GLfloat value1 = (GLfloat)lua_tonumber(L, 3);
	GLfloat value2 = (GLfloat)lua_tonumber(L, 4);
	glUniform3f(uniform_idx, value0, value1, value2);
	return 0;
}

static int l_kmSetUniform4f(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLfloat value0 = (GLfloat)lua_tonumber(L, 2);
	GLfloat value1 = (GLfloat)lua_tonumber(L, 3);
	GLfloat value2 = (GLfloat)lua_tonumber(L, 4);
	GLfloat value3 = (GLfloat)lua_tonumber(L, 5);
	glUniform4f(uniform_idx, value0, value1, value2, value3);
	return 0;
}

static int l_kmGetTexWidth(lua_State* L){

	int ret = 0;
	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	glTexInfomap_t& mapper = pctx->texinfo_map_;

	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if (itor != mapper.end()){
		
		texInfo& info = itor->second;
		ret = info.width_;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmGetTexHeight(lua_State* L){

	int ret = 0;
	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	glTexInfomap_t& mapper = pctx->texinfo_map_;

	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if (itor != mapper.end()){

		texInfo& info = itor->second;
		ret = info.height_;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmGetSrcWidth(lua_State* L){

	int ret = 0;
	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	glTexInfomap_t& mapper = pctx->texinfo_map_;

	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if (itor != mapper.end()){
		
		texInfo& info = itor->second;
		ret = info.src_width_;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmGetSrcHeight(lua_State* L){

	int ret = 0;
	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	glTexInfomap_t& mapper = pctx->texinfo_map_;

	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if (itor != mapper.end()){

		texInfo& info = itor->second;
		ret = info.src_height_;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmSetRenderToDefault(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	setRenderTarget(renderer, renderer->pdefault_render_target_);
	return 0;
}

static int l_kmSetRenderToTexture(lua_State* L){

	GLuint ret = 0;
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	int width = (int)lua_tointeger(L, 1);
	int height = (int)lua_tointeger(L, 2);
	NXT_RenderTarget* prender_target = NXT_ThemeRenderer_GetRenderTarget(renderer, width, height, 0, 0, 0);
	if (prender_target){
		ret = prender_target->target_texture_;
		renderer->render_target_stack_.push_back(prender_target);
		setRenderTarget(renderer, prender_target);
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmDrawArray(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.pcurrent_render_item_;
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	int data_id = (int)lua_tointeger(L, 3);
	const float* pvertex = prender_item->getVertex(data_id);
	GLuint element_size = (GLuint)lua_tointeger(L, 4);
	int texcoord_data_id = (int)lua_tointeger(L, 5);
	const float* ptexcoord = prender_item->getTexcoord(texcoord_data_id);
	GLuint primitive_type = (GLuint)lua_tointeger(L, 6);
	GLuint primitive_count = (GLuint)lua_tointeger(L, 7);

	if (element_size <= 0)
		element_size = 1;
	
	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, element_size, GL_FLOAT, false, 0, pvertex);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, ptexcoord);  CHECK_GL_ERROR();
	glDrawArrays(primitive_type, 0, primitive_count / element_size);                                      CHECK_GL_ERROR();
	return 0;
}

static int l_kmDrawElement(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.pcurrent_render_item_;
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	int data_id = (int)lua_tointeger(L, 3);
	const float* pvertex = prender_item->getVertex(data_id);
	GLuint element_size = (GLuint)lua_tointeger(L, 4);
	int texcoord_data_id = (int)lua_tointeger(L, 5);
	const float* ptexcoord = prender_item->getTexcoord(texcoord_data_id);
	GLuint primitive_type = (GLuint)lua_tointeger(L, 6);
	GLuint primitive_count = (GLuint)lua_tointeger(L, 7);
	int index_data_id = (int)lua_tointeger(L, 8);
	const GLushort* pindex = prender_item->getIndices(index_data_id);

	if (element_size <= 0)
		element_size = 1;

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, element_size, GL_FLOAT, false, 0, pvertex);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, ptexcoord);  CHECK_GL_ERROR();
	glDrawElements(primitive_type, primitive_count, GL_UNSIGNED_SHORT, pindex);                                      CHECK_GL_ERROR();
	return 0;
}

static int l_kmCopyScrToTexture(lua_State* L){

	GLuint target_texture_id = (GLuint)lua_tointeger(L, 1);
	GLuint start_x = (GLuint)lua_tointeger(L, 2);
	GLuint start_y = (GLuint)lua_tointeger(L, 3);
	GLuint width = (GLuint)lua_tointeger(L, 4);
	GLuint height = (GLuint)lua_tointeger(L, 5);
	glBindTexture(GL_TEXTURE_2D, target_texture_id);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, start_x, start_y, width, height);
	return 0;
}

static int l_kmCreateTexture(lua_State* L){

	GLuint width = (GLuint)lua_tointeger(L, 1);
	GLuint height = (GLuint)lua_tointeger(L, 2);
	
	
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	
	if (tex_id > 0){
		
		texInfo info;
		info.width_ = width;
		info.height_ = height;
		info.src_width_ = width;
		info.src_height_ = height;
		RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
		pctx->texinfo_map_.insert(std::pair<GLuint, texInfo>(tex_id, info));
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	lua_pushinteger(L, tex_id);
	return 1;
}

static int l_kmReleaseTexture(lua_State* L){

	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);	
	glTexInfomap_t& mapper = pctx->texinfo_map_;
	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if(itor == mapper.end())
		return 0;
	glDeleteTextures(1, &tex_id);
	return 0;
}

static int l_kmClear(lua_State* L){

	GLenum bit_mask = (GLenum)lua_tointeger(L, 1);
	glClear(bit_mask);
	return 0;
}

static int l_kmGetAniFrame(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	RenderItem* prender_item = renderer->pcurrent_render_item_;
	const char* ani_name = lua_tostring(L, 1);
	float timing = (float)lua_tonumber(L, 2);

	if (prender_item){

		SnapshotOfAniFrame ret = prender_item->getAniFrame(ani_name, timing);

		lua_newtable(L);

		lua_pushinteger(L, ret.tex_id_);
		lua_setfield(L, -2, "tex_id");
		lua_pushnumber(L, ret.alpha_);
		lua_setfield(L, -2, "alpha");
	}
	return 1;
}

struct func_string{
	lua_CFunction func;
	const char* func_callname;
};

func_string func_strings[] =
{
	{ l_kmUseProgram, "kmUseProgram" },
	{ l_kmSetUniform1f, "kmSetUniform1f" },
	{ l_kmSetTexture, "kmSetTexture" },
	{ l_kmDrawRect, "kmDrawRect" },
	{ l_kmDrawRectWithPos, "kmDrawRectWithPos" },
	{ l_kmDrawSphere, "kmDrawSphere" },
	{ l_kmGetTick, "kmGetTick" },
	{ l_kmNewMatrix, "kmNewMatrix" },
	{ l_kmSetPerspective, "kmSetPerspective" },
	{ l_kmGetDefault3DMatrix, "kmGetDefault3DMatrix" },
	{ l_kmSetUniformMat4, "kmSetUniformMat4" },
	{ l_kmRotate, "kmRotate" },
	{ l_kmTranslate, "kmTranslate" },
	{ l_kmScale, "kmScale" },
	{ l_kmLoadIdentity, "kmLoadIdentity" },
	{ l_kmSetUniform2f, "kmSetUniform2f" },
	{ l_kmSetUniform3f, "kmSetUniform3f" },
	{ l_kmSetUniform4f, "kmSetUniform4f" },
	{ l_kmGetTexWidth, "kmGetTexWidth" },
	{ l_kmGetTexHeight, "kmGetTexHeight" },
	{ l_kmGetSrcWidth, "kmGetSrcWidth" },
	{ l_kmGetSrcHeight, "kmGetSrcHeight" },
	{ l_kmSetRenderToDefault, "kmSetRenderToDefault" },
	{ l_kmSetRenderToTexture, "kmSetRenderToTexture" },
	{ l_kmDrawArray, "kmDrawArray" },
	{ l_kmDrawElement, "kmDrawElement" },
	{ l_kmCopyScrToTexture, "kmCopyScrToTexture" },
	{ l_kmCreateTexture, "kmCreateTexture" },
	{ l_kmReleaseTexture, "kmReleaseTexture" },
	{ l_kmClear, "kmClear" },
	{ l_kmSetBlend, "kmSetBlend" },
	{ l_kmSetFlare, "kmSetFlare" },
	{ l_kmGetAniFrame, "kmGetAniFrame" },
};

void registerLuaExtensionFunc(lua_State* L){

	for (int i = 0; i < sizeof(func_strings) / sizeof(func_string); ++i){
		lua_pushcfunction(L, func_strings[i].func);
		lua_setglobal(L, func_strings[i].func_callname);
	}
}

void setDefaultGLDefineToLUA(lua_State* L){

	lua_pushinteger(L, GL_TRIANGLES);
	lua_setglobal(L, "GL_TRIANGLES");
	lua_pushinteger(L, GL_TRIANGLE_STRIP);
	lua_setglobal(L, "GL_TRIANGLE_STRIP");
	lua_pushinteger(L, GL_TRIANGLE_STRIP);
	lua_setglobal(L, "GL_TRIANGLE_FAN");
	lua_pushinteger(L, GL_COLOR_BUFFER_BIT);
	lua_setglobal(L, "GL_COLOR_BUFFER_BIT");
	lua_pushinteger(L, GL_DEPTH_BUFFER_BIT);
	lua_setglobal(L, "GL_DEPTH_BUFFER_BIT");
}