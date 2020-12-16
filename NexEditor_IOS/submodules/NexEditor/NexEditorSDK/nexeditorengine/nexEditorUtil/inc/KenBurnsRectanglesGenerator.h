/******************************************************************************
 * File Name   :    KenBurnsRectanglesGenerator.hpp
 * Description :
 ******************************************************************************
 * Copyright (c) 2002-2019 KineMaster Corp. All rights reserved.
 * http://www.kinemaster.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 *****************************************************************************/


#ifndef KenBurnsRectanglesGenerator_hpp
#define KenBurnsRectanglesGenerator_hpp

#include <stdio.h>

#ifndef _NEXVIDEOEDITOR_BASERECT__H_
typedef struct _RECT {
    int left, top, right, bottom;
} RECT;
#endif

#ifndef min
#undef min
#define min(x,y) (((x)>(y))?(y):(x))
#define max(x,y) (((x)>(y))?(x):(y))
#endif

class SimpleRectangle;

class KenBurnsRectanglesGenerator {
public:
    
    KenBurnsRectanglesGenerator();
    ~KenBurnsRectanglesGenerator();
    
    int generate(RECT* pSrcRect, RECT* pFaceRects, int nFaceCount, int outputRatioW, int outputRatioH, int durationInMSec, RECT *pOutput);
    
private:
    static int comparator(const void*, const void*);
    int makeEndRect(SimpleRectangle* pSrcSR, RECT* pFaceRects, int nFaceCount, int outputRatioW, int outputRatioH);
    int makeStartRect(SimpleRectangle* pSrcSR, int durationInMSec);
    //int aaaaaa();
    
private:
    int ratioForCuttingoffSamllFaces;
    float inverseRateForMagnification;
    float factorForTargetRectangle; // 0 ~ 1, 0 : don't care, 1 : full, 0.5 : half
    float minSrcDestSizeRatio;
    float maxSrcDestSizeRatio;
    int maxRandomDegree;
    
    SimpleRectangle* srStart, *srEnd;
    
    
};

#endif /* KenBurnsRectanglesGenerator_hpp */

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/03/28    Draft.
 -----------------------------------------------------------------------------*/
