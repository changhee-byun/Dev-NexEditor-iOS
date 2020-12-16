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

NXT_NodeClass NXT_NodeClass_CoordinateSpace = {
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
    "CoordinateSpace",
    sizeof(NXT_Node_CoordinateSpace)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_CoordinateSpace *nodeData = (NXT_Node_CoordinateSpace*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_CoordinateSpace *nodeData = (NXT_Node_CoordinateSpace*)node;
    if( strcasecmp(attrName, "width")==0 ) {
        NXT_FloatVectorFromString(attrValue, &nodeData->width, 1);
        if( nodeData->applyTo == NXT_CoordType_None ) {
            nodeData->applyTo = NXT_CoordType_Vertex;
        }
    } else if( strcasecmp(attrName, "height")==0 ) {
        NXT_FloatVectorFromString(attrValue, &nodeData->height, 1);
        if( nodeData->applyTo == NXT_CoordType_None ) {
            nodeData->applyTo = NXT_CoordType_Vertex;
        }
    } else if( strcasecmp(attrName, "origin")==0 ) {
        float origin[] = {0,0};
        NXT_FloatVectorFromString(attrValue, origin, 2);
        nodeData->originx = origin[0];
        nodeData->originy = origin[1];
        if( nodeData->applyTo == NXT_CoordType_None ) {
            nodeData->applyTo = NXT_CoordType_Vertex;
        }
    } else if( strcasecmp(attrName, "yup")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUFalse:
                nodeData->bYUp = 0;
                break;
            case TFUTrue:
                nodeData->bYUp = 1;
                break;
            default :
                break;
        }
        if( nodeData->applyTo == NXT_CoordType_None ) {
            nodeData->applyTo = NXT_CoordType_Vertex;
        }
    } else if( strcasecmp(attrName, "applyto")==0 ) {
        if( strcasecmp(attrValue, "vertex")==0 )  {
            nodeData->applyTo = NXT_CoordType_Vertex;
        } else if( strcasecmp(attrValue, "mask")==0 ) {
            nodeData->applyTo = NXT_CoordType_Mask;
        } else if( strcasecmp(attrValue, "texture")==0 ) {
            nodeData->applyTo = NXT_CoordType_Texture;
        }
    } else {
        // TODO: Error
    }
    
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_CoordinateSpace *nodeData = (NXT_Node_CoordinateSpace*)node;
    LOGD( "(CoordinateSpace)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    //NXT_Node_CoordinateSpace *nodeData = (NXT_Node_CoordinateSpace*)node;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}

void NXT_CoordSpace_Adjust( NXT_Node_CoordinateSpace* coordSpace, float* coord, int index ) {
    if( !coordSpace )
        return;
    switch( index ) {
        case 0:         // X coord
            if( coordSpace->applyTo==NXT_CoordType_Vertex ) {
                *coord = (*coord + coordSpace->originx) / coordSpace->width * 2.0 - 1.0;
            } else {
                *coord = (*coord + coordSpace->originx) / coordSpace->width;
            }
            break;
        case 1:         // Y coord
            if( coordSpace->bYUp ) {
                if( coordSpace->applyTo==NXT_CoordType_Vertex ) {
                    *coord = (*coord + coordSpace->originy) / coordSpace->height * 2.0 - 1.0;
                } else {
                    *coord = (*coord + coordSpace->originy) / coordSpace->height;
                }
            } else {
                if( coordSpace->applyTo==NXT_CoordType_Vertex ) {
                    *coord = ((coordSpace->height-*coord) + coordSpace->originy) / coordSpace->height * 2.0 - 1.0;
                } else {
                    *coord = ((coordSpace->height-*coord) + coordSpace->originy) / coordSpace->height;
                }
            }
            break;
        case 2:         // Z coord
        case 3:         // W coord
        default:
            break;
    }
}
