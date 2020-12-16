//
//  NXTBezier.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 1/14/15.
//
//

#ifndef __NexVideoEditor__NXTBezier__
#define __NexVideoEditor__NXTBezier__

#include <stdio.h>

namespace NXTBezier {
    float cubicBezierAtTime(float t,float p1x,float p1y,float p2x,float p2y,float duration);
};

#endif /* defined(__NexVideoEditor__NXTBezier__) */
