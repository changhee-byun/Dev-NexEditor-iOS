#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Rotate"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Rotate = {
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
    "Rotate",
    sizeof(NXT_Node_Rotate)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
    nodeData->target = NXT_TransformTarget_Vertex;
    nodeData->axis.e[2] = 1.0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
    if( strcasecmp(attrName, "axis")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->axis.e, sizeof(nodeData->axis.e)/sizeof(*(nodeData->axis.e)) );
    } else if( strcasecmp(attrName, "angle")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->angle, 1 );
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
    //NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
    //LOGV( "(Rotate %f degrees around %f %f %f %f)", nodeData->angle,
    //       nodeData->axis.e[0], nodeData->axis.e[1], nodeData->axis.e[2], nodeData->axis.e[3] );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
    NXT_Matrix4f transform = NXT_Matrix4f_Rotate(nodeData->axis,nodeData->angle*0.0174532925);
    switch (nodeData->target) {
        case NXT_TransformTarget_Texture:
            nodeData->saved_transform = renderer->texture_transform;
            renderer->texture_transform = NXT_Matrix4f_MultMatrix(renderer->texture_transform,transform);
            break;
        case NXT_TransformTarget_Mask:
            nodeData->saved_transform = renderer->mask_transform;
            renderer->mask_transform = NXT_Matrix4f_MultMatrix(renderer->mask_transform,transform);
            break;
        case NXT_TransformTarget_Vertex:
        default:
            nodeData->saved_transform = renderer->transform;
            renderer->transform = NXT_Matrix4f_MultMatrix(renderer->transform,transform);
            break;
    }
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Rotate *nodeData = (NXT_Node_Rotate*)node;
    //LOGV("Rotate-END");
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
