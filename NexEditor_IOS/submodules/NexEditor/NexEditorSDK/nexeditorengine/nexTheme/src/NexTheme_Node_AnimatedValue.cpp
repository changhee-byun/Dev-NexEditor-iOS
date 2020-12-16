#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define  LOG_TAG    "NexTheme_Node_AnimatedValue"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_AnimatedValue = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    NULL,
    NULL,
    NULL,
    NULL,
    "AnimatedValue",
    sizeof(NXT_Node_AnimatedValue)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_AnimatedValue *animvalNode = (NXT_Node_AnimatedValue*)node;
    
    animvalNode->basis = NXT_InterpBasis_Time;
    animvalNode->noiseOctaves = 4;
    animvalNode->noiseType = NXT_NoiseType_None;
    animvalNode->noiseSampleBias = 0.0;
    animvalNode->noiseSampleScale = 1.0;
    animvalNode->noiseScale = 0.5;
    animvalNode->noiseBias = 0.5;
    animvalNode->noiseFunc = NXT_NoiseFunction_Pure;
    animvalNode->startTime = 0.0;
    animvalNode->endTime = 1.0;
    animvalNode->framefit = 0;
    
    // Default
    animvalNode->timing[0] = 0.25;
    animvalNode->timing[1] = 0.10;
    animvalNode->timing[2] = 0.25;
    animvalNode->timing[3] = 1.0;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    int i;
    NXT_Node_AnimatedValue *animvalNode = (NXT_Node_AnimatedValue*)node;
    if( animvalNode->keyframeSet ) {
        for( i=0; i<animvalNode->numKeyframeSets; i++ ) {
            if( animvalNode->keyframeSet[i].keyframes ) {
                free(animvalNode->keyframeSet[i].keyframes);
                animvalNode->keyframeSet[i].keyframes = NULL;
            }
        }
        free( animvalNode->keyframeSet );
        animvalNode->keyframeSet = NULL;
    }
    if( animvalNode->updateTargets ) {
        free(animvalNode->updateTargets);
    }
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_AnimatedValue *animvalNode = (NXT_Node_AnimatedValue*)node;
    if( strcasecmp(attrName, "timingfunction")==0 ) {
        
        if( strcasecmp(attrValue, "ease")==0 ) {
            animvalNode->timing[0] = 0.25;
            animvalNode->timing[1] = 0.10;
            animvalNode->timing[2] = 0.25;
            animvalNode->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-in")==0 ) {
            animvalNode->timing[0] = 0.42;
            animvalNode->timing[1] = 0.0;
            animvalNode->timing[2] = 1.0;
            animvalNode->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-out")==0 ) {
            animvalNode->timing[0] = 0.0;
            animvalNode->timing[1] = 0.0;
            animvalNode->timing[2] = 0.58;
            animvalNode->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "ease-in-out")==0 ) {
            animvalNode->timing[0] = 0.42;
            animvalNode->timing[1] = 0.0;
            animvalNode->timing[2] = 0.58;
            animvalNode->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "linear")==0 ) {
            animvalNode->timing[0] = 0.0;
            animvalNode->timing[1] = 0.0;
            animvalNode->timing[2] = 1.0;
            animvalNode->timing[3] = 1.0;
        } else if( strcasecmp(attrValue, "step")==0 ) {

            //for the consistency of code, add additional timingfunction "step" which replace attribute "frame-fit"
            animvalNode->framefit = 1;
        } else if( strcasecmp(attrValue, "default")==0 ) {
            animvalNode->timing[0] = 0.25;
            animvalNode->timing[1] = 0.10;
            animvalNode->timing[2] = 0.25;
            animvalNode->timing[3] = 1.0;
        } else if( *attrValue == NXT_ELEMENT_REF_CHAR ) {
            NXT_NodeHeader *ref = NXT_FindFirstNodeWithId(node, attrValue+1, NULL);
            if( ref ) {
                if( ref->isa==&NXT_NodeClass_TimingFunc ) {
                    NXT_Node_TimingFunc *timingFunc = (NXT_Node_TimingFunc*)node;
                    animvalNode->timing[0] = timingFunc->e[0];
                    animvalNode->timing[1] = timingFunc->e[1];
                    animvalNode->timing[2] = timingFunc->e[2];
                    animvalNode->timing[3] = timingFunc->e[3];
                } else if( ref->isa==&NXT_NodeClass_Const ) {
                    NXT_Node_Const *constNode = (NXT_Node_Const*)ref;
                    NXT_FloatVectorFromString(constNode->value, animvalNode->timing, 4);
                }
            }
        } else {
            NXT_FloatVectorFromString(attrValue, animvalNode->timing, 4);
        }
        
    } else if( strcasecmp(attrName, "src")==0 ) {
        if( *attrValue == NXT_ELEMENT_REF_CHAR ) {
            animvalNode->altChildNode = NXT_FindFirstNodeWithId(node, attrValue+1, NULL);
        }
    } else if( strcasecmp(attrName, "scope")==0 ) {
        if( strcasecmp(attrValue, "part")==0 ) {
            animvalNode->bUseOverallTime = 0;
        } else if( strcasecmp(attrValue, "whole")==0 ) {
            animvalNode->bUseOverallTime = 1;
        }
    } else if( strcasecmp(attrName, "framefit")==0 ) {
        if( strcasecmp(attrValue, "true")==0 ) {
            animvalNode->framefit = 1;
        } else if( strcasecmp(attrValue, "false")==0 ) {
            animvalNode->framefit = 0;
        }
    } else if( strcasecmp(attrName, "basis")==0 ) {
        if( strcasecmp(attrValue, "time")==0 ) {
            animvalNode->basis = NXT_InterpBasis_Time;
        } else if( strcasecmp(attrValue, "clipindex")==0 ) {
            animvalNode->basis = NXT_InterpBasis_ClipIndex;
        } else if( strcasecmp(attrValue, "clipindexshuffle")==0 ) {
            animvalNode->basis = NXT_InterpBasis_ClipIndexPermuted;
        }
    } else if( strcasecmp(attrName, "noise")==0 ) {
        if( strcasecmp(attrValue, "none")==0 ) {
            animvalNode->noiseType = NXT_NoiseType_None;
        } else if( strcasecmp(attrValue, "perlin-simplex")==0 ) {
            animvalNode->noiseType = NXT_NoiseType_PerlinSimplex;
        }
    } else if( strcasecmp(attrName, "noisefunction")==0 ) {
        if( strcasecmp(attrValue, "pure")==0 ) {
            animvalNode->noiseFunc = NXT_NoiseFunction_Pure;
        } else if( strcasecmp(attrValue, "fractal")==0 ) {
            animvalNode->noiseFunc = NXT_NoiseFunction_Fractal;
        } else if( strcasecmp(attrValue, "turbulence")==0 ) {
            animvalNode->noiseFunc = NXT_NoiseFunction_Turbulence;
        } else if( strcasecmp(attrValue, "wavefractal")==0 ) {
            animvalNode->noiseFunc = NXT_NoiseFunction_WaveFractal;
        }
    } else if( strcasecmp(attrName, "start")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->startTime, 1);
    } else if( strcasecmp(attrName, "end")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->endTime, 1);
    } else if( strcasecmp(attrName, "noisebias")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->noiseBias, 1);
    } else if( strcasecmp(attrName, "noisescale")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->noiseScale, 1);
    } else if( strcasecmp(attrName, "noisesamplebias")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->noiseSampleBias, 1);
    } else if( strcasecmp(attrName, "noisesamplescale")==0 ) {
        NXT_FloatVectorFromString(attrValue, &animvalNode->noiseSampleScale, 1);
    } else if( strcasecmp(attrName, "noiseoctaves")==0 ) {
        float octaves = 0.0;
        NXT_FloatVectorFromString(attrValue, &octaves, 1);
        animvalNode->noiseOctaves = (int)octaves;
        if( animvalNode->noiseOctaves < 1 )
            animvalNode->noiseOctaves = 1;
        if( animvalNode->noiseOctaves > 32 )
            animvalNode->noiseOctaves = 32;
    } else {
        // TODO: Error        
    }
}

static void processKeyframeSet(NXT_Node_AnimatedValue *animvalNode, NXT_NodeHeader *pStartNode, NXT_KeyFrameSet *kfset) {
    
    //LOGD( "AnimatedValue : processKeyframeSet IN" );
    
    NXT_NodeHeader *pn;
    
    int nKeyFrame = 0;
    
    pn = pStartNode;
    while( pn ) {
        if( pn->isa == &NXT_NodeClass_KeyFrame )
            nKeyFrame++;
        pn = pn->next;
    }
    kfset->numKeyFrames = nKeyFrame;
    
    if( kfset->keyframes )
        free(kfset->keyframes);
    kfset->keyframes = (NXT_KeyFrame*)malloc(kfset->numKeyFrames*sizeof(NXT_KeyFrame));
    
    nKeyFrame = 0;
    pn = pStartNode;
    while( pn ) {
        if( pn->isa == &NXT_NodeClass_KeyFrame ) {
            NXT_Node_KeyFrame *pkf = (NXT_Node_KeyFrame*)pn;
            kfset->keyframes[nKeyFrame].t = animvalNode->startTime + (pkf->t*(animvalNode->endTime - animvalNode->startTime));
            kfset->keyframes[nKeyFrame].e[0] = pkf->e[0];
            kfset->keyframes[nKeyFrame].e[1] = pkf->e[1];
            kfset->keyframes[nKeyFrame].e[2] = pkf->e[2];
            kfset->keyframes[nKeyFrame].e[3] = pkf->e[3];
            kfset->keyframes[nKeyFrame].timing[0] = pkf->timing[0];
            kfset->keyframes[nKeyFrame].timing[1] = pkf->timing[1];
            kfset->keyframes[nKeyFrame].timing[2] = pkf->timing[2];
            kfset->keyframes[nKeyFrame].timing[3] = pkf->timing[3];
            kfset->keyframes[nKeyFrame].framefit = pkf->framefit;
            if( pkf->element_count > animvalNode->element_count ) {
                animvalNode->element_count = pkf->element_count;
            }
            nKeyFrame++;
    		LOGD( "AnimatedValue : processKeyframeSet nKeyFrame=%d", nKeyFrame );
        }
        pn = pn->next;
    }
    
    //LOGD( "AnimatedValue : processKeyframeSet -> interpolate unspecified values" );
    
    int numzeroes = 0;
    float nzval = 0.0;
    int i;
    for( i=0; i<kfset->numKeyFrames; i++ ) {
        float wkval = kfset->keyframes[i].t;
        
        // Limit to 0...1
        if( wkval > 1.0 )
            wkval = 1.0;
        else if( wkval < 0.0 )
            wkval = 0.0;
        
        if( wkval == 0.0 && i==kfset->numKeyFrames-1 )
            wkval = 1.0;
        if( wkval == 0.0 && i>0 ) {
            numzeroes++;
        } else {
            if( numzeroes ) {
                float step = (wkval - nzval)/((float)(numzeroes+1));
                int j;
                for( j=i-numzeroes; j<i; j++ ) {
                    kfset->keyframes[j].t = step * (float)(j-(i-numzeroes)+1);
                }
            }
            if( wkval > nzval ) {
                nzval = wkval;
            }
            kfset->keyframes[i].t = nzval;
        }
    }
    //LOGD( "AnimatedValue : processKeyframeSet OUT" );
    
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {

    //LOGD( "AnimatedValue : nodeChildrenDoneFunc IN" );

    NXT_Node_AnimatedValue *animvalNode = (NXT_Node_AnimatedValue*)node;
    
    NXT_KeyFrameSet *kfset;
    
    NXT_NodeHeader *pn = animvalNode->altChildNode?animvalNode->altChildNode:node->child;
    int partCount = 0;
    int keyFrameFound = 0;
    while( pn ) {
        if( pn->isa == &NXT_NodeClass_Part )
            partCount++;
        else if( pn->isa == &NXT_NodeClass_KeyFrame )
            keyFrameFound=1;
        pn = pn->next;
    }
    
    animvalNode->element_count = 0;
    animvalNode->numKeyframeSets = partCount + keyFrameFound;
    
    if( animvalNode->numKeyframeSets==0 ) {
        animvalNode->numKeyframeSets=1;
    }
    
    animvalNode->keyframeSet = (NXT_KeyFrameSet*)malloc(sizeof(NXT_KeyFrameSet)*animvalNode->numKeyframeSets);
    memset(animvalNode->keyframeSet, 0, sizeof(NXT_KeyFrameSet)*animvalNode->numKeyframeSets);

    kfset = animvalNode->keyframeSet;
    kfset->framefit = animvalNode->framefit;

    LOGD( "AnimatedValue : animvalNode->numKeyframeSets=%d", animvalNode->numKeyframeSets );

    pn = node->child;    
    if( keyFrameFound || partCount<1 ) {
        kfset->partType=NXT_PartType_All;
        kfset->clipType=NXT_ClipType_All;
        kfset->bDefault=1;
        processKeyframeSet(animvalNode, pn, kfset);
        kfset++;
    }
    
    while( pn ) {
        if( pn->isa == &NXT_NodeClass_Part ) {
            NXT_Node_Part *part = (NXT_Node_Part*)pn;
            
            kfset->partType = part->type;
            kfset->clipType = part->clipType;
            kfset->bDefault=0;
            
            processKeyframeSet(animvalNode, pn->child, kfset);
            kfset++;
        }
        pn = pn->next;
    }
    
    
    NXT_DeleteChildNodes(NULL, node, 1);
    
    
#ifdef NEX_THEME_RENDERER_DEBUG_LOGGING
    LOGXV( "(AnimatedValue / numSets=%d)", animvalNode->numKeyframeSets );
    int nset;
    for( nset=0; nset<animvalNode->numKeyframeSets; nset++ ) {
        LOGXV( "    set %i: (default=%d, partType=%d, clipType=%d)", nset, animvalNode->keyframeSet[nset].bDefault, animvalNode->keyframeSet[nset].partType, animvalNode->keyframeSet[nset].clipType );
        int i;
        for( i=0; i<animvalNode->keyframeSet[nset].numKeyFrames; i++ ) {
            LOGXV( "                              >>Keyframe %d: t=%f (%f %f %f %f)",
                   i,
                   animvalNode->keyframeSet[nset].keyframes[i].t,
                   animvalNode->keyframeSet[nset].keyframes[i].e[0],
                   animvalNode->keyframeSet[nset].keyframes[i].e[1],
                   animvalNode->keyframeSet[nset].keyframes[i].e[2],
                   animvalNode->keyframeSet[nset].keyframes[i].e[3]);
        }
    }
#endif //NEX_THEME_RENDERER_DEBUG_LOGGING

    //LOGD( "AnimatedValue : nodeChildrenDoneFunc OUT" );

}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_AnimatedValue *animvalNode = (NXT_Node_AnimatedValue*)node;
    
    LOGD( "(AnimatedValue / numSets=%d)", animvalNode->numKeyframeSets );
    int nset;
    for( nset=0; nset<animvalNode->numKeyframeSets; nset++ ) {
        LOGD( "\n    set %i: (default=%d, partType=%d, clipType=%d)", nset, animvalNode->keyframeSet[nset].bDefault, animvalNode->keyframeSet[nset].partType, animvalNode->keyframeSet[nset].clipType );
        int i;
        for( i=0; i<animvalNode->keyframeSet[nset].numKeyFrames; i++ ) {
            LOGD( "\n                              >>Keyframe %d: t=%f (%f %f %f %f)",
                   i,
                   animvalNode->keyframeSet[nset].keyframes[i].t,
                   animvalNode->keyframeSet[nset].keyframes[i].e[0],
                   animvalNode->keyframeSet[nset].keyframes[i].e[1],
                   animvalNode->keyframeSet[nset].keyframes[i].e[2],
                   animvalNode->keyframeSet[nset].keyframes[i].e[3]);
        }
    }
    
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    renderer->bSkipChildren = 1;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}

int NXT_Node_AnimatedValue_GetElementCount( NXT_Node_AnimatedValue *animvalNode ) {
    if( !animvalNode || animvalNode->header.isa!=&NXT_NodeClass_AnimatedValue ) {
        return 0;
    }   
    return animvalNode->element_count;
}

void NXT_Node_AnimatedValue_AddUpdateTarget( NXT_Node_AnimatedValue *animvalNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *notifyNode, void* notifyPrivateData ) {
    if( !targetVector || numElements<1 || !animvalNode || elementOffset<0 || animvalNode->header.isa!=&NXT_NodeClass_AnimatedValue ) {
        // TODO: Error
        return;
    }
    
    if( animvalNode->numUpdateTargets >= animvalNode->allocUpdateTargets ) {
        NXT_UpdateTarget *pUpdateTargets = (NXT_UpdateTarget*)malloc(sizeof(NXT_UpdateTarget)*(animvalNode->numUpdateTargets + 64));
        memcpy(pUpdateTargets, animvalNode->updateTargets, sizeof(NXT_UpdateTarget)*animvalNode->numUpdateTargets);
        free(animvalNode->updateTargets);
        animvalNode->updateTargets = pUpdateTargets;
        animvalNode->allocUpdateTargets = animvalNode->numUpdateTargets + 64;
    }
    animvalNode->updateTargets[animvalNode->numUpdateTargets].targetVector = targetVector;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].numElements = numElements;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].elementOffset = elementOffset;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].factor = factor;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].bias = bias;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].notifyNode = notifyNode;
    animvalNode->updateTargets[animvalNode->numUpdateTargets].notifyPrivateData = notifyPrivateData;
    animvalNode->numUpdateTargets++;
}

