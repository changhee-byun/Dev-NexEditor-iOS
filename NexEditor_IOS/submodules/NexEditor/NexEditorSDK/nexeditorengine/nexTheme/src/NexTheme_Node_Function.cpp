#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"
#include "simplexnoise1234.h"

#define  LOG_TAG    "NexTheme_Node_Function"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static void nodeUpdateNotify(NXT_NodeHeader *node, void* notifyPrivateData);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Function = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    NULL,//precacheFunc
    NULL,//getPrecacheResourceFunc
    nodeUpdateNotify,
    NULL,//freeTextureFunc
    "Function",
    sizeof(NXT_Node_Function)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Function *funcNode = (NXT_Node_Function*)node;
    funcNode->octaves = 8;
    funcNode->inputBias  = NXT_Vector4f(0.0, 0.0, 0.0, 0.0);
    funcNode->inputScale = NXT_Vector4f(1.0, 1.0, 1.0, 1.0);
    funcNode->outputBias  = NXT_Vector4f(0.0, 0.0, 0.0, 0.0);
    funcNode->outputScale = NXT_Vector4f(1.0, 1.0, 1.0, 1.0);
    funcNode->bNormalize = 1;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //int i;
    NXT_Node_Function *funcNode = (NXT_Node_Function*)node;

    if( funcNode->updateTargets ) {
        free(funcNode->updateTargets);
    }

}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Function *funcNode = (NXT_Node_Function*)node;
    if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "simplexpure")==0 ) {
            funcNode->function = NXT_Function_SimplexPure;
        } else if( strcasecmp(attrValue, "simplexfractal")==0 ) {
            funcNode->function = NXT_Function_SimplexFractal;
        } else if( strcasecmp(attrValue, "simplexturbulence")==0 ) {
            funcNode->function = NXT_Function_SimplexTurbulence;
        } else if( strcasecmp(attrValue, "simplexwavefractal")==0 ) {
            funcNode->function = NXT_Function_SimplexWaveFractal;
        }
    } else if( strcasecmp(attrName, "inputbias")==0 ) {
        NXT_FloatVectorFromString(attrValue, funcNode->inputBias.e, 4);
    } else if( strcasecmp(attrName, "inputscale")==0 ) {
        NXT_FloatVectorFromString(attrValue, funcNode->inputScale.e, 4);
    } else if( strcasecmp(attrName, "outputbias")==0 ) {
        NXT_FloatVectorFromString(attrValue, funcNode->outputBias.e, 4);
    } else if( strcasecmp(attrName, "outputscale")==0 ) {
        NXT_FloatVectorFromString(attrValue, funcNode->outputScale.e, 4);
    } else if( strcasecmp(attrName, "normalize")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                funcNode->bNormalize = 1;
                break;
            case TFUFalse:
                funcNode->bNormalize = 0;
                break;
            default:
                funcNode->bNormalize = 0;
                break;
        }
    } else if( strcasecmp(attrName, "octaves")==0 ) {
        float octaves = 0.0;
        NXT_FloatVectorFromString(attrValue, &octaves, 1);
        funcNode->octaves = (int)octaves;
        if( funcNode->octaves < 1 )
            funcNode->octaves = 1;
        if( funcNode->octaves > 32 )
            funcNode->octaves = 32;
    } else {
        // TODO: Error        
    }
}


static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    //NXT_Node_Function *funcNode = (NXT_Node_Function*)node;

}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //NXT_Node_Function *funcNode = (NXT_Node_Function*)node;
    
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    //renderer->bSkipChildren = 1;
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}

int NXT_Node_Function_GetElementCount( NXT_Node_Function *funcNode ) {
    if( !funcNode || funcNode->header.isa!=&NXT_NodeClass_Function ) {
        return 0;
    }   
    return 1;
}

static void nodeUpdateNotify(NXT_NodeHeader *node, void* notifyPrivateData) {
    
    int i;
    
    NXT_Node_Function *funcNode = (NXT_Node_Function*)node;
    int targetIndex = *((int*)(&notifyPrivateData));
    LOGV( "FunctionNode (0x%p) : nodeUpdateNotify (targetIndex=%d) ", node,  targetIndex );
    if( targetIndex<0 || targetIndex>=funcNode->numUpdateTargets ) {
        // TODO: Error
        return;
    }
    NXT_UpdateTarget *target = &(funcNode->updateTargets[targetIndex]);
    
    if( target->numElements < 1 || target->numElements > 4 )
    {
        // TODO: Error
        return;
    }
    else if( target->numParams < 1 || target->numParams >= NXT_UPDATETARGET_MAXPARAMS )
    {
        // TODO: Error
        return;
    }
    
    float params[NXT_UPDATETARGET_MAXPARAMS] = {0};
    
    for( i=0; i<target->numParams; i++ ) {
        LOGV( "FunctionNode (0x%p) :     param %d = %.4f  ", node, i, target->params[i] );
        params[i] = (target->params[i] * funcNode->inputScale.e[i]) + funcNode->inputBias.e[i];
        LOGV( "FunctionNode (0x%p) :       after preprocesssing, param %d = %.4f  ", node, i, target->params[i] );
    }
    
    NXT_Vector4f result;
    
    int octave;
    float factor = 1.0;
    float maxval = 0.0;
    
    switch( funcNode->function ) {
        case NXT_Function_SimplexPure:
        {
            switch( target->numParams ) {
                case 1:
                    LOGV( "FunctionNode (0x%08X) : SimplexPure 1 ", node );
                    result.e[0] = simplex_noise1(params[0]);
                    break;
                case 2:
                    LOGV( "FunctionNode (0x%08X) : SimplexPure 2 ", node );
                    result.e[0] = simplex_noise2(params[0], params[1]);
                    break;
                case 3:
                    LOGV( "FunctionNode (0x%08X) : SimplexPure 3 ", node );
                    result.e[0] = simplex_noise3(params[0], params[1], params[2]);
                    break;
                case 4:
                    LOGV( "FunctionNode (0x%08X) : SimplexPure 4 ", node );
                    result.e[0] = simplex_noise4(params[0], params[1], params[2], params[3]);
                    break;
                default:
                    // TODO: Error
                    break;
            }
            maxval = 1.0;            
            break;
        }
        case NXT_Function_SimplexFractal:
        {
            switch( target->numParams ) {
                case 1:
                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 1 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise1(params[0]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 2:
                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 2 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise2(params[0]*factor, params[1]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 3:
                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 3 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise3(params[0]*factor, params[1]*factor, params[2]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 4:
                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 4 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise4(params[0]*factor, params[1]*factor, params[2]*factor, params[3]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                default:
                    // TODO: Error
                    break;
            }
            break;
        }
        case NXT_Function_SimplexTurbulence:
        {
            switch( target->numParams ) {
                case 1:
                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 1 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise1(params[0]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 2:
                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 2 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise2(params[0]*factor, params[1]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 3:
                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 3 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise3(params[0]*factor, params[1]*factor, params[2]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 4:
                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 4 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise4(params[0]*factor, params[1]*factor, params[2]*factor, params[3]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                default:
                    // TODO: Error
                    break;
            }
            break;
        }
        case NXT_Function_SimplexWaveFractal:
        {
            switch( target->numParams ) {
                case 1:
                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 1 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise1(params[0]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        factor++;
                    }
                    maxval = 1;
                    result.e[0] = sinf( result.e[0] + params[0] );
                    break;
                case 2:
                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 2 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise2(params[0]*factor, params[1]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        factor++;
                    }
                    maxval = 1;
                    result.e[0] = sinf( result.e[0] + params[0] );
                    break;
                case 3:
                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 3 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise3(params[0]*factor, params[1]*factor, params[2]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        factor++;
                    }
                    maxval = 1;
                    result.e[0] = sinf( result.e[0] + params[0] );
                    break;
                case 4:
                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 4 ", node );
                    for( octave = 0; octave < funcNode->octaves; octave++ ) {
                        float n = simplex_noise4(params[0]*factor, params[1]*factor, params[2]*factor, params[3]*factor);
                        if( n<0 )
                            result.e[0] += (1.0/factor)*(-n);
                        else
                            result.e[0] += (1.0/factor)*n;
                        factor++;
                    }
                    maxval = 1;
                    result.e[0] = sinf( result.e[0] + params[0] );
                    break;
                default:
                    // TODO: Error
                    break;
            }
            break;
        }
        default:
        {
            // TODO: Error
            break;
        }
    }
    
    LOGV( "FunctionNode (0x%08X) : RESULT=%.4f (maxval=%.4f) elemnets=%d ", node, result.e[0], maxval, target->numElements );
    
    if( maxval > 0 && funcNode->bNormalize ) {
        result.e[0] /= maxval;
        result.e[1] /= maxval;
        result.e[2] /= maxval;
        result.e[3] /= maxval;
    }
    
    for( i=0; i<target->numElements; i++ ) {
        target->targetVector[i] = (result.e[i] * funcNode->outputScale.e[i]) + funcNode->outputBias.e[i];;
    }
    
}


void NXT_Node_Function_ProcessUpdateTargets( NXT_Node_Function *funcNode ) {
    if( !funcNode || funcNode->header.isa!=&NXT_NodeClass_Function ) {
        // TODO: Error
        return;
    }
    
    long i;
    LOGV( "FunctionNode : begin processUpdateTargets (%d targets) ", funcNode->numUpdateTargets );
    for( i=0; i<funcNode->numUpdateTargets; i++ ) {
        nodeUpdateNotify( (NXT_NodeHeader*)funcNode, (void*)i );
    }
    //LOGV( "FunctionNode : end processUpdateTargets" );
}

void NXT_Node_Function_AddUpdateTarget( NXT_Node_Function *funcNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *paramsBaseNode, char* params ) {
    if( !targetVector || numElements<1 || !funcNode || elementOffset<0 || funcNode->header.isa!=&NXT_NodeClass_Function ) {
        // TODO: Error
        return;
    }
    
    if( funcNode->numUpdateTargets >= funcNode->allocUpdateTargets ) {
        NXT_UpdateTarget *pUpdateTargets = (NXT_UpdateTarget*)malloc(sizeof(NXT_UpdateTarget)*(funcNode->numUpdateTargets + 64));
        memcpy(pUpdateTargets, funcNode->updateTargets, sizeof(NXT_UpdateTarget)*funcNode->numUpdateTargets);
        free(funcNode->updateTargets);
        funcNode->updateTargets = pUpdateTargets;
        funcNode->allocUpdateTargets = funcNode->numUpdateTargets + 64;
    }

    funcNode->updateTargets[funcNode->numUpdateTargets].targetVector = targetVector;
    funcNode->updateTargets[funcNode->numUpdateTargets].numElements = numElements;
    funcNode->updateTargets[funcNode->numUpdateTargets].elementOffset = elementOffset;
    funcNode->updateTargets[funcNode->numUpdateTargets].factor = factor;
    funcNode->updateTargets[funcNode->numUpdateTargets].bias = bias;
    NXT_AnimFloatVectorFromStringNotify(paramsBaseNode, params, funcNode->updateTargets[funcNode->numUpdateTargets].params, NXT_UPDATETARGET_MAXPARAMS, &(funcNode->updateTargets[funcNode->numUpdateTargets].numParams), (NXT_NodeHeader*)funcNode, (void*)(long)funcNode->numUpdateTargets );
    
    funcNode->numUpdateTargets++;
}

