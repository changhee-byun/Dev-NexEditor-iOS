/******************************************************************************
 * File Name   :	NEXVIDEOEDITOR_UDTA.h
 * Description :
 *******************************************************************************

 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation

 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 cskb		2015/10/21	Draft.
 -----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_UDTA_H__
#define __NEXVIDEOEDITOR_UDTA_H__

#include "NexSAL_Internal.h"
#include <vector>

class CNexUDTA
{
public:
	CNexUDTA();
	~CNexUDTA();

	NXBOOL setUDTA(int iType, const char* pUDTA);
	
	int m_iType;
	char m_pData[128];
};

typedef std::vector<CNexUDTA> vecUDTA;
typedef vecUDTA::iterator vecUDTAItr;

#endif // __NEXVIDEOEDITOR_UDTA_H__
