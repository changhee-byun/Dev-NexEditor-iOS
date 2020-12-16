/******************************************************************************
* File Name   :	nexProtection.h
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

#ifndef _NEXPROTECTION_H_
#define _NEXPROTECTION_H_

#include "NexSDKInfo.h"
#include "NexSecureWare.h"
#include "NexBase64.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define USING_NEXSECURE

bool checkSDKProtection();
void setPackageName4Protection(const char* packageName);

NXINT32 getSDKInfo(NEXSDKInformation* pSDKInfo);

#ifdef __cplusplus
}
#endif
	
#endif
