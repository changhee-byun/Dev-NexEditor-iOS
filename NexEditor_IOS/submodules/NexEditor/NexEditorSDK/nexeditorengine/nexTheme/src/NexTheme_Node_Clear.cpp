#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Clear"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Clear = {
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
    "Clear",
    sizeof(NXT_Node_Clear)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
    nodeData->bStencil = 0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
    if( strcasecmp(attrName, "color")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->color.e, sizeof(nodeData->color.e)/sizeof(*(nodeData->color.e)) );
    } else if( strcasecmp(attrName, "stencil")==0 ) {
        nodeData->bStencil = 1;
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
    LOGD( "(Clear)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
    
    if( nodeData->bStencil ) {
        glClearStencil(0);
        glClearStencil(GL_STENCIL_BUFFER_BIT);
        return;
    }

    glClearColor(nodeData->color.e[0],
                     nodeData->color.e[1],
                     nodeData->color.e[2],
                     nodeData->color.e[3]);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    // NexThemeRenderer_PrepStateForRender(renderer,NULL,NULL,NULL,NULL, 0);
    
    // GLfloat vertexCoord[] = {
    //     -1.0,     -1.0,
    //     1.0,      -1.0,
    //     -1.0,     1.0,   
    //     1.0,      1.0
    // };
    
    // /*GLfloat vertexColor[] = {
    //     1.0, 0.0, 0.0, 1.0,
    //     0.0, 1.0, 0.0, 1.0,
    //     0.0, 0.0, 1.0, 1.0,
    //     1.0, 1.0, 0.0, 1.0,
    // };*/
    // GLfloat vertexColor[] = {
    //     nodeData->color.e[0],nodeData->color.e[1],nodeData->color.e[2],nodeData->color.e[3],
    //     nodeData->color.e[0],nodeData->color.e[1],nodeData->color.e[2],nodeData->color.e[3],
    //     nodeData->color.e[0],nodeData->color.e[1],nodeData->color.e[2],nodeData->color.e[3],
    //     nodeData->color.e[0],nodeData->color.e[1],nodeData->color.e[2],nodeData->color.e[3]
    // };
    
    // glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 2, GL_FLOAT, 0, 0, vertexCoord );
    // CHECK_GL_ERROR();
    // glVertexAttribPointer(renderer->pSolidShaderActive->a_color, 4, GL_FLOAT, 0, 0, vertexColor );
    // CHECK_GL_ERROR();

    // glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    // CHECK_GL_ERROR();
    
    
    
    /*if( renderer->outputType == NXT_RendererOutputType_YUVA_8888 || renderer->outputType == NXT_RendererOutputType_Y2CrA_8888 ) {
        NXT_Vector4f yuvacolor = NXT_RGBtoYUV( nodeData->color );
        glClearColor(yuvacolor.e[0],
                     yuvacolor.e[1],
                     yuvacolor.e[2],
                     yuvacolor.e[3]);
    } else {
        glClearColor(nodeData->color.e[0],
                     nodeData->color.e[1],
                     nodeData->color.e[2],
                     nodeData->color.e[3]);
    }
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);*/
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
//    NXT_Node_Clear *nodeData = (NXT_Node_Clear*)node;
    return NXT_NodeAction_Next;
}
