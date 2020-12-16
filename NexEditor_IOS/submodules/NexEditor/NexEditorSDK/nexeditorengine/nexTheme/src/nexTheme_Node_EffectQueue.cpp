#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_EffectQueue"

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

NXT_NodeClass NXT_NodeClass_EffectQueue = {
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
    "effectqueue",
    sizeof(NXT_Node_KEDL)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {

}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
     

}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {

    renderer->bSkipChildren = 1;
    //find transition from child-nodes and apply rendertarget for trnasition's child nodes
    //find effect from child-nodes and do effect render
    //
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
    return NXT_NodeAction_Next;
}