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

#ifndef __NEXVIDEOEDITOR_CODECCACHEINFO_H__
#define __NEXVIDEOEDITOR_CODECCACHEINFO_H__

#include "NexSAL_Internal.h"
#include <vector>
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include "NexCAL.h"
#include "NexMediaDef.h"
#include "nexLock.h"

class CCodecTime
{
public:
	CCodecTime();
	CCodecTime(unsigned int uiClipID, unsigned int uiStartTime, unsigned int uiEndTime);
	~CCodecTime(){};

	unsigned int getStartTime();
	unsigned int getEndTime();
	unsigned int getClipID();
	
	NXBOOL getMyDecoder();
	void setMyDecoder(NXBOOL bMy);

	NXBOOL getMyRenderer();
	void setMyRenderer(NXBOOL bMy);
	
	unsigned int m_uClipID;
	unsigned int m_uiStartTime;
	unsigned int m_uiEndTime;
	NXBOOL m_bMyDecoder;
	NXBOOL m_bMyRenderer;	
};

typedef std::vector<CCodecTime*> vecCodecTime;
typedef vecCodecTime::iterator vecCodecTimeItr;

class CCodecCacheInfo : public vecCodecTime, public CNxRef<INxRefObj>
{
public:
	CCodecCacheInfo();
	~CCodecCacheInfo();
	
	void	setCodecWrap(CNexCodecWrap* pCodecWrap);
	CNexCodecWrap*	getCodecWrap();
	void setNextDecoder(unsigned int uClipID);
	void setNextRenderer(unsigned int uClipID);	
	NXBOOL getMyDecoder(unsigned int uClipID);
	NXBOOL getMyRenderer(unsigned int uClipID);	
	void	addCodecTimeVec(unsigned int uClipID, NEX_CODEC_TYPE eCodecType, unsigned int uiStartTime, unsigned int uiEndTime, unsigned char* pDSI, unsigned int uDSISize);
	void removeCodecTimeVec(unsigned int uClipID);
	unsigned int getSize();
	NXBOOL checkReUseCodec(unsigned int uClipID, NEX_CODEC_TYPE eCodecType, unsigned int uStartTime, unsigned int uEndTime, unsigned char* pDSI, unsigned int uDSISize);

	CNexLock					m_CachedLock;

	NEX_CODEC_TYPE		m_eCodecType;
	unsigned char* m_pDSI;
	unsigned int m_uDSISize;	
	CNexCodecWrap* m_pCodecWrap;	
	
};

typedef std::vector<CCodecCacheInfo*> vecCodecCacheInfo;
typedef vecCodecCacheInfo::iterator vecCodecCacheInfoItr;

#endif // __NEXVIDEOEDITOR_CODECCACHEINFO_H__
