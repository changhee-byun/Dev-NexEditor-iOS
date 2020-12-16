#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
//#include "NexThemeRenderer_Internal.h"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_TimingFunc = {
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
    "TimingFunction",
    sizeof(NXT_Node_TimingFunc)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_TimingFunc *tf = (NXT_Node_TimingFunc*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_TimingFunc *tf = (NXT_Node_TimingFunc*)node;
    if( strcasecmp(attrName, "values")==0 ) {
        NXT_FloatVectorFromString( attrValue, tf->e, sizeof(tf->e)/sizeof(*(tf->e)) );
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_TimingFunc *tf = (NXT_Node_TimingFunc*)node;
    LOGD( "(TimingFunction; %f %f %f %f)", tf->e[0], tf->e[1], tf->e[2], tf->e[3] );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
