#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Projection"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Projection = {
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
    "Projection",
    sizeof(NXT_Node_Projection)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
    nodeData->left = -1.0;
    nodeData->right = 1.0;
    nodeData->bottom = -1.0;
    nodeData->top = 1.0;
    nodeData->near = 0.01;
    nodeData->far = 10.0;
    nodeData->fov = 60.0;
    nodeData->type = NXT_ProjectionType_Perspective;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
    if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "ortho")==0 ) {
            nodeData->type = NXT_ProjectionType_Ortho;
        } else if( strcasecmp(attrValue, "perspective")==0 ) {
            nodeData->type = NXT_ProjectionType_Perspective;
        } else if( strcasecmp(attrValue, "frustum")==0 ) {
            nodeData->type = NXT_ProjectionType_Frustum;
        } else if( strcasecmp(attrValue, "title-bottom")==0 ) {
            nodeData->type = NXT_ProjectionType_TitleBottom;
        } else if( strcasecmp(attrValue, "title-top")==0 ) {
            nodeData->type = NXT_ProjectionType_TitleTop;
        } else if( strcasecmp(attrValue, "title-left")==0 ) {
            nodeData->type = NXT_ProjectionType_TitleLeft;
        } else if( strcasecmp(attrValue, "title-right")==0 ) {
            nodeData->type = NXT_ProjectionType_TitleRight;
        } else if( strcasecmp(attrValue, "title-middle")==0 ) {
            nodeData->type = NXT_ProjectionType_TitleMiddle;
        }
    } else if( strcasecmp(attrName, "left")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->left, 1 );
    } else if( strcasecmp(attrName, "right")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->right, 1 );
    } else if( strcasecmp(attrName, "top")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->top, 1 );
    } else if( strcasecmp(attrName, "bottom")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->bottom, 1 );
    } else if( strcasecmp(attrName, "near")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->near, 1 );
    } else if( strcasecmp(attrName, "far")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->far, 1 );
    } else if( strcasecmp(attrName, "fov")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->fov, 1 );
    } else if( strcasecmp(attrName, "aspect")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->aspect, 1 );
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
    LOGD( "(Projection; type=%d)", nodeData->type );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
    nodeData->saved_proj = renderer->proj;
    
    float aspect;
        
    if( nodeData->aspect==0.0 ) {
        if( renderer->view_width > 0 && renderer->view_height > 0 ) {
            aspect = (float)renderer->view_width / (float)renderer->view_height;
        } else {
            aspect = 4.0/3.0;
            
        }
    } else {
        aspect = nodeData->aspect;
    }
    
    LOGD( "type=%d, aspect=%f(%f) near=%f far=%f fov=%f", nodeData->type, aspect, nodeData->aspect, nodeData->near, nodeData->far, nodeData->fov );

    
    switch( nodeData->type ) {
        case NXT_ProjectionType_Perspective:
            renderer->proj = NXT_Matrix4f_Perspective(nodeData->fov, aspect, 
                                                      nodeData->near, nodeData->far); 
            break;
        case NXT_ProjectionType_Frustum:
            renderer->proj = NXT_Matrix4f_Frustum(nodeData->left, nodeData->right,
                                                  nodeData->bottom, nodeData->top,
                                                  nodeData->near, nodeData->far); 
            break;
        case NXT_ProjectionType_Ortho:
            renderer->proj = NXT_Matrix4f_Ortho(nodeData->left, nodeData->right,
                                                nodeData->bottom, nodeData->top,
                                                nodeData->near, nodeData->far); 
            break;
        case NXT_ProjectionType_TitleTop:
        {
            NXT_Matrix4f proj = NXT_Matrix4f_Ortho(-aspect, aspect,
                                                   -1.0, 1.0,
                                                   1.0, 10.0);
            NXT_Matrix4f translate = NXT_Matrix4f_Translate( NXT_Vector4f( -aspect, 1.0, 0.0, 0.0 ) );
            NXT_Matrix4f scale = NXT_Matrix4f_Scale( 2.0*aspect, 2.0*aspect, 1.0 );
            NXT_Matrix4f view = NXT_Matrix4f_MultMatrix( scale, translate );
            
            renderer->proj =  NXT_Matrix4f_MultMatrix( view, proj );
            break;
        }
        case NXT_ProjectionType_TitleMiddle:
        {
            NXT_Matrix4f proj = NXT_Matrix4f_Ortho(-aspect, aspect,
                                                   -1.0, 1.0,
                                                   1.0, 10.0);
            NXT_Matrix4f translate = NXT_Matrix4f_Translate( NXT_Vector4f( 0.0, 0.0, 0.0, 0.0 ) );
            NXT_Matrix4f scale = NXT_Matrix4f_Scale( 2.0*aspect, 2.0*aspect, 1.0 );
            NXT_Matrix4f view = NXT_Matrix4f_MultMatrix( scale, translate );
            
            renderer->proj =  NXT_Matrix4f_MultMatrix( view, proj );
            break;
        }
        case NXT_ProjectionType_TitleBottom:
        {
            NXT_Matrix4f proj = NXT_Matrix4f_Ortho(-aspect, aspect,
                                                   -1.0, 1.0,
                                                   1.0, 10.0);
            NXT_Matrix4f translate = NXT_Matrix4f_Translate( NXT_Vector4f( -aspect, -1.0, 0.0, 0.0 ) );
            NXT_Matrix4f scale = NXT_Matrix4f_Scale( 2.0*aspect, 2.0*aspect, 1.0 );
            NXT_Matrix4f view = NXT_Matrix4f_MultMatrix( scale, translate );
            
            renderer->proj =  NXT_Matrix4f_MultMatrix( view, proj );
            
            /*LOGI("[%s %d] proj={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}", __func__, __LINE__, proj.e[0], proj.e[1], proj.e[2], proj.e[3], proj.e[4], proj.e[5], proj.e[6], proj.e[7], proj.e[8], proj.e[9], proj.e[10], proj.e[11], proj.e[12], proj.e[13], proj.e[14], proj.e[15]);
            LOGI("[%s %d] translate={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}", __func__, __LINE__, translate.e[0], translate.e[1], translate.e[2], translate.e[3], translate.e[4], translate.e[5], translate.e[6], translate.e[7], translate.e[8], translate.e[9], translate.e[10], translate.e[11], translate.e[12], translate.e[13], translate.e[14], translate.e[15]);
            LOGI("[%s %d] scale={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}", __func__, __LINE__, scale.e[0], scale.e[1], scale.e[2], scale.e[3], scale.e[4], scale.e[5], scale.e[6], scale.e[7], scale.e[8], scale.e[9], scale.e[10], scale.e[11], scale.e[12], scale.e[13], scale.e[14], scale.e[15]);
            LOGI("[%s %d] view={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}", __func__, __LINE__, view.e[0], view.e[1], view.e[2], view.e[3], view.e[4], view.e[5], view.e[6], view.e[7], view.e[8], view.e[9], view.e[10], view.e[11], view.e[12], view.e[13], view.e[14], view.e[15]);
            LOGI("[%s %d] renderer->proj={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}", __func__, __LINE__, renderer->proj.e[0], renderer->proj.e[1], renderer->proj.e[2], renderer->proj.e[3], renderer->proj.e[4], renderer->proj.e[5], renderer->proj.e[6], renderer->proj.e[7], renderer->proj.e[8], renderer->proj.e[9], renderer->proj.e[10], renderer->proj.e[11], renderer->proj.e[12], renderer->proj.e[13], renderer->proj.e[14], renderer->proj.e[15]);*/

            /*NXT_Matrix4f proj = NXT_Matrix4f_Frustum(-aspect, aspect,
                                                     -1.0, 1.0,
                                                     1.0, 10.0);
            NXT_Matrix4f view = NXT_Matrix4f_Translate( NXT_Vector4f( -aspect, -1.0, 0.0, 0.0 ) );
            
            renderer->proj = NXT_Matrix4f_MultMatrix( view, proj );*/
            
            break;
        }
        case NXT_ProjectionType_TitleLeft:
        {
            NXT_Matrix4f proj = NXT_Matrix4f_Ortho(-aspect, aspect,
                                                   -1.0, 1.0,
                                                   1.0, 10.0);
            NXT_Matrix4f translate = NXT_Matrix4f_Translate( NXT_Vector4f( -aspect, -1.0, 0.0, 0.0 ) );
            NXT_Matrix4f scale = NXT_Matrix4f_Scale( 2.0, 2.0, 1.0 );
            NXT_Matrix4f view = NXT_Matrix4f_MultMatrix( scale, translate );
            
            renderer->proj =  NXT_Matrix4f_MultMatrix( view, proj );
            
            break;
        }
        case NXT_ProjectionType_TitleRight:
        {
            NXT_Matrix4f proj = NXT_Matrix4f_Ortho(-aspect, aspect,
                                                   -1.0, 1.0,
                                                   1.0, 10.0);
            NXT_Matrix4f translate = NXT_Matrix4f_Translate( NXT_Vector4f( aspect, -1.0, 0.0, 0.0 ) );
            NXT_Matrix4f scale = NXT_Matrix4f_Scale( 2.0, 2.0, 1.0 );
            NXT_Matrix4f view = NXT_Matrix4f_MultMatrix( scale, translate );
            
            renderer->proj =  NXT_Matrix4f_MultMatrix( view, proj );
            
            break;
        }
        default:
        {
            break;
        }
    }
    
    //renderer->proj = NXT_Matrix4f_Perspective(60.0, 1.3333f, 0.1f, 10.0f); 
    
    //NXT_Matrix4f_MultMatrix(transform,renderer->transform);
    
    //renderer->proj = NXT_Matrix4f_Frustum( -1.0, 1.0, -1.0, 1.0, 0.001, 2.0 );
    
    /*renderer->proj = (NXT_Matrix4f){
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        }
    };*/
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Projection *nodeData = (NXT_Node_Projection*)node;
    renderer->proj = nodeData->saved_proj;
    return NXT_NodeAction_Next;
}
