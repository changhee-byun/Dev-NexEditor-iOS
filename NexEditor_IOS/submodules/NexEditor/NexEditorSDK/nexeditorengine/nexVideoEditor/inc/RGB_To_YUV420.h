/******************************************************************************
* File Name   :	RGB_To_YUV420.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/11/23	Draft.
-----------------------------------------------------------------------------*/

#ifndef __RGB_TO_YUV420_H__
#define __RGB_TO_YUV420_H__

int NXCC_RGBToYUV420(unsigned char* pDstYUV, const unsigned char* pSrcRGB, int iWidth, int iHeight, int iByteCount);
int NXCC_RGBToYUV420Semi(unsigned char* pDstYUV, const unsigned char* pSrcRGB, int iWidth, int iHeight, int iByteCount);

#endif // __RGB_TO_YUV420_H__