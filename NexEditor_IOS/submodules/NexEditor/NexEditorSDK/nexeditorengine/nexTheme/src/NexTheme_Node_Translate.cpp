#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexTheme_Math.h"
#include "NexThemeRenderer_Internal.h"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Translate = {
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
    "Translate",
    sizeof(NXT_Node_Translate)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Translate *transNode = (NXT_Node_Translate*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Translate *transNode = (NXT_Node_Translate*)node;
    if( strcasecmp(attrName, "offset")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, transNode->vector.e, sizeof(transNode->vector.e)/sizeof(*(transNode->vector.e)) );
    } else if( strcasecmp(attrName, "textureoffset")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, transNode->texoffs.e, sizeof(transNode->texoffs.e)/sizeof(*(transNode->texoffs.e)) );
    } else if( strcasecmp(attrName, "maskoffset")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, transNode->maskoffs.e, sizeof(transNode->maskoffs.e)/sizeof(*(transNode->maskoffs.e)) );
    } else if( strcasecmp(attrName, "repeat")==0 ) {
        float repeatCount = 0.0;
        NXT_FloatVectorFromString( attrValue, &repeatCount, 1 );
        transNode->repeat = (int)roundf(repeatCount);
    } else if( strcasecmp(attrName, "jitterseed")==0 ) {
        float jitterSeed = 0.0;
        NXT_FloatVectorFromString( attrValue, &jitterSeed, 1 );
        transNode->jitterSeed = (int)roundf(jitterSeed);
    } else if( strcasecmp(attrName, "jitteralpha")==0 ) {
        NXT_FloatVectorFromString( attrValue, &transNode->jitter_alpha, 1 );
        transNode->bJitterAlpha = 1;
    } else if( strcasecmp(attrName, "jitter")==0 ) {
        if( !attrValue || !*attrValue || strcasecmp(attrValue, "yes")==0 || strcasecmp(attrValue, "1")==0 ) {
            transNode->bJitter = 1;
        }
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Translate *transNode = (NXT_Node_Translate*)node;
    LOGD( "(Translate %f %f %f %f)", transNode->vector.e[0], transNode->vector.e[1], transNode->vector.e[2], transNode->vector.e[3] );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Translate *transNode = (NXT_Node_Translate*)node;
    
    float jitter = 1.0;
    if(transNode->bJitter) {
        if(transNode->jitterSeed) {
            NXT_SRandom(&renderer->seed, transNode->jitterSeed, 295863);
            //srandom(transNode->jitterSeed);
        }
        jitter = NXT_FRandom(&renderer->seed);
    }
    
    NXT_Vector4f vector = transNode->vector;
    vector.e[0] *= jitter;
    vector.e[1] *= jitter;
    vector.e[2] *= jitter;

    transNode->saved_alpha = renderer->alpha;
    
    if( transNode->bJitterAlpha ) 
        renderer->alpha = jitter*transNode->jitter_alpha + (1-jitter)*transNode->saved_alpha;
    
    transNode->saved_transform = renderer->transform;
    transNode->saved_tex_transform = renderer->texture_transform;
    transNode->saved_mask_transform = renderer->mask_transform;
    if( transNode->repeat < 1 ) {
        renderer->transform = NXT_Matrix4f_MultMatrix(renderer->transform,NXT_Matrix4f_Translate( vector ));
        renderer->texture_transform = NXT_Matrix4f_MultMatrix(renderer->texture_transform,NXT_Matrix4f_Translate( transNode->texoffs ));
        renderer->mask_transform = NXT_Matrix4f_MultMatrix(renderer->mask_transform,NXT_Matrix4f_Translate( transNode->maskoffs ));
    }
    transNode->repeats_done = 0;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Translate *transNode = (NXT_Node_Translate*)node;

    transNode->repeats_done++;
    if( transNode->repeats_done < transNode->repeat ) {
        
        float jitter = 1.0;
        if(transNode->bJitter) {
            if(transNode->jitterSeed) {
                NXT_SRandom(&renderer->seed, transNode->jitterSeed, 295863);
                //srandom(transNode->jitterSeed);
            }
            int i;
            for( i=0; i<transNode->repeats_done; i++ ) {
                NXT_FRandom(&renderer->seed);
            }
            jitter = NXT_FRandom(&renderer->seed);
        }

        if( transNode->bJitterAlpha ) 
            renderer->alpha = jitter*renderer->alpha + (1-jitter)*transNode->saved_alpha;
        
        NXT_Vector4f vector;
        vector.e[0] = transNode->vector.e[0] * (float)(transNode->repeats_done);
        vector.e[1] = transNode->vector.e[1] * (float)(transNode->repeats_done);
        vector.e[2] = transNode->vector.e[2] * (float)(transNode->repeats_done);
        vector.e[3] = transNode->vector.e[3] * (float)(transNode->repeats_done);
        NXT_Vector4f texoffs;
        texoffs.e[0] = transNode->texoffs.e[0] * (float)(transNode->repeats_done);
        texoffs.e[1] = transNode->texoffs.e[1] * (float)(transNode->repeats_done);
        texoffs.e[2] = transNode->texoffs.e[2] * (float)(transNode->repeats_done);
        texoffs.e[3] = transNode->texoffs.e[3] * (float)(transNode->repeats_done);
        NXT_Vector4f maskoffs;
        maskoffs.e[0] = transNode->maskoffs.e[0] * (float)(transNode->repeats_done);
        maskoffs.e[1] = transNode->maskoffs.e[1] * (float)(transNode->repeats_done);
        maskoffs.e[2] = transNode->maskoffs.e[2] * (float)(transNode->repeats_done);
        maskoffs.e[3] = transNode->maskoffs.e[3] * (float)(transNode->repeats_done);
        renderer->transform = NXT_Matrix4f_MultMatrix(transNode->saved_transform,NXT_Matrix4f_Translate(vector));
        renderer->texture_transform = NXT_Matrix4f_MultMatrix(transNode->saved_tex_transform,NXT_Matrix4f_Translate(texoffs));
        renderer->mask_transform = NXT_Matrix4f_MultMatrix(transNode->saved_mask_transform,NXT_Matrix4f_Translate(maskoffs));
        return NXT_NodeAction_Repeat;
    } else {
        renderer->alpha = transNode->saved_alpha;
        renderer->transform = transNode->saved_transform;
        renderer->texture_transform = transNode->saved_tex_transform;
        renderer->mask_transform = transNode->saved_mask_transform;
        return NXT_NodeAction_Next;
    }
}
