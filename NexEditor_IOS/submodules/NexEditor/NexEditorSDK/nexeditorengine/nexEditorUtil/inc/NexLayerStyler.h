/******************************************************************************
* File Name   : NexLayerStyler.h
* Description :
*******************************************************************************
* Copyright (c) 2002-2019 KineMaster Corporation. All rights reserved.
* http://www.kinemaster.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/


#ifndef NexLayerStyler_h
#define NexLayerStyler_h

#include <stdio.h>
#include <stdlib.h>
#include "NexShadersForText.h"
#include "NexOffScreenFrameBuffer.h"

typedef struct _DropShadowParameters {
    float distance;
    float size;
    float spread;
    float angle;
    float shadowColor[4];
} DropShadowParameters;


class NexLayerStyler {
public:
    NexLayerStyler();
    virtual ~NexLayerStyler();
    
    enum COMMAND {
        CMD_DROPSHADOW = 0, // refer to DropShadowParameters
        CMD_DROPSHADOW_ONLY_SHADOW = 1, // only shadow without the source image, distance and angle.
        CMD_DRAWTEX = 2,    // no parameter
        CMD_LAST = 3
    };
    
    void setSourceTex(GLuint texName, int w, int h);
    void setTargetTex(GLuint texName, int w, int h);
    bool setParam(COMMAND cmd, void* pValue, int valueSizeInByte);
    bool makeResult(COMMAND cmd);
    
    // if setTargetTex was not called, we can get tex from this api.
    // ownerShipTransferToCaller has a meaning when targetTex was created inside this class object.
    // when ownerShipTransferToCaller was set to true, caller should delete the texture.
    GLuint getResultTex(bool ownerShipTransferToCaller = false);
    // This deleteTex function just call glDeleteTextures with logging.
    // When you call getResultTex(true), use this function.
    static void deleteTex(GLuint texName);
    
private:
    GLuint mSourceTexName;
    int mSourceTexWidth;
    int mSourceTexHeight;

    GLuint mTargetTexName;
    int mTargetTexWidth;
    int mTargetTexHeight;

    bool areShadersReady[CMD_LAST];
    NexOffScreenFrameBuffer mFB1;
    NexOffScreenFrameBuffer mFB2;
    
    // for CMD_DROPSHADOW
    DropShadowParameters* mParametersForDropShader;
    static NexShaderDropShadowFirst* mDropShadowStep1;
    static int mRC4DropShadowStep1;
    static NexShaderDropShadowSecond* mDropShadowStep2;
    static int mRC4DropShadowStep2;
    static NexShaderDropShadowThird* mDropShadowStep3;
    static int mRC4DropShadowStep3;
    static NexShaderAntiAlias* mAntiAliaser;
    static int mRC4AntiAliaser;
    
    bool processDropShadow();
    bool processOnlyDropShadow();

    // for CMD_DRAWTEX
    static NexShaderDrawTex* mDrawTexProgram;
    static int mRC4DrawTex;
    bool processDrawTex();

    bool loadShaders(COMMAND cmd);
    
    virtual void onDrawn() {};
};

class NexLayerStylerForReadingPixels : public NexLayerStyler
{
public:
    NexLayerStylerForReadingPixels() : NexLayerStyler(), bufferPointer(NULL) {};
    virtual ~NexLayerStylerForReadingPixels() { freeResultRGBABuffer(); };
    uint8_t* getResultRGBABuffer() { return bufferPointer; };
    void freeResultRGBABuffer() { if ( bufferPointer ) free(bufferPointer); bufferPointer = NULL; }
private:
    uint8_t* bufferPointer;
    virtual void onDrawn();
};


#endif /* NexLayerStyler_h */

/*-----------------------------------------------------------------------------
Revision History
Author        Date        Description of Changes
-------------------------------------------------------------------------------
Eric        2019/09/02    Draft.
Eric        2019/10/14    Add CMD_DROPSHADOW_ONLY_SHADOW
Eric        2019/10/24    NexLayerStylerForReadingPixels was added.
-----------------------------------------------------------------------------*/
