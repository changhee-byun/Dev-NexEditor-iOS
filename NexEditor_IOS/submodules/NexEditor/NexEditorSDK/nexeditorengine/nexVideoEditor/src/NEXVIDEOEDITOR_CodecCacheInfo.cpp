/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CodecCacheInfo.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
salabara		2018/02/07	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_CodecCacheInfo.h"
#include "NexCodecUtil.h"

CCodecTime::CCodecTime()
{
	m_uClipID = 0;
	m_uiStartTime	= 0;
	m_uiEndTime	= 0;
	m_bMyDecoder = FALSE;
	m_bMyRenderer = FALSE;
}

CCodecTime::CCodecTime(unsigned int uiClipID, unsigned int uiStartTime, unsigned int uiEndTime)
{
	m_uClipID = uiClipID;
	m_uiStartTime	= uiStartTime;
	m_uiEndTime	= uiEndTime;
	m_bMyDecoder = FALSE;
	m_bMyRenderer = FALSE;    
}

unsigned int CCodecTime::getStartTime()
{
	return m_uiStartTime;
}

unsigned int CCodecTime::getEndTime()
{
	return m_uiEndTime;
}

unsigned int CCodecTime::getClipID()
{
	return m_uClipID;	
}

void CCodecTime::setMyDecoder(NXBOOL bMy)
{
	m_bMyDecoder = bMy;	
}

NXBOOL CCodecTime::getMyDecoder()
{
	return m_bMyDecoder;	
}

void CCodecTime::setMyRenderer(NXBOOL bMy)
{
	m_bMyRenderer = bMy;	
}

NXBOOL CCodecTime::getMyRenderer()
{
	return m_bMyRenderer;	
}

CCodecCacheInfo::CCodecCacheInfo()
{
	m_eCodecType = eNEX_CODEC_UNKNOWN;
	m_pDSI = NULL;
	m_uDSISize = 0;	
	m_pCodecWrap = NULL;	

	clear();
}

CCodecCacheInfo::~CCodecCacheInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] ~~~~CCodecCacheInfo In", __LINE__);
	if(m_pDSI)
	{
		nexSAL_MemFree(m_pDSI);
	}
	SAFE_RELEASE(m_pCodecWrap);
	clear();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] ~~~~CCodecCacheInfo In", __LINE__);    
}

void	CCodecCacheInfo::setCodecWrap(CNexCodecWrap* pCodecWrap)
{
	CAutoLock m(m_CachedLock);
    
	SAFE_ADDREF(pCodecWrap);
	m_pCodecWrap = pCodecWrap;
}

CNexCodecWrap*	CCodecCacheInfo::getCodecWrap()
{
	CAutoLock m(m_CachedLock);
	SAFE_ADDREF(m_pCodecWrap);
	return m_pCodecWrap;
}

void CCodecCacheInfo::setNextDecoder(unsigned int uClipID)
{
	CAutoLock m(m_CachedLock);
	vecCodecTimeItr i;

	for( i = begin(); i != end(); i++)
	{
		CCodecTime* codecTime = (CCodecTime*)*i;
		if(codecTime->getClipID() == uClipID)
		{
			i++;		
			CCodecTime* codecTime = (CCodecTime*)*i;         
			codecTime->setMyDecoder(TRUE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] setNextDecoder (%d, %d)", __LINE__, codecTime->getClipID(), codecTime->getMyDecoder());
			break;
		}
	}
}

void CCodecCacheInfo::setNextRenderer(unsigned int uClipID)
{
	CAutoLock m(m_CachedLock);
	vecCodecTimeItr i;

	for( i = begin(); i != end(); i++)
	{
		CCodecTime* codecTime = (CCodecTime*)*i;
		if(codecTime->getClipID() == uClipID)
		{
			i++;		
			CCodecTime* codecTime = (CCodecTime*)*i;         
			codecTime->setMyRenderer(TRUE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] setNextRenderer (%d, %d)", __LINE__, codecTime->getClipID(), codecTime->getMyRenderer());
			break;
		}
	}
}

NXBOOL CCodecCacheInfo::getMyDecoder(unsigned int uClipID)
{
	CAutoLock m(m_CachedLock);
	vecCodecTimeItr i;

	for( i = begin(); i != end(); i++)
	{
		CCodecTime* codecTime = (CCodecTime*)*i;
		if(codecTime->getClipID() == uClipID)
		{
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] getMyDecoder (%d, %d, %d)", __LINE__, uClipID, codecTime->getClipID(), codecTime->getMyDecoder());
			return codecTime->getMyDecoder();
		}
	}

	return FALSE;
}

NXBOOL CCodecCacheInfo::getMyRenderer(unsigned int uClipID)
{
	CAutoLock m(m_CachedLock);
	vecCodecTimeItr i;

	for( i = begin(); i != end(); i++)
	{
		CCodecTime* codecTime = (CCodecTime*)*i;
		if(codecTime->getClipID() == uClipID)
		{
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] getMyRenderer (%d, %d, %d)", __LINE__, uClipID, codecTime->getClipID(), codecTime->getMyRenderer());
			return codecTime->getMyRenderer();
		}
	}

	return FALSE;
}

void	CCodecCacheInfo::addCodecTimeVec(unsigned int uClipID, NEX_CODEC_TYPE eCodecType, unsigned int uiStartTime, unsigned int uiEndTime, unsigned char* pDSI, unsigned int uDSISize)
{
	CAutoLock m(m_CachedLock);

	CCodecTime* codecTime = new CCodecTime(uClipID, uiStartTime, uiEndTime);
	insert(end(), codecTime);

	m_eCodecType = eCodecType;
	if(m_pDSI == NULL && pDSI != NULL)
	{
		m_pDSI = (unsigned char*)nexSAL_MemAlloc(uDSISize);
		memcpy(m_pDSI, pDSI, uDSISize);
		m_uDSISize = uDSISize;
		codecTime->setMyDecoder(TRUE);
		codecTime->setMyRenderer(TRUE);        
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] addCodecTimeVec (%d, %p %d, %d)", __LINE__, size(), this, uClipID, m_eCodecType);
}

void CCodecCacheInfo::removeCodecTimeVec(unsigned int uClipID)
{
	CAutoLock m(m_CachedLock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] removeCodecTimeVec (%p, %d)", __LINE__, this, uClipID);

	if( size() > 0 )
	{
		vecCodecTimeItr i;
		for( i = begin(); i != end(); i++)
		{
			CCodecTime* codecTime = (CCodecTime*)*i;
			if( codecTime->getClipID() == uClipID )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] removeCodecTimeVec (%p %d, %d)", __LINE__, this, uClipID, size());
				erase(i);
				break;
			}
		}
	}
}

unsigned int CCodecCacheInfo::getSize()	
{
	CAutoLock m(m_CachedLock);
	return size();
}

NXBOOL CCodecCacheInfo::checkReUseCodec(unsigned int uClipID, NEX_CODEC_TYPE eCodecType, unsigned int uStartTime, unsigned int uEndTime, unsigned char* pDSI, unsigned int uDSISize)
{
	CAutoLock m(m_CachedLock);
	NXBOOL bReUse = FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec (%d)", __LINE__, size());
    
	if(size() == 0)
		return bReUse;
    
	vecCodecTimeItr i;
	i = end();
	i--;
	CCodecTime* codecTime = (CCodecTime*)*i;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec (%d %d)", __LINE__, m_eCodecType, eCodecType);

	if(m_eCodecType == eCodecType)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec (%d %d)", __LINE__, codecTime->getEndTime(), uStartTime);
		if(codecTime->getEndTime() == uStartTime)
		{
			if( eCodecType == eNEX_CODEC_V_MPEG4V ) 
			{
				if( NexCodecUtil_MPEG4V_IsDirectMixable(m_pDSI, m_uDSISize, pDSI, uDSISize) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec DSI match(%d %d)", __LINE__, codecTime->getClipID(), uClipID);
					bReUse = TRUE;
				}
			}
			else if(eCodecType == eNEX_CODEC_V_H264)
			{
				if( NexCodecUtil_AVC_IsDirectMixable(m_pDSI, m_uDSISize, pDSI, uDSISize) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec DSI match(%d %d)", __LINE__, codecTime->getClipID(), uClipID);
					bReUse = TRUE;
				}
			}
			else if(eCodecType == eNEX_CODEC_V_HEVC)
			{
				if( NexCodecUtil_HEVC_IsDirectMixable(m_pDSI, m_uDSISize, pDSI, uDSISize) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CCodecCache.cpp %d] checkReUseCodec DSI match(%d %d)", __LINE__, codecTime->getClipID(), uClipID);
					bReUse =  TRUE;
				}
			}
		}
	}

	return bReUse;
}
