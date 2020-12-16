/******************************************************************************
* File Name   :	nexWrapColorProcesser_api.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2013/09/23	Draft.
-----------------------------------------------------------------------------*/


#ifndef __NEXWRAPCOLORPROCESSER_API__
#define __NEXWRAPCOLORPROCESSER_API__

extern "C"
{

bool copyNV12TiledToNV12(int iWidth, int iHeight, int iStride, int iSliceHeight, unsigned char* pSrc, unsigned char* pDst);
bool copyYUV420ToNV12(int iWidth, int iHeight, int iStride, int iSliceHeight, unsigned char* pSrc, unsigned char* pDst);

}


#endif // __NEXWRAPCOLORPROCESSER_API__
