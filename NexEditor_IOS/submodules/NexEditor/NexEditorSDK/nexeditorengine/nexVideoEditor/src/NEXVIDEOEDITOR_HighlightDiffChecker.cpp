/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_HighlightDiffChecker.cpp
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
#include "NEXVIDEOEDITOR_HighlightDiffChecker.h"
#include <algorithm>

CHighlightDiff::CHighlightDiff()
{
	m_uiTime = 0;
	m_uiDiff = 0;
}

CHighlightDiff::CHighlightDiff(unsigned int uiTime, unsigned int uiDiff)
{
	m_uiTime = uiTime;
	m_uiDiff = uiDiff;
}

unsigned int CHighlightDiff::getTime()
{
	return m_uiTime;
}

unsigned int CHighlightDiff::getDiff()
{
	return m_uiDiff;
}

CHighlightDiffChecker::CHighlightDiffChecker()
{
	m_vecHighlightTime.clear();
	clear();
}

CHighlightDiffChecker::~CHighlightDiffChecker()
{
	m_vecHighlightTime.clear();
	clear();
}

void	CHighlightDiffChecker::addHighlightDiff(unsigned int uiTime, unsigned int uiDiff)
{
	CHighlightDiff diff(uiTime, uiDiff);
	insert(end(), diff);
}

bool cmp_Time(CHighlightDiff a, CHighlightDiff b) { return a.getTime() < b.getTime();; }
bool cmp_Diff(CHighlightDiff a, CHighlightDiff b) { return a.getDiff() > b.getDiff(); }
bool cmp_DiffestTime(unsigned int a, unsigned int b) { return a < b; }

void CHighlightDiffChecker::sortWithTime()
{
	std::sort(begin(), end(), cmp_Time);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] sortWithTime", __LINE__ );
}

void CHighlightDiffChecker::sortWithDiff()
{
	std::sort(begin(), end(), cmp_Diff);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] sortWithDiff", __LINE__ );
}

void CHighlightDiffChecker::calcHighlight(int iRequestInterval, int iRequestCount, int iEndTime)
{
	CHighlightDiff diff;
	m_vecHighlightTime.clear();

	sortWithDiff();
	vecHighlightDiffItr i = begin();

	for(; i != end(); i++)
	{
		diff = (CHighlightDiff&)*i;

		if(diff.getTime() + iRequestInterval < iEndTime)
		{
			m_vecHighlightTime.insert(m_vecHighlightTime.end(), diff.getTime());
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calcHighlight add Time(%.8d) Diff(%.8d)", __LINE__, diff.getTime(), diff.getDiff());
			i++;
			break;
		}
	}

	if(iRequestCount > 1)
	{
		for(; i != end(); i++)
		{
			int count = 0;
			unsigned int diffestTime;
			diff = (CHighlightDiff&)*i;

			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calcHighlight diff Time(%.8d) Diff(%.8d)", __LINE__, diff.getTime(), diff.getDiff());			

			vecHighlightTimeItr j;
			for( j=m_vecHighlightTime.begin(); j != m_vecHighlightTime.end(); j++)		
			{
				diffestTime = (unsigned int&)*j;

				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calcHighlight diffest Time(%.8d) Diff(%.8d)", __LINE__, diffest.getTime(), diffest.getDiff());			

				if(diff.getTime() > diffestTime)
				{
					if(diff.getTime() > iRequestInterval && diff.getTime() - diffestTime >= iRequestInterval && diff.getTime() + iRequestInterval < iEndTime)
						count++;
				}
				else if(diffestTime > iRequestInterval && diffestTime >diff.getTime())
				{
					if(diffestTime - diff.getTime() >= iRequestInterval && diff.getTime() + iRequestInterval < iEndTime)
						count++;
				}
			}

			if(m_vecHighlightTime.size() == count)
			{
				m_vecHighlightTime.insert(m_vecHighlightTime.end(), diff.getTime());
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calcHighlight add Time(%.8d) Diff(%.8d)", __LINE__, diff.getTime(), diff.getDiff());			
			}

			if(m_vecHighlightTime.size() == iRequestCount)
			{
				break;
			}
		}
	}

	std::sort(m_vecHighlightTime.begin(), m_vecHighlightTime.end(), cmp_DiffestTime);
	
	vecHighlightTimeItr t;
	for( t = m_vecHighlightTime.begin(); t != m_vecHighlightTime.end(); t++)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calc sorted Time(%d)", __LINE__, *t);
	}
	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] calcHighlight m_vecHighlightTime.size() %zu", __LINE__, m_vecHighlightTime.size());			
}

int CHighlightDiffChecker::getHighlightTimeCount()
{
	return m_vecHighlightTime.size();
}

unsigned int CHighlightDiffChecker::getHighlightTime(int iIndex)
{
	if( iIndex >= m_vecHighlightTime.size()  )
		return 0xFFFFFFFF;

	return m_vecHighlightTime[iIndex];
}

void CHighlightDiffChecker::printDiff()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] ", __LINE__ );

	vecHighlightDiffItr i;

	for( i = begin(); i != end(); i++)
	{
		CHighlightDiff diff = (CHighlightDiff&)*i;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] Time(%.8d) Diff(%.8d)", __LINE__, diff.getTime(), diff.getDiff());
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[HighlightDiffChecker.cpp %d] ", __LINE__ );
}

