#include "NexTheme_Config.h"
#include "NexTheme.h"
#include "NexTheme_Util.h"
#include <math.h>
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme"

#define IS_FLOAT_CHAR(xx) (((xx)>='0'&&(xx)<='9')||(xx)=='.'||(xx)=='-')
#define IS_WHITESPACE(xx) ((xx)==' '||(xx)=='\t')

TFU NXT_ParseBool(const char* string) {
    if( strcasecmp(string,"")==0  || strcasecmp(string,"on")==0 || strcasecmp(string,"yes")==0 || strcasecmp(string,"true")==0 || strcasecmp(string,"1")==0 ) {
        return TFUTrue;
    } else if( strcasecmp(string,"no")==0 || strcasecmp(string,"off")==0 || strcasecmp(string,"0")==0 || strcasecmp(string,"false")==0 ) {
        return TFUFalse;
    } else {
        return TFUUnknown;
    }
}

int NXT_FloatVectorFromString( const char* string, float* vector, int numElements ) {

    int elementsFound = 0;
    int i;

    if( !string ) {
        return 0;
    }
    
    const char *s = string;
    while( *s && elementsFound<numElements ) {
        float sign=1.0;
        float result=0.0;
        while( *s && !IS_FLOAT_CHAR(*s) )
            s++;
        if( *s=='-' ) {
            sign = -sign;
            s++;
        }
        while( *s>='0' && *s<='9' ) {
            result = (result*10.0) + (float)(*s - '0');
            s++;
        }
        if( *s=='.' ) {
            s++;
            float fct = 0.1;
            while( *s>='0' && *s<='9' ) {
                result = result + (fct * (float)(*s - '0'));
                fct /= 10.0;
                s++;
            }
        }
        *vector = sign*result;
        vector++;
        elementsFound++;
    }
    
    for( i=elementsFound; i<numElements; i++ ) {
        if( elementsFound==3 )
            *vector = 1.0;
        else
            *vector = 0.0;
        vector++;
    }
    
    return elementsFound;
}

int NXT_PartialFloatVectorFromString( const char** string, float* vector, int numElements ) {
    // Vector may be NULL, in which case we just compute and return
    // the number of elements up to the maximum of numElements
    if( !string || !*string) {
        return 0;
    }
    
    int elementsFound = 0;
    
    char *s = (char*)*string;
    while( *s && elementsFound<numElements ) {
        float sign=1.0;
        float result=0.0;
        while( *s && IS_WHITESPACE(*s) )
            s++;
        if( *s=='#' ) {
            unsigned int value=0;
            int digits=0;
            s++;
            for(;;) {
                if( *s >= '0' && *s <='9' ) {
                    value *= 0x10;
                    value += (*s-'0');
                } else if( *s >= 'a' && *s <='z' ) {
                    value *= 0x10;
                    value += (*s-'a'+0xA);
                } else if( *s >= 'A' && *s <='Z' ) {
                    value *= 0x10;
                    value += (*s-'A'+0xA);
                } else {
                    break;
                }
                s++;
                digits++;
            }
            if( digits==3 ) {
                while( digits > 0 ) {
                    int el = (value >> ((digits-1)*4))&0xF;
                    if( elementsFound<numElements ) {
                        if( vector ) {
                            *vector = (float)el / (float)15.0;
                            vector++;
                        }
                        elementsFound++;
                    }
                    digits -= 1;
                }
            } else {
                digits += (digits % 2); // Round up to an even number
                while( digits > 0 ) {
                    int el = (value >> ((digits-2)*4))&0xFF;
                    if( elementsFound<numElements ) {
                        if( vector ) {
                            *vector = (float)el / (float)255.0;
                            vector++;
                        }
                        elementsFound++;
                    }
                    digits -= 2;
                }
            }
            continue;
        }
        if( *s && *s!=',' && !IS_FLOAT_CHAR(*s) )
            break;
        if( *s=='-' ) {
            sign = -sign;
            s++;
        }
        while( *s>='0' && *s<='9' ) {
            result = (result*10.0) + (float)(*s - '0');
            s++;
        }
        if( *s=='.' ) {
            s++;
            float fct = 0.1;
            while( *s>='0' && *s<='9' ) {
                result = result + (fct * (float)(*s - '0'));
                fct /= 10.0;
                s++;
            }
        }
        if( vector ) {
            *vector = sign*result;
            vector++;
        }
        elementsFound++;
        while( *s && IS_WHITESPACE(*s) )
            s++;
        if( *s && *s==',' )
            s++;
        while( *s && IS_WHITESPACE(*s) )
            s++;
    }
    
    *string = s;
    return elementsFound;
}

// #ifdef __llvm__

#define sampleCurveX(t) (((ax*(t)+bx)*(t)+cx)*(t))
#define sampleCurveY(t) (((ay*(t)+by)*(t)+cy)*(t))
#define sampleCurveDerivativeX(t) ((3.0*ax*(t)+2.0*bx)*(t)+cx)
#define solveEpsilon(duration) (1.0/(200.0*(duration)))

float NXT_CubicBezierAtTime(float t,float p1x,float p1y,float p2x,float p2y,float duration) 
{
    float x=t;
    float epsilon = solveEpsilon(duration);
    float ax=0,bx=0,cx=0,ay=0,by=0,cy=0;
    cx=3.0*p1x; bx=3.0*(p2x-p1x)-cx; ax=1.0-cx-bx; cy=3.0*p1y; by=3.0*(p2y-p1y)-cy; ay=1.0-cy-by;
    
    //float result = 0.0;
    float t0,t1,t2,x2,d2,i;
    for(t2=x, i=0; i<64; i++) 
    {
        x2=sampleCurveX(t2)-x; 
        if(fabs(x2)<epsilon) {
            return sampleCurveY(t2);
        } 
        d2=sampleCurveDerivativeX(t2); 
        if(fabs(d2)<1e-6) {
            break;
        } 
        t2=t2-x2/d2;
    }
    t0=0.0; 
    t1=1.0; 
    t2=x; 
    if(t2<t0) {
        return sampleCurveY(t0);
    } else if(t2>t1) {
        return sampleCurveY(t1);
    } else {
        while(t0<t1) {
            x2=sampleCurveX(t2); 
            if(fabs(x2-x)<epsilon) {
                return t2;
            } 
            if(x>x2) {
                t0=t2;
            } else {
                t1=t2;
            } 
            t2=(t1-t0)*.5+t0;
        }
        return sampleCurveY(t2);
    }
}

// #else

// float NXT_CubicBezierAtTime(float t,float p1x,float p1y,float p2x,float p2y,float duration) {

// //#error "Unsupported compiler"
// //#else
    
//     float ax=0,bx=0,cx=0,ay=0,by=0,cy=0;
//     // `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
//     float sampleCurveX(float t) {
//         return ((ax*t+bx)*t+cx)*t;
//     }
//     float sampleCurveY(float t) {
//         float result = ((ay*t+by)*t+cy)*t;
//         LOGXV("sampleCurveY(%f) -> ay=%f by=%f cy=%f -----> %f", t, ay, by, cy, result);
//         return result;
//     }
//     float sampleCurveDerivativeX(float t) {return (3.0*ax*t+2.0*bx)*t+cx;};
//     // The epsilon value to pass given that the animation is going to run over |dur| seconds. The longer the
//     // animation, the more precision is needed in the timing function result to avoid ugly discontinuities.
//     float solveEpsilon(float duration) {return 1.0/(200.0*duration);};
//     // Given an x value, find a parametric value it came from.
//     float solveCurveX(float x,float epsilon) {
//         float t0,t1,t2,x2,d2;
//         int i;
//         //function fabs(n) {if(n>=0) {return n;}else {return 0-n;}}; 
//         // First try a few iterations of Newton's method -- normally very fast.
//         for(t2=x, i=0; i<8; i++) {
//             x2=sampleCurveX(t2)-x; 
//             if(fabs(x2)<epsilon) {
//                 LOGXV("Returning @%d (%f)", __LINE__, t2);
//                 return t2;
//             } 
//             d2=sampleCurveDerivativeX(t2); 
//             if(fabs(d2)<0.000001) {
//                 LOGXV("Breaking @%d (%f)", __LINE__, d2);
//                 break;
//             } 
//             t2=t2-x2/d2;
//         }
//         // Fall back to the bisection method for reliability.
//         t0=0.0; 
//         t1=1.0; 
//         t2=x; 
//         if(t2<t0) {
//             LOGXV("Returning @%d (%f)", __LINE__, t0);
//             return t0;
//         } 
//         if(t2>t1) {
//             LOGXV("Returning @%d (%f)", __LINE__, t1);
//             return t1;
//         }
//         while(t0<t1) {
//             x2=sampleCurveX(t2); 
//             if(fabs(x2-x)<epsilon) {
//                 LOGXV("Returning @%d (%f)", __LINE__, t2);
//                 return t2;
//             } 
//             if(x>x2) {
//                 t0=t2;
//             } else {
//                 t1=t2;
//             } 
//             t2=(t1-t0)*.5+t0;
//         }
//         LOGXV("Returning @%d (%f)", __LINE__, t2);
//         return t2; // Failure.
//     };
//     float solve(float x,float epsilon) {
//         LOGXV("Solving for x=%f and epsilon=%f", __LINE__, x, epsilon);
//         float solvedx = solveCurveX(x,epsilon);
//         LOGXV("solveCurveX() returned %f", __LINE__, solvedx);
//         return sampleCurveY(solvedx);
//     };
//     // Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
//     cx=3.0*p1x; 
//     bx=3.0*(p2x-p1x)-cx; 
//     ax=1.0-cx-bx; 
//     cy=3.0*p1y; 
//     by=3.0*(p2y-p1y)-cy; 
//     ay=1.0-cy-by;
//     // Convert from input time to parametric value in curve, then from that to output time.
//     return solve(t, solveEpsilon(duration));
// };
// #endif
