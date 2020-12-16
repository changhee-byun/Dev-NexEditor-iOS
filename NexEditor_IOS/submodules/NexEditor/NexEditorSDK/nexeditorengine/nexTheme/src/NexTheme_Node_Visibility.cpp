#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Visibility = {
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
    "Visibility",
    sizeof(NXT_Node_Visibility)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Visibility *nodeData = (NXT_Node_Visibility*)node;
    
    nodeData->startTime = 0.0f;
    nodeData->endTime = 1.0f;
	nodeData->bUseCheckValue = 0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Visibility *nodeData = (NXT_Node_Visibility*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Visibility *nodeData = (NXT_Node_Visibility*)node;
    if( strcasecmp(attrName, "start")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->startTime, 1 );
    } else if( strcasecmp(attrName, "end")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->endTime, 1 );
    } else if( strcasecmp(attrName, "check")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->check, 1 );
		nodeData->bUseCheckValue = 1;
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Visibility *nodeData = (NXT_Node_Visibility*)node;
    LOGD( "(Visibility from=%f to=%f)", nodeData->startTime, nodeData->endTime );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Visibility *nodeData = (NXT_Node_Visibility*)node;
	if( nodeData->bUseCheckValue ) {
		if( nodeData->check < nodeData->startTime || nodeData->check > nodeData->endTime ) {
			renderer->bSkipChildren = 1;
		}
	} else {
		if( renderer->time < nodeData->startTime || renderer->time > nodeData->endTime ) {
			renderer->bSkipChildren = 1;
		}
	}
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
