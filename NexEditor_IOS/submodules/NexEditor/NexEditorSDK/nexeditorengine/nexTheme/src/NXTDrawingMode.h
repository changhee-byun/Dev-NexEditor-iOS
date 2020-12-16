//
//  NXTDrawingMode.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/24/14.
//
//

#ifndef __NexVideoEditor__NXTDrawingMode__
#define __NexVideoEditor__NXTDrawingMode__

#include <iostream>
#include "NexTheme_Math.h"
#include "NXTMatrix.h"
#include "NXTVector.h"
#include "IRenderContext.h"



class NXTDrawingMode {
public:
    NXTCullFace cullFace = NXTCullFace::None;
    NXTMatrix transform;
    NXTMatrix texture_transform;
    NXTMatrix mask_transform;
    NXTMatrix proj;
    NXTVector lightDirection;
    NXTMatrix mask_sample_transform;
};

#endif /* defined(__NexVideoEditor__NXTDrawingMode__) */
