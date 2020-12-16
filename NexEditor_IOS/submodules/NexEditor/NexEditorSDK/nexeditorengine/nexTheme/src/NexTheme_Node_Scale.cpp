#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Scale"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Scale = {
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
    "Scale",
    sizeof(NXT_Node_Scale)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
    nodeData->target = NXT_TransformTarget_Vertex;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
    /*if( strcasecmp(attrName, "factor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->factor.e, sizeof(nodeData->factor.e)/sizeof(*(nodeData->factor.e)) );*/
    if( strcasecmp(attrName, "x")==0 ) {
        if( (nodeData->components & NXT_VectorComponent_X)==0 ) {
            NXT_AnimFloatVectorFromString( node, attrValue, &(nodeData->factor.e[0]), 1 );
            nodeData->components |= NXT_VectorComponent_X;
        }
    } else if( strcasecmp(attrName, "y")==0 ) {
        if( (nodeData->components & NXT_VectorComponent_Y)==0 ) {
            NXT_AnimFloatVectorFromString( node, attrValue, &(nodeData->factor.e[1]), 1 );
            nodeData->components |= NXT_VectorComponent_Y;
        }
    } else if( strcasecmp(attrName, "z")==0 ) {
        if( (nodeData->components & NXT_VectorComponent_Z)==0 ) {
            NXT_AnimFloatVectorFromString( node, attrValue, &(nodeData->factor.e[2]), 1 );
            nodeData->components |= NXT_VectorComponent_Z;
        }
    } else if( strcasecmp(attrName, "uniform")==0 ) {
        if( nodeData->components==0) {
            NXT_AnimFloatVectorFromString( node, attrValue, &(nodeData->factor.e[0]), 1 );
            nodeData->components |= (NXT_VectorComponent_X | NXT_VectorComponent_Y | NXT_VectorComponent_Z | NXT_VectorComponent_Uniform);
        }
    } else if( strcasecmp(attrName, "inverse")==0 ) {
        if( strcasecmp(attrValue,"")==0  || strcasecmp(attrValue,"on")==0 || strcasecmp(attrValue,"yes")==0 || strcasecmp(attrValue,"1")==0 ) {
            nodeData->bInverse = 1;
        } else if( strcasecmp(attrValue,"no")==0 || strcasecmp(attrValue,"off")==0 || strcasecmp(attrValue,"0")==0 ) {
            nodeData->bInverse = 0;
        }
    } else if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "vertex")==0 ) {
            nodeData->target = NXT_TransformTarget_Vertex;
        } else if( strcasecmp(attrValue, "texture")==0 ) {
            nodeData->target = NXT_TransformTarget_Texture;
        } else if( strcasecmp(attrValue, "mask")==0 ) {
            nodeData->target = NXT_TransformTarget_Mask;
        }
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
    LOGD( "(Scale)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
    
    if( nodeData->components & NXT_VectorComponent_Uniform ) {
        nodeData->factor.e[1] = nodeData->factor.e[0];
        nodeData->factor.e[2] = nodeData->factor.e[0];
        nodeData->factor.e[3] = 1.0;
    } else {
        if( (nodeData->components & NXT_VectorComponent_X)==0 ) {
            nodeData->factor.e[0] = 1.0;
        }
        if( (nodeData->components & NXT_VectorComponent_Y)==0 ) {
            nodeData->factor.e[1] = 1.0;
        }
        if( (nodeData->components & NXT_VectorComponent_Z)==0 ) {
            nodeData->factor.e[2] = 1.0;
        }
        nodeData->factor.e[3] = 1.0;
    }
    if( nodeData->bInverse ) {
        nodeData->factor.e[0] = 1.0/nodeData->factor.e[0];
        nodeData->factor.e[1] = 1.0/nodeData->factor.e[1];
        nodeData->factor.e[2] = 1.0/nodeData->factor.e[2];
        nodeData->factor.e[3] = 1.0/nodeData->factor.e[3];
    }
    
    switch (nodeData->target) {
        case NXT_TransformTarget_Texture:
            nodeData->saved_transform = renderer->texture_transform;
            renderer->texture_transform = NXT_Matrix4f_MultMatrix(renderer->texture_transform,NXT_Matrix4f_Scale(nodeData->factor.e[0], nodeData->factor.e[1], 1.0 ));
            break;
        case NXT_TransformTarget_Mask:
            nodeData->saved_transform = renderer->mask_transform;
            renderer->mask_transform = NXT_Matrix4f_MultMatrix(renderer->mask_transform,NXT_Matrix4f_Scale(nodeData->factor.e[0], nodeData->factor.e[1], 1.0 ));
            break;
        case NXT_TransformTarget_Vertex:
        default:
            nodeData->saved_transform = renderer->transform;
            renderer->transform = NXT_Matrix4f_MultMatrix(renderer->transform,NXT_Matrix4f_Scale(nodeData->factor.e[0], nodeData->factor.e[1], 1.0 ));
            break;
    }
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Scale *nodeData = (NXT_Node_Scale*)node;
    switch (nodeData->target) {
        case NXT_TransformTarget_Texture:
            renderer->texture_transform = nodeData->saved_transform;
            break;
        case NXT_TransformTarget_Mask:
            renderer->mask_transform = nodeData->saved_transform;
            break;
        case NXT_TransformTarget_Vertex:
        default:
            renderer->transform = nodeData->saved_transform;
            break;
    }
    return NXT_NodeAction_Next;
}
