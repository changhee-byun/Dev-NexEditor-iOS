/******************************************************************************
 * File Name   : RenderItem.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

typedef int (^LoadThemeCallback)(char **ppOutputData, int *pLength, char *path, void *cbdata);

typedef NS_ENUM(NSUInteger, TargetTypes) {
    TARGET_VIDEO_SRC = 0,
    TARGET_VIDEO_LEFT,
    TARGET_VIDEO_RIGHT,
    TARGET_OVER
};

@interface RenderItem : NSObject

@property (copy, nonatomic) LoadThemeCallback loadThemeCallback;

/**
 * \brief
 */
+ (instancetype)sharedInstance;

/**
 * \brief
 */
- (void *)NXT_GetRenderItem:(void *)pmanager pid:(const char*)pid src:(const char*)src childCount:(int)childCount loadThemeCallback:(LoadThemeCallback)loadThemeCallback cbdata:(void *)cbdata;

/**
 * \brief
 */
- (void *)NXT_CreateRenderItemManager;

/**
 * \brief
 */
- (void)NXT_AddRenderItemToManager:(void *)pmanager pItem:(void *)pItem;

/**
 * \brief
 */
- (void)NXT_UpdateRenderItemManager:(void *)pmanager;

/**
 * \brief
 */
- (int)NXT_CountOfRenderItem:(void *)pmanager;

/**
 * \brief
 */
- (void)NXT_SetTextureInfoRenderItem:(void *)pmanager texid:(int)texid width:(int)width height:(int)height sourceWidth:(int)sourceWidth sourceHeight:(int)sourceHeight;

/**
 * \brief
 */
- (void)NXT_ApplyEffectRenderItem:(void *)pmanager texid:(int)texid effectid:(int)effectid progress:(float)progress;

/**
 * \brief
 */
- (void)NXT_ApplyTransitionRenderItem:(void *)pmanager texidLeft:(int)texidLeft texidRight:(int)texidRight effectid:(int)effectid progress:(float)progress;

/**
 * \brief
 */
- (void)NXT_GetEffectId:(void *)pmanager effectId:(const char*)effectId;

/**
 * \brief
 */
- (void)NXT_GetEffectType:(void *)pmanager effectId:(int)effectId;

/**
 * \brief
 */
- (void)NXT_GetEffectOverlap:(void *)pmanager effectId:(int)effectId;

/**
 * \brief
 */
- (void)NXT_ClearRenderItems:(void *)pmanager;

/**
 * \brief
 */
- (void)NXT_BeginRenderItem:(void *)pamanger effectId:(int)effectId;

/**
 * \brief
 */
- (void)NXT_EndRenderItem:(void *)pmanager;

/**
 * \brief
 */
- (void)NXT_ApplyRenderItem:(void *)pmanager progress:(float)progress;

/**
 * \brief
 */
- (void)NXT_SetTextureInfoTargetRenderItem:(void *)pmanager texid:(int)texid width:(int)width height:(int)height sourceWidth:(int)sourceWidth sourceHeight:(int)sourceHeight;

/**
 * \brief
 */
- (void)NXT_SetTexTargetRenderItem:(void *)pmanager texid:(int)texid target:(int)target;

/**
 * \brief
 */
- (void)NXT_SetTexMatrix:(void *)pmanager pmatrix:(void *)pmatrix;

/**
 * \brief
 */
- (void)NXT_SetValueMatrix:(void *)pmanager key:(const char *)key value:(float *)value;

/**
 * \brief
 */
- (void)NXT_SetValue:(void *)pmanager key:(const char *)key value:(const char *)value;

/**
 * \brief
 */
- (void)NXT_SetValueInt:(void *)pmanager key:(const char *)key value:(int)value;

/**
 * \brief
 */
- (void)NXT_SetValueFloat:(void *)pmanager key:(const char *)key value:(float)value;

/**
 * \brief
 */
- (void)NXT_DoEffect:(void *)pmanager elapsedtime:(int)elapsedtime curtime:(int)curtime starttime:(int)starttime endtime:(int)endtime maxtime:(int)maxtime actualstarttime:(int)actualstarttime actualendtime:(int)actualendtime clipindex:(int)clipindex clipcount:(int)clipcount;

/**
 * \brief
 */
- (void)NXT_ForceBind:(void *)pmanager effectid:(int)effectid;

/**
 * \brief
 */
- (void)NXT_ForceUnBind:(void *)pmanager effectid:(int)effectid;

@end
