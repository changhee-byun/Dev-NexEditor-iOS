#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"
#include "simplexnoise1234.h"

#define  LOG_TAG    "NexTheme_Node_UserField"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Parameter = {
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
    "Parameter",
    sizeof(NXT_Node_UserField)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    //NXT_Node_UserField *userFieldNode = (NXT_Node_UserField*)node;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_UserField *userFieldNode = (NXT_Node_UserField*)node;
    if( userFieldNode->pDefault ) {
        free( userFieldNode->pDefault );
    }
    if( userFieldNode->updateTargets ) {
        free(userFieldNode->updateTargets);
    }
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_UserField *userFieldNode = (NXT_Node_UserField*)node;
    if( strcasecmp(attrName, "default")==0 ) {
        if( userFieldNode->pDefault ) {
            free( userFieldNode->pDefault );
        }
        userFieldNode->pDefault = (char*)malloc(strlen(attrValue)+1);
        strcpy(userFieldNode->pDefault, attrValue);
        //NXT_FloatVectorFromString(attrValue, userFieldNode->inputBias.e, 4);
    } else if( strcasecmp(attrName, "type")==0 ) {
        //text, color, overlay, undefined
        if( strcasecmp(attrValue, "text")==0 ) {
            userFieldNode->type = NXT_UserFieldType_Text;
            userFieldNode->element_count = 0;
        } else if( strcasecmp(attrValue, "color")==0 ) {
            userFieldNode->type = NXT_UserFieldType_Color;
            userFieldNode->element_count = 4;
        } else if( strcasecmp(attrValue, "selection")==0 ) {
            userFieldNode->type = NXT_UserFieldType_Selection;
            userFieldNode->element_count = 4;
        } else if( strcasecmp(attrValue, "overlay")==0 ) {
            userFieldNode->type = NXT_UserFieldType_Overlay;
            userFieldNode->element_count = 4;
        } else if( strcasecmp(attrValue, "range")==0 ) {
            userFieldNode->type = NXT_UserFieldType_Range;
            userFieldNode->element_count = 1;
        }
    } else {
        // TODO: Error        
    }
}


static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    //NXT_Node_UserField *userFieldNode = (NXT_Node_UserField*)node;

}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //NXT_Node_UserField *userFieldNode = (NXT_Node_UserField*)node;
    
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    //renderer->bSkipChildren = 1;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}