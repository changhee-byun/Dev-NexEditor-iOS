/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Rect.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_Rect.h"
#include "NexSAL_Internal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


static int g_i16_9RatioRect[][4] = {	{0, 0, 320, 180},
							{0, 0, 640, 360},
							{0, 0, 640, 360},
							{0, 0, 1280, 720},
							{0, 0, 1920, 1080} };
						
static int g_i5_3RatioRect[][4] = {	{0, 0, 200, 120},
								{0, 0, 400, 240},
								{0, 0, 800, 480},
								{0, 0, 1000, 600},	
								{0, 0, 1600, 960}};

static int g_i4_3RatioRect[][4] = {	{0, 0, 160, 120},
								{0, 0, 320, 240},
								{0, 0, 640, 480},
								{0, 0, 800, 600},	
								{0, 0, 1600, 1200}};
						
//---------------------------------------------------------------------------
CRectangle::CRectangle()
{
	m_iLeft		= 0;
	m_iTop		= 0;
	m_iRight		= 0;
	m_iBottom	= 0;
}

CRectangle::CRectangle(int iLeft, int iTop, int iRight, int iBottom)
{
	m_iLeft		= iLeft;
	m_iTop		= iTop;
	m_iRight		= iRight;
	m_iBottom	= iBottom;
}

CRectangle::~CRectangle()
{
}

int CRectangle::getLeft()
{
	return m_iLeft;
}

int CRectangle::getTop()
{
	return m_iTop;
}

int CRectangle::getRight()
{
	return m_iRight;
}

int CRectangle::getBottom()
{
	return m_iBottom;
}

int CRectangle::getWidth()
{
	return m_iRight - m_iLeft;
}

int CRectangle::getHeight()
{
	return m_iBottom - m_iTop;
}

int CRectangle::setRect(int iLeft, int iTop, int iRight, int iBottom)
{
	m_iLeft		= iLeft;
	m_iTop		= iTop;
	m_iRight	= iRight;
	m_iBottom	= iBottom;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CRectangle::randRect(RAND_RATIO ratio)
{
	if( getWidth() <= 32 || getHeight() <= 32 )
		return;

	int iSizeIndex = 0;
	int		iWidthGap	= 0;
	int		iHeightGap	= 0;
	
	NXBOOL	bRatio		= FALSE;
	int		aRect[5][4];

	switch(ratio)
	{
		case RAND_RATIO_5_3:
			memcpy(aRect, g_i5_3RatioRect, sizeof(g_i5_3RatioRect));
			break;
		case RAND_RATIO_4_3:
			memcpy(aRect, g_i4_3RatioRect, sizeof(g_i4_3RatioRect));
			break;
		case RAND_RATIO_16_9:
		default:
			memcpy(aRect, g_i16_9RatioRect, sizeof(g_i16_9RatioRect));
 			break;
	};

	for( iSizeIndex = 0; iSizeIndex < 5; iSizeIndex++)
	{
		if( aRect[iSizeIndex][2] > getWidth() || aRect[iSizeIndex][3] > getHeight() )
	{
			if( iSizeIndex == 0 )
				return;
			iSizeIndex--;
			break;
		}
	}

	if( iSizeIndex == 5 )
		iSizeIndex--;

	srand(rand());

	iWidthGap = getWidth() - aRect[iSizeIndex][2];
	iHeightGap = getHeight() - aRect[iSizeIndex][3];

	if( iWidthGap != 0 )
	{
		iWidthGap = rand() % iWidthGap;
	}

	if( iHeightGap != 0 )
	{
		iHeightGap = rand() % iHeightGap;
	}

	m_iLeft		= aRect[iSizeIndex][0] + iWidthGap;
	m_iRight		= aRect[iSizeIndex][2] + iWidthGap;
	m_iTop		= aRect[iSizeIndex][1] + iHeightGap;
	m_iBottom	= aRect[iSizeIndex][3] + iHeightGap;

	bRatio = (getWidth()  * 9) == (getHeight() *16) ? TRUE : FALSE;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Rect.cpp %d] randRect(%d,%d,%d,%d) 16:9Ratio is %s", __LINE__, m_iLeft, m_iTop, m_iRight, m_iBottom, bRatio ? "TRUE" : "FALSE");
}

