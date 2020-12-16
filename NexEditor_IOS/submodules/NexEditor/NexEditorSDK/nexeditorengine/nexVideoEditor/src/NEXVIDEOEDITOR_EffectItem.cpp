/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_EffectItem.cpp
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

#include "NEXVIDEOEDITOR_EffectItem.h"
#include "NexThemeRenderer.h"

CVideoEffectItem::CVideoEffectItem()
{
	m_uiStartTime			= 0;
	m_uiEndTime				= 0;
	
	m_uiEffectStartTime		= 0;
	m_uiEffectEndTime		= 0;
	m_uiEffectDuration		= 0;
	m_iEffectOffset			= 0;
	m_iEffectOverlap		= 0;

	m_pEffectID				= NULL;

	m_bEffectStart			= FALSE;
	m_bEffectApplyEnd		= FALSE;
	
	m_uiTitleStartTime		= 0;
	m_uiTitleEndTime		= 0;
	
	m_pTitle				= NULL;
	m_pTitleEffectID		= NULL;
	
	m_bTitleEffectStart		= FALSE;
	m_bTitleEffectApplyEnd	= FALSE;

	m_pFilterEffect			= NULL;

	m_iCurrentIdx			= 0;
	m_iTotalCount			= 0;

	m_eEffectType			= EFFECT_TYPE_UNKNOWN;
}

CVideoEffectItem::~CVideoEffectItem()
{
	clearEffectItemInfo();
}

NXBOOL CVideoEffectItem::isTransitionEffect()
{
	return m_eEffectType == EFFECT_TYPE_TRANSITION;
}

NXBOOL CVideoEffectItem::isTitleEffect()
{
	return m_eEffectType == EFFECT_TYPE_TITLE;
}

NXBOOL CVideoEffectItem::clearApplyFlag()
{
	m_bEffectStart		= FALSE;	
	m_bEffectApplyEnd		= FALSE;
	
	m_bTitleEffectStart			= FALSE;
	m_bTitleEffectApplyEnd		= FALSE;
	return TRUE;
}

NXBOOL CVideoEffectItem::clearEffectItemInfo()
{
	m_uiStartTime		= 0;
	m_uiEndTime			= 0;
	
	m_uiEffectStartTime	= 0;
	m_uiEffectEndTime		= 0;
	m_uiEffectDuration		= 0;
	m_iEffectOffset		= 0;
	m_iEffectOverlap		= 0;
	if( m_pEffectID != NULL )
	{
		nexSAL_MemFree(m_pEffectID);
		m_pEffectID = NULL;
	}

	m_bEffectStart		= FALSE;	
	m_bEffectApplyEnd		= FALSE;

	m_uiTitleStartTime = 0;
	m_uiTitleEndTime = 0;

	if( m_pTitle != NULL )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle = NULL;
	}
	if( m_pTitleEffectID != NULL )
	{
		nexSAL_MemFree(m_pTitleEffectID);
		m_pTitleEffectID = NULL;
	}	
	
	m_bTitleEffectStart			= FALSE;
	m_bTitleEffectApplyEnd		= FALSE;

	if( m_pFilterEffect != NULL )
	{
		nexSAL_MemFree(m_pFilterEffect);
		m_pFilterEffect = NULL;
	}		

	m_pFilterEffect			= NULL;

	m_iCurrentIdx			= 0;
	m_iTotalCount				= 0;

	m_eEffectType			= EFFECT_TYPE_UNKNOWN;
	return TRUE;
}

NXBOOL CVideoEffectItem::isActiveEffectTime(unsigned int uiTime)
{
	if(m_eEffectType != EFFECT_TYPE_TRANSITION)
		return FALSE;

	if( uiTime >= m_uiEffectStartTime && uiTime < m_uiEffectEndTime )
		return TRUE;
	return FALSE;
}

NXBOOL CVideoEffectItem::isActiveTitleTime(unsigned int uiTime)
{
	if(m_eEffectType != EFFECT_TYPE_TITLE)
		return FALSE;

	if( uiTime >= m_uiTitleStartTime && uiTime < m_uiTitleEndTime )
		return TRUE;
	return FALSE;
}

NXBOOL CVideoEffectItem::isActiveBGMTime(unsigned int uiTime)
{
	if(m_eEffectType != EFFECT_TYPE_TRANSITION)
		return FALSE;

	if( uiTime >= m_uiStartTime && uiTime < m_uiEndTime - (m_uiEffectDuration*m_iEffectOverlap/100) )
		return TRUE;
	return FALSE;
}

NXBOOL CVideoEffectItem::isSameForTitle(CVideoEffectItem* pItem)
{
	if( this->m_eEffectType != pItem->m_eEffectType )
		return FALSE;

	if( this->m_uiStartTime != pItem->m_uiStartTime)
		return FALSE;

	if( this->m_uiEndTime != pItem->m_uiEndTime)
		return FALSE;

	if( this->m_uiTitleStartTime != pItem->m_uiTitleStartTime)
		return FALSE;

	if( this->m_uiTitleEndTime != pItem->m_uiTitleEndTime)
		return FALSE;

	if( this->m_pTitle == NULL || pItem->m_pTitle == NULL )
		return FALSE;

	if( strcmp(this->m_pTitle, pItem->m_pTitle) != 0 )
		return FALSE;

	if( this->m_pTitleEffectID == NULL || pItem->m_pTitleEffectID == NULL )
		return FALSE;

	if( strcmp(this->m_pTitleEffectID, pItem->m_pTitleEffectID) != 0 )
		return FALSE;

	return TRUE;
}

const char* CVideoEffectItem::getClipEffectID()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getClipEffectID(%s)", __LINE__, m_uiClipID, m_strEffectTitleID);
	return m_pEffectID;
}

const char* CVideoEffectItem::getTitleEffectID()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getTitleEffectID(%s)", __LINE__, m_uiClipID, m_strEffectTitleID);
	return m_pTitleEffectID;
}

int CVideoEffectItem::setEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiEffectStartTime, unsigned int uiDuration, int iOffset, int iOverlap, const char* pEffectID, const char* pTitle, int iCurrentIdx, int iTotalCount)
{
	clearEffectItemInfo();

	m_uiStartTime		= uiStartTime;
	m_uiEndTime			= uiEndTime;

	m_uiEffectStartTime	= uiEffectStartTime;
	m_uiEffectEndTime		= uiEffectStartTime + uiDuration;
	
	m_uiEffectDuration		= uiDuration;
	m_iEffectOffset		= iOffset;
	m_iEffectOverlap		= iOverlap;

	if( pEffectID != NULL )
	{
		m_pEffectID = (char*)nexSAL_MemAlloc(strlen(pEffectID)+1);
		if( m_pEffectID == NULL )
			return FALSE;
		strcpy(m_pEffectID, pEffectID);
	}

	if( pTitle != NULL )
	{
		m_pTitle = (char*)nexSAL_MemAlloc(strlen(pTitle)+1);
		if( m_pTitle == NULL )
			return FALSE;
		strcpy(m_pTitle, pTitle);
	}

	m_iCurrentIdx			= iCurrentIdx;
	m_iTotalCount				= iTotalCount;

	m_eEffectType			= EFFECT_TYPE_TRANSITION;
	return TRUE;
}

int CVideoEffectItem::setTitleEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiTitleStartTime, unsigned int uiTitleEndTime, const char* pTitleID, const char* pTitle, int iCurrentIdx, int iTotalCount)
{
	clearEffectItemInfo();

	m_uiStartTime			= uiStartTime;
	m_uiEndTime				= uiEndTime;

	m_uiTitleStartTime			= uiTitleStartTime;
	m_uiTitleEndTime			= uiTitleEndTime;
	m_pTitle					= NULL;
	m_pTitleEffectID			= NULL;

	if( pTitleID != NULL ) 
	{
		m_pTitleEffectID = (char*)nexSAL_MemAlloc(strlen(pTitleID)+1);
		if( m_pTitleEffectID == NULL )
			return FALSE;
		strcpy(m_pTitleEffectID, pTitleID);
	}

	if( pTitle != NULL )
	{
		m_pTitle = (char*)nexSAL_MemAlloc(strlen(pTitle)+1);
		if( m_pTitle == NULL )
			return FALSE;
		strcpy(m_pTitle, pTitle);
	}

	m_iCurrentIdx			= iCurrentIdx;
	m_iTotalCount				= iTotalCount;

	m_eEffectType			= EFFECT_TYPE_TITLE;
	return TRUE;
}

int CVideoEffectItem::applyEffectItem(void* pRender, unsigned int uiTime)
{
	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[EffectItem.cpp %d] applyEffect (%d %d %d %d %p) Time:%d", 
	//	__LINE__, m_uiEffectStartTime, m_uiEffectDuration, m_iEffectOffset, m_iEffectOverlap, m_pEffectID, uiTime);

	if( m_bEffectApplyEnd || m_uiEffectStartTime > uiTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[EffectItem.cpp %d] Order(%d) applyEffect not time(Cur:%d Start:%d End:%d)", 
				__LINE__, m_iCurrentIdx, uiTime, m_uiEffectStartTime, m_uiEffectEndTime);
		m_bEffectStart = FALSE;
		return 0;
	}

	float fTime = (float)(uiTime - m_uiEffectStartTime) / m_uiEffectDuration;
	fTime = fTime > 0.96 ? 0.99: fTime;
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Order(%d) applyEffect TimeInfo(Cur:%d Start:%d Dur:%d)", 
	//	__LINE__, m_iCurrentIdx, uiTime, m_uiStartTime, m_uiEndTime);

	if( m_bEffectStart )
	{
		if( uiTime > m_uiEffectEndTime )
		{
			NXT_ThemeRenderer_ClearTransitionEffect((NXT_HThemeRenderer)pRender);
			// NXT_ThemeRenderer_SetCTS((NXT_HThemeRenderer)pRender, uiTime);

			if( m_iEffectOverlap > 0 )
				NXT_ThemeRenderer_SwapTextures((NXT_HThemeRenderer)pRender, NXT_TextureID_Video_1, NXT_TextureID_Video_2);
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Order(%d) applyEffect Effect End", __LINE__, m_iCurrentIdx);	
			m_bEffectStart		= FALSE;
			m_bEffectApplyEnd		= TRUE;
			return 100;
		}

		// NXT_ThemeRenderer_SetCTS((NXT_HThemeRenderer)pRender, uiTime);	
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Order(%d) applyEffect Effect process(T: %f)", __LINE__, m_iCurrentIdx, fTime);	

		return int(fTime * 100);
	}

	if( m_uiEffectStartTime <= uiTime &&  uiTime < m_uiEffectEndTime )
	{
		NXT_ThemeRenderer_SetTransitionEffect(	(NXT_HThemeRenderer)pRender,
												m_pEffectID,  // ID of transition effect, or NULL to clear current effect
												m_pTitle,
												m_iCurrentIdx,         // Index of the transition's clip (the clip before the transition), from 0
												m_iTotalCount,    // Total number of clips in the project
												m_uiEffectStartTime,   // Start time for this effect
												m_uiEffectEndTime );    // End time for this effect

		// NXT_ThemeRenderer_SetCTS((NXT_HThemeRenderer)pRender, uiTime);		

		m_bEffectStart = TRUE;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Order(%d) applyEffect Effect Time(cur:%d s:%d e:%d) Total(%d)",
			__LINE__, m_iCurrentIdx, uiTime, m_uiEffectStartTime, m_uiEffectEndTime, m_iTotalCount);	
		return  int(fTime * 100);
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] applyEffect Out", __LINE__);	
	return 0;	
}

int CVideoEffectItem::applyTitleItem(void* pRender, unsigned int uiTime)
{
	 // nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[EffectItem.cpp %d] ID(%d) applyTitleEffect In(%d %d) Flag : %d, Cur(%d)",
	 // 	 __LINE__, m_uiTrackID, m_uiTitleStartTime, m_uiTitleEndTime, m_bTitleEffectStart, uiCurrentTime);

	if( m_bTitleEffectApplyEnd || m_uiTitleStartTime > uiTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[EffectItem.cpp %d] Order(%d) applyTitleItem not time(Cur:%d Start:%d End:%d)", 
				__LINE__, m_iCurrentIdx, uiTime, m_uiStartTime, m_uiEndTime);	
		m_bTitleEffectStart = FALSE;
		return 100;
	}

	// for mantis 7765 problem
	unsigned int uiTitleEndTime = m_uiTitleEndTime;

	if( (m_iCurrentIdx+1) < m_iTotalCount )
	{
		uiTitleEndTime = uiTitleEndTime < 20 ? 0 : uiTitleEndTime - 20;
	}

	if( m_uiTitleStartTime > uiTime || uiTime > uiTitleEndTime )
	{
		m_bTitleEffectStart = FALSE;
		m_bTitleEffectApplyEnd = TRUE;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] ID(%d) Title Effect Not Time(c:%d s:%d e:%d)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, uiTitleEndTime);
		return 100;
	}

	if( m_bTitleEffectStart )
	{
		if( uiTime > uiTitleEndTime )
		{
			m_bTitleEffectStart = FALSE;
			m_bTitleEffectApplyEnd = TRUE;
			NXT_ThemeRenderer_ClearClipEffect((NXT_HThemeRenderer)pRender);
			// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Order(%d) applyTitleItem End", __LINE__, m_iCurrentIdx);	
			return 100;
		}
		// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] apply Title Effect Process(%d %d %d)",  __LINE__, uiTime, m_uiTitleStartTime, uiTitleEndTime);
		return 2;
	}

	NXT_ThemeRenderer_SetClipEffect(	(NXT_HThemeRenderer)pRender,
										m_pTitleEffectID,  // ID of clip effect, or NULL to clear current effect
										m_pTitle,
										m_iCurrentIdx,         // Index of this clip, from 0
										m_iTotalCount,    // Total number of clips in the project
										m_uiStartTime,     // Start time for this clip
										m_uiEndTime,       // End time for this clip
										m_uiTitleStartTime,   // Start time for this effect (must be >= clipStartTime)
										m_uiTitleEndTime );    // End time for this effect (must be <= clipEndTime)	
                                     
	// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] ID(%d) applyTitleItem Start(%d %d %d) TotalCount:%d CurrentIdx:%d (%s)",  __LINE__, 
		m_iCurrentIdx, uiTime, m_uiTitleStartTime, m_uiTitleEndTime, m_iTotalCount, m_iCurrentIdx, m_pTitle);	
	m_bTitleEffectStart = TRUE;
	return 2;
}

void CVideoEffectItem::printEffect()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] StartTime : %d", __LINE__, m_uiStartTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] EndTime : %d", __LINE__, m_uiEndTime);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] EffectStartTime : %d", __LINE__, m_uiEffectStartTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] EffectEndTime : %d", __LINE__, m_uiEffectEndTime);

	if( m_pEffectID != NULL && strcmp(m_pEffectID, "none") != 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] ID : %s", __LINE__, m_pEffectID);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Duration : %d", __LINE__, m_uiEffectDuration);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Offset : %d", __LINE__, m_iEffectOffset);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Overlap : %d", __LINE__, m_iEffectOverlap);

	}

	if( m_pTitleEffectID != NULL && strcmp(m_pTitleEffectID, "none") != 0 && m_pTitle != NULL ){
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] ID : %s", __LINE__, m_pTitleEffectID);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] Title : %s", __LINE__, m_pTitle);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] TitleStartTime : %d", __LINE__, m_uiTitleStartTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] TitleEndTime : %d", __LINE__, m_uiTitleEndTime);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[EffectItem.cpp %d] -------------------------------------------", __LINE__);

}
