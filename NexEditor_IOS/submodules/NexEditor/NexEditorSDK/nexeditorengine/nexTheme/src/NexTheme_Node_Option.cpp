#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Option = {
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
    "Option",
    sizeof(NXT_Node_Option)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
//    NXT_Node_FieldLabel *nodeData = (NXT_Node_FieldLabel*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
//    NXT_Node_FieldLabel *nodeData = (NXT_Node_FieldLabel*)node;
    if( strcasecmp(attrName, "value")==0 ) {
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    /*NXT_Node_FieldLabel *nodeData = (NXT_Node_FieldLabel*)node;
    LOGD( "(Const value=%s)", nodeData->value );*/
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}