#pragma once

#include "video_target_def.h"

typedef int (*LOADTHEMECALLBACKFUNC)(char** ppOutputData, int* pLength, char* path, void* cbdata);

void* NXT_Theme_GetRenderItem(void* pmanager, const char* pid, const char* uid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* pfunc_for_loadimagecb, void* pfunc_for_freeimagecb, void* cbdata);
void* NXT_Theme_CreateRenderItemManager();
void NXT_Theme_DestroyRenderItemManager(void* pmanager);
void NXT_Theme_InitContextOnManager(void* pmanager);
void NXT_Theme_AddRenderItemToManager(void* pmanager, void* pitem);
void NXT_Theme_UpdateRenderItemManager(void* pmanager, unsigned int current_mask_id, unsigned int current_blend_id, unsigned int white_tex_id);
int NXT_Theme_CountOfRenderItem(void* pmanager);
void NXT_Theme_SetTextureInfoRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height);
void NXT_Theme_ApplyEffectRenderItem(void* pmanager, int texid, int effect_id, float progress);
void NXT_Theme_ApplyTransitionRenderItem(void* pmanager, int texid_left, int texid_right, int effect_id, float progress);
int NXT_Theme_GetEffectID(void* pmanager, const char* id);
int NXT_Theme_GetEffectType(void* pmanager, int effect_id);
int NXT_Theme_GetEffectOverlap(void* pmanager, int effect_id);
void NXT_Theme_ClearRenderItems(void* pmanager);

void NXT_Theme_BeginRenderItem(void* pmanager, int effect_id, int width, int height, void* pdefault_render_target);
void NXT_Theme_EndRenderItem(void* pmanager);
void NXT_Theme_ResetPRS(void* pmanager);
void NXT_Theme_SetPosition(void* pmanager, float x, float y, float z);
void NXT_Theme_SetRotation(void* pmanager, float x, float y, float z, float angle);
void NXT_Theme_SetScale(void* pmanager, float x, float y, float z);
void NXT_Theme_SetMatrix(void* pmanager, float* pmatrix);
void NXT_Theme_SetRect(void* pmanager, float left, float top, float right, float bottom, float alpha_val, bool gmask_enbaled = false);
void NXT_Theme_ResetMatrix(void* pmanager);
int NXT_Theme_ApplyRenderItem(void* pmanager, float progress);
void NXT_Theme_ClearSrcInfo(void* pmanager);
void NXT_Theme_ClearSrcInfoTarget(void* pmanager, int target);
void NXT_Theme_SaveSrcInfo(void* pmanager, int target);
void NXT_Theme_RestoreSrcInfo(void* pmanager, int target);
void NXT_Theme_SetTextureInfoTargetRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height, int target);
void NXT_Theme_SetTexTargetRenderItem(void* pmanager, int texid, int target);
void NXT_Theme_SetTexMatrix(void* pmanager, void* pmatrix, int target);
void NXT_Theme_SetColorconvMatrix(void* pmanager, void* pmatrix, int target);
void NXT_Theme_SetRealXY(void* pmanager, float x, float y, int target);
void NXT_Theme_SetValueMatrix(void* pmanager, const char* key, float* value);
void NXT_Theme_SetValue(void* pmanager, const char* key_string, const char* value_string);
void NXT_Theme_ResetDefaultValuemap(void* pmanager);
void NXT_Theme_ApplyDefaultValues(void* pmanager);
void NXT_Theme_SetValueInt(void* pmanager, const char* key_string, int value);
void NXT_Theme_SetValueFloat(void* pmanager, const char* key_string, float value);
void NXT_Theme_DoEffect(void* pmanager, int elapsed_time, int cur_time, int clipStartTime, int clipEndTime, int max_time, int actualEffectStartCTS, int actualEffectEndCTS, int clipindex, int total_clip_count);

void NXT_Theme_SetDefaultFrameBufferNum(void* pmanager, unsigned int framebuffer_num);
void NXT_Theme_ForceBind(void* pmanager, int effect_id);
void NXT_Theme_ForceUnbind(void* pmanager, int effect_id);
void NXT_Theme_GetResourcelist(void* pmanager, int effect_id, void* resource_load_list);
void NXT_Theme_PrecacheRI(void* pmanager, void* pinfo, void* prenderer, void (*pAquireContextFunc)(void* p), void (*pReleaseContextFunc)(void* p));
void NXT_Theme_SetTextureMaxSize(void* pmanager, int max_size);