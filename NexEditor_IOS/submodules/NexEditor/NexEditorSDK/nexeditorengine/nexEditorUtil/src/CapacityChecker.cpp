/******************************************************************************
 * File Name   :    CapacityChecker.cpp
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author    Date        Description of Changes
 -------------------------------------------------------------------------------
 cskb        2012/08/29    Draft.
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#include "CapacityChecker.h"
#ifdef _ANDROID
#include  <android/log.h>
#endif

#define DEFAULT_PHONE_MODEL_IDENTIFIER "iPhone9,1" /*IPHONE_7*/
#define DEFAULT_OTHER_MODEL_IDENTIFIER "iPad6,3" /*IPAD_PRO_9*/

static const struct {
    char    sModelName[16];
    int        iWidth;
    int        iHeight;
    int        iSupportedCodecCount;
    int        iSupportedMemorySize;
} SupportedCodecMemoryTable[] = {
    /*IPHONE_5S*/        {"iPhone6,1",    1920, 1080,  7,  8294400},
    /*IPHONE_5S*/        {"iPhone6,2",    1920, 1080,  7,  8294400},
    /*IPHONE_6P*/        {"iPhone7,1",    1920, 1080,  8,  8294400},
    /*IPHONE_6*/        {"iPhone7,2",    1920, 1080, 10, 14515200},
    /*IPHONE_6S*/        {"iPhone8,1",    3840, 2160, 10, 16588800},
    /*IPHONE_6SP*/        {"iPhone8,2",    3840, 2160, 10, 16588800},
    /*IPHONE_SE*/        {"iPhone8,4",    1920, 1080, 10, 16588800},
    /*IPHONE_7*/        {"iPhone9,1",    3840, 2160,    5,    16588800},
    /*IPHONE_7P*/        {"iPhone9,2",    3840, 2160,    10, 24883200},
    /*IPHONE_7*/        {"iPhone9,3",    3840, 2160,    5,    16588800},
    /*IPHONE_7P*/        {"iPhone9,4",    3840, 2160,    10, 24883200},
    /*IPHONE_8*/        {"iPhone10,1",    3840, 2160,    3,    16588800},
    /*IPHONE_8P*/        {"iPhone10,2",    3840, 2160,    9,    24883200},
    /*IPHONE_X*/        {"iPhone10,3",    3840, 2160,    9,    16588800},
    /*IPHONE_8*/        {"iPhone10,4",    3840, 2160,    3,    16588800},
    /*IPHONE_8P*/        {"iPhone10,5",    3840, 2160,    9,    24883200},
    /*IPHONE_X*/        {"iPhone10,6",    3840, 2160,    9,    16588800},
    /*IPHONE_XS*/        {"iPhone11,2",    3840, 2160,    10,    24883200},
    /*IPHONE_XSMAX*/    {"iPhone11,4",    3840, 2160,    10,    24883200},
    /*IPHONE_XSMAX*/    {"iPhone11,6",    3840, 2160,    10,    24883200},
    /*IPHONE_XR*/        {"iPhone11,8",    3840, 2160,    10,    16588800},
    
    /*IPOD_TOUCH_6G*/    {"iPod7,1",        1920, 1080,    10,    16588800},
    
    /*IPAD_AIR*/        {"iPad4,1",        1920, 1080,    7,     8294400},
    /*IPAD_AIR*/        {"iPad4,2",        1920, 1080,    7,     8294400},
    /*IPAD_AIR*/        {"iPad4,3",        1920, 1080,    7,     8294400},
    /*IPAD_MINI_2*/        {"iPad4,4",        1920, 1080,    10,     8294400},
    /*IPAD_MINI_2*/        {"iPad4,5",        1920, 1080,    10,     8294400},
    /*IPAD_MINI_2*/        {"iPad4,6",        1920, 1080,    10,     8294400},
    /*IPAD_MINI_3*/        {"iPad4,7",        1920, 1080,    8,     8294400},
    /*IPAD_MINI_3*/        {"iPad4,9",        1920, 1080,    8,     8294400},
    /*IPAD_MINI_3*/        {"iPad4,8",        1920, 1080,    8,     8294400},
    /*IPAD_MINI_4*/        {"iPad5,1",        1920, 1080,    10,    12441600},
    /*IPAD_MINI_4*/        {"iPad5,2",        1920, 1080,    10,    12441600},
    /*IPAD_AIR_2*/        {"iPad5,3",        1920, 1080,    10,    12441600},
    /*IPAD_AIR_2*/        {"iPad5,4",        1920, 1080,    10,    12441600},
    /*IPAD_PRO_9*/        {"iPad6,3",        1920, 1080,    10,    24883200},
    /*IPAD_PRO_9*/        {"iPad6,4",        1920, 1080,    10,    24883200},
    /*IPAD_PRO_12*/        {"iPad6,7",        1920, 1080, 10,    24883200},
    /*IPAD_PRO_12*/        {"iPad6,8",        1920, 1080, 10,    24883200},
    /*IPAD_5_GEN*/        {"iPad6,11",    3840, 2160, 10,    16588800},
    /*IPAD_5_GEN*/        {"iPad6,12",    3840, 2160, 10,    16588800},
    /*IPAD_PRO_12_2GEN*/{"iPad7,1",        3840, 2160,    10,    24883200},
    /*IPAD_PRO_12_2GEN*/{"iPad7,2",        3840, 2160,    10,    24883200},
    /*IPAD_PRO_10*/        {"iPad7,3",        3840, 2160,    10,    24883200},
    /*IPAD_PRO_10*/        {"iPad7,4",        3840, 2160,    10,    24883200},
    /*IPAD_6_GEN*/        {"iPad7,5",        3840, 2160, 10,    16588800},
    /*IPAD_6_GEN*/        {"iPad7,6",        3840, 2160, 10,    16588800},
    /*IPAD_PRO_11*/        {"iPad8,1",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_11*/        {"iPad8,2",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_11*/        {"iPad8,3",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_11*/        {"iPad8,4",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_12_3GEN*/{"iPad8,5",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_12_3GEN*/{"iPad8,6",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_12_3GEN*/{"iPad8,7",        3840, 2160,    10,    33177600},
    /*IPAD_PRO_12_3GEN*/{"iPad8,8",        3840, 2160,    10,    33177600},
    
    // The following devices are not supported.
    /*IPHONE*/            {"iPhone1,1",       0,    0,  0,        0},
    /*IPHONE_3G*/        {"iPhone1,2",       0,    0,  0,        0},
    /*IPHONE_3GS*/        {"iPhone2,1",       0,    0,  0,        0},
    /*IPHONE_4*/        {"iPhone3,1",       0,    0,  0,        0},
    /*IPHONE_4*/        {"iPhone3,2",       0,    0,  0,        0},
    /*IPHONE_4*/        {"iPhone3,3",       0,    0,  0,        0},
    /*IPHONE_4S*/        {"iPhone4,1",       0,    0,  0,        0},
    /*IPHONE_5*/        {"iPhone5,1",       0,    0,  0,        0},
    /*IPHONE_5*/        {"iPhone5,2",       0,    0,  0,        0},
    /*IPHONE_5C*/        {"iPhone5,3",       0,    0,  0,        0},
    /*IPHONE_5C*/        {"iPhone5,4",       0,    0,  0,        0},
    
    /*IPOD_TOUCH*/        {"iPod1,1",           0,    0,  0,        0},
    /*IPOD_TOUCH_2G*/    {"iPod2,1",           0,    0,  0,        0},
    /*IPOD_TOUCH_3G*/    {"iPod3,1",           0,    0,  0,        0},
    /*IPOD_TOUCH_4G*/    {"iPod4,1",           0,    0,  0,        0},
    /*IPOD_TOUCH_5G*/    {"iPod5,1",           0,    0,  0,        0},
    
    /*IPAD*/            {"iPad1,1",           0,    0,  0,        0},
    /*IPAD2*/            {"iPad2,1",           0,    0,  0,        0},
    /*IPAD2*/            {"iPad2,2",           0,    0,  0,        0},
    /*IPAD2*/            {"iPad2,3",           0,    0,  0,        0},
    /*IPAD2*/            {"iPad2,4",           0,    0,  0,        0},
    /*IPAD_MINI*/        {"iPad2,5",           0,    0,  0,        0},
    /*IPAD_MINI*/        {"iPad2,6",           0,    0,  0,        0},
    /*IPAD_MINI*/        {"iPad2,7",           0,    0,  0,        0},
    /*IPAD3*/            {"iPad3,1",           0,    0,  0,        0},
    /*IPAD3*/            {"iPad3,2",           0,    0,  0,        0},
    /*IPAD3*/            {"iPad3,3",           0,    0,  0,        0},
    /*IPAD4*/            {"iPad3,4",           0,    0,  0,        0},
    /*IPAD4*/            {"iPad3,5",           0,    0,  0,        0},
    /*IPAD4*/            {"iPad3,6",           0,    0,  0,        0}
};

void P_LOG( const char* pszFormat, ... )
{
    va_list va;
    char szBuf[256];
    char *pDebug = szBuf;
    
    szBuf[256-1] = '\0';
    
    va_start( va, pszFormat );
    vsnprintf( pDebug, 256-1, pszFormat, va );
    va_end( va );

#ifdef _ANDROID
    //LOGW( "[Time %u] %s", nexSAL_GetTickCount(), pDebug );
    __android_log_print(ANDROID_LOG_INFO, "t", "%s", pDebug);
#else
    printf("%s",pDebug);
#endif
}

CapacityChecker::CapacityChecker( void )
{
}

CapacityChecker::~CapacityChecker( void )
{
}

int CapacityChecker::getDeviceModel(char *pModelIdentifier)
{
    int iRet = -1;
    int iDefaultDeviceIndex = -1;
    char *pDefaultModelIdentifier = NULL;
    
    if (pModelIdentifier)
    {
        if (!strncmp(pModelIdentifier, "iPhone", 6))
            pDefaultModelIdentifier = DEFAULT_PHONE_MODEL_IDENTIFIER;
        else
            pDefaultModelIdentifier = DEFAULT_OTHER_MODEL_IDENTIFIER;
        
        for (int iIndex = 0 ; iIndex < sizeof(SupportedCodecMemoryTable) ; iIndex++)
        {
            if (iDefaultDeviceIndex == -1)
            {
                if (!strncmp(SupportedCodecMemoryTable[iIndex].sModelName, pDefaultModelIdentifier, sizeof(SupportedCodecMemoryTable[iIndex].sModelName) - 1))
                    iDefaultDeviceIndex= iIndex;;
            }
            
            if (!strncmp(SupportedCodecMemoryTable[iIndex].sModelName, pModelIdentifier, sizeof(SupportedCodecMemoryTable[iIndex].sModelName) - 1))
            {
                iRet = iIndex;
                break;
            }
        }
    }
    
    if (iRet == -1)
    {
        P_LOG("[CapabilityChecker.cpp %d] getDeviceModel, can't find the registed device so set Iphone 7", __LINE__);
        iRet = iDefaultDeviceIndex;
    }
    
    return iRet;
}

int CapacityChecker::isAvailableMemoryCapacity(char *pModelIdentifier, int iCount, void* pInfo)
{
    int iRet = 0;
    int iSupportedCodecCount = 0, iSupportedMemorySize = 0, iSupportedWidth = 0, iSupportedHeight = 0;
    
    int iIndex = getDeviceModel(pModelIdentifier);
    
    iSupportedWidth            = SupportedCodecMemoryTable[iIndex].iWidth;
    iSupportedHeight        = SupportedCodecMemoryTable[iIndex].iHeight;
    iSupportedCodecCount    = SupportedCodecMemoryTable[iIndex].iSupportedCodecCount;
    iSupportedMemorySize    = SupportedCodecMemoryTable[iIndex].iSupportedMemorySize;
    
    if ( iSupportedWidth        == 0 ||
        iSupportedHeight        == 0 ||
        iSupportedCodecCount    == 0 ||
        iSupportedMemorySize    == 0 )
    {
        P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable failed, iSupportedWidth(%d), iSupportedHeight(%d), iSupportedCodecCount(%d), iSupportedMemorySize(%d)", __LINE__, iSupportedWidth, iSupportedHeight, iSupportedCodecCount, iSupportedMemorySize);
        return iRet;
    }
    
    VideoInfo *pVideoInfo = (VideoInfo*)pInfo;
    if (pVideoInfo)
    {
        int iContentWidth = 0, iContentHeight = 0, iContentFps = 0;
        float fWeight = 1.0;
        int iAppliedCodecCount = iCount > iSupportedCodecCount ? iSupportedCodecCount : iCount;
        
        for (int i = 0 ; i < iAppliedCodecCount ; i++)
        {
            P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable index:%d, width:%d, height:%d, fps:%d", __LINE__, i, pVideoInfo[i].width, pVideoInfo[i].height, pVideoInfo[i].fps);
            
            iContentWidth    = pVideoInfo[i].width;
            iContentHeight    = pVideoInfo[i].height;
            iContentFps        = pVideoInfo[i].fps;
            
            if (iContentWidth == 0 || iContentHeight == 0 || iContentFps == 0)
            {
                if (iContentWidth == 0 || iContentHeight == 0)
                {
                    iContentWidth    = iSupportedWidth;
                    iContentHeight    = iSupportedHeight;
                }
                if (iContentFps == 0)
                {
                    iContentFps = 60;
                }
                P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable, some video info is zero (index:%d, w:%d, h:%d, fps:%d) so change to (w:%d, h:%d, fps:%d) ", __LINE__, i, pVideoInfo[i].width, pVideoInfo[i].height, pVideoInfo[i].fps, iContentWidth, iContentHeight, iContentFps);
            }
            
            if (iSupportedWidth * iSupportedHeight < iContentWidth * iContentHeight)
            {
                P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable Fail, resolution is exceeded (index:%d) (%d, %d, %d, %d) ", __LINE__, i, iSupportedWidth, iSupportedHeight, iContentWidth, iContentHeight);
                iRet = i;
                break;
            }
#if 0
            if (iContentFps > 0 && iContentFps <= 33)
                fWeight = 0.5;
            else if (iContentFps > 33 && iContentFps <= 63)
                fWeight = 1;
            else if (iContentFps > 63 && iContentFps <= 93)
                fWeight = 1.5;
            else if (iContentFps >= 94)
                fWeight = 2;
#endif
            iSupportedMemorySize -= (int)(iContentWidth * iContentHeight * fWeight);
            if (iSupportedMemorySize < 0)
            {
                P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable Fail, is not enough memory. (%d, %d, %d) ", __LINE__, i, iCount, iSupportedMemorySize);
                break;
            }
            iRet = i+1;
        }
    }
    
    P_LOG("[CapabilityChecker.cpp %d] isHardWareResourceAvailable iRet:%d, pModelIdentifier:%s, iRemainedMemorySize:%d", __LINE__, iRet, pModelIdentifier, iSupportedMemorySize);
    return iRet;
}

int CapacityChecker::getAvailableMemoryCapacity(char *pModelIdentifier, unsigned int *pMaxMemorySize, unsigned int *pMaxDecoderCount)
{
	int iRet = 0;
	int iIndex = getDeviceModel(pModelIdentifier);

	if (pMaxMemorySize && pMaxDecoderCount)
	{
		*pMaxMemorySize	= SupportedCodecMemoryTable[iIndex].iSupportedMemorySize;
		*pMaxDecoderCount = SupportedCodecMemoryTable[iIndex].iSupportedCodecCount;
		P_LOG("[CapabilityChecker.cpp %d] getAvailableMemoryCapacity pModelIdentifier:%s, pMaxMemorySize:%d, pMaxDecoderCount:%d", __LINE__, pModelIdentifier, *pMaxMemorySize, *pMaxDecoderCount);
	}
	else
	{
		P_LOG("[CapabilityChecker.cpp %d] getAvailableMemoryCapacity pModelIdentifier:%s NULL", __LINE__, pModelIdentifier);	
	}

	return iRet;
}

