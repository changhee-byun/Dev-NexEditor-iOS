/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_EffectItemVec.h
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

#ifndef __NEXVIDEOEDITOR_EFFECTITEMVEC_H__
#define __NEXVIDEOEDITOR_EFFECTITEMVEC_H__

#include "NEXVIDEOEDITOR_EffectItem.h"

class CVideoEffectItemVec : public CNxRef<INxRefObj>
{
	CNexLock				m_Lock;
public:
	CVideoEffectItemVec();
	virtual ~CVideoEffectItemVec();

	int getCount();
	CVideoEffectItem* getEffetItem(int index);

	NXBOOL addEffectItem(CVideoEffectItem* pItem);
	NXBOOL clearEffectItem();
	NXBOOL clearEffectFlag();

	CVideoEffectItem* getActiveEffect(unsigned int uiTime);
	CVideoEffectItem* getActiveTitle(unsigned int uiTime);
	
	CVideoEffectItem* getFirstEffect();
	CVideoEffectItem* getNextEffect();

	int applyEffectItem(void* pRender, unsigned int uiTime);
	int applyTitleItem(void* pRender, unsigned int uiTime);	

	void printEffectInfo();
	void lock();
	void unlock();

	EffectItemVec			m_EffectVec;
	CVideoEffectItem*		m_pCurrentEffectItem;
	int					m_iCurrentEffectIndex;
	CVideoEffectItem*		m_pCurrentTitleItem;		
	int					m_iCurrentTitleIndex;	
};

#endif // __NEXVIDEOEDITOR_EFFECTITEMVEC_H__
