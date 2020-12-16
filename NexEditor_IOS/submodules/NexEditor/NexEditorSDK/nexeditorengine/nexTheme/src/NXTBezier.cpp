//
//  NXTBezier.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 1/14/15.
//
//

#include "NXTBezier.h"
#include <math.h>

float NXTBezier::cubicBezierAtTime(float t,float p1x,float p1y,float p2x,float p2y,float duration) {
    float ax=0,bx=0,cx=0,ay=0,by=0,cy=0;
    // `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
    auto sampleCurveX = [&ax,&bx,&cx](float t)->float {
        return ((ax*t+bx)*t+cx)*t;
    };
    auto sampleCurveY = [&ay,&by,&cy](float t)->float {
        float result = ((ay*t+by)*t+cy)*t;
        return result;
    };
    auto sampleCurveDerivativeX = [&ax,&bx,&cx](float t)->float {return (3.0*ax*t+2.0*bx)*t+cx;};
    // The epsilon value to pass given that the animation is going to run over |dur| seconds. The longer the
    // animation, the more precision is needed in the timing function result to avoid ugly discontinuities.
    auto solveEpsilon = [](float duration)->float {return 1.0/(200.0*duration);};
    // Given an x value, find a parametric value it came from.
    auto solveCurveX = [sampleCurveX, sampleCurveDerivativeX](float x,float epsilon)->float {
        float t0,t1,t2,x2,d2;
        int i;
        //function fabs(n) {if(n>=0) {return n;}else {return 0-n;}};
        // First try a few iterations of Newton's method -- normally very fast.
        for(t2=x, i=0; i<8; i++) {
            x2=sampleCurveX(t2)-x;
            if(fabs(x2)<epsilon) {
                return t2;
            }
            d2=sampleCurveDerivativeX(t2);
            if(fabs(d2)<0.000001) {
                break;
            }
            t2=t2-x2/d2;
        }
        // Fall back to the bisection method for reliability.
        t0=0.0;
        t1=1.0;
        t2=x;
        if(t2<t0) {
            return t0;
        }
        if(t2>t1) {
            return t1;
        }
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
        return t2; // Failure.
    };
    auto solve = [solveCurveX,sampleCurveY](float x,float epsilon)->float  {
        float solvedx = solveCurveX(x,epsilon);
        return sampleCurveY(solvedx);
    };
    // Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
    cx=3.0*p1x;
    bx=3.0*(p2x-p1x)-cx;
    ax=1.0-cx-bx;
    cy=3.0*p1y;
    by=3.0*(p2y-p1y)-cy;
    ay=1.0-cy-by;
    // Convert from input time to parametric value in curve, then from that to output time.
    return solve(t, solveEpsilon(duration));
};
