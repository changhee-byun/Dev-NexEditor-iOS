#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Sequence"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Sequence = {
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
    "Sequence",
    sizeof(NXT_Node_Sequence)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    
    nodeData->start = NXT_Vector4f(0.0, 0.0, 0.0, 0.0);
    nodeData->end   = NXT_Vector4f(1.0, 1.0, 1.0, 1.0);
    nodeData->seed1 = 9374513;
    nodeData->seed2 = 3489572;
    nodeData->count = 10;
    nodeData->type  = NXT_SeqType_Interp;
    nodeData->element_count = 0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    if( nodeData->updateTargets )
        free(nodeData->updateTargets);
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    /*if( strcasecmp(attrName, "factor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->factor.e, sizeof(nodeData->factor.e)/sizeof(*(nodeData->factor.e)) );*/
    if( strcasecmp(attrName, "start")==0 ) {
        int num_elements = NXT_AnimFloatVectorFromString( node, attrValue, nodeData->start.e, 4 );
        if( num_elements > nodeData->element_count )
            nodeData->element_count = num_elements;
    } else if( strcasecmp(attrName, "end")==0 ) {
        int num_elements = NXT_AnimFloatVectorFromString( node, attrValue, nodeData->end.e, 4 );
        if( num_elements > nodeData->element_count )
            nodeData->element_count = num_elements;
    } else if( strcasecmp(attrName, "count")==0 ) {
        float count = 0.0;
        NXT_FloatVectorFromString( attrValue, &count, 1 );
        nodeData->count = (int)roundf(count);
        
    } else if( strcasecmp(attrName, "type")==0 ) {
        
        if( strcasecmp(attrValue, "random")==0 ) {
            nodeData->type = NXT_SeqType_Rand;
        } else if( strcasecmp(attrValue, "randmix")==0 ) {
            nodeData->type = NXT_SeqType_RandMix;
        } else if( strcasecmp(attrValue, "vrandom")==0 ) {
            nodeData->type = NXT_SeqType_VRand;
        } else if( strcasecmp(attrValue, "vrandmix")==0 ) {
            nodeData->type = NXT_SeqType_VRandMix;
        } else if( strcasecmp(attrValue, "linear")==0 ) {
            nodeData->type = NXT_SeqType_Interp;
        } else if( strcasecmp(attrValue, "exp")==0 ) {
            nodeData->type = NXT_SeqType_Exp;
        }

    } else if( strcasecmp(attrName, "seed")==0 ) {
        float seed = 0.0;
        union s_ {
            long long int llSeed;
            int iSeed[2];
        } s;
        NXT_FloatVectorFromString( attrValue, &seed, 1 );
        s.llSeed = (long long int)roundf(seed);
        nodeData->seed1 = s.iSeed[0];
        nodeData->seed2 = s.iSeed[1];
        if( nodeData->seed1==0 )
            nodeData->seed1=1;
        if( nodeData->seed2==0 )
            nodeData->seed2=nodeData->seed1*3;
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    LOGD( "(Sequence)" );
}

static void updateTargets( NXT_Node_Sequence *nodeData ) {
    NXT_Vector4f v;
    int i, j;
    float t = 0;
    
    switch( nodeData->type ) {
        case NXT_SeqType_Interp:
            t = (float)nodeData->repeats_done / (float)(nodeData->count-1);
            break;
        case NXT_SeqType_Exp:
            t = (float)nodeData->repeats_done / (float)(nodeData->count-1);
            t = t*t;
            break;
        case NXT_SeqType_Rand:
        case NXT_SeqType_RandMix:
            t = NXT_FRandom(&nodeData->cur_seed);
            break;
        case NXT_SeqType_VRand:
        case NXT_SeqType_VRandMix:
            t = NXT_FRandom(&nodeData->cur_seed);
            break;
    }
    
    v.e[0] = nodeData->start.e[0] + (nodeData->end.e[0]-nodeData->start.e[0])*t;
    if( nodeData->type==NXT_SeqType_RandMix )
        t = NXT_FRandom(&nodeData->cur_seed);
    v.e[1] = nodeData->start.e[1] + (nodeData->end.e[1]-nodeData->start.e[1])*t;
    if( nodeData->type==NXT_SeqType_RandMix )
        t = NXT_FRandom(&nodeData->cur_seed);
    v.e[2] = nodeData->start.e[2] + (nodeData->end.e[2]-nodeData->start.e[2])*t;
    if( nodeData->type==NXT_SeqType_RandMix )
        t = NXT_FRandom(&nodeData->cur_seed);
    v.e[3] = nodeData->start.e[3] + (nodeData->end.e[3]-nodeData->start.e[3])*t;
    
    for( i=0; i<nodeData->numUpdateTargets; i++ ) {
        for( j=0; j<nodeData->updateTargets[i].numElements; j++ ) {
            nodeData->updateTargets[i].targetVector[j] = v.e[j + nodeData->updateTargets[i].elementOffset] * nodeData->updateTargets[i].factor + nodeData->updateTargets[i].bias;
        }
        if( nodeData->updateTargets[i].notifyNode && nodeData->updateTargets[i].notifyNode->isa->updateNotifyFunc ) {
            LOGV( "SEQUENCE -> seqnode(0x%08X) notifynode(0x%08X) private(0x%08X)", nodeData, nodeData->updateTargets[i].notifyNode, nodeData->updateTargets[i].notifyPrivateData );
            nodeData->updateTargets[i].notifyNode->isa->updateNotifyFunc( nodeData->updateTargets[i].notifyNode, nodeData->updateTargets[i].notifyPrivateData );
        }
    }
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    
    if( nodeData->type == NXT_SeqType_Rand || nodeData->type == NXT_SeqType_RandMix ) {
        NXT_SRandom(&nodeData->cur_seed, nodeData->seed1, nodeData->seed2);
    } else if( nodeData->type == NXT_SeqType_VRand || nodeData->type == NXT_SeqType_VRandMix ) {
        NXT_SRandom(&nodeData->cur_seed, nodeData->seed1 + (renderer->clip_index * 0x4A8F0E1), nodeData->seed2 + (renderer->clip_count * 0x1329) + (renderer->clip_index * 0x29BC8C4) );
    }
    nodeData->repeats_done = 0;
    updateTargets(nodeData);
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Sequence *nodeData = (NXT_Node_Sequence*)node;
    
    nodeData->repeats_done++;
    if( nodeData->repeats_done < nodeData->count ) {
        updateTargets(nodeData);
        return NXT_NodeAction_Repeat;
    } else {
        return NXT_NodeAction_Next;
    }
}

int NXT_Node_Sequence_GetElementCount( NXT_Node_Sequence *seqNode ) {
    if( !seqNode || seqNode->header.isa!=&NXT_NodeClass_Sequence ) {
        return 0;
    }   
    return seqNode->element_count;
}

void NXT_Node_Sequence_AddUpdateTarget( NXT_Node_Sequence *seqNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *notifyNode, void* notifyPrivateData  ) {
    if( !targetVector || numElements<1 || !seqNode || seqNode->header.isa!=&NXT_NodeClass_Sequence ) {
        // TODO: Error
        return;
    }
    
    if( seqNode->numUpdateTargets >= seqNode->allocUpdateTargets ) {
        NXT_UpdateTarget *pUpdateTargets = (NXT_UpdateTarget*)malloc(sizeof(NXT_UpdateTarget)*(seqNode->numUpdateTargets + 32));
        memcpy(pUpdateTargets, seqNode->updateTargets, sizeof(NXT_UpdateTarget)*seqNode->numUpdateTargets);
        free(seqNode->updateTargets);
        seqNode->updateTargets = pUpdateTargets;
        seqNode->allocUpdateTargets = seqNode->numUpdateTargets + 32;
    }
    seqNode->updateTargets[seqNode->numUpdateTargets].targetVector = targetVector;
    seqNode->updateTargets[seqNode->numUpdateTargets].numElements = numElements;
    seqNode->updateTargets[seqNode->numUpdateTargets].elementOffset = elementOffset;
    seqNode->updateTargets[seqNode->numUpdateTargets].factor = factor;
    seqNode->updateTargets[seqNode->numUpdateTargets].bias = bias;
    seqNode->updateTargets[seqNode->numUpdateTargets].notifyNode = notifyNode;
    seqNode->updateTargets[seqNode->numUpdateTargets].notifyPrivateData = notifyPrivateData;
    seqNode->numUpdateTargets++;
}
