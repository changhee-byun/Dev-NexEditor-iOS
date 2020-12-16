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

NXT_NodeClass NXT_NodeClass_Point = {
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
    "Point",
    sizeof(NXT_Node_Point)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Point *nodeData = (NXT_Node_Point*)node;
    if( nodeData->location )
        free(nodeData->location);
    if( nodeData->texcoord )
        free(nodeData->texcoord);
    if( nodeData->maskcoord )
        free(nodeData->maskcoord);
    if( nodeData->color )
        free(nodeData->color);
    if( nodeData->normal )
        free(nodeData->normal);
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Point *nodeData = (NXT_Node_Point*)node;
    if( strcasecmp(attrName, "location")==0 ) {
        if( nodeData->location )
            free(nodeData->location);
        nodeData->location = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->location, attrValue);
    } else if( strcasecmp(attrName, "texcoord")==0 ) {
        if( nodeData->texcoord )
            free(nodeData->texcoord);
        nodeData->texcoord = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->texcoord, attrValue);
    } else if( strcasecmp(attrName, "maskcoord")==0 ) {
        if( nodeData->maskcoord )
            free(nodeData->maskcoord);
        nodeData->maskcoord = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->maskcoord, attrValue);
    } else if( strcasecmp(attrName, "color")==0 ) {
        if( nodeData->color )
            free(nodeData->color);
        nodeData->color = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->color, attrValue);
    } else if( strcasecmp(attrName, "normal")==0 ) {
        if( nodeData->normal )
            free(nodeData->normal);
        nodeData->normal = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->normal, attrValue);
    } else {
        // TODO: Error
    }
    
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Point *nodeData = (NXT_Node_Point*)node;
    LOGD( "(Point)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
