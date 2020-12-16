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

NXT_NodeClass NXT_NodeClass_Part = {
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
    "Part",
    sizeof(NXT_Node_Part)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
//    NXT_Node_Part *nodeData = (NXT_Node_Part*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Part *nodeData = (NXT_Node_Part*)node;
    if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "in")==0 ) {
            nodeData->type = NXT_PartType_In;
        } else if( strcasecmp(attrValue, "out")==0 ) {
            nodeData->type = NXT_PartType_Out;
        } else if( strcasecmp(attrValue, "mid")==0 ) {
            nodeData->type = NXT_PartType_Mid;
        } else if( strcasecmp(attrValue, "all")==0 ) {
            nodeData->type = NXT_PartType_All;
        }
    } else if( strcasecmp(attrName, "clip")==0 ) {
        if( strcasecmp(attrValue, "first")==0 ) {
            nodeData->clipType = NXT_ClipType_First;
        } else if( strcasecmp(attrValue, "mid")==0 ) {
            nodeData->clipType = NXT_ClipType_Middle;
        } else if( strcasecmp(attrValue, "middle")==0 ) {
            nodeData->clipType = NXT_ClipType_Middle;
        } else if( strcasecmp(attrValue, "last")==0 ) {
            nodeData->clipType = NXT_ClipType_Last;
        } else if( strcasecmp(attrValue, "even")==0 ) {
            nodeData->clipType = NXT_ClipType_Even;
        } else if( strcasecmp(attrValue, "odd")==0 ) {
            nodeData->clipType = NXT_ClipType_Odd;
        } else if( strcasecmp(attrValue, "all")==0 ) {
            nodeData->clipType = NXT_ClipType_All;
        } else if( strcasecmp(attrValue, "!first")==0 ) {
            nodeData->clipType = NXT_ClipType_NotFirst;
        } else if( strcasecmp(attrValue, "!mid")==0 ) {
            nodeData->clipType = NXT_ClipType_NotMiddle;
        } else if( strcasecmp(attrValue, "!middle")==0 ) {
            nodeData->clipType = NXT_ClipType_NotMiddle;
        } else if( strcasecmp(attrValue, "!last")==0 ) {
            nodeData->clipType = NXT_ClipType_NotLast;
        } else if( strcasecmp(attrValue, "!even")==0 ) {
            nodeData->clipType = NXT_ClipType_NotEven;
        } else if( strcasecmp(attrValue, "!odd")==0 ) {
            nodeData->clipType = NXT_ClipType_NotOdd;
        }
    } else {
        // TODO: Error
    }
    
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Part *nodeData = (NXT_Node_Part*)node;
    LOGD( "(Part)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Part *nodeData = (NXT_Node_Part*)node;
    if( nodeData->type!=NXT_PartType_All && renderer->part != nodeData->type ) {
        renderer->bSkipChildren = 1;
    }
    switch( nodeData->clipType ) {
        case NXT_ClipType_All:
            break;
        case NXT_ClipType_First:
            if( renderer->clip_index != 0 )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_Middle:
            if( renderer->clip_count>0 && renderer->clip_index <= 0 && renderer->clip_index>=(renderer->clip_count-1) )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_Last:
            if( renderer->clip_index<(renderer->clip_count-1) )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_Even:
            if( (renderer->clip_index % 2)!=0 )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_Odd:
            if( (renderer->clip_index % 2)==0 )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_NotFirst:
            if( renderer->clip_index == 0 )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_NotMiddle:
            if( !(renderer->clip_count>0 && renderer->clip_index <= 0 && renderer->clip_index>=(renderer->clip_count-1)) )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_NotLast:
            if( renderer->clip_index>=(renderer->clip_count-1) )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_NotEven:
            if( (renderer->clip_index % 2)==0 )
                renderer->bSkipChildren = 1;
            break;
        case NXT_ClipType_NotOdd:
            if( (renderer->clip_index % 2)!=0 )
                renderer->bSkipChildren = 1;
            break;
        default:
            break;
    }
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
