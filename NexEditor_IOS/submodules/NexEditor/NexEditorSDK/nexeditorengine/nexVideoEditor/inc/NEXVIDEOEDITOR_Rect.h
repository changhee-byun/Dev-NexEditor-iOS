/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Rect.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_RECT__H_
#define _NEXVIDEOEDITOR_RECT__H_
//---------------------------------------------------------------------------

#include "NEXVIDEOEDITOR_Interface.h"
#include "NexSAL_Internal.h"

typedef enum _RAND_RATIO
{
	RAND_RATIO_16_9 = 0,
	RAND_RATIO_5_3,
	RAND_RATIO_4_3,
}RAND_RATIO;


//---------------------------------------------------------------------------
class CRectangle :public CNxRef<IRectangle>
{
public:
	CRectangle();
	CRectangle(int iLeft, int iTop, int iRight, int iBottom);
	virtual ~CRectangle();
	virtual int getLeft();
	virtual int getTop();
	virtual int getBottom();
	virtual int getRight();
	int getWidth();
	int getHeight();	
	virtual int setRect(int iLeft = 0, int iTop = 0, int iRight = 0, int iBottom = 0);

	void randRect(RAND_RATIO ratio);

private:
	int m_iLeft;
	int m_iTop;
	int m_iRight;
	int m_iBottom;
};

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_RECT__H_
