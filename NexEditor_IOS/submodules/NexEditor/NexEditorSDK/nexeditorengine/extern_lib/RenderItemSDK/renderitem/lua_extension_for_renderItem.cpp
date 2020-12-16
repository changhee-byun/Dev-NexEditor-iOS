#include <math.h>
#include "lua_extension_for_renderItem.h"
#include "lua.hpp"
#include "RenderTarget.h"
#include "util.h"
#include "RenderItem.h"
#include "NexSAL_Internal.h"
#include "nexTexturesLogger.h"

#define LOG_TAG	"RenderItem"
#ifdef ANDROID
#include <android/log.h>
#define LOGX(...)	0
#elif defined(__APPLE__)
#define LOGX(...)    0
#endif

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

typedef struct NumArray{

    int size;
    float values[1];
} NumArray;

static int newarray(lua_State* L){

    int n = luaL_checkinteger(L, 1);
    size_t nbytes = sizeof(NumArray) + (n-1)*sizeof(float);
    NumArray* a = (NumArray*)lua_newuserdata(L, nbytes);

    luaL_getmetatable(L, "nexstreaming.array");
    lua_setmetatable(L, -2);

    a->size = n;
    return 1;
}

static NumArray *checkarray (lua_State *L) {

    void *ud = luaL_checkudata(L, 1, "nexstreaming.array");
    luaL_argcheck(L, ud != NULL, 1, "`array' expected");
    return (NumArray *)ud;
}

static float *getelem (lua_State *L) {

    NumArray *a = checkarray(L);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                    "index out of range");

    return &a->values[index - 1];
}

static int setarray(lua_State* L){

    float newvalue = luaL_checknumber(L, 3);
    *getelem(L) = newvalue;
    return 0;
}

static int getarray (lua_State *L) {
    
    lua_pushnumber(L, *getelem(L));
    return 1;
}

static int getsize(lua_State* L){

    NumArray* a = checkarray(L);
    luaL_argcheck(L, a != NULL, 1, "'array' expected");
    lua_pushnumber(L, a->size);
    return 1;
}

static const struct luaL_Reg arraylib[] = {

    {"set", setarray},
    {"get", getarray},
    {"size", getsize},
    {NULL, NULL}
};

static const struct luaL_Reg arrayFunc[] = {

    {"new", newarray},
    {NULL, NULL}
};

int luaopen_array(lua_State* L){

    luaL_newlib(L, arrayFunc);
    luaL_newmetatable(L, "nexstreaming.array");
    lua_pushvalue(L, -1);  /* push metatable */
    lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
    luaL_setfuncs(L, arraylib, 0);  /* add file methods to new metatable */
    lua_pop(L, 1);
    return 1;
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

static int l_kmDrawSolid(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	float* pmatrix = (float*)lua_touserdata(L, 1);
	float left = (float)lua_tonumber(L, 2);
	float top = (float)lua_tonumber(L, 3);
	float right = (float)lua_tonumber(L, 4);
	float bottom = (float)lua_tonumber(L, 5);

	pctx->theme_renderer_.drawSolid(pmatrix, left, top, right, bottom);
	return 0;
}

static int l_kmDrawTexBox(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	int tex = lua_tointeger(L, 1);
	float* pmatrix = (float*)lua_touserdata(L, 2);
	float left = (float)lua_tonumber(L, 3);
	float top = (float)lua_tonumber(L, 4);
	float right = (float)lua_tonumber(L, 5);
	float bottom = (float)lua_tonumber(L, 6);
	int nofilter = (int)lua_tonumber(L, 7);
	int color_reverse = (int)lua_tonumber(L, 8);
	pctx->theme_renderer_.drawTexBox(tex, false, pmatrix, left, top, right, bottom, 1.0f, nofilter, color_reverse);
	return 0;
}

static int l_kmDrawRect(lua_State* L){
	

	float texcoord_buffer[] = 
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,
		
	};
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	float width = (GLfloat)lua_tonumber(L, 3);
	float height = (GLfloat)lua_tonumber(L, 4);
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;
	
	float position_buffer[] =
	{
		-half_w, -half_h, 0, 1,
		-half_w, half_h, 0, 1,
		half_w, -half_h, 0, 1,
		half_w, half_h, 0, 1,
		
	};

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();
	return 0;
}

static int l_kmDrawBound(lua_State* L){
	

	float texcoord_buffer[] = 
	{
		0, 1,
		0, 0,
		1, 1,
		1, 0,
		
	};
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 1);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 2);
	float left = (GLfloat)lua_tonumber(L, 3);
	float top = (GLfloat)lua_tonumber(L, 4);
	float right = (GLfloat)lua_tonumber(L, 5);
	float bottom = (GLfloat)lua_tonumber(L, 6);
	
	float position_buffer[] =
	{
		left, bottom, 0, 1,
		left, top, 0, 1,
		right, bottom, 0, 1,
		right, top, 0, 1,
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

static int l_kmSetTextureExternal(lua_State* L){
#ifndef __APPLE__
	int tex_target = (GLuint)lua_tointeger(L, 1);
	int tex_name = (GLuint)lua_tointeger(L, 2);
	int tex_uniform = (GLuint)lua_tointeger(L, 3);

	glActiveTexture(GL_TEXTURE0 + tex_target);                                             CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex_name);                                   CHECK_GL_ERROR();
	glUniform1i(tex_uniform, tex_target);                                           CHECK_GL_ERROR();
#endif
	return 0;
}

static int l_kmSetUniform1i(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLint value = (GLint)lua_tonumber(L, 2);
	glUniform1i(uniform_idx, value);
	return 0;
}

static int l_kmSetUniform1f(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
	GLfloat value = (GLfloat)lua_tonumber(L, 2);
	glUniform1f(uniform_idx, value);
	return 0;
}

static int l_kmSetUniform1fv(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
    int size = (int)lua_tointeger(L, 2);
    NumArray* val = (NumArray*)lua_touserdata(L, 3);
	glUniform1fv(uniform_idx, size, val->values);
	return 0;
}

static int l_kmSetUniform2fv(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
    int size = (int)lua_tointeger(L, 2);
    NumArray* val = (NumArray*)lua_touserdata(L, 3);
	glUniform2fv(uniform_idx, size, val->values);
	return 0;
}

static int l_kmSetUniform3fv(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
    int size = (int)lua_tointeger(L, 2);
    NumArray* val = (NumArray*)lua_touserdata(L, 3);
	glUniform3fv(uniform_idx, size, val->values);
	return 0;
}

static int l_kmSetUniform4fv(lua_State* L){

	GLuint uniform_idx = (GLuint)lua_tointeger(L, 1);
    int size = (int)lua_tointeger(L, 2);
    NumArray* val = (NumArray*)lua_touserdata(L, 3);
	glUniform4fv(uniform_idx, size, val->values);
	return 0;
}

static int l_kmUseProgram(lua_State* L){

	GLuint program = (GLuint)lua_tointeger(L, 1);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	pctx->theme_renderer_.useProgram(program);
	return 0;
}

static int l_kmGetTick(lua_State* L){

	lua_pushinteger(L, getTickCount());
	return 1;
}

static int l_kmNewMatrix(lua_State* L){

	// nexSAL_TraceCat(0, 0, "[%s %d]", __func__, __LINE__);
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

static float getFov(float aspect_ratio){

	static float cmp_aspect_ratio = 0.0f;
	static float fov = 0.0f;
	if(aspect_ratio != cmp_aspect_ratio){

		cmp_aspect_ratio = aspect_ratio;
		fov = 2.0f * atan(1.0f / aspect_ratio * tan(PI_OVER_180 * 22.5)) / PI_OVER_180;
	}
	return fov;
}


static float getAspectRatio(RenderItemContext* pctx){

	float cur_aspect_ratio = 0.0f;
	float width;
	float height;
	NXT_RenderTarget* prender_target = pctx->theme_renderer_.getDefaultRenderTarget();
	if(prender_target){

		width = float(prender_target->width_);
		height = float(prender_target->height_);
	}
	else{

		width = float(pctx->theme_renderer_.view_width);
		height = float(pctx->theme_renderer_.view_height);
	}

	cur_aspect_ratio = width / height;

	return cur_aspect_ratio;
}

static int l_kmGetDefault3DMatrix(lua_State* L){

	static bool inited = false;
	static RITM_NXT_Matrix4f default_matrix;
	static float aspect_ratio = 1280.0f / 720.0f;

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	float cur_aspect_ratio = 0.0f;
	float width;
	float height;
	NXT_RenderTarget* prender_target = pctx->theme_renderer_.getDefaultRenderTarget();
	if(prender_target){

		width = float(prender_target->width_);
		height = float(prender_target->height_);
	}
	else{

		width = float(pctx->theme_renderer_.view_width);
		height = float(pctx->theme_renderer_.view_height);
	}

	cur_aspect_ratio = width / height;

	if (!inited || (aspect_ratio != cur_aspect_ratio)){

		aspect_ratio = cur_aspect_ratio;
		float fov = 45.0f;
		float near_plane = 0.01f;
		float far_plane = 3000.0f;

		default_matrix = RITM_NXT_Matrix4f_MultMatrix(RITM_NXT_Matrix4f_Perspective(fov, aspect_ratio, near_plane, far_plane), RITM_NXT_Matrix4f_Scale(1.0f, -1.0f, 1.0f));
		RITM_NXT_Vector4f axis = { -width * 0.5f, -height * 0.5f, -width * 0.5f * default_matrix.e[0], 0.0f };
		default_matrix = RITM_NXT_Matrix4f_MultMatrix(default_matrix, RITM_NXT_Matrix4f_Translate(axis));
		inited = true;
	}

	RITM_NXT_Matrix4f& matrix = *((RITM_NXT_Matrix4f*)lua_newuserdata(L, sizeof(RITM_NXT_Matrix4f)));
	matrix = default_matrix;
	return 1;
}

static int l_kmSetUniformMat4(lua_State* L){

	GLuint idx = (GLuint)lua_tointeger(L, 1);
	RITM_NXT_Matrix4f* pmatrix = (RITM_NXT_Matrix4f*)lua_touserdata(L, 2);
	
	if(pmatrix == NULL){

		glUniformMatrix4fv(idx, 1, false, RITM_NXT_Matrix4f_Identity().e);
	}
	else{
		
		glUniformMatrix4fv(idx, 1, false, pmatrix->e);
	}
	return 0;
}

static int l_kmRotate(lua_State* L){

	RITM_NXT_Matrix4f& matrix_dest = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	RITM_NXT_Matrix4f& matrix_src = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 2));
	float radian = (float)(lua_tonumber(L, 3) / 180.0f * M_PI);
	float x = (float)lua_tonumber(L, 4);
	float y = (float)lua_tonumber(L, 5);
	float z = (float)lua_tonumber(L, 6);

	RITM_NXT_Vector4f axis = { x, y, z, 0.0f };
	matrix_dest = RITM_NXT_Matrix4f_MultMatrix(matrix_src, RITM_NXT_Matrix4f_Rotate(axis, radian));
	return 0;
}

static int l_kmTranslate(lua_State* L){

	RITM_NXT_Matrix4f& matrix_dest = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	RITM_NXT_Matrix4f& matrix_src = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 2));
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float z = (float)lua_tonumber(L, 5);
	RITM_NXT_Vector4f axis = { x, y, z, 0.0f };
	matrix_dest = RITM_NXT_Matrix4f_MultMatrix(matrix_src, RITM_NXT_Matrix4f_Translate(axis));
	return 0;
}

static int l_kmScale(lua_State* L){

	RITM_NXT_Matrix4f& matrix_dest = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 1));
	RITM_NXT_Matrix4f& matrix_src = *((RITM_NXT_Matrix4f*)lua_touserdata(L, 2));
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float z = (float)lua_tonumber(L, 5);
	// nexSAL_TraceCat(0,0,"%s %f %f %f", __func__, x, y, z);
	matrix_dest = RITM_NXT_Matrix4f_MultMatrix(matrix_src, RITM_NXT_Matrix4f_Scale(x, y, z));
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
	pctx->theme_renderer_.resetToDefaultRenderTarget();
	return 0;
}

enum{

	RTT_REQUEST_DEPTH = 0x01,
	RTT_REQUEST_STENCIL = 0x02,
	RTT_REQUEST_ALPHA = 0x04
};

static int l_kmSetRenderToTexture(lua_State* L){

	GLuint ret = 0;
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	float width = lua_tonumber(L, 1);
	float height = lua_tonumber(L, 2);
	int request = lua_tointeger(L, 3);

	if(width < 2.0f)
		width = 2.0f;
	if(height < 2.0f)
		height = 2.0f;

	int conv_w = floor(width);
	int conv_h = floor(height);
	
	NXT_RenderTarget* prender_target = renderer->getRenderTarget(conv_w, conv_h, request&RTT_REQUEST_ALPHA, request&RTT_REQUEST_DEPTH, request&RTT_REQUEST_STENCIL);
	if (prender_target){

		ret = prender_target->target_texture_;
		renderer->pushRenderTarget(prender_target);
		renderer->setRenderTarget(prender_target);
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmSetRenderToClearTexture(lua_State* L){

	GLuint ret = 0;
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	float width = lua_tonumber(L, 1);
	float height = lua_tonumber(L, 2);
	int request = lua_tointeger(L, 3);

	if(width < 2.0f)
		width = 2.0f;
	if(height < 2.0f)
		height = 2.0f;

	int conv_w = floor(width);
	int conv_h = floor(height);
	
	NXT_RenderTarget* prender_target = renderer->getRenderTarget(conv_w, conv_h, request&RTT_REQUEST_ALPHA, request&RTT_REQUEST_DEPTH, request&RTT_REQUEST_STENCIL);
	if (prender_target){

		ret = prender_target->target_texture_;
		renderer->pushRenderTarget(prender_target);
		renderer->setRenderTarget(prender_target);

		if(request & RTT_REQUEST_ALPHA)
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		else
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmDrawText(lua_State* L){

	

	char prefix[1024];
        
    prefix[sizeof(prefix)-1]=0;     // snprintf doesn't null-terminate if the length is exceeded; this ensures null termination
    
	const char*         typeface;
    float               textSkewX;
    float               textScaleX;
    float               textSize;
    float               textStrokeWidth;
    float               spacingMult;
    float               spacingAdd;
    unsigned int        textFlags;
    float               textAlign;
    unsigned int        textVAlign;
    unsigned int        longText;
    unsigned int		blurType;
	unsigned int		shadowblurType;
    unsigned int        maxLines;
    float               shadowRadius;
    float               textBlurRadius;
    float               textMargin;
    RITM_NXT_Vector4f   fillColor;
    RITM_NXT_Vector4f   strokeColor;
    RITM_NXT_Vector4f   shadowColor;
    RITM_NXT_Vector4f   bgColor;    
    RITM_NXT_Vector4f   shadowOffset;
    int                 reqWidth;
    int                 reqHeight;
    char*				text;
    const char*			titleText;

    int index = 0;
    int stack_depth = 0;

    RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();

	lua_settop(L, 1);
	luaL_checktype (L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "typeface");index--;
	lua_getfield(L, 1, "textSkewX");index--;
	lua_getfield(L, 1, "textScaleX");index--;
	lua_getfield(L, 1, "textSize");index--;
	lua_getfield(L, 1, "textStrokeWidth");index--;
	lua_getfield(L, 1, "spacingMult");index--;
	lua_getfield(L, 1, "spacingAdd");index--;
	lua_getfield(L, 1, "textFlags");index--;
	lua_getfield(L, 1, "textAlign");index--;
	lua_getfield(L, 1, "textVAlign");index--;
	lua_getfield(L, 1, "longText");index--;
	lua_getfield(L, 1, "blurType");index--;
	lua_getfield(L, 1, "shadowblurType");index--;
	lua_getfield(L, 1, "maxLines");index--;
	lua_getfield(L, 1, "shadowRadius");index--;
	lua_getfield(L, 1, "textBlurRadius");index--;
	lua_getfield(L, 1, "textMargin");index--;
	lua_getfield(L, 1, "fillColorX");index--;
	lua_getfield(L, 1, "fillColorY");index--;
	lua_getfield(L, 1, "fillColorZ");index--;
	lua_getfield(L, 1, "fillColorW");index--;
	lua_getfield(L, 1, "strokeColorX");index--;
	lua_getfield(L, 1, "strokeColorY");index--;	
	lua_getfield(L, 1, "strokeColorZ");index--;	
	lua_getfield(L, 1, "strokeColorW");index--;	
	lua_getfield(L, 1, "shadowColorX");index--;	
	lua_getfield(L, 1, "shadowColorY");index--;	
	lua_getfield(L, 1, "shadowColorZ");index--;	
	lua_getfield(L, 1, "shadowColorW");index--;	
	lua_getfield(L, 1, "bgColorX");index--;
	lua_getfield(L, 1, "bgColorY");index--;
	lua_getfield(L, 1, "bgColorZ");index--;
	lua_getfield(L, 1, "bgColorW");index--;
	lua_getfield(L, 1, "shadowOffsetX");index--;
	lua_getfield(L, 1, "shadowOffsetY");index--;
	lua_getfield(L, 1, "shadowOffsetZ");index--;
	lua_getfield(L, 1, "shadowOffsetW");index--;
	lua_getfield(L, 1, "reqWidth");index--;
	lua_getfield(L, 1, "reqHeight");index--;
	lua_getfield(L, 1, "text");index--;
	lua_getfield(L, 1, "titleText");index--;
	stack_depth = -index;
    typeface 			= luaL_optlstring(L, index++, "typeface", NULL);
	textSkewX 			= luaL_optnumber(L, index++, 0.0f);
	textScaleX 			= luaL_optnumber(L, index++, 0.0f);
	textSize 			= luaL_optnumber(L, index++, 0.0f);
	textStrokeWidth 	= luaL_optnumber(L, index++, 0.0f);
	spacingMult 		= luaL_optnumber(L, index++, 0.0f);
	spacingAdd 			= luaL_optnumber(L, index++, 0.0f);
	textFlags 			= luaL_optinteger(L, index++, 0);
	textAlign 			= luaL_optnumber(L, index++, 0.0f);
	textVAlign 			= luaL_optinteger(L, index++, 0);
	longText			= luaL_optinteger(L, index++, 0);
	blurType			= luaL_optinteger(L, index++, 0);
	shadowblurType			= luaL_optinteger(L, index++, 0);
	maxLines			= luaL_optinteger(L, index++, 0);
	shadowRadius 		= luaL_optnumber(L, index++, 0.0f);
	textBlurRadius 		= luaL_optnumber(L, index++, 0.0f);
	textMargin 			= luaL_optnumber(L, index++, 0.0f);

    fillColor.e[0] 		= luaL_optnumber(L, index++, 0.0f);
    fillColor.e[1] 		= luaL_optnumber(L, index++, 0.0f);
    fillColor.e[2] 		= luaL_optnumber(L, index++, 0.0f);
    fillColor.e[3] 		= luaL_optnumber(L, index++, 0.0f);

    strokeColor.e[0] 	= luaL_optnumber(L, index++, 0.0f);
    strokeColor.e[1] 	= luaL_optnumber(L, index++, 0.0f);
    strokeColor.e[2] 	= luaL_optnumber(L, index++, 0.0f);
    strokeColor.e[3] 	= luaL_optnumber(L, index++, 0.0f);

    shadowColor.e[0] 	= luaL_optnumber(L, index++, 0.0f);
    shadowColor.e[1] 	= luaL_optnumber(L, index++, 0.0f);
    shadowColor.e[2] 	= luaL_optnumber(L, index++, 0.0f);
    shadowColor.e[3] 	= luaL_optnumber(L, index++, 0.0f);

    bgColor.e[0] 		= luaL_optnumber(L, index++, 0.0f);
    bgColor.e[1] 		= luaL_optnumber(L, index++, 0.0f);
    bgColor.e[2] 		= luaL_optnumber(L, index++, 0.0f);
    bgColor.e[3] 		= luaL_optnumber(L, index++, 0.0f);

    shadowOffset.e[0] 	= luaL_optnumber(L, index++, 0.0f);
    shadowOffset.e[1] 	= luaL_optnumber(L, index++, 0.0f);
    shadowOffset.e[2] 	= luaL_optnumber(L, index++, 0.0f);
    shadowOffset.e[3] 	= luaL_optnumber(L, index++, 0.0f);

    reqWidth			= luaL_optinteger(L, index++, 0);
    reqHeight			= luaL_optinteger(L, index++, 0);
    text 				= (char*)luaL_optlstring(L, index++, "text", NULL);
    titleText 			= luaL_optlstring(L, index++, "titleText", NULL);

    char* converted_text = NULL;

    {
        static const char *hexchars = "012345678ABCDEF";
        static const char *needEsc = ";=%'\"";
        char *s;
        const char *e;
        char *t;
        
        int rqlen = 1;
        for( s=text; *s; s++ ) {
            rqlen++;
            for( e=needEsc; *e; e++ ) {
                if( *e == *s ) {
                    rqlen++;
                    break;
                }
            }
        }
//        nodeData->text = malloc(strlen(attrValue)+1);
        converted_text = (char*)malloc(rqlen);
        for( s=text, t=converted_text; *s; s++, t++ ) {
            rqlen++;
            *t = *s;
            for( e=needEsc; *e; e++ ) {
                if( *e == *s ) {
                    *t = '%';
                    t++;
                    *t = hexchars[((*s)>>4)&0xF];
                    t++;
                    *t = hexchars[(*s)&0xF];
                    break;
                }
            }
        }
        *t = 0;
    }

    int tex_id = 0;//todo get texture for text
    // nexSAL_TraceCat(0, 0, "[%s %d] text:%s typeface:%s textScaleX:%f prender_item:0x%x", __func__, __LINE__, text, typeface, textScaleX, prender_item);

    snprintf(prefix, sizeof(prefix)-1, 
             "[Text]baseid=%s;typeface=%s;skewx=%010.4f;scalex=%010.4f;spacingmult=%010.4f;spacingadd=%010.4f;size=%010.4f;"
             "strokewidth=%010.4f;shadowradius=%010.4f;textblur=%010.4f;blurtype=%i;shadowblurtype=%i;margin=%i;flags=%08X;align=%i;longtext=%i;maxlines=%i;"
             "fillcolor=%08X;strokecolor=%08X;shadowcolor=%08X;bgcolor=%08X;"
             "shadowoffsx=%010.4f;shadowoffsy=%010.4f;"
             "width=%d;height=%d;text=%s;;",
             prender_item->getName(),
             typeface,
             textSkewX,
             textScaleX,
             spacingMult,
             spacingAdd,
             textSize,
             textStrokeWidth,
             shadowRadius,
             textBlurRadius,
             blurType,
			 shadowblurType,
             (int)textMargin,
             textFlags,
             (((int)textAlign)&0xF) | (textVAlign<<4),
             longText,
             maxLines,
             RITM_NXT_Vector4f_ToARGBIntColor255(fillColor),
             RITM_NXT_Vector4f_ToARGBIntColor255(strokeColor),
             RITM_NXT_Vector4f_ToARGBIntColor255(shadowColor),
             RITM_NXT_Vector4f_ToARGBIntColor255(bgColor),
             shadowOffset.e[0],
             shadowOffset.e[1],
             reqWidth,
             reqHeight,
             converted_text);
    
    // nexSAL_TraceCat(0, 0, "[%s %d] %s textScaleX:%f", __func__, __LINE__, converted_text, textScaleX);
    
    if( !titleText )
        titleText="";
    
    int titlelen = (int)strlen(titleText);
    int prefixlen = (int)strlen(prefix);
    char* query = (char*)malloc(prefixlen + titlelen + 1);
    strcpy(query, prefix);
    strcpy(query + prefixlen, titleText);

    // nexSAL_TraceCat(0, 0, "[%s %d] titleText:%s", __func__, __LINE__, titleText);

    ResourceManager<TextureBin>& manager = pctx->texture_manager_;
	TextureBin* image = manager.get(std::string(query));

	if(NULL == image){

		NXT_ImageInfo imgInfo;//ToDo
    
	    int result = (*pctx->file_ops_.ploadimagefunc)(&imgInfo,
	                                         query,
	                                         0,
	                                         pctx->file_ops_.cbdata_for_loadfunc);
	    
	    if( result != 0 ) {
	        //Error
	        //ToDo
	    }
	    

	    image = new TextureBin();
	    tex_id = Img::loadImageRaw((char*)imgInfo.pixels, imgInfo.width, imgInfo.height);
	    // nexSAL_TraceCat(0, 0, "[%s %d] result:%d imgInfo.width:%d imgInfo.height:%d tex_id:%d", __func__, __LINE__, result, imgInfo.width, imgInfo.height, tex_id);

		image->setTexID(tex_id);
		image->setWidth(imgInfo.width);
		image->setHeight(imgInfo.height);

		manager.insert(std::string(query), image);

	    texInfo info;
		info.width_ = imgInfo.width;
		info.height_ = imgInfo.height;
		info.src_width_ = imgInfo.width;
		info.src_height_ = imgInfo.height;
		pctx->texinfo_map_.insert(std::pair<GLuint, texInfo>(tex_id, info));
		(*pctx->file_ops_.pfreeimagefunc)(&imgInfo, pctx->file_ops_.cbdata_for_loadfunc);
	}
	else{

		tex_id = image->getTexID();
	}
    free(converted_text);
    free(query);

    lua_pop(L, stack_depth);

    lua_pushinteger(L, tex_id);

    return 1;
}

static int l_kmGetImage(lua_State* L){
    
    RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	const char* query = lua_tostring(L, 1);

    int tex_id = 0;

    ResourceManager<TextureBin>& manager = pctx->texture_manager_;
	TextureBin* image = manager.get(std::string(query));

	if(NULL == image){

		NXT_ImageInfo imgInfo;//ToDo
    
	    int result = (*pctx->file_ops_.ploadimagefunc)(&imgInfo,
	                                         (char*)query,
	                                         0,
	                                         pctx->file_ops_.cbdata_for_loadfunc);
	    
	    if( result != 0 ) {
	        //Error
	        //ToDo
	    }
	    

	    image = new TextureBin();
	    tex_id = Img::loadImageRaw((char*)imgInfo.pixels, imgInfo.width, imgInfo.height);
	    // nexSAL_TraceCat(0, 0, "[%s %d] result:%d imgInfo.width:%d imgInfo.height:%d tex_id:%d", __func__, __LINE__, result, imgInfo.width, imgInfo.height, tex_id);

		image->setTexID(tex_id);
		image->setWidth(imgInfo.width);
		image->setHeight(imgInfo.height);

		manager.insert(std::string(query), image);

	    texInfo info;
		info.width_ = imgInfo.width;
		info.height_ = imgInfo.height;
		info.src_width_ = imgInfo.width;
		info.src_height_ = imgInfo.height;
		pctx->texinfo_map_.insert(std::pair<GLuint, texInfo>(tex_id, info));
		(*pctx->file_ops_.pfreeimagefunc)(&imgInfo, pctx->file_ops_.cbdata_for_loadfunc);
	}
	else{

		tex_id = image->getTexID();
	}
    lua_pushinteger(L, tex_id);

    return 1;
}

static MeshGroup* getMeshGroup_internal(RenderItem* prender_item, int id){

    MeshGroupBin* pbin = prender_item->getMeshGroupBin(id);
	MeshGroup* pmesh = pbin->get();

    if(!pmesh){
        
        for(int i = 0; i < 10; ++i){

            nexSAL_TaskSleep(100);
            pmesh = pbin->get();
            if(pmesh)
                break;
        }
    }
    return  pmesh;
}

static int l_kmDrawNexEffect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();

	float cur_aspect_ratio = getAspectRatio(pctx);
	float fov = getFov(cur_aspect_ratio);
	
	int id = lua_tointeger(L, 1);
	int video_src_0 = lua_tointeger(L, 2);
	float* pmatrix_for_video_src_0 = (float*)lua_touserdata(L, 3);
	int video_src_1 = lua_tointeger(L, 4);
	float* pmatrix_for_video_src_1 = (float*)lua_touserdata(L, 5);
	float frame = lua_tonumber(L, 6);

	MeshGroup* pmesh = getMeshGroup_internal(prender_item, id);
    if(!pmesh)
        return 0;
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//AT PASS
	pmesh->draw(video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, frame, 0, cur_aspect_ratio, fov);

	glDepthMask(GL_FALSE);
	//AB PASS
	pmesh->draw(video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, frame, 1, cur_aspect_ratio, fov);

	//Flare Pass
	glBlendFunc(GL_ONE, GL_ONE);
	pmesh->draw(video_src_0, pmatrix_for_video_src_0, video_src_1, pmatrix_for_video_src_1, frame, 2, cur_aspect_ratio, fov);


	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	return 0;
}

static int l_kmDrawNexEffectEasy(lua_State* L){

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	float cur_aspect_ratio = getAspectRatio(pctx);
	float fov = getFov(cur_aspect_ratio);
	int id = lua_tointeger(L, 1);
	float frame = lua_tonumber(L, 2);
	int reverse = lua_tointeger(L, 3);



	MeshGroup* pmesh = getMeshGroup_internal(prender_item, id);
    if(!pmesh)
        return 0;

	pmesh->drawFull(pctx, frame, reverse, cur_aspect_ratio, fov);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	return 0;
}

static int l_kmGetFrameNexEffect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int id = lua_tointeger(L, 1);

	MeshGroup* pmesh = getMeshGroup_internal(prender_item, id);
    if(!pmesh){

        lua_pushinteger(L, 1);
	    return 1;
    }
    
	int frame = pmesh->getFrame();
	lua_pushinteger(L, frame);
	return 1;
}


static int l_kmDrawArray(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
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
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
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
	GL_GenTextures(1, &tex_id);
	
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

static int l_kmGetScreenTexture(lua_State* L){

	float* pmatrix = (float*)lua_touserdata(L, 1);
	if(pmatrix == NULL){

		lua_pushinteger(L, 0);
		return 1;
	}
	float left = lua_tonumber(L, 2);
	float top = lua_tonumber(L, 3);
	float right = lua_tonumber(L, 4);
	float bottom = lua_tonumber(L, 5);
	float feather = lua_tonumber(L, 6);

	RITM_NXT_Vector4f result[4], lefttop, righttop, leftbottom, rightbottom;

	lefttop.e[0] = left;
	lefttop.e[1] = top;
	lefttop.e[2] = 0.0f;
	lefttop.e[3] = 1.0f; 

	righttop.e[0] = right;
	righttop.e[1] = top;
	righttop.e[2] = 0.0f;
	righttop.e[3] = 1.0f;

	leftbottom.e[0] = left;
	leftbottom.e[1] = bottom;
	leftbottom.e[2] = 0.0f;
	leftbottom.e[3] = 1.0f; 

	rightbottom.e[0] = right;
	rightbottom.e[1] = bottom;
	rightbottom.e[2] = 0.0f;
	rightbottom.e[3] = 1.0f;

	result[0] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), lefttop);
	result[1] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), righttop);
	result[2] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), leftbottom);
	result[3] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), rightbottom);


	float min_x = 1000000000.0f;
	float min_y = 1000000000.0f;
	float max_x = -1000000000.0f;
	float max_y = -1000000000.0f;

	for(int i = 0; i < 4; ++i){

		float x = result[i].e[0] /= result[i].e[3];
		float y = result[i].e[1] /= result[i].e[3];

		if(x < min_x)
			min_x = x;
		if(x > max_x)
			max_x = x;

		if(y < min_y)
			min_y = y;
		if(y > max_y)
			max_y = y;
	}

	min_x -= feather;
	min_y -= feather;
	max_x += feather;
	max_y += feather;

	if(min_x < -1.0f)
		min_x = -1.0f;
	if(min_y < -1.0f)
		min_y = -1.0f;
	if(max_x > 1.0f)
		max_x = 1.0f;
	if(max_y > 1.0f)
		max_y = 1.0f;


	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);

	float half_w = (float)(pctx->theme_renderer_.view_width / 2);
	float half_h = (float)(pctx->theme_renderer_.view_height / 2);

	GLuint start_x = (int)(min_x * half_w + half_w);
	GLuint end_y = (int)(max_y * half_h + half_h);
	GLuint end_x = (int)(max_x * half_w + half_w);
	GLuint start_y = (int)(min_y * half_h + half_h);

	// nexSAL_TraceCat(0, 0, "[%s %d] min_x:%f min_y:%f max_x:%f max_y:%f starty_x:%d start_y:%d end_x:%d end_y:%d", __func__, __LINE__, min_x, min_y, max_x, max_y, start_x, start_y, end_x, end_y);

	GLuint width = end_x - start_x;
	GLuint height = end_y - start_y;
	
	GLuint tex_id;
	GL_GenTextures(1, &tex_id);
	
	if (tex_id > 0){
		
		texInfo info;
		info.width_ = width;
		info.height_ = height;
		info.src_width_ = width;
		info.src_height_ = height;
		pctx->texinfo_map_.insert(std::pair<GLuint, texInfo>(tex_id, info));
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, start_x, start_y, width, height);

		lua_pushinteger(L, tex_id);
		return 1;
	}

	return 0;
}


static int l_kmGetSizeWithMatrix(lua_State* L){

	float* pmatrix = (float*)lua_touserdata(L, 1);
	if(pmatrix == NULL){

		lua_pushinteger(L, 0);
		return 1;
	}
	float left = lua_tonumber(L, 2);
	float top = lua_tonumber(L, 3);
	float right = lua_tonumber(L, 4);
	float bottom = lua_tonumber(L, 5);
	float feather = lua_tonumber(L, 6);

	RITM_NXT_Vector4f result[4], lefttop, righttop, leftbottom, rightbottom;

	lefttop.e[0] = left;
	lefttop.e[1] = top;
	lefttop.e[2] = 0.0f;
	lefttop.e[3] = 1.0f; 

	righttop.e[0] = right;
	righttop.e[1] = top;
	righttop.e[2] = 0.0f;
	righttop.e[3] = 1.0f;

	leftbottom.e[0] = left;
	leftbottom.e[1] = bottom;
	leftbottom.e[2] = 0.0f;
	leftbottom.e[3] = 1.0f; 

	rightbottom.e[0] = right;
	rightbottom.e[1] = bottom;
	rightbottom.e[2] = 0.0f;
	rightbottom.e[3] = 1.0f;

	result[0] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), lefttop);
	result[1] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), righttop);
	result[2] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), leftbottom);
	result[3] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), rightbottom);


	float min_x = 1000000000.0f;
	float min_y = 1000000000.0f;
	float max_x = -1000000000.0f;
	float max_y = -1000000000.0f;

	for(int i = 0; i < 4; ++i){

		float x = result[i].e[0] /= result[i].e[3];
		float y = result[i].e[1] /= result[i].e[3];

		if(x < min_x)
			min_x = x;
		if(x > max_x)
			max_x = x;

		if(y < min_y)
			min_y = y;
		if(y > max_y)
			max_y = y;
	}

	min_x -= feather;
	min_y -= feather;
	max_x += feather;
	max_y += feather;

	if(min_x < -1.0f)
		min_x = -1.0f;
	if(min_y < -1.0f)
		min_y = -1.0f;
	if(max_x > 1.0f)
		max_x = 1.0f;
	if(max_y > 1.0f)
		max_y = 1.0f;


	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);

	float half_w = (float)(pctx->theme_renderer_.view_width / 2);
	float half_h = (float)(pctx->theme_renderer_.view_height / 2);

	GLuint start_x = (int)(min_x * half_w + half_w);
	GLuint end_y = (int)(max_y * half_h + half_h);
	GLuint end_x = (int)(max_x * half_w + half_w);
	GLuint start_y = (int)(min_y * half_h + half_h);

	GLuint width = end_x - start_x;
	GLuint height = end_y - start_y;
	
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	return 2;
}

static int l_kmDrawRegionTexBox(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int tex = lua_tointeger(L, 1);
	float* pmatrix = (float*)lua_touserdata(L, 2);
	float left = lua_tonumber(L, 3);
	float top = lua_tonumber(L, 4);
	float right = lua_tonumber(L, 5);
	float bottom = lua_tonumber(L, 6);
	float feather = lua_tonumber(L, 7);
	float alpha = lua_tonumber(L, 8);
	int no_filter = lua_tonumber(L, 9);
	int reverse = lua_tonumber(L, 10);
	int reverse_poly = lua_tonumber(L, 11);

	RITM_NXT_Vector4f result[4], lefttop, righttop, leftbottom, rightbottom;

	lefttop.e[0] = left;
	lefttop.e[1] = top;
	lefttop.e[2] = 0.0f;
	lefttop.e[3] = 1.0f; 

	righttop.e[0] = right;
	righttop.e[1] = top;
	righttop.e[2] = 0.0f;
	righttop.e[3] = 1.0f;

	leftbottom.e[0] = left;
	leftbottom.e[1] = bottom;
	leftbottom.e[2] = 0.0f;
	leftbottom.e[3] = 1.0f; 

	rightbottom.e[0] = right;
	rightbottom.e[1] = bottom;
	rightbottom.e[2] = 0.0f;
	rightbottom.e[3] = 1.0f;

	result[0] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), lefttop);
	result[1] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), righttop);
	result[2] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), leftbottom);
	result[3] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), rightbottom);


	float min_x = 1000000000.0f;
	float min_y = 1000000000.0f;
	float max_x = -1000000000.0f;
	float max_y = -1000000000.0f;

	for(int i = 0; i < 4; ++i){

		float x = result[i].e[0] /= result[i].e[3];
		float y = result[i].e[1] /= result[i].e[3];

		if(x < min_x)
			min_x = x;
		if(x > max_x)
			max_x = x;

		if(y < min_y)
			min_y = y;
		if(y > max_y)
			max_y = y;
	}

	min_x -= feather;
	min_y -= feather;
	max_x += feather;
	max_y += feather;

	if(min_x < -1.0f)
		min_x = -1.0f;
	if(min_y < -1.0f)
		min_y = -1.0f;
	if(max_x > 1.0f)
		max_x = 1.0f;
	if(max_y > 1.0f)
		max_y = 1.0f;

	
	RITM_NXT_Matrix4f idmatrix = RITM_NXT_Matrix4f_Identity();
	if(reverse_poly)
		pctx->theme_renderer_.drawTexBox(tex, prender_item->gmask_enabled_, idmatrix.e, min_x, min_y, max_x, max_y, alpha, no_filter, reverse);
	else
		pctx->theme_renderer_.drawTexBox(tex, prender_item->gmask_enabled_, idmatrix.e, min_x, max_y, max_x, min_y, alpha, no_filter, reverse);
	return 0;
}

static int l_kmReleaseTexture(lua_State* L){

	GLuint tex_id = (GLuint)lua_tointeger(L, 1);
	if(tex_id <= 0)
		return 0;
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);	
	glTexInfomap_t& mapper = pctx->texinfo_map_;
	glTexInfomap_t::iterator itor = mapper.find(tex_id);
	if(itor == mapper.end())
		return 0;
	mapper.erase(itor);
	pctx->addDeleteTexture(tex_id);
	return 0;
}

static int l_kmClear(lua_State* L){

	GLenum bit_mask = (GLenum)lua_tointeger(L, 1);
	GLboolean depthwrite_flag = GL_FALSE;
	bool depth_mask_recover = false;

	if(bit_mask&GL_DEPTH_BUFFER_BIT){

		
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthwrite_flag);
		if(depthwrite_flag == GL_FALSE){

			glDepthMask(GL_TRUE);
			depth_mask_recover = true;
		}
	}

	glClear(bit_mask);

	if(depth_mask_recover)
		glDepthMask(depthwrite_flag);

	return 0;
}

static int l_kmGetAniFrame(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	RenderItem* prender_item = renderer->getCurrentRenderItem();
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

static int l_kmSetRenderDest(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	NXT_HThemeRenderer renderer = &pctx->theme_renderer_;
	RenderItem* prender_item = renderer->getCurrentRenderItem();

	int render_dest = lua_tointeger(L, 1);
	int render_test = lua_tointeger(L, 2);

	unsigned int bNeedStencil = 0;
    switch (render_dest) {
            
        case NXT_RenderDest_Stencil:
            bNeedStencil = 1;
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
            break;
		case NXT_RenderDest_Mask:
			break;
		case NXT_RenderDest_Screen:
        default:
            glColorMask(renderer->colorMask[0], renderer->colorMask[1], renderer->colorMask[2], renderer->colorMask[3]);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
            break;
    }
    
    switch (render_test) {
        case NXT_RenderTest_Stencil:
            bNeedStencil = 1;
            glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFFU);
            break;
        case NXT_RenderTest_NegStencil:
            bNeedStencil = 1;
            glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFFU);
            break;
        case NXT_RenderTest_Mask:
			break;
		case NXT_RenderTest_NegMask:
			break;
        case NXT_RenderTest_None:
        default:
            break;
    }
    if( bNeedStencil ) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
    return 0;
}

static int l_kmFilteredSrcOnNexEffect(lua_State* L){

	int slot = lua_tointeger(L, 1);
	int texture = lua_tointeger(L, 2);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	pctx->setVideoSrc(slot|0x10, texture);

	return 0;
}

static int l_kmTextOnNexEffect(lua_State* L){

	int text_slot = lua_tointeger(L, 1);
	int text_texture = lua_tointeger(L, 2);
	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	pctx->setVideoSrc(text_slot + TextureBin::TEXT_PLACE_HOLDER, text_texture);

	return 0;
}

static int l_kmGetTextWidthOnNexEffect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int id = lua_tointeger(L, 1);
	int text_slot = lua_tointeger(L, 2);

	MeshGroup* pmesh = getMeshGroup_internal(prender_item, id);
    if(!pmesh){

        lua_pushinteger(L, 0);
	    return 1;
    }

	int ret = pmesh->getTexWidthOnSlot(text_slot + TextureBin::TEXT_PLACE_HOLDER);
	lua_pushinteger(L, ret);

	return 1;
}

static int l_kmGetTextHeightOnNexEffect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int id = lua_tointeger(L, 1);
	int text_slot = lua_tointeger(L, 2);

	MeshGroup* pmesh = getMeshGroup_internal(prender_item, id);
    if(!pmesh){

        lua_pushinteger(L, 0);
	    return 1;
    }

	int ret = pmesh->getTexHeightOnSlot(text_slot + TextureBin::TEXT_PLACE_HOLDER);
	lua_pushinteger(L, ret);

	return 1;
}

static int l_kmGetWhiteTexture(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	lua_pushinteger(L, pctx->theme_renderer_.getWhiteTexID());
	return 1;
}

static int l_kmGetOutputWidth(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	int ret = pctx->theme_renderer_.view_width;
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmGetOutputHeight(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	int ret = pctx->theme_renderer_.view_height;
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmInitVideoSrc(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);

	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int ret = 0;
	
	if((0 == pctx->getVideoSrc(0)) && (0 == pctx->getVideoSrc(1))){

		texInfo info;

		LOGX("kmInitVideoSrc Here");

		ret = prender_item->makeVideoSrc();
		lua_pushinteger(L, ret);
		return 1;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmInitVideoSrcForDirect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);

	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();
	int ret = 0;
	
	if((0 == pctx->getVideoSrc(0)) && (0 == pctx->getVideoSrc(1))){

		texInfo info;

		LOGX("kmInitVideoSrc Here");

		ret = prender_item->makeVideoSrcForDirect();
		lua_pushinteger(L, ret);
		return 1;
	}
	lua_pushinteger(L, ret);
	return 1;
}

static int l_kmDrawScreen(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();

	int flag = lua_tointeger(L, 1);
	GLuint u_mvp = (GLuint)lua_tointeger(L, 2);
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 3);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 4);
	float* pmatrix = (float*)lua_touserdata(L, 5);
	float left = (GLfloat)lua_tonumber(L, 6);
	float top = (GLfloat)lua_tonumber(L, 7);
	float right = (GLfloat)lua_tonumber(L, 8);
	float bottom = (GLfloat)lua_tonumber(L, 9);

	float texcoord_buffer[] = 
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,
		
	};

	float position_buffer[] =
	{
		left, bottom, 0, 1,
		left, top, 0, 1,
		right, bottom, 0, 1,
		right, top, 0, 1,
	};

	if(flag){

		LOGX("l_kmDrawScreen Here");

		RITM_NXT_Vector4f result[4], lefttop, righttop, leftbottom, rightbottom;

		lefttop.e[0] = left;
		lefttop.e[1] = top;
		lefttop.e[2] = 0.0f;
		lefttop.e[3] = 1.0f; 

		righttop.e[0] = right;
		righttop.e[1] = top;
		righttop.e[2] = 0.0f;
		righttop.e[3] = 1.0f;

		leftbottom.e[0] = left;
		leftbottom.e[1] = bottom;
		leftbottom.e[2] = 0.0f;
		leftbottom.e[3] = 1.0f; 

		rightbottom.e[0] = right;
		rightbottom.e[1] = bottom;
		rightbottom.e[2] = 0.0f;
		rightbottom.e[3] = 1.0f;

		result[0] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), lefttop);
		result[1] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), righttop);
		result[2] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), leftbottom);
		result[3] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), rightbottom);

		float min_x = 1000000000.0f;
		float min_y = 1000000000.0f;
		float max_x = -1000000000.0f;
		float max_y = -1000000000.0f;

		float vtx_min_x = 1000000000.0f;
		float vtx_min_y = 1000000000.0f;
		float vtx_max_x = -1000000000.0f;
		float vtx_max_y = -1000000000.0f;

		for(int i = 0; i < 4; ++i){

			float x = result[i].e[0] /= result[i].e[3];
			float y = result[i].e[1] /= result[i].e[3];

			if(x < min_x)
				min_x = x;
			if(x > max_x)
				max_x = x;

			if(y < min_y)
				min_y = y;
			if(y > max_y)
				max_y = y;
		}

		vtx_min_x = min_x;
		vtx_min_y = min_y;
		vtx_max_x = max_x;
		vtx_max_y = max_y;

		bool out_of_view = false;


		if(max_x < -1.0f){
			max_x = -1.0f;
			out_of_view = true;
		}

		if(max_y < -1.0f){
			max_y = -1.0f;
			out_of_view = true;
		}

		if(min_x < -1.0f){
			min_x = -1.0f;
			out_of_view = true;
		}
		if(min_y < -1.0f){
			out_of_view = true;
			min_y = -1.0f;
		}

		if(min_x > 1.0f){
			min_x = 1.0f;
			out_of_view = true;
		}
		if(min_y > 1.0f){
			out_of_view = true;
			min_y = 1.0f;
		}

		if(max_x > 1.0f){
			out_of_view = true;
			max_x = 1.0f;
		}
		if(max_y > 1.0f){
			out_of_view = true;
			max_y = 1.0f;
		}

		float uv_width = max_x - min_x;
		float uv_height = max_y - min_y;

		if(uv_width > 0.0f && uv_height > 0.0f){

			if(prender_item->gmask_enabled_ || out_of_view){

				float vtx_uv_width = vtx_max_x - vtx_min_x;
				float vtx_uv_height = vtx_max_y - vtx_min_y;

				position_buffer[4*1 + 0] = -1.0f;
				texcoord_buffer[2*1 + 0] = 0.0f;
				position_buffer[4*1 + 1] = 1.0f;
				texcoord_buffer[2*1 + 1] = 1.0f;
				position_buffer[4*1 + 2] = 0;
				position_buffer[4*1 + 3] = 1;
				position_buffer[4*3 + 0] = 1.0f;
				texcoord_buffer[2*3 + 0] = 1.0f;
				position_buffer[4*3 + 1] = 1.0f;
				texcoord_buffer[2*3 + 1] = 1.0f;
				position_buffer[4*3 + 2] = 0;
				position_buffer[4*3 + 3] = 1;
				position_buffer[4*0 + 0] = -1.0f;
				texcoord_buffer[2*0 + 0] = 0.0f;
				position_buffer[4*0 + 1] = -1.0f;
				texcoord_buffer[2*0 + 1] = 0.0f;
				position_buffer[4*0 + 2] = 0;
				position_buffer[4*0 + 3] = 1;
				position_buffer[4*2 + 0] = 1.0f;
				texcoord_buffer[2*2 + 0] = 1.0f;
				position_buffer[4*2 + 1] = -1.0f;
				texcoord_buffer[2*2 + 1] = 0.0f;
				position_buffer[4*2 + 2] = 0;
				position_buffer[4*2 + 3] = 1;

				glUniformMatrix4fv(u_mvp, 1, false, RITM_NXT_Matrix4f_Identity().e);

				NXT_HThemeRenderer renderer = &pctx->theme_renderer_;

				NXT_RenderTarget* prs_target = NULL;
				NXT_RenderTarget* prender_target = renderer->getDefaultRenderTarget();
				int width = (int)fabsf(right - left);
				int height = (int)fabsf(bottom - top);
				prs_target = renderer->getRenderTarget(width, height, 1, 1, 1);
				renderer->setRenderTargetAsDefault(prs_target);

				glClearColor(0, 0, 0, 0);
				glDepthMask(GL_TRUE);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				glDepthMask(GL_FALSE);

				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);

				GLboolean stencil_test = GL_FALSE;
				glGetBooleanv(GL_STENCIL_TEST, &stencil_test);
				glDisable(GL_STENCIL_TEST);					

				glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
				glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
				glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
				glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();

				renderer->setRenderTargetAsDefault(prender_target);

				glEnable(GL_STENCIL_TEST);
                glClear(GL_STENCIL_BUFFER_BIT);

				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	            glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	            glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);

	            renderer->drawSolid(pmatrix, left, top, right, bottom);

	            glColorMask(renderer->colorMask[0], renderer->colorMask[1], renderer->colorMask[2], renderer->colorMask[3]);

	            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	            glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFFU);

				renderer->drawTexBox(prs_target->target_texture_, prender_item->gmask_enabled_, RITM_NXT_Matrix4f_Identity().e, vtx_min_x, vtx_max_y, vtx_max_x, vtx_min_y, 1.0f, 0, 0);
				renderer->releaseRenderTarget(prs_target);

				if(!stencil_test){

					glDisable(GL_STENCIL_TEST);
				}
				return 0;
			}
			else{
				position_buffer[4*1 + 0] = result[0].e[0];
				texcoord_buffer[2*1 + 0] = (position_buffer[4*1 + 0] - min_x) / uv_width;
				position_buffer[4*1 + 1] = result[0].e[1];
				texcoord_buffer[2*1 + 1] = (position_buffer[4*1 + 1] - min_y) / uv_height;
				position_buffer[4*1 + 2] = 0;
				position_buffer[4*1 + 3] = 1;
				position_buffer[4*3 + 0] = result[1].e[0];
				texcoord_buffer[2*3 + 0] = (position_buffer[4*3 + 0] - min_x) / uv_width;
				position_buffer[4*3 + 1] = result[1].e[1];
				texcoord_buffer[2*3 + 1] = (position_buffer[4*3 + 1] - min_y) / uv_height;
				position_buffer[4*3 + 2] = 0;
				position_buffer[4*3 + 3] = 1;
				position_buffer[4*0 + 0] = result[2].e[0];
				texcoord_buffer[2*0 + 0] = (position_buffer[4*0 + 0] - min_x) / uv_width;
				position_buffer[4*0 + 1] = result[2].e[1];
				texcoord_buffer[2*0 + 1] = (position_buffer[4*0 + 1] - min_y) / uv_height;
				position_buffer[4*0 + 2] = 0;
				position_buffer[4*0 + 3] = 1;
				position_buffer[4*2 + 0] = result[3].e[0];
				texcoord_buffer[2*2 + 0] = (position_buffer[4*2 + 0] - min_x) / uv_width;
				position_buffer[4*2 + 1] = result[3].e[1];
				texcoord_buffer[2*2 + 1] = (position_buffer[4*2 + 1] - min_y) / uv_height;
				position_buffer[4*2 + 2] = 0;
				position_buffer[4*2 + 3] = 1;
			}
		}
		else
			return 0;
		
		glUniformMatrix4fv(u_mvp, 1, false, RITM_NXT_Matrix4f_Identity().e);
	}

	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();

	return 0;
}

static int l_kmDrawScreenDirect(lua_State* L){

	RenderItemContext* pctx = (RenderItemContext*)lua_getRenderItemContext(L);
	RenderItem* prender_item = pctx->theme_renderer_.getCurrentRenderItem();

	int flag = lua_tointeger(L, 1);
	GLuint u_mvp = (GLuint)lua_tointeger(L, 2);
	GLuint vertex_attrib = (GLuint)lua_tointeger(L, 3);
	GLuint texcoord_attrib = (GLuint)lua_tointeger(L, 4);
	float* pmatrix = (float*)lua_touserdata(L, 5);
	float left = (GLfloat)lua_tonumber(L, 6);
	float top = (GLfloat)lua_tonumber(L, 7);
	float right = (GLfloat)lua_tonumber(L, 8);
	float bottom = (GLfloat)lua_tonumber(L, 9);

	float texcoord_buffer[] =
	{
		0, 0,
		0, 1,
		1, 0,
		1, 1,
		
	};

	float position_buffer[] =
	{
		left, bottom, 0, 1,
		left, top, 0, 1,
		right, bottom, 0, 1,
		right, top, 0, 1,
	};

	if(flag){

		LOGX("l_kmDrawScreen Here");

		RITM_NXT_Vector4f result[4], lefttop, righttop, leftbottom, rightbottom;

		lefttop.e[0] = left;
		lefttop.e[1] = top;
		lefttop.e[2] = 0.0f;
		lefttop.e[3] = 1.0f; 

		righttop.e[0] = right;
		righttop.e[1] = top;
		righttop.e[2] = 0.0f;
		righttop.e[3] = 1.0f;

		leftbottom.e[0] = left;
		leftbottom.e[1] = bottom;
		leftbottom.e[2] = 0.0f;
		leftbottom.e[3] = 1.0f; 

		rightbottom.e[0] = right;
		rightbottom.e[1] = bottom;
		rightbottom.e[2] = 0.0f;
		rightbottom.e[3] = 1.0f;

		result[0] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), lefttop);
		result[1] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), righttop);
		result[2] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), leftbottom);
		result[3] = RITM_NXT_Matrix4f_MultVector(*((RITM_NXT_Matrix4f*)pmatrix), rightbottom);

		float min_x = 1000000000.0f;
		float min_y = 1000000000.0f;
		float max_x = -1000000000.0f;
		float max_y = -1000000000.0f;

		float vtx_min_x = 1000000000.0f;
		float vtx_min_y = 1000000000.0f;
		float vtx_max_x = -1000000000.0f;
		float vtx_max_y = -1000000000.0f;

		for(int i = 0; i < 4; ++i){

			float x = result[i].e[0] /= result[i].e[3];
			float y = result[i].e[1] /= result[i].e[3];

			if(x < min_x)
				min_x = x;
			if(x > max_x)
				max_x = x;

			if(y < min_y)
				min_y = y;
			if(y > max_y)
				max_y = y;
		}
		vtx_min_x = min_x;
		vtx_min_y = min_y;
		vtx_max_x = max_x;
		vtx_max_y = max_y;

		bool out_of_view = false;


		if(min_x < -1.0f){
			min_x = -1.0f;
			out_of_view = true;
		}
		if(min_y < -1.0f){
			out_of_view = true;
			min_y = -1.0f;
		}
		if(max_x > 1.0f){
			out_of_view = true;
			max_x = 1.0f;
		}
		if(max_y > 1.0f){
			out_of_view = true;
			max_y = 1.0f;
		}

		float uv_width = max_x - min_x;
		float uv_height = max_y - min_y;

		if(uv_width <= 0.0f || uv_height <= 0.0f)
			return 0;

		position_buffer[4*1 + 0] = result[0].e[0];
		texcoord_buffer[2*1 + 0] = (position_buffer[4*1 + 0] - min_x) / uv_width;
		position_buffer[4*1 + 1] = result[0].e[1];
		texcoord_buffer[2*1 + 1] = (position_buffer[4*1 + 1] - min_y) / uv_height;
		position_buffer[4*1 + 2] = 0;
		position_buffer[4*1 + 3] = 1;
		position_buffer[4*3 + 0] = result[1].e[0];
		texcoord_buffer[2*3 + 0] = (position_buffer[4*3 + 0] - min_x) / uv_width;
		position_buffer[4*3 + 1] = result[1].e[1];
		texcoord_buffer[2*3 + 1] = (position_buffer[4*3 + 1] - min_y) / uv_height;
		position_buffer[4*3 + 2] = 0;
		position_buffer[4*3 + 3] = 1;
		position_buffer[4*0 + 0] = result[2].e[0];
		texcoord_buffer[2*0 + 0] = (position_buffer[4*0 + 0] - min_x) / uv_width;
		position_buffer[4*0 + 1] = result[2].e[1];
		texcoord_buffer[2*0 + 1] = (position_buffer[4*0 + 1] - min_y) / uv_height;
		position_buffer[4*0 + 2] = 0;
		position_buffer[4*0 + 3] = 1;
		position_buffer[4*2 + 0] = result[3].e[0];
		texcoord_buffer[2*2 + 0] = (position_buffer[4*2 + 0] - min_x) / uv_width;
		position_buffer[4*2 + 1] = result[3].e[1];
		texcoord_buffer[2*2 + 1] = (position_buffer[4*2 + 1] - min_y) / uv_height;
		position_buffer[4*2 + 2] = 0;
		position_buffer[4*2 + 3] = 1;

		glUniformMatrix4fv(u_mvp, 1, false, RITM_NXT_Matrix4f_Identity().e);
	}
	
	glEnableVertexAttribArray(vertex_attrib); CHECK_GL_ERROR();
	glEnableVertexAttribArray(texcoord_attrib); CHECK_GL_ERROR();
	glVertexAttribPointer(vertex_attrib, 4, GL_FLOAT, false, 0, position_buffer);    CHECK_GL_ERROR();
	glVertexAttribPointer(texcoord_attrib, 2, GL_FLOAT, false, 0, texcoord_buffer);  CHECK_GL_ERROR();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                                      CHECK_GL_ERROR();

	return 0;
}

struct func_string{
	lua_CFunction func;
	const char* func_callname;
};

func_string func_strings[] =
{
	{l_kmUseProgram, "kmUseProgram"},
    {l_kmSetUniform1i, "kmSetUniform1i"},
	{l_kmSetUniform1f, "kmSetUniform1f"},
    {l_kmSetUniform1fv, "kmSetUniform1fv"},
    {l_kmSetUniform2fv, "kmSetUniform2fv"},
    {l_kmSetUniform3fv, "kmSetUniform3fv"},
    {l_kmSetUniform4fv, "kmSetUniform4fv"},
	{l_kmSetTexture, "kmSetTexture"},
	{l_kmSetTextureExternal, "kmSetTextureExternal"},
	{l_kmDrawRect, "kmDrawRect"},
	{l_kmDrawRectWithPos, "kmDrawRectWithPos"},
	{l_kmDrawSphere, "kmDrawSphere"},
	{l_kmGetTick, "kmGetTick"},
	{l_kmNewMatrix, "kmNewMatrix"},
	{l_kmSetPerspective, "kmSetPerspective"},
	{l_kmGetDefault3DMatrix, "kmGetDefault3DMatrix"},
	{l_kmSetUniformMat4, "kmSetUniformMat4"},
	{l_kmRotate, "kmRotate"},
	{l_kmTranslate, "kmTranslate"},
	{l_kmScale, "kmScale"},
	{l_kmLoadIdentity, "kmLoadIdentity"},
	{l_kmSetUniform2f, "kmSetUniform2f"},
	{l_kmSetUniform3f, "kmSetUniform3f"},
	{l_kmSetUniform4f, "kmSetUniform4f"},
	{l_kmGetTexWidth, "kmGetTexWidth"},
	{l_kmGetTexHeight, "kmGetTexHeight"},
	{l_kmGetSrcWidth, "kmGetSrcWidth"},
	{l_kmGetSrcHeight, "kmGetSrcHeight"},
	{l_kmSetRenderToDefault, "kmSetRenderToDefault"},
	{l_kmSetRenderToTexture, "kmSetRenderToTexture"},
	{l_kmSetRenderToClearTexture, "kmSetRenderToClearTexture"},
	{l_kmDrawText, "kmDrawText"},
	{l_kmGetImage, "kmGetImage"},
	{l_kmDrawArray, "kmDrawArray"},
	{l_kmDrawElement, "kmDrawElement"},
	{l_kmCopyScrToTexture, "kmCopyScrToTexture"},
	{l_kmCreateTexture, "kmCreateTexture"},
	{l_kmReleaseTexture, "kmReleaseTexture"},
	{l_kmClear, "kmClear"},
	{l_kmSetBlend, "kmSetBlend"},
	{l_kmSetFlare, "kmSetFlare"},
	{l_kmGetAniFrame, "kmGetAniFrame"},
	{l_kmSetRenderDest, "kmSetRenderDest"},
	{l_kmGetScreenTexture, "kmGetScreenTexture"},
	{l_kmGetSizeWithMatrix, "kmGetSizeWithMatrix"},
	{l_kmDrawNexEffect, "kmDrawNexEffect"},
	{l_kmDrawNexEffectEasy, "kmDrawNexEffectEasy"},
	{l_kmGetFrameNexEffect, "kmGetFrameNexEffect"},
	{l_kmDrawBound, "kmDrawBound"},
	{l_kmTextOnNexEffect, "kmTextOnNexEffect"},
	{l_kmFilteredSrcOnNexEffect, "kmFilteredSrcOnNexEffect"},
	{l_kmGetTextWidthOnNexEffect, "kmGetTextWidthOnNexEffect"},
	{l_kmGetTextHeightOnNexEffect, "kmGetTextHeightOnNexEffect"},
	{l_kmDrawSolid, "kmDrawSolid"},
	{l_kmDrawTexBox, "kmDrawTexBox"},
	{l_kmDrawRegionTexBox, "kmDrawRegionTexBox"},
	{l_kmGetWhiteTexture, "kmGetWhiteTexture"},
	{l_kmGetOutputWidth, "kmGetOutputWidth"},
	{l_kmGetOutputHeight, "kmGetOutputHeight"},
	{l_kmInitVideoSrc, "kmInitVideoSrc"},
    {l_kmInitVideoSrcForDirect, "kmInitVideoSrcForDirect"},
	{l_kmDrawScreen, "kmDrawScreen"},
    {l_kmDrawScreenDirect, "kmDrawScreenDirect"}
};



void registerLuaExtensionFunc(lua_State* L){

    luaL_requiref(L, "array", luaopen_array, 1);
    lua_pop(L, 1);
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
	lua_pushinteger(L, GL_STENCIL_BUFFER_BIT);
	lua_setglobal(L, "GL_STENCIL_BUFFER_BIT");
	lua_pushinteger(L, RTT_REQUEST_DEPTH);
	lua_setglobal(L, "RTT_REQUEST_DEPTH");
	lua_pushinteger(L, RTT_REQUEST_STENCIL);
	lua_setglobal(L, "RTT_REQUEST_STENCIL");
	lua_pushinteger(L, RTT_REQUEST_ALPHA);
	lua_setglobal(L, "RTT_REQUEST_ALPHA");
}
