/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Util.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
ryu		2011/06/21	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_UTIL_H__
#define __NEXVIDEOEDITOR_UTIL_H__

#include "NEXVIDEOEDITOR_Types.h"

int getAACProfile(unsigned char* pBuf, int iSize, int* piAACProfile);
IMAGE_TYPE		getImageType(char* pFile);
int findString(unsigned char *src_str, int src_size ,unsigned char *find_str , int find_size );

#endif // __NEXVIDEOEDITOR_UTIL_H__
