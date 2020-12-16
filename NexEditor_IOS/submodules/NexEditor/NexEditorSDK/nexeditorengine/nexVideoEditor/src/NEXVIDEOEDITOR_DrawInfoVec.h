/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DrawInfoVec.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
case		2017/05/15	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_DRAWINFOVEC_H__
#define __NEXVIDEOEDITOR_DRAWINFOVEC_H__

#include "NEXVIDEOEDITOR_DrawInfo.h"

class CNexDrawInfoVec: public CNxRef<IDrawInfoList>
{
public:
	CNexDrawInfoVec();
	virtual ~CNexDrawInfoVec();

	virtual IDrawInfo* createDrawInfo();
	virtual void addDrawInfo(IDrawInfo* info);
	virtual void clearDrawInfo();

	int sizeDrawInfos();

	void applyDrawInfo(void* pRender, unsigned int uiTime);
	void applyDrawInfo(void* pRender, unsigned int uiTime, int textureID);
	void resetDrawInfos(void* pRender);

	CNexDrawInfoVec* getDrawInfoList(unsigned int trackID);
	void updateDrawInfo();
	void updateDrawInfo(CNexDrawInfo* pInfo);
	CNexDrawInfo* getDrawInfo(int id);
	NXBOOL isDrawTime(unsigned int uiTime);
	NXBOOL checkFaceDetectDoneAtTime(unsigned int uiTime);

	void printDrawInfo();

	CDrawInfoVec mDrawVec;
};

#endif // __NEXVIDEOEDITOR_DRAWINFOVEC_H__
