/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FrameYUVInfo.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
salabara		2019/03/22	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_FrameYUVInfo.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include <algorithm>

CYUVInfo::CYUVInfo()
{
	m_uiTime = 0;
	m_pBuffer = NULL;
}

CYUVInfo::CYUVInfo(unsigned int uiTime, unsigned char* pBuffer)
{
	m_uiTime = uiTime;
	m_pBuffer = pBuffer;
}

CYUVInfo::CYUVInfo(unsigned int uiTime)
{
	m_uiTime = uiTime;
	m_pBuffer = NULL;
}

unsigned int CYUVInfo::getTime()
{
	return m_uiTime;
}

unsigned char* CYUVInfo::getBuffer()
{
	return m_pBuffer;
}

CFrameYUVInfo::CFrameYUVInfo()
{
	clear();
}

CFrameYUVInfo::~CFrameYUVInfo()
{
	removeAll();
	clear();
}

unsigned int CFrameYUVInfo::getSize()
{
	return size();
}

NXBOOL CFrameYUVInfo::addTime(unsigned int uiTime)
{
	CYUVInfo info(uiTime);
	insert(end(), info);	
//	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] addTime (%d)", __LINE__, uiTime);

	return TRUE;
}
    
NXBOOL CFrameYUVInfo::addYUVData(unsigned int uiTime, unsigned char* pData)
{
	CYUVInfo info(uiTime, pData);
	insert(end(), info);	

	return TRUE;
}

NXBOOL CFrameYUVInfo::isExistTime(unsigned int uiTime)
{
	vecYUVInfoItr i = begin();
	for(; i != end(); i++)
	{
		CYUVInfo info = (CYUVInfo&)*i;
       	if(info.getTime() == uiTime)
       	{
			return TRUE;
       	}
	}
	return FALSE;
}


unsigned int CFrameYUVInfo::getFirstTime()
{
	vecYUVInfoItr i = begin();

	if(size())
	{
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getFirstTime (%d)", __LINE__, info.getTime());
		return info.getTime();
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getFirstTime there is not available data", __LINE__);
	return 0;
}

unsigned int CFrameYUVInfo::getLastTime()
{
	vecYUVInfoItr i = end();

	if(size())
	{
		i--;
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getLastTime (%d)", __LINE__, info.getTime());
		return info.getTime();
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getLastTime there is not available data", __LINE__);
	return 0;
}

unsigned char* CFrameYUVInfo::getFirstData()
{
	vecYUVInfoItr i = begin();

	if(size())
	{
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getFirstData (0x%x)", __LINE__, info.getBuffer());
		return info.getBuffer();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getFirstData there is not available data %d", __LINE__);
	return NULL;
}

unsigned char* CFrameYUVInfo::getLastData()
{
	vecYUVInfoItr i = end();

	if(size())
	{
		i--;
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getLastData (%d, 0x%x)", __LINE__, info.getTime(), info.getBuffer());
		return info.getBuffer();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] getFirstData there is not available data %d", __LINE__);
	return NULL;
}

NXBOOL CFrameYUVInfo::removeFirst()
{
	vecYUVInfoItr i = begin();
	if(size())
	{
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] removeFirst delete %d", __LINE__, info.getTime() );
       	if(info.getBuffer() != NULL)
       	{
			nexSAL_MemFree(info.getBuffer());
       	}
		erase(i);
	}
	return TRUE;
}

NXBOOL CFrameYUVInfo::removeLast()
{
	vecYUVInfoItr i = end();
	if(size())
	{
		i--;
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] removeFirst delete %d", __LINE__, info.getTime() );
       	if(info.getBuffer() != NULL)
       	{
			nexSAL_MemFree(info.getBuffer());
       	}
		erase(i);
	}
	return TRUE;
}

NXBOOL CFrameYUVInfo::removeAll()
{
	vecYUVInfoItr i = begin();
	for(; i != end(); i++)
	{
		CYUVInfo info = (CYUVInfo&)*i;
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] removeAll delete %d", __LINE__, info.getTime() );
       	if(info.getBuffer() != NULL)
       	{
			nexSAL_MemFree(info.getBuffer());
       	}
		erase(i);
	}
	return TRUE;
}


bool cmp_Time(CYUVInfo a, CYUVInfo b) { return a.getTime() < b.getTime();; }
void CFrameYUVInfo::sortWithTime()
{
	std::sort(begin(), end(), cmp_Time);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] sortWithTime", __LINE__ );
}

void CFrameYUVInfo::printTime()
{
	vecYUVInfoItr i;

	for( i = begin(); i != end(); i++)
	{
		CYUVInfo info = (CYUVInfo&)*i;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameYUVInfo.cpp %d] printTime(%.8d), (0x%x) ", __LINE__, info.getTime(), info.getBuffer());
	}
}

