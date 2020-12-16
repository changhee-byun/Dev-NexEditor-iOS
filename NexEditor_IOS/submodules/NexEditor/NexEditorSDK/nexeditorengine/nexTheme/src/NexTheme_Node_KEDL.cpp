#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_KEDL"

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
static int nodePrecacheFunc(NXT_NodeHeader *node, NXT_HThemeRenderer renderer, NXT_PrecacheMode precacheMode, int* asyncmode, int* max_replaceable);
static int nodeGetPrecacheResource(NXT_NodeHeader* node, NXT_HThemeRenderer renderer, void* load_resource_list);
static void nodeFreeTextureFunc(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node);

NXT_NodeClass NXT_NodeClass_KEDL = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    nodePrecacheFunc,//precacheFunc
    nodeGetPrecacheResource,//getPrecacheResourceFunc
    NULL,//nodeUpdateNotify
    nodeFreeTextureFunc,//freeTextureFunc
    "kedlcomp",
    sizeof(NXT_Node_KEDL)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;

    if( nodeData->uid ){
        free( nodeData->uid );
        nodeData->uid = NULL;
    }
}

static void nodeFreeTextureFunc(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node)
{
    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;

    if(nodeData->uid){
        
        NXT_ThemeRenderer_ReleaseKEDLEffectIDByContext(context, nodeData->uid, 0);
        nodeData->themeset = NULL;
        nodeData->effect = NULL;
    }
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;
    if( strcasecmp(attrName, "uid")==0 ) {
        if( nodeData->uid ) {
            free( nodeData->uid );
        }
        if( *attrValue=='@' )
            attrValue++;
        nodeData->uid = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->uid, attrValue);
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;
    LOGD( "(KEDL uid='%s')", nodeData->uid );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;
        
    if(nodeData->themeset == NULL && renderer->loadFileCallback) {

        int asyncmode = 0;
        int dummy_size = 100000000;
        nodePrecacheFunc(node, renderer, NXT_PrecacheMode_Upload, &asyncmode, &dummy_size);
    }

    if(nodeData->themeset && nodeData->effect == NULL){

        nodeData->effect = NXT_ThemeSet_GetEffectById(nodeData->themeset, nodeData->uid);
    }
}

static int nodeGetPrecacheResource(NXT_NodeHeader* node, NXT_HThemeRenderer renderer, void* load_resource_list){

    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;

    LOGFORCE("TM30 KEDL precache %s", nodeData->uid);
    
    nodeData->themeset = NXT_ThemeRenderer_GetKEDLEffectID(renderer, nodeData->uid);
        
    if(nodeData->themeset == NULL && renderer->loadFileCallback) {

        char* poutput = NULL;
        int length = 0;

        if(0 == renderer->loadFileCallback(&poutput, &length, nodeData->uid, renderer->imageCallbackPvtData)){

            nodeData->themeset = NXT_ThemeRenderer_GetKEDLItem(renderer, nodeData->uid, poutput);
            if(poutput)
                delete[] poutput;
            poutput = NULL;
        }
        nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
    }

    if(nodeData->themeset && nodeData->effect == NULL){

        nodeData->effect = NXT_ThemeSet_GetEffectById(nodeData->themeset, nodeData->uid);
        if(nodeData->effect)
            if(NXT_Theme_GetPrecacheResource(nodeData->effect, renderer, load_resource_list))
                return 1;
    }

    return 0;
}

static int nodePrecacheFunc(NXT_NodeHeader *node, NXT_HThemeRenderer renderer, NXT_PrecacheMode precacheMode, int* asyncmode, int* max_replaceable) {

    NXT_Node_KEDL *nodeData = (NXT_Node_KEDL*)node;

    LOGFORCE("TM30 KEDL precache %s", nodeData->uid);
    
    if(nodeData->themeset == NULL)
        nodeData->themeset = NXT_ThemeRenderer_GetKEDLEffectID(renderer, nodeData->uid);
        
    if(nodeData->themeset == NULL && renderer->loadFileCallback) {

        char* poutput = NULL;
        int length = 0;

        if(0 == renderer->loadFileCallback(&poutput, &length, nodeData->uid, renderer->imageCallbackPvtData)){

            nodeData->themeset = NXT_ThemeRenderer_GetKEDLItem(renderer, nodeData->uid, poutput);
            if(poutput)
                delete[] poutput;
            poutput = NULL;
        }
        nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
    }

    if(nodeData->themeset && nodeData->effect == NULL){

        nodeData->effect = NXT_ThemeSet_GetEffectById(nodeData->themeset, nodeData->uid);
        if(nodeData->effect)
            if(NXT_Theme_DoPrecache(nodeData->effect, renderer, asyncmode, max_replaceable))
                return 1;
    }

    if(nodeData->effect)
        nodeData->total_video_slot = NXT_Theme_GetTotalVideoSlot(nodeData->effect);

    LOGFORCE("TM30 KEDL precache %s total video slot:%d", nodeData->uid, nodeData->total_video_slot);
    return 0;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}