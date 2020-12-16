/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_EffectItemVec.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
case		2011/09/24	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_EffectItemVec.h"
#ifdef __APPLE__ // NESI-499
#include "NexThemeRenderer.h"
#endif

void CVideoEffectItemVec::lock(){

	m_Lock.Lock();
}

void CVideoEffectItemVec::unlock(){

	m_Lock.Unlock();
}

CVideoEffectItemVec::CVideoEffectItemVec()
{
	CAutoLock m(m_Lock);
	m_EffectVec.clear();
	m_pCurrentEffectItem	= NULL;
	m_iCurrentEffectIndex	= 0;	
	m_pCurrentTitleItem	= NULL;
	m_iCurrentTitleIndex	= 0;
}

CVideoEffectItemVec::~CVideoEffectItemVec()
{
	CAutoLock m(m_Lock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] ~~~~CVideoEffectItemVec In", __LINE__);
	clearEffectItem();
	m_iCurrentEffectIndex	= 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] ~~~~CVideoEffectItemVec Out", __LINE__);
}

int CVideoEffectItemVec::getCount()
{
//	CAutoLock m(m_Lock);
	return (int)m_EffectVec.size();
}

CVideoEffectItem* CVideoEffectItemVec::getEffetItem(int index)
{
	if(index > m_EffectVec.size())
		return NULL;
	return (CVideoEffectItem*)m_EffectVec[index];
}

NXBOOL CVideoEffectItemVec::addEffectItem(CVideoEffectItem* pItem)
{
	if( pItem == NULL ) return FALSE;

	for(int i = 0; i < m_EffectVec.size(); i++)
	{
		if( m_EffectVec[i]->isSameForTitle(pItem) )
			return TRUE;
	}

	SAFE_ADDREF(pItem);
	m_EffectVec.insert(m_EffectVec.end(), pItem);
	return TRUE;
}

NXBOOL CVideoEffectItemVec::clearEffectItem()
{
	for(int i = 0; i < m_EffectVec.size(); i++)
		m_EffectVec[i]->Release();
	
	SAFE_RELEASE(m_pCurrentEffectItem);
	m_iCurrentEffectIndex	= 0;
	
	SAFE_RELEASE(m_pCurrentTitleItem);
	m_iCurrentTitleIndex	= 0;
	
	m_EffectVec.clear();
	return TRUE;

}

NXBOOL CVideoEffectItemVec::clearEffectFlag()
{
	for(int i = 0; i < m_EffectVec.size(); i++)
		m_EffectVec[i]->clearApplyFlag();

	SAFE_RELEASE(m_pCurrentEffectItem);
	m_iCurrentEffectIndex	= 0;		

	SAFE_RELEASE(m_pCurrentTitleItem);
	m_iCurrentTitleIndex	= 0;	
	return TRUE;
}

CVideoEffectItem* CVideoEffectItemVec::getActiveEffect(unsigned int uiTime)
{
	for(int i = 0; i < m_EffectVec.size(); i++)
	{
		if( m_EffectVec[i]->isActiveEffectTime(uiTime) )
		{
			m_iCurrentEffectIndex = i;
			SAFE_ADDREF(m_EffectVec[i]);
			return m_EffectVec[i];
		}
	}
	return NULL;
}

CVideoEffectItem* CVideoEffectItemVec::getActiveTitle(unsigned int uiTime)
{
	for(int i = 0; i < m_EffectVec.size(); i++)
	{
		if( m_EffectVec[i]->isActiveTitleTime(uiTime) )
		{
			m_iCurrentTitleIndex = i;
			SAFE_ADDREF(m_EffectVec[i]);
			return m_EffectVec[i];
		}
	}
	return NULL;
}

CVideoEffectItem* CVideoEffectItemVec::getFirstEffect()
{
	if( m_EffectVec.size() <= 0)
	{
		return NULL;
	}
	m_iCurrentEffectIndex	= 0;	
	
	SAFE_ADDREF(m_EffectVec[m_iCurrentEffectIndex]);
	return m_EffectVec[m_iCurrentEffectIndex];
}

CVideoEffectItem* CVideoEffectItemVec::getNextEffect()
{
	if( (m_iCurrentEffectIndex + 1 ) >= m_EffectVec.size() )
	{
		return NULL;
	}

	m_iCurrentEffectIndex++;
	SAFE_ADDREF(m_EffectVec[m_iCurrentEffectIndex]);
	return m_EffectVec[m_iCurrentEffectIndex];
}

int CVideoEffectItemVec::applyEffectItem(void* pRender, unsigned int uiTime)
{
	if( m_pCurrentEffectItem == NULL )
	{
		m_pCurrentEffectItem = getActiveEffect(uiTime);
		if( m_pCurrentEffectItem == NULL )
		{
#ifdef __APPLE__ // NESI-499
			NXT_ThemeRenderer_ClearTransitionEffect((NXT_HThemeRenderer)pRender);
#endif
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Not exist Active effect", __LINE__);
			return 100;
		}
	}

	int iRet = m_pCurrentEffectItem->applyEffectItem(pRender, uiTime);
	if( iRet >= 100 )
	{
		SAFE_RELEASE(m_pCurrentEffectItem);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Effect apply end", __LINE__);
	}
	return iRet;
}

int CVideoEffectItemVec::applyTitleItem(void* pRender, unsigned int uiTime)
{
	if( m_pCurrentTitleItem == NULL )
	{
		m_pCurrentTitleItem = getActiveTitle(uiTime);
		if( m_pCurrentTitleItem == NULL )
		{
#ifdef __APPLE__ // NESI-499
			NXT_ThemeRenderer_ClearClipEffect((NXT_HThemeRenderer)pRender);
#endif
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Not exist Active effect", __LINE__);
			return 0;
		}
	}

	int iRet = m_pCurrentTitleItem->applyTitleItem(pRender, uiTime);
	if( iRet >= 100 )
	{
		SAFE_RELEASE(m_pCurrentTitleItem);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Title apply end", __LINE__);

		m_pCurrentTitleItem = getActiveTitle(uiTime);
		if( m_pCurrentTitleItem == NULL )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Not exist Active effect", __LINE__);
			return 0;
		}
		iRet = m_pCurrentTitleItem->applyTitleItem(pRender, uiTime);
		if( iRet >= 100 )
		{
			SAFE_RELEASE(m_pCurrentTitleItem);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Title apply end", __LINE__);
		}
	}
	return iRet;
}

void CVideoEffectItemVec::printEffectInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] -------------- Effect(%d) Time Info ------------------", __LINE__, m_EffectVec.size() );
	for(int i = 0; i < m_EffectVec.size(); i++)
	{
		m_EffectVec[i]->printEffect();

//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] StartTime : %d", __LINE__, m_EffectVec[i]->m_uiStartTime);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] EndTime : %d", __LINE__, m_EffectVec[i]->m_uiEndTime);
//
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] EffectStartTime : %d", __LINE__, m_EffectVec[i]->m_uiEffectStartTime);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] EffectEndTime : %d", __LINE__, m_EffectVec[i]->m_uiEffectEndTime);
//
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] TitleStartTime : %d", __LINE__, m_EffectVec[i]->m_uiTitleStartTime);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] TitleEndTime : %d", __LINE__, m_EffectVec[i]->m_uiTitleEndTime);
//
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Duration : %d", __LINE__, m_EffectVec[i]->m_uiEffectDuration);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Offset : %d", __LINE__, m_EffectVec[i]->m_iEffectOffset);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] Overlap : %d", __LINE__, m_EffectVec[i]->m_iEffectOverlap);
//		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] -------------------------------------------", __LINE__);		
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItemVec.cpp %d] ------------- Effect Time Info End -----------------", __LINE__);
}

