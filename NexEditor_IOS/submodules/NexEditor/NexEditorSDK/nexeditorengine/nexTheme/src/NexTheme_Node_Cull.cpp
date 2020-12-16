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

NXT_NodeClass NXT_NodeClass_Cull = {
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
    "Cull",
    sizeof(NXT_Node_Cull)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Cull *nodeData = (NXT_Node_Cull*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Cull *nodeData = (NXT_Node_Cull*)node;
    if( strcasecmp(attrName, "face")==0 ) {
        if( strcasecmp(attrValue, "none")==0 ) {
            nodeData->cullFace = NXT_CullFace_None;
        } else if( strcasecmp(attrValue, "back")==0 ) {
            nodeData->cullFace = NXT_CullFace_Back;
        } else if( strcasecmp(attrValue, "front")==0 ) {
            nodeData->cullFace = NXT_CullFace_Front;
        } else if( strcasecmp(attrValue, "all")==0 ) {
            nodeData->cullFace = NXT_CullFace_All;
        }
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Cull *nodeData = (NXT_Node_Cull*)node;
    LOGD( "(Cull face=%d)", nodeData->cullFace );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Cull *nodeData = (NXT_Node_Cull*)node;
    nodeData->prevCullFace = renderer->cullFace;
    renderer->cullFace = nodeData->cullFace;
    switch( renderer->cullFace ) {
        case NXT_CullFace_None:
            glDisable(GL_CULL_FACE);
            break;
        case NXT_CullFace_Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case NXT_CullFace_Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case NXT_CullFace_All:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
    }
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Cull *nodeData = (NXT_Node_Cull*)node;
    renderer->cullFace = nodeData->prevCullFace;
    switch( renderer->cullFace ) {
        case NXT_CullFace_None:
            glDisable(GL_CULL_FACE);
            break;
        case NXT_CullFace_Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case NXT_CullFace_Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case NXT_CullFace_All:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
    }
    return NXT_NodeAction_Next;
}
