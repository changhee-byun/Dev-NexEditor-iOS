/******************************************************************************
 * File Name   : RenderItem.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "RenderItem.h"
#include "nxtRenderItem.h"

static RenderItem *instance = nil;
static dispatch_once_t onceToken;

int themeCallback(char** ppOutputData, int* pLength, char* path, void* cbdata)
{
    RenderItem *renderItem = [RenderItem sharedInstance];
    return renderItem.loadThemeCallback(ppOutputData, pLength, path, cbdata);
}

@implementation RenderItem

+ (instancetype)sharedInstance
{
    dispatch_once(&onceToken, ^{
        if(instance == nil) {
            instance = [[RenderItem alloc] init];
        }
    });
    return instance;
}

- (void *)NXT_GetRenderItem:(void *)pmanager
                        pid:(const char*)pid
                        src:(const char*)src
                 childCount:(int)childCount
          loadThemeCallback:(LoadThemeCallback)callback
                     cbdata:(void *)cbdata
{
    self.loadThemeCallback = callback;
    return NXT_Theme_GetRenderItem(pmanager, pid, src, childCount, themeCallback, cbdata);
}

- (void *)NXT_CreateRenderItemManager
{
    return NXT_Theme_CreateRenderItemManager();
}

- (void)NXT_AddRenderItemToManager:(void *)pmanager pItem:(void *)pItem
{
    NXT_Theme_AddRenderItemToManager(pmanager, pItem);
}

- (void)NXT_UpdateRenderItemManager:(void *)pmanager
{
    NXT_Theme_UpdateRenderItemManager(pmanager);
}

- (int)NXT_CountOfRenderItem:(void *)pmanager
{
    return NXT_Theme_CountOfRenderItem(pmanager);
}

- (void)NXT_SetTextureInfoRenderItem:(void *)pmanager
                               texid:(int)texid
                               width:(int)width
                              height:(int)height
                         sourceWidth:(int)sourceWidth
                        sourceHeight:(int)sourceHeight
{
    NXT_Theme_SetTextureInfoRenderItem(pmanager, texid, width, height, sourceWidth, sourceHeight);
}

- (void)NXT_ApplyEffectRenderItem:(void *)pmanager
                            texid:(int)texid
                         effectid:(int)effectid
                         progress:(float)progress
{
    NXT_Theme_ApplyEffectRenderItem(pmanager, texid, effectid, progress);
}


- (void)NXT_ApplyTransitionRenderItem:(void *)pmanager
                            texidLeft:(int)texidLeft
                           texidRight:(int)texidRight
                             effectid:(int)effectid
                             progress:(float)progress
{
    NXT_Theme_ApplyTransitionRenderItem(pmanager, texidLeft, texidRight, effectid, progress);
}

- (void)NXT_GetEffectId:(void *)pmanager effectId:(const char*)effectId
{
    NXT_Theme_GetEffectID(pmanager, effectId);
}

- (void)NXT_GetEffectType:(void *)pmanager effectId:(int)effectId
{
    NXT_Theme_GetEffectType(pmanager, effectId);
}

- (void)NXT_GetEffectOverlap:(void *)pmanager effectId:(int)effectId
{
    NXT_Theme_GetEffectOverlap(pmanager, effectId);
}

- (void)NXT_ClearRenderItems:(void *)pmanager
{
    NXT_Theme_ClearRenderItems(pmanager);
}

- (void)NXT_BeginRenderItem:(void *)pmanager effectId:(int)effectId
{
    NXT_Theme_BeginRenderItem(pmanager, effectId);
}

- (void)NXT_EndRenderItem:(void *)pmanager
{
    NXT_Theme_EndRenderItem(pmanager);
}

- (void)NXT_ApplyRenderItem:(void *)pmanager progress:(float)progress
{
    NXT_Theme_ApplyRenderItem(pmanager, progress);
}

- (void)NXT_SetTextureInfoTargetRenderItem:(void *)pmanager
                                     texid:(int)texid
                                     width:(int)width
                                    height:(int)height
                               sourceWidth:(int)sourceWidth
                              sourceHeight:(int)sourceHeight
{
    NXT_Theme_SetTextureInfoRenderItem(pmanager, texid, width, height, sourceWidth, sourceHeight);
}

- (void)NXT_SetTexTargetRenderItem:(void *)pmanager texid:(int)texid target:(int)target
{
    NXT_Theme_SetTexTargetRenderItem(pmanager, texid, target);
}

- (void)NXT_SetTexMatrix:(void *)pmanager pmatrix:(void *)pmatrix
{
    NXT_Theme_SetTexMatrix(pmanager, pmatrix);
}

- (void)NXT_SetValueMatrix:(void *)pmanager key:(const char *)key value:(float *)value
{
    NXT_Theme_SetValueMatrix(pmanager, key, value);
}

- (void)NXT_SetValue:(void *)pmanager key:(const char *)key value:(const char *)value
{
    NXT_Theme_SetValue(pmanager, key, value);
}

- (void)NXT_SetValueInt:(void *)pmanager key:(const char *)key value:(int)value
{
    NXT_Theme_SetValueInt(pmanager, key, value);
}

- (void)NXT_SetValueFloat:(void *)pmanager key:(const char *)key value:(float)value
{
    NXT_Theme_SetValueFloat(pmanager, key, value);
}

- (void)NXT_DoEffect:(void *)pmanager
         elapsedtime:(int)elapsedtime
             curtime:(int)curtime
           starttime:(int)starttime
             endtime:(int)endtime
             maxtime:(int)maxtime
     actualstarttime:(int)actualstarttime
       actualendtime:(int)actualendtime
           clipindex:(int)clipindex
           clipcount:(int)clipcount
{
    NXT_Theme_DoEffect(pmanager, elapsedtime, curtime, starttime, endtime, maxtime, actualstarttime, actualendtime, clipindex, clipcount);
}

- (void)NXT_ForceBind:(void *)pmanager effectid:(int)effectid
{
    NXT_Theme_ForceBind(pmanager, effectid);
}

- (void)NXT_ForceUnBind:(void *)pmanager effectid:(int)effectid
{
    NXT_Theme_ForceUnbind(pmanager, effectid);
}

@end
