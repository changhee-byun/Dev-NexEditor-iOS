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

NXT_NodeClass NXT_NodeClass_UserField = {
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
    "UserField",
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

int NXT_Node_UserField_GetElementCount( NXT_Node_UserField *userFieldNode ) {
    if( !userFieldNode || userFieldNode->header.isa!=&NXT_NodeClass_UserField ) {
        return 0;
    }
    return userFieldNode->element_count;
}

void NXT_Node_UserField_ProcessUpdateTargets( NXT_Node_UserField *userFieldNode, NXT_HThemeRenderer renderer ) {
    
    LOGD( "In NXT_Node_UserField_ProcessUpdateTargets (%s)", userFieldNode->header.node_id );
    
    int i;
    
    if( !userFieldNode || userFieldNode->header.isa!=&NXT_NodeClass_UserField ) {
        LOGW( "UserField -> WRONG NODE CLASS" );
        return;
    }

    if( userFieldNode->element_count < 1 ) {
        LOGD( "UserField -> Element count too low" );
        return;
    }
    
    char *pOption = NXT_ThemeRenderer_GetEffectOption( renderer, userFieldNode->header.node_id );
    if( pOption==NULL ) {
        LOGD( "UserField -> Defaulting" );
        pOption = userFieldNode->pDefault;
    }
    if( pOption==NULL ) {
        LOGW( "UserField -> pOption is NULL" );
        return;
    }
    
    NXT_Vector4f value;
    value.e[3] = 1.0f;
    //char *p = pOption;
    NXT_PartialFloatVectorFromString((const char**)&pOption, value.e, 4);
    
    float *e = value.e;
    
    LOGD( "UserField - Updating %d targets (%s)", userFieldNode->numUpdateTargets, pOption );
    for( i=0; i<userFieldNode->numUpdateTargets; i++ ) {
        int j;
        if( userFieldNode->updateTargets[i].targetVector ) {
            for( j=0; j<userFieldNode->updateTargets[i].numElements; j++ ) {
                userFieldNode->updateTargets[i].targetVector[j] = e[j+userFieldNode->updateTargets[i].elementOffset] * userFieldNode->updateTargets[i].factor + userFieldNode->updateTargets[i].bias;
            }
        }
    }

}

void NXT_Node_UserField_AddUpdateTarget( NXT_Node_UserField *userFieldNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *paramsBaseNode, char* params ) {
    //LOGD( "In NXT_Node_UserField_AddUpdateTarget" );
    if( !targetVector || numElements<1 || !userFieldNode || elementOffset<0 || userFieldNode->header.isa!=&NXT_NodeClass_UserField ) {
        // TODO: Error
        return;
    }

    if( userFieldNode->numUpdateTargets >= userFieldNode->allocUpdateTargets ) {
        NXT_UpdateTarget *pUpdateTargets = (NXT_UpdateTarget*)malloc(sizeof(NXT_UpdateTarget)*(userFieldNode->numUpdateTargets + 64));
        memcpy(pUpdateTargets, userFieldNode->updateTargets, sizeof(NXT_UpdateTarget)*userFieldNode->numUpdateTargets);
        free(userFieldNode->updateTargets);
        userFieldNode->updateTargets = pUpdateTargets;
        userFieldNode->allocUpdateTargets = userFieldNode->numUpdateTargets + 64;
    }
    
    userFieldNode->updateTargets[userFieldNode->numUpdateTargets].targetVector = targetVector;
    userFieldNode->updateTargets[userFieldNode->numUpdateTargets].numElements = numElements;
    userFieldNode->updateTargets[userFieldNode->numUpdateTargets].elementOffset = elementOffset;
    userFieldNode->updateTargets[userFieldNode->numUpdateTargets].factor = factor;
    userFieldNode->updateTargets[userFieldNode->numUpdateTargets].bias = bias;
    
    userFieldNode->numUpdateTargets++;
    //LOGD( "OUT NXT_Node_UserField_AddUpdateTarget" );
}

