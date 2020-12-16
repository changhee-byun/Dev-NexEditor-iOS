/******************************************************************************
* File Name   : NexLayerStyler.cpp
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

#include <stdlib.h>
#include <string.h>
#include "NexLayerStyler.h"

#define LOGE printf

//static member variables.
NexShaderDropShadowFirst* NexLayerStyler::mDropShadowStep1 = NULL;
int NexLayerStyler::mRC4DropShadowStep1 = 0;
NexShaderDropShadowSecond* NexLayerStyler::mDropShadowStep2 = NULL;
int NexLayerStyler::mRC4DropShadowStep2 = 0;
NexShaderDropShadowThird* NexLayerStyler::mDropShadowStep3 = NULL;
int NexLayerStyler::mRC4DropShadowStep3 = 0;

NexShaderAntiAlias* NexLayerStyler::mAntiAliaser = NULL;
int NexLayerStyler::mRC4AntiAliaser = 0;

NexShaderDrawTex* NexLayerStyler::mDrawTexProgram = NULL;
int NexLayerStyler::mRC4DrawTex = 0;

NexLayerStyler::NexLayerStyler()
: mSourceTexName(-1)
, mSourceTexWidth(0)
, mSourceTexHeight(0)
, mTargetTexName(-1)
, mTargetTexWidth(0)
, mTargetTexHeight(0)
, mParametersForDropShader(NULL)
{
    for ( int i = 0 ; i < CMD_LAST ; i++ ) {
        areShadersReady[i] = false;
    }
    
    //LOGE("LAYERSTYLE:NexLayerStyler object was created.\n");
}

NexLayerStyler::~NexLayerStyler()
{
    for ( int i = 0 ; i < CMD_LAST ; i++ ) {
        if ( i == CMD_DROPSHADOW && areShadersReady[i] ) { // CMD_DROPSHADOW
            mRC4DropShadowStep1--;
            if ( 0 == mRC4DropShadowStep1 ) {
                delete mDropShadowStep1;
                mDropShadowStep1 = NULL;
            }

            mRC4DropShadowStep2--;
            if ( 0 == mRC4DropShadowStep2 ) {
                delete mDropShadowStep2;
                mDropShadowStep2 = NULL;
            }

            mRC4DropShadowStep3--;
            if ( 0 == mRC4DropShadowStep3 ) {
                delete mDropShadowStep3;
                mDropShadowStep3 = NULL;
            }
            
            mRC4AntiAliaser--;
            if ( 0 == mRC4AntiAliaser ) {
                delete mAntiAliaser;
                mAntiAliaser = NULL;
            }
        }
        if ( i == CMD_DROPSHADOW_ONLY_SHADOW && areShadersReady[i] ) {
            mRC4DropShadowStep1--;
            if ( 0 == mRC4DropShadowStep1 ) {
                delete mDropShadowStep1;
                mDropShadowStep1 = NULL;
            }

            mRC4DropShadowStep2--;
            if ( 0 == mRC4DropShadowStep2 ) {
                delete mDropShadowStep2;
                mDropShadowStep2 = NULL;
            }

            mRC4AntiAliaser--;
            if ( 0 == mRC4AntiAliaser ) {
                delete mAntiAliaser;
                mAntiAliaser = NULL;
            }
        }
        if ( i == CMD_DRAWTEX && areShadersReady[i] ) { // CMD_DRAWTEX
            mRC4DrawTex--;
            if ( 0 == mRC4DrawTex ) {
                delete mDrawTexProgram;
                mDrawTexProgram = NULL;
            }
        }
    }
    
    if ( mParametersForDropShader ) {
        free(mParametersForDropShader);
        mParametersForDropShader = NULL;
    }
    
    mFB1.destroy();
    mFB2.destroy();
    
    //LOGE("LAYERSTYLE:NexLayerStyler object was delete.\n");
}
    
void NexLayerStyler::setSourceTex(GLuint texName, int w, int h)
{
    mSourceTexName = texName;
    mSourceTexWidth = w;
    mSourceTexHeight = h;
}

void NexLayerStyler::setTargetTex(GLuint texName, int w, int h)
{
    mTargetTexName = texName;
    mTargetTexWidth = w;
    mTargetTexHeight = h;
}

bool NexLayerStyler::setParam(COMMAND cmd, void* pValue, int valueSizeInByte)
{
    if ( CMD_DROPSHADOW == cmd || CMD_DROPSHADOW_ONLY_SHADOW == cmd ) {
        if ( sizeof(DropShadowParameters) != valueSizeInByte )
            return false;
        
        if ( !mParametersForDropShader ) {
            mParametersForDropShader = (DropShadowParameters*)malloc(sizeof(DropShadowParameters));
        }
        
        memcpy(mParametersForDropShader, pValue, sizeof(DropShadowParameters));
        
        return true;
    }
    
    return false;
}

bool NexLayerStyler::processOnlyDropShadow()
{
    if ( !mParametersForDropShader )
        return false;
    
    mFB1.create(mSourceTexWidth, mSourceTexHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, mFB1);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mDropShadowStep1->setTex(mSourceTexName, mSourceTexWidth, mSourceTexHeight);
    mDropShadowStep1->setSize(mParametersForDropShader->size);
    mDropShadowStep1->draw();

    
    mFB2.create(mSourceTexWidth, mSourceTexHeight);//, mTargetTexName );
    glBindFramebuffer(GL_FRAMEBUFFER, mFB2);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mDropShadowStep2->setTex(mFB1.getTex(),mSourceTexWidth, mSourceTexHeight);
    mDropShadowStep2->setOriginalTex(mSourceTexName);
    mDropShadowStep2->setSize(mParametersForDropShader->size);
    mDropShadowStep2->setColor(mParametersForDropShader->shadowColor[0],
                               mParametersForDropShader->shadowColor[1],
                               mParametersForDropShader->shadowColor[2],
                               mParametersForDropShader->shadowColor[3]);
    mDropShadowStep2->setSoftness(mParametersForDropShader->spread);

    mDropShadowStep2->draw();
    
    NexOffScreenFrameBuffer* pTemp = NULL;
    if ( -1 == mTargetTexName ) {

        glBindFramebuffer(GL_FRAMEBUFFER, mFB1);
        glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        pTemp = new NexOffScreenFrameBuffer;
        pTemp->create(mSourceTexWidth, mSourceTexHeight,mTargetTexName);
        glBindFramebuffer(GL_FRAMEBUFFER, *pTemp);
        glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    mAntiAliaser->setTex(mFB2.getTex(), mSourceTexWidth, mSourceTexHeight);
    mAntiAliaser->draw();

    if ( -1 != mTargetTexName ) {
        mFB1.destroy();
        //ASSERT(pTemp);
        if (pTemp) {
            pTemp->destroy();
            delete pTemp;
            pTemp = NULL;
        }
    }

    mFB2.destroy();
    
    return true;
}

bool NexLayerStyler::processDropShadow()
{
    if ( !mParametersForDropShader )
        return false;

    mFB2.create(mSourceTexWidth, mSourceTexHeight);//, mTargetTexName );
    glBindFramebuffer(GL_FRAMEBUFFER, mFB2);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mDropShadowStep1->setTex(mSourceTexName, mSourceTexWidth, mSourceTexHeight);
    mDropShadowStep1->setSize(mParametersForDropShader->size);
    mDropShadowStep1->draw();

    mFB1.create(mSourceTexWidth, mSourceTexHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, mFB1);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mDropShadowStep2->setTex(mFB2.getTex(),mSourceTexWidth, mSourceTexHeight);
    mDropShadowStep2->setOriginalTex(mSourceTexName);
    mDropShadowStep2->setSize(mParametersForDropShader->size);
    mDropShadowStep2->setColor(mParametersForDropShader->shadowColor[0],
                               mParametersForDropShader->shadowColor[1],
                               mParametersForDropShader->shadowColor[2],
                               mParametersForDropShader->shadowColor[3]);
    mDropShadowStep2->setSoftness(mParametersForDropShader->spread);
    mDropShadowStep2->draw();
    
    glBindFramebuffer(GL_FRAMEBUFFER, mFB2);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mAntiAliaser->setTex(mFB1.getTex(),mSourceTexWidth, mSourceTexHeight);
    mAntiAliaser->draw();

    NexOffScreenFrameBuffer* pTemp = NULL;
    
    if ( -1 == mTargetTexName ) {
        glBindFramebuffer(GL_FRAMEBUFFER, mFB1);
        glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
        // resuse fb1
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        pTemp = new NexOffScreenFrameBuffer;
        pTemp->create(mSourceTexWidth, mSourceTexHeight,mTargetTexName);
        glBindFramebuffer(GL_FRAMEBUFFER, *pTemp);
        glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    mDropShadowStep3->setTex(mFB2.getTex(), mSourceTexWidth, mSourceTexHeight);
    mDropShadowStep3->setOriginalTex(mSourceTexName);
    mDropShadowStep3->setDistance(mParametersForDropShader->distance);
    mDropShadowStep3->setAngle(mParametersForDropShader->angle);

    mDropShadowStep3->draw();

    if ( -1 != mTargetTexName ) {
        mFB1.destroy();
        //ASSERT(pTemp);
        if (pTemp) {
            pTemp->destroy();
            delete pTemp;
            pTemp = NULL;
        }
    }

    mFB2.destroy();
    // mFB1 has the result tex.
    
    return true;
}

bool NexLayerStyler::processDrawTex()
{
    mFB1.create(mSourceTexWidth, mSourceTexHeight, mTargetTexName);
    glBindFramebuffer(GL_FRAMEBUFFER, mFB1);
    glViewport(0, 0, mSourceTexWidth, mSourceTexHeight);
    
    if ( -1 == mTargetTexName ) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    mDrawTexProgram->setTex(mSourceTexName);
    mDrawTexProgram->draw();

    return true;
}

bool NexLayerStyler::makeResult(COMMAND cmd)
{
    if ( mSourceTexName == -1 || mSourceTexWidth <= 0 || mSourceTexHeight <= 0 ) {
        return false;
    }

    if ( !loadShaders(cmd) )
        return false;

    GLint old_fbo = -1;
    GLint old_viewport[4];
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
    glGetIntegerv(GL_VIEWPORT, &old_viewport[0]);

    bool bRet = false;
    
    switch( cmd ) {
        case CMD_DROPSHADOW :
            bRet = processDropShadow();
            break;
        case CMD_DROPSHADOW_ONLY_SHADOW :
            bRet = processOnlyDropShadow();
            break;
        case CMD_DRAWTEX :
            bRet = processDrawTex();
            break;
        default :
            bRet = false;
    }
    
    this->onDrawn();
   
    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

    return bRet;
}

bool NexLayerStyler::loadShaders(COMMAND cmd)
{
    if ( CMD_DROPSHADOW == cmd ) {
        if ( !areShadersReady[cmd] ) {
            if ( 0 == mRC4DropShadowStep1 ) {
                mDropShadowStep1 = new NexShaderDropShadowFirst();
                mDropShadowStep1->prepare();
            }
            mRC4DropShadowStep1++;
            
            if ( 0 == mRC4DropShadowStep2 ) {
                mDropShadowStep2 = new NexShaderDropShadowSecond();
                mDropShadowStep2->prepare();
            }
            mRC4DropShadowStep2++;
            
            if ( 0 == mRC4DropShadowStep3 ) {
                mDropShadowStep3 = new NexShaderDropShadowThird();
                mDropShadowStep3->prepare();
            }
            mRC4DropShadowStep3++;
            
            if ( 0 == mRC4AntiAliaser ) {
                mAntiAliaser = new NexShaderAntiAlias();
                mAntiAliaser->prepare();
            }
            mRC4AntiAliaser++;

            areShadersReady[cmd] = true;
        }
    }
    else if ( CMD_DROPSHADOW_ONLY_SHADOW == cmd ) {
        if ( !areShadersReady[cmd] ) {
            if ( 0 == mRC4DropShadowStep1 ) {
                mDropShadowStep1 = new NexShaderDropShadowFirst();
                mDropShadowStep1->prepare();
            }
            mRC4DropShadowStep1++;
            
            if ( 0 == mRC4DropShadowStep2 ) {
                mDropShadowStep2 = new NexShaderDropShadowSecond();
                mDropShadowStep2->prepare();
            }
            mRC4DropShadowStep2++;
            
            if ( 0 == mRC4AntiAliaser ) {
                mAntiAliaser = new NexShaderAntiAlias();
                mAntiAliaser->prepare();
            }
            mRC4AntiAliaser++;

            areShadersReady[cmd] = true;
        }
    }
    else if ( CMD_DRAWTEX == cmd ) {
        if ( !areShadersReady[cmd] ) {
            if ( 0 == mRC4DrawTex ) {
                mDrawTexProgram = new NexShaderDrawTex();
                mDrawTexProgram->prepare();
            }
            mRC4DrawTex++;

            areShadersReady[cmd] = true;
        }
    }
    else
        return false;
    
    return true;
}

GLuint NexLayerStyler::getResultTex(bool ownershipTransferToCaller/* = false*/)
{
    if (mTargetTexName != -1)
        return mTargetTexName;
    
    if ( ownershipTransferToCaller )
        mFB1.releaseTexOwnership();
    
    return mFB1.getTex();
}

//static
void NexLayerStyler::deleteTex(GLuint texName)
{
    glDeleteTextures(1, &texName);
#ifdef LOGGING_GLRESOURCE
    LOGE("LAYERSTYLE: del TEX %d\n", texName);
#endif
}

//
// Implementation part of
// NexLayerStylerForReadingPixels
//

void NexLayerStylerForReadingPixels::onDrawn()
{
    GLint dims[4] = {0}; //x,y,width,height from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGet.xhtml
    glGetIntegerv(GL_VIEWPORT, dims);
    
    freeResultRGBABuffer();
    
    const int MAX_RESULTION_WIDTH = 4096;
    if ( dims[2] > MAX_RESULTION_WIDTH || dims[2] < 0 || dims[3] > MAX_RESULTION_WIDTH || dims[3] < 0 )
        return;
    
    bufferPointer = (uint8_t*)malloc(dims[2]*dims[3]*4);
    glReadPixels(dims[0],dims[1],dims[2],dims[3],GL_RGBA,GL_UNSIGNED_BYTE, bufferPointer);
}

/*-----------------------------------------------------------------------------
Revision History
Author        Date        Description of Changes
-------------------------------------------------------------------------------
Eric        2019/09/02    Draft.
Eric        2019/10/14    Add CMD_DROPSHADOW_ONLY_SHADOW
Eric        2019/10/24    NexLayerStylerForReadingPixels was added.
-----------------------------------------------------------------------------*/
