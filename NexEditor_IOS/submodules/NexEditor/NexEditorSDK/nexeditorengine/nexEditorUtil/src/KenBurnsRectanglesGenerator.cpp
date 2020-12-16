/******************************************************************************
 * File Name   :    KenBurnsRectanglesGenerator.cpp
 * Description :
 *******************************************************************************
 * Copyright (c) 2002-2019 KineMaster Corp. All rights reserved.
 * http://www.kinemaster.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#ifdef ANDROID
        #include  <android/log.h>
#endif

#include "KenBurnsRectanglesGenerator.h"

class SimpleRectangle {
public:
    int l, r, t, b;
    SimpleRectangle() { l = r = t = b = 0; };
    SimpleRectangle(RECT*p) { setFrom(p); };
    
    void setFrom(RECT*p) {
        if ( p ) {
            l = p->left;
            r = p->right;
            t = p->top;
            b = p->bottom;
        }
    }
    void setTo(RECT*p) {
        if ( p ) {
            p->left = l;
            p->right = r;
            p->top = t;
            p->bottom = b;
        }
    }
    int getWidth() { return (r-l); };
    int getHeight() { return (b-t); };
    int getDimension() { return getWidth()*getHeight(); };
    int getCenterX() { return (l+r)/2; }
    int getCenterY() { return (t+b)/2; }
    void expandToContainRectangle(SimpleRectangle*p) {
        l = min(l, p->l);
        r = max(r, p->r);
        t = min(t, p->t);
        b = max(b, p->b);
    };
    void expandToContainRectangle(RECT*p) {
        SimpleRectangle sr(p);
        expandToContainRectangle(&sr);
    }
    void expand(int w, int h) {
        l -= w/2;
        r += w/2;
        t -= h/2;
        b += h/2;
    }
    void expand(int _l, int _r, int _t, int _b) {
        l -= _l;
        r += _r;
        t -= _t;
        b += _b;
    }
    
    void clipToBeInside(SimpleRectangle*p) {
        l = max(l, p->l);
        r = min(r, p->r);
        t = max(t, p->t);
        b = min(b, p->b);
    };
    void translation(int dx, int dy) {
        l += dx; r += dx; t += dy; b += dy;
    }
    void translationToBeInsideRectangle(SimpleRectangle*p) {
        int dx = 0, dy = 0;
        if ( l < p->l ) dx = p->l - l;
        else if ( r > p->r ) dx = p->r - r;
        if ( t < p->t ) dy = p->t - t;
        else if ( b > p->b ) dy = p->b - b;
        translation(dx, dy);
    }
    bool isProperSubset(SimpleRectangle*p) {
        bool bRet = true;
        if ( l > p->l ) bRet = false;
        else if ( r < p->r ) bRet = false;
        else if ( t > p->t ) bRet = false;
        else if ( b < p->b ) bRet = false;
        
        return bRet;
    }
};

KenBurnsRectanglesGenerator::KenBurnsRectanglesGenerator() {
    ratioForCuttingoffSamllFaces = 6;
    inverseRateForMagnification = 0.6;
    factorForTargetRectangle = 0.8;
    minSrcDestSizeRatio = 1.02;
    maxSrcDestSizeRatio = 1.06;
    maxRandomDegree = 15;
    srStart = new SimpleRectangle;
    srEnd = new SimpleRectangle;
};

KenBurnsRectanglesGenerator::~KenBurnsRectanglesGenerator() {
    delete srStart;
    delete srEnd;
};

//static
int KenBurnsRectanglesGenerator::comparator(const void* pFirst, const void* pSecond)
{
    int nFirstDimension = ( ((RECT*)pFirst)->right - ((RECT*)pFirst)->left ) * ( ((RECT*)pFirst)->bottom - ((RECT*)pFirst)->top );
    int nSecondDimension = ( ((RECT*)pSecond)->right - ((RECT*)pSecond)->left ) * ( ((RECT*)pSecond)->bottom - ((RECT*)pSecond)->top );
    return (nSecondDimension - nFirstDimension);
}

int KenBurnsRectanglesGenerator::makeEndRect(SimpleRectangle* pSrcSR, RECT* pFaceRects, int nFaceCount, int outputRatioW, int outputRatioH)
{
    if ( nFaceCount > 1 ) {
        // 1. Sort by Area
        qsort( pFaceRects, nFaceCount, sizeof(RECT), comparator);
        
        // 2. Cut off the small faces.
        int nFaceArea = 0, nPreFaceArea = 0, i;
        for ( i = 0 ; i < nFaceCount ; i++ ) {
            nFaceArea = ( pFaceRects[i].right - pFaceRects[i].left ) * ( pFaceRects[i].bottom - pFaceRects[i].top );
            
            if ( i > 0 ) {
                if ( nPreFaceArea > 100*100 && (nPreFaceArea/ratioForCuttingoffSamllFaces) > nFaceArea ) {
                    break;
                }
            }
            nPreFaceArea = nFaceArea;
        }
        nFaceCount = i;
    } else if ( nFaceCount == 0 )
        return 0;
    
    // 3. 남은 Face 영역들을 M1 ( 너비 기준 50% ) 확대한다.
    // Face detector 의 결과물에 따라 M1값을 정한다. 눈썹과 입을 포함한 사각형 정보를 주는 detector의 경우, 이 값이 클 수 있다.
    // 4. 영역(들)을 감싸는 최소 사각형을 구한다.
    
    srEnd->setFrom(pFaceRects);
    SimpleRectangle *pFaceSR = new SimpleRectangle[nFaceCount];
    
    for ( int i = 0 ; i < nFaceCount ; i++ ) {
        pFaceSR[i].setFrom(pFaceRects+i);
        
        
        int nWidthGap = (int)((float)pFaceSR[i].getWidth() / inverseRateForMagnification);
        int nHeightGap = (int)((float)pFaceSR[i].getHeight() / inverseRateForMagnification);
        pFaceSR[i].expand(nWidthGap/2,nWidthGap/2, nHeightGap/2,nHeightGap*2/3);
        //pFaceSR[i].expand(nWidthGap, nHeightGap);
        pFaceSR[i].clipToBeInside(pSrcSR);

        srEnd->expandToContainRectangle(pFaceSR+i);
    }
    delete [] pFaceSR;
    
    // make the ratio same as output.
    float r = ((float)srEnd->getWidth())/((float)srEnd->getHeight());
    float tr = ((float)outputRatioW)/((float)outputRatioH);
    
    if (r > tr) {
        // if too small, make height long.
        int gap;
        float fGap;
        if ( (fGap = ((float)pSrcSR->getHeight())*factorForTargetRectangle - (float)srEnd->getHeight()) > 0. ) { // too small
            srEnd->expand(0,(int)round(fGap));
            ////
            if (srEnd->t < 0) srEnd->translation(0,-srEnd->t);
            else if (srEnd->b > pSrcSR->b) srEnd->translation(0,pSrcSR->b-srEnd->b);
            ////
        }
        
        // enlarge the width to fit the ratio.
        int newv = (int)round((float)srEnd->getHeight()*tr);
        srEnd->expand(newv - srEnd->getWidth(), 0);
        
        ////
        if (srEnd->l < 0) srEnd->translation(-srEnd->l, 0);
        else if (srEnd->r > pSrcSR->r) srEnd->translation(pSrcSR->r-srEnd->r,0);
        ////
        
        // if overflow, shrink
        if ( (gap = pSrcSR->getWidth()-newv) < 0 ) { // overflow
            srEnd->expand(gap,(int)round((float)gap/tr));
        }
        
    } else {
        // if too small, make width long.
        int gap;
        float fGap;
        if ( (fGap = ((float)pSrcSR->getWidth())*factorForTargetRectangle - (float)srEnd->getWidth()) > 0. ) { // too small
            srEnd->expand((int)round(fGap),0);
            ////
            if (srEnd->l < 0) srEnd->translation(-srEnd->l, 0);
            else if (srEnd->r > pSrcSR->r) srEnd->translation(pSrcSR->r-srEnd->r,0);
            ////
        }

        // enlarge the height to fit the ratio.
        int newv = (int)round((float)srEnd->getWidth()/tr);
        srEnd->expand(0, newv - srEnd->getHeight());

        ////
        if (srEnd->t < 0) srEnd->translation(0,-srEnd->t);
        else if (srEnd->b > pSrcSR->b) srEnd->translation(0,pSrcSR->b-srEnd->b);
        ////

        // if overflow, shrink
        if ( (gap = pSrcSR->getHeight()-newv) < 0 ) { // overflow
            srEnd->expand((int)round((float)gap*tr),gap);
        }

    }
    
    //srEnd->translationToBeInsideRectangle(pSrcSR);
    
    
    
    // 5. 이 영역을 Cropping 사각형의 가로/세로 비율중 큰 값기준으로 D2 (예, ½) 보다 작으면, 최소 D2가 되도록 확대한다.
    // * 목적 사각형이 너무 작으면, 마지막 부분에 계단 현상이 있을 수 있다.
    //float r = ((float)srEnd->getWidth())/((float)srEnd->getHeight());
    //float tr = ((float)outputRatioW)/((float)outputRatioH);
    /*
     if ( r > tr ) {
     // width
     // factorForTargetRectangle : 0 ~ 1, 0 : don't care, 1 : full, 0.5 : half
     
     float fGap;
     if ( (fGap = ((float)pSrcSR->getWidth())*factorForTargetRectangle - (float)srEnd->getWidth()) > 0. ) { // too small
     int newheight = (int)round(((float)srEnd->getWidth()+fGap)/tr);
     srEnd->expand((int)fGap, newheight - srEnd->getHeight());
     }
     } else {
     // height
     
     float fGap;
     if ( (fGap = ((float)pSrcSR->getHeight())*factorForTargetRectangle - (float)srEnd->getHeight()) > 0. ) { // too small
     int newwidth = (int)(((float)srEnd->getHeight()+fGap)*tr);
     srEnd->expand( newwidth - srEnd->getWidth(), (int)fGap);
     }
     }
     */
    
    //이 영역과 원본 사각형과의 교집합 영역을 구한다.
    //srEnd->clipToBeInside(pSrcSR);
    return 0;
}

void PLOG( const char* pszFormat, ... )
{
    va_list va;
    char szBuf[256];
    char *pDebug = szBuf;
    
    szBuf[256-1] = '\0';
    
    va_start( va, pszFormat );
    vsnprintf( pDebug, 256-1, pszFormat, va );
    va_end( va );
    
#ifdef ANDROID
    //LOGW( "[Time %u] %s", nexSAL_GetTickCount(), pDebug );
    __android_log_print(ANDROID_LOG_INFO, "t", "%s", pDebug);
#else
    printf("%s",pDebug);
#endif
}



int KenBurnsRectanglesGenerator::makeStartRect(SimpleRectangle* pSrcSR, int durationInMSec)
{
    *srStart = *srEnd;
    
    durationInMSec = min(durationInMSec, 6000);
    
    // maxSrcDestSizeRatio, minSrcDestSizeRatio;
    // durationInMSec == 3000 : no change
    //
    
    float maxSizeRatio = 1.0 + (maxSrcDestSizeRatio-1.0) * (float)durationInMSec/3000.0;
    float minSizeRatio = 1.0 + (minSrcDestSizeRatio-1.0) * (float)durationInMSec/3000.0;
    float sumVariation = (float)(maxSrcDestSizeRatio-minSrcDestSizeRatio) * (float)durationInMSec/3000.0;

    
    //* old version:
    // 목적 사각형의 가로 기준 Random 값 R1 배 크기의 사각형을 구한다.
    // R1의 범위는 Range1~Range2 (예, 1.4 ~ 0.8 )
    //* new version :
    // R1의 범위는 End rectangle의 1.4 ~ 2.2 배
    
    
    float srcSizeRatio2Dest = 1.;
    int nr=0;
    int point1percent = 0;// = (rand() % (int)(1.0*1000. - 0.1*1000.)) + 0.1*1000.;
    
    //for (bool bDone=false; !bDone ;) {
    if ( (int)abs((maxSizeRatio - minSizeRatio) * 100.) != 0 ) {
        //point1percent = (rand() % (int)(1.0*1000. - 0.1*1000.)) + 0.1*1000.;
        nr = rand() % (int)abs((maxSizeRatio - minSizeRatio) * 100.);
        srcSizeRatio2Dest = (float)(nr + (int)round(minSizeRatio*100.))/100.;
        //PLOG("Eric. nr=%d,minSizeRatio=%f,srcSizeRatio2Dest=%f",nr,minSizeRatio,srcSizeRatio2Dest);
        int srcWidthGap = (int)((srcSizeRatio2Dest-1.0)*(float)srEnd->getWidth());
        int srcHeightGap = (int)((srcSizeRatio2Dest-1.0)*(float)srEnd->getHeight());
        
        srStart->expand(srcWidthGap, srcHeightGap);
        
        // if the size of start rectangle is bigger than source rectangle, fit its size to be inside source rectangle.
        if ( pSrcSR->getWidth() < srStart->getWidth() || pSrcSR->getHeight() < srStart->getHeight() ) {
            float sourceRatio = (float)pSrcSR->getWidth() / (float)pSrcSR->getHeight();
            float targetRatio = (float)srEnd->getWidth() / (float)srEnd->getHeight();
            int widthGap, heightGap;
            
            if ( sourceRatio > targetRatio ) {
                // height
                heightGap = srStart->getHeight() - pSrcSR->getHeight();
                //                if ( heightGap%2 == 1 ) heightGap
                widthGap = srStart->getWidth() - (int)round((float)pSrcSR->getHeight()*targetRatio);
            } else {
                // width
                widthGap = srStart->getWidth() - pSrcSR->getWidth();
                heightGap = srStart->getHeight() - (int)round((float)pSrcSR->getWidth()/targetRatio);
            }
            srStart->expand(-widthGap,-heightGap);
        }
        
        
        //if ( srcSizeRatio2Dest != 1.0 && (srStart->getWidth() <= pSrcSR->getWidth() && srStart->getHeight() <= pSrcSR->getHeight() ) ) {
        //            bDone = true;
        //} else {
        //    srStart->expand(-srcWidthGap, -srcHeightGap);
        //}
    }
    
    //PLOG("Eric. srcSizeRatio2Dest=%f(%d)", srcSizeRatio2Dest,nr);
    
    // real srcSizeRatioDest
    
    float srcSizeRealRatio2Dest = 1.;
    int rnr=0;
    
    //PLOG("Eric. 1 rnr=%d",rnr);
    srcSizeRealRatio2Dest = ((float)srStart->getWidth() / (float)srEnd->getWidth());
    float rgap = srcSizeRealRatio2Dest - minSizeRatio;
    if ( rgap < 0. ) {
        rnr = 0;
        //PLOG("Eric. 2 rnr=%d",rnr);
    }
    else {
        rnr = (int)round(rgap * 100.);
        //PLOG("Eric. 2 rnr=%d,rgap=%d",rnr,rgap);
    }

    // 목적 사각형의 중심점과 원본 영상의 중심점을 이은 선위에 목적 사각형 중심점에서 원본영상 중심점 방향으로 Random 값 R2 떨어진 점을 구한다. 이 점을 P라 한다.
    // * R2의 범위는 목적사각형 너비기준으로 하며, Range3~Range4 (예, 0.1 ~ 1.0)
    // * 목적 사각형 중심점과 원본 영상의 중심점이 같으면, 중심점으로 지나는 가로축을 기준으로 오른쪽 방향으로 한다. ( 이에 대한 좀 더 자세한 조절은 큰 의미 없다)
    //*100.;
    
    if ( abs(srcSizeRatio2Dest-1.0) > 0.75 )
        point1percent = (int)( 0.05 * 1000. );
    else if ( durationInMSec > 3000 )
        point1percent = (int)( abs( 0.9 - min(0.9,abs(srcSizeRatio2Dest-1.0)) ) * 1000.);
    else
        point1percent = (int)( abs( 0.5 - min(0.5,abs(srcSizeRatio2Dest-1.0)) ) * 1000.);
    //point1percent = (rand() % (int)(1.0*1000. - 0.1*1000.)) + 0.1*1000.;
    
    point1percent = ( sumVariation * 2. - (float)rnr/100.)*1000. / 1.;
    
    //PLOG("Eric. point1percent=%f (%f,%d)", point1percent,sumVariation,rnr);

#if 1
    int nLength = (int)((float)(srEnd->getWidth()+srEnd->getHeight()) / 2.5 * (float)point1percent / 1000.);
#else
    int nLength = (int)((float)pSrcSR->getWidth() / 2 * (float)point1percent / 1000.);
#endif
    
    //PLOG("Eric. nLength=%d", nLength);

    int deltaX = pSrcSR->getCenterX() - srStart->getCenterX();
    int deltaY = pSrcSR->getCenterY() - srStart->getCenterY();
    int newDeltaX, newDeltaY;
    
    if ( deltaX == 0 ) {
        newDeltaX = 0;
        newDeltaY = nLength;
    } else {
        // length^2 == ∆x^2 + ∆y^2;
        // ∆y = a(==h_slash_w) * ∆x;
        // ∆x = length / sqrt(1+a^2)
        
        double h_slash_w = (double)deltaY / (double)deltaX;
        newDeltaX = (int)round((double)nLength / sqrt(1.0+h_slash_w*h_slash_w));
        //newDeltaX = (int)round(sqrt((float)nLength*nLength/(1.0+h_slash_w*h_slash_w)));
        newDeltaX *= (deltaX < 0. ? -1. : 1.);
        newDeltaY = (int)round((double)newDeltaX * h_slash_w);
    }
    
    //목적 사각형의 중심점과 P을 이은 선을 목적 사각형 중심 기준으로 Random 값 R2 degree 만큼 회전 시킨다.
    //* R2의 범위는 Range3~Range4 (예, -15도 ~ 15도 )
    int theta = ( rand() % (maxRandomDegree*2) ) - maxRandomDegree;
    
    
    int newDeltaX2, newDeltaY2;
    double cosinev = cos((double)theta* 3.14159265/180.0);
    double sinev = sin((double)theta* 3.14159265/180.0);
    
    newDeltaX2 = (int)round((double)newDeltaX * cosinev - (double)newDeltaY * sinev);
    newDeltaY2 = (int)round((double)newDeltaX * sinev + (double)newDeltaY * cosinev);
    
    srStart->translation(newDeltaX2, newDeltaY2);

    if ( srStart->getWidth() < pSrcSR->getWidth() ) {
        if (srStart->l < 0) srStart->translation(-srStart->l, 0);
        else if (srStart->r > pSrcSR->r) srStart->translation(pSrcSR->r-srStart->r,0);

    } else {
        if ( srStart->l > pSrcSR->l || srStart->r < pSrcSR->r ) {
            int g = srStart->getWidth() - pSrcSR->getWidth();
            srStart->translation( -(srStart->l + g/2), 0 );
        }
    }

    if ( srStart->getHeight() < pSrcSR->getHeight() ) {
        if (srStart->t < 0) srStart->translation(0,-srStart->t);
        else if (srStart->b > pSrcSR->b) srStart->translation(0,pSrcSR->b-srStart->b);
        
    } else {
        if ( srStart->t > pSrcSR->t || srStart->b < pSrcSR->b ) {
            int g = srStart->getHeight()- pSrcSR->getHeight();
            srStart->translation( 0, -(srStart->t + g/2) );
        }
    }


//    srStart->translationToBeInsideRectangle(pSrcSR);
//    srStart->clipToBeInside(pSrcSR);
    //
    return 0;
}

int KenBurnsRectanglesGenerator::generate(RECT* pSrcRect, RECT* pFaceRects, int nFaceCount, int outputRatioW, int outputRatioH, int durationInMSec, RECT *pOutput)
{
    //RECT rectStart, rectEnd;
    SimpleRectangle srcSR(pSrcRect);
    RECT tempFace, * _pFaceRects = &tempFace;
    
    srand((unsigned) time(0));
    
    if ( 0 == nFaceCount ) {
        nFaceCount = 1;
        char n = rand() % 4;
        
        int width_unit = srcSR.getWidth() / 6;
        int height_unit = srcSR.getHeight() / 6;
        
        tempFace.left = ((n & 0x01) ? 3 : 1) * width_unit;
        tempFace.right = ((n & 0x01) ? 5 : 3) * width_unit;

        tempFace.top = ((n & 0x02) ? 3 : 1) * height_unit;
        tempFace.bottom = ((n & 0x02) ? 5 : 3) * height_unit;

    } else
        _pFaceRects = pFaceRects;
    
    makeEndRect(&srcSR, _pFaceRects, nFaceCount, outputRatioW, outputRatioH);
    makeStartRect(&srcSR, durationInMSec);
    
    srStart->setTo(&pOutput[0]);
    srEnd->setTo(&pOutput[1]);
    
    //rectStart = rectEnd;
    
    return 0;
}

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/03/28    Draft.
 -----------------------------------------------------------------------------*/
