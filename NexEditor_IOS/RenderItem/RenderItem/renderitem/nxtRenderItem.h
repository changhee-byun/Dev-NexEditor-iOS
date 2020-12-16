#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	enum{

		E_TARGET_VIDEO_SRC,
		E_TARGET_VIDEO_LEFT,
		E_TARGET_VIDEO_RIGHT,
		E_TARGET_OVER
	};

typedef int (*LOADTHEMECALLBACKFUNC)(char** ppOutputData, int* pLength, char* path, void* cbdata);

void* NXT_Theme_GetRenderItem(void* pmanager, const char* pid, const char* src, int child_count, LOADTHEMECALLBACKFUNC pfunc, void* cbdata);
void* NXT_Theme_CreateRenderItemManager();
void NXT_Theme_AddRenderItemToManager(void* pmanager, void* pitem);
void NXT_Theme_UpdateRenderItemManager(void* pmanager);
int NXT_Theme_CountOfRenderItem(void* pmanager);
void NXT_Theme_SetTextureInfoRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height);
void NXT_Theme_ApplyEffectRenderItem(void* pmanager, int texid, int effect_id, float progress);
void NXT_Theme_ApplyTransitionRenderItem(void* pmanager, int texid_left, int texid_right, int effect_id, float progress);
int NXT_Theme_GetEffectID(void* pmanager, const char* id);
int NXT_Theme_GetEffectType(void* pmanager, int effect_id);
int NXT_Theme_GetEffectOverlap(void* pmanager, int effect_id);
void NXT_Theme_ClearRenderItems(void* pmanager);

void NXT_Theme_BeginRenderItem(void* pmanager, int effect_id);
void NXT_Theme_EndRenderItem(void* pmanager);
void NXT_Theme_ApplyRenderItem(void* pmanager, float progress);
void NXT_Theme_SetTextureInfoTargetRenderItem(void* pmanager, int texid, int width, int height, int src_width, int src_height, int target);
void NXT_Theme_SetTexTargetRenderItem(void* pmanager, int texid, int target);
void NXT_Theme_SetTexMatrix(void* pmanager, void* pmatrix);
void NXT_Theme_SetValueMatrix(void* pmanager, const char* key, float* value);
void NXT_Theme_SetValue(void* pmanager, const char* key_string, const char* value_string);
void NXT_Theme_SetValueInt(void* pmanager, const char* key_string, int value);
void NXT_Theme_SetValueFloat(void* pmanager, const char* key_string, float value);
void NXT_Theme_DoEffect(void* pmanager, int elapsed_time, int cur_time, int clipStartTime, int clipEndTime, int max_time, int actualEffectStartCTS, int actualEffectEndCTS, int clipindex, int total_clip_count);

void NXT_Theme_ForceBind(void* pmanager, int effect_id);
void NXT_Theme_ForceUnbind(void* pmanager, int effect_id);

#ifdef __cplusplus
}
#endif