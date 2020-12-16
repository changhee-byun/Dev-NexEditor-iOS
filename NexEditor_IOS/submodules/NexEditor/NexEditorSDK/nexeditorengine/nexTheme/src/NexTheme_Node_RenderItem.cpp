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

NXT_NodeClass NXT_NodeClass_RenderItem = {
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
    "renderitemcomp",
    sizeof(NXT_Node_RenderItem)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_RenderItem *nodeData = (NXT_Node_RenderItem*)node;

    if( nodeData->src ) {
        free( nodeData->src );
        nodeData->src = NULL;
    }

    if( nodeData->path ) {
        free( nodeData->path );
        nodeData->path = NULL;
    }

    if( nodeData->uid ){
        free( nodeData->uid );
        nodeData->uid = NULL;
    }
}

static int getPointOfDelimiter(const char* value, const char delimiter){

    int len = (int)strlen(value);
    int i = 0;
    for(i = len - 1; i > -1; --i){

        if(value[i] == delimiter)
            return i;
    }
    return len;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_RenderItem *nodeData = (NXT_Node_RenderItem*)node;
    if( strcasecmp(attrName, "src")==0 ) {
        
        if( nodeData->src ) {
            free( nodeData->src );
        }

        if( nodeData->path ) {
            free( nodeData->path );
        }
        
        const char *base_id = "";
        NXT_NodeHeader *pn;
        
        for( pn = node; pn; pn = pn->parent ) {
            if( pn->isa==&NXT_NodeClass_Effect || pn->isa==&NXT_NodeClass_Theme ) {
                base_id = pn->node_id;
            }
        }
        
        //char *tag = "";
        int attrlen = (int)strlen(attrValue);
        int taglen = 0;//(int)strlen(tag);
        int baseidlen = (int)strlen(base_id);
        nodeData->src = (char*)malloc(taglen + attrlen + baseidlen + 2);
        
        //strcpy(nodeData->path, tag);
        strcpy(nodeData->src + taglen, base_id);
        strcpy(nodeData->src + taglen + baseidlen, "/");
        strcpy(nodeData->src + taglen + baseidlen + 1, attrValue);

        int attr2len = getPointOfDelimiter(attrValue, '/');
        char attr2[256] = "";
        if(attr2len > 0){

            memcpy(attr2, attrValue, attr2len * sizeof(char));
            attr2[attr2len] = 0;
        }       

        nodeData->path = (char*)malloc(baseidlen + attr2len + 2);
        strcpy(nodeData->path, base_id);
        if(attr2len > 0){

            strcpy(nodeData->path + baseidlen, "/");
            strcpy(nodeData->path + baseidlen + 1, attr2);
        }
    }
    else if( strcasecmp(attrName, "uid")==0 ) {
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
    NXT_Node_RenderItem *nodeData = (NXT_Node_RenderItem*)node;
    LOGD( "(RenderItem src='%s')", nodeData->src );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_RenderItem *nodeData = (NXT_Node_RenderItem*)node;
    
    nodeData->id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, nodeData->uid);
        
    if( nodeData->id < 0 && renderer->loadFileCallback ) {

        char* poutput = NULL;
        int length = 0;

        if(0 == renderer->loadFileCallback(&poutput, &length, nodeData->uid, renderer->imageCallbackPvtData)){

            if(poutput){

                NXT_ThemeRenderer_GetRenderItem(renderer, nodeData->uid, nodeData->uid, poutput, 0, renderer->loadFileCallback, renderer->imageCallbackPvtData);
                nodeData->id = NXT_ThemeRenderer_GetRenderItemEffectID(renderer, nodeData->uid);
                if(poutput)
                    delete[] poutput;
                poutput = NULL;
            }
        }
        nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
    }
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}