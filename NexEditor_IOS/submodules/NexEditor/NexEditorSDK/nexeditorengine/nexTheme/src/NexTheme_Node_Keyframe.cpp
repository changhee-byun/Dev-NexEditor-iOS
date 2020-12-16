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

NXT_NodeClass NXT_NodeClass_KeyFrame = {
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
    "KeyFrame",
    sizeof(NXT_Node_KeyFrame)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_KeyFrame *kf = (NXT_Node_KeyFrame*)node;
    kf->timing[0] = 0.0;
    kf->timing[1] = 0.0;
    kf->timing[2] = 1.0;
    kf->timing[3] = 1.0;
    kf->framefit = 0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_KeyFrame *kf = (NXT_Node_KeyFrame*)node;
//    kf->timing[0] = 0.0;
//    kf->timing[1] = 0.0;
//    kf->timing[2] = 1.0;
//    kf->timing[3] = 1.0;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_KeyFrame *kf = (NXT_Node_KeyFrame*)node;
    if( strcasecmp(attrName, "time")==0 ) {
        NXT_FloatVectorFromString( attrValue, &kf->t, 1 );
    } else if( strcasecmp(attrName, "value")==0 ) {
        kf->element_count = NXT_FloatVectorFromString( attrValue, kf->e, sizeof(kf->e)/sizeof(*(kf->e)) );
    } else if( strcasecmp(attrName, "timingfunction")==0 ) {
        
        if( strcasecmp(attrValue, "ease")==0 ) {
            kf->timing[0] = 0.25;
            kf->timing[1] = 0.10;
            kf->timing[2] = 0.25;
            kf->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-in")==0 ) {
            kf->timing[0] = 0.42;
            kf->timing[1] = 0.0;
            kf->timing[2] = 1.0;
            kf->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-out")==0 ) {
            kf->timing[0] = 0.0;
            kf->timing[1] = 0.0;
            kf->timing[2] = 0.58;
            kf->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-in-out")==0 ) {
            kf->timing[0] = 0.42;
            kf->timing[1] = 0.0;
            kf->timing[2] = 0.58;
            kf->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "linear")==0 ) {
            kf->timing[0] = 0.0;
            kf->timing[1] = 0.0;
            kf->timing[2] = 1.0;
            kf->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "step")==0 ){

            kf->timing[0] = 0.25;
            kf->timing[1] = 0.10;
            kf->timing[2] = 0.25;
            kf->timing[3] = 1.0;
            kf->framefit = 1;
        }
        else if( strcasecmp(attrValue, "default")==0 ) {
            kf->timing[0] = 0.25;
            kf->timing[1] = 0.10;
            kf->timing[2] = 0.25;
            kf->timing[3] = 1.0;
        } else if( *attrValue == NXT_ELEMENT_REF_CHAR ) {
            NXT_NodeHeader *ref = NXT_FindFirstNodeWithId(node, attrValue+1, NULL);
            if( ref ) {
                if( ref->isa==&NXT_NodeClass_TimingFunc ) {
                    NXT_Node_TimingFunc *timingFunc = (NXT_Node_TimingFunc*)node;
                    kf->timing[0] = timingFunc->e[0];
                    kf->timing[1] = timingFunc->e[1];
                    kf->timing[2] = timingFunc->e[2];
                    kf->timing[3] = timingFunc->e[3];
                } else if( ref->isa==&NXT_NodeClass_Const ) {
                    NXT_Node_Const *constNode = (NXT_Node_Const*)ref;
                    NXT_FloatVectorFromString(constNode->value, kf->timing, 4);
                }
            }
        } else {
            NXT_FloatVectorFromString(attrValue, kf->timing, 4);
        }
        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_KeyFrame *kf = (NXT_Node_KeyFrame*)node;
    LOGD( "(KeyFrame t=%f, v=%f %f %f %f)", kf->t, kf->e[0], kf->e[1], kf->e[2], kf->e[3] );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
