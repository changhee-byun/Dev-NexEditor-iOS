/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_EffectItem.h
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

#ifndef __NEXVIDEOEDITOR_EFFECTITEM_H__
#define __NEXVIDEOEDITOR_EFFECTITEM_H__

#include "NexSAL_Internal.h"
#include "nexIRef.h"
#include "nexDef.h"
#include "nexEvent.h"
#include "nexLock.h"
#include <vector>

#include "NEXVIDEOEDITOR_Types.h"

class CVideoEffectItem: public CNxRef<INxRefObj>
{
public:
	CVideoEffectItem();
	virtual ~CVideoEffectItem();

	NXBOOL isTransitionEffect();
	NXBOOL isTitleEffect();
	NXBOOL clearApplyFlag();
	NXBOOL clearEffectItemInfo();

	NXBOOL isActiveEffectTime(unsigned int uiTime);
	NXBOOL isActiveTitleTime(unsigned int uiTime);
	NXBOOL isActiveBGMTime(unsigned int uiTime);

	NXBOOL isSameForTitle(CVideoEffectItem* pItem);
	
	const char* getClipEffectID();
	const char* getTitleEffectID();
	
	virtual int setEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiEffectStartTime, unsigned int uiDuration, int iOffset, int iOverlap, const char* pEffectID, const char* pTitle, int iCurrentIdx, int iTotalCount);
	virtual int setTitleEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiTitleStartTime, unsigned int uiTitleEndTime, const char* pTitleID, const char* pTitle, int iCurrentIdx, int iTotalCount);

	int applyEffectItem(void* pRender, unsigned int uiTime);
	int applyTitleItem(void* pRender, unsigned int uiTime);

	void printEffect();
	
	unsigned int			m_uiStartTime;
	unsigned int			m_uiEndTime;

	unsigned int			m_uiEffectStartTime;
	unsigned int			m_uiEffectEndTime;
	unsigned int			m_uiEffectDuration;
	int 					m_iEffectOffset;
	int 					m_iEffectOverlap;
    
	char*					m_pEffectID;

	NXBOOL					m_bEffectStart;
	NXBOOL					m_bEffectApplyEnd;
	
	unsigned int			m_uiTitleStartTime;
	unsigned int			m_uiTitleEndTime;
		
	char*					m_pTitle;
	char*					m_pTitleEffectID;
	
	NXBOOL					m_bTitleEffectStart;
	NXBOOL					m_bTitleEffectApplyEnd;

	void*					m_pFilterEffect;

	int						m_iCurrentIdx;
	int						m_iTotalCount;

	EFFECT_TYPE				m_eEffectType;

};

typedef std::vector<CVideoEffectItem*>	EffectItemVec;
typedef EffectItemVec::iterator			EffectItemVecIter;

#endif // __NEXVIDEOEDITOR_EFFECTITEM_H__
