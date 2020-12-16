#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_EffectItem"

#ifdef ANDROID
#include <android/log.h>
#define LOGFORCE(...)    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
extern "C" {
    void NSLog(CFStringRef format, ...);
}
#define LOGFORCE(format, ...)   NSLog(CFSTR(format), ##__VA_ARGS__)
#endif

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

enum{

    EFFECT_UNKNOWN = 0,
    EFFECT_TITLE,
    EFFECT_TRANSITION,
};

NXT_NodeClass NXT_NodeClass_EffectItem = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    NULL,//precacheFunc
    NULL,//getPrecacheResourceFunc
    NULL,//nodeUpdateNotify
    NULL,//freeTextureFunc
    "effectitem",
    sizeof(NXT_Node_KEDL)
};

static void nodeInitFunc(NXT_NodeHeader *node) {

}

static void nodeFreeFunc(NXT_NodeHeader *node) {

    NXT_Node_EffectItem *nodeData = (NXT_Node_EffectItem*)node;
    if(nodeData->kedlstateblock){

        NXT_ThemeRenderer_FreeAppliedEffect(nodeData->kedlstateblock);
        delete nodeData->kedlstateblock;
        nodeData->kedlstateblock = NULL;
    }    
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {

    NXT_Node_EffectItem *nodeData = (NXT_Node_EffectItem*)node;

    if(strcasecmp(attrName, "kedlitem") == 0){

        LOGXV("id: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {

            nodeData->kedlitem = (NXT_Node_KEDL*)NXT_FindFirstNodeWithId(node, attrValue+1, &NXT_NodeClass_KEDL);
            if(NULL == nodeData->kedlstateblock){

                nodeData->kedlstateblock = new NXT_AppliedEffect();
            }

            NXT_ThemeRenderer_FreeAppliedEffect((NXT_AppliedEffect*)nodeData->kedlstateblock);
        }
    }
    else if(strcasecmp(attrName, "type")){

        if( strcasestr(attrValue, "transition") ){

            nodeData->effect_type = EFFECT_TRANSITION;
        }
        else if( strcasestr(attrValue, "title") ){

            nodeData->effect_type = EFFECT_TITLE;
        }
        else
            nodeData->effect_type = EFFECT_UNKNOWN;
    }
    else if( strcasecmp(attrName, "start")==0 ) {

        float value = 0;
        NXT_FloatVectorFromString( attrValue, &value, 1 );
        nodeData->start_time = value;
    }
    else if( strcasecmp(attrName, "end")==0 ) {
        
        float value = 0;
        NXT_FloatVectorFromString( attrValue, &value, 1 );
        nodeData->end_time = value;
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
     

}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {

    NXT_Node_EffectItem *nodeData = (NXT_Node_EffectItem*)node;

    renderer->pushVideoTrack();

    NXT_AppliedEffect* pstate_backup = renderer->pActiveEffect;
    renderer->pActiveEffect = nodeData->kedlstateblock;

    NXT_ThemeRenderer_SetAppliedEffect(renderer,
                         renderer->pActiveEffect,
                         NXT_EffectType_Title,
                         nodeData->kedlitem->uid,  // ID of clip effect, or NULL to clear current effect
                         NULL,
                         nodeData->effect_index_,         // Index of this clip, from 0
                         nodeData->effect_count_,    // Total number of clips in the project
                         renderer->actualEffectStartCTS,     // Start time for this clip
                         renderer->actualEffectEndCTS,       // End time for this clip
                         renderer->actualEffectStartCTS,   // Start time for this effect (must be >= clipStartTime)
                         renderer->actualEffectEndCTS);

//Do something to render kedl item
    NXT_ThemeRenderer_TimeClipEffect(renderer, nodeData->kedlitem->effect, renderer->currentTime, renderer->actualEffectStartCTS, renderer->actualEffectEndCTS);
    NXT_ThemeRenderer_RenderEffect(renderer, nodeData->kedlitem->effect, renderer->currentTime);

    renderer->pActiveEffect = pstate_backup;

    renderer->popVideoTrack();
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
    return NXT_NodeAction_Next;
}