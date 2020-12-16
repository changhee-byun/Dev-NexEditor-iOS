/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FrameTimeChecker.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2013/10/08	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_FRAMETIMECHECKER_H__
#define __NEXVIDEOEDITOR_FRAMETIMECHECKER_H__

#include "NexSAL_Internal.h"
#include <vector>

class CFrameTime
{
public:
	CFrameTime();
	CFrameTime(unsigned int uiCTS, unsigned int uiPTS);
	CFrameTime(unsigned int uiCTS, unsigned int uiPTS, int iMultipleFactor);
	NXBOOL isValidTime(unsigned int uiTime);
	~CFrameTime(){};

	unsigned int getCTS();
	unsigned int getPTS();
	unsigned int getApplyMultipleCTS();
	unsigned int getApplyMultiplePTS();
	unsigned int getMultipleFactor();

private:
	unsigned int m_uiCTS;
	unsigned int m_uiPTS;
	int m_iMultipleFactor;
};

typedef std::vector<CFrameTime> vecFrameTime;
typedef vecFrameTime::iterator vecFrameTimeItr;

class CFrameTimeChecker : public vecFrameTime
{
public:
	CFrameTimeChecker();
	~CFrameTimeChecker();

	void	addFrameCTSVec(unsigned int uiFrameCTS, unsigned int uiFramePTS);
	void	addFrameCTSVec(unsigned int uiFrameCTS, unsigned int uiFramePTS, int iMultipleFactor);
	NXBOOL isValidFrameCTS(unsigned int uiCTS, NXBOOL bRemove);

	NXBOOL getValidFrameCTSWithApplyMutipleCTS(unsigned int uiCTS, unsigned int* pCTS, NXBOOL bRemove);
	NXBOOL getValidFrameCTSWithApplyMutiplePTS(unsigned int uiCTS, unsigned int* pPTS, NXBOOL bRemove);
	
	void removeFrameCTSVec(unsigned int uiCTS);

	void removeSmallerFrameTime(unsigned int uiTime);
	void removeSmallTime(unsigned int uiTime);
	unsigned int getFirstCTSInVec();
		
};

#endif // __NEXVIDEOEDITOR_FRAMETIMECHECKER_H__
