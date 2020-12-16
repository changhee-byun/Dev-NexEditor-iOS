/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_HighlightDiffChecker.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/06/06	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_HIGHLIGHTDIFFCHECKER_H__
#define __NEXVIDEOEDITOR_HIGHLIGHTDIFFCHECKER_H__

#include "NexSAL_Internal.h"
#include <vector>

class CHighlightDiff
{
public:
	CHighlightDiff();
	CHighlightDiff(unsigned int uiTime, unsigned int uiDiff);

	unsigned int getTime();
	unsigned int getDiff();

	 bool operator() (int i,int j) {return (i<j);}

private:
	unsigned int m_uiTime;
	unsigned int m_uiDiff;
};

typedef std::vector<CHighlightDiff> vecHighlightDiff;
typedef vecHighlightDiff::iterator vecHighlightDiffItr;

typedef std::vector<unsigned int> vecHighlightTime;
typedef vecHighlightTime::iterator vecHighlightTimeItr;



class CHighlightDiffChecker : public vecHighlightDiff
{
public:
	CHighlightDiffChecker();
	~CHighlightDiffChecker();

	void	addHighlightDiff(unsigned int uiTime, unsigned int uiDiff);
	void sortWithTime();
	void sortWithDiff();

	void calcHighlight(int iRequestInterval, int iRequestCount, int iEndTime);
	int getHighlightTimeCount();
	unsigned int getHighlightTime(int Index);

	void printDiff();

	vecHighlightTime m_vecHighlightTime;
};

#endif // __NEXVIDEOEDITOR_HIGHLIGHTDIFFCHECKER_H__

