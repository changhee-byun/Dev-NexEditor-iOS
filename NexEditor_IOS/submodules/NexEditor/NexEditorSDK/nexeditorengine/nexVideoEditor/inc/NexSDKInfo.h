/******************************************************************************
* File Name   :	NexSDKInfo.h
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#ifndef _NEXSDKINFORMATION_H_INCLUDED_
#define _NEXSDKINFORMATION_H_INCLUDED_

#include "NexSAL_Com.h"

typedef struct _SDKInformation_
{
	NXINT8						pProjectID[128];
	
	//reserved 
	NXINT8						pSDKName[128];

	NXINT8						PackageName[128];

	//reserved 
	NXINT8						AppUCode[128];
	
    NXUINT32					bCheckTimeLock;
	NXUINT32					bCheckMFGLock;
	NXUINT32					bCheckPKGLock;

	NXINT8						LockStart[64];
	NXINT8						LockEnd[64];

	// reserved 
	NXINT8 						LicenseKey[128];      

	NXUINT32					bCheckPKGname; //sylee 130520
    NXINT8						pPKGName[150][128]; 
    NXUINT32					bCheckWaterMark;	
} NEXSDKInformation;

const NXUINT8* NexSDKInfo_GetInfoPtr();
NXUINT32 NexSDKInfo_GetInfoSize();

#endif

