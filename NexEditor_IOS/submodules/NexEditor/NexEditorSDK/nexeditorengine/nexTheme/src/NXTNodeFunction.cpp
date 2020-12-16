//
//  NXTNodeFunction.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/10/14.
//
//

#include "NXTNodeFunction.h"
#include "simplexnoise1234.h"

void NXTNodeFunction::processIndividualUpdateTarget( NXTUpdateTarget &target ) {
    
    if( target.numElements < 1 || target.numElements > 4 )
    {
        // TODO: Error
        return;
    }
    else if( target.numParams < 1 || target.numParams >= NXT_UPDATETARGET_MAXPARAMS )
    {
        // TODO: Error
        return;
    }
    
    float params[NXT_UPDATETARGET_MAXPARAMS] = {0};
    
    for( int i=0; i<target.numParams; i++ ) {
//        LOGV( "FunctionNode (0x%08X) :     param %d = %.4f  ", node, i, target->params[i] );
        params[i] = (target.params[i] * inputScale.e[i]) + inputBias.e[i];
//        LOGV( "FunctionNode (0x%08X) :       after preprocesssing, param %d = %.4f  ", node, i, target->params[i] );
    }
    
    NXT_Vector4f result = {0};
    
    int octave;
    float factor = 1.0;
    float noise = 0.0;
    float maxval = 0.0;
    
    switch( function ) {
        case NXT_Function_SimplexPure:
        {
            switch( target.numParams ) {
                case 1:
//                    LOGV( "FunctionNode (0x%08X) : SimplexPure 1 ", node );
                    result.e[0] = simplex_noise1(params[0]);
                    break;
                case 2:
//                    LOGV( "FunctionNode (0x%08X) : SimplexPure 2 ", node );
                    result.e[0] = simplex_noise2(params[0], params[1]);
                    break;
                case 3:
//                    LOGV( "FunctionNode (0x%08X) : SimplexPure 3 ", node );
                    result.e[0] = simplex_noise3(params[0], params[1], params[2]);
                    break;
                case 4:
//                    LOGV( "FunctionNode (0x%08X) : SimplexPure 4 ", node );
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
            switch( target.numParams ) {
                case 1:
//                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 1 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise1(params[0]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 2:
//                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 2 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise2(params[0]*factor, params[1]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 3:
//                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 3 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
                        result.e[0] += (1.0/factor)*simplex_noise3(params[0]*factor, params[1]*factor, params[2]*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    break;
                case 4:
//                    LOGV( "FunctionNode (0x%08X) : SimplexFractal 4 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
            switch( target.numParams ) {
                case 1:
//                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 1 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 2 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 3 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexTurbulence 4 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
            switch( target.numParams ) {
                case 1:
//                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 1 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 2 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 3 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
//                    LOGV( "FunctionNode (0x%08X) : SimplexWaveFractal 4 ", node );
                    for( octave = 0; octave < octaves; octave++ ) {
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
    
//    LOGV( "FunctionNode (0x%08X) : RESULT=%.4f (maxval=%.4f) elemnets=%d ", node, result.e[0], maxval, target->numElements );
    
    if( maxval > 0 && bNormalize ) {
        result.e[0] /= maxval;
        result.e[1] /= maxval;
        result.e[2] /= maxval;
        result.e[3] /= maxval;
    }
    
    for( int i=0; i<target.numElements; i++ ) {
        target.targetVector[i] = (result.e[i] * outputScale.e[i]) + outputBias.e[i];;
    }

    
}
