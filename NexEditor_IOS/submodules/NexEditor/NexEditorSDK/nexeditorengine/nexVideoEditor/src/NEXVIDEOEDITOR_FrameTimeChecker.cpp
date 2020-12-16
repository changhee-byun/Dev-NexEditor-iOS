/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FrameTimeChecker.cpp
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
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"

CFrameTime::CFrameTime()
{
	m_uiCTS	= 0;
	m_uiPTS	= 0;
	m_iMultipleFactor = 1;
}

CFrameTime::CFrameTime(unsigned int uiCTS, unsigned int uiPTS)
{
	m_uiCTS	= uiCTS;
	m_uiPTS	= uiPTS;
	m_iMultipleFactor = 1;
}

CFrameTime::CFrameTime(unsigned int uiCTS, unsigned int uiPTS, int iMultipleFactor)
{
	m_uiCTS	= uiCTS;
	m_uiPTS	= uiPTS;
	m_iMultipleFactor = iMultipleFactor;
}

NXBOOL CFrameTime::isValidTime(unsigned int uiTime)
{
	// if( m_uiCTS == uiTime || m_uiPTS == uiTime )
	if( m_uiPTS == uiTime )
		return TRUE;
	return FALSE;
}

unsigned int CFrameTime::getCTS()
{
	return m_uiCTS;
}

unsigned int CFrameTime::getPTS()
{
	return m_uiPTS;
}

unsigned int CFrameTime::getApplyMultipleCTS()
{
	if(m_iMultipleFactor == 1 || m_iMultipleFactor == 2)
		return m_uiCTS / m_iMultipleFactor;
	else
		return m_uiCTS*m_iMultipleFactor/100;	
}

unsigned int CFrameTime::getApplyMultiplePTS()
{
	if(m_iMultipleFactor == 1 || m_iMultipleFactor == 2)
		return m_uiPTS / m_iMultipleFactor;
	else
		return m_uiPTS*m_iMultipleFactor/100;	
}

unsigned int CFrameTime::getMultipleFactor()
{
	return m_iMultipleFactor;
}

CFrameTimeChecker::CFrameTimeChecker()
{
	clear();
}

CFrameTimeChecker::~CFrameTimeChecker()
{
	clear();
}

void	CFrameTimeChecker::addFrameCTSVec(unsigned int uiFrameCTS, unsigned int uiFramePTS)
{
	CFrameTime frameTime(uiFrameCTS, uiFramePTS);
	
	insert(end(), frameTime);
}

void	CFrameTimeChecker::addFrameCTSVec(unsigned int uiFrameCTS, unsigned int uiFramePTS, int iMultipleFactor)
{
	CFrameTime frameTime(uiFrameCTS, uiFramePTS, iMultipleFactor);
	
	insert(end(), frameTime);
}

NXBOOL CFrameTimeChecker::isValidFrameCTS(unsigned int uiCTS, NXBOOL bRemove)
{
	if( size() > 0 )
	{
		vecFrameTimeItr i;

		for( i = begin(); i != end(); i++)
		{
			CFrameTime frameTime = (CFrameTime&)*i;
			
			if( frameTime.isValidTime(uiCTS) )
			{
				if( bRemove)
					erase(i);
				return TRUE;
			}
		}
	}

	return FALSE;
	
}

NXBOOL CFrameTimeChecker::getValidFrameCTSWithApplyMutipleCTS(unsigned int uiCTS, unsigned int* pCTS, NXBOOL bRemove)
{
	if( pCTS == NULL )
		return FALSE;
	
	if( size() > 0 )
	{
		vecFrameTimeItr i;

		for( i = begin(); i != end(); i++)
		{
			CFrameTime frameTime = (CFrameTime&)*i;
			
			if( frameTime.isValidTime(uiCTS) )
			{
				*pCTS = frameTime.getApplyMultipleCTS();
				if( bRemove)
					erase(i);
				return TRUE;
			}
		}
	}
	*pCTS = 0;
	return FALSE;
	
}

NXBOOL CFrameTimeChecker::getValidFrameCTSWithApplyMutiplePTS(unsigned int uiCTS, unsigned int* pPTS, NXBOOL bRemove)
{
	if( pPTS == NULL )
		return FALSE;
	
	if( size() > 0 )
	{
		vecFrameTimeItr i;

		for( i = begin(); i != end(); i++)
		{
			CFrameTime frameTime = (CFrameTime&)*i;
			
			if( frameTime.isValidTime(uiCTS) )
			{
				*pPTS = frameTime.getApplyMultiplePTS();
				if( bRemove)
					erase(i);
				return TRUE;
			}
		}
	}
	*pPTS = 0;
	return FALSE;
	
}

void CFrameTimeChecker::removeFrameCTSVec(unsigned int uiCTS)
{
	if( size() > 0 )
	{
		vecFrameTimeItr i;
		for( i = begin(); i != end(); i++)
		{
			CFrameTime frameTime = (CFrameTime&)*i;
			if( frameTime.isValidTime(uiCTS) )
			{
				erase(i);
			}
		}
	}
}

void CFrameTimeChecker::removeSmallerFrameTime(unsigned int uiTime)
{
	while(size() > 0)
	{
		CFrameTime frameTime = (CFrameTime&)*(begin());
		if( uiTime >= frameTime.getPTS())
		{
			erase(begin());
			continue;
		}
		break;
	}
	return;
}

void CFrameTimeChecker::removeSmallTime(unsigned int uiTime)
{
	while(size() > 0)
	{
		CFrameTime frameTime = (CFrameTime&)*(begin());
		if( uiTime > frameTime.getPTS())
		{
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CFrameTimeChecker.cpp %d] removeSmallTime %d, %d, %d", __LINE__, frameTime.getCTS(), frameTime.getPTS(), uiTime);
			erase(begin());
			continue;
		}
		break;
	}
	return;
}

unsigned int CFrameTimeChecker::getFirstCTSInVec()	
{
	CFrameTime frameTime = (CFrameTime&)*(begin());
	return frameTime.getCTS();
}
