/******************************************************************************
* File Name   :	RGB_To_YUV420.cpp
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

#include "NexSAL_Internal.h"
#include "RGB_To_YUV420.h"

int NXCC_RGBToYUV420(unsigned char* pDstYUV, const unsigned char* pSrcRGB, int iWidth, int iHeight, int iByteCount)
{
	if( pDstYUV == NULL || pSrcRGB == NULL )
		return FALSE;
	
	int j, i; 
	int idx; 
	int sum[2]; 
	int t_width = iWidth;

	int alpha = 1;
	
	if( iByteCount == 3 )
		alpha = 1;
	else if( iByteCount == 4 )
		alpha = 2;
	else
	{
		return FALSE;
	}


	int r[2], g[2], b[2]; 
	unsigned char* pos; 
	unsigned char* line; 

	unsigned char* pY = pDstYUV;
	unsigned char* pU = pY + iWidth*iHeight;
	unsigned char* pV = pU + iWidth/2*iHeight/2;

	idx = 0; line = (unsigned char*)pSrcRGB; 
	for( j = 0 ; j < iHeight ; j++ )
	{
		pos = line; 
		for( i = 0 ; i < iWidth ; i++ )
		{
			b[0] = *pos; pos++; 
			g[0] = *pos; pos++;
			r[0] = *pos; pos+= alpha; 
			pY[idx] = ((  66 * r[0] + 129 * g[0] +  25 * b[0] + 128) >> 8) + 16; 
			idx++; 
		} 
		line += t_width*iByteCount; 
	} 

	idx = 0; line = (unsigned char*)pSrcRGB; 

	for( j = 0 ; j < iHeight ; j+=2 )
	{ 
		pos = line; 
		for( i = 0 ; i < iWidth ; i+=2 )
		{ 
			b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos++; 
			g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++; 
			r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos+= alpha; 

			sum[0]  = (( -38 * r[0] -  74 * g[0] + 112 * b[0] + 128) >> 8) + 128; 
			sum[1]  = (( 112 * r[0] -  94 * g[0] -  18 * b[0] + 128) >> 8) + 128; 

			sum[0] += (( -38 * r[1] -  74 * g[1] + 112 * b[1] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[1] -  94 * g[1] -  18 * b[1] + 128) >> 8) + 128; 

			b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos++; 
			g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++; 
			r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos+= alpha; 

			sum[0] += (( -38 * r[0] -  74 * g[0] + 112 * b[0] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[0] -  94 * g[0] -  18 * b[0] + 128) >> 8) + 128; 

			sum[0] += (( -38 * r[1] -  74 * g[1] + 112 * b[1] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[1] -  94 * g[1] -  18 * b[1] + 128) >> 8) + 128; 

			pU[idx] = sum[0]/4; 
			pV[idx] = sum[1]/4; 

			idx++; 
		} 
		line += 2*t_width*iByteCount; 
	} 

	return 1;
}

int NXCC_RGBToYUV420Semi(unsigned char* pDstYUV, const unsigned char* pSrcRGB, int iWidth, int iHeight, int iByteCount)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[RGB2TUV420.cpp %d] RGB2YUV420Semi(%p %p %d %d %d)",  __LINE__, pDstYUV, pSrcRGB, iWidth, iHeight, iByteCount);
	
	if( pDstYUV == NULL || pSrcRGB == NULL )
		return FALSE;
	
	int j, i; 
	int idx; 
	int sum[2]; 
	int t_width = iWidth;

	int alpha = 1;
	
	if( iByteCount == 3 )
		alpha = 1;
	else if( iByteCount == 4 )
		alpha = 2;
	else
	{
		return FALSE;
	}

	int r[2], g[2], b[2]; 
	unsigned char* pos; 
	unsigned char* line; 

	unsigned char* pY = pDstYUV;
	unsigned char* pUV = pY + iWidth*iHeight;

	idx = 0; line = (unsigned char*)pSrcRGB; 
	for( j = 0 ; j < iHeight ; j++ )
	{
		pos = line; 
		for( i = 0 ; i < iWidth ; i++ )
		{
#if defined(_ANDROID)
			b[0] = *pos; pos++; 
			g[0] = *pos; pos++;
			r[0] = *pos; pos+= alpha; 
#elif defined(__APPLE__)
            r[0] = *pos; pos++;
            g[0] = *pos; pos++;
            b[0] = *pos; pos+= alpha;
#endif
			pY[idx] = ((  66 * r[0] + 129 * g[0] +  25 * b[0] + 128) >> 8) + 16; 
			idx++; 
		} 
		line += t_width*iByteCount; 
	} 

	idx = 0; line = (unsigned char*)pSrcRGB; 

	for( j = 0 ; j < iHeight ; j+=2 )
	{ 
		pos = line; 
		for( i = 0 ; i < iWidth ; i+=2 )
		{ 
#if defined(_ANDROID)
			b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos++; 
			g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++; 
			r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos+= alpha; 
#elif defined(__APPLE__)
            r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos++;
            g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++;
            b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos+= alpha;
#endif
			sum[0]  = (( -38 * r[0] -  74 * g[0] + 112 * b[0] + 128) >> 8) + 128; 
			sum[1]  = (( 112 * r[0] -  94 * g[0] -  18 * b[0] + 128) >> 8) + 128; 

			sum[0] += (( -38 * r[1] -  74 * g[1] + 112 * b[1] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[1] -  94 * g[1] -  18 * b[1] + 128) >> 8) + 128; 

#if defined(_ANDROID)
			b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos++; 
			g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++; 
			r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos+= alpha; 
#elif defined(__APPLE__)
            r[0] = *pos; r[1] = *(pos+t_width*iByteCount); pos++;
            g[0] = *pos; g[1] = *(pos+t_width*iByteCount); pos++;
            b[0] = *pos; b[1] = *(pos+t_width*iByteCount); pos+= alpha;
#endif
			sum[0] += (( -38 * r[0] -  74 * g[0] + 112 * b[0] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[0] -  94 * g[0] -  18 * b[0] + 128) >> 8) + 128; 

			sum[0] += (( -38 * r[1] -  74 * g[1] + 112 * b[1] + 128) >> 8) + 128; 
			sum[1] += (( 112 * r[1] -  94 * g[1] -  18 * b[1] + 128) >> 8) + 128; 

			pUV[idx] = sum[0]/4; 
			pUV[idx+1] = sum[1]/4; 

			idx +=2; 
		} 
		line += 2*t_width*iByteCount; 
	} 

	return 1;
}

